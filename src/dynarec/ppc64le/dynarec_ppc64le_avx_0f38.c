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

uintptr_t dynarec64_AVX_0F38(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, vex_t vex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    rex_t rex = vex.rex;
    MAYUSE(u8);

    switch (opcode) {
        case 0xF2:
            INST_NAME("ANDN Gd, Vd, Ed");
            nextop = F8;
            DEFAULT;
            break;
        case 0xF3:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 1:
                    INST_NAME("BLSR Vd, Ed");
                    DEFAULT;
                    break;
                case 2:
                    INST_NAME("BLSMSK Vd, Ed");
                    DEFAULT;
                    break;
                case 3:
                    INST_NAME("BLSI Vd, Ed");
                    DEFAULT;
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xF5:
            INST_NAME("BZHI Gd, Ed, Vd");
            nextop = F8;
            DEFAULT;
            break;
        case 0xF7:
            INST_NAME("BEXTR Gd, Vd, Ed");
            nextop = F8;
            DEFAULT;
            break;
        default:
            DEFAULT;
    }

    return addr;
}
