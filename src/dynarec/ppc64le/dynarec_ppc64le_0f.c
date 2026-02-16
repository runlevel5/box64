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
#include "emu/x64shaext.h"
#include "bitutils.h"
#include "freq.h"

#include "ppc64le_printer.h"
#include "dynarec_ppc64le_private.h"
#include "elfloader.h"
#include "../dynarec_helper.h"
#include "dynarec_ppc64le_functions.h"


uintptr_t dynarec64_0F(dynarec_ppc64le_t* dyn, uintptr_t addr, uintptr_t ip, int ninst, rex_t rex, int* ok, int* need_epilog)
{
    (void)ip;
    (void)need_epilog;

    uint8_t opcode = F8;
    uint8_t nextop, u8;
    uint8_t gd, ed;
    uint8_t wback, wb1, wb2, gback;
    uint8_t eb1, eb2;
    uint8_t gb1, gb2;
    uint8_t tmp1, tmp2, tmp3;
    int32_t i32, i32_;
    int64_t j64;
    int64_t fixedaddress, gdoffset;
    int unscaled;
    int lock;
    int cacheupd = 0;
    int v0, v1, q0, q1, d0, d1;
    MAYUSE(u8);
    MAYUSE(eb1);
    MAYUSE(eb2);
    MAYUSE(gb1);
    MAYUSE(gb2);
    MAYUSE(wb1);
    MAYUSE(wb2);
    MAYUSE(j64);
    MAYUSE(lock);
    MAYUSE(gdoffset);
    MAYUSE(v0);
    MAYUSE(v1);
    MAYUSE(q0);
    MAYUSE(q1);
    MAYUSE(d0);
    MAYUSE(d1);

    switch (opcode) {

        case 0x01:
            // TODO:, /0 is SGDT. While 0F 01 D0 is XGETBV, etc...
            nextop = F8;
            if (MODREG) {
                switch (nextop) {
                    case 0xD0:
                        INST_NAME("XGETBV");
                        BEQZ_MARK(xRCX);
                        TRAP();  // illegal if ECX != 0
                        MARK;
                        MOV32w(xRAX, 0b111);
                        MOV32w(xRDX, 0);
                        break;
                    default:
                        DEFAULT;
                }
            } else {
                switch ((nextop >> 3) & 7) {
                    default:
                        DEFAULT;
                }
            }
            break;
        case 0x05:
            INST_NAME("SYSCALL");
            NOTEST(x1);
            SMEND();
            GETIP(addr, x7);
            STORE_XEMU_CALL();
            if(!box64_wine || FindElfAddress(my_context, ip)) {
                CALL_S(const_x64syscall_linux, -1, 0);
            } else {
                CALL_S(const_x64syscall, -1, 0);
            }
            LOAD_XEMU_CALL();
            TABLE64(x3, addr); // expected return address
            BNE_MARK(xRIP, x3);
            LWZ(x1, offsetof(x64emu_t, quit), xEmu);
            CBZ_NEXT(x1);
            MARK;
            LOAD_XEMU_REM();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            break;

        case 0x0B:
            INST_NAME("UD2");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_PEND);
            } else {
                SETFLAGS(X_ALL, SF_SET_NODF, NAT_FLAGS_NOFUSION); // Hack to set flags in "don't care" state
            }
            GETIP(ip, x7);
            STORE_XEMU_CALL();
            CALL(const_native_ud, -1, 0, 0);
            LOAD_XEMU_CALL();
            jump_to_epilog(dyn, 0, xRIP, ninst);
            *need_epilog = 0;
            *ok = 0;
            break;
        case 0x0D:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 0:
                    INST_NAME("PREFETCH");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    NOP();  // prefetch hint, treat as NOP on PPC64LE
                    break;
                case 1:
                    INST_NAME("PREFETCHW");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    NOP();
                    break;
                case 2:
                    INST_NAME("PREFETCHWT1");
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                    NOP();
                    break;
                default: // NOP
                    FAKEED;
                    break;
            }
            break;

        case 0x18:
            nextop = F8;
            if (MODREG) {
                INST_NAME("NOP (multibyte)");
            } else
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("PREFETCHNTA Ed");
                        FAKEED;
                        break;
                    case 1:
                        INST_NAME("PREFETCHT0 Ed");
                        addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 0, 0);
                        NOP();  // prefetch hint
                        break;
                    case 2:
                        INST_NAME("PREFETCHT1 Ed");
                        FAKEED;
                        break;
                    case 3:
                        INST_NAME("PREFETCHT2 Ed");
                        FAKEED;
                        break;
                    default:
                        INST_NAME("NOP (multibyte)");
                        FAKEED;
                }
            break;
        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            INST_NAME("NOP (multibyte)");
            nextop = F8;
            FAKEED;
            break;

        case 0x10:
            INST_NAME("MOVUPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x11:
            INST_NAME("MOVUPS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x28:
            INST_NAME("MOVAPS Gx, Ex");
            nextop = F8;
            GETG;
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg(dyn, ninst, x1, ed, 0);
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                XXLOR(VSXREG(v0), VSXREG(v1), VSXREG(v1));
            } else {
                v0 = sse_get_reg_empty(dyn, ninst, x1, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                LXV(VSXREG(v0), fixedaddress, ed);
            }
            break;
        case 0x29:
            INST_NAME("MOVAPS Ex, Gx");
            nextop = F8;
            GETG;
            v0 = sse_get_reg(dyn, ninst, x1, gd, 0);
            if (MODREG) {
                ed = (nextop & 7) + (rex.b << 3);
                v1 = sse_get_reg_empty(dyn, ninst, x1, ed);
                XXLOR(VSXREG(v1), VSXREG(v0), VSXREG(v0));
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x3, &fixedaddress, rex, NULL, 1, 0);
                STXV(VSXREG(v0), fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0x38:
            nextop = F8;
            switch (nextop) {
                case 0xF0:
                    INST_NAME("MOVBE Gd, Ed");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    LDxw(gd, ed, fixedaddress);
                    REVBxw(gd, gd, x1);
                    break;
                case 0xF1:
                    INST_NAME("MOVBE Ed, Gd");
                    nextop = F8;
                    GETGD;
                    SMREAD();
                    addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                    REVBxw(x1, gd, x3);
                    SDxw(x1, wback, fixedaddress);
                    SMWRITE2();
                    break;
                default:
                    DEFAULT;
            }
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                               \
        GETFLAGS;                                                                            \
    }                                                                                        \
    nextop = F8;                                                                             \
    GETGD;                                                                                   \
    if (MODREG) {                                                                            \
        ed = TO_NAT((nextop & 7) + (rex.b << 3));                                            \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP(NATNO, 8);                                                            \
        } else {                                                                             \
            B##NO##_MARK2(tmp1);                                                             \
        }                                                                                    \
        MR(gd, ed);                                                                          \
        if (!rex.w) ZEROUP(gd);                                                              \
        MARK2;                                                                               \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp3, &fixedaddress, rex, NULL, 1, 0); \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP(NATNO, 8);                                                            \
        } else {                                                                             \
            B##NO##_MARK2(tmp1);                                                             \
        }                                                                                    \
        LDxw(gd, ed, fixedaddress);                                                          \
        if (!rex.w) ZEROUP(gd);                                                              \
        MARK2;                                                                               \
    }

            GOCOND(0x40, "CMOV", "Gd, Ed");

#undef GO

        case 0x31:
            INST_NAME("RDTSC");
            NOTEST(x1);
            if (box64_rdtsc) {
                CALL(const_readtsc, x3, 0, 0); // will return the u64 in x3
            } else {
                MFTB(x3);
            }
            if (box64_rdtsc_shift) {
                SLDI(x3, x3, box64_rdtsc_shift);
            }
            SRDI(xRDX, x3, 32);
            ZEROUP2(xRAX, x3);
            break;

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    i32_ = F32S;                                                                             \
    if (rex.is32bits)                                                                        \
        j64 = (uint32_t)(addr + i32_);                                                       \
    else                                                                                     \
        j64 = addr + i32_;                                                                   \
    JUMP(j64, 1);                                                                            \
    if (!dyn->insts[ninst].nat_flags_fusion) {                                               \
        GETFLAGS;                                                                            \
    }                                                                                        \
    if (dyn->insts[ninst].x64.jmp_insts == -1 || CHECK_CACHE()) {                            \
        /* out of the block */                                                               \
        i32 = dyn->insts[ninst].epilog - (dyn->native_size);                                 \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP_safe(NATNO, i32);                                                     \
        } else {                                                                             \
            B##NO##_safe(tmp1, i32);                                                         \
        }                                                                                    \
        if (dyn->insts[ninst].x64.jmp_insts == -1) {                                         \
            if (!(dyn->insts[ninst].x64.barrier & BARRIER_FLOAT))                            \
                fpu_purgecache(dyn, ninst, 1, tmp1, tmp2, tmp3);                             \
            jump_to_next(dyn, j64, 0, ninst, rex.is32bits);                                  \
        } else {                                                                             \
            CacheTransform(dyn, ninst, cacheupd, tmp1, tmp2, tmp3);                          \
            i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);  \
            B(i32);                                                                          \
        }                                                                                    \
    } else {                                                                                 \
        /* inside the block */                                                               \
        i32 = dyn->insts[dyn->insts[ninst].x64.jmp_insts].address - (dyn->native_size);      \
        if (dyn->insts[ninst].nat_flags_fusion) {                                            \
            NATIVEJUMP_safe(NATYES, i32);                                                    \
        } else {                                                                             \
            B##YES##_safe(tmp1, i32);                                                        \
        }                                                                                    \
    }

            GOCOND(0x80, "J", "Id");

#undef GO

#define GO(GETFLAGS, NO, YES, NATNO, NATYES, F, I)                                          \
    READFLAGS_FUSION(F, x1, x2, x3, x4, x5);                                                \
    nextop = F8;                                                                             \
    if (dyn->insts[ninst].nat_flags_fusion) {                                                \
        NATIVESET(NATYES, tmp3);                                                             \
    } else {                                                                                 \
        GETFLAGS;                                                                            \
        S##YES(tmp3, tmp1);                                                                  \
    }                                                                                        \
    if (MODREG) {                                                                            \
        if (rex.rex) {                                                                       \
            eb1 = TO_NAT((nextop & 7) + (rex.b << 3));                                       \
            eb2 = 0;                                                                         \
        } else {                                                                             \
            ed = (nextop & 7);                                                               \
            eb2 = (ed >> 2) * 8;                                                             \
            eb1 = TO_NAT(ed & 3);                                                            \
        }                                                                                    \
        RLDIMI(eb1, tmp3, eb2, 56 - eb2);                                                    \
    } else {                                                                                 \
        addr = geted(dyn, addr, ninst, nextop, &ed, tmp2, tmp1, &fixedaddress, rex, NULL, 1, 0); \
        STB(tmp3, fixedaddress, ed);                                                         \
        SMWRITE();                                                                           \
    }

            GOCOND(0x90, "SET", "Eb");

#undef GO

        case 0x6E:
            INST_NAME("MOVD Gm, Ed");
            nextop = F8;
            GETG;
            v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MTVSRD(VSXREG(v0), ed);
                } else {
                    RLWINM(x4, ed, 0, 0, 31);  // zero-extend 32-bit
                    MTVSRD(VSXREG(v0), x4);
                }
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w) {
                    LFD(v0, fixedaddress, wback);
                } else {
                    LWZ(x4, fixedaddress, wback);
                    MTVSRD(VSXREG(v0), x4);
                }
            }
            break;
        case 0x6F:
            INST_NAME("MOVQ Gm, Em");
            nextop = F8;
            GETG;
            if (MODREG) {
                v1 = mmx_get_reg(dyn, ninst, x1, x2, x3, nextop & 7);
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                FMR(v0, v1);
            } else {
                v0 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, gd);
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LFD(v0, fixedaddress, wback);
            }
            break;

        case 0x77:
            INST_NAME("EMMS");
            // empty MMX, FPU now usable
            mmx_purgecache(dyn, ninst, 0, x1);
            break;

        case 0x7E:
            INST_NAME("MOVD Ed, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                if (rex.w) {
                    MFVSRD(ed, VSXREG(v0));
                } else {
                    MFVSRWZ(ed, VSXREG(v0));
                    ZEROUP(ed);
                }
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w) {
                    STFD(v0, fixedaddress, ed);
                } else {
                    MFVSRWZ(x4, VSXREG(v0));
                    STW(x4, fixedaddress, ed);
                }
                SMWRITE2();
            }
            break;
        case 0x7F:
            INST_NAME("MOVQ Em, Gm");
            nextop = F8;
            GETGM(v0);
            if (MODREG) {
                v1 = mmx_get_reg_empty(dyn, ninst, x1, x2, x3, nextop & 7);
                FMR(v1, v0);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x2, &fixedaddress, rex, NULL, 1, 0);
                STFD(v0, fixedaddress, ed);
                SMWRITE2();
            }
            break;

        case 0xA0:
            INST_NAME("PUSH FS");
            LHZ(x2, offsetof(x64emu_t, segs[_FS]), xEmu);
            PUSH1z(x2);
            break;
        case 0xA1:
            INST_NAME("POP FS");
            POP1z(x2);
            STH(x2, offsetof(x64emu_t, segs[_FS]), xEmu);
            CBZ_NEXT(x2);
            MOV32w(x1, _FS);
            CALL(const_getsegmentbase, -1, x1, 0);
            break;
        case 0xA2:
            INST_NAME("CPUID");
            NOTEST(x1);
            CALL_(const_cpuid, -1, 0, 0, 0);
            // BX and DX are not synchronized during the call, so need to force the update
            LD(xRDX, offsetof(x64emu_t, regs[_DX]), xEmu);
            LD(xRBX, offsetof(x64emu_t, regs[_BX]), xEmu);
            break;
        case 0xA8:
            INST_NAME("PUSH GS");
            LHZ(x2, offsetof(x64emu_t, segs[_GS]), xEmu);
            PUSH1z(x2);
            break;
        case 0xA9:
            INST_NAME("POP GS");
            POP1z(x2);
            STH(x2, offsetof(x64emu_t, segs[_GS]), xEmu);
            CBZ_NEXT(x2);
            MOV32w(x1, _GS);
            CALL(const_getsegmentbase, -1, x1, 0);
            break;

        case 0xA3:
            INST_NAME("BT Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                ALSLy(x3, x1, wback, 2 + rex.w); // (&ed) += r1*4;
                LDxw(x1, x3, fixedaddress);
                ed = x1;
            }
            IFX (X_CF) {
                ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
                SRLxw(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            break;

        case 0xAB:
            INST_NAME("BTS Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                ALSLy(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            IFX (X_CF) {
                SRD(x4, ed, x2);
                RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            }
            LI(x4, 1);
            SLD(x4, x4, x2);
            OR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;

        case 0xA4:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 63 : 31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8 & (rex.w ? 63 : 31);
                emit_shld32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                F8;
            }
            break;
        case 0xA5:
            nextop = F8;
            INST_NAME("SHLD Ed, Gd, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            CBZ_NEXT(x3);
            emit_shld32(dyn, ninst, rex, ed, gd, x3, x4, x5, x6);
            WBACK;
            break;
        case 0xAC:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, Ib");
            if (geted_ib(dyn, addr, ninst, nextop) & (rex.w ? 63 : 31)) {
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
                GETED(1);
                GETGD;
                u8 = F8 & (rex.w ? 63 : 31);
                emit_shrd32c(dyn, ninst, rex, ed, gd, u8, x3, x4);
                WBACK;
            } else {
                FAKEED;
                if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
                F8;
            }
            break;
        case 0xAD:
            nextop = F8;
            INST_NAME("SHRD Ed, Gd, CL");
            if (BOX64DRENV(dynarec_safeflags) > 1) {
                READFLAGS(X_ALL);
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_FUSION);
            } else
                SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            GETGD;
            GETED(0);
            if (!rex.w && !rex.is32bits && MODREG) { ZEROUP(ed); }
            ANDI(x3, xRCX, rex.w ? 0x3f : 0x1f);
            CBZ_NEXT(x3);
            emit_shrd32(dyn, ninst, rex, ed, gd, x3, x5, x4, x6);
            WBACK;
            break;

        case 0xAE:
            nextop = F8;
            if (MODREG)
                switch (nextop) {
                    case 0xE8:
                        INST_NAME("LFENCE");
                        LWSYNC();
                        break;
                    case 0xF0:
                        INST_NAME("MFENCE");
                        SYNC();
                        break;
                    case 0xF8:
                        INST_NAME("SFENCE");
                        LWSYNC();
                        break;
                    default:
                        DEFAULT;
                }
            else
                switch ((nextop >> 3) & 7) {
                    case 0:
                        INST_NAME("FXSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                        CALL(rex.is32bits ? const_fpu_fxsave32 : const_fpu_fxsave64, -1, ed, 0);
                        break;
                    case 1:
                        INST_NAME("FXRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x3, &fixedaddress, rex, NULL, 0, 0);
                        CALL(rex.is32bits ? const_fpu_fxrstor32 : const_fpu_fxrstor64, -1, ed, 0);
                        break;
                    case 2:
                        INST_NAME("LDMXCSR Md");
                        GETED(0);
                        STW(ed, offsetof(x64emu_t, mxcsr), xEmu);
                        break;
                    case 3:
                        INST_NAME("STMXCSR Md");
                        addr = geted(dyn, addr, ninst, nextop, &wback, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        LWZ(x4, offsetof(x64emu_t, mxcsr), xEmu);
                        STW(x4, fixedaddress, wback);
                        break;
                    case 4:
                        INST_NAME("XSAVE Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xsave, -1, ed, x2);
                        break;
                    case 5:
                        INST_NAME("XRSTOR Ed");
                        MESSAGE(LOG_DUMP, "Need Optimization\n");
                        BARRIER(BARRIER_FLOAT);
                        addr = geted(dyn, addr, ninst, nextop, &ed, x1, x2, &fixedaddress, rex, NULL, 0, 0);
                        MOV32w(x2, rex.w ? 0 : 1);
                        CALL(const_fpu_xrstor, -1, ed, x2);
                        break;
                    case 7:
                        INST_NAME("CLFLUSH Ed");
                        FAKEED;
                        SYNC();
                        break;
                    default:
                        DEFAULT;
                }
            break;
        case 0xAF:
            INST_NAME("IMUL Gd, Ed");
            SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            CLEAR_FLAGS(x3);
            if (rex.w) {
                UFLAG_IF {
                    MULHD(x3, gd, ed);
                    MULLD(gd, gd, ed);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        SRADI(x4, gd, 63);
                        XOR(x3, x3, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                        IFX (X_OF) RLDIMI(xFlags, x3, F_OF, 63 - F_OF);
                    }
                } else {
                    MULLD(gd, gd, ed);
                }
            } else {
                UFLAG_IF {
                    EXTSW(gd, gd);
                    EXTSW(x3, ed);
                    MULLD(x4, gd, x3);
                    SRDI(x3, x4, 32);
                    EXTSW(gd, x4);
                    SET_DFNONE();
                    IFX (X_CF | X_OF) {
                        XOR(x3, gd, x4);
                        SNEZ(x3, x3);
                        IFX (X_CF) RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                        IFX (X_OF) RLDIMI(xFlags, x3, F_OF, 63 - F_OF);
                    }
                } else {
                    MULLW(gd, gd, ed);
                }
                ZEROUP(gd);
            }
            IFX (X_SF) {
                SRDI(x3, gd, rex.w ? 63 : 31);
                RLDIMI(xFlags, x3, F_SF, 63 - F_SF);
            }
            IFX (X_PF) emit_pf(dyn, ninst, gd, x3, x4);
            IFX (X_ALL) SPILL_EFLAGS();
            break;

        case 0xB3:
            INST_NAME("BTR Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                ALSLy(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRD(x4, ed, x2);
            RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            LI(x4, 1);
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SLD(x4, x4, x2);
            ANDC(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;

        case 0xB6:
            INST_NAME("MOVZX Gd, Eb");
            nextop = F8;
            GETGD;
            SCRATCH_USAGE(0);
            if (MODREG) {
                if (rex.rex) {
                    eb1 = TO_NAT((nextop & 7) + (rex.b << 3));
                    eb2 = 0;
                } else {
                    ed = (nextop & 7);
                    eb1 = TO_NAT(ed & 3); // Ax, Cx, Dx or Bx
                    eb2 = (ed & 4) >> 2;  // L or H
                }
                RLDICL(gd, eb1, 64 - eb2 * 8, 56);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LBZ(gd, fixedaddress, ed);
            }
            break;
        case 0xB7:
            INST_NAME("MOVZX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                RLDICL(gd, ed, 0, 48);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                LHZ(gd, fixedaddress, ed);
            }
            break;
        case 0xBA:
            nextop = F8;
            switch ((nextop >> 3) & 7) {
                case 4:
                    INST_NAME("BT Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    RLDICL(x4, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    break;
                case 5:
                    INST_NAME("BTS Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    RLDICL(x4, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    LI(x4, 1);
                    SLDI(x4, x4, u8);
                    OR(ed, ed, x4);
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                case 6:
                    INST_NAME("BTR Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= (rex.w ? 0x3f : 0x1f);
                    RLDICL(x4, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
                    LI(x4, 1);
                    SLDI(x4, x4, u8);
                    ANDC(ed, ed, x4);
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                case 7:
                    INST_NAME("BTC Ed, Ib");
                    SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
                    SET_DFNONE();
                    GETED(1);
                    u8 = F8;
                    u8 &= rex.w ? 0x3f : 0x1f;
                    RLDICL(x3, ed, 64 - u8, 63);
                    RLDIMI(xFlags, x3, F_CF, 63 - F_CF);
                    if (u8 <= 15) {
                        XORI(ed, ed, (1LL << u8));
                    } else {
                        MOV64xw(x3, (1LL << u8));
                        XOR(ed, ed, x3);
                    }
                    if (wback) {
                        SDxw(ed, wback, fixedaddress);
                        SMWRITE();
                    } else if (!rex.w) {
                        ZEROUP(ed);
                    }
                    break;
                default:
                    DEFAULT;
            }
            break;
        case 0xBB:
            INST_NAME("BTC Ed, Gd");
            SETFLAGS(X_CF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            SET_DFNONE();
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                wback = 0;
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &wback, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                if (rex.w)
                    SRADI(x1, gd, 6);
                else
                    SRAWI(x1, gd, 5);
                if (!rex.w && !rex.is32bits) { EXTSW(x1, x1); }
                ALSLy(x3, x1, wback, 2 + rex.w);
                LDxw(x1, x3, fixedaddress);
                ed = x1;
                wback = x3;
            }
            ANDI(x2, gd, rex.w ? 0x3f : 0x1f);
            SRD(x4, ed, x2);
            RLDIMI(xFlags, x4, F_CF, 63 - F_CF);
            LI(x4, 1);
            SLD(x4, x4, x2);
            XOR(ed, ed, x4);
            if (wback) {
                SDxw(ed, wback, fixedaddress);
                SMWRITE();
            } else if (!rex.w) {
                ZEROUP(ed);
            }
            break;
        case 0xBC:
            INST_NAME("BSF Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS(x2);
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            BNEZ_MARK(ed);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            if (rex.w) {
                CNTTZD(gd, ed);
            } else {
                CNTTZW(gd, ed);
            }
            MARK2;
            IFX (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, gd, x2, x5);
            }
            SPILL_EFLAGS();
            break;
        case 0xBD:
            INST_NAME("BSR Gd, Ed");
            if (!BOX64DRENV(dynarec_safeflags)) {
                SETFLAGS(X_ZF, SF_SUBSET, NAT_FLAGS_NOFUSION);
            } else {
                SETFLAGS(X_ALL, SF_SET, NAT_FLAGS_NOFUSION);
            }
            SET_DFNONE();
            CLEAR_FLAGS(x2);
            nextop = F8;
            GETED(0);
            GETGD;
            if (!rex.w && MODREG) {
                ZEROUP2(x4, ed);
                ed = x4;
            }
            BNEZ_MARK(ed);
            IFX (X_ZF) ORI(xFlags, xFlags, 1 << F_ZF);
            B_MARK2_nocond;
            MARK;
            if (rex.w) {
                CNTLZD(gd, ed);
            } else {
                CNTLZW(gd, ed);
            }
            LI(x1, rex.w ? 63 : 31);
            SUB(gd, x1, gd);
            MARK2;
            if (BOX64DRENV(dynarec_safeflags)) {
                IFX (X_PF) emit_pf(dyn, ninst, gd, x2, x5);
            }
            SPILL_EFLAGS();
            break;
        case 0xBE:
            INST_NAME("MOVSX Gd, Eb");
            nextop = F8;
            GETGD;
            if (MODREG) {
                if (rex.rex) {
                    wback = TO_NAT((nextop & 7) + (rex.b << 3));
                    wb2 = 0;
                } else {
                    wback = (nextop & 7);
                    wb2 = (wback >> 2) * 8;
                    wback = TO_NAT(wback & 3);
                }
                RLDICL(gd, wback, 64 - wb2, 56);
                EXTSB(gd, gd);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LBZ(gd, fixedaddress, ed);
                EXTSB(gd, gd);
            }
            if (!rex.w) ZEROUP(gd);
            break;
        case 0xBF:
            INST_NAME("MOVSX Gd, Ew");
            nextop = F8;
            GETGD;
            if (MODREG) {
                ed = TO_NAT((nextop & 7) + (rex.b << 3));
                EXTSH(gd, ed);
            } else {
                SMREAD();
                addr = geted(dyn, addr, ninst, nextop, &ed, x3, x1, &fixedaddress, rex, NULL, 1, 0);
                LHA(gd, fixedaddress, ed);
            }
            if (!rex.w) ZEROUP(gd);
            break;

        case 0xC0:
            INST_NAME("XADD Eb, Gb");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGB(x1);
            GETEB(x2, 0);
            gd = x2;
            ed = x1;
            emit_add8(dyn, ninst, ed, gd, x4, x5);
            GBBACK();
            EBBACK();
            break;
        case 0xC1:
            INST_NAME("XADD Ed, Gd");
            SETFLAGS(X_ALL, SF_SET_PENDING, NAT_FLAGS_FUSION);
            nextop = F8;
            GETGD;
            GETED(0);
            if (ed != gd) MV(x7, ed);
            emit_add32(dyn, ninst, rex, ed, gd, x4, x5, x6);
            if (ed != gd) MVxw(gd, x7);
            WBACK;
            break;
        case 0xC3:
            INST_NAME("MOVNTI Ed, Gd");
            nextop = F8;
            GETGD;
            if (MODREG) {
                MVxw(TO_NAT((nextop & 7) + (rex.b << 3)), gd);
            } else {
                addr = geted(dyn, addr, ninst, nextop, &ed, x2, x1, &fixedaddress, rex, NULL, 1, 0);
                SDxw(gd, ed, fixedaddress);
            }
            break;
        case 0xC8:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCC:
        case 0xCD:
        case 0xCE:
        case 0xCF:
            INST_NAME("BSWAP Reg");
            gd = TO_NAT((opcode & 7) + (rex.b << 3));
            REVBxw(gd, gd, x1);
            break;

        default:
            DEFAULT;
    }

    return addr;
}
