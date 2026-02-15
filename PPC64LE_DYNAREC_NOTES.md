# PPC64LE Dynarec Development Notes

> This document preserves session context for the PPC64LE dynarec implementation in box64.
> It should be updated as work progresses so a new session can pick up where the last left off.
> Last updated: 2026-02-16 (session 4 — emit helper files)

---

## Goal

Add PPC64LE (PowerPC 64-bit Little Endian) dynarec (dynamic recompiler) support to the box64 project. Box64 is a Linux userspace x86-64 emulator. PPC64LE currently exists as an interpreter-only target; the goal is to add a full dynarec backend similar to the existing ARM64, RV64, and LA64 backends. The immediate milestone is to get the project to **compile** with `PPC64LE_DYNAREC=ON` (even with stub implementations), then iteratively add opcode translations.

---

## Instructions

- **Git branch**: All work on the `ppc64le-dynarec` branch (created off `main` at commit `521b3784b`)
- **Minimum ISA target**: POWER9 (ISA 3.0)
- **Follow existing patterns**: ARM64 backend is the primary reference. LA64 is secondary reference (closest in structure for helper.c). All new files follow same naming conventions, macro patterns, and architectural abstractions.
- **Primary reference for helper.c**: `src/dynarec/la64/dynarec_la64_helper.c` (2312 lines). Adapt instruction-by-instruction with PPC64LE substitutions.
- **Remote build target**: `tle@192.168.1.247` — Fedora 43 ppc64le, GCC 15, cmake. Source synced via:
  ```
  rsync -avz --exclude='.git' --exclude='build*' --exclude='.cache' /Users/tle/Work/box64/ tle@192.168.1.247:~/box64/
  ```
  The repo is at `~/box64/` on remote, build dir is `~/box64/build/`.
- **CRITICAL rsync note**: The rsync command `--exclude='build*'` also excludes `src/build_info.c` and `src/build_info.h`. These must be synced separately:
  ```
  rsync -avz /Users/tle/Work/box64/src/build_info.c /Users/tle/Work/box64/src/build_info.h tle@192.168.1.247:~/box64/src/
  ```

---

## Discoveries

### Architecture Overview
- Box64 has 3 dynarec backends (ARM64, RV64, LA64) and 2 interpreter-only targets (PPC64LE, SW64)
- Each dynarec backend has ~50-60 files in `src/dynarec/<arch>/`
- The dynarec uses a 4-pass compilation system (pass 0-3): the same `.c` opcode files are compiled 4 times with different `STEP` defines
- Architecture-specific code is dispatched via `#ifdef` chains in ~6 "dispatch headers"

### PPC64LE Register Mapping
- 32 GPRs (r0-r31): 18 callee-saved (r14-r31)
- Mapping: r14-r29 = xRAX-xR15 (sequential), r30 = xFlags, r31 = xEmu, r9 = xRIP, r12 = xSavedSP
- Scratch registers: r3-r8 (x1-x6), r10 (x7), r11 (x87pc)
- r0 is special (treated as 0 in D-form load/store base), r1 = SP, r2 = TOC (reserved), r13 = TLS (reserved)
- 32 VMX regs (vr0-vr31, 128-bit): 12 callee-saved (vr20-vr31) for persistent XMM caching
- **No xRA register** — PPC64LE LR is special-purpose, accessed via MFLR/MTLR. Use scratch GPR + MTCTR/BCTRL for calls.
- **No zero register** — xZR=0 (r0) only acts as literal 0 in D-form base context. Cannot store to xZR; must use `LI(scratch, 0); STW(scratch, ...)`.

### PPC64LE Key Technical Details
- ELFv2 ABI: 288-byte red zone, no mandatory parameter save area
- Atomics: LL/SC at all widths (lbarx/stbcx., lharx/sthcx., lwarx/stwcx., ldarx/stdcx.) — follows ARM64's separate read/write lock pattern, NOT CAS pattern
- Branch ranges: unconditional ±32MB (I-form, 24-bit), conditional ±32KB (B-form, 14-bit)
- Immediates: 16-bit signed for most D-form (wider than LA64's ±2048), 14-bit×4 for DS-form (ld/std)
- Memory barriers: `sync` (full), `lwsync` (lightweight), `isync` (instruction)
- Cache coherency: needs `dcbst` + `sync` + `icbi` + `isync` after code generation
- **D-form operand order**: `LD(Rt, offset, Ra)` — offset comes BEFORE base register
- **ANDId (andi.)** always sets CR0 — useful for branch-after-test
- No hardware CRC32 — software fallback in lock.S
- No PC-relative load (pre-POWER10)

### PPC64LE Assembly Syntax
- **GAS requires bare numbers or %rN for registers** — `std 14, 32(1)` or `std %r14, 32(%r1)` work, but `std r14, 32(r1)` does NOT (treated as symbols, gives "unsupported relocation" errors)
- All `.S` files must use bare register numbers (no `r` prefix) since C preprocessor macros expand to numbers

### TABLE64 Strategy
4 instructions per TABLE64 load:
1. `BCL 20,31,.+4` — get PC into LR
2. `MFLR(Rd)` — move LR to target register
3. `ADDIS(Rd, Rd, PPC64_HI16(delta-8))` — add upper 16 bits
4. `LD(Rd, PPC64_LO16(delta-8), Rd)` — load 64-bit value

### 4-Pass System
- Pass 0: EMIT counts bytes. Pass 1: no-op EMIT, FPU type resolution. Pass 2: EMIT counts per-instruction sizes. Pass 3: EMIT writes actual code.
- Uses `dyn->v` for VMX cache (like `dyn->n` for ARM64 NEON, `dyn->lsx` for LA64)
- Uses enum `flagcache_t` pattern (like ARM64/LA64)

### Key LA64→PPC64LE Instruction Mappings
| LA64 | PPC64LE | Notes |
|------|---------|-------|
| `ADDI_D` | `ADDI` | |
| `ADD_D` | `ADD` | |
| `SLLI_D` | `SLDI` | |
| `SRLI_D` | `SRDI` | |
| `LD_D` | `LD` | |
| `ST_D` | `STD` | |
| `LD_W` | `LWZ` | |
| `ST_W` | `STW` | |
| `LD_H` | `LHA` | |
| `ST_H` | `STH` | |
| `VLD` | `LXV` | swap operands |
| `VST` | `STXV` | swap operands |
| `FLD_D` | `LFD` | |
| `FST_D` | `STFD` | |
| `FCVT_D_S` | `FMR` | no-op, FPRs always hold doubles |
| `FCVT_S_D` | `FRSP` | |
| `FFINT_D_L` | `FCFID` | |
| `FTINTRZ_L_D` | `FCTIDZ` | |
| `SEXT_W` | `EXTSW` | |
| `BR(reg)` | `MTCTR(reg); BCTR()` | |
| `JIRL(xRA, reg, 0)` | `MTCTR(reg); BCTRL()` | |
| `BNE(r1,r2,off)` | `CMPD(r1,r2); BNE(off)` | |
| `BEQZ(r,off)` | `CMPDI(r,0); BEQ(off)` | |
| `VOR_V` | `XXLOR` | |
| `VXOR_V` | `XXLXOR` | |

- All LASX 256-bit ops → 128-bit VMX + memory ops for upper half (stored in `ymm[]` memory)
- `geted()` maxval: 32767 (vs LA64's 2047)
- VMX cache: 32 entries (vs LA64's 24)

### VSX Register File Model
- All 32 vmxcache slots map to VSX vs0-vs31 (= FPR f0-f31), NOT VMX vr0-vr31
- Slots 0-15 (XMM/YMM): 128-bit via VSX instructions (LXV, STXV, XXLOR, XXLXOR)
- Slots 16-23+ (x87/MMX): 64-bit via FPR instructions (LFD, STFD, FMR, FCTIDZ, FCFID, FRSP)
- SCRATCH=31 (vs31), SCRATCH0=24 (vs24/f24)

### AVX Without 256-bit Registers
- VMX register holds lower 128 bits, upper 128 bits stay in `ymm[i]` memory
- `zero_upper==1` → store zeros to ymm[i] on reflect; `zero_upper==0` → upper already in memory

### Function Signature Differences
- `ppc64le_move32` takes 5 args: `(dyn, ninst, reg, val, zeroup)` — MOV32w macro must pass `1` for zeroup
- `fpu_get_reg_x87`: LA64/PPC64LE = 3 args `(dyn, t, n)`, ARM64 = 4 args
- `fpu_get_reg_emm`: LA64/PPC64LE = 2 args `(dyn, emm)`
- `fpu_purgecache`: PPC64LE uses 6-arg version like LA64/RV64 (NOT 7-arg ARM64 version)

---

## Accomplished

### All PPC64LE Dynarec Files Created (28 files in `src/dynarec/ppc64le/`)
1. `ppc64le_mapping.h` — Register mapping definitions
2. `dynarec_ppc64le_private.h` — Data structures (vmx_cache_t, avx_cache_t, vmxcache_t, flagcache_t, instruction_ppc64le_t, dynarec_ppc64le_t)
3. `ppc64le_emitter.h` — Comprehensive instruction encoding macros (~1000+ lines)
4. `ppc64le_prolog.S` — Function entry, saves callee-saved regs, loads x86 state
5. `ppc64le_epilog.S` — Stores x86 regs back, restores callee-saved regs
6. `ppc64le_next.S` — Saves volatile regs, calls LinkNext, branches to resolved target
7. `ppc64le_lock.S` — Complete atomic operations + software CRC32C
8. `ppc64le_lock.h` — Lock function prototypes
9. `dynarec_ppc64le_jmpnext.c` — 7-instruction CreateJmpNext
10. `ppc64le_printer.h` / `ppc64le_printer.c` — Stub printer
11. `dynarec_ppc64le_arch.h` / `dynarec_ppc64le_arch.c` — Arch stubs
12. `dynarec_ppc64le_consts.h` / `dynarec_ppc64le_consts.c` — ~95 constants with full getConst
13. `dynarec_ppc64le_pass0.h` through `dynarec_ppc64le_pass3.h` — All 4 pass macros
14. `dynarec_ppc64le_functions.h` / `dynarec_ppc64le_functions.c` — VMX cache management (~700 lines)
15. `dynarec_ppc64le_helper.h` — Complete header (1987 lines)
16. `dynarec_ppc64le_helper.c` — **COMPLETE** (2414 lines)
17. `dynarec_ppc64le_00.c` — Primary opcode table stub (DEFAULT for all opcodes)
18. `dynarec_ppc64le_f0.c` — LOCK prefix opcode table stub
19. `dynarec_ppc64le_66.c` — 66h prefix opcode table stub
20. `dynarec_ppc64le_66f0.c` — 66h+LOCK prefix opcode table stub

### Emit Helper Files (Phase 1 — in progress)
21. `dynarec_ppc64le_emit_tests.c` — **COMPLETE** (532 lines, 11 functions: emit_cmp8/8_0/16/16_0/32/32_0, emit_test8/8c/16/32/32c) — commit `adc2d162c`
22. `dynarec_ppc64le_emit_math.c` — **COMPLETE** (1471 lines, 27 functions: emit_add32/32c/8/8c/16, emit_sub8/8c/16/32/32c, emit_sbb8/8c/16/32, emit_adc8/8c/16/32, emit_inc8/16/32, emit_dec8/16/32, emit_neg8/16/32) — commit `337af4a4b`
23. `dynarec_ppc64le_emit_logic.c` — **COMPLETE** (15 functions: emit_xor8/8c/16/32/32c, emit_and8/8c/16/32/32c, emit_or8/8c/16/32/32c)

### helper.c Contains All Functions
`geted`, `geted16`, `jump_to_epilog`, `jump_to_epilog_fast`, `jump_to_next`, `ret_to_next`, `iret_to_next`, `call_c`, `call_n`, `grab_segdata`, `x87_stackcount`, `x87_unstackcount`, `x87_forget`, `x87_reget_st`, `x87_free`, `x87_swapreg`, `x87_setround`, `sse_setround`, `vmxcache_st_coherency`, `x87_restoreround`, `x87_do_push`, `x87_do_push_empty`, `x87_do_pop`, `x87_purgecache`, `x87_reflectcount`, `x87_unreflectcount`, `x87_get_current_cache`, `x87_get_cache`, `x87_get_vmxcache`, `x87_get_st`, `x87_get_st_empty`, `mmx_get_reg`, `mmx_get_reg_empty`, `mmx_purgecache`, `sse_get_reg`, `sse_get_reg_empty`, `sse_forget_reg`, `sse_reflect_reg`, `sse_purge07cache`, `avx_get_reg`, `avx_get_reg_empty`, `avx_reflect_reg_upper128`, `avx_forget_reg`, `avx_reflect_reg`, `fpu_pushcache`, `fpu_popcache`, `fpu_purgecache`, `fpu_reflectcache`, `fpu_unreflectcache`, `emit_pf`, `fpu_reset_cache`, `fpu_propagate_stack`, `findCacheSlot`, `swapCache`, `loadCache`, `unloadCache`, `fpuCacheTransform`, `flagsCacheTransform`, `CacheTransform`, `ppc64le_move32`, `ppc64le_move64`

### Dispatch Header Modifications (5 files)
- `src/dynarec/dynarec_arch.h`
- `src/dynarec/native_lock.h`
- `src/dynarec/dynarec_next.h`
- `src/dynarec/dynarec_helper.h`
- `src/dynarec/dynacache_reloc.h`

### Other Modified Source Files (5 files)
- `src/dynarec/dynarec_native_functions.c` — added `|| defined(PPC64LE)` for flagsCacheNeedsTransform
- `src/dynarec/dynarec_native_pass.c` — added `|| defined(PPC64LE)` to 8 architecture guards
- `src/libtools/signals.c` — added PPC64LE CONTEXT_REG/CONTEXT_PC + arch fixup section
- `src/libtools/signal32.c` — added PPC64LE in 2 locations

### Build System / Host Detection (6 files)
- `CMakeLists.txt`
- `src/include/hostext.h`
- `src/os/hostext_linux.c`
- `src/os/hostext_common.c`
- `src/build_info.h`
- `src/tools/env.c`

---

## Current Status — EMIT HELPER FILES IN PROGRESS

The PPC64LE dynarec builds successfully with `PPC64LE_DYNAREC=ON`:
```
cmake .. -DPPC64LE_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
```

Output:
```
$ ./box64 --version
Box64 ppc64le v0.4.1  with Dynarec built on Feb 16 2026 07:06:25

$ BOX64_LOG=1 ./box64 /bin/true
[BOX64] Dynarec for PPC64LE (POWER9+, ISA 3.0)
[BOX64] Running on POWER9, altivec supported with 32 cores, pagesize: 65536
```

Emit helper file status:
- `emit_tests.c` — **COMPLETE** (11/11 functions)
- `emit_math.c` — **COMPLETE** (27/27 functions)
- `emit_logic.c` — **COMPLETE** (15/15 functions)
- `emit_shift.c` — NOT STARTED (0/50 functions)

All opcode tables currently stub to `DEFAULT` (fallback to interpreter). The binary runs but all x86-64 code falls back to interpretation.

### Compilation Fix History
1. **`build_info.c` missing on remote** — rsync `--exclude='build*'` pattern matches `build_info.*` files. Must sync separately.
2. **`flagsCacheNeedsTransform` in `dynarec_native_functions.c`** — had `#if defined(ARM64) || defined(LA64)` guard, PPC64LE uses same enum-based `flagcache_t`, so added `|| defined(PPC64LE)` to the guard.
3. **`native_lock_write_dq` expanding to empty** — `tmp32s = ;` syntax error. Fixed by making macro expand to `0` and `native_lock_read_dq` expand to `do {} while(0)`.
4. **`dynarec_native_pass.c`** — Multiple `#if defined(ARM64) || defined(LA64)` guards for `dyn->f = status_unk` vs struct-based `dyn->f.pending/dfnone`. PPC64LE uses enum-based status like ARM64/LA64. Added `|| defined(PPC64LE)` to ALL FIVE instances.
5. **`dynarec_native_pass.c`** — `fpu_purgecache` called with 7 args in `#else` branch but PPC64LE uses 6-arg version. Added `|| defined(PPC64LE)` to ALL THREE `#if defined(RV64) || defined(LA64)` guards.
6. **`signals.c` and `signal32.c`** — Missing `CONTEXT_REG`/`CONTEXT_PC` macros for PPC64LE. Added `#elif defined(PPC64LE)` blocks using `gp_regs[X]` and `gp_regs[PT_NIP]`.
7. **`GO_TRACE` macro in `dynarec_ppc64le_private.h`** — Had wrong argument counts for `GETIP`, `STORE_XEMU_CALL`, `CALL_`/`CALL`, `LOAD_XEMU_CALL`. Fixed to match LA64 pattern.
8. **`CALLRET_LOOP` not defined for pass 0 and pass 1** — Added `#ifndef CALLRET_LOOP` / `#define CALLRET_LOOP() NOP()` fallback in `dynarec_ppc64le_helper.h`.
9. **`XMM0`/`X870`/`EMM0` redefinition** — Were erroneously defined in both `functions.h` (wrong values: 0, 8, 8) and `functions.c` (correct values: 0, 16, 16). Removed from `functions.h` to match LA64 pattern.
10. **IFX/UFLAG_IF redefinition warnings** — pass0.h defines these with `set_flags`, helper.h redefines with `gen_flags`. Added `#undef` before redefinitions in helper.h.
11. **Missing `#endif` for `#ifdef DYNAREC` in signals.c** — Edit at lines 56-62 lost the second `#endif`. Added `#endif //DYNAREC` after `#endif //arch`.
12. **Missing linker symbols `dynarec64_F0`, `dynarec64_66`, `dynarec64_66F0`** — Created stub opcode files (`dynarec_ppc64le_f0.c`, `dynarec_ppc64le_66.c`, `dynarec_ppc64le_66f0.c`) and added to CMakeLists.txt.

---

## Complete Opcode Table Plan

### Dispatch Architecture

`dynarec_native_pass.c` calls 4 top-level dispatch functions. Each dispatches to sub-tables based on prefix bytes. The complete call tree:

```
dynarec_native_pass.c  (entry point)
│
├── dynarec64_00()          ← primary opcode table (no prefix)
│   ├── dynarec64_0F()      ← two-byte opcodes (0F escape)
│   ├── dynarec64_F20F()    ← REPNE + 0F (SSE2 scalar double)
│   ├── dynarec64_F30F()    ← REP + 0F (SSE scalar float)
│   ├── dynarec64_AVX()     ← VEX-prefixed (AVX dispatch hub)
│   │   ├── dynarec64_AVX_0F()
│   │   ├── dynarec64_AVX_0F38()
│   │   ├── dynarec64_AVX_66_0F()
│   │   ├── dynarec64_AVX_66_0F38()
│   │   ├── dynarec64_AVX_66_0F3A()
│   │   ├── dynarec64_AVX_F2_0F()
│   │   ├── dynarec64_AVX_F2_0F38()
│   │   ├── dynarec64_AVX_F2_0F3A()
│   │   ├── dynarec64_AVX_F3_0F()
│   │   └── dynarec64_AVX_F3_0F38()
│   ├── dynarec64_D8()      ← x87 FPU
│   ├── dynarec64_D9()
│   ├── dynarec64_DA()
│   ├── dynarec64_DB()
│   ├── dynarec64_DC()
│   ├── dynarec64_DD()
│   ├── dynarec64_DE()
│   └── dynarec64_DF()
│
├── dynarec64_66()          ← operand-size override prefix
│   ├── dynarec64_660F()    ← 66 + 0F (SSE2 packed double/misc)
│   ├── dynarec64_66F20F()  ← 66 + F2 + 0F
│   ├── dynarec64_66F30F()  ← 66 + F3 + 0F
│   └──(fallback to dynarec64_00 when rex.w set)
│
├── dynarec64_F0()          ← LOCK prefix (atomic operations)
│
└── dynarec64_66F0()        ← 66 + LOCK (16-bit atomic operations)
```

### File Status Matrix

29 opcode table files needed. Status: EXISTS = file created, STUB = has only `DEFAULT`, IMPL = has translated opcodes.

| # | File | Function | Category | PPC64LE Status | LA64 Translated |
|---|------|----------|----------|---------------|-----------------|
| 1 | `_00.c` | `dynarec64_00` | Primary opcodes | EXISTS (STUB) | 202 |
| 2 | `_0f.c` | `dynarec64_0F` | Two-byte opcodes | **MISSING** | 175 |
| 3 | `_66.c` | `dynarec64_66` | Operand-size prefix | EXISTS (STUB) | 133 |
| 4 | `_660f.c` | `dynarec64_660F` | 66+0F (SSE2 packed) | **MISSING** | 203 |
| 5 | `_66f0.c` | `dynarec64_66F0` | 66+LOCK | EXISTS (STUB) | 6 |
| 6 | `_66f20f.c` | `dynarec64_66F20F` | 66+F2+0F | **MISSING** | 0 |
| 7 | `_66f30f.c` | `dynarec64_66F30F` | 66+F3+0F | **MISSING** | 2 |
| 8 | `_f0.c` | `dynarec64_F0` | LOCK prefix | EXISTS (STUB) | 13 |
| 9 | `_f20f.c` | `dynarec64_F20F` | F2+0F (SSE2 scalar) | **MISSING** | 23 |
| 10 | `_f30f.c` | `dynarec64_F30F` | F3+0F (SSE scalar) | **MISSING** | 29 |
| 11 | `_d8.c` | `dynarec64_D8` | x87 FPU | **MISSING** | 7 |
| 12 | `_d9.c` | `dynarec64_D9` | x87 FPU | **MISSING** | 30 |
| 13 | `_da.c` | `dynarec64_DA` | x87 FPU | **MISSING** | 4 |
| 14 | `_db.c` | `dynarec64_DB` | x87 FPU | **MISSING** | 7 |
| 15 | `_dc.c` | `dynarec64_DC` | x87 FPU | **MISSING** | 7 |
| 16 | `_dd.c` | `dynarec64_DD` | x87 FPU | **MISSING** | 4 |
| 17 | `_de.c` | `dynarec64_DE` | x87 FPU | **MISSING** | 7 |
| 18 | `_df.c` | `dynarec64_DF` | x87 FPU | **MISSING** | 9 |
| 19 | `_avx.c` | `dynarec64_AVX` | AVX dispatch hub | **MISSING** | 0 (pure dispatcher) |
| 20 | `_avx_0f.c` | `dynarec64_AVX_0F` | VEX.NP 0F | **MISSING** | 43 |
| 21 | `_avx_0f38.c` | `dynarec64_AVX_0F38` | VEX.NP 0F38 | **MISSING** | 2 |
| 22 | `_avx_66_0f.c` | `dynarec64_AVX_66_0F` | VEX.66 0F | **MISSING** | 110 |
| 23 | `_avx_66_0f38.c` | `dynarec64_AVX_66_0F38` | VEX.66 0F38 | **MISSING** | 105 |
| 24 | `_avx_66_0f3a.c` | `dynarec64_AVX_66_0F3A` | VEX.66 0F3A | **MISSING** | 34 |
| 25 | `_avx_f2_0f.c` | `dynarec64_AVX_F2_0F` | VEX.F2 0F | **MISSING** | 36 |
| 26 | `_avx_f2_0f38.c` | `dynarec64_AVX_F2_0F38` | VEX.F2 0F38 | **MISSING** | 2 |
| 27 | `_avx_f2_0f3a.c` | `dynarec64_AVX_F2_0F3A` | VEX.F2 0F3A | **MISSING** | 0 |
| 28 | `_avx_f3_0f.c` | `dynarec64_AVX_F3_0F` | VEX.F3 0F | **MISSING** | 39 |
| 29 | `_avx_f3_0f38.c` | `dynarec64_AVX_F3_0F38` | VEX.F3 0F38 | **MISSING** | 1 |

**Total: 29 table files, 4 exist as stubs, 25 still needed. ~1233 opcodes to translate (LA64 reference count).**

### Emit Helper Files

4 emit helper files needed. These contain architecture-specific implementations of x86 flag-setting arithmetic/logic/shift/test operations. Each function generates native PPC64LE code to perform the operation AND compute the resulting x86 flags.

| File | Category | LA64 Function Count |
|------|----------|-------------------|
| `_emit_tests.c` | CMP, TEST | 11 functions |
| `_emit_math.c` | ADD, SUB, ADC, SBB, INC, DEC, NEG | 27 functions |
| `_emit_logic.c` | AND, OR, XOR | 15 functions |
| `_emit_shift.c` | SHL, SHR, SAR, ROL, ROR, RCL, RCR, SHLD, SHRD | 50 functions |

**Total: ~104 emit functions (matching LA64's complete set). All declared in `dynarec_ppc64le_helper.h` already.**

Complete emit function list (from LA64 reference, all needed for PPC64LE):

**Tests (11):** `emit_cmp8`, `emit_cmp8_0`, `emit_cmp16`, `emit_cmp16_0`, `emit_cmp32`, `emit_cmp32_0`, `emit_test8`, `emit_test8c`, `emit_test16`, `emit_test32`, `emit_test32c`

**Math (27):** `emit_add8`, `emit_add8c`, `emit_add16`, `emit_add32`, `emit_add32c`, `emit_sub8`, `emit_sub8c`, `emit_sub16`, `emit_sub32`, `emit_sub32c`, `emit_adc8`, `emit_adc8c`, `emit_adc16`, `emit_adc32`, `emit_sbb8`, `emit_sbb8c`, `emit_sbb16`, `emit_sbb32`, `emit_inc8`, `emit_inc16`, `emit_inc32`, `emit_dec8`, `emit_dec16`, `emit_dec32`, `emit_neg8`, `emit_neg16`, `emit_neg32`

**Logic (15):** `emit_and8`, `emit_and8c`, `emit_and16`, `emit_and32`, `emit_and32c`, `emit_or8`, `emit_or8c`, `emit_or16`, `emit_or32`, `emit_or32c`, `emit_xor8`, `emit_xor8c`, `emit_xor16`, `emit_xor32`, `emit_xor32c`

**Shift (50):** `emit_shl8`, `emit_shl8c`, `emit_shl16`, `emit_shl16c`, `emit_shl32`, `emit_shl32c`, `emit_shr8`, `emit_shr8c`, `emit_shr16`, `emit_shr16c`, `emit_shr32`, `emit_shr32c`, `emit_sar8`, `emit_sar8c`, `emit_sar16`, `emit_sar16c`, `emit_sar32`, `emit_sar32c`, `emit_rol8`, `emit_rol8c`, `emit_rol16`, `emit_rol16c`, `emit_rol32`, `emit_rol32c`, `emit_ror8`, `emit_ror8c`, `emit_ror16`, `emit_ror16c`, `emit_ror32`, `emit_ror32c`, `emit_rcl8`, `emit_rcl8c`, `emit_rcl16`, `emit_rcl16c`, `emit_rcl32`, `emit_rcl32c`, `emit_rcr8`, `emit_rcr8c`, `emit_rcr16`, `emit_rcr16c`, `emit_rcr32`, `emit_rcr32c`, `emit_shld16`, `emit_shld16c`, `emit_shld32`, `emit_shld32c`, `emit_shrd16`, `emit_shrd16c`, `emit_shrd32`, `emit_shrd32c`

**Other (1):** `emit_pf` (already in `helper.c`)

### Implementation Priority Order

Recommended order based on frequency in real x86-64 code and dependency chains:

#### Phase 1: Core Integer (gets basic programs running)
1. **`_emit_tests.c`** — `emit_cmp*`, `emit_test*` (needed by almost every conditional)
2. **`_emit_math.c`** — `emit_add*`, `emit_sub*`, `emit_inc*`, `emit_dec*`, `emit_neg*`
3. **`_emit_logic.c`** — `emit_and*`, `emit_or*`, `emit_xor*`
4. **`_emit_shift.c`** — `emit_shl*`, `emit_shr*`, `emit_sar*` (the rest can wait)
5. **`_00.c`** — Primary opcode translations: MOV, LEA, PUSH/POP, CALL/RET, JMP, Jcc, CMP, TEST, ADD/SUB/AND/OR/XOR, XCHG, MOVZX/MOVSX, IMUL, DIV/IDIV, CDQ/CQO, NOP, INT3, CPUID
6. **`_0f.c`** — Two-byte opcodes: CMOVcc, SETcc, MOVZX/MOVSX, BSWAP, BT/BTS/BTR/BTC, BSF/BSR, IMUL r,r/m, SYSCALL, CPUID, UD2

#### Phase 2: SSE/SSE2 (needed for most modern programs)
7. **`_660f.c`** — SSE2 packed: MOVDQA, MOVDQU, PADDB/W/D/Q, PSUBB/W/D/Q, PAND/POR/PXOR, PCMPEQB/W/D, PACKSS/PACKUS, PUNPCK*, PSLL*/PSRL*/PSRA*, PMULL*, MOVD/MOVQ
8. **`_f20f.c`** — SSE2 scalar double: MOVSD, ADDSD, SUBSD, MULSD, DIVSD, SQRTSD, UCOMISD, CVTSD2SI, CVTSI2SD
9. **`_f30f.c`** — SSE scalar float: MOVSS, ADDSS, SUBSS, MULSS, DIVSS, SQRTSS, UCOMISS, CVTSS2SI, CVTSI2SS, REP MOVSB/STOSB/SCASB

#### Phase 3: x87 FPU (needed for legacy floating-point code)
10. **`_d8.c`** through **`_df.c`** — x87 operations: FLD, FST, FSTP, FADD, FSUB, FMUL, FDIV, FCOM, FCOMP, FILD, FISTP, FXCH, FCHS, FABS, FLDZ, FLD1, FLDPI, etc.

#### Phase 4: LOCK prefix (needed for multithreaded programs)
11. **`_f0.c`** — Atomic operations: LOCK ADD, LOCK SUB, LOCK AND, LOCK OR, LOCK XOR, LOCK XADD, LOCK CMPXCHG, LOCK INC, LOCK DEC, LOCK BTS/BTR/BTC
12. **`_66f0.c`** — 16-bit atomic operations

#### Phase 5: 66h prefix and misc
13. **`_66.c`** — 16-bit integer operations
14. **`_66f20f.c`**, **`_66f30f.c`** — rare 66+F2/F3+0F opcodes

#### Phase 6: AVX (needed for optimized programs)
15. **`_avx.c`** — AVX dispatch hub (pure dispatcher)
16. **`_avx_66_0f.c`** — VEX.66 0F (largest AVX table: 110 opcodes)
17. **`_avx_66_0f38.c`** — VEX.66 0F38 (105 opcodes)
18. **`_avx_0f.c`** — VEX.NP 0F (43 opcodes)
19. **`_avx_f2_0f.c`** — VEX.F2 0F (36 opcodes)
20. **`_avx_f3_0f.c`** — VEX.F3 0F (39 opcodes)
21. **`_avx_66_0f3a.c`** — VEX.66 0F3A (34 opcodes)
22. **`_avx_0f38.c`**, **`_avx_f2_0f38.c`**, **`_avx_f2_0f3a.c`**, **`_avx_f3_0f38.c`** — remaining AVX tables (5 opcodes total)

### Key Opcodes in `_00.c` (Primary Table) — Detailed Breakdown

The primary opcode table has ~202 translated opcodes in LA64. The most important ones grouped by category:

**Data Movement:**
- `0x88-0x8B` — MOV r/m8↔r8, MOV r/m↔r (most common instruction)
- `0x8D` — LEA (address calculation)
- `0xA0-0xA3` — MOV AL/AX/EAX/RAX, moffs
- `0xB0-0xBF` — MOV imm8/imm64 to register
- `0xC6-0xC7` — MOV r/m, imm
- `0x50-0x5F` — PUSH/POP register
- `0x86-0x87` — XCHG r/m, r
- `0x90` — NOP (XCHG EAX, EAX)
- `0x63` — MOVSXD

**Arithmetic:**
- `0x00-0x05` — ADD r/m, r/imm
- `0x08-0x0D` — OR r/m, r/imm
- `0x10-0x15` — ADC r/m, r/imm
- `0x18-0x1D` — SBB r/m, r/imm
- `0x20-0x25` — AND r/m, r/imm
- `0x28-0x2D` — SUB r/m, r/imm
- `0x30-0x35` — XOR r/m, r/imm
- `0x38-0x3D` — CMP r/m, r/imm
- `0x80-0x83` — Grp1 (ADD/OR/ADC/SBB/AND/SUB/XOR/CMP) r/m, imm
- `0xF6-0xF7` — Grp3 (TEST/NOT/NEG/MUL/IMUL/DIV/IDIV)
- `0xFE-0xFF` — Grp4/5 (INC/DEC/CALL/JMP/PUSH)
- `0x69, 0x6B` — IMUL r, r/m, imm

**Control Flow:**
- `0x70-0x7F` — Jcc short (conditional jumps)
- `0xE8` — CALL rel32
- `0xE9` — JMP rel32
- `0xEB` — JMP rel8
- `0xC3` — RET
- `0xC2` — RET imm16
- `0xE0-0xE2` — LOOP/LOOPE/LOOPNE

**Shifts/Rotates:**
- `0xC0-0xC1` — Grp2 (ROL/ROR/RCL/RCR/SHL/SHR/SAR) r/m, imm8
- `0xD0-0xD3` — Grp2 r/m, 1 or CL

**String Operations:**
- `0xA4-0xA7` — MOVS, CMPS
- `0xAA-0xAF` — STOS, LODS, SCAS

**Miscellaneous:**
- `0x84-0x85` — TEST r/m, r
- `0x98` — CBW/CWDE/CDQE
- `0x99` — CWD/CDQ/CQO
- `0x9C-0x9D` — PUSHF/POPF
- `0xCC` — INT3
- `0xF8-0xFD` — CLC/STC/CLI/STI/CLD/STD

---

## Known Gaps / Future Work

### Architecture Gaps
- `emit_*` functions (104 total) — declared in helper.h, not yet implemented. Need 4 new `.c` files.
- `x87_refresh` — declared in helper.h but not implemented (same as LA64). Needed when D9 opcodes are added.
- `avx_forget_reg` macro in helper.h: `#define avx_forget_reg STEPNAME(sse_forget_reg)` — intentional, matches LA64
- Printer (`ppc64le_printer.c`) is a stub — needs real disassembly for debugging
- Arch detection (`dynarec_ppc64le_arch.c`) is a stub — needs real HWCAP detection

---

## Key File Locations

### Created files (in `src/dynarec/ppc64le/`)
All 27 files listed in "Accomplished" section above.

### Modified files
All 11 files listed in "Dispatch Header Modifications" and "Build System / Host Detection" sections above.

### Primary reference files
| File | Purpose |
|------|---------|
| `src/dynarec/la64/dynarec_la64_helper.c` | PRIMARY reference for helper.c |
| `src/dynarec/la64/dynarec_la64_helper.h` | Reference for helper.h |
| `src/dynarec/la64/la64_emitter.h` | Instruction pattern cross-reference |
| `src/dynarec/arm64/dynarec_arm64_helper.c` | Secondary reference |
| `src/dynarec/la64/dynarec_la64_functions.c` | Reference for functions.c |
| `src/dynarec/la64/dynarec_la64_private.h` | Reference for private.h |
