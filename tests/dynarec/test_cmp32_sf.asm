; test_cmp32_sf.asm - Test SF flag extraction for 32-bit CMP/TEST against zero
; Specifically targets the bug where SRDI(reg, 31) on a 64-bit value
; with dirty upper 32 bits produces wrong SF.
;
; On real x86_64, CMP EAX,0 only examines EAX (bits 31:0).
; A buggy dynarec that does a 64-bit shift-right by 31 on the full
; 64-bit host register would extract bits from the upper 32 bits too.
%include "test_framework.inc"

section .data
    ; CMP EAX, 0 tests
    t1_name:  db "cmp eax,0 after mov rax,0x100000001 (SF=0)", 0
    t2_name:  db "cmp eax,0 after mov rax,0x100000000 (SF=0,ZF=1)", 0
    t3_name:  db "cmp eax,0 after mov rax,0x1FFFFFFFF (SF=1)", 0
    t4_name:  db "cmp eax,0 after mov rax,0x80000000 (SF=1)", 0
    t5_name:  db "cmp eax,0 after mov rax,0x7FFFFFFF (SF=0)", 0
    t6_name:  db "cmp eax,0 after mov rax,0xFFFFFFFF00000001 (SF=0)", 0
    t7_name:  db "cmp eax,0 after mov rax,0xFFFFFFFF80000000 (SF=1)", 0
    t8_name:  db "cmp eax,0 after mov rax,0x8000000000000000 (SF=0,ZF=1)", 0
    ; CMP with non-zero immediate to exercise emit_cmp32 path
    t9_name:  db "cmp eax,1 after mov rax,0x100000001 (SF=0,ZF=1)", 0
    t10_name: db "cmp eax,1 after mov rax,0xFFFFFFFF00000001 (SF=0,ZF=1)", 0
    ; TEST EAX, EAX tests (emit_test32 path)
    t11_name: db "test eax,eax after mov rax,0x100000001 (SF=0)", 0
    t12_name: db "test eax,eax after mov rax,0x100000000 (SF=0,ZF=1)", 0
    t13_name: db "test eax,eax after mov rax,0xFFFFFFFF80000000 (SF=1)", 0
    ; CMP Eb, 0 (8-bit) with upper bits dirty
    t14_name: db "cmp al,0 after mov rax,0x100 (SF=0,ZF=1)", 0
    t15_name: db "cmp al,0 after mov rax,0x180 (SF=1)", 0
    ; 32-bit register ops that should zero-extend, followed by CMP
    t16_name: db "mov eax,1 then cmp eax,0 (SF=0)", 0
    t17_name: db "add eax,0 after mov rax,0x100000001 (SF=0)", 0
    ; CMP ECX, 0 (different register)
    t18_name: db "cmp ecx,0 after mov rcx,0x200000003 (SF=0)", 0
    t19_name: db "cmp ecx,0 after mov rcx,0x280000000 (SF=1)", 0
    ; TEST with immediate (emit_test32c path)
    t20_name: db "test eax,0x7FFFFFFF after mov rax,0x100000001 (SF=0)", 0
    t21_name: db "test eax,0xFFFFFFFF after mov rax,0x1FFFFFFFF (SF=1)", 0
    ; Edge case: REX.W CMP (64-bit) should use bit 63
    t22_name: db "cmp rax,0 after mov rax,0x100000000 (SF=0)", 0
    t23_name: db "cmp rax,0 after mov rax,0x8000000000000000 (SF=1)", 0
    ; CMP Ed (memory operand), 0 -- emit_cmp32_0 via 0x83 /7
    t24_name: db "cmp dword [mem],0 where mem=0x100000001 (SF=0)", 0

section .bss
    alignb 8
    scratch_qword: resq 1

section .text
global _start

_start:
    INIT_TESTS

    ; ================================================================
    ; Test 1: CMP EAX, 0 where RAX = 0x1_00000001
    ; EAX = 1, bit 31 = 0 -> SF should be 0
    ; Bug: 64-bit SRDI(reg, 31) would see bit 32, giving SF=1
    ; ================================================================
    TEST_CASE t1_name
    mov rax, 0x100000001
    cmp eax, 0
    setns cl              ; cl = 1 if SF=0, cl = 0 if SF=1
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=0 (setns=1)

    ; ================================================================
    ; Test 2: CMP EAX, 0 where RAX = 0x1_00000000
    ; EAX = 0, bit 31 = 0 -> SF=0, ZF=1
    ; Bug: SRDI would see bit 32
    ; ================================================================
    TEST_CASE t2_name
    mov rax, 0x100000000
    cmp eax, 0
    pushfq
    pop r14
    ; Check SF=0 (bit 7)
    test r14, 0x80
    setz cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF should be 0

    ; ================================================================
    ; Test 3: CMP EAX, 0 where RAX = 0x1_FFFFFFFF
    ; EAX = 0xFFFFFFFF, bit 31 = 1 -> SF=1
    ; This should work even with the bug (bit 32 is also set)
    ; ================================================================
    TEST_CASE t3_name
    mov rax, 0x1FFFFFFFF
    cmp eax, 0
    sets cl               ; cl = 1 if SF=1
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=1

    ; ================================================================
    ; Test 4: CMP EAX, 0 where RAX = 0x80000000
    ; EAX = 0x80000000, bit 31 = 1 -> SF=1
    ; ================================================================
    TEST_CASE t4_name
    mov rax, 0x80000000
    cmp eax, 0
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=1

    ; ================================================================
    ; Test 5: CMP EAX, 0 where RAX = 0x7FFFFFFF
    ; EAX = 0x7FFFFFFF, bit 31 = 0 -> SF=0
    ; ================================================================
    TEST_CASE t5_name
    mov rax, 0x7FFFFFFF
    cmp eax, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=0

    ; ================================================================
    ; Test 6: CMP EAX, 0 where RAX = 0xFFFFFFFF_00000001
    ; EAX = 1, bit 31 = 0 -> SF=0
    ; Bug: all upper 32 bits are set, SRDI gives huge value
    ; ================================================================
    TEST_CASE t6_name
    mov rax, 0xFFFFFFFF00000001
    cmp eax, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=0

    ; ================================================================
    ; Test 7: CMP EAX, 0 where RAX = 0xFFFFFFFF_80000000
    ; EAX = 0x80000000, bit 31 = 1 -> SF=1
    ; ================================================================
    TEST_CASE t7_name
    mov rax, 0xFFFFFFFF80000000
    cmp eax, 0
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=1

    ; ================================================================
    ; Test 8: CMP EAX, 0 where RAX = 0x80000000_00000000
    ; EAX = 0, bit 31 = 0 -> SF=0, ZF=1
    ; Bug: bit 63 set, SRDI gives enormous shift result
    ; ================================================================
    TEST_CASE t8_name
    mov rax, 0x8000000000000000
    cmp eax, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=0

    ; ================================================================
    ; Test 9: CMP EAX, 1 where RAX = 0x1_00000001
    ; EAX = 1, 1-1 = 0 -> SF=0, ZF=1
    ; (exercises emit_cmp32 not emit_cmp32_0, but related)
    ; ================================================================
    TEST_CASE t9_name
    mov rax, 0x100000001
    cmp eax, 1
    pushfq
    pop r14
    test r14, 0x80        ; SF
    setz cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF should be 0

    ; ================================================================
    ; Test 10: CMP EAX, 1 where RAX = 0xFFFFFFFF_00000001
    ; EAX = 1, 1-1 = 0 -> SF=0, ZF=1
    ; ================================================================
    TEST_CASE t10_name
    mov rax, 0xFFFFFFFF00000001
    cmp eax, 1
    pushfq
    pop r14
    test r14, 0x80        ; SF
    setz cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF should be 0

    ; ================================================================
    ; Test 11: TEST EAX, EAX where RAX = 0x1_00000001
    ; EAX & EAX = 1, bit 31 = 0 -> SF=0
    ; Note: on real x86_64, TEST uses 32-bit operand size,
    ; so TEST EAX,EAX = AND of lower 32 bits only
    ; ================================================================
    TEST_CASE t11_name
    mov rax, 0x100000001
    test eax, eax
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=0

    ; ================================================================
    ; Test 12: TEST EAX, EAX where RAX = 0x1_00000000
    ; EAX = 0, AND = 0 -> SF=0, ZF=1
    ; ================================================================
    TEST_CASE t12_name
    mov rax, 0x100000000
    test eax, eax
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=0

    ; ================================================================
    ; Test 13: TEST EAX, EAX where RAX = 0xFFFFFFFF_80000000
    ; EAX = 0x80000000, AND = 0x80000000 -> SF=1
    ; ================================================================
    TEST_CASE t13_name
    mov rax, 0xFFFFFFFF80000000
    test eax, eax
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; expect SF=1

    ; ================================================================
    ; Test 14: CMP AL, 0 where RAX = 0x100 (AL = 0)
    ; SF=0, ZF=1
    ; ================================================================
    TEST_CASE t14_name
    mov rax, 0x100
    cmp al, 0
    pushfq
    pop r14
    test r14, 0x80        ; SF
    setz cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0

    ; ================================================================
    ; Test 15: CMP AL, 0 where RAX = 0x180 (AL = 0x80)
    ; bit 7 = 1 -> SF=1
    ; ================================================================
    TEST_CASE t15_name
    mov rax, 0x180
    cmp al, 0
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=1

    ; ================================================================
    ; Test 16: mov eax,1 (zero-extends RAX) then cmp eax,0
    ; After mov eax,1: RAX = 1 (upper 32 cleared by x86_64 rule)
    ; SF=0
    ; ================================================================
    TEST_CASE t16_name
    mov rax, 0xFFFFFFFFFFFFFFFF  ; dirty all bits
    mov eax, 1                    ; zero-extends: RAX = 1
    cmp eax, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0

    ; ================================================================
    ; Test 17: ADD EAX, 0 after MOV RAX, 0x1_00000001
    ; ADD EAX,0 should zero-extend result: EAX = 1, RAX = 1
    ; Then CMP EAX, 0: SF=0
    ; (tests whether add eax,0 properly cleans upper bits for later cmp)
    ; ================================================================
    TEST_CASE t17_name
    mov rax, 0x100000001
    add eax, 0                    ; 32-bit add, zero-extends result
    cmp eax, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0

    ; ================================================================
    ; Test 18: CMP ECX, 0 where RCX = 0x2_00000003
    ; ECX = 3, bit 31 = 0 -> SF=0
    ; ================================================================
    TEST_CASE t18_name
    mov rcx, 0x200000003
    cmp ecx, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0

    ; ================================================================
    ; Test 19: CMP ECX, 0 where RCX = 0x2_80000000
    ; ECX = 0x80000000, bit 31 = 1 -> SF=1
    ; ================================================================
    TEST_CASE t19_name
    mov rcx, 0x280000000
    cmp ecx, 0
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=1

    ; ================================================================
    ; Test 20: TEST EAX, 0x7FFFFFFF where RAX = 0x1_00000001
    ; result = 1 & 0x7FFFFFFF = 1, SF=0
    ; ================================================================
    TEST_CASE t20_name
    mov rax, 0x100000001
    test eax, 0x7FFFFFFF
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0

    ; ================================================================
    ; Test 21: TEST EAX, 0xFFFFFFFF where RAX = 0x1_FFFFFFFF
    ; On real x86_64: TEST EAX, imm32 — imm32 is sign-extended to 32-bit
    ; EAX = 0xFFFFFFFF, mask = 0xFFFFFFFF
    ; result = 0xFFFFFFFF, bit 31 = 1 -> SF=1
    ; ================================================================
    TEST_CASE t21_name
    mov rax, 0x1FFFFFFFF
    test eax, -1          ; -1 = 0xFFFFFFFF in 32-bit
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=1

    ; ================================================================
    ; Test 22: CMP RAX, 0 (64-bit) where RAX = 0x1_00000000
    ; bit 63 = 0 -> SF=0
    ; ================================================================
    TEST_CASE t22_name
    mov rax, 0x100000000
    cmp rax, 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0

    ; ================================================================
    ; Test 23: CMP RAX, 0 (64-bit) where RAX = 0x8000000000000000
    ; bit 63 = 1 -> SF=1
    ; ================================================================
    TEST_CASE t23_name
    mov rax, 0x8000000000000000
    cmp rax, 0
    sets cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=1

    ; ================================================================
    ; Test 24: CMP DWORD [mem], 0 where memory contains low 32 bits = 1
    ; (but we store full 64 bits = 0x100000001 to the qword)
    ; CMP reads only the DWORD, which is 1 -> SF=0
    ; ================================================================
    TEST_CASE t24_name
    mov rax, 0x100000001
    mov [rel scratch_qword], rax
    cmp dword [rel scratch_qword], 0
    setns cl
    movzx ecx, cl
    CHECK_EQ_32 ecx, 1   ; SF=0 (only low dword = 1 is compared)

    END_TESTS
