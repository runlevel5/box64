#ifndef __PPC64LE_EMITTER_H__
#define __PPC64LE_EMITTER_H__
/*
    PPC64LE Emitter

    PowerPC64 Little-Endian instruction encoding macros.
    Target ISA: POWER9 (ISA 3.0) minimum.

    Instruction formats:
    - I-form:   OPCD | LI(24) | AA | LK              (unconditional branch)
    - B-form:   OPCD | BO(5) | BI(5) | BD(14) | AA | LK  (conditional branch)
    - D-form:   OPCD | RT(5) | RA(5) | D/SI(16)       (load/store, addi, etc.)
    - DS-form:  OPCD | RT(5) | RA(5) | DS(14) | XO(2) (ld, std, lwa)
    - X-form:   OPCD | RT(5) | RA(5) | RB(5) | XO(10) | Rc  (indexed load/store, ALU)
    - XO-form:  OPCD | RT(5) | RA(5) | RB(5) | OE | XO(9) | Rc  (arithmetic)
    - M-form:   OPCD | RS(5) | RA(5) | RB/SH(5) | MB(5) | ME(5) | Rc  (rotate 32)
    - MD-form:  OPCD | RS(5) | RA(5) | sh(5) | mb/me(6) | XO(3) | sh(1) | Rc  (rotate 64)
    - XFX-form: OPCD | RT(5) | spr(10) | XO(10)       (mtspr, mfspr)
    - VX-form:  OPCD(6) | VRT(5) | VRA(5) | VRB(5) | XO(11)  (VMX)

    Note: PPC is big-endian at the instruction level — bit 0 is MSB.
    All encoding macros produce a uint32_t in native (little-endian) byte order
    since the assembler/CPU handles instruction endianness transparently in LE mode.
*/

#include "ppc64le_mapping.h"

// ===========================================================================
// Helper to load a 64-bit constant (up to 5 instructions)
// ===========================================================================
#define MOV64x(Rd, imm64) ppc64le_move64(dyn, ninst, Rd, (uint64_t)(imm64))
#define MOV32w(Rd, imm32) ppc64le_move32(dyn, ninst, Rd, (uint32_t)(imm32), 1)
#define MOV64xw(Rd, imm64)   if(rex.w) {MOV64x(Rd, imm64);} else {MOV32w(Rd, (uint32_t)(imm64));}
#define MOV64z(Rd, imm64)    if(rex.is32bits) {MOV32w(Rd, (uint32_t)(imm64));} else {MOV64x(Rd, imm64);}

// TABLE64 is defined per-pass in pass0.h/pass1.h/pass2.h/pass3.h and helper.h fallback
// Do NOT define it here — each pass has a different implementation

// ===========================================================================
// Instruction Encoding Macros — I-form (unconditional branch)
// ===========================================================================
// I-form: OPCD(6) | LI(24) | AA(1) | LK(1)
// LI is a signed 24-bit value, shifted left 2 to give ±32MB range
#define I_gen(opcd, li, aa, lk) \
    ((uint32_t)(opcd) << 26 | ((li) & 0x00FFFFFF) << 2 | ((aa) & 1) << 1 | ((lk) & 1))

// B — branch (relative)
#define B(offset)               EMIT(I_gen(18, ((offset) >> 2) & 0x00FFFFFF, 0, 0))
// BA — branch absolute
#define BA(addr)                EMIT(I_gen(18, ((addr) >> 2) & 0x00FFFFFF, 1, 0))
// BL — branch and link (relative, for calls)
#define BL(offset)              EMIT(I_gen(18, ((offset) >> 2) & 0x00FFFFFF, 0, 1))
// BLA — branch and link absolute
#define BLA(addr)               EMIT(I_gen(18, ((addr) >> 2) & 0x00FFFFFF, 1, 1))

// ===========================================================================
// B-form (conditional branch)
// ===========================================================================
// B-form: OPCD(6) | BO(5) | BI(5) | BD(14) | AA(1) | LK(1)
// BD is a signed 14-bit value, shifted left 2 to give ±32KB range
#define B_form_gen(opcd, bo, bi, bd, aa, lk) \
    ((uint32_t)(opcd) << 26 | ((bo) & 0x1F) << 21 | ((bi) & 0x1F) << 16 | (((bd) >> 2) & 0x3FFF) << 2 | ((aa) & 1) << 1 | ((lk) & 1))

// BC — branch conditional (relative)
#define BC(bo, bi, offset)      EMIT(B_form_gen(16, bo, bi, offset, 0, 0))
// BCL — branch conditional and link
#define BCL(bo, bi, offset)     EMIT(B_form_gen(16, bo, bi, offset, 0, 1))

// Simplified conditional branches using CR0
// BEQ: branch if CR0[EQ] is set
#define BEQ(offset)             BC(BO_TRUE, BI(CR0, CR_EQ), offset)
// BNE: branch if CR0[EQ] is clear
#define BNE(offset)             BC(BO_FALSE, BI(CR0, CR_EQ), offset)
// BLT: branch if CR0[LT] is set
#define BLT(offset)             BC(BO_TRUE, BI(CR0, CR_LT), offset)
// BGE: branch if CR0[LT] is clear
#define BGE(offset)             BC(BO_FALSE, BI(CR0, CR_LT), offset)
// BGT: branch if CR0[GT] is set
#define BGT(offset)             BC(BO_TRUE, BI(CR0, CR_GT), offset)
// BLE: branch if CR0[GT] is clear
#define BLE(offset)             BC(BO_FALSE, BI(CR0, CR_GT), offset)

// Conditional branches using arbitrary CR field
#define BEQ_CR(cr, offset)      BC(BO_TRUE, BI(cr, CR_EQ), offset)
#define BNE_CR(cr, offset)      BC(BO_FALSE, BI(cr, CR_EQ), offset)
#define BLT_CR(cr, offset)      BC(BO_TRUE, BI(cr, CR_LT), offset)
#define BGE_CR(cr, offset)      BC(BO_FALSE, BI(cr, CR_LT), offset)
#define BGT_CR(cr, offset)      BC(BO_TRUE, BI(cr, CR_GT), offset)
#define BLE_CR(cr, offset)      BC(BO_FALSE, BI(cr, CR_GT), offset)

// ===========================================================================
// XL-form (branch to LR/CTR)
// ===========================================================================
// XL-form: OPCD(6) | BO(5) | BI(5) | 000(3) | BH(2) | XO(10) | LK(1)
#define XL_gen(opcd, bo, bi, bh, xo, lk) \
    ((uint32_t)(opcd) << 26 | ((bo) & 0x1F) << 21 | ((bi) & 0x1F) << 16 | ((bh) & 0x3) << 11 | ((xo) & 0x3FF) << 1 | ((lk) & 1))

// BCLR — branch conditional to LR
#define BCLR(bo, bi, bh)        EMIT(XL_gen(19, bo, bi, bh, 16, 0))
// BCLRL — branch conditional to LR and link
#define BCLRL(bo, bi, bh)       EMIT(XL_gen(19, bo, bi, bh, 16, 1))
// BLR — branch to LR (unconditional)
#define BLR()                   BCLR(BO_ALWAYS, 0, 0)
// BLRL — branch to LR and link
#define BLRL()                  BCLRL(BO_ALWAYS, 0, 0)

// BCCTR — branch conditional to CTR
#define BCCTR(bo, bi, bh)       EMIT(XL_gen(19, bo, bi, bh, 528, 0))
// BCCTRL — branch conditional to CTR and link
#define BCCTRL(bo, bi, bh)      EMIT(XL_gen(19, bo, bi, bh, 528, 1))
// BCTR — branch to CTR (unconditional)
#define BCTR()                  BCCTR(BO_ALWAYS, 0, 0)
// BCTRL — branch to CTR and link
#define BCTRL()                 BCCTRL(BO_ALWAYS, 0, 0)

// Conditional branches to LR
#define BEQLR()                 BCLR(BO_TRUE, BI(CR0, CR_EQ), 0)
#define BNELR()                 BCLR(BO_FALSE, BI(CR0, CR_EQ), 0)
#define BLTLR()                 BCLR(BO_TRUE, BI(CR0, CR_LT), 0)
#define BGELR()                 BCLR(BO_FALSE, BI(CR0, CR_LT), 0)
#define BGTLR()                 BCLR(BO_TRUE, BI(CR0, CR_GT), 0)
#define BLELR()                 BCLR(BO_FALSE, BI(CR0, CR_GT), 0)

// ===========================================================================
// D-form (load/store with 16-bit displacement, addi, etc.)
// ===========================================================================
// D-form: OPCD(6) | RT/RS(5) | RA(5) | D/SI(16)
#define D_form_gen(opcd, rt, ra, d) \
    ((uint32_t)(opcd) << 26 | ((rt) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((d) & 0xFFFF))

// ADDI — RT = RA + SI (if RA=0, RT = SI)
#define ADDI(Rt, Ra, si)        EMIT(D_form_gen(14, Rt, Ra, (si) & 0xFFFF))
// LI — load immediate (pseudo: addi Rt, 0, si)
#define LI(Rt, si)              ADDI(Rt, 0, si)
// ADDIS — RT = RA + (SI << 16)
#define ADDIS(Rt, Ra, si)       EMIT(D_form_gen(15, Rt, Ra, (si) & 0xFFFF))
// LIS — load immediate shifted (pseudo: addis Rt, 0, si)
#define LIS(Rt, si)             ADDIS(Rt, 0, si)

// Arithmetic immediate
// ADDIC — RT = RA + SI, sets CA (XER carry)
#define ADDIC(Rt, Ra, si)       EMIT(D_form_gen(12, Rt, Ra, (si) & 0xFFFF))
// ADDIC. — RT = RA + SI, sets CA and CR0
#define ADDICd(Rt, Ra, si)      EMIT(D_form_gen(13, Rt, Ra, (si) & 0xFFFF))
// SUBFIC — RT = SI - RA (note: operand order!)
#define SUBFIC(Rt, Ra, si)      EMIT(D_form_gen(8, Rt, Ra, (si) & 0xFFFF))

// Compare immediate
// CMPI — compare RA with SI (signed), result in CR field
// D-form with BF(3) | 0 | L(1) in RT field
#define CMPI_gen(bf, l, ra, si) \
    D_form_gen(11, ((bf) & 0x7) << 2 | ((l) & 1), ra, (si) & 0xFFFF)
#define CMPWI(Ra, si)           EMIT(CMPI_gen(0, 0, Ra, si))
#define CMPWI_CR(cr, Ra, si)    EMIT(CMPI_gen(cr, 0, Ra, si))
#define CMPDI(Ra, si)           EMIT(CMPI_gen(0, 1, Ra, si))
#define CMPDI_CR(cr, Ra, si)    EMIT(CMPI_gen(cr, 1, Ra, si))

// CMPLI — compare RA with UI (unsigned), result in CR field
#define CMPLI_gen(bf, l, ra, ui) \
    D_form_gen(10, ((bf) & 0x7) << 2 | ((l) & 1), ra, (ui) & 0xFFFF)
#define CMPLWI(Ra, ui)          EMIT(CMPLI_gen(0, 0, Ra, ui))
#define CMPLWI_CR(cr, Ra, ui)   EMIT(CMPLI_gen(cr, 0, Ra, ui))
#define CMPLDI(Ra, ui)          EMIT(CMPLI_gen(0, 1, Ra, ui))
#define CMPLDI_CR(cr, Ra, ui)   EMIT(CMPLI_gen(cr, 1, Ra, ui))

// Logical immediate
// ORI — RA = RS | UI
#define ORI(Ra, Rs, ui)         EMIT(D_form_gen(24, Rs, Ra, (ui) & 0xFFFF))
// ORIS — RA = RS | (UI << 16)
#define ORIS(Ra, Rs, ui)        EMIT(D_form_gen(25, Rs, Ra, (ui) & 0xFFFF))
// NOP — ori 0,0,0
#define NOP()                   ORI(0, 0, 0)
// XORI — RA = RS ^ UI
#define XORI(Ra, Rs, ui)        EMIT(D_form_gen(26, Rs, Ra, (ui) & 0xFFFF))
// XORIS — RA = RS ^ (UI << 16)
#define XORIS(Ra, Rs, ui)       EMIT(D_form_gen(27, Rs, Ra, (ui) & 0xFFFF))
// ANDI. — RA = RS & UI, sets CR0
#define ANDId(Ra, Rs, ui)       EMIT(D_form_gen(28, Rs, Ra, (ui) & 0xFFFF))
// ANDI alias (LA64 compat) — note: PPC64LE andi. always sets CR0
#define ANDI(Ra, Rs, ui)        ANDId(Ra, Rs, ui)
// ANDIS. — RA = RS & (UI << 16), sets CR0
#define ANDISd(Ra, Rs, ui)      EMIT(D_form_gen(29, Rs, Ra, (ui) & 0xFFFF))

// Load byte/halfword/word (D-form)
// LBZ — load byte and zero
#define LBZ(Rt, offset, Ra)     EMIT(D_form_gen(34, Rt, Ra, (offset) & 0xFFFF))
// LBZU — load byte and zero with update
#define LBZU(Rt, offset, Ra)    EMIT(D_form_gen(35, Rt, Ra, (offset) & 0xFFFF))
// LHZ — load halfword and zero
#define LHZ(Rt, offset, Ra)     EMIT(D_form_gen(40, Rt, Ra, (offset) & 0xFFFF))
// LHA — load halfword algebraic (sign-extend)
#define LHA(Rt, offset, Ra)     EMIT(D_form_gen(42, Rt, Ra, (offset) & 0xFFFF))
// LWZ — load word and zero
#define LWZ(Rt, offset, Ra)     EMIT(D_form_gen(32, Rt, Ra, (offset) & 0xFFFF))

// Store byte/halfword/word (D-form)
// STB — store byte
#define STB(Rs, offset, Ra)     EMIT(D_form_gen(38, Rs, Ra, (offset) & 0xFFFF))
// STBU — store byte with update
#define STBU(Rs, offset, Ra)    EMIT(D_form_gen(39, Rs, Ra, (offset) & 0xFFFF))
// STH — store halfword
#define STH(Rs, offset, Ra)     EMIT(D_form_gen(44, Rs, Ra, (offset) & 0xFFFF))
// STW — store word
#define STW(Rs, offset, Ra)     EMIT(D_form_gen(36, Rs, Ra, (offset) & 0xFFFF))
// STWU — store word with update
#define STWU(Rs, offset, Ra)    EMIT(D_form_gen(37, Rs, Ra, (offset) & 0xFFFF))

// Load/store word algebraic (D-form, opcode 58/62)
// These use DS-form (14-bit displacement, must be multiple of 4)

// ===========================================================================
// DS-form (load/store doubleword, load word algebraic)
// ===========================================================================
// DS-form: OPCD(6) | RT/RS(5) | RA(5) | DS(14) | XO(2)
#define DS_form_gen(opcd, rt, ra, ds, xo) \
    ((uint32_t)(opcd) << 26 | ((rt) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((ds) & 0xFFFC) | ((xo) & 0x3))

// LD — load doubleword
#define LD(Rt, offset, Ra)      EMIT(DS_form_gen(58, Rt, Ra, (offset) & 0xFFFC, 0))
// LDU — load doubleword with update
#define LDU(Rt, offset, Ra)     EMIT(DS_form_gen(58, Rt, Ra, (offset) & 0xFFFC, 1))
// LWA — load word algebraic (sign-extend word to doubleword)
#define LWA(Rt, offset, Ra)     EMIT(DS_form_gen(58, Rt, Ra, (offset) & 0xFFFC, 2))

// STD — store doubleword
#define STD(Rs, offset, Ra)     EMIT(DS_form_gen(62, Rs, Ra, (offset) & 0xFFFC, 0))
// STDU — store doubleword with update
#define STDU(Rs, offset, Ra)    EMIT(DS_form_gen(62, Rs, Ra, (offset) & 0xFFFC, 1))

// ===========================================================================
// X-form (indexed load/store, register ALU, compare)
// ===========================================================================
// X-form: OPCD(6) | RT/RS/BF(5) | RA(5) | RB(5) | XO(10) | Rc(1)
#define X_form_gen(opcd, rt, ra, rb, xo, rc) \
    ((uint32_t)(opcd) << 26 | ((rt) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((rb) & 0x1F) << 11 | ((xo) & 0x3FF) << 1 | ((rc) & 1))

// Compare register
#define CMP_gen(bf, l, ra, rb)  X_form_gen(31, ((bf) & 0x7) << 2 | ((l) & 1), ra, rb, 0, 0)
#define CMPW(Ra, Rb)            EMIT(CMP_gen(0, 0, Ra, Rb))
#define CMPW_CR(cr, Ra, Rb)     EMIT(CMP_gen(cr, 0, Ra, Rb))
#define CMPD(Ra, Rb)            EMIT(CMP_gen(0, 1, Ra, Rb))
#define CMPD_CR(cr, Ra, Rb)     EMIT(CMP_gen(cr, 1, Ra, Rb))

#define CMPL_gen(bf, l, ra, rb) X_form_gen(31, ((bf) & 0x7) << 2 | ((l) & 1), ra, rb, 32, 0)
#define CMPLW(Ra, Rb)           EMIT(CMPL_gen(0, 0, Ra, Rb))
#define CMPLW_CR(cr, Ra, Rb)    EMIT(CMPL_gen(cr, 0, Ra, Rb))
#define CMPLD(Ra, Rb)           EMIT(CMPL_gen(0, 1, Ra, Rb))
#define CMPLD_CR(cr, Ra, Rb)    EMIT(CMPL_gen(cr, 1, Ra, Rb))

// Indexed load/store
// LBZX — load byte and zero indexed
#define LBZX(Rt, Ra, Rb)       EMIT(X_form_gen(31, Rt, Ra, Rb, 87, 0))
// LHZX — load halfword and zero indexed
#define LHZX(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 279, 0))
// LHAX — load halfword algebraic indexed
#define LHAX(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 343, 0))
// LWZX — load word and zero indexed
#define LWZX(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 23, 0))
// LWAX — load word algebraic indexed
#define LWAX(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 341, 0))
// LDX — load doubleword indexed
#define LDX(Rt, Ra, Rb)       EMIT(X_form_gen(31, Rt, Ra, Rb, 21, 0))

// STBX — store byte indexed
#define STBX(Rs, Ra, Rb)      EMIT(X_form_gen(31, Rs, Ra, Rb, 215, 0))
// STHX — store halfword indexed
#define STHX(Rs, Ra, Rb)      EMIT(X_form_gen(31, Rs, Ra, Rb, 407, 0))
// STWX — store word indexed
#define STWX(Rs, Ra, Rb)      EMIT(X_form_gen(31, Rs, Ra, Rb, 151, 0))
// STDX — store doubleword indexed
#define STDX(Rs, Ra, Rb)      EMIT(X_form_gen(31, Rs, Ra, Rb, 149, 0))

// Byte-reverse load/store
// LHBRX — load halfword byte-reverse indexed
#define LHBRX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 790, 0))
// LWBRX — load word byte-reverse indexed
#define LWBRX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 534, 0))
// LDBRX — load doubleword byte-reverse indexed
#define LDBRX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 532, 0))
// STHBRX — store halfword byte-reverse indexed
#define STHBRX(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 918, 0))
// STWBRX — store word byte-reverse indexed
#define STWBRX(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 662, 0))
// STDBRX — store doubleword byte-reverse indexed
#define STDBRX(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 660, 0))

// Extend / count / population
// EXTSB — extend sign byte (Rc=0)
#define EXTSB(Ra, Rs)          EMIT(X_form_gen(31, Rs, Ra, 0, 954, 0))
// EXTSBd — extend sign byte (Rc=1, sets CR0)
#define EXTSBd(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 954, 1))
// EXTSH — extend sign halfword
#define EXTSH(Ra, Rs)          EMIT(X_form_gen(31, Rs, Ra, 0, 922, 0))
// EXTSHd — extend sign halfword (Rc=1)
#define EXTSHd(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 922, 1))
// EXTSW — extend sign word
#define EXTSW(Ra, Rs)          EMIT(X_form_gen(31, Rs, Ra, 0, 986, 0))
// EXTSWd — extend sign word (Rc=1)
#define EXTSWd(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 986, 1))

// CNTLZW — count leading zeros word
#define CNTLZW(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 26, 0))
// CNTLZD — count leading zeros doubleword
#define CNTLZD(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 58, 0))
// CNTTZW — count trailing zeros word (POWER9)
#define CNTTZW(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 538, 0))
// CNTTZD — count trailing zeros doubleword (POWER9)
#define CNTTZD(Ra, Rs)         EMIT(X_form_gen(31, Rs, Ra, 0, 570, 0))
// POPCNTW — population count word
#define POPCNTW(Ra, Rs)        EMIT(X_form_gen(31, Rs, Ra, 0, 378, 0))
// POPCNTD — population count doubleword
#define POPCNTD(Ra, Rs)        EMIT(X_form_gen(31, Rs, Ra, 0, 506, 0))
// POPCNTB — population count bytes
#define POPCNTB(Ra, Rs)        EMIT(X_form_gen(31, Rs, Ra, 0, 122, 0))

// Logical register (X-form, opcode 31)
// AND
#define AND(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 28, 0))
#define ANDd(Ra, Rs, Rb)       EMIT(X_form_gen(31, Rs, Ra, Rb, 28, 1))
// ANDC — and with complement
#define ANDC(Ra, Rs, Rb)       EMIT(X_form_gen(31, Rs, Ra, Rb, 60, 0))
// OR
#define OR(Ra, Rs, Rb)         EMIT(X_form_gen(31, Rs, Ra, Rb, 444, 0))
#define ORd(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 444, 1))
// MR — move register (pseudo: or Ra, Rs, Rs)
#define MR(Ra, Rs)             OR(Ra, Rs, Rs)
// MV — alias for MR
#define MV(Rd, Rs)             MR(Rd, Rs)
// ORC — or with complement
#define ORC(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 412, 0))
// XOR
#define XOR(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 316, 0))
#define XORd(Ra, Rs, Rb)       EMIT(X_form_gen(31, Rs, Ra, Rb, 316, 1))
// NAND
#define NAND(Ra, Rs, Rb)       EMIT(X_form_gen(31, Rs, Ra, Rb, 476, 0))
// NOR
#define NOR(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 124, 0))
// NOT — pseudo: nor Ra, Rs, Rs
#define NOT(Ra, Rs)            NOR(Ra, Rs, Rs)
// EQV — equivalence (xnor)
#define EQV(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 284, 0))

// ===========================================================================
// XO-form (arithmetic register)
// ===========================================================================
// XO-form: OPCD(6) | RT(5) | RA(5) | RB(5) | OE(1) | XO(9) | Rc(1)
#define XO_form_gen(opcd, rt, ra, rb, oe, xo, rc) \
    ((uint32_t)(opcd) << 26 | ((rt) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((rb) & 0x1F) << 11 | ((oe) & 1) << 10 | ((xo) & 0x1FF) << 1 | ((rc) & 1))

// ADD — RT = RA + RB
#define ADD(Rt, Ra, Rb)        EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 266, 0))
#define ADDd(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 266, 1))
#define ADDo(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 1, 266, 0))

// ADDC — RT = RA + RB, set CA
#define ADDC(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 10, 0))
#define ADDCd(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 10, 1))

// ADDE — RT = RA + RB + CA
#define ADDE(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 138, 0))
#define ADDEd(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 138, 1))

// ADDZE — RT = RA + CA
#define ADDZE(Rt, Ra)          EMIT(XO_form_gen(31, Rt, Ra, 0, 0, 202, 0))

// SUBF — RT = RB - RA (subtract from: note operand order!)
#define SUBF(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 40, 0))
#define SUBFd(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 40, 1))
// SUB — pseudo: RT = Ra - Rb = subf Rt, Rb, Ra
#define SUB(Rt, Ra, Rb)        SUBF(Rt, Rb, Ra)
#define SUBd(Rt, Ra, Rb)       SUBFd(Rt, Rb, Ra)

// SUBFC — RT = RB - RA, set CA
#define SUBFC(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 8, 0))
#define SUBFCd(Rt, Ra, Rb)     EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 8, 1))

// SUBFE — RT = ~RA + RB + CA
#define SUBFE(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 136, 0))
#define SUBFEd(Rt, Ra, Rb)     EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 136, 1))

// SUBFZE — RT = ~RA + CA
#define SUBFZE(Rt, Ra)         EMIT(XO_form_gen(31, Rt, Ra, 0, 0, 200, 0))

// NEG — RT = -RA
#define NEG(Rt, Ra)            EMIT(XO_form_gen(31, Rt, Ra, 0, 0, 104, 0))
#define NEGd(Rt, Ra)           EMIT(XO_form_gen(31, Rt, Ra, 0, 0, 104, 1))

// MULLW — multiply low word
#define MULLW(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 235, 0))
#define MULLWd(Rt, Ra, Rb)     EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 235, 1))
// MULHW — multiply high word (signed)
#define MULHW(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 75, 0))
// MULHWU — multiply high word (unsigned)
#define MULHWU(Rt, Ra, Rb)     EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 11, 0))
// MULLD — multiply low doubleword
#define MULLD(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 233, 0))
#define MULLDd(Rt, Ra, Rb)     EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 233, 1))
// MULHD — multiply high doubleword (signed)
#define MULHD(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 73, 0))
// MULHDU — multiply high doubleword (unsigned)
#define MULHDU(Rt, Ra, Rb)     EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 9, 0))

// DIVW — divide word (signed)
#define DIVW(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 491, 0))
// DIVWU — divide word (unsigned)
#define DIVWU(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 459, 0))
// DIVD — divide doubleword (signed)
#define DIVD(Rt, Ra, Rb)       EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 489, 0))
// DIVDU — divide doubleword (unsigned)
#define DIVDU(Rt, Ra, Rb)      EMIT(XO_form_gen(31, Rt, Ra, Rb, 0, 457, 0))
// MODUW — modulo unsigned word (POWER9)
#define MODUW(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 267, 0))
// MODSW — modulo signed word (POWER9)
#define MODSW(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 779, 0))
// MODUD — modulo unsigned doubleword (POWER9)
#define MODUD(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 265, 0))
// MODSD — modulo signed doubleword (POWER9)
#define MODSD(Rt, Ra, Rb)      EMIT(X_form_gen(31, Rt, Ra, Rb, 777, 0))

// MADDLD — multiply-add low doubleword (POWER9, VA-form opcode 4, xo=51)
// RT = (RA * RB) + RC   (low 64 bits)
#define VA_form_gen(opcd, rt, ra, rb, rc, xo) \
    ((uint32_t)(opcd) << 26 | ((rt) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((rb) & 0x1F) << 11 | ((rc) & 0x1F) << 6 | ((xo) & 0x3F))
#define MADDLD(Rt, Ra, Rb, Rc) EMIT(VA_form_gen(4, Rt, Ra, Rb, Rc, 51))

// ===========================================================================
// Shift and Rotate instructions
// ===========================================================================

// M-form (32-bit rotate): OPCD(6) | RS(5) | RA(5) | SH/RB(5) | MB(5) | ME(5) | Rc(1)
#define M_form_gen(opcd, rs, ra, sh, mb, me, rc) \
    ((uint32_t)(opcd) << 26 | ((rs) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((sh) & 0x1F) << 11 | ((mb) & 0x1F) << 6 | ((me) & 0x1F) << 1 | ((rc) & 1))

// RLWINM — rotate left word immediate then AND with mask
#define RLWINM(Ra, Rs, sh, mb, me)  EMIT(M_form_gen(21, Rs, Ra, sh, mb, me, 0))
#define RLWINMd(Ra, Rs, sh, mb, me) EMIT(M_form_gen(21, Rs, Ra, sh, mb, me, 1))
// RLWIMI — rotate left word immediate then mask insert
#define RLWIMI(Ra, Rs, sh, mb, me)  EMIT(M_form_gen(20, Rs, Ra, sh, mb, me, 0))
// RLWNM — rotate left word then AND with mask (shift amount in RB)
#define RLWNM(Ra, Rs, Rb, mb, me)   EMIT(M_form_gen(23, Rs, Ra, Rb, mb, me, 0))

// Shift word (pseudo-ops using rlwinm)
// SLWI — shift left word immediate: rlwinm Ra, Rs, n, 0, 31-n
#define SLWI(Ra, Rs, n)         RLWINM(Ra, Rs, n, 0, 31-(n))
// SRWI — shift right word immediate: rlwinm Ra, Rs, 32-n, n, 31
#define SRWI(Ra, Rs, n)         RLWINM(Ra, Rs, 32-(n), n, 31)
// CLRLWI — clear left n bits: rlwinm Ra, Rs, 0, n, 31
#define CLRLWI(Ra, Rs, n)       RLWINM(Ra, Rs, 0, n, 31)
// CLRRWI — clear right n bits: rlwinm Ra, Rs, 0, 0, 31-n
#define CLRRWI(Ra, Rs, n)       RLWINM(Ra, Rs, 0, 0, 31-(n))
// ROTLWI — rotate left word immediate: rlwinm Ra, Rs, n, 0, 31
#define ROTLWI(Ra, Rs, n)       RLWINM(Ra, Rs, n, 0, 31)
// ROTRWI — rotate right word immediate: rlwinm Ra, Rs, 32-n, 0, 31
#define ROTRWI(Ra, Rs, n)       RLWINM(Ra, Rs, 32-(n), 0, 31)

// MD-form (64-bit rotate): OPCD(6) | RS(5) | RA(5) | sh[0:4](5) | mb/me(6) | XO(3) | sh[5](1) | Rc(1)
// sh is 6 bits: sh[5] is bit 1, sh[0:4] is bits 11-15
// MD form: the 6-bit mb/me field is stored with a scrambled bit order.
// The hardware interprets it as b = mb[5] || mb[0:4], where mb[0] is at
// Power bit 21 (C bit 10, the MSB of the 6-bit field in the instruction)
// and mb[5] is at Power bit 26 (C bit 5, the LSB).
// So to encode logical value 'mbe', we must place mbe[0:4] in the upper 5 bits
// and mbe[5] in the lower bit: raw = ((mbe & 0x1F) << 1) | ((mbe >> 5) & 1)
#define MD_form_gen(opcd, rs, ra, sh5, mbe, xo, rc) \
    ((uint32_t)(opcd) << 26 | ((rs) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | (((sh5) & 0x1F)) << 11 | ((((mbe) & 0x1F) << 1 | (((mbe) >> 5) & 1))) << 5 | ((xo) & 0x7) << 2 | ((((sh5) >> 5) & 1)) << 1 | ((rc) & 1))

// RLDICL — rotate left doubleword immediate then clear left
#define RLDICL(Ra, Rs, sh, mb)  EMIT(MD_form_gen(30, Rs, Ra, (sh) & 0x3F, (mb) & 0x3F, 0, 0))
#define RLDICLd(Ra, Rs, sh, mb) EMIT(MD_form_gen(30, Rs, Ra, (sh) & 0x3F, (mb) & 0x3F, 0, 1))
// RLDICR — rotate left doubleword immediate then clear right
#define RLDICR(Ra, Rs, sh, me)  EMIT(MD_form_gen(30, Rs, Ra, (sh) & 0x3F, (me) & 0x3F, 1, 0))
// RLDIC — rotate left doubleword immediate then clear
#define RLDIC(Ra, Rs, sh, mb)   EMIT(MD_form_gen(30, Rs, Ra, (sh) & 0x3F, (mb) & 0x3F, 2, 0))
// RLDIMI — rotate left doubleword immediate then mask insert
#define RLDIMI(Ra, Rs, sh, mb)  EMIT(MD_form_gen(30, Rs, Ra, (sh) & 0x3F, (mb) & 0x3F, 3, 0))

// MDS-form (doubleword rotate, variable shift amount in RB)
// MDS form: OPCD[0:5] | RS[6:10] | RA[11:15] | RB[16:20] | mb[21:26] | XO[27:30] | Rc[31]
// MDS form: same mb/me scrambling as MD form (6-bit field, b = mb[5]||mb[0:4])
#define MDS_form_gen(opcd, rs, ra, rb, mbe, xo, rc) \
    ((uint32_t)(opcd) << 26 | ((rs) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | ((rb) & 0x1F) << 11 | ((((mbe) & 0x1F) << 1 | (((mbe) >> 5) & 1))) << 5 | ((xo) & 0xF) << 1 | ((rc) & 1))

// RLDCL — rotate left doubleword then clear left (variable, Rb has rotate amount)
#define RLDCL(Ra, Rs, Rb, mb)   EMIT(MDS_form_gen(30, Rs, Ra, Rb, (mb) & 0x3F, 8, 0))
// RLDCR — rotate left doubleword then clear right (variable, Rb has rotate amount)
#define RLDCR(Ra, Rs, Rb, me)   EMIT(MDS_form_gen(30, Rs, Ra, Rb, (me) & 0x3F, 9, 0))

// Shift doubleword (pseudo-ops using rldicl/rldicr)
// SLDI — shift left doubleword immediate: rldicr Ra, Rs, n, 63-n
#define SLDI(Ra, Rs, n)         RLDICR(Ra, Rs, n, 63-(n))
// SRDI — shift right doubleword immediate: rldicl Ra, Rs, 64-n, n
#define SRDI(Ra, Rs, n)         RLDICL(Ra, Rs, 64-(n), n)
// CLRLDI — clear left n bits of doubleword: rldicl Ra, Rs, 0, n
#define CLRLDI(Ra, Rs, n)       RLDICL(Ra, Rs, 0, n)
// ROTLDI — rotate left doubleword immediate: rldicl Ra, Rs, n, 0
#define ROTLDI(Ra, Rs, n)       RLDICL(Ra, Rs, n, 0)

// Shift register (X-form, opcode 31)
// SLW — shift left word
#define SLW(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 24, 0))
// SRW — shift right word
#define SRW(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 536, 0))
// SRAW — shift right algebraic word
#define SRAW(Ra, Rs, Rb)       EMIT(X_form_gen(31, Rs, Ra, Rb, 792, 0))
#define SRAWd(Ra, Rs, Rb)      EMIT(X_form_gen(31, Rs, Ra, Rb, 792, 1))
// SRAWI — shift right algebraic word immediate
#define SRAWI(Ra, Rs, sh)      EMIT(X_form_gen(31, Rs, Ra, sh, 824, 0))
#define SRAWId(Ra, Rs, sh)     EMIT(X_form_gen(31, Rs, Ra, sh, 824, 1))
// SLD — shift left doubleword
#define SLD(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 27, 0))
// SRD — shift right doubleword
#define SRD(Ra, Rs, Rb)        EMIT(X_form_gen(31, Rs, Ra, Rb, 539, 0))
// SRAD — shift right algebraic doubleword
#define SRAD(Ra, Rs, Rb)       EMIT(X_form_gen(31, Rs, Ra, Rb, 794, 0))
#define SRADd(Ra, Rs, Rb)      EMIT(X_form_gen(31, Rs, Ra, Rb, 794, 1))

// SRADI — shift right algebraic doubleword immediate (XS-form)
// XS-form: like X-form but sh is 6 bits: sh[0:4] in bits 11-15, sh[5] in bit 1
#define XS_form_gen(opcd, rs, ra, sh5, xo, rc) \
    ((uint32_t)(opcd) << 26 | ((rs) & 0x1F) << 21 | ((ra) & 0x1F) << 16 | (((sh5) & 0x1F)) << 11 | ((xo) & 0x1FF) << 2 | ((((sh5) >> 5) & 1)) << 1 | ((rc) & 1))
#define SRADI(Ra, Rs, sh)      EMIT(XS_form_gen(31, Rs, Ra, (sh) & 0x3F, 413, 0))
#define SRADId(Ra, Rs, sh)     EMIT(XS_form_gen(31, Rs, Ra, (sh) & 0x3F, 413, 1))

// EXTSWSLI — extend sign word and shift left immediate (POWER9, XS-form, xo=445)
#define EXTSWSLI(Ra, Rs, sh)   EMIT(XS_form_gen(31, Rs, Ra, (sh) & 0x3F, 445, 0))

// ===========================================================================
// XFX-form (move to/from special registers)
// ===========================================================================
// XFX-form: OPCD(6) | RT(5) | spr(10) | XO(10) | 0
// Note: spr field is swapped: spr[5:9] | spr[0:4]
#define XFX_form_gen(opcd, rt, spr, xo) \
    ((uint32_t)(opcd) << 26 | ((rt) & 0x1F) << 21 | ((spr) & 0x3FF) << 11 | ((xo) & 0x3FF) << 1)

// SPR numbers (in the swapped encoding PPC uses: spr[5:9]||spr[0:4])
#define SPR_LR      (8 << 5 | 0)       // LR = SPR 8, encoded as 0x100
#define SPR_CTR     (9 << 5 | 0)       // CTR = SPR 9, encoded as 0x120
#define SPR_XER     (1 << 5 | 0)       // XER = SPR 1, encoded as 0x020
#define SPR_VRSAVE  (0 << 5 | 8)       // VRSAVE = SPR 256

// MFSPR — move from special purpose register
#define MFSPR(Rt, spr)         EMIT(XFX_form_gen(31, Rt, spr, 339))
// MTSPR — move to special purpose register
#define MTSPR(spr, Rs)         EMIT(XFX_form_gen(31, Rs, spr, 467))

// MFLR — move from LR
#define MFLR(Rt)               MFSPR(Rt, SPR_LR)
// MTLR — move to LR
#define MTLR(Rs)               MTSPR(SPR_LR, Rs)
// MFCTR — move from CTR
#define MFCTR(Rt)              MFSPR(Rt, SPR_CTR)
// MTCTR — move to CTR
#define MTCTR(Rs)              MTSPR(SPR_CTR, Rs)
// MFXER — move from XER
#define MFXER(Rt)              MFSPR(Rt, SPR_XER)
// MTXER — move to XER
#define MTXER(Rs)              MTSPR(SPR_XER, Rs)
// MFTB — move from time base (SPR 268, reads TBR)
#define SPR_TB                 (8 << 5 | 12)   // TBR = SPR 268
#define MFTB(Rt)               MFSPR(Rt, SPR_TB)

// MFCR — move from condition register (X-form, xo=19)
#define MFCR(Rt)               EMIT(X_form_gen(31, Rt, 0, 0, 19, 0))
// MTCRF — move to condition register fields (XFX-form variant)
// FXM is 8-bit field mask: bit 0 = CR0, bit 7 = CR7
#define MTCRF_gen(fxm, rs) \
    ((uint32_t)(31) << 26 | ((rs) & 0x1F) << 21 | ((fxm) & 0xFF) << 12 | (144) << 1)
#define MTCRF(fxm, Rs)         EMIT(MTCRF_gen(fxm, Rs))
// MTCR — move to full CR (pseudo: mtcrf 0xFF, Rs)
#define MTCR(Rs)               MTCRF(0xFF, Rs)

// MFOCRF — move from one condition register field
#define MFOCRF(Rt, fxm) \
    EMIT((uint32_t)(31) << 26 | ((Rt) & 0x1F) << 21 | 1 << 20 | ((fxm) & 0xFF) << 12 | (19) << 1)
// MTOCRF — move to one condition register field
#define MTOCRF(fxm, Rs) \
    EMIT((uint32_t)(31) << 26 | ((Rs) & 0x1F) << 21 | 1 << 20 | ((fxm) & 0xFF) << 12 | (144) << 1)

// ===========================================================================
// CR logical operations (XL-form, opcode 19)
// ===========================================================================
// CRAND — CR[BT] = CR[BA] & CR[BB]
#define CRAND(bt, ba, bb)      EMIT(XL_gen(19, bt, ba, bb, 257, 0))
// CRANDC — CR[BT] = CR[BA] & ~CR[BB]
#define CRANDC(bt, ba, bb)     EMIT(XL_gen(19, bt, ba, bb, 129, 0))
// CROR — CR[BT] = CR[BA] | CR[BB]
#define CROR(bt, ba, bb)       EMIT(XL_gen(19, bt, ba, bb, 449, 0))
// CRORC — CR[BT] = CR[BA] | ~CR[BB]
#define CRORC(bt, ba, bb)      EMIT(XL_gen(19, bt, ba, bb, 417, 0))
// CRXOR — CR[BT] = CR[BA] ^ CR[BB]
#define CRXOR(bt, ba, bb)      EMIT(XL_gen(19, bt, ba, bb, 193, 0))
// CRNAND — CR[BT] = ~(CR[BA] & CR[BB])
#define CRNAND(bt, ba, bb)     EMIT(XL_gen(19, bt, ba, bb, 225, 0))
// CRNOR — CR[BT] = ~(CR[BA] | CR[BB])
#define CRNOR(bt, ba, bb)      EMIT(XL_gen(19, bt, ba, bb, 33, 0))
// CREQV — CR[BT] = ~(CR[BA] ^ CR[BB])
#define CREQV(bt, ba, bb)      EMIT(XL_gen(19, bt, ba, bb, 289, 0))
// CRSET — set CR bit (pseudo: creqv bt, bt, bt)
#define CRSET(bt)              CREQV(bt, bt, bt)
// CRCLR — clear CR bit (pseudo: crxor bt, bt, bt)
#define CRCLR(bt)              CRXOR(bt, bt, bt)
// CRMOVE — copy CR bit (pseudo: cror bt, ba, ba)
#define CRMOVE(bt, ba)         CROR(bt, ba, ba)
// CRNOT — complement CR bit (pseudo: crnor bt, ba, ba)
#define CRNOT(bt, ba)          CRNOR(bt, ba, ba)

// ===========================================================================
// Synchronization / Memory barriers
// ===========================================================================
// SYNC — full memory barrier (X-form, opcode 31, xo=598)
#define SYNC()                 EMIT(X_form_gen(31, 0, 0, 0, 598, 0))
// LWSYNC — lightweight sync (load-load + store-store, like ARM64 dmb ish)
#define LWSYNC()               EMIT(X_form_gen(31, 1, 0, 0, 598, 0))
// ISYNC — instruction synchronization
#define ISYNC()                EMIT(XL_gen(19, 0, 0, 0, 150, 0))
// EIEIO — enforce in-order execution of I/O
#define EIEIO()                EMIT(X_form_gen(31, 0, 0, 0, 854, 0))

// DMB compatibility macros (match ARM64/LA64/RV64 naming used by strong memory model code)
// PPC64LE mapping: LWSYNC covers load-load, load-store, store-store ordering
// SYNC (heavyweight) covers all ordering including store-load
#define DMB_ISH()              LWSYNC()
#define DMB_ISHLD()            LWSYNC()
#define DMB_ISHST()            LWSYNC()

// Cache management
// DCBST — data cache block store
#define DCBST(Ra, Rb)          EMIT(X_form_gen(31, 0, Ra, Rb, 54, 0))
// DCBF — data cache block flush
#define DCBF(Ra, Rb)           EMIT(X_form_gen(31, 0, Ra, Rb, 86, 0))
// ICBI — instruction cache block invalidate
#define ICBI(Ra, Rb)           EMIT(X_form_gen(31, 0, Ra, Rb, 982, 0))

// ===========================================================================
// Load/store with reservation (atomics, LL/SC)
// ===========================================================================
// LBARX — load byte and reserve indexed (POWER8+)
#define LBARX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 52, 0))
// LHARX — load halfword and reserve indexed (POWER8+)
#define LHARX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 116, 0))
// LWARX — load word and reserve indexed
#define LWARX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 20, 0))
// LDARX — load doubleword and reserve indexed
#define LDARX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 84, 0))
// LQARX — load quadword and reserve indexed
#define LQARX(Rt, Ra, Rb)     EMIT(X_form_gen(31, Rt, Ra, Rb, 276, 0))

// STBCX. — store byte conditional indexed (always Rc=1)
#define STBCXd(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 694, 1))
// STHCX. — store halfword conditional indexed (always Rc=1)
#define STHCXd(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 726, 1))
// STWCX. — store word conditional indexed (always Rc=1)
#define STWCXd(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 150, 1))
// STDCX. — store doubleword conditional indexed (always Rc=1)
#define STDCXd(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 214, 1))
// STQCX. — store quadword conditional indexed (always Rc=1)
#define STQCXd(Rs, Ra, Rb)    EMIT(X_form_gen(31, Rs, Ra, Rb, 182, 1))

// ===========================================================================
// Floating-point load/store (D-form)
// ===========================================================================
// LFS — load floating-point single
#define LFS(Frt, offset, Ra)   EMIT(D_form_gen(48, Frt, Ra, (offset) & 0xFFFF))
// LFD — load floating-point double
#define LFD(Frt, offset, Ra)   EMIT(D_form_gen(50, Frt, Ra, (offset) & 0xFFFF))
// STFS — store floating-point single
#define STFS(Frs, offset, Ra)  EMIT(D_form_gen(52, Frs, Ra, (offset) & 0xFFFF))
// STFD — store floating-point double
#define STFD(Frs, offset, Ra)  EMIT(D_form_gen(54, Frs, Ra, (offset) & 0xFFFF))

// Indexed floating-point load/store (X-form)
// LFSX — load floating-point single indexed
#define LFSX(Frt, Ra, Rb)     EMIT(X_form_gen(31, Frt, Ra, Rb, 535, 0))
// LFDX — load floating-point double indexed
#define LFDX(Frt, Ra, Rb)     EMIT(X_form_gen(31, Frt, Ra, Rb, 599, 0))
// STFSX — store floating-point single indexed
#define STFSX(Frs, Ra, Rb)    EMIT(X_form_gen(31, Frs, Ra, Rb, 663, 0))
// STFDX — store floating-point double indexed
#define STFDX(Frs, Ra, Rb)    EMIT(X_form_gen(31, Frs, Ra, Rb, 727, 0))

// ===========================================================================
// VMX (Altivec/VSX) load/store — DQ-form (POWER9 lxv/stxv)
// ===========================================================================
// DQ-form: OPCD(6) | TX||T(5) | RA(5) | DQ(12) | XO(4)
// TX is the high bit of the 6-bit target register, T is the low 5 bits
#define DQ_form_gen(opcd, rt6, ra, dq, xo) \
    ((uint32_t)(opcd) << 26 | (((rt6) & 0x1F)) << 21 | ((ra) & 0x1F) << 16 | (((dq) >> 4) & 0xFFF) << 4 | ((xo) & 0xF) | ((((rt6) >> 5) & 1) << 3 & 0x8))

// LXV — load VSX vector (16 bytes, DQ-form, POWER9)
// Note: target is vs0-vs63, DQ must be multiple of 16
// Using opcode 61, xo = 1 for lxv
#define LXV(Vrt, offset, Ra)   EMIT(DQ_form_gen(61, Vrt, Ra, offset, 1))
// STXV — store VSX vector (16 bytes, DQ-form, POWER9)
// Using opcode 61, xo = 5 for stxv
#define STXV(Vrs, offset, Ra)  EMIT(DQ_form_gen(61, Vrs, Ra, offset, 5))

// Indexed VMX load/store (X-form, opcode 31)
// LXVX — load VSX vector indexed (POWER9)
#define LXVX(Vrt, Ra, Rb)     EMIT(X_form_gen(31, Vrt, Ra, Rb, 268, 0))
// STXVX — store VSX vector indexed (POWER9)
#define STXVX(Vrs, Ra, Rb)    EMIT(X_form_gen(31, Vrs, Ra, Rb, 396, 0))

// LVX — load vector indexed (VMX, old-style 128-bit load)
#define LVX(Vrt, Ra, Rb)      EMIT(X_form_gen(31, Vrt, Ra, Rb, 103, 0))
// STVX — store vector indexed (VMX, old-style 128-bit store)
#define STVX(Vrs, Ra, Rb)     EMIT(X_form_gen(31, Vrs, Ra, Rb, 231, 0))

// ===========================================================================
// Trap / debug
// ===========================================================================
// TWI — trap word immediate (D-form, opcode 3)
#define TWI(to, Ra, si)        EMIT(D_form_gen(3, to, Ra, (si) & 0xFFFF))
// TDI — trap doubleword immediate (D-form, opcode 2)
#define TDI(to, Ra, si)        EMIT(D_form_gen(2, to, Ra, (si) & 0xFFFF))
// TW — trap word (X-form, opcode 31, xo=4)
#define TW(to, Ra, Rb)         EMIT(X_form_gen(31, to, Ra, Rb, 4, 0))
// TD — trap doubleword (X-form, opcode 31, xo=68)
#define TD(to, Ra, Rb)         EMIT(X_form_gen(31, to, Ra, Rb, 68, 0))
// TRAP — unconditional trap (pseudo: tw 31, 0, 0)
#define TRAP()                 TW(31, 0, 0)

// UDF — undefined instruction (for marking dead code, same as trap)
#define UDF(...)               TRAP()

// ===========================================================================
// Floating-point arithmetic (A-form)
// ===========================================================================
// A-form: OPCD(6) | FRT(5) | FRA(5) | FRB(5) | FRC(5) | XO(5) | Rc(1)
#define A_form_gen(opcd, frt, fra, frb, frc, xo, rc) \
    ((uint32_t)(opcd) << 26 | ((frt) & 0x1F) << 21 | ((fra) & 0x1F) << 16 | ((frb) & 0x1F) << 11 | ((frc) & 0x1F) << 6 | ((xo) & 0x1F) << 1 | ((rc) & 1))

// FADD — FRT = FRA + FRB (double)
#define FADD(Frt, Fra, Frb)    EMIT(A_form_gen(63, Frt, Fra, Frb, 0, 21, 0))
// FADDS — FRT = FRA + FRB (single)
#define FADDS(Frt, Fra, Frb)   EMIT(A_form_gen(59, Frt, Fra, Frb, 0, 21, 0))
// FSUB — FRT = FRA - FRB (double)
#define FSUB(Frt, Fra, Frb)    EMIT(A_form_gen(63, Frt, Fra, Frb, 0, 20, 0))
// FSUBS — FRT = FRA - FRB (single)
#define FSUBS(Frt, Fra, Frb)   EMIT(A_form_gen(59, Frt, Fra, Frb, 0, 20, 0))
// FMUL — FRT = FRA * FRC (double)  NOTE: FRC is in the FRC field, FRB=0
#define FMUL(Frt, Fra, Frc)    EMIT(A_form_gen(63, Frt, Fra, 0, Frc, 25, 0))
// FMULS — FRT = FRA * FRC (single)
#define FMULS(Frt, Fra, Frc)   EMIT(A_form_gen(59, Frt, Fra, 0, Frc, 25, 0))
// FDIV — FRT = FRA / FRB (double)
#define FDIV(Frt, Fra, Frb)    EMIT(A_form_gen(63, Frt, Fra, Frb, 0, 18, 0))
// FDIVS — FRT = FRA / FRB (single)
#define FDIVS(Frt, Fra, Frb)   EMIT(A_form_gen(59, Frt, Fra, Frb, 0, 18, 0))
// FSQRT — FRT = sqrt(FRB) (double)
#define FSQRT(Frt, Frb)        EMIT(A_form_gen(63, Frt, 0, Frb, 0, 22, 0))
// FSQRTS — FRT = sqrt(FRB) (single)
#define FSQRTS(Frt, Frb)       EMIT(A_form_gen(59, Frt, 0, Frb, 0, 22, 0))
// FMADD — FRT = FRA * FRC + FRB (double)
#define FMADD(Frt, Fra, Frc, Frb)  EMIT(A_form_gen(63, Frt, Fra, Frb, Frc, 29, 0))
// FMSUB — FRT = FRA * FRC - FRB (double)
#define FMSUB(Frt, Fra, Frc, Frb)  EMIT(A_form_gen(63, Frt, Fra, Frb, Frc, 28, 0))
// FNMADD — FRT = -(FRA * FRC + FRB) (double)
#define FNMADD(Frt, Fra, Frc, Frb) EMIT(A_form_gen(63, Frt, Fra, Frb, Frc, 31, 0))
// FNMSUB — FRT = -(FRA * FRC - FRB) (double)
#define FNMSUB(Frt, Fra, Frc, Frb) EMIT(A_form_gen(63, Frt, Fra, Frb, Frc, 30, 0))
// FSEL — FRT = (FRA >= 0) ? FRC : FRB (double)
#define FSEL(Frt, Fra, Frc, Frb) EMIT(A_form_gen(63, Frt, Fra, Frb, Frc, 23, 0))

// ===========================================================================
// Floating-point conversion / rounding (X-form, opcode 63)
// ===========================================================================
// FRSP — round to single precision: FRT = (float)FRB
#define FRSP(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 12, 0))
// FCTID — convert to integer doubleword (current rounding mode)
#define FCTID(Frt, Frb)        EMIT(X_form_gen(63, Frt, 0, Frb, 814, 0))
// FCTIDZ — convert to integer doubleword with truncation (round toward zero)
#define FCTIDZ(Frt, Frb)       EMIT(X_form_gen(63, Frt, 0, Frb, 815, 0))
// FCTIW — convert to integer word (current rounding mode)
#define FCTIW(Frt, Frb)        EMIT(X_form_gen(63, Frt, 0, Frb, 14, 0))
// FCTIWZ — convert to integer word with truncation (round toward zero)
#define FCTIWZ(Frt, Frb)       EMIT(X_form_gen(63, Frt, 0, Frb, 15, 0))
// FCFID — convert from integer doubleword to double: FRT = (double)(int64_t)FRB
#define FCFID(Frt, Frb)        EMIT(X_form_gen(63, Frt, 0, Frb, 846, 0))
// FCFIDS — convert from integer doubleword to single: FRT = (float)(int64_t)FRB
#define FCFIDS(Frt, Frb)       EMIT(X_form_gen(59, Frt, 0, Frb, 846, 0))
// FCFIDU — convert from unsigned integer doubleword to double
#define FCFIDU(Frt, Frb)       EMIT(X_form_gen(63, Frt, 0, Frb, 974, 0))
// FCTIDU — convert to unsigned integer doubleword (current rounding)
#define FCTIDU(Frt, Frb)       EMIT(X_form_gen(63, Frt, 0, Frb, 942, 0))
// FCTIDUZ — convert to unsigned integer doubleword with truncation
#define FCTIDUZ(Frt, Frb)      EMIT(X_form_gen(63, Frt, 0, Frb, 943, 0))

// FABS — FRT = |FRB|
#define FABS(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 264, 0))
// FNABS — FRT = -|FRB|
#define FNABS(Frt, Frb)        EMIT(X_form_gen(63, Frt, 0, Frb, 136, 0))
// FNEG — FRT = -FRB
#define FNEG(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 40, 0))
// FMR — FRT = FRB (move register)
#define FMR(Frt, Frb)          EMIT(X_form_gen(63, Frt, 0, Frb, 72, 0))
// FCPSGN — FRT = copysign(FRB, FRA) (POWER6+)
#define FCPSGN(Frt, Fra, Frb)  EMIT(X_form_gen(63, Frt, Fra, Frb, 8, 0))
// FRIZ — round to integer toward zero (POWER5+)
#define FRIZ(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 424, 0))
// FRIN — round to integer toward nearest (POWER5+)
#define FRIN(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 392, 0))
// FRIP — round to integer toward +infinity (POWER5+)
#define FRIP(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 456, 0))
// FRIM — round to integer toward -infinity (POWER5+)
#define FRIM(Frt, Frb)         EMIT(X_form_gen(63, Frt, 0, Frb, 488, 0))

// ===========================================================================
// Floating-point compare (X-form, opcode 63)
// ===========================================================================
// FCMPU — compare unordered, result in CR field
// X-form: 63 | BF(3)||00 | FRA(5) | FRB(5) | XO=0 | 0
#define FCMPU(bf, Fra, Frb)    EMIT(X_form_gen(63, ((bf) & 0x7) << 2, Fra, Frb, 0, 0))
// FCMPO — compare ordered, result in CR field
#define FCMPO(bf, Fra, Frb)    EMIT(X_form_gen(63, ((bf) & 0x7) << 2, Fra, Frb, 32, 0))

// ===========================================================================
// Floating-point status and control register (FPSCR)
// ===========================================================================
// MFFS — move from FPSCR: FRT gets the full 64-bit FPSCR value
// X-form: 63 | FRT(5) | 00000 | 00000 | 583 | 0
#define MFFS(Frt)              EMIT(X_form_gen(63, Frt, 0, 0, 583, 0))
// MFFSd — MFFS and set CR1
#define MFFSd(Frt)             EMIT(X_form_gen(63, Frt, 0, 0, 583, 1))

// MTFSF — move to FPSCR fields: set FPSCR fields selected by FLM from FRB
// XFL-form: 63 | 0 | FLM(8) | 0 | FRB(5) | 711 | Rc
// FLM is an 8-bit mask, each bit selects one of 8 4-bit fields of FPSCR
#define MTFSF_gen(flm, frb, rc) \
    ((uint32_t)(63) << 26 | ((flm) & 0xFF) << 17 | ((frb) & 0x1F) << 11 | (711) << 1 | ((rc) & 1))
#define MTFSF(flm, Frb)       EMIT(MTFSF_gen(flm, Frb, 0))
#define MTFSFd(flm, Frb)      EMIT(MTFSF_gen(flm, Frb, 1))

// MTFSFI — move to FPSCR field immediate
// X-form variant: 63 | BF(3)||0||W(1) | 0000 | U(4)||0 | 134 | Rc
#define MTFSFI_gen(bf, u, rc) \
    ((uint32_t)(63) << 26 | ((bf) & 0x7) << 23 | (((u) & 0xF) << 1) << 11 | (134) << 1 | ((rc) & 1))
#define MTFSFI(bf, u)          EMIT(MTFSFI_gen(bf, u, 0))
// MTFSB0 — clear FPSCR bit
#define MTFSB0(bt)             EMIT(X_form_gen(63, bt, 0, 0, 70, 0))
// MTFSB1 — set FPSCR bit
#define MTFSB1(bt)             EMIT(X_form_gen(63, bt, 0, 0, 38, 0))

// ===========================================================================
// VSX instructions (XX3-form, XX2-form)
// ===========================================================================
// XX3-form: OPCD(6) | T(5) | A(5) | B(5) | XO(8) | AX(1) | BX(1) | TX(1)
// T/A/B are low 5 bits of 6-bit VSR index; TX/AX/BX are the high bits
#define XX3_form_gen(opcd, t, a, b, xo) \
    ((uint32_t)(opcd) << 26 | (((t) & 0x1F)) << 21 | (((a) & 0x1F)) << 16 | (((b) & 0x1F)) << 11 | ((xo) & 0xFF) << 3 | ((((a) >> 5) & 1)) << 2 | ((((b) >> 5) & 1)) << 1 | ((((t) >> 5) & 1)))

// XXLOR — VSX logical OR: XT = XA | XB (128-bit)
// Also used as register move: XXLOR(xt, xa, xa)
#define XXLOR(Xt, Xa, Xb)     EMIT(XX3_form_gen(60, Xt, Xa, Xb, 146))
// XXLXOR — VSX logical XOR: XT = XA ^ XB (128-bit)
// Also used to zero a register: XXLXOR(xt, xt, xt)
#define XXLXOR(Xt, Xa, Xb)    EMIT(XX3_form_gen(60, Xt, Xa, Xb, 154))
// XXLAND — VSX logical AND: XT = XA & XB (128-bit)
#define XXLAND(Xt, Xa, Xb)    EMIT(XX3_form_gen(60, Xt, Xa, Xb, 130))
// XXLANDC — VSX logical AND with complement: XT = XA & ~XB
#define XXLANDC(Xt, Xa, Xb)   EMIT(XX3_form_gen(60, Xt, Xa, Xb, 138))
// XXLNOR — VSX logical NOR: XT = ~(XA | XB)
#define XXLNOR(Xt, Xa, Xb)    EMIT(XX3_form_gen(60, Xt, Xa, Xb, 162))
// XXLEQV — VSX logical equivalence: XT = ~(XA ^ XB)
#define XXLEQV(Xt, Xa, Xb)    EMIT(XX3_form_gen(60, Xt, Xa, Xb, 186))
// XXLORC — VSX logical OR with complement: XT = XA | ~XB
#define XXLORC(Xt, Xa, Xb)    EMIT(XX3_form_gen(60, Xt, Xa, Xb, 170))
// XXLNAND — VSX logical NAND: XT = ~(XA & XB)
#define XXLNAND(Xt, Xa, Xb)   EMIT(XX3_form_gen(60, Xt, Xa, Xb, 178))

// XXMRGHD — VSX merge high doubleword
#define XXMRGHD(Xt, Xa, Xb)   EMIT(XX3_form_gen(60, Xt, Xa, Xb, 10))
// XXMRGLD — VSX merge low doubleword
#define XXMRGLD(Xt, Xa, Xb)   EMIT(XX3_form_gen(60, Xt, Xa, Xb, 42))
// XXPERMDI — VSX permute doubleword immediate
// XX3-form with DM(2) in bits 8-9 of xo field
#define XXPERMDI(Xt, Xa, Xb, dm) \
    EMIT(XX3_form_gen(60, Xt, Xa, Xb, (10 | (((dm) & 0x3) << 3))))

// XX2-form: OPCD(6) | T(5) | 00000 | B(5) | XO(9) | BX(1) | TX(1)
#define XX2_form_gen(opcd, t, b, xo) \
    ((uint32_t)(opcd) << 26 | (((t) & 0x1F)) << 21 | (((b) & 0x1F)) << 11 | ((xo) & 0x1FF) << 2 | ((((b) >> 5) & 1)) << 1 | ((((t) >> 5) & 1)))

// XSCVDPSP — VSX scalar convert double to single (result in word 0)
#define XSCVDPSP(Xt, Xb)      EMIT(XX2_form_gen(60, Xt, Xb, 265))
// XSCVSPDP — VSX scalar convert single (word 0) to double
#define XSCVSPDP(Xt, Xb)      EMIT(XX2_form_gen(60, Xt, Xb, 329))
// XSCVSXDDP — VSX scalar convert signed int64 to double
#define XSCVSXDDP(Xt, Xb)     EMIT(XX2_form_gen(60, Xt, Xb, 376))
// XSCVDPSXDS — VSX scalar convert double to signed int64 (truncation)
#define XSCVDPSXDS(Xt, Xb)    EMIT(XX2_form_gen(60, Xt, Xb, 344))
// XSCVDPUXDS — VSX scalar convert double to unsigned int64 (truncation)
#define XSCVDPUXDS(Xt, Xb)    EMIT(XX2_form_gen(60, Xt, Xb, 328))
// XSCVUXDDP — VSX scalar convert unsigned int64 to double
#define XSCVUXDDP(Xt, Xb)     EMIT(XX2_form_gen(60, Xt, Xb, 360))

// XVCVDPSP — VSX vector convert 2 doubles to 2 singles (packed)
#define XVCVDPSP(Xt, Xb)      EMIT(XX2_form_gen(60, Xt, Xb, 393))
// XVCVSPDP — VSX vector convert 2 singles to 2 doubles
#define XVCVSPDP(Xt, Xb)      EMIT(XX2_form_gen(60, Xt, Xb, 457))
// XVCVSXDDP — VSX vector convert 2 signed int64 to 2 doubles
#define XVCVSXDDP(Xt, Xb)     EMIT(XX2_form_gen(60, Xt, Xb, 504))
// XVCVDPSXDS — VSX vector convert 2 doubles to 2 signed int64
#define XVCVDPSXDS(Xt, Xb)    EMIT(XX2_form_gen(60, Xt, Xb, 472))

// XVABSDP — VSX vector absolute double
#define XVABSDP(Xt, Xb)       EMIT(XX2_form_gen(60, Xt, Xb, 473))
// XVNEGDP — VSX vector negate double
#define XVNEGDP(Xt, Xb)       EMIT(XX2_form_gen(60, Xt, Xb, 505))

// ===========================================================================
// VSX scalar FP compare (XX3-form, opcode 60)
// ===========================================================================
// XSCMPUDP — VSX scalar compare unordered double, result in CR field BF
#define XSCMPUDP(bf, Xa, Xb)  EMIT(XX3_form_gen(60, ((bf) & 0x7) << 2, Xa, Xb, 35))
// XSCMPODP — VSX scalar compare ordered double, result in CR field BF
#define XSCMPODP(bf, Xa, Xb)  EMIT(XX3_form_gen(60, ((bf) & 0x7) << 2, Xa, Xb, 43))

// ===========================================================================
// Convenience macros for the dynarec framework
// ===========================================================================

// Move register (already defined as MR/MV above)
// MOVx_REG — alias for compatibility with ARM64 macro names
#define MOVx_REG(Rd, Rs)       MR(Rd, Rs)

// ===========================================================================
// PUSH / POP x86-64 emulation stack macros
// ===========================================================================
// POP1 — pop 64-bit value from x86 RSP
#define POP1(reg)                               \
    do {                                        \
        LD(reg, 0, xRSP);                       \
        if (reg != xRSP) ADDI(xRSP, xRSP, 8);  \
    } while (0)

// PUSH1 — push 64-bit value to x86 RSP
#define PUSH1(reg)                              \
    do {                                        \
        STD(reg, -8, xRSP);                     \
        ADDI(xRSP, xRSP, -8);                   \
    } while (0)

// POP1_32 — pop 32-bit value from x86 RSP
#define POP1_32(reg)                            \
    do {                                        \
        LWZ(reg, 0, xRSP);                      \
        if (reg != xRSP) ADDI(xRSP, xRSP, 4);  \
    } while (0)

// PUSH1_32 — push 32-bit value to x86 RSP
#define PUSH1_32(reg)                           \
    do {                                        \
        STW(reg, -4, xRSP);                     \
        ADDI(xRSP, xRSP, -4);                   \
    } while (0)

// POP1_16 — pop 16-bit value from x86 RSP
#define POP1_16(reg)                            \
    do {                                        \
        LHZ(reg, 0, xRSP);                      \
        if (reg != xRSP) ADDI(xRSP, xRSP, 2);  \
    } while (0)

// PUSH1_16 — push 16-bit value to x86 RSP
#define PUSH1_16(reg)                           \
    do {                                        \
        STH(reg, -2, xRSP);                     \
        ADDI(xRSP, xRSP, -2);                   \
    } while (0)

// POP1z — pop 32 or 64 bits depending on rex.is32bits
#define POP1z(reg)          \
    do {                    \
        if (rex.is32bits) { \
            POP1_32(reg);   \
        } else {            \
            POP1(reg);      \
        }                   \
    } while (0)

// PUSH1z — push 32 or 64 bits depending on rex.is32bits
#define PUSH1z(reg)         \
    do {                    \
        if (rex.is32bits) { \
            PUSH1_32(reg);  \
        } else {            \
            PUSH1(reg);     \
        }                   \
    } while (0)

// ===========================================================================
// Branch convenience macros
// ===========================================================================
// BNEZ_safe — branch if register is non-zero, with near/far fallback
// PPC64LE conditional branches have ±32KB range, unconditional ±32MB.
// imm is byte offset from the start of this macro sequence to the target.
// Always emits exactly 3 instructions (CMPDI + cond_branch + NOP/B).
// Near case: CMPDI(+0) BNE(+4) NOP(+8)
// Far case:  CMPDI(+0) BEQ_skip(+4) B(+8)
#define BNEZ_safe(rj, imm)                              \
    do {                                                \
        CMPDI(rj, 0);                                   \
        if ((imm) > -0x7000 && (imm) < 0x7000) {        \
            BNE((imm) - 4);                              \
            NOP();                                       \
        } else {                                         \
            BEQ(2 * 4);                                  \
            B((imm) - 8);                                \
        }                                                \
    } while (0)

// BEZ_safe — branch if register is zero, with near/far fallback
#define BEZ_safe(rj, imm)                               \
    do {                                                \
        CMPDI(rj, 0);                                   \
        if ((imm) > -0x7000 && (imm) < 0x7000) {        \
            BEQ((imm) - 4);                              \
            NOP();                                       \
        } else {                                         \
            BNE(2 * 4);                                  \
            B((imm) - 8);                                \
        }                                                \
    } while (0)

// Aliases for GOCOND macro compatibility (B##EQZ##_safe, B##NEZ##_safe)
#define BEQZ_safe(rj, imm) BEZ_safe(rj, imm)

// IMARK / GETIP / MARKLOCK etc. will be defined in the pass headers

#endif // __PPC64LE_EMITTER_H__
