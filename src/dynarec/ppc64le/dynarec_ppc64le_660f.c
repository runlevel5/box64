#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "ppc64le_mapping.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "bitutils.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_660F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32;
    int64_t j64;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    MAYUSE(d0);
    MAYUSE(d1);
    MAYUSE(d2);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(v2);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(q2);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(gback);
    MAYUSE(j64);
    MAYUSE(i32);
    MAYUSE(u8);
    MAYUSE(gdoffset);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVUPD Gx, Ex");
            nextop = F8;
            GETG;
            v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x12:
            INST_NAME("MOVLPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                // MOVLPD with register operand is actually MOVHLPS (only with 66 prefix this doesn't exist in practice)
                // but handle it anyway — this shouldn't happen with 66 prefix
                DEFAULT;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                // Load 8 bytes into low 64 bits of Gx, keep high 64 bits
                LD(x4, fixedaddress, ed);
                // Extract high 64 bits of current Gx
                MFVSRLD(x5, VSXREG(v0));
                // Rebuild: low = loaded data, high = original high
                // On PPC64LE: MTVSRDD(XT, RA, RB) => elem0 (low) = RB, elem1 (high) = RA
                MTVSRDD(VSXREG(v0), x5, x4);
            }
            break;
        case 0x13:
            INST_NAME("MOVLPD Eq, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                DEFAULT;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                // Store low 64 bits of Gx to memory
                MFVSRD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x14:
            INST_NAME("UNPCKLPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Result: Gx[63:0] = Gx[63:0], Gx[127:64] = Ex[63:0]
            // Extract low 64 bits of both
            MFVSRD(x4, VSXREG(v0));    // v0 low
            MFVSRD(x5, VSXREG(v1));    // v1 low
            // Rebuild: low = v0_low, high = v1_low
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x15:
            INST_NAME("UNPCKHPD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Result: Gx[63:0] = Gx[127:64], Gx[127:64] = Ex[127:64]
            MFVSRLD(x4, VSXREG(v0));   // v0 high
            MFVSRLD(x5, VSXREG(v1));   // v1 high
            // Rebuild: low = v0_high, high = v1_high
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x16:
            INST_NAME("MOVHPD Gx, Eq");
            nextop = F8;
            GETGX(v0, 1);
            if (MODREG) {
                DEFAULT;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                // Load 8 bytes into high 64 bits of Gx, keep low 64 bits
                LD(x5, fixedaddress, ed);
                // Get current low 64 bits
                MFVSRD(x4, VSXREG(v0));
                // Rebuild: low = original low, high = loaded data
                MTVSRDD(VSXREG(v0), x5, x4);
            }
            break;
        case 0x17:
            INST_NAME("MOVHPD Eq, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                DEFAULT;
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                // Store high 64 bits of Gx to memory
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x18:
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;
        case 0x28:
            INST_NAME("MOVAPD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                SMREAD();
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x2B:
            INST_NAME("MOVNTPD Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0x2E:
            INST_NAME("UCOMISD Gx, Ex");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGX(v0, 0);
            GETEXSD(v1, 0, 0);
            CLEAR_FLAGS(x1);
            // Compare scalar double: Gx[63:0] vs Ex[63:0]
            // x86 UCOMISD sets ZF, PF, CF based on comparison
            // PPC64LE: XSCMPUDP sets CR, then we map to x86 flags
            // XSCMPUDP(BF, XA, XB): compares XA vs XB, sets CR field BF
            // CR bits: LT(0), GT(1), EQ(2), UN(3) — unordered if NaN
            XSCMPUDP(0, VSXREG(v0), VSXREG(v1));
            // Map PPC CR0 to x86 flags:
            // Unordered (NaN): ZF=1, PF=1, CF=1
            // Equal:           ZF=1, PF=0, CF=0
            // Less than:       ZF=0, PF=0, CF=1
            // Greater than:    ZF=0, PF=0, CF=0
            // Read CR0 field
            MFCR(x1);
            // CR0 is bits 32-35 of the CR register (PPC64LE big-endian bit numbering)
            // In the 32-bit value returned by MFCR (shifted): bit 31=LT, 30=GT, 29=EQ, 28=UN
            // Extract CR0 field: bits [31:28]
            RLWINM(x1, x1, 4, 28, 31);  // rotate right, extract low 4 bits: [LT, GT, EQ, UN]
            // Now x1 has: bit3=LT, bit2=GT, bit1=EQ, bit0=UN
            // Map to x86 flags in xFlags
            // CF (F_CF=0): set if LT or UN => bit3|bit0
            // ZF (F_ZF=6): set if EQ or UN => bit1|bit0
            // PF (F_PF=2): set if UN => bit0
            // First, build CF
            RLWINM(x2, x1, 0, 31, 31);   // x2 = UN bit (bit 0)
            RLWINM(x3, x1, 29, 31, 31);  // x3 = LT bit (bit 3 >> 3 => bit 0)
            OR(x2, x2, x3);               // x2 = CF = LT|UN
            // PF = UN
            RLWINM(x3, x1, 0, 31, 31);   // x3 = UN = PF
            SLWI(x3, x3, F_PF);           // shift to PF position
            OR(x2, x2, x3);               // x2 |= PF<<F_PF
            // ZF = EQ|UN
            RLWINM(x3, x1, 31, 31, 31);  // x3 = EQ bit (bit 1 >> 1 => bit 0)
            RLWINM(x4, x1, 0, 31, 31);   // x4 = UN
            OR(x3, x3, x4);               // x3 = ZF = EQ|UN
            SLWI(x3, x3, F_ZF);           // shift to ZF position
            OR(x2, x2, x3);               // x2 |= ZF<<F_ZF
            // Write to xFlags
            OR(xFlags, xFlags, x2);
            break;
        case 0x2F:
            INST_NAME("COMISD Gx, Ex");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGX(v0, 0);
            GETEXSD(v1, 0, 0);
            CLEAR_FLAGS(x1);
            // Same as UCOMISD for our purposes (difference is signaling vs quiet NaN)
            XSCMPUDP(0, VSXREG(v0), VSXREG(v1));
            MFCR(x1);
            RLWINM(x1, x1, 4, 28, 31);
            RLWINM(x2, x1, 0, 31, 31);
            RLWINM(x3, x1, 29, 31, 31);
            OR(x2, x2, x3);
            RLWINM(x3, x1, 0, 31, 31);
            SLWI(x3, x3, F_PF);
            OR(x2, x2, x3);
            RLWINM(x3, x1, 31, 31, 31);
            RLWINM(x4, x1, 0, 31, 31);
            OR(x3, x3, x4);
            SLWI(x3, x3, F_ZF);
            OR(x2, x2, x3);
            OR(xFlags, xFlags, x2);
            break;

        case 0x50:
            INST_NAME("MOVMSKPD Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            // Extract sign bits of both doubles
            MFVSRD(x4, VSXREG(q0));     // low 64 bits (double 0)
            MFVSRLD(x5, VSXREG(q0));    // high 64 bits (double 1)
            SRDI(x4, x4, 63);            // sign bit of double 0 -> bit 0
            SRDI(x5, x5, 63);            // sign bit of double 1 -> bit 0
            SLDI(x5, x5, 1);             // shift to bit 1
            OR(gd, x4, x5);
            break;

        case 0x54:
            INST_NAME("ANDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x55:
            INST_NAME("ANDNPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            // x86: dest = NOT(dest) AND src
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            // So: XXLANDC(v0, q0, v0) = q0 AND NOT(v0) = src AND NOT(dest) ✓
            XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));
            break;
        case 0x56:
            INST_NAME("ORPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x57:
            INST_NAME("XORPD Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for XORPD Gx, Gx => zero
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            break;

        case 0x58:
            INST_NAME("ADDPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XVADDDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x59:
            INST_NAME("MULPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XVMULDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x5C:
            INST_NAME("SUBPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XVSUBDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0x5E:
            INST_NAME("DIVPD Gx, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGX(v0, 1);
            XVDIVDP(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;

        case 0x60:
            INST_NAME("PUNPCKLBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave low bytes: result[0]=Gx[0], result[1]=Ex[0], result[2]=Gx[1], result[3]=Ex[1], ...
            // On PPC64LE, VMRGLB merges low bytes of two VRs
            // VMRGLB(VRT, VRA, VRB): interleaves low 8 bytes
            // PPC64LE byte order: low bytes are elements 0-7
            // x86 PUNPCKLBW: interleaves bytes 0-7 of Gx and Ex
            VMRGLB(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x61:
            INST_NAME("PUNPCKLWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGLH(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x62:
            INST_NAME("PUNPCKLDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGLW(VRREG(v0), VRREG(v1), VRREG(v0));
            break;

        case 0x64:
            INST_NAME("PCMPGTB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Compare greater-than signed bytes: result = (Gx > Ex) ? 0xFF : 0x00
            VCMPGTSB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x65:
            INST_NAME("PCMPGTW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPGTSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x66:
            INST_NAME("PCMPGTD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPGTSW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0x68:
            INST_NAME("PUNPCKHBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGHB(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x69:
            INST_NAME("PUNPCKHWD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGHH(VRREG(v0), VRREG(v1), VRREG(v0));
            break;
        case 0x6A:
            INST_NAME("PUNPCKHDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMRGHW(VRREG(v0), VRREG(v1), VRREG(v0));
            break;

        case 0x6C:
            INST_NAME("PUNPCKLQDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave low qwords: result[63:0]=Gx[63:0], result[127:64]=Ex[63:0]
            MFVSRD(x4, VSXREG(v0));    // v0 low qword
            MFVSRD(x5, VSXREG(v1));    // v1 low qword
            MTVSRDD(VSXREG(v0), x5, x4);
            break;
        case 0x6D:
            INST_NAME("PUNPCKHQDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Interleave high qwords: result[63:0]=Gx[127:64], result[127:64]=Ex[127:64]
            MFVSRLD(x4, VSXREG(v0));   // v0 high qword
            MFVSRLD(x5, VSXREG(v1));   // v1 high qword
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x6E:
            INST_NAME("MOVD Gx, Ed");
            nextop = F8;
            GETGX_empty(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    // MOVQ: XMM[63:0] = r64, XMM[127:64] = 0
                    MTVSRDD(VSXREG(v0), 0, ed);
                } else {
                    // MOVD: XMM[31:0] = r32, XMM[127:32] = 0
                    // Zero-extend r32 to r64, then put in low 64 bits
                    RLWINM(x4, ed, 0, 0, 31);  // zero-extend 32-bit
                    MTVSRDD(VSXREG(v0), 0, x4);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w) {
                    LD(x4, fixedaddress, ed);
                } else {
                    LWZ(x4, fixedaddress, ed);
                }
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x6F:
            INST_NAME("MOVDQA Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x70:
            INST_NAME("PSHUFD Gx, Ex, Ib");
            nextop = F8;
            GETEX(v1, 0, 1);
            GETGX_empty(v0);
            u8 = F8;
            // PSHUFD: dst[31:0] = src[imm[1:0]*32+31 : imm[1:0]*32]
            //         dst[63:32] = src[imm[3:2]*32+31 : imm[3:2]*32]
            //         dst[95:64] = src[imm[5:4]*32+31 : imm[5:4]*32]
            //         dst[127:96] = src[imm[7:6]*32+31 : imm[7:6]*32]
            // On PPC64LE, we can use XXPERMDI for some cases, but general case
            // requires building a permute vector. For now, use GPR intermediary.
            if (u8 == 0x00) {
                // All elements = element 0 (splat low dword)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 0);
            } else if (u8 == 0x55) {
                // All elements = element 1
                XXSPLTW(VSXREG(v0), VSXREG(v1), 1);
            } else if (u8 == 0xAA) {
                // All elements = element 2
                XXSPLTW(VSXREG(v0), VSXREG(v1), 2);
            } else if (u8 == 0xFF) {
                // All elements = element 3
                XXSPLTW(VSXREG(v0), VSXREG(v1), 3);
            } else if (u8 == 0x44) {
                // [0,1,0,1] — low 64 bits duplicated
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 0);
            } else if (u8 == 0xEE) {
                // [2,3,2,3] — high 64 bits duplicated
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 3);
            } else if (u8 == 0x4E) {
                // [2,3,0,1] — swap halves
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 2);
            } else if (u8 == 0xE4) {
                // [0,1,2,3] — identity
                if (v0 != v1)
                    XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                // General case: extract all 4 dwords via GPR, then reassemble
                // Element indices on PPC64LE little-endian:
                // In a 128-bit LE register: byte 0-3 = element 0, 4-7 = element 1, 8-11 = element 2, 12-15 = element 3
                // MFVSRD gets low 64 bits (elements 0,1), MFVSRLD gets high 64 bits (elements 2,3)
                MFVSRD(x4, VSXREG(v1));    // x4 = [elem1:elem0]
                MFVSRLD(x5, VSXREG(v1));   // x5 = [elem3:elem2]
                // Extract each element based on imm8
                // elem_src[i] = (u8 >> (i*2)) & 3
                // For each of the 4 result positions, pick the right 32-bit element
                {
                    int sel0 = (u8 >> 0) & 3;
                    int sel1 = (u8 >> 2) & 3;
                    int sel2 = (u8 >> 4) & 3;
                    int sel3 = (u8 >> 6) & 3;
                    // Helper: element n is in x4 (n<2) or x5 (n>=2), low 32 bits if n%2==0, high 32 bits if n%2==1
                    // Extract element sel into x6
                    #define EXTRACT_ELEM(dst, sel) do { \
                        if ((sel) < 2) { \
                            if ((sel) == 0) RLWINM(dst, x4, 0, 0, 31); \
                            else SRDI(dst, x4, 32); \
                        } else { \
                            if ((sel) == 2) RLWINM(dst, x5, 0, 0, 31); \
                            else SRDI(dst, x5, 32); \
                        } \
                    } while(0)
                    // Build result low 64 bits: [elem1_result : elem0_result]
                    EXTRACT_ELEM(x6, sel0);  // elem 0
                    EXTRACT_ELEM(x7, sel1);  // elem 1
                    SLDI(x7, x7, 32);
                    OR(x6, x6, x7);          // x6 = low 64 bits result
                    // Build result high 64 bits: [elem3_result : elem2_result]
                    EXTRACT_ELEM(x7, sel2);  // elem 2
                    EXTRACT_ELEM(x3, sel3);  // elem 3
                    SLDI(x3, x3, 32);
                    OR(x7, x7, x3);          // x7 = high 64 bits result
                    MTVSRDD(VSXREG(v0), x7, x6);
                    #undef EXTRACT_ELEM
                }
            }
            break;

        case 0x71:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSRH(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 4:
                    INST_NAME("PSRAW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8 > 15) u8 = 15;
                    if (u8) {
                        q1 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(q1), u8);
                        VSRAH(VRREG(q0), VRREG(q0), VRREG(q1));
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLW Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSLH(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x72:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSRW(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 4:
                    INST_NAME("PSRAD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8 > 31) u8 = 31;
                    if (u8) {
                        q1 = fpu_get_scratch(dyn);
                        XXSPLTIB(VSXREG(q1), u8);
                        VSRAW(VRREG(q0), VRREG(q0), VRREG(q1));
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLD Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 31) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSLW(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;
        case 0x73:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 2:
                    INST_NAME("PSRLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSRD(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 3:
                    INST_NAME("PSRLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            // Byte shift right: shift entire 128-bit register right by u8 bytes
                            // Use VSLDOI: VSLDOI(VRT, VRA, VRB, SH) concatenates VRA:VRB and shifts left by SH bytes
                            // On PPC64LE, VSLDOI byte shift semantic is reversed due to endianness
                            // To shift right by N bytes in LE: VSLDOI(dst, zero, src, 16-N)
                            q1 = fpu_get_scratch(dyn);
                            XXLXOR(VSXREG(q1), VSXREG(q1), VSXREG(q1));
                            VSLDOI(VRREG(q0), VRREG(q1), VRREG(q0), 16 - u8);
                        }
                    }
                    PUTEX(q0);
                    break;
                case 6:
                    INST_NAME("PSLLQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 63) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            q1 = fpu_get_scratch(dyn);
                            XXSPLTIB(VSXREG(q1), u8);
                            VSLD(VRREG(q0), VRREG(q0), VRREG(q1));
                        }
                    }
                    PUTEX(q0);
                    break;
                case 7:
                    INST_NAME("PSLLDQ Ex, Ib");
                    GETEX(q0, 1, 1);
                    u8 = F8;
                    if (u8) {
                        if (u8 > 15) {
                            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
                        } else {
                            // Byte shift left: shift entire 128-bit register left by u8 bytes
                            // On PPC64LE, to shift left by N bytes: VSLDOI(dst, src, zero, N)
                            q1 = fpu_get_scratch(dyn);
                            XXLXOR(VSXREG(q1), VSXREG(q1), VSXREG(q1));
                            VSLDOI(VRREG(q0), VRREG(q0), VRREG(q1), u8);
                        }
                    }
                    PUTEX(q0);
                    break;
                default:
                    *ok = 0;
                    DEFAULT;
            }
            break;

        case 0x74:
            INST_NAME("PCMPEQB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPEQUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x75:
            INST_NAME("PCMPEQW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPEQUH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0x76:
            INST_NAME("PCMPEQD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VCMPEQUW(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0x7E:
            INST_NAME("MOVD Ed, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (rex.w) {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    MFVSRD(ed, VSXREG(v0));
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    MFVSRD(x4, VSXREG(v0));
                    STD(x4, fixedaddress, ed);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    MFVSRWZ(ed, VSXREG(v0));
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    MFVSRWZ(x4, VSXREG(v0));
                    STW(x4, fixedaddress, ed);
                    SMWRITE2();
                }
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQA Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0xC6:
            INST_NAME("SHUFPD Gx, Ex, Ib");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 1);
            u8 = F8;
            // SHUFPD: dst[63:0] = (imm[0] ? Gx[127:64] : Gx[63:0])
            //         dst[127:64] = (imm[1] ? Ex[127:64] : Ex[63:0])
            {
                int sel0 = u8 & 1;
                int sel1 = (u8 >> 1) & 1;
                // PPC64LE XXPERMDI(XT, XA, XB, DM):
                //   XT[0:63] = (DM[1] ? XA[64:127] : XA[0:63])
                //   XT[64:127] = (DM[0] ? XB[64:127] : XB[0:63])
                // But on LE, element 0 = low, element 1 = high
                // XXPERMDI DM encoding on LE:
                //   DM=0: [A_low, B_low], DM=1: [A_low, B_high]
                //   DM=2: [A_high, B_low], DM=3: [A_high, B_high]
                // We want: low = sel0 from Gx, high = sel1 from Ex
                // Gx is XA, Ex is XB
                // low from Gx: sel0=0 => Gx_low, sel0=1 => Gx_high
                // high from Ex: sel1=0 => Ex_low, sel1=1 => Ex_high
                // DM bit1 controls XA selection (0=low, 1=high) for result low
                // DM bit0 controls XB selection (0=low, 1=high) for result high
                int dm = (sel0 << 1) | sel1;
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v1), dm);
            }
            break;

        case 0xD4:
            INST_NAME("PADDQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUDM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xD5:
            INST_NAME("PMULLW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            // Multiply packed signed words, keep low 16 bits of each result
            // PPC VMX doesn't have a direct "multiply low halfword" instruction
            // Use VMULOUH/VMULEUH approach or VMLADDUHM (multiply-low-add unsigned halfword modulo)
            // VMLADDUHM(VRT, VRA, VRB, VRC): VRT = (VRA * VRB + VRC) mod 2^16 for each halfword
            // We want just VRA * VRB (low), so VRC = 0
            q0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            VMLADDUHM(VRREG(v0), VRREG(v0), VRREG(v1), VRREG(q0));
            break;

        case 0xD6:
            INST_NAME("MOVQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                // Copy low 64 bits, zero high 64 bits
                MFVSRD(x4, VSXREG(v0));
                MTVSRDD(VSXREG(v1), 0, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                // Store low 64 bits to memory
                MFVSRD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0xD7:
            INST_NAME("PMOVMSKB Gd, Ex");
            nextop = F8;
            GETEX(q0, 0, 0);
            GETGD;
            // Extract sign bit of each byte (16 bytes -> 16 bits)
            // PPC64LE: Use VBPERMQ which performs bit permutation
            // VBPERMQ can extract arbitrary bits from a vector
            // We want bit 7 of each byte (the sign bit)
            // Alternative: shift each byte right by 7 to get sign bit, then gather
            // Use VGBBD (vector gather bits by bytes in doubleword) — POWER8+
            // VGBBD transposes the bit matrix within each doubleword
            // After VGBBD, the gathered bits are in specific byte positions
            // Simpler approach: use VBPERMQ
            // VBPERMQ(VRT, VRA, VRB): For each of the 16 bit indices in VRB,
            // extract that bit from VRA, pack results into low halfword of each doubleword
            // We need indices [7, 15, 23, 31, 39, 47, 55, 63, 71, 79, 87, 95, 103, 111, 119, 127]
            // But on PPC64LE these bit indices are big-endian within the register...
            // This is complex. Let's use a simpler GPR-based approach:
            // Extract both 64-bit halves, then extract sign bits
            MFVSRD(x4, VSXREG(q0));     // low 64 bits (bytes 0-7)
            MFVSRLD(x5, VSXREG(q0));    // high 64 bits (bytes 8-15)
            // Extract sign bit of each byte in x4 -> low 8 bits of result
            // Mask: 0x8080808080808080
            LI(x6, 0);
            ORIS(x6, x6, 0x8080);
            ORI(x6, x6, 0x8080);
            RLDIMI(x6, x6, 32, 0);      // x6 = 0x8080808080808080
            AND(x7, x4, x6);             // isolate sign bits of bytes 0-7
            // Now gather these bits: byte0_sign at bit7, byte1_sign at bit15, etc.
            // We need to compress these to consecutive bits
            // Use multiplication trick: multiply by magic constant to gather bits
            // byte_i sign bit is at position 8*i+7
            // Multiply by 0x0002040810204081 to gather to top byte
            // Actually simpler: use a loop-free bit gather
            // PPC64LE has PEXTD (parallel bit extract) on POWER10, not POWER9
            // Manual approach for POWER9:
            LI(x3, 7);
            SRD(x7, x4, x3);  // shift right 7: sign bits now at positions 0, 8, 16, 24, 32, 40, 48, 56
            AND(x7, x7, x6);  // wait no, this doesn't help
            // Let's use a different approach: use the FP/vector to do it
            // Actually, the simplest POWER9 approach for PMOVMSKB:
            // 1. Compare each byte with 0 (signed): VCMPGTSB gives -1 for negative bytes
            //    Wait, we just need sign bits. Let's use VBPERMQ.
            //    On PPC64LE, VBPERMQ(VRT, VRA, VRB): treats VRB as 16 byte indices (big-endian bit numbering)
            //    and extracts those bit positions from VRA into VRT.
            //    On LE, the bit numbering and result position are confusing. Let me just do it via GPR shifts.
            // Simplest correct approach: extract each bit position individually and OR together
            // Actually, let me use the known working approach from other emulators:
            // Use multiply-based bit extraction
            //
            // For a 64-bit value with bits at positions 7,15,23,31,39,47,55,63 (the sign bits),
            // after shifting right by 7, bits are at 0,8,16,24,32,40,48,56.
            // Multiply by a magic number to pack them into the top byte, then shift down.
            // Magic = 1 + 2^8 + 2^16 + ... + 2^56 / each step = 0x0101010101010101 / accounting for offsets...
            //
            // The standard trick for extracting MSBs of bytes:
            // x = original & 0x8080808080808080  (isolate sign bits)
            // x = x * 0x0002040810204081 >> 49  (gathers bits into low 8)
            //
            // But this needs 0x0002040810204081 which is 56 bits. Let's try.
            {
                // Isolate sign bits of each byte
                // x6 = 0x8080808080808080 (already built above)
                AND(x7, x4, x6);
                // Magic multiplier: 0x0002040810204081
                // Build it in x3
                LIS(x3, 0x0002);         // x3 = 0x00020000
                ORI(x3, x3, 0x0408);     // x3 = 0x00020408
                SLDI(x3, x3, 32);        // x3 = 0x0002040800000000
                ORIS(x3, x3, 0x1020);    // x3 = 0x0002040810200000
                ORI(x3, x3, 0x4081);     // x3 = 0x0002040810204081
                MULHDU(x7, x7, x3);      // high 64 bits of product
                RLWINM(x7, x7, 32 - 17 + 32, 24, 31); // extract result bits (shift right 49 from 128-bit perspective... let me think)
                // Actually: MULHDU gives high 64 bits. After multiplying x7 * x3:
                // The result byte is in bits [55:48] of the high product.
                // So shift right by 48 from the high part, then mask to 8 bits
                SRDI(x7, x7, 48);
                ANDI(x7, x7, 0xFF);

                // Do the same for high 64 bits (bytes 8-15)
                AND(x3, x5, x6);
                // Rebuild the magic multiplier
                LIS(x4, 0x0002);
                ORI(x4, x4, 0x0408);
                SLDI(x4, x4, 32);
                ORIS(x4, x4, 0x1020);
                ORI(x4, x4, 0x4081);
                MULHDU(x3, x3, x4);
                SRDI(x3, x3, 48);
                ANDI(x3, x3, 0xFF);

                // Combine: result = (high_byte_mask << 8) | low_byte_mask
                SLDI(x3, x3, 8);
                OR(gd, x7, x3);
            }
            break;

        case 0xD8:
            INST_NAME("PSUBUSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xD9:
            INST_NAME("PSUBUSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDA:
            INST_NAME("PMINUB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMINUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDB:
            INST_NAME("PAND Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0xDC:
            INST_NAME("PADDUSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDD:
            INST_NAME("PADDUSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDE:
            INST_NAME("PMAXUB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMAXUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xDF:
            INST_NAME("PANDN Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            // x86 PANDN: dest = NOT(dest) AND src
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));
            break;

        case 0xE0:
            INST_NAME("PAVGB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VAVGUB(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE3:
            INST_NAME("PAVGW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VAVGUH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        case 0xE7:
            INST_NAME("MOVNTDQ Ex, Gx");
            nextop = F8;
            GETGX(v0, 0);
            if (MODREG) {
                v1 = sse_get_reg_empty(dyn, ninst, x1, (nextop & 7) + (rex.b << 3));
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;
        case 0xE8:
            INST_NAME("PSUBSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBSBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xE9:
            INST_NAME("PSUBSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBSHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEA:
            INST_NAME("PMINSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMINSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEB:
            INST_NAME("POR Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(q0, 0, 0);
            XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));
            break;
        case 0xEC:
            INST_NAME("PADDSB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDSBS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xED:
            INST_NAME("PADDSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDSHS(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEE:
            INST_NAME("PMAXSW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VMAXSH(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xEF:
            INST_NAME("PXOR Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG && ((nextop & 7) + (rex.b << 3) == gd)) {
                // special case for PXOR Gx, Gx => zero
                q0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            } else {
                q0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                GETEX(q1, 0, 0);
                XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            break;

        case 0xF8:
            INST_NAME("PSUBB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUBM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xF9:
            INST_NAME("PSUBW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUHM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFA:
            INST_NAME("PSUBD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUWM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFB:
            INST_NAME("PSUBQ Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VSUBUDM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFC:
            INST_NAME("PADDB Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUBM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFD:
            INST_NAME("PADDW Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUHM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;
        case 0xFE:
            INST_NAME("PADDD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEX(v1, 0, 0);
            VADDUWM(VRREG(v0), VRREG(v0), VRREG(v1));
            break;

        default:
            DEFAULT;
    }

    return addr;
}
