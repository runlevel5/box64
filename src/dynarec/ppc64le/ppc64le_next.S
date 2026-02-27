// PPC64LE next linker for dynarec
// Called when a dynarec block needs to jump to the next block.
// First checks a per-thread block dispatch cache for fast hits (~12 insns).
// On miss, saves volatile state, calls LinkNext to resolve the target,
// then branches to the resolved address.
//
// Called with:
//   Emu (r31) = pointer to x64emu_t
//   RIP (r9)  = target x86 IP
//   LR        = "from" address (for linking)

#define ASM_MAPPING
#include "ppc64le_mapping.h"

#ifdef BLOCK_CACHE_BITS
// x64emu_t struct offsets for block dispatch cache (PPC64LE)
// The base offset differs with BOX32 due to extra fields in x64emu_t.
// The gen/ptr fields follow the cache array, so their offsets depend on size.
// Verified at compile time by _Static_assert in x64emu.c.
#define BLOCK_CACHE_SIZE            (1 << BLOCK_CACHE_BITS)
#ifdef BOX32
#define EMU_BLOCK_CACHE             0x1E80
#else
#define EMU_BLOCK_CACHE             0x0F38
#endif
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
    // ===== Fast-path: per-thread block dispatch cache =====
    // Check generation: if global generation != local, cache is stale -> miss
    // Uses r11/r12 as scratch (not mapped by dynarec, not yet saved)
    ld      11, EMU_BLOCK_CACHE_GEN_PTR(Emu)   // r11 = emu->block_cache_gen_ptr
    ld      12, EMU_BLOCK_CACHE_GEN(Emu)        // r12 = emu->block_cache_gen (local)
    ld      11, 0(11)                            // r11 = *block_cache_gen_ptr (global)
    cmpd    12, 11                               // local == global?
    bne     .Lcache_miss                         // stale generation -> slow path

    // Compute cache index: (RIP >> 1) & 0xFF, then *16 for 16-byte entries
    // rldicl r11, r9, 63, 56  =>  r11 = rotl64(r9, 63) & mask(56..63)
    //   = (r9 >> 1) & 0xFF
    rldicl  11, RIP, 63, 56
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
    // ===== Slow path: full LinkNext call =====
    // Save volatile registers that the dynarec uses
    // We need to preserve: r3-r10 (args/scratch used by dynarec)
    // and RIP (r9), plus the LR (return "from" address)
    mflr    %r0

    // Allocate save area on stack
    stdu    %r1, -128(%r1)

    // Save registers
    std     %r0,   16(%r1)        // LR (the "from" address)
    std     %r3,   32(%r1)        // x1 / A0
    std     %r4,   40(%r1)        // x2 / A1
    std     %r5,   48(%r1)        // x3 / A2
    std     %r6,   56(%r1)        // x4 / A3
    std     %r7,   64(%r1)        // x5 / A4
    std     %r8,   72(%r1)        // x6 / A5
    std     %r9,   80(%r1)        // xRIP - also save to allow change in LinkNext
    std     %r10,  88(%r1)        // x7 / A7

    // Call LinkNext(emu, ip, from, &rip_on_stack)
    mr      %r3, Emu               // arg0 = emu (r31)
    mr      %r4, RIP               // arg1 = xRIP (r9)
    ld      %r5, 16(%r1)           // arg2 = "from" (saved LR)
    addi    %r6, %r1, 80           // arg3 = address of saved RIP on stack

    // Restore TOC for calling C code
    // We need to load the TOC for LinkNext - use the saved TOC
    // Note: In ELFv2, r12 must point to the function entry for local calls
    // For external calls via PLT, the linker handles it
    bl      LinkNext
    nop                            // TOC restore slot (linker fills if needed)

    // Preserve return value (jump target) in r12
    mr      %r12, %r3

    // Restore registers
    ld      %r3,   32(%r1)
    ld      %r4,   40(%r1)
    ld      %r5,   48(%r1)
    ld      %r6,   56(%r1)
    ld      %r7,   64(%r1)
    ld      %r8,   72(%r1)
    ld      %r9,   80(%r1)        // RIP may have been modified by LinkNext
    ld      %r10,  88(%r1)

    // Deallocate save area
    addi    %r1, %r1, 128

    // Jump to resolved target
    mtctr   %r12
    bctr
.size ppc64le_next, .-ppc64le_next
