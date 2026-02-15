// PPC64LE epilog for dynarec
// Store x86 register state back to emu struct,
// restore callee-saved registers, return to caller.
// Called with:
//   Emu (r31) = pointer to x64emu_t
//
// Must store back all x86 registers including RIP.

#define ASM_MAPPING
#include "ppc64le_mapping.h"

.text
.align 4

.global ppc64le_epilog
.type ppc64le_epilog, @function
ppc64le_epilog:
    // Store x86 registers back to emu struct
    // Emu (r31) points to x64emu_t
    std     RAX, (8 *  0)(Emu)   // r14
    std     RCX, (8 *  1)(Emu)   // r15
    std     RDX, (8 *  2)(Emu)   // r16
    std     RBX, (8 *  3)(Emu)   // r17
    std     RSP, (8 *  4)(Emu)   // r18
    std     RBP, (8 *  5)(Emu)   // r19
    std     RSI, (8 *  6)(Emu)   // r20
    std     RDI, (8 *  7)(Emu)   // r21
    std     R8,  (8 *  8)(Emu)   // r22
    std     R9,  (8 *  9)(Emu)   // r23
    std     R10, (8 * 10)(Emu)   // r24
    std     R11, (8 * 11)(Emu)   // r25
    std     R12, (8 * 12)(Emu)   // r26
    std     R13, (8 * 13)(Emu)   // r27
    std     R14, (8 * 14)(Emu)   // r28
    std     R15, (8 * 15)(Emu)   // r29

    // Store flags
    std     Flags, (8 * 16)(Emu) // r30

    // Store RIP (r9 = xRIP, must contain current IP)
    std     RIP, (8 * 17)(Emu)   // r9

    // Restore stack pointer from SavedSP
    // SavedSP (r12) = original caller SP
    addi    1, SavedSP, -256     // point back to our frame

    // Restore TOC
    ld      2, 24(1)

    // Restore CR
    lwz     0, 8(1)
    mtcrf   0xFF, 0

    // Restore callee-saved GPRs
    ld      14, 32(1)
    ld      15, 40(1)
    ld      16, 48(1)
    ld      17, 56(1)
    ld      18, 64(1)
    ld      19, 72(1)
    ld      20, 80(1)
    ld      21, 88(1)
    ld      22, 96(1)
    ld      23, 104(1)
    ld      24, 112(1)
    ld      25, 120(1)
    ld      26, 128(1)
    ld      27, 136(1)
    ld      28, 144(1)
    ld      29, 152(1)
    ld      30, 160(1)
    ld      31, 168(1)

    // Deallocate frame
    addi    1, 1, 256

    // Restore LR and return
    ld      0, 16(1)
    mtlr    0
    blr
.size ppc64le_epilog, .-ppc64le_epilog
