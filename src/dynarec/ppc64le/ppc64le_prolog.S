// PPC64LE prolog for dynarec
// Save callee-saved registers, load x86 state from emu struct,
// then jump to the dynarec block.
// Called with:
//   r3 = pointer to x64emu_t (emu)
//   r4 = address to jump to (dynarec block entry)
//
// ELFv2 ABI: r1=SP, r2=TOC (preserved), r13=TLS (preserved)
// Callee-saved GPRs: r14-r31
// Callee-saved VMX: vr20-vr31

#define ASM_MAPPING
#include "ppc64le_mapping.h"

.text
.align 4

.global ppc64le_prolog
.type ppc64le_prolog, @function
ppc64le_prolog:
    // Save LR
    mflr    0
    std     0, 16(1)

    // Stack frame: 256 bytes (16-byte aligned)
    // Layout (from new SP):
    //   0: back chain
    //   8: CR save
    //  16: LR save (for functions called from dynarec)
    //  24: TOC save
    //  32: r14-r31 save area (144 bytes = 18*8)
    // 176: jump_next slot (16 bytes = 2*8)
    // 192-255: padding

    stdu    1, -256(1)          // allocate stack frame

    // Save callee-saved GPRs
    std     14, 32(1)
    std     15, 40(1)
    std     16, 48(1)
    std     17, 56(1)
    std     18, 64(1)
    std     19, 72(1)
    std     20, 80(1)
    std     21, 88(1)
    std     22, 96(1)
    std     23, 104(1)
    std     24, 112(1)
    std     25, 120(1)
    std     26, 128(1)
    std     27, 136(1)
    std     28, 144(1)
    std     29, 152(1)
    std     30, 160(1)
    std     31, 168(1)

    // Save CR
    mfcr    0
    stw     0, 8(1)

    // Save TOC (r2)
    std     2, 24(1)

    // Setup: load x86 register state from emu struct
    // emu struct layout: regs[0..15] = RAX,RCX,RDX,RBX,RSP,RBP,RSI,RDI,R8..R15
    // Each is 8 bytes, starting at offset 0 in x64emu_t
    // r3 = emu pointer

    // Load x86 registers from emu into mapped PPC64LE registers
    ld      RAX, (8 *  0)(3)   // RAX = r14
    ld      RCX, (8 *  1)(3)   // RCX = r15
    ld      RDX, (8 *  2)(3)   // RDX = r16
    ld      RBX, (8 *  3)(3)   // RBX = r17
    ld      RSP, (8 *  4)(3)   // RSP = r18
    ld      RBP, (8 *  5)(3)   // RBP = r19
    ld      RSI, (8 *  6)(3)   // RSI = r20
    ld      RDI, (8 *  7)(3)   // RDI = r21
    ld      R8,  (8 *  8)(3)   // R8  = r22
    ld      R9,  (8 *  9)(3)   // R9  = r23
    ld      R10, (8 * 10)(3)   // R10 = r24
    ld      R11, (8 * 11)(3)   // R11 = r25
    ld      R12, (8 * 12)(3)   // R12 = r26
    ld      R13, (8 * 13)(3)   // R13 = r27
    ld      R14, (8 * 14)(3)   // R14 = r28
    ld      R15, (8 * 15)(3)   // R15 = r29

    // Load flags (at offset 16*8 = 128)
    ld      Flags, (8 * 16)(3) // Flags = r30

    // Load RIP into r9 (xRIP)
    ld      RIP, (8 * 17)(3)   // RIP = r9

    // Store emu pointer
    mr      Emu, 3              // Emu = r31

    // Save host SP for restoration in epilog (SavedSP = r12)
    addi    SavedSP, 1, 256     // SavedSP points to caller's SP

    // Push a jump_next slot on stack (2 doublewords of zeros)
    li      0, 0
    std     0, 176(1)          // jump_next[0] = 0
    std     0, 184(1)          // jump_next[1] = 0

    // Jump to dynarec block (address in r4)
    mtctr   4
    bctr                         // branch to CTR (no link)
.size ppc64le_prolog, .-ppc64le_prolog
