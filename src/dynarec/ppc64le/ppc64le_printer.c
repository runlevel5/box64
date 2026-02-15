#include <stdio.h>
#include <stdint.h>

#include "ppc64le_printer.h"

// Minimal PPC64LE instruction disassembler stub.
// TODO: Implement full disassembly for debugging output.
const char* ppc64le_print(uint32_t opcode, uint64_t addr)
{
    static char buff[64];
    snprintf(buff, sizeof(buff), "%08X", opcode);
    return buff;
}
