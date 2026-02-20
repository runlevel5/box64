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
    uint8_t gd, ed, vd;
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

        case 0x06:
            INST_NAME("VPHSUBD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 32);
                VSRD(VRREG(d0), VRREG(v1), VRREG(d2_tmp));
                VSUBUWM(VRREG(d0), VRREG(v1), VRREG(d0));
                VSRD(VRREG(d1), VRREG(v2), VRREG(d2_tmp));
                VSUBUWM(VRREG(d1), VRREG(v2), VRREG(d1));
                VPKUDUM(VRREG(v0), VRREG(d1), VRREG(d0));
            }
            break;

        case 0x07:
            INST_NAME("VPHSUBSW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 16);
                // Vx: sign-extend even/odd halfwords to dwords, subtract
                VSLW(VRREG(d0), VRREG(v1), VRREG(d2_tmp));
                VSRAW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));
                VSRAW(VRREG(d1), VRREG(v1), VRREG(d2_tmp));
                VSUBUWM(VRREG(d0), VRREG(d0), VRREG(d1));
                // Ex: same
                VSLW(VRREG(d1), VRREG(v2), VRREG(d2_tmp));
                VSRAW(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                VSRAW(VRREG(d2_tmp), VRREG(v2), VRREG(d2_tmp));
                VSUBUWM(VRREG(d1), VRREG(d1), VRREG(d2_tmp));
                // Pack with signed saturation
                VPKSWSS(VRREG(v0), VRREG(d1), VRREG(d0));
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

        case 0x0C:
            INST_NAME("VPERMILPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                int d3 = fpu_get_scratch(dyn);
                // Mask control to 2 bits per dword: idx = Ex[i] & 3
                VSPLTISW(VRREG(d0), 3);
                VAND(VRREG(d0), VRREG(v2), VRREG(d0));         // d0 = dword indices [0-3]
                // Convert dword index to byte offset: multiply by 4
                VSPLTISW(VRREG(d2_tmp), 2);
                VSLW(VRREG(d0), VRREG(d0), VRREG(d2_tmp));     // d0 = byte offsets [0,4,8,12]
                // Replicate each dword's byte offset to all 4 bytes: VMULUWM with 0x01010101
                LI(x4, 0x0101);
                ORIS(x4, x4, 0x0101);                          // x4 = 0x01010101
                MTVSRWZ(VSXREG(d1), x4);
                VSPLTW(VRREG(d1), VRREG(d1), 3);               // d1 = 0x01010101 splatted (LE dw0)
                VMULUWM(VRREG(d0), VRREG(d0), VRREG(d1));      // replicate byte to all 4 positions
                // Build base ramp for VPERM (BE byte numbering):
                // LE dword identity is [0,1,2,3] per dword, XOR 0x0F gives [0xF,0xE,0xD,0xC]
                // As a LE u32: 0x0C0D0E0F, splatted to all dwords
                LI(x4, 0x0E0F);
                ORIS(x4, x4, 0x0C0D);                          // x4 = 0x0C0D0E0F
                MTVSRWZ(VSXREG(d3), x4);
                VSPLTW(VRREG(d3), VRREG(d3), 3);               // d3 = base ramp splatted
                // VPERM control = base_ramp - byte_offset (byte-wise subtract)
                VSUBUBM(VRREG(d0), VRREG(d3), VRREG(d0));
                // Perform the permutation
                VPERM(VRREG(v0), VRREG(v1), VRREG(v1), VRREG(d0));
            }
            break;

        case 0x0D:
            INST_NAME("VPERMILPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                // Extract bit 1 from each qword of Ex and create all-1s/all-0s mask
                LI(x4, 1);
                MTVSRDD(VSXREG(d0), x4, x4);                   // d0 = [1, 1] as qwords
                VSRD(VRREG(d1), VRREG(v2), VRREG(d0));         // shift right by 1: bit 1 → bit 0
                VAND(VRREG(d1), VRREG(d1), VRREG(d0));         // mask to just bit 0 per qword
                // Negate to create mask: 0→0x0, 1→0xFFFFFFFFFFFFFFFF
                XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));   // zero
                VSUBUDM(VRREG(d1), VRREG(d0), VRREG(d1));      // 0-0=0, 0-1=all-1s
                // Create swapped version of Vx
                XXPERMDI(VSXREG(d2_tmp), VSXREG(v1), VSXREG(v1), 0b10);  // swap halves
                // Select: mask=0 → original (qw0), mask=1 → swapped (qw1)
                XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(d2_tmp), VSXREG(d1));
            }
            break;

        case 0x0E:
        case 0x0F:
            if (opcode == 0x0E) {
                INST_NAME("VTESTPS Gx, Ex");
            } else {
                INST_NAME("VTESTPD Gx, Ex");
            }
            nextop = F8;
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            GETGYxy(v0, 0);
            GETEYxy(v1, 0, 0);
            CLEAR_FLAGS(x3);
            SET_DFNONE();
            d0 = fpu_get_scratch(dyn);
            IFX (X_ZF) {
                // ZF = (Gx AND Ex) == 0 (check sign bits only)
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

        case 0x13:
            INST_NAME("VCVTPH2PS Gx, Ex");
            nextop = F8;
            // TODO: implement with XSCVHPDP or xvcvhpsp when emitter support is added
            DEFAULT;
            break;

        case 0x16:
            INST_NAME("VPERMPS Gx, Vx, Ex");
            nextop = F8;
            // Cross-lane 256-bit permute — requires access to full 256-bit source
            DEFAULT;
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

        case 0x18:
            INST_NAME("VBROADCASTSS Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYxy_empty(v0);
                XXSPLTW(VSXREG(v0), VSXREG(v1), 3);    // splat LE dword 0 to all dwords
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LWZ(x4, fixedaddress, ed);
                MTVSRWZ(VSXREG(v0), x4);
                XXSPLTW(VSXREG(v0), VSXREG(v0), 3);
            }
            break;

        case 0x19:
            INST_NAME("VBROADCASTSD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYxy_empty(v0);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), x4, x4);
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), x4, x4);
            }
            break;

        case 0x1A:
            INST_NAME("VBROADCASTF128 Gx, Ex");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
                // For 256-bit: upper 128 = same as lower 128 (stored via GETGYxy_empty)
            }
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

        case 0x28:
            INST_NAME("VPMULDQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULESW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x29:
            INST_NAME("VPCMPEQQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPEQUD(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x2A:
            INST_NAME("VMOVNTDQA Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYx_empty(v0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGYx_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;

        case 0x2B:
            INST_NAME("VPACKUSDW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            VSPLTISW(VRREG(d0), 0);
            // Clamp Vx: max(Vx, 0)
            VMAXSW(VRREG(d1), VRREG(v1), VRREG(d0));
            if (v1 == v2) {
                VPKSWUS(VRREG(v0), VRREG(d1), VRREG(d1));
            } else {
                // Clamp Ex: max(Ex, 0)
                VMAXSW(VRREG(d0), VRREG(v2), VRREG(d0));
                // VPKSWUS on LE: low hw from vb, high hw from va
                VPKSWUS(VRREG(v0), VRREG(d0), VRREG(d1));
            }
            break;

        case 0x2C:
            INST_NAME("VMASKMOVPS Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            // Create all-ones/all-zeros mask from sign bits
            VSPLTISW(VRREG(d0), 0);
            VSRAW(VRREG(d1), VRREG(v1), VRREG(d0));  // Need shift by 31...
            // Actually use XXSPLTIB for shift count
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 31);
                VSRAW(VRREG(d1), VRREG(v1), VRREG(d2_tmp));    // d1 = 0xFFFFFFFF or 0x00000000
                XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                VSEL(VRREG(v0), VRREG(d0), VRREG(v2), VRREG(d1));
            }
            break;

        case 0x2D:
            INST_NAME("VMASKMOVPD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 63);
                VSRAD(VRREG(d1), VRREG(v1), VRREG(d2_tmp));    // d1 = all-ones or all-zeros per qword
                XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                VSEL(VRREG(v0), VRREG(d0), VRREG(v2), VRREG(d1));
            }
            break;

        case 0x2E:
            INST_NAME("VMASKMOVPS Ex, Vx, Gx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 31);
                VSRAW(VRREG(d0), VRREG(v1), VRREG(d2_tmp));    // mask from sign bits
                VSEL(VRREG(v0), VRREG(v0), VRREG(v2), VRREG(d0));
                PUTEYxy(v0);
            }
            break;

        case 0x2F:
            INST_NAME("VMASKMOVPD Ex, Vx, Gx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                XXSPLTIB(VSXREG(d2_tmp), 63);
                VSRAD(VRREG(d0), VRREG(v1), VRREG(d2_tmp));    // mask from sign bits
                VSEL(VRREG(v0), VRREG(v0), VRREG(v2), VRREG(d0));
                PUTEYxy(v0);
            }
            break;

        case 0x36:
            INST_NAME("VPERMD Gx, Vx, Ex");
            nextop = F8;
            // Cross-lane 256-bit permute — requires access to full 256-bit source
            DEFAULT;
            break;

        case 0x37:
            INST_NAME("VPCMPGTQ Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VCMPGTSD(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x38:
            INST_NAME("VPMINSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINSB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x39:
            INST_NAME("VPMINSD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINSW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3A:
            INST_NAME("VPMINUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINUH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3B:
            INST_NAME("VPMINUD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMINUW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3C:
            INST_NAME("VPMAXSB Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXSB(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3D:
            INST_NAME("VPMAXSD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXSW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3E:
            INST_NAME("VPMAXUW Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXUH(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x3F:
            INST_NAME("VPMAXUD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMAXUW(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x40:
            INST_NAME("VPMULLD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            VMULUWM(VRREG(v0), VRREG(v1), VRREG(v2));
            break;

        case 0x41:
            INST_NAME("VPHMINPOSUW Gx, Ex");
            nextop = F8;
            GETEYx(v1, 0, 0);
            GETGYx_empty(v0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            {
                int d2_tmp = fpu_get_scratch(dyn);
                // Tournament reduction: find min of 8 unsigned halfwords in low 128 bits
                // Round 1: swap dwords within qwords and min
                XXPERMDI(VSXREG(d0), VSXREG(v1), VSXREG(v1), 2);  // swap qwords
                VMINUH(VRREG(d0), VRREG(v1), VRREG(d0));
                // Round 2: rotate by 2 words (32 bits) within each qword and min
                XXSPLTIB(VSXREG(d2_tmp), 32);
                VSRD(VRREG(d1), VRREG(d0), VRREG(d2_tmp));
                // Fill with max to not affect min
                VMINUH(VRREG(d0), VRREG(d0), VRREG(d1));
                // Round 3: rotate by 1 word (16 bits) within each dword and min
                XXSPLTIB(VSXREG(d2_tmp), 16);
                VSRW(VRREG(d1), VRREG(d0), VRREG(d2_tmp));
                VMINUH(VRREG(d0), VRREG(d0), VRREG(d1));
                // Now d0 LE hw 0 has the min value; broadcast it
                VSPLTH(VRREG(d0), VRREG(d0), 7);   // BE hw 7 = LE hw 0; splat min to all hw
                // Find index: compare original with broadcast min
                VCMPEQUH(VRREG(d1), VRREG(v1), VRREG(d0));
                // Extract mask of matching halfwords, find lowest set bit index
                // On PPC LE, extract the low 64 bits as a GPR to find the first matching hw
                MFVSRLD(x1, VSXREG(d1));
                // Find first set bit (lowest byte position of 0xFFFF match)
                // CNTTZD gives trailing zeros; divide by 16 to get halfword index
                CNTTZD(x1, x1);
                SRADI(x1, x1, 4);      // divide by 16 → halfword index (0-3)
                // Get the minimum value from d0 (it's splatted, so any hw works)
                MFVSRLD(x2, VSXREG(d0));
                RLWINM(x2, x2, 0, 16, 31);   // extract low 16 bits
                // Combine: result = min_value | (index << 16), zero the rest
                RLDIMI(x2, x1, 16, 0);   // insert index at bits 16-18
                MTVSRDD(VSXREG(v0), 0, x2);  // zero high qword, low qword = result
            }
            break;

        case 0x45:
            INST_NAME("VPSRLVD/Q Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // VPSRLVQ: shift right logical, qword elements
                // Create limit = 64 in each dword
                LI(x4, 64);
                MTVSRDD(VSXREG(d0), x4, x4);
                // mask[i] = (64 > count[i]) ? all 1s : 0
                VCMPGTUD(VRREG(d0), VRREG(d0), VRREG(v2));
                VSRD(VRREG(v0), VRREG(v1), VRREG(v2));
                VAND(VRREG(v0), VRREG(v0), VRREG(d0));
            } else {
                // VPSRLVD: shift right logical, dword elements
                // Create limit = 32 in each word
                LI(x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSPLTW(VSXREG(d0), VSXREG(d0), 3);
                // mask[i] = (32 > count[i]) ? all 1s : 0
                VCMPGTUW(VRREG(d0), VRREG(d0), VRREG(v2));
                VSRW(VRREG(v0), VRREG(v1), VRREG(v2));
                VAND(VRREG(v0), VRREG(v0), VRREG(d0));
            }
            break;
        case 0x46:
            INST_NAME("VPSRAVD Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            // Clamp shift count to 31 (x86 saturates arithmetic shifts)
            LI(x4, 31);
            MTVSRDD(VSXREG(d0), x4, x4);
            XXSPLTW(VSXREG(d0), VSXREG(d0), 3);  // d0 = [31, 31, 31, 31]
            VMINUW(VRREG(d1), VRREG(v2), VRREG(d0));
            VSRAW(VRREG(v0), VRREG(v1), VRREG(d1));
            break;
        case 0x47:
            INST_NAME("VPSLLVD/Q Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // VPSLLVQ: shift left logical, qword elements
                LI(x4, 64);
                MTVSRDD(VSXREG(d0), x4, x4);
                VCMPGTUD(VRREG(d0), VRREG(d0), VRREG(v2));
                VSLD(VRREG(v0), VRREG(v1), VRREG(v2));
                VAND(VRREG(v0), VRREG(v0), VRREG(d0));
            } else {
                // VPSLLVD: shift left logical, dword elements
                LI(x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSPLTW(VSXREG(d0), VSXREG(d0), 3);
                VCMPGTUW(VRREG(d0), VRREG(d0), VRREG(v2));
                VSLW(VRREG(v0), VRREG(v1), VRREG(v2));
                VAND(VRREG(v0), VRREG(v0), VRREG(d0));
            }
            break;

        case 0x58:
            INST_NAME("VPBROADCASTD Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYxy_empty(v0);
                XXSPLTW(VSXREG(v0), VSXREG(v1), 3);    // splat LE dword 0
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LWZ(x4, fixedaddress, ed);
                MTVSRWZ(VSXREG(v0), x4);
                XXSPLTW(VSXREG(v0), VSXREG(v0), 3);
            }
            break;

        case 0x59:
            INST_NAME("VPBROADCASTQ Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYxy_empty(v0);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), x4, x4);
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), x4, x4);
            }
            break;

        case 0x5A:
            INST_NAME("VBROADCASTI128 Gx, Ex");
            nextop = F8;
            if (MODREG) {
                DEFAULT;
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, DQ_ALIGN|1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
                // For 256-bit: both lanes get the same 128-bit value
            }
            break;

        case 0x78:
            INST_NAME("VPBROADCASTB Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYxy_empty(v0);
                VSPLTB(VRREG(v0), VRREG(v1), 15);    // BE byte 15 = LE byte 0
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LBZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
                VSPLTB(VRREG(v0), VRREG(v0), 15);
            }
            break;

        case 0x79:
            INST_NAME("VPBROADCASTW Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = avx_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0, VMX_AVX_WIDTH_128);
                GETGYxy_empty(v0);
                VSPLTH(VRREG(v0), VRREG(v1), 7);    // BE hw 7 = LE hw 0
            } else {
                GETGYxy_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
                VSPLTH(VRREG(v0), VRREG(v0), 7);
            }
            break;

        case 0x90:
            INST_NAME("VPGATHERDD/Q Gx, Ey, Vx");
            nextop = F8;
            // Complex VSIB gather — fall back to interpreter
            DEFAULT;
            break;
        case 0x91:
            INST_NAME("VPGATHERQD/Q Gx, Ey, Vx");
            nextop = F8;
            DEFAULT;
            break;
        case 0x92:
            INST_NAME("VGATHERDPS/D Gx, Ey, Vx");
            nextop = F8;
            DEFAULT;
            break;
        case 0x93:
            INST_NAME("VGATHERQPS/D Gx, Ey, Vx");
            nextop = F8;
            DEFAULT;
            break;

        case 0x96:
            INST_NAME("VFMADDSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            // FMADDSUB: even elements = SUB, odd elements = ADD
            // Compute MADD into q0, MSUB into v0, then blend
            if (rex.w) {
                // PD: v0 = v0*v2 + v1 (MADD), save to q0; v0 = v0*v2 - v1 (MSUB)
                // But v0 is destroyed after first op, so: copy v0 first
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMADDADP(VSXREG(q0), VSXREG(v2), VSXREG(v1));   // q0 = v2*q0 + v1 (MADD)
                XVMSUBADP(VSXREG(v0), VSXREG(v2), VSXREG(v1));   // v0 = v2*v0 - v1 (MSUB)
                // Blend: LE dw0 (element 0) = MSUB (v0), LE dw1 (element 1) = MADD (q0)
                // XXPERMDI: XT[ISA dw0] from q0[ISA dw0], XT[ISA dw1] from v0[ISA dw1]
                // ISA dw0 = LE dw1 = element 1 (MADD), ISA dw1 = LE dw0 = element 0 (MSUB)
                XXPERMDI(VSXREG(v0), VSXREG(q0), VSXREG(v0), 1);
            } else {
                // PS: copy v0, compute MADD and MSUB, blend with XXSEL
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMADDASP(VSXREG(q0), VSXREG(v2), VSXREG(v1));   // q0 = MADD result
                XVMSUBASP(VSXREG(v0), VSXREG(v2), VSXREG(v1));   // v0 = MSUB result
                // Build mask: odd elements (1,3) from MADD, even elements (0,2) from MSUB
                // Mask: LE word 0=0, word 1=FF, word 2=0, word 3=FF
                // ISA dw0 = 0xFFFFFFFF00000000, ISA dw1 = 0xFFFFFFFF00000000
                LI(x4, -1);
                SLDI(x4, x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSEL(VSXREG(v0), VSXREG(v0), VSXREG(q0), VSXREG(d0));
            }
            break;
        case 0x97:
            INST_NAME("VFMSUBADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            // FMSUBADD: even elements = ADD, odd elements = SUB
            if (rex.w) {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMSUBADP(VSXREG(q0), VSXREG(v2), VSXREG(v1));   // q0 = MSUB result
                XVMADDADP(VSXREG(v0), VSXREG(v2), VSXREG(v1));   // v0 = MADD result
                // Blend: LE dw0 (element 0) = MADD (v0), LE dw1 (element 1) = MSUB (q0)
                XXPERMDI(VSXREG(v0), VSXREG(q0), VSXREG(v0), 1);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMSUBASP(VSXREG(q0), VSXREG(v2), VSXREG(v1));   // q0 = MSUB result
                XVMADDASP(VSXREG(v0), VSXREG(v2), VSXREG(v1));   // v0 = MADD result
                // Build mask: odd elements (1,3) from MSUB, even elements (0,2) from MADD
                // Mask: LE word 0=0, word 1=FF, word 2=0, word 3=FF (same mask, but swap XXSEL args)
                LI(x4, -1);
                SLDI(x4, x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSEL(VSXREG(v0), VSXREG(v0), VSXREG(q0), VSXREG(d0));
            }
            break;
        case 0x98:
            INST_NAME("VFMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = v0*v2 + v1 (A-form: XT = XA*XT + XB)
            if (rex.w) {
                XVMADDADP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            } else {
                XVMADDASP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            }
            break;
        case 0x99:
            INST_NAME("VFMADD132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // SD: extract Gx and Vx doubles from LE dw0, Ex may need extraction
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d0*q0 + d1 → A-form: d0 = q0*d0 + d1
                XSMADDADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                // Insert result into Gx, copy Vx non-scalar lanes
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                // SS: extract Gx and Vx floats, convert to DP
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d0*q0 + d1 → A-form: d0 = q0*d0 + d1
                XSMADDADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                // Convert back to SP, insert into Gx
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0x9A:
            INST_NAME("VFMSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = v0*v2 - v1 (A-form: XT = XA*XT - XB)
            if (rex.w) {
                XVMSUBADP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            } else {
                XVMSUBASP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            }
            break;
        case 0x9B:
            INST_NAME("VFMSUB132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d0*q0 - d1 → A-form: d0 = q0*d0 - d1
                XSMSUBADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSMSUBADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0x9C:
            INST_NAME("VFNMADD132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = -(v0*v2) + v1 = -(v0*v2 - v1) => PPC NMSUB (A-form: XT = -(XA*XT - XB))
            if (rex.w) {
                XVNMSUBADP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            } else {
                XVNMSUBASP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            }
            break;
        case 0x9D:
            INST_NAME("VFNMADD132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // -(d0*q0) + d1 = -(d0*q0 - d1) => PPC NMSUB A-form
                XSNMSUBADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSNMSUBADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0x9E:
            INST_NAME("VFNMSUB132PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = -(v0*v2) - v1 = -(v0*v2 + v1) => PPC NMADD (A-form: XT = -(XA*XT + XB))
            if (rex.w) {
                XVNMADDADP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            } else {
                XVNMADDASP(VSXREG(v0), VSXREG(v2), VSXREG(v1));
            }
            break;
        case 0x9F:
            INST_NAME("VFNMSUB132SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // -(d0*q0) - d1 = -(d0*q0 + d1) => PPC NMADD A-form
                XSNMADDADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSNMADDADP(VSXREG(d0), VSXREG(q0), VSXREG(d1));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xA6:
            INST_NAME("VFMADDSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMADDADP(VSXREG(q0), VSXREG(v1), VSXREG(v2));   // q0 = v1*q0 + v2 (MADD)
                XVMSUBADP(VSXREG(v0), VSXREG(v1), VSXREG(v2));   // v0 = v1*v0 - v2 (MSUB)
                XXPERMDI(VSXREG(v0), VSXREG(q0), VSXREG(v0), 1);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMADDASP(VSXREG(q0), VSXREG(v1), VSXREG(v2));
                XVMSUBASP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                LI(x4, -1);
                SLDI(x4, x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSEL(VSXREG(v0), VSXREG(v0), VSXREG(q0), VSXREG(d0));
            }
            break;
        case 0xA7:
            INST_NAME("VFMSUBADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMSUBADP(VSXREG(q0), VSXREG(v1), VSXREG(v2));   // q0 = MSUB result
                XVMADDADP(VSXREG(v0), VSXREG(v1), VSXREG(v2));   // v0 = MADD result
                XXPERMDI(VSXREG(v0), VSXREG(q0), VSXREG(v0), 1);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMSUBASP(VSXREG(q0), VSXREG(v1), VSXREG(v2));
                XVMADDASP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                LI(x4, -1);
                SLDI(x4, x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSEL(VSXREG(v0), VSXREG(v0), VSXREG(q0), VSXREG(d0));
            }
            break;
        case 0xA8:
            INST_NAME("VFMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = v1*v0 + v2 (A-form: XT = XA*XT + XB)
            if (rex.w) {
                XVMADDADP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVMADDASP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xA9:
            INST_NAME("VFMADD213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d1*d0 + q0 → A-form: d0 = d1*d0 + q0
                XSMADDADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSMADDADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xAA:
            INST_NAME("VFMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = v1*v0 - v2 (A-form: XT = XA*XT - XB)
            if (rex.w) {
                XVMSUBADP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVMSUBASP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xAB:
            INST_NAME("VFMSUB213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d1*d0 - q0 → A-form: d0 = d1*d0 - q0
                XSMSUBADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSMSUBADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xAC:
            INST_NAME("VFNMADD213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = -(v1*v0) + v2 = -(v1*v0 - v2) => PPC NMSUB
            if (rex.w) {
                XVNMSUBADP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVNMSUBASP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xAD:
            INST_NAME("VFNMADD213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // -(d1*d0) + q0 = -(d1*d0 - q0) => PPC NMSUB A-form
                XSNMSUBADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSNMSUBADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xAE:
            INST_NAME("VFNMSUB213PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = -(v1*v0) - v2 = -(v1*v0 + v2) => PPC NMADD
            if (rex.w) {
                XVNMADDADP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVNMADDASP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xAF:
            INST_NAME("VFNMSUB213SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // -(d1*d0) - q0 = -(d1*d0 + q0) => PPC NMADD A-form
                XSNMADDADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSNMADDADP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xB6:
            INST_NAME("VFMADDSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMADDMDP(VSXREG(q0), VSXREG(v1), VSXREG(v2));   // q0 = v1*v2 + q0 (MADD)
                XVMSUBMDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));   // v0 = v1*v2 - v0 (MSUB)
                XXPERMDI(VSXREG(v0), VSXREG(q0), VSXREG(v0), 1);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMADDMSP(VSXREG(q0), VSXREG(v1), VSXREG(v2));
                XVMSUBMSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                LI(x4, -1);
                SLDI(x4, x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSEL(VSXREG(v0), VSXREG(v0), VSXREG(q0), VSXREG(d0));
            }
            break;
        case 0xB7:
            INST_NAME("VFMSUBADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            q0 = fpu_get_scratch(dyn);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMSUBMDP(VSXREG(q0), VSXREG(v1), VSXREG(v2));   // q0 = MSUB result
                XVMADDMDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));   // v0 = MADD result
                XXPERMDI(VSXREG(v0), VSXREG(q0), VSXREG(v0), 1);
            } else {
                XXLOR(VSXREG(q0), VSXREG(v0), VSXREG(v0));
                XVMSUBMSP(VSXREG(q0), VSXREG(v1), VSXREG(v2));
                XVMADDMSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
                LI(x4, -1);
                SLDI(x4, x4, 32);
                MTVSRDD(VSXREG(d0), x4, x4);
                XXSEL(VSXREG(v0), VSXREG(v0), VSXREG(q0), VSXREG(d0));
            }
            break;
        case 0xB8:
            INST_NAME("VFMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = v1*v2 + v0 (M-form: XT = XA*XB + XT)
            if (rex.w) {
                XVMADDMDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVMADDMSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xB9:
            INST_NAME("VFMADD231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d1*q0 + d0 → M-form: d0 = d1*q0 + d0
                XSMADDMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSMADDMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xBA:
            INST_NAME("VFMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = v1*v2 - v0 (M-form: XT = XA*XB - XT)
            if (rex.w) {
                XVMSUBMDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVMSUBMSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xBB:
            INST_NAME("VFMSUB231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // d0 = d1*q0 - d0 → M-form: d0 = d1*q0 - d0
                XSMSUBMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSMSUBMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xBC:
            INST_NAME("VFNMADD231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = -(v1*v2) + v0 = -(v1*v2 - v0) => PPC NMSUB (M-form: XT = -(XA*XB - XT))
            if (rex.w) {
                XVNMSUBMDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVNMSUBMSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xBD:
            INST_NAME("VFNMADD231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // -(d1*q0) + d0 = -(d1*q0 - d0) => PPC NMSUB M-form
                XSNMSUBMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSNMSUBMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;
        case 0xBE:
            INST_NAME("VFNMSUB231PS/D Gx, Vx, Ex");
            nextop = F8;
            GETGY_VYEY_xy(v0, v1, v2, 0);
            // v0 = -(v1*v2) - v0 = -(v1*v2 + v0) => PPC NMADD (M-form: XT = -(XA*XB + XT))
            if (rex.w) {
                XVNMADDMDP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            } else {
                XVNMADDMSP(VSXREG(v0), VSXREG(v1), VSXREG(v2));
            }
            break;
        case 0xBF:
            INST_NAME("VFNMSUB231SS/D Gx, Vx, Ex");
            nextop = F8;
            GETGYx_VYx_EYxw(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            if (rex.w) {
                MFVSRLD(x4, VSXREG(v0));
                MTVSRD(VSXREG(d0), x4);
                MFVSRLD(x4, VSXREG(v1));
                MTVSRD(VSXREG(d1), x4);
                if (MODREG) {
                    MFVSRLD(x4, VSXREG(v2));
                    MTVSRD(VSXREG(q0), x4);
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                // -(d1*q0) - d0 = -(d1*q0 + d0) => PPC NMADD M-form
                XSNMADDMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d0), 0);
            } else {
                XXSPLTW(VSXREG(d0), VSXREG(v0), 3);
                XSCVSPDPN(VSXREG(d0), VSXREG(d0));
                XXSPLTW(VSXREG(d1), VSXREG(v1), 3);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
                if (MODREG) {
                    XXSPLTW(VSXREG(q0), VSXREG(v2), 3);
                    XSCVSPDPN(VSXREG(q0), VSXREG(q0));
                } else {
                    XXLOR(VSXREG(q0), VSXREG(v2), VSXREG(v2));
                }
                XSNMADDMDP(VSXREG(d0), VSXREG(d1), VSXREG(q0));
                XSCVDPSPN(VSXREG(d0), VSXREG(d0));
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                VINSERTW(VRREG(v0), VRREG(d0), 12);
            }
            break;

        case 0x8C:
            INST_NAME("VPMASKMOVD/Q Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            // Create mask from sign bits of Vx: all 1s where negative, all 0s otherwise
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));  // zero
            if (rex.w) {
                VCMPGTSD(VRREG(d0), VRREG(d0), VRREG(v1));  // d0[i] = (0 > v1[i]) ? -1 : 0
            } else {
                VCMPGTSW(VRREG(d0), VRREG(d0), VRREG(v1));
            }
            // Select: where mask is all 1s, take from Ex (v2); else 0
            VAND(VRREG(v0), VRREG(v2), VRREG(d0));
            break;
        case 0x8E:
            INST_NAME("VPMASKMOVD/Q Ex, Vx, Gx");
            nextop = F8;
            GETEY_VYGY_xy(v0, v1, v2, 0);
            d0 = fpu_get_scratch(dyn);
            // Create mask from sign bits of Vx
            XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
            if (rex.w) {
                VCMPGTSD(VRREG(d0), VRREG(d0), VRREG(v1));
            } else {
                VCMPGTSW(VRREG(d0), VRREG(d0), VRREG(v1));
            }
            // Select: where mask bit set, take from Gx (v2); else keep Ex (v0)
            VSEL(VRREG(v0), VRREG(v0), VRREG(v2), VRREG(d0));
            PUTEYxy(v0);
            break;

        case 0xDB:
            INST_NAME("VAESIMC Gx, Ex");
            nextop = F8;
            GETEYx(v1, 0, 0);
            GETGYx_empty(v0);
            if (v0 != v1) {
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aesimc, -1, x1, 0);
            GETGYx(v0, 1);  // re-get writable for upper zero
            break;
        case 0xDC:
            INST_NAME("VAESENC Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            // Save Ex if Gx == Ex (will be clobbered by forget+call)
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2));
            } else
                d0 = -1;
            // Copy Vx to Gx
            if (gd != vex.v) {
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aese, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aese_y, -1, x1, 0);
            }
            GETGYxy(v0, 1);
            XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v2));
            break;
        case 0xDD:
            INST_NAME("VAESENCLAST Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2));
            } else
                d0 = -1;
            if (gd != vex.v) {
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aeselast, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aeselast_y, -1, x1, 0);
            }
            GETGYxy(v0, 1);
            XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v2));
            break;
        case 0xDE:
            INST_NAME("VAESDEC Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2));
            } else
                d0 = -1;
            if (gd != vex.v) {
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aesd, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aesd_y, -1, x1, 0);
            }
            GETGYxy(v0, 1);
            XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v2));
            break;
        case 0xDF:
            INST_NAME("VAESDECLAST Gx, Vx, Ex");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 0);
            if (MODREG && (gd == (nextop & 7) + (rex.b << 3))) {
                d0 = fpu_get_scratch(dyn);
                XXLOR(VSXREG(d0), VSXREG(v2), VSXREG(v2));
            } else
                d0 = -1;
            if (gd != vex.v) {
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            }
            avx_forget_reg(dyn, ninst, gd);
            MOV32w(x1, gd);
            CALL(const_native_aesdlast, -1, x1, 0);
            if (vex.l) {
                MOV32w(x1, gd);
                CALL(const_native_aesdlast_y, -1, x1, 0);
            }
            GETGYxy(v0, 1);
            XXLXOR(VSXREG(v0), VSXREG(v0), (d0 != -1) ? VSXREG(d0) : VSXREG(v2));
            break;

        case 0xF7:
            INST_NAME("SHLX Gd, Ed, Vd");
            nextop = F8;
            GETGD;
            GETED(0);
            GETVD;
            ANDI(x5, vd, rex.w ? 0x3f : 0x1f);
            SLLxw(gd, ed, x5);
            break;

        default:
            DEFAULT;
    }

    return addr;
}
