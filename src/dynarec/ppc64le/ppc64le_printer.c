#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ppc64le_printer.h"

// PPC64LE instruction disassembler for box64 dynarec debugging.
// Uses switch-based decoding on primary opcode (bits 26-31) then extended opcode.
// Register names use box64 symbolic names (xRAX, xEmu, etc.) for readability.

static char buff[200];

// ---- Field extraction macros ----
// PPC64LE stores instructions as little-endian uint32_t on LE hosts.
// The opcode passed in has already been read as a native uint32_t,
// so bit numbering is: bit 31 = MSB (PPC ISA bit 0), bit 0 = LSB (PPC ISA bit 31).

// Primary opcode: bits 26-31
#define OPCD(op)        (((op) >> 26) & 0x3F)
// D-form / DS-form fields
#define RT(op)          (((op) >> 21) & 0x1F)
#define RS(op)          RT(op)
#define RA(op)          (((op) >> 16) & 0x1F)
#define RB(op)          (((op) >> 11) & 0x1F)
#define SI(op)          ((int16_t)((op) & 0xFFFF))
#define UI(op)          ((op) & 0xFFFF)
#define DS(op)          ((int16_t)((op) & 0xFFFC))
#define DS_XO(op)       ((op) & 0x3)
// I-form
#define LI(op)          ((int32_t)(((op) & 0x03FFFFFC) << 6) >> 6)
#define AA(op)          (((op) >> 1) & 1)
#define LK(op)          ((op) & 1)
// B-form
#define BO(op)          RT(op)
#define BI_F(op)        RA(op)
#define BD(op)          ((int16_t)((op) & 0xFFFC))
// X-form
#define XO_X(op)        (((op) >> 1) & 0x3FF)
#define RC(op)          ((op) & 1)
// XO-form
#define XO_XO(op)       (((op) >> 1) & 0x1FF)
#define OE(op)          (((op) >> 10) & 1)
// XL-form
#define XO_XL(op)       XO_X(op)
// M-form
#define SH_M(op)        RB(op)
#define MB_M(op)        (((op) >> 6) & 0x1F)
#define ME_M(op)        (((op) >> 1) & 0x1F)
// MD-form: sh = sh[0:4] || sh[5], mb/me = mbe[0:4] || mbe[5]
#define SH_MD(op)       ((RB(op)) | (((op) >> 1) & 1) << 5)
#define MB_MD(op)       ((((op) >> 6) & 0x1F) | (((op) >> 5) & 1) << 5)
#define ME_MD(op)       MB_MD(op)
#define XO_MD(op)       (((op) >> 2) & 0x7)
// MDS-form
#define XO_MDS(op)      (((op) >> 1) & 0xF)
// XS-form: sh = sh[0:4] || sh[5]
#define SH_XS(op)       SH_MD(op)
#define XO_XS(op)       (((op) >> 2) & 0x1FF)
// XFX-form: spr = spr[5:9] || spr[0:4] (swapped halves)
#define SPR(op)         ((((op) >> 16) & 0x1F) | (((op) >> 6) & 0x3E0))
#define FXM(op)         (((op) >> 12) & 0xFF)
// XFL-form
#define FLM(op)         (((op) >> 17) & 0xFF)
// A-form
#define FRA(op)         RA(op)
#define FRB(op)         RB(op)
#define FRC(op)         (((op) >> 6) & 0x1F)
#define XO_A(op)        (((op) >> 1) & 0x1F)
// VA-form
#define VRA(op)         RA(op)
#define VRB(op)         RB(op)
#define VRC(op)         (((op) >> 6) & 0x1F)
#define XO_VA(op)       ((op) & 0x3F)
// VX-form
#define XO_VX(op)       ((op) & 0x7FF)
// DQ-form: T = T[0:4] || TX (bit 28), dq = bits 4-15 << 4
#define DQ(op)          ((int16_t)(((op) & 0xFFF0)))
#define DQ_TX(op)       (((op) >> 28) & 1)
#define DQ_T(op)        ((RT(op) << 1) | DQ_TX(op))
#define DQ_XO(op)       ((op) & 0x7)  // bits 0-2 but typically bit 3 is part of dq — actually XO is bits 0-2
// XX1-form: T = T[0:4](bits 25-21) || TX(bit 0)
#define XX1_TX(op)      ((op) & 1)
#define XX1_T(op)       ((RT(op) << 1) | XX1_TX(op))
#define XX1_XO(op)      (((op) >> 1) & 0x3FF)
// For XX1 with AX/BX: these don't apply to XX1 (XX1 only has TX)
// XX2-form: T[0:4]=bits25-21, B[0:4]=bits15-11, TX=bit0, BX=bit1
#define XX2_BX(op)      (((op) >> 1) & 1)
#define XX2_TX(op)      ((op) & 1)
#define XX2_T(op)       ((RT(op) << 1) | XX2_TX(op))
#define XX2_B(op)       ((RB(op) << 1) | XX2_BX(op))
#define XX2_XO(op)      (((op) >> 2) & 0x1FF)
#define XX2_UIM(op)     (((op) >> 16) & 0x3)   // for XXSPLTW
// XX3-form: T[0:4]=bits25-21, A[0:4]=bits20-16, B[0:4]=bits15-11, TX=bit0, AX=bit2, BX=bit1
#define XX3_AX(op)      (((op) >> 2) & 1)
#define XX3_BX(op)      (((op) >> 1) & 1)
#define XX3_TX(op)      ((op) & 1)
#define XX3_T(op)       ((RT(op) << 1) | XX3_TX(op))
#define XX3_A(op)       ((RA(op) << 1) | XX3_AX(op))
#define XX3_B(op)       ((RB(op) << 1) | XX3_BX(op))
#define XX3_XO(op)      (((op) >> 3) & 0xFF)
// XX4-form: T[0:4]=bits25-21, A[0:4]=bits20-16, B[0:4]=bits15-11, C[0:4]=bits10-6, TX=bit0, AX=bit2, BX=bit1, CX=bit3... no, actually:
// XX4: opcd(6)|T(5)|A(5)|B(5)|C(5)|XO(2)|CX(1)|AX(1)|BX(1)|TX(1)
#define XX4_TX(op)      ((op) & 1)
#define XX4_BX(op)      (((op) >> 1) & 1)
#define XX4_AX(op)      (((op) >> 2) & 1)
#define XX4_CX(op)      (((op) >> 3) & 1)
#define XX4_T(op)       ((RT(op) << 1) | XX4_TX(op))
#define XX4_A(op)       ((RA(op) << 1) | XX4_AX(op))
#define XX4_B(op)       ((RB(op) << 1) | XX4_BX(op))
#define XX4_C(op)       (((((op) >> 6) & 0x1F) << 1) | XX4_CX(op))
#define XX4_XO(op)      (((op) >> 4) & 0x3)

// Helpers
#define BF(op)          (((op) >> 23) & 0x7)
#define L_CMP(op)       (((op) >> 21) & 1)

static int signExtend(uint32_t val, int bits) {
    int shift = 32 - bits;
    return ((int32_t)(val << shift)) >> shift;
}

// ---- Register name tables ----
// GPR names with box64 symbolic aliases
static const char* Rt[32] = {
    "r0",    "sp",    "toc",   "x1",    "x2",    "x3",    "x4",    "x5",    // r0-r7
    "x6",    "xRIP",  "x7",    "r11",   "r12",   "r13",   "xRAX",  "xRCX",  // r8-r15
    "xRDX",  "xRBX",  "xRSP",  "xRBP",  "xRSI",  "xRDI",  "xR8",   "xR9",   // r16-r23
    "xR10",  "xR11",  "xR12",  "xR13",  "xR14",  "xR15",  "xFlags","xEmu"   // r24-r31
};

// FPR names
static const char* Ft[32] = {
    "f0",  "f1",  "f2",  "f3",  "f4",  "f5",  "f6",  "f7",
    "f8",  "f9",  "f10", "f11", "f12", "f13", "f14", "f15",
    "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
    "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
};

// VMX/VR names
static const char* Vt[32] = {
    "vr0",  "vr1",  "vr2",  "vr3",  "vr4",  "vr5",  "vr6",  "vr7",
    "vr8",  "vr9",  "vr10", "vr11", "vr12", "vr13", "vr14", "vr15",
    "vr16", "vr17", "vr18", "vr19", "vr20", "vr21", "vr22", "vr23",
    "vr24", "vr25", "vr26", "vr27", "vr28", "vr29", "vr30", "vr31"
};

// VSX register names (vs0-vs63)
static const char* VSXname(int idx) {
    static char vbuf[8];
    snprintf(vbuf, sizeof(vbuf), "vs%d", idx);
    return vbuf;
}

// CR field names
static const char* CRnames[8] = {
    "cr0", "cr1", "cr2", "cr3", "cr4", "cr5", "cr6", "cr7"
};

// SPR names
static const char* SPRname(int spr) {
    switch(spr) {
        case 1: return "XER";
        case 8: return "LR";
        case 9: return "CTR";
        case 268: return "TB";
        default: {
            static char sbuf[16];
            snprintf(sbuf, sizeof(sbuf), "SPR%d", spr);
            return sbuf;
        }
    }
}

// ---- Condition branch helpers ----
static const char* condName(int bo, int bi) {
    int cr = bi / 4;
    int bit = bi % 4;
    static char cbuf[16];
    const char* bitsuf[] = {"lt", "gt", "eq", "so"};
    if (bo == 12) {  // branch if true
        if (cr == 0)
            snprintf(cbuf, sizeof(cbuf), "b%s", bitsuf[bit]);
        else
            snprintf(cbuf, sizeof(cbuf), "b%s %s", bitsuf[bit], CRnames[cr]);
    } else if (bo == 4) {  // branch if false
        const char* negbitsuf[] = {"ge", "le", "ne", "nso"};
        if (cr == 0)
            snprintf(cbuf, sizeof(cbuf), "b%s", negbitsuf[bit]);
        else
            snprintf(cbuf, sizeof(cbuf), "b%s %s", negbitsuf[bit], CRnames[cr]);
    } else if (bo == 20) {
        snprintf(cbuf, sizeof(cbuf), "b");
    } else {
        snprintf(cbuf, sizeof(cbuf), "bc %d,%d", bo, bi);
    }
    return cbuf;
}

// ---- Main disassembler ----
const char* ppc64le_print(uint32_t opcode, uint64_t addr)
{
    int op = OPCD(opcode);
    int rt = RT(opcode);
    int rs = rt;
    int ra = RA(opcode);
    int rb = RB(opcode);
    int rc_bit = RC(opcode);
    const char* dot = rc_bit ? "." : "";

    switch(op) {
    // ---- opcode 2: TDI ----
    case 2:
        snprintf(buff, sizeof(buff), "%-15s %d, %s, %d", "tdi", rt, Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 3: TWI ----
    case 3:
        snprintf(buff, sizeof(buff), "%-15s %d, %s, %d", "twi", rt, Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 4: VMX/Altivec ----
    case 4: {
        // Check VA-form first (bits 0-5)
        int va_xo = XO_VA(opcode);
        switch(va_xo) {
        case 32: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmhaddshs", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 33: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmhraddshs", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 34: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmladduhm", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 36: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmsumubm", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 37: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmsummbm", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 38: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmsumuhm", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 40: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmsumshm", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 41: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vmsumshs", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 42: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vsel", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 43: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "vperm", Vt[rt], Vt[ra], Vt[rb], Vt[VRC(opcode)]); return buff;
        case 44: {
            int shb = VRC(opcode) >> 1;  // vsldoi shb is in bits 6-9
            shb = (opcode >> 6) & 0xF;
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "vsldoi", Vt[rt], Vt[ra], Vt[rb], shb);
            return buff;
        }
        case 51: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "maddld", Rt[rt], Rt[ra], Rt[rb], Rt[VRC(opcode)]); return buff;
        }
        // VX-form (bits 0-10)
        int vx_xo = XO_VX(opcode);
        switch(vx_xo) {
        // Integer add
        case    0: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vaddubm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   64: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vadduhm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  128: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vadduwm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  192: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vaddudm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Integer add saturate
        case  512: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vaddubs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  576: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vadduhs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  768: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vaddsbs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  832: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vaddshs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  896: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vaddsws", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Integer subtract
        case 1024: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsububm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1088: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsubuhm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1152: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsubuwm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1216: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsubudm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Integer subtract saturate
        case 1536: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsububs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1600: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsubuhs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1792: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsubsbs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1856: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsubshs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Compare equal
        case    6: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpequb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   70: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpequh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  134: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpequw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  199: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpequd", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Compare greater than signed
        case  774: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtsb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  838: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtsh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  902: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtsw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  967: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtsd", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Compare greater than unsigned
        case  518: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  582: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtuh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  646: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtuw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  711: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vcmpgtud", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Min/Max unsigned
        case  514: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  578: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminuh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  642: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminuw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  706: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminud", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case    2: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   66: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxuh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  130: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxuw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  194: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxud", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Min/Max signed
        case  770: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminsb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  834: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminsh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  898: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminsw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  962: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vminsd", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  258: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxsb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  322: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxsh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  386: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxsw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  450: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmaxsd", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Multiply even/odd
        case  520: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmuleub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  584: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmuleuh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  648: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmuleuw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  776: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulesb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  840: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulesh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  904: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulesw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case    8: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmuloub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   72: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulouh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  136: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulouw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  264: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulosb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  328: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulosh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  392: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmulosw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  137: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmuluwm", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Pack
        case   14: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkuhum", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   78: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkuwum", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  398: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkshss", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  462: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkswss", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  270: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkshus", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  334: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkswus", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1486: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpksdss", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1358: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpksdus", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1102: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vpkudum", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Unpack
        case  526: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vupkhsb", Vt[rt], Vt[rb]); return buff;
        case  590: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vupkhsh", Vt[rt], Vt[rb]); return buff;
        case 1614: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vupkhsw", Vt[rt], Vt[rb]); return buff;
        case  654: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vupklsb", Vt[rt], Vt[rb]); return buff;
        case  718: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vupklsh", Vt[rt], Vt[rb]); return buff;
        case 1742: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vupklsw", Vt[rt], Vt[rb]); return buff;
        // Extend sign (P9) — share XO 1538, distinguished by VRA field
        case 1538:
            switch(ra) {
            case 16: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vextsb2w", Vt[rt], Vt[rb]); return buff;
            case 17: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vextsh2w", Vt[rt], Vt[rb]); return buff;
            case 24: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vextsb2d", Vt[rt], Vt[rb]); return buff;
            case 25: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vextsh2d", Vt[rt], Vt[rb]); return buff;
            case 26: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vextsw2d", Vt[rt], Vt[rb]); return buff;
            case  6: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vnegw", Vt[rt], Vt[rb]); return buff;
            case  7: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vnegd", Vt[rt], Vt[rb]); return buff;
            default: snprintf(buff, sizeof(buff), "%-15s %s, %d, %s", "vx_1538", Vt[rt], ra, Vt[rb]); return buff;
            }
        // Merge
        case   12: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmrghb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   76: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmrghh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  140: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmrghw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  268: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmrglb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  332: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmrglh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  396: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vmrglw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Shift
        case  260: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vslb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  324: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vslh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  388: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vslw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1476: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsld", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  516: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  580: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  644: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1732: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrd", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  772: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrab", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  836: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrah", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  900: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsraw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  964: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsrad", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  452: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsl", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  708: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsr", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1036: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vslo", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1100: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsro", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Rotate
        case    4: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vrlb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case   68: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vrlh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  132: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vrlw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case  196: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vrld", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Splat
        case  524: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vspltb", Vt[rt], Vt[rb], ra); return buff;
        case  588: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vsplth", Vt[rt], Vt[rb], ra); return buff;
        case  652: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vspltw", Vt[rt], Vt[rb], ra); return buff;
        case  780: snprintf(buff, sizeof(buff), "%-15s %s, %d", "vspltisb", Vt[rt], signExtend(ra, 5)); return buff;
        case  844: snprintf(buff, sizeof(buff), "%-15s %s, %d", "vspltish", Vt[rt], signExtend(ra, 5)); return buff;
        case  908: snprintf(buff, sizeof(buff), "%-15s %s, %d", "vspltisw", Vt[rt], signExtend(ra, 5)); return buff;
        // Logical
        case 1028: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vand", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1092: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vandc", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1156:
            if (ra == rb)
                snprintf(buff, sizeof(buff), "%-15s %s, %s", "vmr", Vt[rt], Vt[ra]);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vor", Vt[rt], Vt[ra], Vt[rb]);
            return buff;
        case 1220: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vxor", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1284: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vnor", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1348: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vorc", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1412: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vnand", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1668: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "veqv", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Average
        case 1026: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vavgub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1090: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vavguh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1282: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vavgsb", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1346: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vavgsh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Absolute difference (P9)
        case 1027: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vabsdub", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1091: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vabsduh", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1155: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vabsduw", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Sum across
        case 1672: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsum4ubs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1800: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsum4sbs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        case 1608: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vsum4shs", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Count leading zeros
        case 1794: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vclzb", Vt[rt], Vt[rb]); return buff;
        case 1858: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vclzh", Vt[rt], Vt[rb]); return buff;
        case 1922: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vclzw", Vt[rt], Vt[rb]); return buff;
        case 1986: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vclzd", Vt[rt], Vt[rb]); return buff;
        // Population count
        case 1795: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vpopcntb", Vt[rt], Vt[rb]); return buff;
        case 1859: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vpopcnth", Vt[rt], Vt[rb]); return buff;
        case 1923: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vpopcntw", Vt[rt], Vt[rb]); return buff;
        case 1987: snprintf(buff, sizeof(buff), "%-15s %s, %s", "vpopcntd", Vt[rt], Vt[rb]); return buff;
        // Bit permute
        case 1356: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "vbpermq", Vt[rt], Vt[ra], Vt[rb]); return buff;
        // Extract/Insert (P9) - use ra as immediate
        case  525: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vextractub", Vt[rt], Vt[rb], ra); return buff;
        case  589: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vextractuh", Vt[rt], Vt[rb], ra); return buff;
        case  653: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vextractuw", Vt[rt], Vt[rb], ra); return buff;
        case  717: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vextractd", Vt[rt], Vt[rb], ra); return buff;
        case  781: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vinsertb", Vt[rt], Vt[rb], ra); return buff;
        case  845: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vinserth", Vt[rt], Vt[rb], ra); return buff;
        case  909: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vinsertw", Vt[rt], Vt[rb], ra); return buff;
        case  973: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vinsertd", Vt[rt], Vt[rb], ra); return buff;
        // Convert
        case  778: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vcfux", Vt[rt], Vt[rb], ra); return buff;
        case  842: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vcfsx", Vt[rt], Vt[rb], ra); return buff;
        case  906: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vctuxs", Vt[rt], Vt[rb], ra); return buff;
        case  970: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "vctsxs", Vt[rt], Vt[rb], ra); return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s (vx_xo=%d)", opcode, "vmx???", vx_xo);
        return buff;
    }

    // ---- opcode 7: MULLI ----
    case 7:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "mulli", Rt[rt], Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 8: SUBFIC ----
    case 8:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "subfic", Rt[rt], Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 10: CMPLI ----
    case 10: {
        int bf = BF(opcode);
        int l = L_CMP(opcode);
        const char* mn = l ? "cmpldi" : "cmplwi";
        if (bf == 0)
            snprintf(buff, sizeof(buff), "%-15s %s, %u", mn, Rt[ra], UI(opcode));
        else
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %u", mn, CRnames[bf], Rt[ra], UI(opcode));
        return buff;
    }

    // ---- opcode 11: CMPI ----
    case 11: {
        int bf = BF(opcode);
        int l = L_CMP(opcode);
        const char* mn = l ? "cmpdi" : "cmpwi";
        if (bf == 0)
            snprintf(buff, sizeof(buff), "%-15s %s, %d", mn, Rt[ra], SI(opcode));
        else
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", mn, CRnames[bf], Rt[ra], SI(opcode));
        return buff;
    }

    // ---- opcode 12: ADDIC ----
    case 12:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "addic", Rt[rt], Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 13: ADDIC. ----
    case 13:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "addic.", Rt[rt], Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 14: ADDI / LI ----
    case 14:
        if (ra == 0)
            snprintf(buff, sizeof(buff), "%-15s %s, %d", "li", Rt[rt], SI(opcode));
        else
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "addi", Rt[rt], Rt[ra], SI(opcode));
        return buff;

    // ---- opcode 15: ADDIS / LIS ----
    case 15:
        if (ra == 0)
            snprintf(buff, sizeof(buff), "%-15s %s, 0x%X", "lis", Rt[rt], UI(opcode));
        else
            snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "addis", Rt[rt], Rt[ra], UI(opcode));
        return buff;

    // ---- opcode 16: BC (conditional branch) ----
    case 16: {
        int bo = BO(opcode);
        int bi = BI_F(opcode);
        int bd = BD(opcode);
        int lk = LK(opcode);
        int aa = AA(opcode);
        const char* cn = condName(bo, bi);
        if (aa)
            snprintf(buff, sizeof(buff), "%-15s 0x%X%s", cn, (unsigned)(bd & 0xFFFF), lk ? "l" : "");
        else
            snprintf(buff, sizeof(buff), "%-15s #%+d\t; %p%s", cn, bd, (void*)(addr + bd), lk ? " (link)" : "");
        return buff;
    }

    // ---- opcode 17: SC ----
    case 17:
        snprintf(buff, sizeof(buff), "%-15s", "sc");
        return buff;

    // ---- opcode 18: B/BL/BA/BLA ----
    case 18: {
        int li = LI(opcode);
        int lk = LK(opcode);
        int aa = AA(opcode);
        if (aa)
            snprintf(buff, sizeof(buff), "%-15s 0x%X", lk ? "bla" : "ba", (unsigned)(li & 0x03FFFFFF));
        else
            snprintf(buff, sizeof(buff), "%-15s #%+d\t; %p", lk ? "bl" : "b", li, (void*)(addr + li));
        return buff;
    }

    // ---- opcode 19: XL-form (Branch to LR/CTR, CR logical, isync) ----
    case 19: {
        int xo = XO_XL(opcode);
        int bo = BO(opcode);
        int bi = BI_F(opcode);
        int lk = LK(opcode);
        switch(xo) {
        case 16: // BCLR
            if (bo == 20 && bi == 0)
                snprintf(buff, sizeof(buff), "%-15s", lk ? "blrl" : "blr");
            else {
                const char* cn = condName(bo, bi);
                snprintf(buff, sizeof(buff), "%slr%s", cn, lk ? "l" : "");
            }
            return buff;
        case 528: // BCCTR
            if (bo == 20 && bi == 0)
                snprintf(buff, sizeof(buff), "%-15s", lk ? "bctrl" : "bctr");
            else {
                const char* cn = condName(bo, bi);
                snprintf(buff, sizeof(buff), "%sctr%s", cn, lk ? "l" : "");
            }
            return buff;
        case 150:
            snprintf(buff, sizeof(buff), "%-15s", "isync");
            return buff;
        // CR logical
        case 257: snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "crand", rt, ra, rb); return buff;
        case 129: snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "crandc", rt, ra, rb); return buff;
        case 449:
            if (ra == rb)
                snprintf(buff, sizeof(buff), "%-15s %d, %d", "crmove", rt, ra);
            else
                snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "cror", rt, ra, rb);
            return buff;
        case 417: snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "crorc", rt, ra, rb); return buff;
        case 193:
            if (ra == rt && rb == rt)
                snprintf(buff, sizeof(buff), "%-15s %d", "crclr", rt);
            else
                snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "crxor", rt, ra, rb);
            return buff;
        case 225: snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "crnand", rt, ra, rb); return buff;
        case  33:
            if (ra == rb)
                snprintf(buff, sizeof(buff), "%-15s %d, %d", "crnot", rt, ra);
            else
                snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "crnor", rt, ra, rb);
            return buff;
        case 289:
            if (ra == rt && rb == rt)
                snprintf(buff, sizeof(buff), "%-15s %d", "crset", rt);
            else
                snprintf(buff, sizeof(buff), "%-15s %d, %d, %d", "creqv", rt, ra, rb);
            return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s (xl_xo=%d)", opcode, "xl???", xo);
        return buff;
    }

    // ---- opcode 20: RLWIMI ----
    case 20: {
        int sh = SH_M(opcode);
        int mb = MB_M(opcode);
        int me = ME_M(opcode);
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %d, %d, %d", rc_bit ? "rlwimi." : "rlwimi", Rt[ra], Rt[rs], sh, mb, me);
        return buff;
    }

    // ---- opcode 21: RLWINM ----
    case 21: {
        int sh = SH_M(opcode);
        int mb = MB_M(opcode);
        int me = ME_M(opcode);
        // Detect pseudo-ops
        if (mb == 0 && me == (31 - sh))
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "slwi." : "slwi", Rt[ra], Rt[rs], sh);
        else if (sh == (32 - mb) && me == 31)
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "srwi." : "srwi", Rt[ra], Rt[rs], mb);
        else if (sh == 0 && me == 31)
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "clrlwi." : "clrlwi", Rt[ra], Rt[rs], mb);
        else if (sh == 0 && mb == 0)
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "clrrwi." : "clrrwi", Rt[ra], Rt[rs], 31 - me);
        else
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d, %d, %d", rc_bit ? "rlwinm." : "rlwinm", Rt[ra], Rt[rs], sh, mb, me);
        return buff;
    }

    // ---- opcode 23: RLWNM ----
    case 23: {
        int mb = MB_M(opcode);
        int me = ME_M(opcode);
        snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d, %d", rc_bit ? "rlwnm." : "rlwnm", Rt[ra], Rt[rs], Rt[rb], mb, me);
        return buff;
    }

    // ---- opcode 24: ORI ----
    case 24:
        if (rs == 0 && ra == 0 && UI(opcode) == 0)
            snprintf(buff, sizeof(buff), "%-15s", "nop");
        else
            snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "ori", Rt[ra], Rt[rs], UI(opcode));
        return buff;

    // ---- opcode 25: ORIS ----
    case 25:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "oris", Rt[ra], Rt[rs], UI(opcode));
        return buff;

    // ---- opcode 26: XORI ----
    case 26:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "xori", Rt[ra], Rt[rs], UI(opcode));
        return buff;

    // ---- opcode 27: XORIS ----
    case 27:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "xoris", Rt[ra], Rt[rs], UI(opcode));
        return buff;

    // ---- opcode 28: ANDI. ----
    case 28:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "andi.", Rt[ra], Rt[rs], UI(opcode));
        return buff;

    // ---- opcode 29: ANDIS. ----
    case 29:
        snprintf(buff, sizeof(buff), "%-15s %s, %s, 0x%X", "andis.", Rt[ra], Rt[rs], UI(opcode));
        return buff;

    // ---- opcode 30: MD/MDS-form (64-bit rotate) ----
    case 30: {
        int md_xo = XO_MD(opcode);
        int sh = SH_MD(opcode);
        int mbe = MB_MD(opcode);
        switch(md_xo) {
        case 0: // RLDICL
            if (sh == 0)
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "clrldi." : "clrldi", Rt[ra], Rt[rs], mbe);
            else if (mbe == 0)
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "rotldi." : "rotldi", Rt[ra], Rt[rs], sh);
            else if (mbe == (64 - sh))
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "srdi." : "srdi", Rt[ra], Rt[rs], 64 - sh);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %d, %d", rc_bit ? "rldicl." : "rldicl", Rt[ra], Rt[rs], sh, mbe);
            return buff;
        case 1: // RLDICR
            if (mbe == (63 - sh))
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "sldi." : "sldi", Rt[ra], Rt[rs], sh);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %d, %d", rc_bit ? "rldicr." : "rldicr", Rt[ra], Rt[rs], sh, mbe);
            return buff;
        case 2: // RLDIC
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d, %d", rc_bit ? "rldic." : "rldic", Rt[ra], Rt[rs], sh, mbe);
            return buff;
        case 3: // RLDIMI
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d, %d", rc_bit ? "rldimi." : "rldimi", Rt[ra], Rt[rs], sh, mbe);
            return buff;
        }
        // MDS-form: xo is bits 1-4
        int mds_xo = XO_MDS(opcode);
        switch(mds_xo) {
        case 8: // RLDCL
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", rc_bit ? "rldcl." : "rldcl", Rt[ra], Rt[rs], Rt[rb], mbe);
            return buff;
        case 9: // RLDCR
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", rc_bit ? "rldcr." : "rldcr", Rt[ra], Rt[rs], Rt[rb], mbe);
            return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s", opcode, "rld???");
        return buff;
    }

    // ---- opcode 31: X/XO/XFX-form (the big one) ----
    case 31: {
        int xo10 = XO_X(opcode);   // 10-bit extended opcode
        int xo9 = XO_XO(opcode);   // 9-bit extended opcode (XO-form)
        int oe = OE(opcode);

        // XO-form instructions (9-bit xo, OE bit)
        // Check XO-form first by matching on the 9-bit xo
        // ADD/SUB/MUL/DIV family
        switch(xo9) {
        case 266: // ADD
            if (oe)
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "addo." : "addo", Rt[rt], Rt[ra], Rt[rb]);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "add." : "add", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 10: // ADDC
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "addc." : "addc", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 138: // ADDE
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "adde." : "adde", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 202: // ADDZE
            snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "addze." : "addze", Rt[rt], Rt[ra]);
            return buff;
        case 40: // SUBF
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "subf." : "subf", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 8: // SUBFC
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "subfc." : "subfc", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 136: // SUBFE
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "subfe." : "subfe", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 200: // SUBFZE
            snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "subfze." : "subfze", Rt[rt], Rt[ra]);
            return buff;
        case 104: // NEG
            snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "neg." : "neg", Rt[rt], Rt[ra]);
            return buff;
        case 235: // MULLW
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "mullw." : "mullw", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 75: // MULHW
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "mulhw." : "mulhw", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 11: // MULHWU
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "mulhwu." : "mulhwu", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 233: // MULLD
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "mulld." : "mulld", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 73: // MULHD
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "mulhd", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 9: // MULHDU
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "mulhdu", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 491: // DIVW
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "divw", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 459: // DIVWU
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "divwu", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 489: // DIVD
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "divd", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        case 457: // DIVDU
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "divdu", Rt[rt], Rt[ra], Rt[rb]);
            return buff;
        }

        // X-form instructions (10-bit xo)
        switch(xo10) {
        // Compare
        case 0: {
            int bf = BF(opcode);
            int l = (rt >> 0) & 1;  // L bit is bit 21
            l = L_CMP(opcode);
            const char* mn = l ? "cmpd" : "cmpw";
            if (bf == 0)
                snprintf(buff, sizeof(buff), "%-15s %s, %s", mn, Rt[ra], Rt[rb]);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", mn, CRnames[bf], Rt[ra], Rt[rb]);
            return buff;
        }
        case 32: {
            int bf = BF(opcode);
            int l = L_CMP(opcode);
            const char* mn = l ? "cmpld" : "cmplw";
            if (bf == 0)
                snprintf(buff, sizeof(buff), "%-15s %s, %s", mn, Rt[ra], Rt[rb]);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", mn, CRnames[bf], Rt[ra], Rt[rb]);
            return buff;
        }
        // TW/TD
        case 4: snprintf(buff, sizeof(buff), "%-15s %d, %s, %s", "tw", rt, Rt[ra], Rt[rb]); return buff;
        case 68: snprintf(buff, sizeof(buff), "%-15s %d, %s, %s", "td", rt, Rt[ra], Rt[rb]); return buff;
        // MFCR
        case 19: {
            int fxm = FXM(opcode);
            int one_bit = (opcode >> 20) & 1;  // bit 20 distinguishes mfcr/mfocrf
            if (one_bit)
                snprintf(buff, sizeof(buff), "%-15s %s, 0x%02X", "mfocrf", Rt[rt], fxm);
            else
                snprintf(buff, sizeof(buff), "%-15s %s", "mfcr", Rt[rt]);
            return buff;
        }
        // MTCRF / MTOCRF
        case 144: {
            int fxm = FXM(opcode);
            int one_bit = (opcode >> 20) & 1;
            if (one_bit)
                snprintf(buff, sizeof(buff), "%-15s 0x%02X, %s", "mtocrf", fxm, Rt[rs]);
            else if (fxm == 0xFF)
                snprintf(buff, sizeof(buff), "%-15s %s", "mtcr", Rt[rs]);
            else
                snprintf(buff, sizeof(buff), "%-15s 0x%02X, %s", "mtcrf", fxm, Rt[rs]);
            return buff;
        }
        // Logical
        case 28: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "and." : "and", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 60: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "andc." : "andc", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 444:
            if (rs == rb) {
                if (rs == ra)
                    snprintf(buff, sizeof(buff), "%-15s", "nop");  // or r,r,r = nop
                else
                    snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "mr." : "mr", Rt[ra], Rt[rs]);
            } else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "or." : "or", Rt[ra], Rt[rs], Rt[rb]);
            return buff;
        case 412: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "orc", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 316: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "xor." : "xor", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 476: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "nand", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 124:
            if (rs == rb)
                snprintf(buff, sizeof(buff), "%-15s %s, %s", "not", Rt[ra], Rt[rs]);
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "nor", Rt[ra], Rt[rs], Rt[rb]);
            return buff;
        case 284: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "eqv", Rt[ra], Rt[rs], Rt[rb]); return buff;
        // Extend/Count
        case 954: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "extsb." : "extsb", Rt[ra], Rt[rs]); return buff;
        case 922: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "extsh." : "extsh", Rt[ra], Rt[rs]); return buff;
        case 986: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "extsw." : "extsw", Rt[ra], Rt[rs]); return buff;
        case 26: snprintf(buff, sizeof(buff), "%-15s %s, %s", "cntlzw", Rt[ra], Rt[rs]); return buff;
        case 58: snprintf(buff, sizeof(buff), "%-15s %s, %s", "cntlzd", Rt[ra], Rt[rs]); return buff;
        case 538: snprintf(buff, sizeof(buff), "%-15s %s, %s", "cnttzw", Rt[ra], Rt[rs]); return buff;
        case 570: snprintf(buff, sizeof(buff), "%-15s %s, %s", "cnttzd", Rt[ra], Rt[rs]); return buff;
        case 378: snprintf(buff, sizeof(buff), "%-15s %s, %s", "popcntw", Rt[ra], Rt[rs]); return buff;
        case 506: snprintf(buff, sizeof(buff), "%-15s %s, %s", "popcntd", Rt[ra], Rt[rs]); return buff;
        case 122: snprintf(buff, sizeof(buff), "%-15s %s, %s", "popcntb", Rt[ra], Rt[rs]); return buff;
        // Shift
        case 24: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "slw", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 536: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "srw", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 792: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "sraw." : "sraw", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 824: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "srawi." : "srawi", Rt[ra], Rt[rs], rb); return buff;
        case 27: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "sld", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 539: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "srd", Rt[ra], Rt[rs], Rt[rb]); return buff;
        case 794: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "srad." : "srad", Rt[ra], Rt[rs], Rt[rb]); return buff;
        // SRADI / EXTSWSLI (XS-form, but primary opcode is still 31)
        // SRADI xo = 413, in XS encoding
        // The 10-bit xo actually is 413<<1|sh5 for XS-form, but we already extract xo10 as bits 1-10
        // For SRADI: bits 2-10 = 413 (9-bit), bit 1 = sh[5]
        // XS_XO = bits 2-10 = 9-bit
        // Let's check by the 9-bit: (xo10 >> 1) for the top 9 bits
        // Actually SRADI occupies xo10 = 826 or 827 (bit 0 = sh[5])
        case 826: case 827: {
            int sh6 = SH_XS(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "sradi." : "sradi", Rt[ra], Rt[rs], sh6);
            return buff;
        }
        // EXTSWSLI: xo = 445 in XS 9-bit → xo10 = 890 or 891
        case 890: case 891: {
            int sh6 = SH_XS(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", rc_bit ? "extswsli." : "extswsli", Rt[ra], Rt[rs], sh6);
            return buff;
        }
        // Modulo (P9)
        case 267: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "moduw", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 779: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "modsw", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 265: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "modud", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 777: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "modsd", Rt[rt], Rt[ra], Rt[rb]); return buff;
        // Load indexed
        case 87: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lbzx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 279: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lhzx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 343: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lhax", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 23: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lwzx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 341: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lwax", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 21: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ldx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        // Store indexed
        case 215: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stbx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 407: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "sthx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 151: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stwx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 149: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stdx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        // Byte-reverse load/store
        case 790: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lhbrx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 534: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lwbrx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 532: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ldbrx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 918: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "sthbrx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 662: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stwbrx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 660: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stdbrx", Rt[rs], Rt[ra], Rt[rb]); return buff;
        // Load/Store with reservation (atomics)
        case 52: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lbarx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 116: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lharx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 20: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lwarx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 84: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "ldarx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 276: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lqarx", Rt[rt], Rt[ra], Rt[rb]); return buff;
        case 694: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stbcx.", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 726: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "sthcx.", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 150: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stwcx.", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 214: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stdcx.", Rt[rs], Rt[ra], Rt[rb]); return buff;
        case 182: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stqcx.", Rt[rs], Rt[ra], Rt[rb]); return buff;
        // FP indexed load/store
        case 535: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lfsx", Ft[rt], Rt[ra], Rt[rb]); return buff;
        case 599: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lfdx", Ft[rt], Rt[ra], Rt[rb]); return buff;
        case 663: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stfsx", Ft[rs], Rt[ra], Rt[rb]); return buff;
        case 727: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stfdx", Ft[rs], Rt[ra], Rt[rb]); return buff;
        // VMX indexed load/store
        case 103: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lvx", Vt[rt], Rt[ra], Rt[rb]); return buff;
        case 231: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stvx", Vt[rs], Rt[ra], Rt[rb]); return buff;
        // Sync / barriers
        case 598: {
            int l_field = (opcode >> 21) & 3;
            if (l_field == 0)
                snprintf(buff, sizeof(buff), "%-15s", "sync");
            else if (l_field == 1)
                snprintf(buff, sizeof(buff), "%-15s", "lwsync");
            else
                snprintf(buff, sizeof(buff), "%-15s %d", "sync", l_field);
            return buff;
        }
        case 854: snprintf(buff, sizeof(buff), "%-15s", "eieio"); return buff;
        // Cache management
        case 54: snprintf(buff, sizeof(buff), "%-15s %s, %s", "dcbst", Rt[ra], Rt[rb]); return buff;
        case 86: snprintf(buff, sizeof(buff), "%-15s %s, %s", "dcbf", Rt[ra], Rt[rb]); return buff;
        case 982: snprintf(buff, sizeof(buff), "%-15s %s, %s", "icbi", Rt[ra], Rt[rb]); return buff;
        // SPR move
        case 339: {
            int spr = SPR(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s", "mfspr", Rt[rt], SPRname(spr));
            return buff;
        }
        case 467: {
            int spr = SPR(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s", "mtspr", SPRname(spr), Rt[rs]);
            return buff;
        }
        // ISEL — special encoding: xo = BC<<1 | 15  (bits 1-5 = 01111, bits 6-10 = BC)
        // The 10-bit xo for ISEL has bits 1-5 = 01111 = 15
        // So xo10 & 0x1F = 15 and BC = (xo10 >> 5) & 0x1F
        default:
            if ((xo10 & 0x1F) == 15) {
                int bc = (xo10 >> 5) & 0x1F;
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "isel", Rt[rt], Rt[ra], Rt[rb], bc);
                return buff;
            }
            break;
        }
        // XX1-form instructions under opcode 31
        {
            int xx1_xo = XX1_XO(opcode);
            int xt = XX1_T(opcode);
            switch(xx1_xo) {
            case 51: snprintf(buff, sizeof(buff), "%-15s %s, %s", "mfvsrd", Rt[ra], VSXname(xt)); return buff;
            case 179: snprintf(buff, sizeof(buff), "%-15s %s, %s", "mtvsrd", VSXname(xt), Rt[ra]); return buff;
            case 115: snprintf(buff, sizeof(buff), "%-15s %s, %s", "mfvsrwz", Rt[ra], VSXname(xt)); return buff;
            case 243: snprintf(buff, sizeof(buff), "%-15s %s, %s", "mtvsrwz", VSXname(xt), Rt[ra]); return buff;
            case 435: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "mtvsrdd", VSXname(xt), Rt[ra], Rt[rb]); return buff;
            case 307: snprintf(buff, sizeof(buff), "%-15s %s, %s", "mfvsrld", Rt[ra], VSXname(xt)); return buff;
            case 268: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "lxvx", VSXname(xt), Rt[ra], Rt[rb]); return buff;
            case 396: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "stxvx", VSXname(xt), Rt[ra], Rt[rb]); return buff;
            }
        }
        snprintf(buff, sizeof(buff), "%08X %-9s (xo=%d)", opcode, "x???", xo10);
        return buff;
    }

    // ---- opcode 32: LWZ ----
    case 32:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lwz", Rt[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 33: LWZU ----
    case 33:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lwzu", Rt[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 34: LBZ ----
    case 34:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lbz", Rt[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 35: LBZU ----
    case 35:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lbzu", Rt[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 36: STW ----
    case 36:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stw", Rt[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 37: STWU ----
    case 37:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stwu", Rt[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 38: STB ----
    case 38:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stb", Rt[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 39: STBU ----
    case 39:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stbu", Rt[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 40: LHZ ----
    case 40:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lhz", Rt[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 42: LHA ----
    case 42:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lha", Rt[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 44: STH ----
    case 44:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "sth", Rt[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 48: LFS ----
    case 48:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lfs", Ft[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 50: LFD ----
    case 50:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lfd", Ft[rt], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 52: STFS ----
    case 52:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stfs", Ft[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 54: STFD ----
    case 54:
        snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stfd", Ft[rs], SI(opcode), Rt[ra]);
        return buff;

    // ---- opcode 58: DS-form (LD, LDU, LWA) ----
    case 58: {
        int ds_xo = DS_XO(opcode);
        int offset = DS(opcode);
        switch(ds_xo) {
        case 0: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "ld", Rt[rt], offset, Rt[ra]); return buff;
        case 1: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "ldu", Rt[rt], offset, Rt[ra]); return buff;
        case 2: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lwa", Rt[rt], offset, Rt[ra]); return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s", opcode, "ds58???");
        return buff;
    }

    // ---- opcode 59: A-form single-precision FP ----
    case 59: {
        int a_xo = XO_A(opcode);
        int frt = rt, fra = ra, frb = rb, frc = FRC(opcode);
        switch(a_xo) {
        case 18: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fdivs." : "fdivs", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 20: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fsubs." : "fsubs", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 21: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fadds." : "fadds", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 22: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "fsqrts." : "fsqrts", Ft[frt], Ft[frb]); return buff;
        case 25: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fmuls." : "fmuls", Ft[frt], Ft[fra], Ft[frc]); return buff;
        // FCFIDS (opcode 59 xo 846)
        // A-form xo is only 5 bits (bits 1-5), but FCFIDS uses X-form under opcode 59
        // Actually FCFIDS: opcode=59, xo=846 in X-form
        }
        // X-form under opcode 59 (10-bit xo)
        int x59_xo = XO_X(opcode);
        switch(x59_xo) {
        case 846: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fcfids", Ft[rt], Ft[rb]); return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s (a_xo=%d)", opcode, "fp59???", a_xo);
        return buff;
    }

    // ---- opcode 60: XX2/XX3/XX4-form (VSX operations) ----
    case 60: {
        // Try XX3-form first (8-bit xo, bits 3-10)
        int xx3_xo = XX3_XO(opcode);
        int xt3 = XX3_T(opcode);
        int xa3 = XX3_A(opcode);
        int xb3 = XX3_B(opcode);

        switch(xx3_xo) {
        // Logical
        case 146:
            if (xa3 == xb3)
                snprintf(buff, sizeof(buff), "%-15s %s, %s", "xxmr", VSXname(xt3), VSXname(xa3));
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxlor", VSXname(xt3), VSXname(xa3), VSXname(xb3));
            return buff;
        case 154: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxlxor", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 130: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxland", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 138: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxlandc", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 162: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxlnor", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 170: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxlorc", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 178: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxlnand", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 186: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxleqv", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        // Permute/merge — note: dm is encoded in bits 8-9 of the XO, so different dm values
        // give different xx3_xo. xxpermdi base=10, xxmrghd=xxpermdi(dm=0)=10, xxmrgld=xxpermdi(dm=1)=42
        case 10: case 42: case 74: case 106: {
            int dm = (xx3_xo >> 5) & 3;
            if (dm == 0)
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxmrghd", VSXname(xt3), VSXname(xa3), VSXname(xb3));
            else if (dm == 1 && xa3 == xb3)
                snprintf(buff, sizeof(buff), "%-15s %s, %s", "xxswapd", VSXname(xt3), VSXname(xa3));
            else if (dm == 3)
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xxmrgld", VSXname(xt3), VSXname(xa3), VSXname(xb3));
            else
                snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %d", "xxpermdi", VSXname(xt3), VSXname(xa3), VSXname(xb3), dm);
            return buff;
        }
        // FP double vector arithmetic
        case 96: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvadddp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 104: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvsubdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 112: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvmuldp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 120: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvdivdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 224: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvmaxdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 232: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvmindp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 99: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvcmpeqdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 115: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvcmpgedp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 107: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvcmpgtdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        // FP single vector arithmetic
        case 64: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvaddsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 72: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvsubsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 80: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvmulsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 88: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvdivsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 192: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvmaxsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 200: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvminsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 67: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvcmpeqsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 83: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvcmpgesp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 75: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xvcmpgtsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        // Scalar FP double arithmetic
        case 32: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsadddp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 40: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xssubdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 48: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsmuldp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 56: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsdivdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 160: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsmaxdp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 168: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsmindp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        // Scalar FP single arithmetic
        case 0: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsaddsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 8: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xssubsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 16: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsmulsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 24: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xsdivsp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        // Scalar compare
        case 35: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xscmpudp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        case 43: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "xscmpodp", VSXname(xt3), VSXname(xa3), VSXname(xb3)); return buff;
        }

        // XX4-form (bits 4-5 = xo)
        int xx4_xo = XX4_XO(opcode);
        if (xx4_xo == 3) {
            int xt4 = XX4_T(opcode);
            int xa4 = XX4_A(opcode);
            int xb4 = XX4_B(opcode);
            int xc4 = XX4_C(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", "xxsel", VSXname(xt4), VSXname(xa4), VSXname(xb4), VSXname(xc4));
            return buff;
        }

        // XX2-form (9-bit xo, bits 2-10)
        int xx2_xo = XX2_XO(opcode);
        int xt2 = XX2_T(opcode);
        int xb2 = XX2_B(opcode);
        switch(xx2_xo) {
        // Byte-reverse
        case 471: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xxbrh", VSXname(xt2), VSXname(xb2)); return buff;
        case 475: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xxbrw", VSXname(xt2), VSXname(xb2)); return buff;
        case 503: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xxbrd", VSXname(xt2), VSXname(xb2)); return buff;
        case 507: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xxbrq", VSXname(xt2), VSXname(xb2)); return buff;
        // Splat word
        case 164: snprintf(buff, sizeof(buff), "%-15s %s, %s, %d", "xxspltw", VSXname(xt2), VSXname(xb2), XX2_UIM(opcode)); return buff;
        // Vector sqrt
        case 203: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvsqrtdp", VSXname(xt2), VSXname(xb2)); return buff;
        case 139: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvsqrtsp", VSXname(xt2), VSXname(xb2)); return buff;
        // Scalar sqrt
        case 11: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xssqrtsp", VSXname(xt2), VSXname(xb2)); return buff;
        case 75: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xssqrtdp", VSXname(xt2), VSXname(xb2)); return buff;
        // Reciprocal estimate
        case 154: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvresp", VSXname(xt2), VSXname(xb2)); return buff;
        case 138: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrsqrtesp", VSXname(xt2), VSXname(xb2)); return buff;
        // Scalar convert
        case 265: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvdpsp", VSXname(xt2), VSXname(xb2)); return buff;
        case 267: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvdpspn", VSXname(xt2), VSXname(xb2)); return buff;
        case 329: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvspdp", VSXname(xt2), VSXname(xb2)); return buff;
        case 331: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvspdpn", VSXname(xt2), VSXname(xb2)); return buff;
        case 376: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvsxddp", VSXname(xt2), VSXname(xb2)); return buff;
        case 312: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvsxdsp", VSXname(xt2), VSXname(xb2)); return buff;
        case 344: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvdpsxds", VSXname(xt2), VSXname(xb2)); return buff;
        case 88: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvdpsxws", VSXname(xt2), VSXname(xb2)); return buff;
        case 328: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvdpuxds", VSXname(xt2), VSXname(xb2)); return buff;
        case 360: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xscvuxddp", VSXname(xt2), VSXname(xb2)); return buff;
        // Vector convert
        case 393: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvdpsp", VSXname(xt2), VSXname(xb2)); return buff;
        case 457: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvspdp", VSXname(xt2), VSXname(xb2)); return buff;
        case 504: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvsxddp", VSXname(xt2), VSXname(xb2)); return buff;
        case 472: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvdpsxds", VSXname(xt2), VSXname(xb2)); return buff;
        case 216: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvdpsxws", VSXname(xt2), VSXname(xb2)); return buff;
        case 248: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvsxwdp", VSXname(xt2), VSXname(xb2)); return buff;
        case 152: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvspsxws", VSXname(xt2), VSXname(xb2)); return buff;
        case 184: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvsxwsp", VSXname(xt2), VSXname(xb2)); return buff;
        case 136: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvcvspuxws", VSXname(xt2), VSXname(xb2)); return buff;
        // Vector round double
        case 201: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrdpi", VSXname(xt2), VSXname(xb2)); return buff;
        case 217: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrdpiz", VSXname(xt2), VSXname(xb2)); return buff;
        case 235: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrdpic", VSXname(xt2), VSXname(xb2)); return buff;
        case 233: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrdpip", VSXname(xt2), VSXname(xb2)); return buff;
        case 249: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrdpim", VSXname(xt2), VSXname(xb2)); return buff;
        // Vector round single
        case 137: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrspi", VSXname(xt2), VSXname(xb2)); return buff;
        case 153: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrspiz", VSXname(xt2), VSXname(xb2)); return buff;
        case 171: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrspic", VSXname(xt2), VSXname(xb2)); return buff;
        case 169: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrspip", VSXname(xt2), VSXname(xb2)); return buff;
        case 185: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvrspim", VSXname(xt2), VSXname(xb2)); return buff;
        // Scalar round double
        case 73: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xsrdpi", VSXname(xt2), VSXname(xb2)); return buff;
        case 89: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xsrdpiz", VSXname(xt2), VSXname(xb2)); return buff;
        case 107: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xsrdpic", VSXname(xt2), VSXname(xb2)); return buff;
        case 105: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xsrdpip", VSXname(xt2), VSXname(xb2)); return buff;
        case 121: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xsrdpim", VSXname(xt2), VSXname(xb2)); return buff;
        // Vector abs/neg
        case 409: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvabssp", VSXname(xt2), VSXname(xb2)); return buff;
        case 441: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvnegsp", VSXname(xt2), VSXname(xb2)); return buff;
        case 473: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvabsdp", VSXname(xt2), VSXname(xb2)); return buff;
        case 505: snprintf(buff, sizeof(buff), "%-15s %s, %s", "xvnegdp", VSXname(xt2), VSXname(xb2)); return buff;
        }

        // XXSPLTIB: 60 | T(5) | 00 | IMM8(8) | 0101101000 | TX(1)
        // bits 1-10 = 0101101000 = 360, XX2_XO = bits 2-10 = 010110100 = 180
        if (xx2_xo == 180) {  // XXSPLTIB
            int imm8 = (opcode >> 11) & 0xFF;
            snprintf(buff, sizeof(buff), "%-15s %s, %d", "xxspltib", VSXname(xt2), imm8);
            return buff;
        }

        snprintf(buff, sizeof(buff), "%08X %-9s (xx3=%d,xx2=%d)", opcode, "vsx60???", xx3_xo, xx2_xo);
        return buff;
    }

    // ---- opcode 61: DQ-form (LXV, STXV) ----
    case 61: {
        int dq_xo = opcode & 0x7;  // bits 0-2
        int tx = (opcode >> 28) & 1;
        int t6 = (rt << 1) | tx;
        int dq = (int16_t)(opcode & 0xFFF0);
        switch(dq_xo) {
        case 1: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "lxv", VSXname(t6), dq, Rt[ra]); return buff;
        case 5: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stxv", VSXname(t6), dq, Rt[ra]); return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s (dq_xo=%d)", opcode, "dq???", dq_xo);
        return buff;
    }

    // ---- opcode 62: DS-form (STD, STDU) ----
    case 62: {
        int ds_xo = DS_XO(opcode);
        int offset = DS(opcode);
        switch(ds_xo) {
        case 0: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "std", Rt[rs], offset, Rt[ra]); return buff;
        case 1: snprintf(buff, sizeof(buff), "%-15s %s, %d(%s)", "stdu", Rt[rs], offset, Rt[ra]); return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s", opcode, "ds62???");
        return buff;
    }

    // ---- opcode 63: X/A-form double-precision FP and FPSCR ----
    case 63: {
        // A-form first (5-bit xo, bits 1-5)
        int a_xo = XO_A(opcode);
        int frt = rt, fra = ra, frb = rb, frc = FRC(opcode);
        switch(a_xo) {
        case 18: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fdiv." : "fdiv", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 20: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fsub." : "fsub", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 21: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fadd." : "fadd", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 22: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "fsqrt." : "fsqrt", Ft[frt], Ft[frb]); return buff;
        case 23: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", rc_bit ? "fsel." : "fsel", Ft[frt], Ft[fra], Ft[frc], Ft[frb]); return buff;
        case 25: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", rc_bit ? "fmul." : "fmul", Ft[frt], Ft[fra], Ft[frc]); return buff;
        case 28: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", rc_bit ? "fmsub." : "fmsub", Ft[frt], Ft[fra], Ft[frc], Ft[frb]); return buff;
        case 29: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", rc_bit ? "fmadd." : "fmadd", Ft[frt], Ft[fra], Ft[frc], Ft[frb]); return buff;
        case 30: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", rc_bit ? "fnmsub." : "fnmsub", Ft[frt], Ft[fra], Ft[frc], Ft[frb]); return buff;
        case 31: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s, %s", rc_bit ? "fnmadd." : "fnmadd", Ft[frt], Ft[fra], Ft[frc], Ft[frb]); return buff;
        }
        // X-form (10-bit xo)
        int x63_xo = XO_X(opcode);
        switch(x63_xo) {
        // FP compare
        case 0: {
            int bf = BF(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "fcmpu", CRnames[bf], Ft[ra], Ft[rb]);
            return buff;
        }
        case 32: {
            int bf = BF(opcode);
            snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "fcmpo", CRnames[bf], Ft[ra], Ft[rb]);
            return buff;
        }
        // FP move/convert
        case 72: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "fmr." : "fmr", Ft[frt], Ft[frb]); return buff;
        case 264: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fabs", Ft[frt], Ft[frb]); return buff;
        case 136: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fnabs", Ft[frt], Ft[frb]); return buff;
        case 40: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fneg", Ft[frt], Ft[frb]); return buff;
        case 8: snprintf(buff, sizeof(buff), "%-15s %s, %s, %s", "fcpsgn", Ft[frt], Ft[fra], Ft[frb]); return buff;
        case 12: snprintf(buff, sizeof(buff), "%-15s %s, %s", rc_bit ? "frsp." : "frsp", Ft[frt], Ft[frb]); return buff;
        // Convert
        case 814: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fctid", Ft[frt], Ft[frb]); return buff;
        case 815: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fctidz", Ft[frt], Ft[frb]); return buff;
        case 14: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fctiw", Ft[frt], Ft[frb]); return buff;
        case 15: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fctiwz", Ft[frt], Ft[frb]); return buff;
        case 846: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fcfid", Ft[frt], Ft[frb]); return buff;
        case 974: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fcfidu", Ft[frt], Ft[frb]); return buff;
        case 942: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fctidu", Ft[frt], Ft[frb]); return buff;
        case 943: snprintf(buff, sizeof(buff), "%-15s %s, %s", "fctiduz", Ft[frt], Ft[frb]); return buff;
        // Round
        case 392: snprintf(buff, sizeof(buff), "%-15s %s, %s", "frin", Ft[frt], Ft[frb]); return buff;
        case 424: snprintf(buff, sizeof(buff), "%-15s %s, %s", "friz", Ft[frt], Ft[frb]); return buff;
        case 456: snprintf(buff, sizeof(buff), "%-15s %s, %s", "frip", Ft[frt], Ft[frb]); return buff;
        case 488: snprintf(buff, sizeof(buff), "%-15s %s, %s", "frim", Ft[frt], Ft[frb]); return buff;
        // FPSCR
        case 583: snprintf(buff, sizeof(buff), "%-15s %s", rc_bit ? "mffs." : "mffs", Ft[frt]); return buff;
        case 711: {
            int flm = FLM(opcode);
            snprintf(buff, sizeof(buff), "%-15s 0x%02X, %s", rc_bit ? "mtfsf." : "mtfsf", flm, Ft[frb]);
            return buff;
        }
        case 134: {
            int bf = BF(opcode);
            int u = (opcode >> 12) & 0xF;
            snprintf(buff, sizeof(buff), "%-15s %d, %d", "mtfsfi", bf, u);
            return buff;
        }
        case 70: snprintf(buff, sizeof(buff), "%-15s %d", "mtfsb0", rt); return buff;
        case 38: snprintf(buff, sizeof(buff), "%-15s %d", "mtfsb1", rt); return buff;
        }
        snprintf(buff, sizeof(buff), "%08X %-9s (x63_xo=%d,a_xo=%d)", opcode, "fp63???", x63_xo, a_xo);
        return buff;
    }

    default:
        break;
    }

    // Unknown instruction
    snprintf(buff, sizeof(buff), "%08X ???", opcode);
    return buff;
}
