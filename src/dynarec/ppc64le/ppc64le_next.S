// PPC64LE next linker for dynarec
// Called when a dynarec block needs to jump to the next block.
// If block cache is enabled (BLOCK_CACHE_BITS defined), first checks a
// per-thread block dispatch cache for fast hits (~12 insns).
// On miss (or if cache disabled), saves volatile state, calls LinkNext
// to resolve the target, then branches to the resolved address.
//
// Called with:
//   Emu (r31) = pointer to x64emu_t
//   RIP (r9)  = target x86 IP
//   LR        = "from" address (for linking)

#define ASM_MAPPING
#include "ppc64le_mapping.h"

#ifdef BLOCK_CACHE_BITS
// x64emu_t struct offsets for block dispatch cache (PPC64LE + BOX32)
// EMU_BLOCK_CACHE is always at 0x1E80 regardless of cache size.
// The gen/ptr fields follow the cache array, so their offsets depend on size.
// Verified at compile time by _Static_assert in x64emu.c.
#define BLOCK_CACHE_SIZE            (1 << BLOCK_CACHE_BITS)
#define EMU_BLOCK_CACHE             0x1E80
#define EMU_BLOCK_CACHE_GEN         (EMU_BLOCK_CACHE + (BLOCK_CACHE_SIZE * 16))
#define EMU_BLOCK_CACHE_GEN_PTR     (EMU_BLOCK_CACHE_GEN + 8)
#endif

.text
.align 4

.extern LinkNext

.global ppc64le_next
.type ppc64le_next, @function

    // NULL pointer before ppc64le_next, for getDB
    .8byte  0
ppc64le_next:
#ifdef BLOCK_CACHE_BITS
    // ===== Fast-path: per-thread block dispatch cache =====
    // Check generation: if global generation != local, cache is stale -> miss
    // Uses r11/r12 as scratch (not mapped by dynarec, not yet saved)

#if EMU_BLOCK_CACHE_GEN_PTR > 32764
    // Large cache (>1024 entries): offset exceeds DS-form displacement range.
    // Use addis+addi to materialize the address of block_cache_gen_ptr,
    // then load through it. HA/LO split handles signed 16-bit low part.
#define _GEN_PTR_HA  ((EMU_BLOCK_CACHE_GEN_PTR + 0x8000) >> 16)
#define _GEN_PTR_LO  (EMU_BLOCK_CACHE_GEN_PTR - (_GEN_PTR_HA << 16))
#define _GEN_HA      ((EMU_BLOCK_CACHE_GEN + 0x8000) >> 16)
#define _GEN_LO      (EMU_BLOCK_CACHE_GEN - (_GEN_HA << 16))
    addis   11, Emu, _GEN_PTR_HA
    ld      11, _GEN_PTR_LO(11)                  // r11 = emu->block_cache_gen_ptr
    addis   12, Emu, _GEN_HA
    ld      12, _GEN_LO(12)                      // r12 = emu->block_cache_gen (local)
#else
    ld      11, EMU_BLOCK_CACHE_GEN_PTR(Emu)     // r11 = emu->block_cache_gen_ptr
    ld      12, EMU_BLOCK_CACHE_GEN(Emu)         // r12 = emu->block_cache_gen (local)
#endif
    ld      11, 0(11)                            // r11 = *block_cache_gen_ptr (global)
    cmpd    12, 11                               // local == global?
    bne     .Lcache_miss                         // stale generation -> slow path

    // Compute cache index: (RIP >> 1) & (BLOCK_CACHE_SIZE-1), then *16 for 16-byte entries
    // rldicl r11, r9, 63, (64-BLOCK_CACHE_BITS) =>  r11 = (r9 >> 1) & (BLOCK_CACHE_SIZE-1)
    rldicl  11, RIP, 63, (64 - BLOCK_CACHE_BITS)
    sldi    11, 11, 4                            // r11 = index * 16 (entry offset)
    addi    12, Emu, EMU_BLOCK_CACHE             // r12 = &emu->block_cache[0]
    add     11, 11, 12                           // r11 = &emu->block_cache[index]
    ld      12, 0(11)                            // r12 = cached x86_addr
    cmpd    12, RIP                              // match target?
    bne     .Lcache_miss
    ld      12, 8(11)                            // r12 = cached native_addr
    mtctr   12
    bctr                                         // direct jump - cache hit!

.Lcache_miss:
#endif
    // ===== Slow path: full LinkNext call =====
    // Save volatile registers that the dynarec uses
    // We need to preserve: r3-r10 (args/scratch used by dynarec)
    // and RIP (r9), plus the LR (return "from" address)
    mflr    0

    // Allocate save area on stack
    stdu    1, -128(1)

    // Save registers
    std     0,   16(1)        // LR (the "from" address)
    std     3,   32(1)        // x1 / A0
    std     4,   40(1)        // x2 / A1
    std     5,   48(1)        // x3 / A2
    std     6,   56(1)        // x4 / A3
    std     7,   64(1)        // x5 / A4
    std     8,   72(1)        // x6 / A5
    std     9,   80(1)        // xRIP - also save to allow change in LinkNext
    std     10,  88(1)        // x7 / A7

    // Call LinkNext(emu, ip, from, &rip_on_stack)
    mr      3, Emu             // arg0 = emu (r31)
    mr      4, RIP             // arg1 = xRIP (r9)
    ld      5, 16(1)           // arg2 = "from" (saved LR)
    addi    6, 1, 80           // arg3 = address of saved RIP on stack

    // Restore TOC for calling C code
    // We need to load the TOC for LinkNext - use the saved TOC
    // Note: In ELFv2, r12 must point to the function entry for local calls
    // For external calls via PLT, the linker handles it
    bl      LinkNext
    nop                          // TOC restore slot (linker fills if needed)

    // Preserve return value (jump target) in r12
    mr      12, 3

    // Restore registers
    ld      3,   32(1)
    ld      4,   40(1)
    ld      5,   48(1)
    ld      6,   56(1)
    ld      7,   64(1)
    ld      8,   72(1)
    ld      9,   80(1)        // RIP may have been modified by LinkNext
    ld      10,  88(1)

    // Deallocate save area
    addi    1, 1, 128

    // Jump to resolved target
    mtctr   12
    bctr
.size ppc64le_next, .-ppc64le_next
