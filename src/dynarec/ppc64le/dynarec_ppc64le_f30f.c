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
#include "my_cpuid.h"
#include "emu/x87emu_private.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_F30F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1;
    int64_t fixedaddress;
    int unscaled;
    int v0, v1, q0, q1, d0, d1;
    MAYUSE(u8);
    MAYUSE(wb1);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);

    switch (opcode) {
        case 0x10:
            INST_NAME("MOVSS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                // reg-reg: merge low 32 bits of source into dest, keep upper 96 bits
                v0 = sse_get_reg(dyn, ninst, x1, gd, 1);
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // VINSERTW inserts bits 96:127 of src (LE word 0) into byte offset 12 of dest (LE word 0)
                VINSERTW(VRREG(v0), VRREG(v1), 12);
            } else {
                // mem: zero dest, load 32-bit float into low 32 bits
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
                LWZ(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x11:
            INST_NAME("MOVSS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                // reg-reg: merge low 32 bits of source into dest
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 1);
                VINSERTW(VRREG(v1), VRREG(v0), 12);
            } else {
                // mem: store low 32 bits (x86 scalar float = LE word 0)
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                MFVSRLD(x4, VSXREG(v0));  // x86 low 64 bits; float is in low 32
                STW(x4, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x1E:
            INST_NAME("NOP / ENDBR32 / ENDBR64");
            nextop = F8;
            FAKEED;
            break;

        case 0x2A:
            INST_NAME("CVTSI2SS Gx, Ed");
            nextop = F8;
            GETGX(v0, 1);
            GETED(0);
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // Convert 64-bit int to single float
                MTVSRD(VSXREG(d0), ed);
                XSCVSXDSP(VSXREG(d0), VSXREG(d0));
            } else {
                // Convert 32-bit int to single float
                EXTSW(x4, ed);
                MTVSRD(VSXREG(d0), x4);
                XSCVSXDSP(VSXREG(d0), VSXREG(d0));
            }
            // Result is in d0 as a double-precision scalar; convert to single in VSX format
            // XSCVDPSPN converts DP scalar to SP in word 0 of result (BE byte 0)
            XSCVDPSPN(VSXREG(d0), VSXREG(d0));
            // Extract to BE byte 0, then insert at LE word 0 (BE byte 12)
            VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
            VINSERTW(VRREG(v0), VRREG(d0), 12);
            break;
        case 0x2C:
            INST_NAME("CVTTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            // Convert single-precision float to integer with truncation
            d1 = fpu_get_scratch(dyn);
            // d0 may be a VR (SSE register). Extract low float to scratch FPR for scalar conversion
            // For MODREG case, d0 is the full SSE register - need to get scalar from it
            if (MODREG) {
                // d0 is actually the SSE register; extract float from LE word 0
                // LE word 0 is at BE byte offset 12; XSCVSPDPN reads from BE word 0
                // So first move LE word 0 to BE word 0 position in scratch d1
                VEXTRACTUW(VRREG(d1), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                // d0 is a scratch FPR loaded via LFS (already double-precision in FPR)
                FMR(d1, d0);
            }
            if (rex.w) {
                XSCVDPSXDS(VSXREG(d1), VSXREG(d1));  // truncate to signed 64-bit
                MFVSRD(gd, VSXREG(d1));
            } else {
                XSCVDPSXWS(VSXREG(d1), VSXREG(d1));  // truncate to signed 32-bit
                MFVSRWZ(gd, VSXREG(d1));
                ZEROUP(gd);
            }
            break;
        case 0x2D:
            INST_NAME("CVTSS2SI Gd, Ex");
            nextop = F8;
            GETGD;
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                VEXTRACTUW(VRREG(d1), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                FMR(d1, d0);
            }
            if (rex.w) {
                XSCVDPSXDS(VSXREG(d1), VSXREG(d1));  // round to signed 64-bit (current rounding mode)
                MFVSRD(gd, VSXREG(d1));
            } else {
                XSCVDPSXWS(VSXREG(d1), VSXREG(d1));  // round to signed 32-bit
                MFVSRWZ(gd, VSXREG(d1));
                ZEROUP(gd);
            }
            break;

        case 0x51:
            INST_NAME("SQRTSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                VEXTRACTUW(VRREG(d1), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                FMR(d1, d0);
            }
            XSSQRTDP(VSXREG(d1), VSXREG(d1));
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x58:
            INST_NAME("ADDSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            // Extract Gx float to double
            VEXTRACTUW(VRREG(d1), VRREG(v0), 12);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            // Extract Ex float to double
            if (MODREG) {
                VEXTRACTUW(VRREG(q0), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                FMR(q0, d0);
            }
            FADDS(d1, d1, q0);
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x59:
            INST_NAME("MULSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            VEXTRACTUW(VRREG(d1), VRREG(v0), 12);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                VEXTRACTUW(VRREG(q0), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                FMR(q0, d0);
            }
            FMULS(d1, d1, q0);
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5A:
            INST_NAME("CVTSS2SD Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            if (MODREG) {
                VEXTRACTUW(VRREG(d1), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            } else {
                FMR(d1, d0);  // LFS already converts to double
            }
            // Result is now a double in d1; insert into low 64 bits of v0
            // d1 FPR scalar result is in ISA dw0; insert into v0's ISA dw1 (x86 low)
            XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(d1), 0);
            break;
        case 0x5C:
            INST_NAME("SUBSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            VEXTRACTUW(VRREG(d1), VRREG(v0), 12);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                VEXTRACTUW(VRREG(q0), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                FMR(q0, d0);
            }
            FSUBS(d1, d1, q0);
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;
        case 0x5E:
            INST_NAME("DIVSS Gx, Ex");
            nextop = F8;
            GETGX(v0, 1);
            GETEXSS(d0, 0, 0);
            d1 = fpu_get_scratch(dyn);
            q0 = fpu_get_scratch(dyn);
            VEXTRACTUW(VRREG(d1), VRREG(v0), 12);
            XSCVSPDPN(VSXREG(d1), VSXREG(d1));
            if (MODREG) {
                VEXTRACTUW(VRREG(q0), VRREG(d0), 12);
                XSCVSPDPN(VSXREG(q0), VSXREG(q0));
            } else {
                FMR(q0, d0);
            }
            FDIVS(d1, d1, q0);
            XSCVDPSPN(VSXREG(d1), VSXREG(d1));
            VEXTRACTUW(VRREG(d1), VRREG(d1), 0);
            VINSERTW(VRREG(v0), VRREG(d1), 12);
            break;

        case 0x6F:
            INST_NAME("MOVDQU Gx, Ex");
            nextop = F8;
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                GETGX_empty(v0);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                GETGX_empty(v0);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;

        case 0x7E:
            INST_NAME("MOVQ Gx, Ex");
            nextop = F8;
            GETGX_empty(v0);
            if (MODREG) {
                v1 = sse_get_reg(dyn, ninst, x1, (nextop & 7) + (rex.b << 3), 0);
                // Copy low 64 bits, zero upper 64 bits (x86 low = ISA dw1)
                MFVSRLD(x4, VSXREG(v1));
                MTVSRDD(VSXREG(v0), 0, x4);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LD(x4, fixedaddress, ed);
                MTVSRDD(VSXREG(v0), 0, x4);
            }
            break;
        case 0x7F:
            INST_NAME("MOVDQU Ex, Gx");
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

        case 0xE6:
            INST_NAME("CVTDQ2PD Gx, Ex");
            nextop = F8;
            GETEX(v1, 0, 0);
            GETGX_empty(v0);
            // Convert 2 int32 from low 64 bits of source to 2 doubles
            // Extract low 2 dwords
            MFVSRLD(x4, VSXREG(v1));  // x86 low 64 bits (ISA dw1)
            // Low 32 bits = int0
            EXTSW(x5, x4);
            // High 32 bits = int1
            SRDI(x6, x4, 32);
            EXTSW(x6, x6);
            // Convert each to double
            d0 = fpu_get_scratch(dyn);
            MTVSRD(VSXREG(d0), x5);
            XSCVSXDDP(VSXREG(d0), VSXREG(d0));
            d1 = fpu_get_scratch(dyn);
            MTVSRD(VSXREG(d1), x6);
            XSCVSXDDP(VSXREG(d1), VSXREG(d1));
            // Combine: v0 = [double1, double0] where double0 is in low 64
            MFVSRD(x5, VSXREG(d0));
            MFVSRD(x6, VSXREG(d1));
            MTVSRDD(VSXREG(v0), x6, x5);
            break;

        default:
            DEFAULT;
    }

    return addr;
}
