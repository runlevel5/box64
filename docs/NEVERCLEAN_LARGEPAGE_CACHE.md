# NEVERCLEAN Large-Page Dispatch Cache (D+B Hybrid)

## Problem

On systems with host pages larger than 4KB (e.g., 64KB on PPC64LE, 16KB on some ARM64),
box64's dynarec cannot use `mprotect()` to write-protect code pages when they share a
host page with writable data. Instead, these blocks are marked `PROT_NEVERCLEAN`, forcing
a full hash revalidation on every dispatch.

Profiling on PPC64LE with Grim Fandango Remastered shows:
- **100%** of 441M block dispatches are NEVERCLEAN
- **100%** of hash validations match (code never changes)
- **~55.6%** of CPU time is spent in block lookup/validation
- Only **~3.3%** runs actual dynarec'd code

The hash check (`ppc64le_fast_hash` at 19.81% CPU) is pure overhead for games with
stable code.

## Root Cause

When `box64_pagesize > 4096`, `protectDB()` and `protectDBJumpTable()` in `custommem.c`
call `hostPageHasExternalWrite_locked()` to check if other 4KB sub-pages within the same
host page have writable data. If yes, calling `mprotect()` to remove `PROT_WRITE` would
break data writes (kernel syscalls like `read()` would return `EFAULT`), so the page is
marked `PROT_NEVERCLEAN` instead.

For 32-bit games on 64KB-page systems, code and data are tightly packed in the low 4GB —
nearly every host page contains both.

Commit `fe400fbaa` excluded all NEVERCLEAN blocks from the per-thread dispatch cache (to
fix test21's self-modifying code), which means every block dispatch goes through the full
`LinkNext -> DBGetBlock -> hash computation` path (~160 instructions vs ~15 for a cache hit).

## Solution: D+B Hybrid

Distinguish "NEVERCLEAN because of large host pages" (code is stable, safe to cache) from
"NEVERCLEAN for other reasons" (truly self-modifying, MAP_SHARED, etc). Cache only the
former, with a configurable countdown that forces periodic hash revalidation as
defense-in-depth.

### New Protection Flag

A new flag `PROT_NEVERCLEAN_LARGEPAGE` (0x200) is added to `custommem.h`. Both
`PROT_NEVERCLEAN` and `PROT_NEVERCLEAN_LARGEPAGE` are set together in the large-page path
of `protectDB()`/`protectDBJumpTable()`. Other NEVERCLEAN sources (`neverprotectDB()`,
MAP_SHARED mmap) set only `PROT_NEVERCLEAN`, so the flag precisely distinguishes the safe
case.

### New `always_test` Value

The `always_test` field in `dynablock_t` (2-bit, values 0-3) gains a new value:

| Value | Meaning | Cached? | Hash Validated? |
|-------|---------|---------|-----------------|
| 0 | Clean (write-protected) | Yes (gen counter) | No (mprotect traps writes) |
| 1 | NEVERCLEAN (truly unsafe) | No | Every dispatch |
| 2 | Hot page (transient) | No | Every dispatch |
| 3 | NEVERCLEAN-largepage (stable) | Yes (countdown) | Every Nth dispatch |

### Countdown Mechanism

A per-thread countdown (`block_cache_validate_countdown` in `x64emu_t`) is decremented on
every dispatch cache hit in `ppc64le_next.S`. When it reaches 0, the assembly falls
through to the slow path (`LinkNext -> DBGetBlock`), which:

1. Hash-validates the block (catching any theoretical code mutation)
2. Re-populates the cache entry
3. Resets the countdown to N

The countdown value N is configurable at build time via cmake:
`-DNEVERCLEAN_COUNTDOWN=1024` (default: 1024). Setting it to 0 disables caching of
NEVERCLEAN-largepage blocks entirely (equivalent to current behavior).

### Correctness by Game Category

| Scenario | `always_test` | Cached? | Why It's Safe |
|----------|---------------|---------|---------------|
| Normal game on 4KB pages | 0 | Yes | mprotect traps writes, gen counter invalidates cache |
| Normal game on 64KB/16KB pages | 3 | Yes | Code doesn't change; countdown revalidates every Nth hit |
| JIT engine (memcpy to RWX) | 1 | No | neverprotectDB sets only PROT_NEVERCLEAN (no LARGEPAGE) |
| Hot page (dirty > 1) | 2 | No | Same as today |
| MAP_SHARED + O_RDWR mmap | 1 | No | wrappedlibc.c sets only PROT_NEVERCLEAN |
| Write to code on large page | 3 | Cached but... | Countdown catches mutation within N dispatches |

### The Theoretical Gap

On a NEVERCLEAN-largepage page, the OS-level permission is RW (mprotect wasn't called).
If a game writes directly to code bytes, the write succeeds silently — no SEGV, no
`block_cache_generation` bump. With caching, stale code would be served for up to N
dispatches before the countdown forces revalidation.

This is acceptable because:
- Non-JIT games don't write to their `.text` segment
- JIT engines go through `mmap(PROT_RWX)` or `mprotect()`, which triggers
  `neverprotectDB()` (sets `always_test=1`, NOT cached)
- The countdown bounds the staleness window
- Setting `NEVERCLEAN_COUNTDOWN=0` disables caching entirely for paranoid builds

## Files Modified

| File | Change |
|------|--------|
| `src/include/custommem.h` | Add `PROT_NEVERCLEAN_LARGEPAGE` (0x200), update `PROT_DYN`/`PROT_CUSTOM` masks |
| `src/custommem.c` | Set both flags in protectDB/protectDBJumpTable large-page path |
| `src/dynarec/dynarec_native.c` | Post-compilation: set `always_test=3` when `PROT_NEVERCLEAN_LARGEPAGE` is present |
| `src/dynarec/dynarec_native_pass.c` | Pass0 page-crossing: set `always_test=3` for largepage NEVERCLEAN pages |
| `src/dynarec/dynablock.c` | Include `always_test==3` in PPC64LE mutex-free hash fast path |
| `src/dynarec/dynarec.c` | Cache `always_test==3` blocks in dispatch cache, reset countdown on validation |
| `src/dynarec/ppc64le/ppc64le_next.S` | No changes needed (countdown mechanism already implemented) |
| `CMakeLists.txt` | Add `NEVERCLEAN_COUNTDOWN` cmake option (default 1024) |

## Build

```sh
cmake .. -DPPC64LE_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DBOX32=ON -DBLOCK_CACHE=4096 -DNEVERCLEAN_COUNTDOWN=1024
make -j$(nproc)
```

## Testing

1. **ctests**: All tests must pass, especially `test21` (self-modifying code / `PROT_RWX`)
2. **Grim Fandango Remastered**: Verify performance improvement and no regressions
3. **perf profile**: Confirm `ppc64le_fast_hash` drops from ~20% to near-zero

## Related

- Commit `fe400fbaa`: Original fix excluding NEVERCLEAN from dispatch cache
- PR #3707: Grim Fandango ogg hook crash fix
- `NEVERCLEAN_PROFILE` cmake option: Profiling instrumentation that produced the data
  motivating this change
