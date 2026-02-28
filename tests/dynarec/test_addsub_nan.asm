; test_addsub_nan.asm - Test ADDSUBPS/ADDSUBPD NaN sign behavior
; Verifies that NaN sign bits are handled correctly:
;   - Subtraction lanes flip NaN sign (like x86 SUB) when NaN comes from Ex
;   - Subtraction lanes preserve NaN sign when NaN comes from Vx (Vx NaN wins)
;   - Addition lanes preserve NaN sign
;   - INF-INF / INF+(-INF) produce negative indefinite NaN (0xFFC00000 / 0xFFF8...)
;   - NaN payloads are preserved through subtraction
;
; Expected values validated on Rosetta 2 (x86_64 on Apple Silicon).

%include "test_framework.inc"

section .data
    ; ADDSUBPS tests
    t1_name:  db "addsubps +NaN Vx sub[0]", 0
    t2_name:  db "addsubps -NaN Vx sub[0]", 0
    t3_name:  db "addsubps +NaN Ex sub[0]", 0
    t4_name:  db "addsubps -NaN Ex sub[0]", 0
    t5_name:  db "addsubps +NaN Ex add[1]", 0
    t6_name:  db "addsubps -NaN Ex add[1]", 0
    t7_name:  db "addsubps INF-INF sub[0]", 0
    t8_name:  db "addsubps INF+(-INF) add[1]", 0
    t9_name:  db "addsubps payload sub[0]", 0
    t10_name: db "addsubps +NaN Ex sub[2]", 0
    t11_name: db "addsubps both NaN sub[0]", 0
    t12_name: db "addsubps sanity 3-1,3+1", 0

    ; ADDSUBPD tests
    t13_name: db "addsubpd +NaN Ex sub[0]", 0
    t14_name: db "addsubpd -NaN Ex sub[0]", 0
    t15_name: db "addsubpd +NaN Vx sub[0]", 0
    t16_name: db "addsubpd -NaN Vx sub[0]", 0
    t17_name: db "addsubpd +NaN Ex add[1]", 0
    t18_name: db "addsubpd INF-INF sub[0]", 0
    t19_name: db "addsubpd sanity 3-1,3+1", 0
    t20_name: db "addsubpd payload sub[0]", 0

    align 16
    ; SP constants
    sp_pos_nan:   dd 0x7FC00000
    sp_neg_nan:   dd 0xFFC00000
    sp_one:       dd 0x3F800000
    sp_three:     dd 0x40400000
    sp_pos_inf:   dd 0x7F800000
    sp_neg_inf:   dd 0xFF800000
    sp_custom_nan: dd 0x7FC0BEEF

    ; DP constants
    align 16
    dp_pos_nan:   dq 0x7FF8000000000000
    dp_neg_nan:   dq 0xFFF8000000000000
    dp_one:       dq 0x3FF0000000000000
    dp_three:     dq 0x4008000000000000
    dp_pos_inf:   dq 0x7FF0000000000000
    dp_custom_nan: dq 0x7FF80000DEADBEEF

    ; Packed vectors (x86 lane order: low to high)
    align 16
    ; ADDSUBPS vectors
    ; T1: Vx=[+NaN, 1, 1, 1], Ex=[1, 1, 1, 1]
    t1_vx: dd 0x7FC00000, 0x3F800000, 0x3F800000, 0x3F800000
    t1_ex: dd 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000

    ; T2: Vx=[-NaN, 1, 1, 1], Ex=[1, 1, 1, 1]
    t2_vx: dd 0xFFC00000, 0x3F800000, 0x3F800000, 0x3F800000

    ; T3: Vx=[1, 1, 1, 1], Ex=[+NaN, 1, 1, 1]
    t3_ex: dd 0x7FC00000, 0x3F800000, 0x3F800000, 0x3F800000

    ; T4: Vx=[1, 1, 1, 1], Ex=[-NaN, 1, 1, 1]
    t4_ex: dd 0xFFC00000, 0x3F800000, 0x3F800000, 0x3F800000

    ; T5: Vx=[1, 1, 1, 1], Ex=[1, +NaN, 1, 1]
    t5_ex: dd 0x3F800000, 0x7FC00000, 0x3F800000, 0x3F800000

    ; T6: Vx=[1, 1, 1, 1], Ex=[1, -NaN, 1, 1]
    t6_ex: dd 0x3F800000, 0xFFC00000, 0x3F800000, 0x3F800000

    ; T7: Vx=[+INF, 1, 1, 1], Ex=[+INF, 1, 1, 1]
    t7_vx: dd 0x7F800000, 0x3F800000, 0x3F800000, 0x3F800000
    t7_ex: dd 0x7F800000, 0x3F800000, 0x3F800000, 0x3F800000

    ; T8: Vx=[1, +INF, 1, 1], Ex=[1, -INF, 1, 1]
    t8_vx: dd 0x3F800000, 0x7F800000, 0x3F800000, 0x3F800000
    t8_ex: dd 0x3F800000, 0xFF800000, 0x3F800000, 0x3F800000

    ; T9: Vx=[1, 1, 1, 1], Ex=[0x7FC0BEEF, 1, 1, 1]
    t9_ex: dd 0x7FC0BEEF, 0x3F800000, 0x3F800000, 0x3F800000

    ; T10: Vx=[1, 1, 1, 1], Ex=[1, 1, +NaN, 1]
    t10_ex: dd 0x3F800000, 0x3F800000, 0x7FC00000, 0x3F800000

    ; T11: Vx=[+NaN, 1, 1, 1], Ex=[-NaN, 1, 1, 1]
    t11_vx: dd 0x7FC00000, 0x3F800000, 0x3F800000, 0x3F800000
    t11_ex: dd 0xFFC00000, 0x3F800000, 0x3F800000, 0x3F800000

    ; T12: Vx=[3, 3, 3, 3], Ex=[1, 1, 1, 1]
    t12_vx: dd 0x40400000, 0x40400000, 0x40400000, 0x40400000
    t12_ex: dd 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000

    ; ADDSUBPD vectors
    align 16
    ; T13: Vx=[1.0, 1.0], Ex=[+NaN, 1.0]
    t13_vx: dq 0x3FF0000000000000, 0x3FF0000000000000
    t13_ex: dq 0x7FF8000000000000, 0x3FF0000000000000

    ; T14: Vx=[1.0, 1.0], Ex=[-NaN, 1.0]
    t14_ex: dq 0xFFF8000000000000, 0x3FF0000000000000

    ; T15: Vx=[+NaN, 1.0], Ex=[1.0, 1.0]
    t15_vx: dq 0x7FF8000000000000, 0x3FF0000000000000
    t15_ex: dq 0x3FF0000000000000, 0x3FF0000000000000

    ; T16: Vx=[-NaN, 1.0], Ex=[1.0, 1.0]
    t16_vx: dq 0xFFF8000000000000, 0x3FF0000000000000

    ; T17: Vx=[1.0, 1.0], Ex=[1.0, +NaN]
    t17_ex: dq 0x3FF0000000000000, 0x7FF8000000000000

    ; T18: Vx=[+INF, 1.0], Ex=[+INF, 1.0]
    t18_vx: dq 0x7FF0000000000000, 0x3FF0000000000000
    t18_ex: dq 0x7FF0000000000000, 0x3FF0000000000000

    ; T19: Vx=[3.0, 3.0], Ex=[1.0, 1.0]
    t19_vx: dq 0x4008000000000000, 0x4008000000000000
    t19_ex: dq 0x3FF0000000000000, 0x3FF0000000000000

    ; T20: Vx=[1.0, 1.0], Ex=[custom NaN, 1.0]
    t20_ex: dq 0x7FF80000DEADBEEF, 0x3FF0000000000000

section .text
global _start
_start:
    INIT_TESTS

    ; ===================== ADDSUBPS tests =====================
    ; Lane layout: [0]=sub, [1]=add, [2]=sub, [3]=add

    ; T1: +NaN in Vx sub lane[0] → Vx NaN wins, sign preserved → 0x7FC00000
    TEST_CASE t1_name
    movaps xmm0, [rel t1_vx]
    movaps xmm1, [rel t1_ex]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x7FC00000

    ; T2: -NaN in Vx sub lane[0] → Vx NaN wins, sign preserved → 0xFFC00000
    TEST_CASE t2_name
    movaps xmm0, [rel t2_vx]
    movaps xmm1, [rel t1_ex]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC00000

    ; T3: +NaN in Ex sub lane[0] → sign FLIPPED → 0xFFC00000
    TEST_CASE t3_name
    movaps xmm0, [rel t1_ex]     ; Vx = [1, 1, 1, 1]
    movaps xmm1, [rel t3_ex]     ; Ex = [+NaN, 1, 1, 1]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC00000

    ; T4: -NaN in Ex sub lane[0] → sign FLIPPED → 0x7FC00000
    TEST_CASE t4_name
    movaps xmm0, [rel t1_ex]
    movaps xmm1, [rel t4_ex]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x7FC00000

    ; T5: +NaN in Ex add lane[1] → sign preserved → 0x7FC00000
    TEST_CASE t5_name
    movaps xmm0, [rel t1_ex]
    movaps xmm1, [rel t5_ex]
    addsubps xmm0, xmm1
    ; Extract lane 1: shift right by 32 bits
    psrldq xmm0, 4
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x7FC00000

    ; T6: -NaN in Ex add lane[1] → sign preserved → 0xFFC00000
    TEST_CASE t6_name
    movaps xmm0, [rel t1_ex]
    movaps xmm1, [rel t6_ex]
    addsubps xmm0, xmm1
    psrldq xmm0, 4
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC00000

    ; T7: INF-INF sub lane[0] → negative indefinite → 0xFFC00000
    TEST_CASE t7_name
    movaps xmm0, [rel t7_vx]
    movaps xmm1, [rel t7_ex]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC00000

    ; T8: INF+(-INF) add lane[1] → negative indefinite → 0xFFC00000
    TEST_CASE t8_name
    movaps xmm0, [rel t8_vx]
    movaps xmm1, [rel t8_ex]
    addsubps xmm0, xmm1
    psrldq xmm0, 4
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC00000

    ; T9: Custom NaN payload 0x7FC0BEEF in Ex sub[0] → sign flipped, payload preserved → 0xFFC0BEEF
    TEST_CASE t9_name
    movaps xmm0, [rel t1_ex]
    movaps xmm1, [rel t9_ex]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC0BEEF

    ; T10: +NaN in Ex sub lane[2] → sign flipped → 0xFFC00000
    TEST_CASE t10_name
    movaps xmm0, [rel t1_ex]
    movaps xmm1, [rel t10_ex]
    addsubps xmm0, xmm1
    ; Extract lane 2: shift right by 64 bits
    psrldq xmm0, 8
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xFFC00000

    ; T11: Both NaN sub[0]: Vx=+NaN, Ex=-NaN → Vx NaN wins → 0x7FC00000
    TEST_CASE t11_name
    movaps xmm0, [rel t11_vx]
    movaps xmm1, [rel t11_ex]
    addsubps xmm0, xmm1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x7FC00000

    ; T12: sanity: 3.0 addsub 1.0 → [0]=2.0, [1]=4.0, [2]=2.0, [3]=4.0
    ; Check as two 64-bit values: low64 = {2.0, 4.0} = 0x40800000_40000000
    ;                              high64 = {2.0, 4.0} = 0x40800000_40000000
    TEST_CASE t12_name
    movaps xmm0, [rel t12_vx]
    movaps xmm1, [rel t12_ex]
    addsubps xmm0, xmm1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4080000040000000

    ; ===================== ADDSUBPD tests =====================
    ; Lane layout: [0]=sub, [1]=add

    ; T13: +NaN in Ex sub lane[0] → sign FLIPPED → 0xFFF8000000000000
    TEST_CASE t13_name
    movapd xmm0, [rel t13_vx]
    movapd xmm1, [rel t13_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0xFFF8000000000000
    CHECK_EQ_64 rax, rbx

    ; T14: -NaN in Ex sub lane[0] → sign FLIPPED → 0x7FF8000000000000
    TEST_CASE t14_name
    movapd xmm0, [rel t13_vx]
    movapd xmm1, [rel t14_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0x7FF8000000000000
    CHECK_EQ_64 rax, rbx

    ; T15: +NaN in Vx sub lane[0] → Vx NaN wins, sign preserved → 0x7FF8000000000000
    TEST_CASE t15_name
    movapd xmm0, [rel t15_vx]
    movapd xmm1, [rel t15_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0x7FF8000000000000
    CHECK_EQ_64 rax, rbx

    ; T16: -NaN in Vx sub lane[0] → Vx NaN wins, sign preserved → 0xFFF8000000000000
    TEST_CASE t16_name
    movapd xmm0, [rel t16_vx]
    movapd xmm1, [rel t15_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0xFFF8000000000000
    CHECK_EQ_64 rax, rbx

    ; T17: +NaN in Ex add lane[1] → sign preserved → 0x7FF8000000000000
    TEST_CASE t17_name
    movapd xmm0, [rel t13_vx]
    movapd xmm1, [rel t17_ex]
    addsubpd xmm0, xmm1
    ; Extract lane 1 (high 64 bits)
    psrldq xmm0, 8
    movq rax, xmm0
    mov rbx, 0x7FF8000000000000
    CHECK_EQ_64 rax, rbx

    ; T18: INF-INF sub lane[0] → negative indefinite → 0xFFF8000000000000
    TEST_CASE t18_name
    movapd xmm0, [rel t18_vx]
    movapd xmm1, [rel t18_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0xFFF8000000000000
    CHECK_EQ_64 rax, rbx

    ; T19: sanity: 3.0 addsub 1.0 → [0]=2.0, [1]=4.0
    ; 2.0 = 0x4000000000000000, 4.0 = 0x4010000000000000
    TEST_CASE t19_name
    movapd xmm0, [rel t19_vx]
    movapd xmm1, [rel t19_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0x4000000000000000
    CHECK_EQ_64 rax, rbx

    ; T20: Custom NaN payload in Ex sub[0] → sign flipped, payload preserved
    ; 0x7FF80000DEADBEEF → 0xFFF80000DEADBEEF
    TEST_CASE t20_name
    movapd xmm0, [rel t13_vx]
    movapd xmm1, [rel t20_ex]
    addsubpd xmm0, xmm1
    movq rax, xmm0
    mov rbx, 0xFFF80000DEADBEEF
    CHECK_EQ_64 rax, rbx

    END_TESTS
