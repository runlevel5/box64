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

uintptr_t dynarec64_AVX_66_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x00:
            INST_NAME("VPSHUFB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                // VPERM uses big-endian byte numbering; XOR low nibble with 0x0F to convert
                XXSPLTIB(VSXREG(d2_tmp), 0x0F);
                VXOR(VRREG(d0), VRREG(v2), VRREG(d2_tmp));
            }
            VPERM(VRREG(d1), VRREG(v1), VRREG(v1), VRREG(d0));
            // Zero where Ex[i] has bit 7 set
            VSPLTISB(VRREG(d0), 7);
            VSRAB(VRREG(d0), VRREG(v2), VRREG(d0));
            XXLANDC(VSXREG(v0), VSXREG(d1), VSXREG(d0));
            break;

        case 0x01:
            INST_NAME("VPHADDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 16);
                VSRW(VRREG(d0), VRREG(v1), VRREG(d2_tmp));
                VADDUHM(VRREG(d0), VRREG(v1), VRREG(d0));
                VSRW(VRREG(d1), VRREG(v2), VRREG(d2_tmp));
                VADDUHM(VRREG(d1), VRREG(v2), VRREG(d1));
                VPKUWUM(VRREG(v0), VRREG(d1), VRREG(d0));
            }
            break;

        case 0x02:
            INST_NAME("VPHADDD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 32);
                VSRD(VRREG(d0), VRREG(v1), VRREG(d2_tmp));
                VADDUWM(VRREG(d0), VRREG(v1), VRREG(d0));
                VSRD(VRREG(d1), VRREG(v2), VRREG(d2_tmp));
                VADDUWM(VRREG(d1), VRREG(v2), VRREG(d1));
                VPKUDUM(VRREG(v0), VRREG(d1), VRREG(d0));
            }
            break;

        case 0x03:
            INST_NAME("VPHADDSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 16);
                // Vx: sign-extend even/odd halfwords to dwords, add
                VSLW(VRREG(d0), VRREG(v1), VRREG(d2_tmp));
                VSRAW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                VSRAW(VRREG(d1), VRREG(v1), VRREG(d2_tmp));
                VADDUWM(VRREG(d0), VRREG(d0), VRREG(d1));
                // Ex: same
                VSLW(VRREG(d1), VRREG(v2), VRREG(d2_tmp));
                VSRAW(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                VSRAW(VRREG(d2_tmp), VRREG(v2), VRREG(d2_tmp));
                VADDUWM(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                // Pack with signed saturation
                VPKSWSS(VRREG(v0), VRREG(d1), VRREG(d0));
            }
            break;

        case 0x04:
            INST_NAME("VPMADDUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                int d4 = fpu_get_scratch(dyn);
                XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                // Vx unsigned bytes → halfwords
                VMRGLB(VRREG(d0), VRREG(d2_tmp), VRREG(v1));
                VMRGHB(VRREG(d1), VRREG(d2_tmp), VRREG(v1));
                // Ex signed bytes → halfwords
                VUPKLSB(VRREG(d2_tmp), VRREG(v2));
                VUPKHSB(VRREG(d3), VRREG(v2));
                // Multiply and add pairs with saturation
                VMULESH(VRREG(d4), VRREG(d0), VRREG(d2_tmp));
                VMULOSH(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                VADDSWS(VRREG(d0), VRREG(d4), VRREG(d0));
                VMULESH(VRREG(d4), VRREG(d1), VRREG(d3));
                VMULOSH(VRREG(d1), VRREG(d1), VRREG(d3));
                VADDSWS(VRREG(d1), VRREG(d4), VRREG(d1));
                VPKSWSS(VRREG(v0), VRREG(d1), VRREG(d0));
            }
            break;

        case 0x05:
            INST_NAME("VPHSUBW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 16);
                VSRW(VRREG(d0), VRREG(v1), VRREG(d2_tmp));
                VSUBUHM(VRREG(d0), VRREG(v1), VRREG(d0));
                VSRW(VRREG(d1), VRREG(v2), VRREG(d2_tmp));
                VSUBUHM(VRREG(d1), VRREG(v2), VRREG(d1));
                VPKUWUM(VRREG(v0), VRREG(d1), VRREG(d0));
            }
            break;

        case 0x08:
            INST_NAME("VPSIGNB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                VSUBUBM(VRREG(d0), VRREG(d2_tmp), VRREG(v1));
                VCMPGTSB(VRREG(d1), VRREG(d2_tmp), VRREG(v2));
                VCMPEQUB(VRREG(d2_tmp), VRREG(v2), VRREG(d2_tmp));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VSEL(VRREG(v0), VRREG(v0), VRREG(d0), VRREG(d1));
                XXLANDC(VSXREG(v0), VSXREG(v0), VSXREG(d2_tmp));
            }
            break;

        case 0x09:
            INST_NAME("VPSIGNW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                VSUBUHM(VRREG(d0), VRREG(d2_tmp), VRREG(v1));
                VCMPGTSH(VRREG(d1), VRREG(d2_tmp), VRREG(v2));
                VCMPEQUH(VRREG(d2_tmp), VRREG(v2), VRREG(d2_tmp));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VSEL(VRREG(v0), VRREG(v0), VRREG(d0), VRREG(d1));
                XXLANDC(VSXREG(v0), VSXREG(v0), VSXREG(d2_tmp));
            }
            break;

        case 0x0A:
            INST_NAME("VPSIGND Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXLXOR(VSXREG(d2_tmp), VSXREG(d2_tmp), VSXREG(d2_tmp));
                VSUBUWM(VRREG(d0), VRREG(d2_tmp), VRREG(v1));
                VCMPGTSW(VRREG(d1), VRREG(d2_tmp), VRREG(v2));
                VCMPEQUW(VRREG(d2_tmp), VRREG(v2), VRREG(d2_tmp));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VSEL(VRREG(v0), VRREG(v0), VRREG(d0), VRREG(d1));
                XXLANDC(VSXREG(v0), VSXREG(v0), VSXREG(d2_tmp));
            }
            break;

        case 0x0B:
            INST_NAME("VPMULHRSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                VMULESH(VRREG(d0), VRREG(v1), VRREG(v2));
                VMULOSH(VRREG(d1), VRREG(v1), VRREG(v2));
                XXSPLTIB(VSXREG(d2_tmp), 14);
                VSRAW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                VSRAW(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                VSPLTISW(VRREG(d2_tmp), 1);
                VADDUWM(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                VADDUWM(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                VSRAW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                VSRAW(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                VMRGLW(VRREG(d2_tmp), VRREG(d0), VRREG(d1));
                VMRGHW(VRREG(d3), VRREG(d0), VRREG(d1));
                VPKUWUM(VRREG(v0), VRREG(d3), VRREG(d2_tmp));
            }
            break;

        case 0x17:
            INST_NAME("VPTEST Gx, Ex");
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGYxy(v0, 0);
            GETEYxy(v1, 0, 0);
            CLEAR_FLAGS(x3);
            SET_DFNONE();
            d0 = fpu_get_scratch(dyn);
            IFX (X_ZF) {
                // ZF = (Gx AND Ex) == 0
                XXLAND(VSXREG(d0), VSXREG(v1), VSXREG(v0));
                MFVSRD(x1, VSXREG(d0));
                MFVSRLD(x2, VSXREG(d0));
                OR(x1, x1, x2);
                CMPDI(x1, 0);
                BNE(8);
                ORI(xFlags, xFlags, 1 << F_ZF);
            }
            IFX (X_CF) {
                // CF = (NOT(Gx) AND Ex) == 0
                XXLANDC(VSXREG(d0), VSXREG(v1), VSXREG(v0));
                MFVSRD(x1, VSXREG(d0));
                MFVSRLD(x2, VSXREG(d0));
                OR(x1, x1, x2);
                CMPDI(x1, 0);
                BNE(8);
                ORI(xFlags, xFlags, 1 << F_CF);
            }
            break;

        case 0x1C:
            INST_NAME("VPABSB Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYxy_empty(v0);
            d0 = fpu_get_scratch(dyn);
            VSPLTISB(VRREG(d0), 7);
            VSRAB(VRREG(d0), VRREG(v1), VRREG(d0));
            XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(d0));
            VSUBUBM(VRREG(v0), VRREG(v0), VRREG(d0));
            break;

        case 0x1D:
            INST_NAME("VPABSW Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYxy_empty(v0);
            d0 = fpu_get_scratch(dyn);
            VSPLTISH(VRREG(d0), 15);
            VSRAH(VRREG(d0), VRREG(v1), VRREG(d0));
            XXLXOR(VSXREG(v0), VSXREG(v1), VSXREG(d0));
            VSUBUHM(VRREG(v0), VRREG(v0), VRREG(d0));
            break;

        case 0x1E:
            INST_NAME("VPABSD Gx, Ex");
            nextop = F8;
            GETEYxy(v1, 0, 0);
            GETGYxy_empty(v0);
            d0 = fpu_get_scratch(dyn);
            VSPLTISW(VRREG(d0), 0);
            VSUBUWM(VRREG(d0), VRREG(d0), VRREG(v1));
            VMAXSW(VRREG(v0), VRREG(v1), VRREG(d0));
            break;

        case 0x20:
            INST_NAME("VPMOVSXBW Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            VUPKLSB(VRREG(v0), VRREG(v1));
            break;

        case 0x21:
            INST_NAME("VPMOVSXBD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            VUPKLSB(VRREG(v0), VRREG(v1));
            VUPKLSH(VRREG(v0), VRREG(v0));
            break;

        case 0x22:
            INST_NAME("VPMOVSXBQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LHZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            VUPKLSB(VRREG(v0), VRREG(v1));
            VUPKLSH(VRREG(v0), VRREG(v0));
            VUPKLSW(VRREG(v0), VRREG(v0));
            break;

        case 0x23:
            INST_NAME("VPMOVSXWD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            VUPKLSH(VRREG(v0), VRREG(v1));
            break;

        case 0x24:
            INST_NAME("VPMOVSXWQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            VUPKLSH(VRREG(v0), VRREG(v1));
            VUPKLSW(VRREG(v0), VRREG(v0));
            break;

        case 0x25:
            INST_NAME("VPMOVSXDQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            VUPKLSW(VRREG(v0), VRREG(v1));
            break;

        case 0x30:
            INST_NAME("VPMOVZXBW Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            VMRGLB(VRREG(v0), VRREG(d0), VRREG(v1));
            break;

        case 0x31:
            INST_NAME("VPMOVZXBD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            VMRGLB(VRREG(v0), VRREG(d0), VRREG(v1));
            VMRGLH(VRREG(v0), VRREG(d0), VRREG(v0));
            break;

        case 0x32:
            INST_NAME("VPMOVZXBQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LHZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            VMRGLB(VRREG(v0), VRREG(d0), VRREG(v1));
            VMRGLH(VRREG(v0), VRREG(d0), VRREG(v0));
            VMRGLW(VRREG(v0), VRREG(d0), VRREG(v0));
            break;

        case 0x33:
            INST_NAME("VPMOVZXWD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            VMRGLH(VRREG(v0), VRREG(d0), VRREG(v1));
            break;

        case 0x34:
            INST_NAME("VPMOVZXWQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            VMRGLH(VRREG(v0), VRREG(d0), VRREG(v1));
            VMRGLW(VRREG(v0), VRREG(d0), VRREG(v0));
            break;

        case 0x35:
            INST_NAME("VPMOVZXDQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                v1 = fpu_get_scratch(dyn);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v1), 0, x4);
            }
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            VMRGLW(VRREG(v0), VRREG(d0), VRREG(v1));
            break;

        case 0x39:
            INST_NAME("VPMINSD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINSW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3D:
            INST_NAME("VPMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXSW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        default:
            DEFAULT;
    }

    return addr;
}
