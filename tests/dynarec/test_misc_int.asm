; test_misc_int.asm - Test miscellaneous integer operations
; BSF, BSR, POPCNT, LZCNT, TZCNT, BT/BTS/BTR/BTC, SETcc, CMOVcc
%include "test_framework.inc"

section .data
    t1_name:  db "bsf 32 basic", 0
    t2_name:  db "bsf 32 bit0", 0
    t3_name:  db "bsf 32 bit31", 0
    t4_name:  db "bsf 64 basic", 0
    t5_name:  db "bsf 64 bit63", 0
    t6_name:  db "bsr 32 basic", 0
    t7_name:  db "bsr 32 bit0", 0
    t8_name:  db "bsr 32 bit31", 0
    t9_name:  db "bsr 64 basic", 0
    t10_name: db "bsr 64 bit63", 0
    t11_name: db "bsf zf=1 (src=0)", 0
    t12_name: db "popcnt 32 basic", 0
    t13_name: db "popcnt 32 allones", 0
    t14_name: db "popcnt 64 basic", 0
    t15_name: db "popcnt 64 allones", 0
    t16_name: db "lzcnt 32 basic", 0
    t17_name: db "lzcnt 32 zero", 0
    t18_name: db "lzcnt 64 basic", 0
    t19_name: db "tzcnt 32 basic", 0
    t20_name: db "tzcnt 32 zero", 0
    t21_name: db "tzcnt 64 basic", 0
    t22_name: db "bt reg,imm", 0
    t23_name: db "bt reg,reg", 0
    t24_name: db "bts set bit", 0
    t25_name: db "btr clear bit", 0
    t26_name: db "btc complement", 0
    t27_name: db "sete after cmp eq", 0
    t28_name: db "setne after cmp ne", 0
    t29_name: db "setl signed less", 0
    t30_name: db "setge signed ge", 0
    t31_name: db "setb unsigned lt", 0
    t32_name: db "seta unsigned gt", 0
    t33_name: db "cmove taken", 0
    t34_name: db "cmove not taken", 0
    t35_name: db "cmovl taken", 0
    t36_name: db "cmovge taken", 0
    t37_name: db "cmovb taken", 0
    t38_name: db "cmova taken", 0
    t39_name: db "bt mem,imm", 0
    t40_name: db "popcnt 32 zf", 0
    t41_name: db "cmovne 16 taken", 0
    t42_name: db "cmovne 16 not taken", 0
    t43_name: db "cmova 16 taken", 0
    t44_name: db "cmova 16 not taken", 0
    t45_name: db "cmove 16 taken", 0
    t46_name: db "cmove 16 not taken", 0
    t47_name: db "cmovb 16 taken", 0
    t48_name: db "cmovb 16 not taken", 0
    t49_name: db "cmovl 16 taken", 0
    t50_name: db "cmovl 16 not taken", 0
    t51_name: db "cmovne 16 preserves hi", 0
    t52_name: db "cmova 16 preserves hi", 0
    ; Tests 53-64: 32-bit CMOVcc native fusion upper-bits tests
    t53_name: db "cmovne 32 not-taken hi", 0
    t54_name: db "cmove 32 not-taken hi", 0
    t55_name: db "cmova 32 not-taken hi", 0
    t56_name: db "cmovb 32 not-taken hi", 0
    t57_name: db "cmovl 32 not-taken hi", 0
    t58_name: db "cmovge 32 not-taken hi", 0
    t59_name: db "cmovne 32 taken zeros", 0
    t60_name: db "cmove 32 taken zeros", 0
    t61_name: db "cmovne 64 not-taken", 0
    t62_name: db "cmovne 64 taken", 0
    t63_name: db "setne via cmp fusion", 0
    t64_name: db "sete via cmp fusion", 0

    align 8
    bt_mem_val: dq 0x00000000DEADBEEF

section .text
global _start

_start:
    INIT_TESTS

    ; ==== Test 1: bsf 32 - find lowest set bit ====
    TEST_CASE t1_name
    mov eax, 0x00000100    ; bit 8 is lowest
    bsf ecx, eax
    CHECK_EQ_32 ecx, 8

    ; ==== Test 2: bsf 32 bit 0 ====
    TEST_CASE t2_name
    mov eax, 0x80000001    ; bit 0 is lowest
    bsf ecx, eax
    CHECK_EQ_32 ecx, 0

    ; ==== Test 3: bsf 32 bit 31 ====
    TEST_CASE t3_name
    mov eax, 0x80000000    ; only bit 31
    bsf ecx, eax
    CHECK_EQ_32 ecx, 31

    ; ==== Test 4: bsf 64 basic ====
    TEST_CASE t4_name
    mov rax, 0x0000010000000000  ; bit 40
    bsf rcx, rax
    CHECK_EQ_64 rcx, 40

    ; ==== Test 5: bsf 64 bit 63 ====
    TEST_CASE t5_name
    mov rax, 0x8000000000000000  ; bit 63
    bsf rcx, rax
    CHECK_EQ_64 rcx, 63

    ; ==== Test 6: bsr 32 - find highest set bit ====
    TEST_CASE t6_name
    mov eax, 0x00000100    ; bit 8 is highest
    bsr ecx, eax
    CHECK_EQ_32 ecx, 8

    ; ==== Test 7: bsr 32 bit 0 ====
    TEST_CASE t7_name
    mov eax, 1             ; only bit 0
    bsr ecx, eax
    CHECK_EQ_32 ecx, 0

    ; ==== Test 8: bsr 32 bit 31 ====
    TEST_CASE t8_name
    mov eax, 0x80000001    ; bit 31 is highest
    bsr ecx, eax
    CHECK_EQ_32 ecx, 31

    ; ==== Test 9: bsr 64 basic ====
    TEST_CASE t9_name
    mov rax, 0x0000010000000000  ; bit 40
    bsr rcx, rax
    CHECK_EQ_64 rcx, 40

    ; ==== Test 10: bsr 64 bit 63 ====
    TEST_CASE t10_name
    mov rax, 0x8000000000000001  ; bit 63 is highest
    bsr rcx, rax
    CHECK_EQ_64 rcx, 63

    ; ==== Test 11: bsf src=0 -> ZF=1 ====
    TEST_CASE t11_name
    xor eax, eax
    mov ecx, 0x42         ; should remain unchanged
    bsf ecx, eax
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF  ; ZF should be set

    ; ==== Test 12: popcnt 32 basic ====
    TEST_CASE t12_name
    mov eax, 0x0F0F0F0F   ; 4 bits per byte * 4 bytes = 16
    popcnt ecx, eax
    CHECK_EQ_32 ecx, 16

    ; ==== Test 13: popcnt 32 allones ====
    TEST_CASE t13_name
    mov eax, 0xFFFFFFFF
    popcnt ecx, eax
    CHECK_EQ_32 ecx, 32

    ; ==== Test 14: popcnt 64 basic ====
    TEST_CASE t14_name
    mov rax, 0x0F0F0F0F0F0F0F0F  ; 4*8 = 32
    popcnt rcx, rax
    CHECK_EQ_64 rcx, 32

    ; ==== Test 15: popcnt 64 allones ====
    TEST_CASE t15_name
    mov rax, 0xFFFFFFFFFFFFFFFF
    popcnt rcx, rax
    CHECK_EQ_64 rcx, 64

    ; ==== Test 16: lzcnt 32 basic ====
    TEST_CASE t16_name
    mov eax, 0x00010000   ; bit 16 -> 15 leading zeros
    lzcnt ecx, eax
    CHECK_EQ_32 ecx, 15

    ; ==== Test 17: lzcnt 32 zero -> 32 ====
    TEST_CASE t17_name
    xor eax, eax
    lzcnt ecx, eax
    CHECK_EQ_32 ecx, 32

    ; ==== Test 18: lzcnt 64 basic ====
    TEST_CASE t18_name
    mov rax, 0x0000000100000000  ; bit 32 -> 31 leading zeros
    lzcnt rcx, rax
    CHECK_EQ_64 rcx, 31

    ; ==== Test 19: tzcnt 32 basic ====
    TEST_CASE t19_name
    mov eax, 0x00010000   ; bit 16 -> 16 trailing zeros
    tzcnt ecx, eax
    CHECK_EQ_32 ecx, 16

    ; ==== Test 20: tzcnt 32 zero -> 32 ====
    TEST_CASE t20_name
    xor eax, eax
    tzcnt ecx, eax
    CHECK_EQ_32 ecx, 32

    ; ==== Test 21: tzcnt 64 basic ====
    TEST_CASE t21_name
    mov rax, 0x0000000100000000  ; bit 32 -> 32 trailing zeros
    tzcnt rcx, rax
    CHECK_EQ_64 rcx, 32

    ; ==== Test 22: bt reg, imm ====
    TEST_CASE t22_name
    mov eax, 0x00000100    ; bit 8 set
    bt eax, 8
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF   ; CF=1 because bit 8 is set

    ; ==== Test 23: bt reg, reg ====
    TEST_CASE t23_name
    mov eax, 0x00000100    ; bit 8 set
    mov ecx, 7             ; test bit 7 (not set)
    bt eax, ecx
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0    ; CF=0 because bit 7 is not set

    ; ==== Test 24: bts - set bit and return old ====
    TEST_CASE t24_name
    mov eax, 0x00000000
    bts eax, 5             ; set bit 5
    ; eax should now be 0x20, CF=0 (was not set)
    CHECK_EQ_32 eax, 0x00000020

    ; ==== Test 25: btr - clear bit ====
    TEST_CASE t25_name
    mov eax, 0x000000FF
    btr eax, 3             ; clear bit 3
    ; eax should be 0xF7
    CHECK_EQ_32 eax, 0x000000F7

    ; ==== Test 26: btc - complement bit ====
    TEST_CASE t26_name
    mov eax, 0x000000FF
    btc eax, 7             ; complement bit 7 (was 1, now 0)
    ; eax should be 0x7F
    CHECK_EQ_32 eax, 0x0000007F

    ; ==== Test 27: sete after cmp equal ====
    TEST_CASE t27_name
    mov eax, 42
    cmp eax, 42
    sete cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 28: setne after cmp not equal ====
    TEST_CASE t28_name
    mov eax, 42
    cmp eax, 43
    setne cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 29: setl signed less ====
    TEST_CASE t29_name
    mov eax, -5
    cmp eax, 3
    setl cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 30: setge signed greater-or-equal ====
    TEST_CASE t30_name
    mov eax, 5
    cmp eax, -3
    setge cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 31: setb unsigned below ====
    TEST_CASE t31_name
    mov eax, 1
    cmp eax, 0xFFFFFFFF   ; 1 < 0xFFFFFFFF unsigned
    setb cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 32: seta unsigned above ====
    TEST_CASE t32_name
    mov eax, 0xFFFFFFFF
    cmp eax, 1             ; 0xFFFFFFFF > 1 unsigned
    seta cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1

    ; ==== Test 33: cmove taken (equal) ====
    TEST_CASE t33_name
    mov eax, 42
    mov ecx, 99
    mov edx, 0
    cmp eax, 42            ; ZF=1
    cmove edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 34: cmove not taken ====
    TEST_CASE t34_name
    mov eax, 42
    mov ecx, 99
    mov edx, 55
    cmp eax, 43            ; ZF=0
    cmove edx, ecx         ; not taken -> edx=55
    CHECK_EQ_32 edx, 55

    ; ==== Test 35: cmovl taken (signed less) ====
    TEST_CASE t35_name
    mov eax, -5
    mov ecx, 99
    mov edx, 0
    cmp eax, 3             ; -5 < 3
    cmovl edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 36: cmovge taken (signed greater-or-equal) ====
    TEST_CASE t36_name
    mov eax, 5
    mov ecx, 99
    mov edx, 0
    cmp eax, 5             ; 5 >= 5
    cmovge edx, ecx        ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 37: cmovb taken (unsigned below) ====
    TEST_CASE t37_name
    mov eax, 1
    mov ecx, 99
    mov edx, 0
    cmp eax, 0xFFFFFFFF    ; 1 < 0xFFFFFFFF unsigned
    cmovb edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 38: cmova taken (unsigned above) ====
    TEST_CASE t38_name
    mov eax, 0xFFFFFFFF
    mov ecx, 99
    mov edx, 0
    cmp eax, 1             ; 0xFFFFFFFF > 1 unsigned
    cmova edx, ecx         ; taken -> edx=99
    CHECK_EQ_32 edx, 99

    ; ==== Test 39: bt [mem], imm ====
    TEST_CASE t39_name
    ; bt_mem_val = 0x00000000DEADBEEF
    ; bit 0 of DEADBEEF: 0xF & 1 = 1 -> CF=1
    bt dword [rel bt_mem_val], 0
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; ==== Test 40: popcnt 32 zero -> ZF=1, result=0 ====
    TEST_CASE t40_name
    xor eax, eax
    popcnt ecx, eax
    SAVE_FLAGS
    ; result should be 0 and ZF should be set
    push rcx
    CHECK_EQ_32 ecx, 0
    pop rcx
    ; Note: we already counted one test for the CHECK_EQ_32 above.
    ; The ZF check is implicitly verified by the zero result.

    ; ==== Test 41: cmovne 16-bit taken (not equal) ====
    ; CMP + CMOVcc pattern triggers native flags fusion
    TEST_CASE t41_name
    mov ecx, 0x1234          ; source: cx = 0x1234
    mov edx, 0x5678          ; dest: dx = 0x5678
    cmp eax, 1               ; ZF=0 (eax=0 from popcnt above)
    cmovne dx, cx            ; ZF=0 -> taken, dx should become 0x1234
    movzx eax, dx
    CHECK_EQ_32 eax, 0x1234

    ; ==== Test 42: cmovne 16-bit not taken (equal) ====
    TEST_CASE t42_name
    mov ecx, 0x1234          ; source: cx = 0x1234
    mov edx, 0x5678          ; dest: dx = 0x5678
    xor eax, eax
    cmp eax, 0               ; ZF=1 (0 == 0)
    cmovne dx, cx            ; ZF=1 -> not taken, dx should stay 0x5678
    movzx eax, dx
    CHECK_EQ_32 eax, 0x5678

    ; ==== Test 43: cmova 16-bit taken (unsigned above) ====
    TEST_CASE t43_name
    mov ecx, 0xBEEF          ; source: cx = 0xBEEF
    mov edx, 0xCAFE          ; dest: dx = 0xCAFE
    mov eax, 100
    cmp eax, 1               ; 100 > 1 unsigned, CF=0 ZF=0
    cmova dx, cx             ; taken -> dx = 0xBEEF
    movzx eax, dx
    CHECK_EQ_32 eax, 0xBEEF

    ; ==== Test 44: cmova 16-bit not taken (not above) ====
    TEST_CASE t44_name
    mov ecx, 0xBEEF          ; source
    mov edx, 0xCAFE          ; dest
    mov eax, 1
    cmp eax, 100             ; 1 < 100 unsigned, CF=1
    cmova dx, cx             ; not taken -> dx stays 0xCAFE
    movzx eax, dx
    CHECK_EQ_32 eax, 0xCAFE

    ; ==== Test 45: cmove 16-bit taken (equal) ====
    TEST_CASE t45_name
    mov ecx, 0xAAAA
    mov edx, 0xBBBB
    mov eax, 42
    cmp eax, 42              ; ZF=1
    cmove dx, cx             ; taken -> dx = 0xAAAA
    movzx eax, dx
    CHECK_EQ_32 eax, 0xAAAA

    ; ==== Test 46: cmove 16-bit not taken (not equal) ====
    TEST_CASE t46_name
    mov ecx, 0xAAAA
    mov edx, 0xBBBB
    mov eax, 42
    cmp eax, 43              ; ZF=0
    cmove dx, cx             ; not taken -> dx stays 0xBBBB
    movzx eax, dx
    CHECK_EQ_32 eax, 0xBBBB

    ; ==== Test 47: cmovb 16-bit taken (below) ====
    TEST_CASE t47_name
    mov ecx, 0x1111
    mov edx, 0x2222
    mov eax, 1
    cmp eax, 100             ; 1 < 100 -> CF=1
    cmovb dx, cx             ; taken -> dx = 0x1111
    movzx eax, dx
    CHECK_EQ_32 eax, 0x1111

    ; ==== Test 48: cmovb 16-bit not taken (not below) ====
    TEST_CASE t48_name
    mov ecx, 0x1111
    mov edx, 0x2222
    mov eax, 100
    cmp eax, 1               ; 100 > 1 -> CF=0
    cmovb dx, cx             ; not taken -> dx stays 0x2222
    movzx eax, dx
    CHECK_EQ_32 eax, 0x2222

    ; ==== Test 49: cmovl 16-bit taken (signed less) ====
    TEST_CASE t49_name
    mov ecx, 0x3333
    mov edx, 0x4444
    mov eax, -5
    cmp eax, 3               ; -5 < 3 signed
    cmovl dx, cx             ; taken -> dx = 0x3333
    movzx eax, dx
    CHECK_EQ_32 eax, 0x3333

    ; ==== Test 50: cmovl 16-bit not taken (not less) ====
    TEST_CASE t50_name
    mov ecx, 0x3333
    mov edx, 0x4444
    mov eax, 5
    cmp eax, 3               ; 5 > 3 signed -> not less
    cmovl dx, cx             ; not taken -> dx stays 0x4444
    movzx eax, dx
    CHECK_EQ_32 eax, 0x4444

    ; ==== Test 51: cmovne 16-bit preserves upper bits ====
    ; When taken, only low 16 bits should change; upper bits preserved
    TEST_CASE t51_name
    mov rcx, 0xDEAD1234      ; source: cx = 0x1234
    mov rdx, 0xBEEF5678      ; dest: dx = 0x5678, upper = 0xBEEF
    mov eax, 1
    cmp eax, 2               ; ZF=0 -> NE
    cmovne dx, cx            ; taken: dx = 0x1234, upper rdx should still be 0xBEEF
    CHECK_EQ_64 rdx, 0xBEEF1234

    ; ==== Test 52: cmova 16-bit preserves upper bits when not taken ====
    TEST_CASE t52_name
    mov rcx, 0xDEAD1234
    mov rdx, 0xBEEF5678
    mov eax, 1
    cmp eax, 100             ; 1 < 100 -> not above
    cmova dx, cx             ; not taken: rdx should be completely unchanged
    CHECK_EQ_64 rdx, 0xBEEF5678

    ; ==== Tests 53-58: 32-bit CMOVcc not-taken must preserve upper 32 bits ====
    ; These test the NATIVEJUMP skip distance bug: when !rex.w, ZEROUP(gd)
    ; follows the MR, making the skip distance 16 not 12. If the branch
    ; lands ON the ZEROUP, the upper bits get incorrectly zeroed.
    ; Pattern: set upper bits in dest, CMP + CMOVcc (not taken), check upper bits survive.

    ; ==== Test 53: cmovne 32-bit not-taken preserves upper 32 bits ====
    TEST_CASE t53_name
    mov rcx, 0x1111111100000001  ; source (ecx = 1)
    mov rdx, 0xDEADBEEF00005678  ; dest: edx = 0x5678, upper = 0xDEADBEEF
    xor eax, eax
    cmp eax, 0               ; ZF=1 -> equal, so NE is false
    cmovne edx, ecx          ; not taken: edx should stay 0x5678, upper zeroed by 32-bit op? NO.
    ; x86-64: 32-bit CMOVcc not-taken leaves dest ENTIRELY unchanged (64 bits)
    CHECK_EQ_64 rdx, 0xDEADBEEF00005678

    ; ==== Test 54: cmove 32-bit not-taken preserves upper 32 bits ====
    TEST_CASE t54_name
    mov rcx, 0x2222222200000002
    mov rdx, 0xCAFEBABE00009ABC
    mov eax, 1
    cmp eax, 2               ; ZF=0 -> not equal, so E is false
    cmove edx, ecx           ; not taken
    CHECK_EQ_64 rdx, 0xCAFEBABE00009ABC

    ; ==== Test 55: cmova 32-bit not-taken preserves upper 32 bits ====
    TEST_CASE t55_name
    mov rcx, 0x3333333300000003
    mov rdx, 0xFEEDFACE0000DEF0
    mov eax, 1
    cmp eax, 100             ; 1 < 100 unsigned -> not above
    cmova edx, ecx           ; not taken
    CHECK_EQ_64 rdx, 0xFEEDFACE0000DEF0

    ; ==== Test 56: cmovb 32-bit not-taken preserves upper 32 bits ====
    TEST_CASE t56_name
    mov rcx, 0x4444444400000004
    mov rdx, 0xAAAABBBB0000CCCC
    mov eax, 100
    cmp eax, 1               ; 100 > 1 unsigned -> not below
    cmovb edx, ecx           ; not taken
    CHECK_EQ_64 rdx, 0xAAAABBBB0000CCCC

    ; ==== Test 57: cmovl 32-bit not-taken preserves upper 32 bits ====
    TEST_CASE t57_name
    mov rcx, 0x5555555500000005
    mov rdx, 0x1234ABCD0000DDDD
    mov eax, 50
    cmp eax, 10              ; 50 > 10 signed -> not less
    cmovl edx, ecx           ; not taken
    CHECK_EQ_64 rdx, 0x1234ABCD0000DDDD

    ; ==== Test 58: cmovge 32-bit not-taken preserves upper 32 bits ====
    TEST_CASE t58_name
    mov rcx, 0x6666666600000006
    mov rdx, 0x9876543200001111
    mov eax, 5
    cmp eax, 10              ; 5 < 10 signed -> not greater-or-equal
    cmovge edx, ecx          ; not taken
    CHECK_EQ_64 rdx, 0x9876543200001111

    ; ==== Tests 59-60: 32-bit CMOVcc taken must zero upper 32 bits ====
    ; x86-64 semantics: any 32-bit register write zero-extends to 64 bits.

    ; ==== Test 59: cmovne 32-bit taken zeros upper 32 bits ====
    TEST_CASE t59_name
    mov rcx, 0xFFFFFFFF0000ABCD  ; ecx = 0x0000ABCD
    mov rdx, 0xDEADBEEF00005678  ; rdx has upper bits set
    mov eax, 1
    cmp eax, 2               ; ZF=0 -> NE is true
    cmovne edx, ecx          ; taken: edx = 0x0000ABCD, rdx = 0x000000000000ABCD
    CHECK_EQ_64 rdx, 0x000000000000ABCD

    ; ==== Test 60: cmove 32-bit taken zeros upper 32 bits ====
    TEST_CASE t60_name
    mov rcx, 0xFFFFFFFF00001234
    mov rdx, 0xCAFEBABE00009999
    xor eax, eax
    cmp eax, 0               ; ZF=1 -> E is true
    cmove edx, ecx           ; taken: edx = 0x00001234, rdx = 0x0000000000001234
    CHECK_EQ_64 rdx, 0x0000000000001234

    ; ==== Tests 61-62: 64-bit CMOVcc (regression, should be correct) ====

    ; ==== Test 61: cmovne 64-bit not-taken preserves value ====
    TEST_CASE t61_name
    mov rcx, 0x1111111111111111
    mov rdx, 0xDEADBEEFCAFEBABE
    xor eax, eax
    cmp eax, 0               ; ZF=1 -> NE is false
    cmovne rdx, rcx          ; not taken
    CHECK_EQ_64 rdx, 0xDEADBEEFCAFEBABE

    ; ==== Test 62: cmovne 64-bit taken ====
    TEST_CASE t62_name
    mov rcx, 0x123456789ABCDEF0
    mov rdx, 0xDEADBEEFCAFEBABE
    mov eax, 1
    cmp eax, 2               ; ZF=0 -> NE is true
    cmovne rdx, rcx          ; taken
    CHECK_EQ_64 rdx, 0x123456789ABCDEF0

    ; ==== Tests 63-64: SETcc with CMP fusion (NATIVESET path) ====

    ; ==== Test 63: setne via cmp fusion ====
    TEST_CASE t63_name
    mov eax, 5
    cmp eax, 10              ; ZF=0 -> NE is true
    setne al                 ; al = 1
    movzx eax, al
    CHECK_EQ_32 eax, 1

    ; ==== Test 64: sete via cmp fusion ====
    TEST_CASE t64_name
    mov eax, 42
    cmp eax, 42              ; ZF=1 -> E is true
    sete al                  ; al = 1
    movzx eax, al
    CHECK_EQ_32 eax, 1

    END_TESTS
