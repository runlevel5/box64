#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "x64trace.h"
#include "dynablock.h"
#include "dynarec_native.h"
#include "../dynablock_private.h"
#include "custommem.h"
#include "x64test.h"
#include "pe_tools.h"

#include "../dynarec_arch.h"

#if STEP == 0
    #define EMIT(A)         dyn->native_size+=4
    #define SETMARK(A)      jmp_df[A] = 1
    #define MESSAGE(A, ...) do {} while (0)
    #define TABLE64C(A, V)  do { EMIT(0); EMIT(0); EMIT(0); EMIT(0); } while(0)
#elif STEP == 1
    #define EMIT(A)         do {} while (0)
    #define SETMARK(A)      jmp_df[A] = 0
    #define MESSAGE(A, ...) do {} while (0)
    #define TABLE64C(A, V)  do {} while (0)
#elif STEP == 2
    #define EMIT(A)         dyn->native_size+=4
    #define SETMARK(A)      jmp_df[A] = dyn->native_size
    #define MESSAGE(A, ...) do {} while (0)
    #define TABLE64C(A, V)  do { Table64(dyn, getConst(V), 2); EMIT(0); EMIT(0); EMIT(0); EMIT(0); } while(0)
#elif STEP == 3
    #define MESSAGE(A, ...)                                                   \
        do {                                                                  \
            if (dyn->need_dump) dynarec_log_prefix(0, LOG_NONE, __VA_ARGS__); \
        } while (0)
    #define EMIT(A)                                         \
        do{                                                 \
            if(dyn->need_dump) print_opcode(dyn, ninst, (uint32_t)(A)); \
            *(uint32_t*)(dyn->block) = (uint32_t)(A);       \
            dyn->block += 4; dyn->native_size += 4;         \
            dyn->insts[ninst].size2 += 4;                   \
        }while(0)
    #define SETMARK(A)      MESSAGE(LOG_DUMP, "Mark(%d)=%p\n", A, dyn->block)
    #define PPC64_LO16(x) ((int16_t)((x) & 0xFFFF))
    #define PPC64_HI16(x) ((int16_t)((((x) >> 16) + (((x) & 0x8000) ? 1 : 0)) & 0xFFFF))
    #define TABLE64C(A, V) do {                                                                 \
                int val64offset = Table64(dyn, getConst(V), 3);                                 \
                int _delta = val64offset - 4;                                                   \
                MESSAGE(LOG_DUMP, "  Table64C: 0x%lx (offset %d)\n", (V), val64offset);         \
                BCL(20, 31, 4);                                                                 \
                MFLR(A);                                                                        \
                ADDIS(A, A, PPC64_HI16(_delta));                                                \
                LD(A, PPC64_LO16(_delta), A);                                                   \
            } while(0)
#else
#error Meh!
#endif
#define STEP_PASS
#include "../dynarec_helper.h"

/*
    PPC64LE native UpdateFlags block.

    Called from dynarec when deferred flags need to be resolved.
    Entry: xEmu (r31) = x64emu_t*, xFlags (r30) = current eflags.
    LR = return address (set by caller's BCTRL).
    Reads emu->df, dispatches to native handler or falls back to C UpdateFlags().

    Scratch registers: x1(r3)..x6(r8), x7(r10).
    xFlags (r30) is modified by native handlers.
    xEmu (r31) is preserved.

    Batch 1: d_none, d_cmp8/16/32/64, d_tst8/16/32/64 are native.
    All other df types fall back to the C UpdateFlags() function.

    IMPORTANT: We do NOT clear emu->df in the prologue.
    - Native handlers clear df after computing flags.
    - Fallback handler calls C UpdateFlags() which reads and clears df itself.
*/

void updateflags_pass(dynarec_ppc64le_t* dyn, uint64_t jmp_df[])
{
    int ninst = 0;
    rex_t rex = {0};

    // === Prologue: save caller LR, load df, bounds-check, dispatch ===

    // Save caller's return address (LR set by caller's BCTRL).
    // BCL below will clobber LR, so we must save it first.
    // x7 (r10) is a scratch register not used by emit functions.
    MFLR(x7);

    // Load df from emu struct into x1
    LWZ(x1, offsetof(x64emu_t, df), xEmu);
    // Bounds check: if df >= d_unknown, just return (bogus df)
    CMPWI(x1, d_unknown);
    BLT(12);        // if df < d_unknown, skip MTLR+BLR and go to dispatch
    MTLR(x7);       // restore caller LR before returning
    BLR();          // df >= d_unknown: return without doing anything

    // === Branch table dispatch ===
    // x1 = df (0..d_unknown-1)
    // Compute: target = table_base + df * 4
    // table_base = address of first B instruction after BCTR
    // From MFLR result (= addr of MFLR instruction):
    //   MFLR(x2)  +4 = SLWI
    //   SLWI      +4 = ADD
    //   ADD        +4 = ADDI
    //   ADDI       +4 = MTCTR
    //   MTCTR      +4 = BCTR
    //   BCTR       +4 = table[0]
    // So table[0] = x2 + 24 bytes from x2 (6 instructions * 4 bytes)
    BCL(20, 31, 4);     // LR = addr(next instruction)
    MFLR(x2);           // x2 = addr(this MFLR instruction)
    SLWI(x1, x1, 2);   // x1 = df * 4
    ADD(x1, x2, x1);   // x1 = x2 + df*4
    ADDI(x1, x1, 24);  // x1 = x2 + df*4 + 24 = &table[df]
    MTCTR(x1);
    BCTR();

    // === Branch table: one B(offset) per df type ===
    for (int i = d_none; i < d_unknown; ++i)
        B(jmp_df[i] - dyn->native_size);

    // === d_none: flags are already resolved, just return ===
SETMARK(d_none);
    MTLR(x7);
    BLR();

    // === d_cmp8 ===
SETMARK(d_cmp8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    // Clear df before emit (emit won't clear it with gen_flags=X_ALL)
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_cmp8(dyn, ninst, x1, x2, x3, x4, x5, x6);
    MTLR(x7);
    BLR();

    // === d_cmp16 ===
SETMARK(d_cmp16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_cmp16(dyn, ninst, x1, x2, x3, x4, x5, x6);
    MTLR(x7);
    BLR();

    // === d_cmp32 (THE HOT PATH — 96.99% of all UpdateFlags calls) ===
SETMARK(d_cmp32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    MTLR(x7);
    BLR();

    // === d_cmp64 ===
SETMARK(d_cmp64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_cmp32(dyn, ninst, rex, x1, x2, x3, x4, x5, x6);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_tst8 ===
SETMARK(d_tst8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xff);
    emit_test8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_tst16 ===
SETMARK(d_tst16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffff);
    emit_test16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_tst32 ===
SETMARK(d_tst32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_tst64 ===
SETMARK(d_tst64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_test32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // Batch 2: Arithmetic handlers (d_add, d_sub, d_inc, d_dec, d_neg)
    // ====================================================================

    // === d_add8 / d_add8b (aliases — same implementation) ===
SETMARK(d_add8);
SETMARK(d_add8b);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_add8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_add16 / d_add16b ===
SETMARK(d_add16);
SETMARK(d_add16b);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_add16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_add32 / d_add32b ===
SETMARK(d_add32);
SETMARK(d_add32b);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_add64 ===
SETMARK(d_add64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_add32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_sub8 ===
SETMARK(d_sub8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sub8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sub16 ===
SETMARK(d_sub16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sub16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sub32 ===
SETMARK(d_sub32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sub64 ===
SETMARK(d_sub64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_sub32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_inc8 ===
SETMARK(d_inc8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_inc8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_inc16 ===
SETMARK(d_inc16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_inc16(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_inc32 ===
SETMARK(d_inc32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_inc64 ===
SETMARK(d_inc64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_inc32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_dec8 ===
SETMARK(d_dec8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_dec8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_dec16 ===
SETMARK(d_dec16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_dec16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_dec32 ===
SETMARK(d_dec32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_dec64 ===
SETMARK(d_dec64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_dec32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_neg8 ===
SETMARK(d_neg8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_neg8(dyn, ninst, x1, x2, x3);
    MTLR(x7);
    BLR();

    // === d_neg16 ===
SETMARK(d_neg16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    emit_neg16(dyn, ninst, x1, x2, x3);
    MTLR(x7);
    BLR();

    // === d_neg32 ===
SETMARK(d_neg32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_neg32(dyn, ninst, rex, x1, x2, x3);
    MTLR(x7);
    BLR();

    // === d_neg64 ===
SETMARK(d_neg64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LI(x2, 0);
    STW(x2, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_neg32(dyn, ninst, rex, x1, x2, x3);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // Batch 3: Logic handlers (d_and, d_or, d_xor)
    // All logic ops load only 'res' from emu. The second operand is a
    // constant mask: all-ones for AND, zero for OR and XOR.
    // ====================================================================

    // === d_and8 ===
SETMARK(d_and8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xff);
    emit_and8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_and16 ===
SETMARK(d_and16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffff);
    emit_and16(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_and32 ===
SETMARK(d_and32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV32w(x2, 0xffffffff);
    rex.w = 0;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_and64 ===
SETMARK(d_and64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    MOV64x(x2, 0xffffffffffffffffULL);
    rex.w = 1;
    emit_and32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_or8 ===
SETMARK(d_or8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_or8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_or16 ===
SETMARK(d_or16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_or16(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_or32 ===
SETMARK(d_or32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 0;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_or64 ===
SETMARK(d_or64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 1;
    emit_or32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_xor8 ===
SETMARK(d_xor8);
    LBZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_xor8(dyn, ninst, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_xor16 ===
SETMARK(d_xor16);
    LHZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    emit_xor16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_xor32 ===
SETMARK(d_xor32);
    LWZ(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 0;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_xor64 ===
SETMARK(d_xor64);
    LD(x1, offsetof(x64emu_t, res), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    LI(x2, 0);
    rex.w = 1;
    emit_xor32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // ====================================================================
    // Batch 4: Shift handlers (d_shl, d_shr, d_sar)
    // All shift ops load op1 and op2 (shift count).
    // SAR 8/16 loads op1 sign-extended.
    // ====================================================================

    // === d_shl8 ===
SETMARK(d_shl8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shl8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shl16 ===
SETMARK(d_shl16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shl16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shl32 ===
SETMARK(d_shl32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shl64 ===
SETMARK(d_shl64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_shl32(dyn, ninst, rex, x1, x2, x3, x4, x5);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_shr8 ===
SETMARK(d_shr8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shr8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shr16 ===
SETMARK(d_shr16);
    LHZ(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_shr16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_shr32 ===
SETMARK(d_shr32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_shr64 ===
SETMARK(d_shr64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_shr32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === d_sar8 (sign-extended op1) ===
SETMARK(d_sar8);
    LBZ(x1, offsetof(x64emu_t, op1), xEmu);
    EXTSB(x1, x1);
    LBZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sar8(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sar16 (sign-extended op1) ===
SETMARK(d_sar16);
    LHA(x1, offsetof(x64emu_t, op1), xEmu);
    LHZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    emit_sar16(dyn, ninst, x1, x2, x3, x4, x5);
    MTLR(x7);
    BLR();

    // === d_sar32 ===
SETMARK(d_sar32);
    LWZ(x1, offsetof(x64emu_t, op1), xEmu);
    LWZ(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 0;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    MTLR(x7);
    BLR();

    // === d_sar64 ===
SETMARK(d_sar64);
    LD(x1, offsetof(x64emu_t, op1), xEmu);
    LD(x2, offsetof(x64emu_t, op2), xEmu);
    LI(x3, 0);
    STW(x3, offsetof(x64emu_t, df), xEmu);
    rex.w = 1;
    emit_sar32(dyn, ninst, rex, x1, x2, x3, x4);
    rex.w = 0;
    MTLR(x7);
    BLR();

    // === Fallback handler: call C UpdateFlags() for all other df types ===
    // All unimplemented df types branch here.
    // emu->df is still set to the original df value (not cleared in prologue).
    // C UpdateFlags() reads emu->df, computes flags into emu->eflags, clears df.
    //
    // We need to:
    // 1. Save our return LR (from caller's BCTRL)
    // 2. Store xFlags -> emu->eflags (so C function has current flags)
    // 3. Create stack frame for C call
    // 4. Call C UpdateFlags(emu)
    // 5. Reload xFlags from emu->eflags (C function updated it)
    // 6. Restore LR and stack, return

    // Mark all unimplemented df types to jump to the fallback
SETMARK(d_imul8);
SETMARK(d_imul16);
SETMARK(d_imul32);
SETMARK(d_imul64);
SETMARK(d_mul8);
SETMARK(d_mul16);
SETMARK(d_mul32);
SETMARK(d_mul64);
SETMARK(d_adc8);
SETMARK(d_adc8b);
SETMARK(d_adc16);
SETMARK(d_adc16b);
SETMARK(d_adc32);
SETMARK(d_adc32b);
SETMARK(d_adc64);
SETMARK(d_sbb8);
SETMARK(d_sbb16);
SETMARK(d_sbb32);
SETMARK(d_sbb64);
SETMARK(d_rol8);
SETMARK(d_rol16);
SETMARK(d_rol32);
SETMARK(d_rol64);
SETMARK(d_ror8);
SETMARK(d_ror16);
SETMARK(d_ror32);
SETMARK(d_ror64);
SETMARK(d_shrd16);
SETMARK(d_shrd32);
SETMARK(d_shrd64);
SETMARK(d_shld16);
SETMARK(d_shld32);
SETMARK(d_shld64);

    // Fallback code: call C UpdateFlags(emu)

    // x7 already holds the caller's return LR (saved in prologue).
    // We need to preserve it across the C call.

    // Store current xFlags to emu->eflags so C function sees them
    STD(xFlags, offsetof(x64emu_t, eflags), xEmu);

    // Save xRIP (r9) to emu->ip — it is caller-saved and WILL be clobbered
    // by the C function call. All other x86 regs are in callee-saved r14-r29.
    STD(xRIP, offsetof(x64emu_t, ip), xEmu);

    // Create stack frame: 48 bytes (ELFv2 minimum 32 + 16 for local save)
    // Layout: SP+0=backchain, SP+8=CR, SP+16=callee LR save,
    //         SP+24=saved x7 (our caller's LR), SP+32..47=param save
    STDU(xSP, -48, xSP);       // create frame, store backchain
    STD(x7, 24, xSP);          // save caller's LR in local area

    // Load C UpdateFlags() address via TABLE64C
    TABLE64C(x6, const_updateflags);
    MTCTR(x6);

    // ELFv2 ABI: r12 = function entry address
    MFCTR(12);

    // Set up argument: r3 (A0) = emu pointer
    MV(A0, xEmu);

    // Call C UpdateFlags()
    BCTRL();

    // Reload xFlags from emu->eflags (C function updated it in memory)
    LD(xFlags, offsetof(x64emu_t, eflags), xEmu);

    // Restore xRIP (r9) from emu->ip
    LD(xRIP, offsetof(x64emu_t, ip), xEmu);

    // Restore LR and tear down stack frame
    LD(x7, 24, xSP);           // load saved caller's LR
    ADDI(xSP, xSP, 48);       // tear down frame
    MTLR(x7);                  // restore LR

    // Return to caller
    BLR();
}
