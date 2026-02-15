#include <stddef.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "debug.h"
#include "dynablock.h"
#include "x64emu.h"
#include "emu/x64emu_private.h"
#include "emu/x64run_private.h"
#include "dynarec/dynablock_private.h"
#include "dynarec_ppc64le_arch.h"
#include "dynarec_ppc64le_functions.h"
#include "dynarec_native.h"

// PPC64LE arch.c — signal recovery metadata
// Stubbed initially: no arch-specific signal recovery yet.
// This means CHECK_FLAGS will be called on every signal in dynarec code,
// which is correct but slower than reconstructing from native state.

size_t get_size_arch(dynarec_ppc64le_t* dyn)
{
    (void)dyn;
    return 0;
}

void* populate_arch(dynarec_ppc64le_t* dyn, void* p, size_t sz)
{
    (void)dyn;
    (void)p;
    (void)sz;
    return NULL;
}

#ifndef _WIN32
void adjust_arch(dynablock_t* db, x64emu_t* emu, ucontext_t* p, uintptr_t x64pc)
{
    (void)db;
    (void)emu;
    (void)p;
    (void)x64pc;
    // Stub: just do CHECK_FLAGS for now
    if(db)
        CHECK_FLAGS(emu);
}
#endif

int arch_unaligned(dynablock_t* db, uintptr_t x64pc)
{
    (void)db;
    (void)x64pc;
    return 0;
}
