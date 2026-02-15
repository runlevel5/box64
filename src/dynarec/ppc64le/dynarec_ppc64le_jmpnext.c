#include <stdint.h>

#include "ppc64le_emitter.h"

#define EMIT(A)       \
    do {              \
        *block = (A); \
        ++block;      \
    } while (0)
void CreateJmpNext(void* addr, void* next)
{
    uint32_t* block = (uint32_t*)addr;
    // PPC64LE can't do PC-relative data load like ARM64's LDR literal.
    // We load the 64-bit address from the 'next' pointer using a 5-instruction
    // sequence (worst case), then branch via CTR.
    // However, 'next' here is a pointer to the jump target address stored
    // right after this code sequence, so we load it indirectly.
    // Actually, looking at the ARM64 version: LDRx_literal loads from a
    // PC-relative offset. For PPC64LE we need to load the 64-bit value
    // stored at 'next' into a register and branch to it.
    // The value at *next is the target address.
    // We use: load the address of 'next' into r12, then ld r12, 0(r12), then mtctr+bctr
    // But we need to materialize the address of 'next' itself...
    // Simpler approach: materialize the 64-bit value at *next directly.
    uint64_t target = *(uint64_t*)next;
    // Load 64-bit immediate into r12 (scratch, also used for function entry in ELFv2)
    // Use up to 5 instructions to load a 64-bit constant
    // lis r12, highest
    // ori r12, r12, higher
    // rldicr r12, r12, 32, 31
    // oris r12, r12, hi16
    // ori r12, r12, lo16
    int reg = 12;   // r12 is scratch
    uint16_t lo = target & 0xFFFF;
    uint16_t hi = (target >> 16) & 0xFFFF;
    uint16_t higher = (target >> 32) & 0xFFFF;
    uint16_t highest = (target >> 48) & 0xFFFF;

    LIS(reg, highest);
    ORI(reg, reg, higher);
    RLDICR(reg, reg, 32, 31);
    ORIS(reg, reg, hi);
    ORI(reg, reg, lo);
    MTCTR(reg);
    BCTR();
}
