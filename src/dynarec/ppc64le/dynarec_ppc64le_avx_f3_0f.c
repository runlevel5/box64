#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_helper.h"

uintptr_t dynarec64_AVX_F3_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    int32_t i32, i32_;
    int cacheupd = 0;
    int v0, v1, v2;
    int q0, q1, q2;
    int d0, d1, d2;
    int s0;
    int64_t j64;
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
    MAYUSE(i32_);
    MAYUSE(u8);
    MAYUSE(s0);
    MAYUSE(cacheupd);
    MAYUSE(gdoffset);

    rex_t rex = vex.rex;

    switch (opcode) {
        case 0x10:
            INST_NAME("VMOVSS Gx, [Vx,] Ex");
            nextop = F8;
            if (MODREG) {
                // reg-reg: 3-operand form: Gx = Vx with low 32 bits replaced by Ex
                GETVYx(v1, 0);
                GETEYSS(v2, 0, 0);
                GETGYx_empty(v0);
                if (v0 != v1) {
                    XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                }
                // Insert Ex low float (LE word 0) into v0 LE word 0
                // For MODREG, v2 is a full 128-bit AVX reg; extract and insert word 0
                VINSERTW(VRREG(v0), VRREG(v2), 12);  // LE word 0 = BE byte offset 12
            } else {
                // mem: zero dest, load 32-bit float into low 32 bits
                GETGYx_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x11:
            INST_NAME("VMOVSS Ex, [Vx,] Gx");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                // reg-reg: 3-operand form: Ex = Vx with low 32 bits replaced by Gx
                GETVYx(v1, 0);
                GETEYx_empty(v2, 0);
                if (v2 != v1) {
                    XXLOR(VSXREG(v2), VSXREG(v1), VSXREG(v1));
                }
                VINSERTW(VRREG(v2), VRREG(v0), 12);  // LE word 0 = BE byte offset 12
            } else {
                // mem: store low 32 bits
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                MFVSRLD(x4, VSXREG(v0));  // x86 low 64 bits; float is in low 32
                STW(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x5A:
            INST_NAME("VCVTSS2SD Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                // v2 is a full VMX reg, float in LE word 0 (ISA word 3)
                XXSPLTW(VSXREG(d1), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                // v2 is a scratch with float already converted to double by GETEYSS
                XXLOR(VSXREG(d1), VSXREG(v2), VSXREG(v2));
            }
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert double result into low 64 bits: d1 has scalar in ISA dw0
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;

        case 0x5D:
            INST_NAME("VMINSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);    // ISA word 3 = LE word 0
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                // GETEYSS for memory already gives a double in ISA dw0
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // x86 MINSS semantics: if either is NaN or equal, return source (Ex)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);  // unordered → use source
            j64 = GETMARK2 - dyn->native_size;
            BLT(j64);  // dest < src → keep dest
            // dest >= src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            // Convert result back to float
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert float result into LE word 0 (BE byte offset 12)
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);  // after XSCVDPSPN, float is in ISA word 0 (BE byte 0)
            VINSERTW(VRREG(v0), VRREG(d1), 12);    // LE word 0 = BE byte offset 12
            break;

        case 0x5F:
            INST_NAME("VMAXSS Gx, Vx, Ex");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 0);
            GETGYx_empty(v0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);    // ISA word 3 = LE word 0
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // x86 MAXSS semantics: if either is NaN or equal, return source (Ex)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            j64 = GETMARK - dyn->native_size;
            BC(BO_TRUE, BI(CR0, CR_SO), j64);  // unordered → use source
            j64 = GETMARK2 - dyn->native_size;
            BGT(j64);  // dest > src → keep dest
            // dest <= src: take src
            MARK;
            XXLOR(VSXREG(d1), VSXREG(q0), VSXREG(q0));
            MARK2;
            // Convert result back to float
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert float result into LE word 0 (BE byte offset 12)
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x7E:
            INST_NAME("VMOVQ Gx, Ex");
            nextop = F8;
            GETGYx_empty(v0);
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                // Copy low 64 bits, zero upper 64 bits
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), 0, x4);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;

        case 0xC2:
            INST_NAME("VCMPSS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETVYx(v1, 0);
            GETEYSS(v2, 0, 1);
            GETGYx_empty(v0);
            u8 = F8;
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Vx scalar float to double
            XXSPLTW(VSXREG(d1), VSXREG(v1), 3);    // ISA word 3 = LE word 0
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex scalar float to double
            if (MODREG) {
                XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
            }
            // Compare: XSCMPUDP sets CR0: LT, GT, EQ, SO(unordered)
            XSCMPUDP(0, VSXREG(d1), VSXREG(q0));
            switch (u8 & 0xf) {
                case 0x00: // EQ_OQ (equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    break;
                case 0x01: // LT_OS (less than, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // LT bit
                    break;
                case 0x02: // LE_OS (less or equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);
                    break;
                case 0x03: // UNORD_Q (unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // SO bit
                    break;
                case 0x04: // NEQ_UQ (not equal, unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x05: // NLT_US (not less than = GE or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 1, 31, 31);  // LT bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x06: // NLE_US (not less or equal = GT or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 1, 31, 31);  // LT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);               // LE
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x07: // ORD_Q (ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 4, 31, 31);  // SO bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x08: // EQ_UQ (equal or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 3, 31, 31);  // EQ
                    RLWINM(x5, x5, 4, 31, 31);  // SO (unordered)
                    OR(x5, x5, x4);
                    break;
                case 0x09: // NGE_US (not GE = LT or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 2, 31, 31);  // GT bit
                    RLWINM(x5, x5, 3, 31, 31);  // EQ bit
                    OR(x5, x5, x4);               // GE = GT | EQ
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x0A: // NGT_US (not GT = LE or unordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 2, 31, 31);  // GT bit
                    XORI(x5, x5, 1);              // invert
                    break;
                case 0x0B: // FALSE_OQ
                    LI(x5, 0);
                    break;
                case 0x0C: // NEQ_OQ (not equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 4, 31, 31);  // SO (unordered)
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);               // EQ | unordered
                    XORI(x5, x5, 1);              // invert → NEQ AND ordered
                    break;
                case 0x0D: // GE_OS (greater or equal, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x4, x5, 2, 31, 31);  // GT
                    RLWINM(x5, x5, 3, 31, 31);  // EQ
                    OR(x5, x5, x4);
                    break;
                case 0x0E: // GT_OS (greater, ordered)
                    MFOCRF(x5, 0x80);
                    RLWINM(x5, x5, 2, 31, 31);  // GT bit
                    break;
                case 0x0F: // TRUE_UQ
                    LI(x5, 1);
                    break;
            }
            // x5 = 0 or 1; negate to get 0x00000000 or 0xFFFFFFFF mask
            NEG(x5, x5);   // 0 → 0, 1 → 0xFFFFFFFFFFFFFFFF
            // Copy Vx to dest (preserves upper bits)
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            // Insert 32-bit mask into LE word 0
            MTVSRD(VSXREG(d1), x5);
            VINSERTW(VRREG(v0), VRREG(d1), 12);  // LE word 0 = BE byte offset 12
            break;

        default:
            DEFAULT;
    }

    return addr;
}
