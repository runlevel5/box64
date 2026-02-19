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

uintptr_t dynarec64_AVX_66_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
            INST_NAME("VMOVUPD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(v0, v1, 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;

        case 0x11:
            INST_NAME("VMOVUPD Ex, Gx");
            nextop = F8;
            GETGYxy(v0, 0);
            if (MODREG) {
                GETEYxy_empty(v1, 0);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x12:
            INST_NAME("VMOVLPD Gx, Vx, Eq");
            nextop = F8;
            if (MODREG) {
                // register form is undefined for VMOVLPD
                DEFAULT;
                return addr;
            }
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            // Load 8 bytes from memory into low 64 bits, keep Vx high 64 bits
            LD(x4, fixedaddress, ed);
            // Extract high 64 of Vx (x86 high = ISA dw0)
            MFVSRD(x5, VSXREG(v1));
            // Build: low = loaded data, high = Vx high
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x13:
            INST_NAME("VMOVLPD Eq, Gx");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
                return addr;
            }
            GETGYx(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            // Store low 64 bits of Gx (x86 low = ISA dw1)
            MFVSRLD(x4, VSXREG(v0));
            STD(x4, fixedaddress, ed);
            SMWRITE2();
            break;

        case 0x14:
            INST_NAME("VUNPCKLPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Result: [Vx low qword, Ex low qword]
            MFVSRLD(x4, VSXREG(v1));   // Vx x86 low (ISA dw1)
            MFVSRLD(x5, VSXREG(v2));   // Ex x86 low (ISA dw1)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x15:
            INST_NAME("VUNPCKHPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Result: [Vx high qword, Ex high qword]
            MFVSRD(x4, VSXREG(v1));   // Vx x86 high (ISA dw0)
            MFVSRD(x5, VSXREG(v2));   // Ex x86 high (ISA dw0)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x16:
            INST_NAME("VMOVHPD Gx, Vx, Eq");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
                return addr;
            }
            GETVYx(v1, 0);
            GETGYx_empty(v0);
            SMREAD();
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            // Load 8 bytes into high 64 bits, keep Vx low 64 bits
            LD(x5, fixedaddress, ed);
            MFVSRLD(x4, VSXREG(v1));   // Vx low 64 (ISA dw1)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x17:
            INST_NAME("VMOVHPD Eq, Gx");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
                return addr;
            }
            GETGYx(v0, 0);
            addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
            // Store high 64 bits of Gx (x86 high = ISA dw0)
            MFVSRD(x4, VSXREG(v0));
            STD(x4, fixedaddress, ed);
            SMWRITE2();
            break;

        case 0x28:
            INST_NAME("VMOVAPD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(v0, v1, 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;

        case 0x29:
            INST_NAME("VMOVAPD Ex, Gx");
            nextop = F8;
            GETGYxy(v0, 0);
            if (MODREG) {
                GETEYxy_empty(v1, 0);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x2E:
            // no special check...
        case 0x2F:
            if (opcode == 0x2F) {
                INST_NAME("VCOMISD Gx, Ex");
            } else {
                INST_NAME("VUCOMISD Gx, Ex");
            }
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGYx(v0, 0);
            // Extract Gx scalar double: x86 double is in ISA dw1 (low 64 bits)
            d0 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v0));
            MTVSRD(VSXREG(d0), x4);         // scalar double now in ISA dw0
            // Get Ex operand
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, fixedaddress, ed);
                MTVSRD(VSXREG(d1), x4);
            }
            CLEAR_FLAGS(x1);
            // Compare scalar double
            XSCMPUDP(0, VSXREG(d0), VSXREG(d1));
            // Map PPC CR0 to x86 flags
            MFCR(x1);
            RLWINM(x1, x1, 4, 28, 31);    // CR0 bits to low 4: [LT, GT, EQ, UN]
            // CF = LT|UN
            RLWINM(x2, x1, 0, 31, 31);    // UN bit
            RLWINM(x3, x1, 29, 31, 31);   // LT bit
            OR(x2, x2, x3);
            // PF = UN
            RLWINM(x3, x1, 0, 31, 31);
            SLWI(x3, x3, F_PF);
            OR(x2, x2, x3);
            // ZF = EQ|UN
            RLWINM(x3, x1, 31, 31, 31);   // EQ bit
            RLWINM(x4, x1, 0, 31, 31);    // UN bit
            OR(x3, x3, x4);
            SLWI(x3, x3, F_ZF);
            OR(x2, x2, x3);
            OR(xFlags, xFlags, x2);
            break;

        case 0x50:
            INST_NAME("VMOVMSKPD Gd, Ex");
            nextop = F8;
            GETEYxy(v0, 0, 0);
            GETGD;
            // Extract sign bits of both doubles
            MFVSRLD(x4, VSXREG(v0));    // x86 low 64 (double 0, ISA dw1)
            MFVSRD(x5, VSXREG(v0));     // x86 high 64 (double 1, ISA dw0)
            SRDI(x4, x4, 63);            // sign bit of double 0 -> bit 0
            SRDI(x5, x5, 63);            // sign bit of double 1 -> bit 0
            SLDI(x5, x5, 1);             // shift to bit 1
            OR(gd, x4, x5);
            break;

        case 0x51:
            INST_NAME("VSQRTPD Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            XVSQRTDP(VSXREG(v0), VSXREG(v1));
            break;

        case 0x54:
            INST_NAME("VANDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLAND(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0x55:
            INST_NAME("VANDNPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86 ANDNPD: dest = NOT(Vx) AND Ex
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            XXLANDC(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            break;

        case 0x56:
            INST_NAME("VORPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0x57:
            INST_NAME("VXORPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0x58:
            INST_NAME("VADDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVADDDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 63);
                VSLD(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x59:
            INST_NAME("VMULPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVMULDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 63);
                VSLD(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x5B:
            INST_NAME("VCVTPS2DQ Gx, Ex");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 0);
            u8 = sse_setround(dyn, ninst, x4, x5);
            XVRSPIC(VSXREG(v0), VSXREG(v1));    // round to integer floats using RN
            XVCVSPSXWS(VSXREG(v0), VSXREG(v0)); // truncate to int32
            x87_restoreround(dyn, ninst, u8);
            // Fix positive overflow: PPC gives 0x7FFFFFFF, x86 wants 0x80000000
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VSPLTISW(VRREG(q0), -1);
            VSPLTISW(VRREG(q1), 1);
            VSRW(VRREG(q0), VRREG(q0), VRREG(q1));     // q0 = 0x7FFFFFFF
            VCMPEQUW(VRREG(q0), VRREG(v0), VRREG(q0));  // mask where result == 0x7FFFFFFF
            VSUBUWM(VRREG(v0), VRREG(v0), VRREG(q0));   // 0x7FFFFFFF - (-1) = 0x80000000
            break;

        case 0x5C:
            INST_NAME("VSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVSUBDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 63);
                VSLD(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x5D:
            INST_NAME("VMINPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86 MINPD: return min, but if either is NaN or both equal, return src2 (Ex)
            // XVCMPGTDP(q0, Ex, Vx): mask = (Ex > Vx) ? -1 : 0
            //   Ex > Vx:  mask=-1, XXSEL picks B=Vx — correct (Vx is smaller)
            //   Ex <= Vx: mask=0,  XXSEL picks A=Ex — correct (Ex is smaller or equal)
            //   NaN:      mask=0,  XXSEL picks A=Ex — correct (x86 returns src2)
            q0 = fpu_get_scratch(dyn);
            XVCMPGTDP(VSXREG(q0), VSXREG(v2), VSXREG(v1));
            XXSEL(VSXREG(v0), VSXREG(v2), VSXREG(v1), VSXREG(q0));
            break;

        case 0x5E:
            INST_NAME("VDIVPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (!BOX64ENV(dynarec_fastnan)) {
                q0 = fpu_get_scratch(dyn);
                q1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));
            }
            XVDIVDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(q1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(q1), VSXREG(q0), VSXREG(q1));
                XXSPLTIB(VSXREG(q0), 63);
                VSLD(VRREG(q1), VRREG(q1), VRREG(q0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            }
            break;

        case 0x5F:
            INST_NAME("VMAXPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86 MAXPD: if either is NaN, return src2 (Ex)
            // XVCMPGTDP(q0, Vx, Ex): mask = (Vx > Ex) ? -1 : 0
            //   Vx > Ex: mask=-1, XXSEL picks B=Vx — correct (Vx is larger)
            //   Vx <= Ex or NaN: mask=0, XXSEL picks A=Ex — correct
            q0 = fpu_get_scratch(dyn);
            XVCMPGTDP(VSXREG(q0), VSXREG(v1), VSXREG(v2));
            XXSEL(VSXREG(v0), VSXREG(v2), VSXREG(v1), VSXREG(q0));
            break;

        case 0x60:
            INST_NAME("VPUNPCKLBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMRGLB(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x61:
            INST_NAME("VPUNPCKLWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMRGLH(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x62:
            INST_NAME("VPUNPCKLDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMRGLW(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x63:
            INST_NAME("VPACKSSWB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VPKSHSS(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x64:
            INST_NAME("VPCMPGTB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPGTSB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x65:
            INST_NAME("VPCMPGTW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPGTSH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x66:
            INST_NAME("VPCMPGTD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPGTSW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x67:
            INST_NAME("VPACKUSWB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VPKSHUS(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x68:
            INST_NAME("VPUNPCKHBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMRGHB(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x69:
            INST_NAME("VPUNPCKHWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMRGHH(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x6A:
            INST_NAME("VPUNPCKHDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMRGHW(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x6B:
            INST_NAME("VPACKSSDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VPKSWSS(VRREG(v0), VRREG(v2), VRREG(v1));
            break;

        case 0x6C:
            INST_NAME("VPUNPCKLQDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Result: [Vx low qword, Ex low qword]
            MFVSRLD(x4, VSXREG(v1));   // Vx x86 low (ISA dw1)
            MFVSRLD(x5, VSXREG(v2));   // Ex x86 low (ISA dw1)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x6D:
            INST_NAME("VPUNPCKHQDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Result: [Vx high qword, Ex high qword]
            MFVSRD(x4, VSXREG(v1));   // Vx x86 high (ISA dw0)
            MFVSRD(x5, VSXREG(v2));   // Ex x86 high (ISA dw0)
            MTVSRDD(VSXREG(v0), x5, x4);
            break;

        case 0x6E:
            INST_NAME("VMOVD Gx, Ed");
            nextop = F8;
            GETGYx_empty(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MTVSRDD(VSXREG(v0), 0, ed);
                } else {
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
            INST_NAME("VMOVDQA Gx, Ex");
            nextop = F8;
            if (MODREG) {
                GETGY_empty_EY_xy(v0, v1, 0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;

        case 0x70:
            INST_NAME("VPSHUFD Gx, Ex, Ib");
            nextop = F8;
            GETGY_empty_EY_xy(v0, v1, 1);
            u8 = F8;
            // PSHUFD: shuffle dwords based on imm8
            if (u8 == 0x00) {
                // All elements = element 0 (x86 dw0 = LE word 0 = XXSPLTW UIM 3)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 3);
            } else if (u8 == 0x55) {
                // All elements = element 1 (x86 dw1 = XXSPLTW UIM 2)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 2);
            } else if (u8 == 0xAA) {
                // All elements = element 2 (x86 dw2 = XXSPLTW UIM 1)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 1);
            } else if (u8 == 0xFF) {
                // All elements = element 3 (x86 dw3 = XXSPLTW UIM 0)
                XXSPLTW(VSXREG(v0), VSXREG(v1), 0);
            } else if (u8 == 0x44) {
                // [0,1,0,1] — low 64 bits duplicated
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 3);
            } else if (u8 == 0xEE) {
                // [2,3,2,3] — high 64 bits duplicated
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 0);
            } else if (u8 == 0x4E) {
                // [2,3,0,1] — swap halves
                XXPERMDI(VSXREG(v0), VSXREG(v1), VSXREG(v1), 2);
            } else if (u8 == 0xE4) {
                // [0,1,2,3] — identity
                if (v0 != v1)
                    XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                // General case: extract all 4 dwords via GPR, then reassemble
                MFVSRLD(x4, VSXREG(v1));   // x4 = [elem1:elem0] (x86 low = ISA dw1)
                MFVSRD(x5, VSXREG(v1));    // x5 = [elem3:elem2] (x86 high = ISA dw0)
                {
                    int sel0 = (u8 >> 0) & 3;
                    int sel1 = (u8 >> 2) & 3;
                    int sel2 = (u8 >> 4) & 3;
                    int sel3 = (u8 >> 6) & 3;
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
                    EXTRACT_ELEM(x6, sel0);
                    EXTRACT_ELEM(x7, sel1);
                    SLDI(x7, x7, 32);
                    OR(x6, x6, x7);
                    // Build result high 64 bits: [elem3_result : elem2_result]
                    EXTRACT_ELEM(x7, sel2);
                    EXTRACT_ELEM(x3, sel3);
                    SLDI(x3, x3, 32);
                    OR(x7, x7, x3);
                    MTVSRDD(VSXREG(v0), x7, x6);
                    #undef EXTRACT_ELEM
                }
            }
            break;

        case 0x74:
            INST_NAME("VPCMPEQB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPEQUB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x75:
            INST_NAME("VPCMPEQW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPEQUH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x76:
            INST_NAME("VPCMPEQD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPEQUW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x7C:
            INST_NAME("VHADDPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // q0 = {Ex_dw0, Vx_dw0} (x86 highs)
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v1), 0b00);
            // q1 = {Ex_dw1, Vx_dw1} (x86 lows)
            XXPERMDI(VSXREG(q1), VSXREG(v2), VSXREG(v1), 0b11);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XVCMPEQDP(VSXREG(d1), VSXREG(q1), VSXREG(q1));
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));
            }
            XVADDDP(VSXREG(v0), VSXREG(q0), VSXREG(q1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));
                XXSPLTIB(VSXREG(d0), 63);
                VSLD(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));
            }
            break;

        case 0x7D:
            INST_NAME("VHSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            // q0 = x86 lows (ISA dw1s), q1 = x86 highs (ISA dw0s)
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v1), 0b11);
            XXPERMDI(VSXREG(q1), VSXREG(v2), VSXREG(v1), 0b00);
            if (!BOX64ENV(dynarec_fastnan)) {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                XVCMPEQDP(VSXREG(d0), VSXREG(q0), VSXREG(q0));
                XVCMPEQDP(VSXREG(d1), VSXREG(q1), VSXREG(q1));
                XXLAND(VSXREG(d0), VSXREG(d0), VSXREG(d1));
            }
            XVSUBDP(VSXREG(v0), VSXREG(q0), VSXREG(q1));
            if (!BOX64ENV(dynarec_fastnan)) {
                XVCMPEQDP(VSXREG(d1), VSXREG(v0), VSXREG(v0));
                XXLANDC(VSXREG(d1), VSXREG(d0), VSXREG(d1));
                XXSPLTIB(VSXREG(d0), 63);
                VSLD(VRREG(d1), VRREG(d1), VRREG(d0));
                XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(d1));
            }
            break;

        case 0x7E:
            INST_NAME("VMOVD Ed, Gx");
            nextop = F8;
            GETGYx(v0, 0);
            if (rex.w) {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    MFVSRLD(ed, VSXREG(v0));
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    MFVSRLD(x4, VSXREG(v0));
                    STD(x4, fixedaddress, ed);
                    SMWRITE2();
                }
            } else {
                if (MODREG) {
                    ed = TO_NAT((nextop & 7) + (rex.b << 3));
                    MFVSRLD(ed, VSXREG(v0));
                    RLWINM(ed, ed, 0, 0, 31);  // zero-extend 32 bits
                } else {
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                    MFVSRLD(x4, VSXREG(v0));
                    STW(x4, fixedaddress, ed);
                    SMWRITE2();
                }
            }
            break;

        case 0x7F:
            INST_NAME("VMOVDQA Ex, Gx");
            nextop = F8;
            GETGYxy(v0, 0);
            if (MODREG) {
                GETEYxy_empty(v1, 0);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0xC2:
            INST_NAME("VCMPPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            switch (u8 & 0xf) {
                case 0x00: // EQ_OQ
                    XVCMPEQDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    break;
                case 0x01: // LT_OS
                    XVCMPGTDP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    break;
                case 0x02: // LE_OS
                    XVCMPGEDP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    break;
                case 0x03: // UNORD_Q
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x04: // NEQ_UQ
                    XVCMPEQDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x05: // NLT_US (GE or unordered)
                    XVCMPGTDP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x06: // NLE_US (GT or unordered)
                    XVCMPGEDP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x07: // ORD_Q
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(v0), VSXREG(q0), VSXREG(q1));
                    break;
                case 0x08: // EQ_UQ
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));       // both ordered
                    XVCMPEQDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));    // equal
                    XXLNOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));       // unordered
                    XXLOR(VSXREG(v0), VSXREG(v0), VSXREG(q0));        // equal OR unordered
                    break;
                case 0x09: // NGE_US (LT or unordered)
                    XVCMPGEDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x0A: // NGT_US (LE or unordered)
                    XVCMPGTDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    XXLNOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x0B: // FALSE_OQ
                    XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                    break;
                case 0x0C: // NEQ_OQ (not equal AND ordered)
                    q0 = fpu_get_scratch(dyn);
                    q1 = fpu_get_scratch(dyn);
                    XVCMPEQDP(VSXREG(q0), VSXREG(v1), VSXREG(v1));
                    XVCMPEQDP(VSXREG(q1), VSXREG(v2), VSXREG(v2));
                    XXLAND(VSXREG(q0), VSXREG(q0), VSXREG(q1));       // both ordered
                    XVCMPEQDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));    // equal
                    XXLANDC(VSXREG(v0), VSXREG(q0), VSXREG(v0));      // ordered AND NOT equal
                    break;
                case 0x0D: // GE_OS
                    XVCMPGEDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    break;
                case 0x0E: // GT_OS
                    XVCMPGTDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                    break;
                case 0x0F: // TRUE_UQ
                    VCMPEQUB(VRREG(v0), VRREG(v0), VRREG(v0));  // all ones
                    break;
            }
            break;

        case 0xC6:
            INST_NAME("VSHUFPD Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            {
                int sel0 = u8 & 1;
                int sel1 = (u8 >> 1) & 1;
                int dm = ((1 - sel1) << 1) | (1 - sel0);
                XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v1), dm);
            }
            break;

        case 0xD0:
            INST_NAME("VADDSUBPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            XVSUBDP(VSXREG(q0), VSXREG(v1), VSXREG(v2));   // q0 = Vx - Ex (both)
            XVADDDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));   // v0 = Vx + Ex (both)
            // x86 low = sub, x86 high = add => ISA dw0=add(v0), ISA dw1=sub(q0)
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(q0), 0b01);
            break;

        case 0xD1:
            INST_NAME("VPSRLW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v2));
            LI(x5, 15);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v2), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTH(VRREG(q0), VRREG(q0), 7);
            VSRH(VRREG(v0), VRREG(v1), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xD2:
            INST_NAME("VPSRLD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v2));
            LI(x5, 31);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v2), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTW(VRREG(q0), VRREG(q0), 3);
            VSRW(VRREG(v0), VRREG(v1), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xD3:
            INST_NAME("VPSRLQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            LI(x5, 63);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v2), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSRD(VRREG(v0), VRREG(v1), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xD4:
            INST_NAME("VPADDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDUDM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xD5:
            INST_NAME("VPMULLW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));
            VMLADDUHM(VRREG(v0), VRREG(v1), VRREG(v2), VRREG(q0));
            break;

        case 0xD6:
            INST_NAME("VMOVQ Ex, Gx");
            nextop = F8;
            GETGYx(v0, 0);
            if (MODREG) {
                GETEYx_empty(v1, 0);
                // Copy low 64 bits, zero high 64 bits
                MFVSRLD(x4, VSXREG(v0));
                MTVSRDD(VSXREG(v1), 0, x4);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                MFVSRLD(x4, VSXREG(v0));
                STD(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0xD8:
            INST_NAME("VPSUBUSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBUBS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xD9:
            INST_NAME("VPSUBUSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBUHS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xDA:
            INST_NAME("VPMINUB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINUB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xDB:
            INST_NAME("VPAND Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLAND(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0xDC:
            INST_NAME("VPADDUSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDUBS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xDD:
            INST_NAME("VPADDUSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDUHS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xDE:
            INST_NAME("VPMAXUB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXUB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xDF:
            INST_NAME("VPANDN Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // x86 PANDN: dest = NOT(Vx) AND Ex
            // PPC XXLANDC(T, A, B) = A AND NOT(B)
            XXLANDC(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            break;

        case 0xE0:
            INST_NAME("VPAVGB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAVGUB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xE1:
            INST_NAME("VPSRAW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            // Arithmetic right shift: clamp count to 15 (all sign bits)
            MFVSRLD(x4, VSXREG(v2));
            CMPLDI(x4, 15);
            LI(x5, 15);
            ISEL(x4, x4, x5, 0);    // x4 = (x4 < 15) ? x4 : 15
            // Splat to all halfword lanes
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTH(VRREG(q0), VRREG(q0), 7);
            VSRAH(VRREG(v0), VRREG(v1), VRREG(q0));
            break;

        case 0xE2:
            INST_NAME("VPSRAD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            // Clamp count to 31
            MFVSRLD(x4, VSXREG(v2));
            CMPLDI(x4, 31);
            LI(x5, 31);
            ISEL(x4, x4, x5, 0);    // x4 = (x4 < 31) ? x4 : 31
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTW(VRREG(q0), VRREG(q0), 3);
            VSRAW(VRREG(v0), VRREG(v1), VRREG(q0));
            break;

        case 0xE3:
            INST_NAME("VPAVGW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VAVGUH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xE4:
            INST_NAME("VPMULHUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            {
                int d2 = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                VMULEUH(VRREG(q0), VRREG(v1), VRREG(v2));
                VMULOUH(VRREG(q1), VRREG(v1), VRREG(v2));
                XXSPLTIB(VSXREG(d2), 16);
                VSRW(VRREG(q0), VRREG(q0), VRREG(d2));
                VSRW(VRREG(q1), VRREG(q1), VRREG(d2));
                VMRGLW(VRREG(d2), VRREG(q0), VRREG(q1));
                VMRGHW(VRREG(d3), VRREG(q0), VRREG(q1));
                VPKUWUM(VRREG(v0), VRREG(d3), VRREG(d2));
            }
            break;

        case 0xE5:
            INST_NAME("VPMULHW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            {
                int d2 = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                VMULESH(VRREG(q0), VRREG(v1), VRREG(v2));
                VMULOSH(VRREG(q1), VRREG(v1), VRREG(v2));
                XXSPLTIB(VSXREG(d2), 16);
                VSRAW(VRREG(q0), VRREG(q0), VRREG(d2));
                VSRAW(VRREG(q1), VRREG(q1), VRREG(d2));
                VMRGLW(VRREG(d2), VRREG(q0), VRREG(q1));
                VMRGHW(VRREG(d3), VRREG(q0), VRREG(q1));
                VPKUWUM(VRREG(v0), VRREG(d3), VRREG(d2));
            }
            break;

        case 0xE8:
            INST_NAME("VPSUBSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBSBS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xE9:
            INST_NAME("VPSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBSHS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xEA:
            INST_NAME("VPMINSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINSH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xEB:
            INST_NAME("VPOR Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0xEC:
            INST_NAME("VPADDSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDSBS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xED:
            INST_NAME("VPADDSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDSHS(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xEE:
            INST_NAME("VPMAXSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXSH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xEF:
            INST_NAME("VPXOR Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            break;

        case 0xF1:
            INST_NAME("VPSLLW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v2));
            LI(x5, 15);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v2), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTH(VRREG(q0), VRREG(q0), 7);
            VSLH(VRREG(v0), VRREG(v1), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xF2:
            INST_NAME("VPSLLD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            MFVSRLD(x4, VSXREG(v2));
            LI(x5, 31);
            MTVSRDD(VSXREG(q1), x5, x5);
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v2), 3);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            MTVSRDD(VSXREG(q0), x4, x4);
            VSPLTW(VRREG(q0), VRREG(q0), 3);
            VSLW(VRREG(v0), VRREG(v1), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xF3:
            INST_NAME("VPSLLQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            XXPERMDI(VSXREG(q0), VSXREG(v2), VSXREG(v2), 3);
            LI(x4, 63);
            MTVSRDD(VSXREG(q1), x4, x4);
            VCMPGTUD(VRREG(q1), VRREG(q0), VRREG(q1));
            VNOR(VRREG(q1), VRREG(q1), VRREG(q1));
            VSLD(VRREG(v0), VRREG(v1), VRREG(q0));
            XXLAND(VSXREG(v0), VSXREG(v0), VSXREG(q1));
            break;

        case 0xF4:
            INST_NAME("VPMULUDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULOUW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xF5:
            INST_NAME("VPMADDWD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            q1 = fpu_get_scratch(dyn);
            VMULESH(VRREG(q0), VRREG(v1), VRREG(v2));
            VMULOSH(VRREG(q1), VRREG(v1), VRREG(v2));
            VADDUWM(VRREG(v0), VRREG(q0), VRREG(q1));
            break;

        case 0xF6:
            INST_NAME("VPSADBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            {
                // Process low qword
                MFVSRLD(x4, VSXREG(v1));
                MFVSRLD(x5, VSXREG(v2));
                LI(x3, 0);
                for (int byte_idx = 0; byte_idx < 8; byte_idx++) {
                    int shift = byte_idx * 8;
                    if (shift == 0) {
                        CLRLDI(x6, x4, 56);
                        CLRLDI(x7, x5, 56);
                    } else {
                        SRDI(x6, x4, shift);
                        CLRLDI(x6, x6, 56);
                        SRDI(x7, x5, shift);
                        CLRLDI(x7, x7, 56);
                    }
                    SUBF(x6, x7, x6);
                    SRADI(x7, x6, 63);
                    XOR(x6, x6, x7);
                    SUBF(x6, x7, x6);
                    ADD(x3, x3, x6);
                }
                MR(x1, x3);
                // Process high qword
                MFVSRD(x4, VSXREG(v1));
                MFVSRD(x5, VSXREG(v2));
                LI(x3, 0);
                for (int byte_idx = 0; byte_idx < 8; byte_idx++) {
                    int shift = byte_idx * 8;
                    if (shift == 0) {
                        CLRLDI(x6, x4, 56);
                        CLRLDI(x7, x5, 56);
                    } else {
                        SRDI(x6, x4, shift);
                        CLRLDI(x6, x6, 56);
                        SRDI(x7, x5, shift);
                        CLRLDI(x7, x7, 56);
                    }
                    SUBF(x6, x7, x6);
                    SRADI(x7, x6, 63);
                    XOR(x6, x6, x7);
                    SUBF(x6, x7, x6);
                    ADD(x3, x3, x6);
                }
                MTVSRDD(VSXREG(v0), x3, x1);
            }
            break;

        case 0xF8:
            INST_NAME("VPSUBB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBUBM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xF9:
            INST_NAME("VPSUBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBUHM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xFA:
            INST_NAME("VPSUBD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBUWM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xFB:
            INST_NAME("VPSUBQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VSUBUDM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xFC:
            INST_NAME("VPADDB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDUBM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xFD:
            INST_NAME("VPADDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDUHM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0xFE:
            INST_NAME("VPADDD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VADDUWM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        default:
            DEFAULT;
    }

    return addr;
}
