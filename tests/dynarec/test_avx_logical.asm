; test_avx_logical.asm - Test AVX logical/bitwise operations (VEX-encoded)
; VANDPS/PD, VANDNPS/PD, VORPS/PD, VXORPS/PD, VPAND, VPOR, VPXOR, VPANDN, VPTEST
%include "test_framework.inc"

section .data
    t1_name:  db "vandps", 0
    t2_name:  db "vandpd", 0
    t3_name:  db "vandnps", 0
    t4_name:  db "vandnpd", 0
    t5_name:  db "vorps", 0
    t6_name:  db "vorpd", 0
    t7_name:  db "vxorps zero self", 0
    t8_name:  db "vxorpd zero self", 0
    t9_name:  db "vpand", 0
    t10_name: db "vpandn", 0
    t11_name: db "vpor", 0
    t12_name: db "vpxor", 0
    t13_name: db "vptest ZF=1", 0
    t14_name: db "vptest ZF=0", 0
    t15_name: db "vptest CF=1", 0
    t16_name: db "vptest CF=0", 0
    t17_name: db "vandps 3-op preserves", 0
    t18_name: db "vxorps upper zeroed", 0
    t19_name: db "vpand sign mask", 0
    t20_name: db "vpor combine bits", 0
    t21_name: db "vtestps ZF=1", 0
    t22_name: db "vtestps ZF=0", 0
    t23_name: db "vtestpd ZF=1", 0
    t24_name: db "vtestpd CF=1", 0
    t25_name: db "vandnps abs value", 0

    align 16
    vec_aa: dq 0xAAAAAAAAAAAAAAAA, 0xAAAAAAAAAAAAAAAA
    vec_55: dq 0x5555555555555555, 0x5555555555555555
    vec_ff: dq 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF
    vec_00: dq 0, 0
    vec_f0: dq 0xF0F0F0F0F0F0F0F0, 0xF0F0F0F0F0F0F0F0
    vec_0f: dq 0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F
    vec_ab: dq 0x0123456789ABCDEF, 0xFEDCBA9876543210
    vec_cd: dq 0xFFFF0000FFFF0000, 0x00FFFF0000FFFF00
    ; abs mask for floats (clear sign bit)
    vec_abs_mask: dd 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF
    vec_neg_vals: dd 0xC0400000, 0xC0800000, 0xC0A00000, 0xC0C00000  ; -3,-4,-5,-6
    ; For VTESTPS/PD: sign bit patterns
    vec_signs_set: dd 0x80000000, 0x80000000, 0x80000000, 0x80000000
    vec_signs_clr: dd 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF
    vec_mixed_sign: dd 0x80000000, 0x00000000, 0x80000000, 0x00000000
    vec_pd_neg: dq 0x8000000000000000, 0x8000000000000000
    vec_pd_pos: dq 0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vandps ---
    TEST_CASE t1_name
    vmovaps xmm0, [rel vec_aa]
    vmovaps xmm1, [rel vec_55]
    vandps xmm2, xmm0, xmm1
    ; 0xAA & 0x55 = 0x00
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 2: vandpd ---
    TEST_CASE t2_name
    vmovapd xmm0, [rel vec_ff]
    vmovapd xmm1, [rel vec_f0]
    vandpd xmm2, xmm0, xmm1
    ; 0xFF & 0xF0 = 0xF0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xF0F0F0F0F0F0F0F0

    ; --- Test 3: vandnps ---
    TEST_CASE t3_name
    vmovaps xmm0, [rel vec_f0]
    vmovaps xmm1, [rel vec_ff]
    vandnps xmm2, xmm0, xmm1
    ; ~0xF0 & 0xFF = 0x0F
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0F0F0F0F0F0F0F0F

    ; --- Test 4: vandnpd ---
    TEST_CASE t4_name
    vmovapd xmm0, [rel vec_aa]
    vmovapd xmm1, [rel vec_ff]
    vandnpd xmm2, xmm0, xmm1
    ; ~0xAA & 0xFF = 0x55
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x5555555555555555

    ; --- Test 5: vorps ---
    TEST_CASE t5_name
    vmovaps xmm0, [rel vec_f0]
    vmovaps xmm1, [rel vec_0f]
    vorps xmm2, xmm0, xmm1
    ; 0xF0 | 0x0F = 0xFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 6: vorpd ---
    TEST_CASE t6_name
    vmovapd xmm0, [rel vec_aa]
    vmovapd xmm1, [rel vec_55]
    vorpd xmm2, xmm0, xmm1
    ; 0xAA | 0x55 = 0xFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 7: vxorps zero self ---
    TEST_CASE t7_name
    vmovaps xmm0, [rel vec_ff]
    vxorps xmm2, xmm0, xmm0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 8: vxorpd zero self ---
    TEST_CASE t8_name
    vmovapd xmm0, [rel vec_ab]
    vxorpd xmm2, xmm0, xmm0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 9: vpand ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_ab]
    vmovdqa xmm1, [rel vec_cd]
    vpand xmm2, xmm0, xmm1
    ; 0x0123456789ABCDEF & 0xFFFF0000FFFF0000 = 0x01230000FFAB0000
    ; Wait: 0x0123456789ABCDEF & 0xFFFF0000FFFF0000
    ;   = 0x0123 0000 89AB 0000 ... hmm let me recalc byte-by-byte
    ; Actually in 64-bit: 0x0123456789ABCDEF & 0xFFFF0000FFFF0000
    ; = 0x012300008 9AB0000  no...
    ; 0x0123456789ABCDEF
    ; & 0xFFFF0000FFFF0000
    ; = 0x0123000089AB0000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0123000089AB0000

    ; --- Test 10: vpandn ---
    TEST_CASE t10_name
    vmovdqa xmm0, [rel vec_f0]
    vmovdqa xmm1, [rel vec_ab]
    vpandn xmm2, xmm0, xmm1
    ; ~0xF0F0F0F0F0F0F0F0 & 0x0123456789ABCDEF
    ; = 0x0F0F0F0F0F0F0F0F & 0x0123456789ABCDEF
    ; = 0x01030507090B0D0F
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x01030507090B0D0F

    ; --- Test 11: vpor ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_f0]
    vmovdqa xmm1, [rel vec_0f]
    vpor xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 12: vpxor ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_ff]
    vmovdqa xmm1, [rel vec_aa]
    vpxor xmm2, xmm0, xmm1
    ; 0xFF ^ 0xAA = 0x55
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x5555555555555555

    ; --- Test 13: vptest ZF=1 (AND is zero) ---
    TEST_CASE t13_name
    vmovdqa xmm0, [rel vec_aa]
    vmovdqa xmm1, [rel vec_55]
    vptest xmm0, xmm1
    ; AA & 55 = 0 -> ZF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; --- Test 14: vptest ZF=0 (AND is non-zero) ---
    TEST_CASE t14_name
    vmovdqa xmm0, [rel vec_ff]
    vmovdqa xmm1, [rel vec_55]
    vptest xmm0, xmm1
    ; FF & 55 = 55 -> ZF=0
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, 0

    ; --- Test 15: vptest CF=1 (ANDN is zero) ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_55]
    vmovdqa xmm1, [rel vec_55]
    vptest xmm0, xmm1
    ; ~55 & 55 = AA & 55 = 0 -> CF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test 16: vptest CF=0 (ANDN is non-zero) ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_55]
    vmovdqa xmm1, [rel vec_ff]
    vptest xmm0, xmm1
    ; ~55 & FF = AA -> CF=0
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; --- Test 17: vandps 3-op preserves sources ---
    TEST_CASE t17_name
    vmovaps xmm3, [rel vec_ff]
    vmovaps xmm4, [rel vec_f0]
    vandps xmm5, xmm3, xmm4
    ; Check xmm3 is still all-ones
    vmovq rax, xmm3
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 18: vxorps zeroes upper ymm ---
    TEST_CASE t18_name
    ; Fill ymm0 with ones
    vpcmpeqd ymm0, ymm0, ymm0
    ; 128-bit vxorps should zero upper 128 bits
    vxorps xmm0, xmm0, xmm0
    vextractf128 xmm1, ymm0, 1
    vmovq rax, xmm1
    CHECK_EQ_64 rax, 0

    ; --- Test 19: vpand sign mask ---
    TEST_CASE t19_name
    ; Keep only sign bits of negative floats
    vmovdqa xmm0, [rel vec_neg_vals]
    vmovdqa xmm1, [rel vec_signs_set]
    vpand xmm2, xmm0, xmm1
    ; All have sign bit set -> all 0x80000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x8000000080000000

    ; --- Test 20: vpor combine bits ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_aa]
    vmovdqa xmm1, [rel vec_55]
    vpor xmm2, xmm0, xmm1
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 21: vtestps ZF=1 ---
    TEST_CASE t21_name
    vmovaps xmm0, [rel vec_signs_clr]  ; no sign bits
    vmovaps xmm1, [rel vec_signs_set]  ; only sign bits
    vtestps xmm0, xmm1
    ; xmm0 & xmm1 = 0 -> ZF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; --- Test 22: vtestps ZF=0 ---
    TEST_CASE t22_name
    vmovaps xmm0, [rel vec_signs_set]
    vmovaps xmm1, [rel vec_signs_set]
    vtestps xmm0, xmm1
    ; 0x80000000 & 0x80000000 != 0 -> ZF=0
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, 0

    ; --- Test 23: vtestpd ZF=1 ---
    TEST_CASE t23_name
    vmovapd xmm0, [rel vec_pd_pos]  ; no sign bits
    vmovapd xmm1, [rel vec_pd_neg]  ; only sign bits
    vtestpd xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; --- Test 24: vtestpd CF=1 ---
    TEST_CASE t24_name
    vmovapd xmm0, [rel vec_pd_neg]
    vmovapd xmm1, [rel vec_pd_neg]
    vtestpd xmm0, xmm1
    ; ~(sign bits) & (sign bits) = 0 -> CF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test 25: vandnps to get absolute value ---
    TEST_CASE t25_name
    ; abs = andnot(sign_mask, value) = ~sign_mask & value
    vmovaps xmm0, [rel vec_signs_set]  ; 0x80000000 (sign mask)
    vmovaps xmm1, [rel vec_neg_vals]   ; -3,-4,-5,-6
    vandnps xmm2, xmm0, xmm1
    ; ~0x80000000 & neg_val = clear sign bit = abs value
    ; abs(-3)=3=0x40400000, abs(-4)=4=0x40800000
    ; low64 = 0x4080000040400000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4080000040400000

    END_TESTS
