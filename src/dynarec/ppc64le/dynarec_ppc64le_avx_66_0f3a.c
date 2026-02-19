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

uintptr_t dynarec64_AVX_66_0F3A(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
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
        case 0x0E:
            INST_NAME("VPBLENDW Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            if (v1 == v2) {
                // Same source: just copy
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else if (u8 == 0xFF) {
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else if (u8 == 0x00) {
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                // Start with Vx
                if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                d0 = fpu_get_scratch(dyn);
                // Optimize aligned groups
                if ((u8 & 0x0F) == 0x0F) {
                    // Replace x86 hw 0-3 (ISA dw1)
                    XXPERMDI(VSXREG(v0), VSXREG(v0), VSXREG(v2), 0b01);
                    u8 &= ~0x0F;
                }
                if ((u8 & 0xF0) == 0xF0) {
                    // Replace x86 hw 4-7 (ISA dw0)
                    XXPERMDI(VSXREG(v0), VSXREG(v2), VSXREG(v0), 0b01);
                    u8 &= ~0xF0;
                }
                // Word-aligned pairs
                if ((u8 & 0x03) == 0x03) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 12);
                    VINSERTW(VRREG(v0), VRREG(d0), 12);
                    u8 &= ~0x03;
                }
                if ((u8 & 0x0C) == 0x0C) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 8);
                    VINSERTW(VRREG(v0), VRREG(d0), 8);
                    u8 &= ~0x0C;
                }
                if ((u8 & 0x30) == 0x30) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 4);
                    VINSERTW(VRREG(v0), VRREG(d0), 4);
                    u8 &= ~0x30;
                }
                if ((u8 & 0xC0) == 0xC0) {
                    VEXTRACTUW(VRREG(d0), VRREG(v2), 0);
                    VINSERTW(VRREG(v0), VRREG(d0), 0);
                    u8 &= ~0xC0;
                }
                // Individual halfwords
                for (int i = 0; i < 8; ++i) {
                    if (u8 & (1 << i)) {
                        int boff = (7 - i) * 2;
                        VEXTRACTUH(VRREG(d0), VRREG(v2), boff);
                        VINSERTH(VRREG(v0), VRREG(d0), boff);
                    }
                }
            }
            break;

        case 0x0F:
            INST_NAME("VPALIGNR Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            if (u8 > 31) {
                // Result is all zeros
                XXLXOR(VSXREG(v0), VSXREG(v0), VSXREG(v0));
            } else if (u8 > 15) {
                // Shift Vx right by (u8-16) bytes
                if (u8 == 16) {
                    if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
                } else {
                    d0 = fpu_get_scratch(dyn);
                    XXLXOR(VSXREG(d0), VSXREG(d0), VSXREG(d0));
                    VSLDOI(VRREG(d0), VRREG(v1), VRREG(d0), 32 - u8);
                    XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
                }
            } else if (u8 == 0) {
                if (v0 != v2) XXLOR(VSXREG(v0), VSXREG(v2), VSXREG(v2));
            } else {
                // General case: concat Vx(high):Ex(low), shift right by u8 bytes
                d0 = fpu_get_scratch(dyn);
                VSLDOI(VRREG(d0), VRREG(v1), VRREG(v2), 16 - u8);
                XXLOR(VSXREG(v0), VSXREG(d0), VSXREG(d0));
            }
            break;

        case 0x22:
            if (rex.w) {
                INST_NAME("VPINSRQ Gx, Vx, ED, Ib");
            } else {
                INST_NAME("VPINSRD Gx, Vx, ED, Ib");
            }
            nextop = F8;
            GETVYx(v1, 0);
            GETED(1);
            GETGYx_empty(v0);
            u8 = F8;
            if (v0 != v1) XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            d0 = fpu_get_scratch(dyn);
            if (rex.w) {
                // VPINSRQ: insert 64-bit value
                MTVSRDD(VSXREG(d0), ed, ed);
                VINSERTD(VRREG(v0), VRREG(d0), (1 - (u8 & 1)) * 8);
            } else {
                // VPINSRD: insert 32-bit value
                MTVSRDD(VSXREG(d0), ed, ed);
                VEXTRACTUW(VRREG(d0), VRREG(d0), 0);
                VINSERTW(VRREG(v0), VRREG(d0), (3 - (u8 & 3)) * 4);
            }
            break;

        case 0x40:
            INST_NAME("VDPPS Gx, Vx, Ex, Ib");
            nextop = F8;
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            u8 = F8;
            {
                d0 = fpu_get_scratch(dyn);
                d1 = fpu_get_scratch(dyn);
                d2 = fpu_get_scratch(dyn);
                // Multiply all 4 single lanes
                XVMULSP(VSXREG(d0), VSXREG(v1), VSXREG(v2));
                // Zero out lanes not selected by input mask (bits 7:4)
                XXLXOR(VSXREG(d2), VSXREG(d2), VSXREG(d2));
                for (int i = 0; i < 4; ++i) {
                    if (!(u8 & (1 << (4 + i)))) {
                        // Zero x86 word i = BE byte offset (3-i)*4
                        VINSERTW(VRREG(d0), VRREG(d2), (3 - i) * 4);
                    }
                }
                // Horizontal sum: swap adjacent pairs, add
                VSLDOI(VRREG(d1), VRREG(d0), VRREG(d0), 4);
                XVADDSP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                // Sum pairs: VSLDOI by 8 bytes
                VSLDOI(VRREG(d1), VRREG(d0), VRREG(d0), 8);
                XVADDSP(VSXREG(d0), VSXREG(d0), VSXREG(d1));
                // Broadcast the sum to all 4 words
                VSPLTW(VRREG(v0), VRREG(d0), 0);
                // Zero out lanes not selected by output mask (bits 3:0)
                for (int i = 0; i < 4; ++i) {
                    if (!(u8 & (1 << i))) {
                        VINSERTW(VRREG(v0), VRREG(d2), (3 - i) * 4);
                    }
                }
            }
            break;

        case 0x4A:
            INST_NAME("VBLENDVPS Gx, Vx, Ex, XMMImm8");
            nextop = F8;
            u8 = geted_ib(dyn, addr, ninst, nextop) >> 4;
            d0 = avx_get_reg(dyn, ninst, x5, u8, 0, VMX_AVX_WIDTH_128);
            GETGY_empty_VYEY_xy(v0, v1, v2, 1);
            F8;
            // d0 holds the selector register; sign bit of each word selects
            q0 = fpu_get_scratch(dyn);
            // VCMPGTSW: compares signed words, result = -1 where A > B
            // Compare 0 > d0: gives -1 where d0 word is negative (sign bit set)
            XXLXOR(VSXREG(q0), VSXREG(q0), VSXREG(q0));  // q0 = 0
            VCMPGTSW(VRREG(q0), VRREG(q0), VRREG(d0));     // q0 = -1 where d0 < 0
            // XXSEL: for each bit, mask=1 picks B (v2/Ex), mask=0 picks A (v1/Vx)
            XXSEL(VSXREG(v0), VSXREG(v1), VSXREG(v2), VSXREG(q0));
            break;

        default:
            DEFAULT;
    }

    return addr;
}
