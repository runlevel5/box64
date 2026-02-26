#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#include "ppc64le_mapping.h"
#include "x64_signals.h"
#include "os.h"
#include "debug.h"
#include "box64context.h"
#include "box64cpu.h"
#include "emu/x64emu_private.h"
#include "ppc64le_emitter.h"
#include "x64emu.h"
#include "box64stack.h"
#include "callback.h"
#include "bridge.h"
#include "emu/x64run_private.h"
#include "x64trace.h"
#include "dynarec_native.h"
#include "custommem.h"
#include "alternate.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "dynarec_ppc64le_functions.h"
#include "../dynarec_helper.h"

int isSimpleWrapper(wrapper_t fun);
int isRetX87Wrapper(wrapper_t fun);

uintptr_t dynarec64_00(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    uint8_t nextop, opcode;
    uint8_t gd, ed, tmp1, tmp2, tmp3;
    int64_t j64;
    int v0, v1;
    int i32;
    int64_t i64;
    int32_t tmp;
    MAYUSE(tmp1);
    MAYUSE(tmp2);
    MAYUSE(tmp3);
    MAYUSE(j64);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(i32);
    MAYUSE(i64);
    MAYUSE(tmp);

    opcode = F8;

    switch (opcode) {
        default:
            DEFAULT;
    }

    return addr;
}
