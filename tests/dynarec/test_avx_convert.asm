; test_avx_convert.asm - Test AVX conversion operations (VEX-encoded)
; VCVTPS2PD, VCVTPD2PS, VCVTDQ2PS, VCVTPS2DQ, VCVTTPS2DQ,
; VCVTSI2SS/SD, VCVTSS2SI, VCVTSD2SI, VCVTSS2SD, VCVTSD2SS,
; VCVTDQ2PD, VCVTPD2DQ, VCVTTPD2DQ
%include "test_framework.inc"

section .data
    t1_name:  db "vcvtsi2ss 42", 0
    t2_name:  db "vcvtsi2sd 42", 0
    t3_name:  db "vcvtss2si 3.0", 0
    t4_name:  db "vcvtsd2si 3.0", 0
    t5_name:  db "vcvtss2sd", 0
    t6_name:  db "vcvtsd2ss", 0
    t7_name:  db "vcvtps2pd", 0
    t8_name:  db "vcvtpd2ps", 0
    t9_name:  db "vcvtdq2ps", 0
    t10_name: db "vcvtps2dq", 0
    t11_name: db "vcvttps2dq", 0
    t12_name: db "vcvtdq2pd", 0
    t13_name: db "vcvtpd2dq", 0
    t14_name: db "vcvttpd2dq", 0
    t15_name: db "vcvtsi2ss neg", 0
    t16_name: db "vcvtsi2sd neg", 0
    t17_name: db "vcvtss2si neg", 0
    t18_name: db "vcvtsd2si neg", 0
    t19_name: db "vcvttss2si trunc", 0
    t20_name: db "vcvttsd2si trunc", 0
    t21_name: db "vcvtsi2ss 64-bit", 0
    t22_name: db "vcvtsi2sd 64-bit", 0
    t23_name: db "vcvtss2si 64-bit", 0
    t24_name: db "vcvtsd2si 64-bit", 0
    t25_name: db "vcvtps2dq round", 0
    t26_name: db "vcvtdq2ps large", 0
    t27_name: db "vcvtps2pd preserves", 0
    t28_name: db "vcvtpd2ps zeroes hi", 0
    t29_name: db "vcvtsi2ss zero", 0
    t30_name: db "vcvttps2dq neg", 0

    align 16
    ; Single precision values
    val_ps_3: dd 0x40400000    ; 3.0f
    val_ps_neg3: dd 0xC0400000 ; -3.0f
    val_ps_2p5: dd 0x40200000  ; 2.5f
    val_ps_neg2p7: dd 0xC02CCCCD ; -2.7f (approx)
    ; Double precision values
    val_pd_3: dq 0x4008000000000000    ; 3.0
    val_pd_neg3: dq 0xC008000000000000 ; -3.0
    val_pd_2p5: dq 0x4004000000000000  ; 2.5
    ; -2.7 in double is 0xC005999999999999 but unused, removed
    val_pd_neg2p5: dq 0xC004000000000000 ; -2.5

    ; Float vectors
    vec_ps_a: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_round: dd 0x40200000, 0xC0200000, 0x40500000, 0xC0500000  ; 2.5,-2.5,3.25,-3.25
    vec_ps_neg: dd 0xC0400000, 0xC0800000, 0xC0A00000, 0xC0C00000  ; -3,-4,-5,-6
    ; Double vectors
    vec_pd_a: dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0
    vec_pd_34: dq 0x4008000000000000, 0x4010000000000000  ; 3.0, 4.0
    vec_pd_neg: dq 0xC008000000000000, 0xC010000000000000  ; -3.0, -4.0
    vec_pd_2p5: dq 0x4004000000000000, 0xC004000000000000  ; 2.5, -2.5
    ; Integer vectors
    vec_dq_a: dd 1, 2, 3, 4
    vec_dq_big: dd 100000, -100000, 42, -42
    vec_dq_large: dd 0x4B000000, 0x4B800000, 0, 0  ; 8388608, 16777216

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vcvtsi2ss 32-bit int -> float ---
    TEST_CASE t1_name
    mov eax, 42
    vxorps xmm0, xmm0, xmm0
    vcvtsi2ss xmm2, xmm0, eax
    ; 42.0f = 0x42280000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x42280000

    ; --- Test 2: vcvtsi2sd 32-bit int -> double ---
    TEST_CASE t2_name
    mov eax, 42
    vxorpd xmm0, xmm0, xmm0
    vcvtsi2sd xmm2, xmm0, eax
    ; 42.0 = 0x4045000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4045000000000000

    ; --- Test 3: vcvtss2si float -> 32-bit int ---
    TEST_CASE t3_name
    vmovss xmm0, [rel val_ps_3]
    vcvtss2si eax, xmm0
    CHECK_EQ_32 eax, 3

    ; --- Test 4: vcvtsd2si double -> 32-bit int ---
    TEST_CASE t4_name
    vmovsd xmm0, [rel val_pd_3]
    vcvtsd2si eax, xmm0
    CHECK_EQ_32 eax, 3

    ; --- Test 5: vcvtss2sd float -> double ---
    TEST_CASE t5_name
    vmovss xmm0, [rel val_ps_3]
    vxorpd xmm1, xmm1, xmm1
    vcvtss2sd xmm2, xmm1, xmm0
    ; 3.0f -> 3.0 = 0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 6: vcvtsd2ss double -> float ---
    TEST_CASE t6_name
    vmovsd xmm0, [rel val_pd_3]
    vxorps xmm1, xmm1, xmm1
    vcvtsd2ss xmm2, xmm1, xmm0
    ; 3.0 -> 3.0f = 0x40400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 7: vcvtps2pd (2 floats -> 2 doubles) ---
    TEST_CASE t7_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vcvtps2pd xmm2, xmm0
    ; Converts low 2 floats: 1.0f->1.0, 2.0f->2.0
    ; low64 = 1.0 = 0x3FF0000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 8: vcvtpd2ps (2 doubles -> 2 floats in low64) ---
    TEST_CASE t8_name
    vmovapd xmm0, [rel vec_pd_34]  ; [3.0, 4.0]
    vcvtpd2ps xmm2, xmm0
    ; 3.0->3.0f=0x40400000, 4.0->4.0f=0x40800000
    ; low64 = [3.0f, 4.0f, 0, 0] but in xmm low 64: d0=3.0f, d1=4.0f
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4080000040400000

    ; --- Test 9: vcvtdq2ps (packed dword int -> packed float) ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_dq_a]  ; [1,2,3,4]
    vcvtdq2ps xmm2, xmm0
    ; [1.0f,2.0f,3.0f,4.0f] low64=[1.0f,2.0f]=0x400000003F800000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x400000003F800000

    ; --- Test 10: vcvtps2dq (packed float -> packed dword, round nearest) ---
    TEST_CASE t10_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vcvtps2dq xmm2, xmm0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000200000001

    ; --- Test 11: vcvttps2dq (packed float -> packed dword, truncate) ---
    TEST_CASE t11_name
    vmovaps xmm0, [rel vec_ps_round]  ; [2.5,-2.5,3.25,-3.25]
    vcvttps2dq xmm2, xmm0
    ; truncate: 2.5->2, -2.5->-2
    vmovq rax, xmm2
    ; d0=2, d1=-2=0xFFFFFFFE
    CHECK_EQ_64 rax, 0xFFFFFFFE00000002

    ; --- Test 12: vcvtdq2pd (2 dwords -> 2 doubles) ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_dq_a]  ; [1,2,3,4]
    vcvtdq2pd xmm2, xmm0
    ; Converts low 2 dwords: 1->1.0, 2->2.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 13: vcvtpd2dq (2 doubles -> 2 dwords, round nearest) ---
    TEST_CASE t13_name
    vmovapd xmm0, [rel vec_pd_34]  ; [3.0, 4.0]
    vcvtpd2dq xmm2, xmm0
    ; 3.0->3, 4.0->4. Result in low64: d0=3, d1=4
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000400000003

    ; --- Test 14: vcvttpd2dq (2 doubles -> 2 dwords, truncate) ---
    TEST_CASE t14_name
    vmovapd xmm0, [rel vec_pd_2p5]  ; [2.5, -2.5]
    vcvttpd2dq xmm2, xmm0
    ; truncate: 2.5->2, -2.5->-2=0xFFFFFFFE
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFE00000002

    ; --- Test 15: vcvtsi2ss negative ---
    TEST_CASE t15_name
    mov eax, -42
    vxorps xmm0, xmm0, xmm0
    vcvtsi2ss xmm2, xmm0, eax
    ; -42.0f = 0xC2280000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xC2280000

    ; --- Test 16: vcvtsi2sd negative ---
    TEST_CASE t16_name
    mov eax, -42
    vxorpd xmm0, xmm0, xmm0
    vcvtsi2sd xmm2, xmm0, eax
    ; -42.0 = 0xC045000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xC045000000000000

    ; --- Test 17: vcvtss2si negative ---
    TEST_CASE t17_name
    vmovss xmm0, [rel val_ps_neg3]
    vcvtss2si eax, xmm0
    ; -3.0f -> -3
    cmp eax, -3
    je .t17_pass
    CHECK_EQ_32 eax, 0xFFFFFFFD
    jmp .t17_done
.t17_pass:
    CHECK_EQ_32 eax, 0xFFFFFFFD
.t17_done:

    ; --- Test 18: vcvtsd2si negative ---
    TEST_CASE t18_name
    vmovsd xmm0, [rel val_pd_neg3]
    vcvtsd2si eax, xmm0
    CHECK_EQ_32 eax, 0xFFFFFFFD

    ; --- Test 19: vcvttss2si truncate ---
    TEST_CASE t19_name
    vmovss xmm0, [rel val_ps_2p5]
    vcvttss2si eax, xmm0
    ; truncate 2.5 -> 2
    CHECK_EQ_32 eax, 2

    ; --- Test 20: vcvttsd2si truncate ---
    TEST_CASE t20_name
    vmovsd xmm0, [rel val_pd_2p5]
    vcvttsd2si eax, xmm0
    ; truncate 2.5 -> 2
    CHECK_EQ_32 eax, 2

    ; --- Test 21: vcvtsi2ss 64-bit ---
    TEST_CASE t21_name
    mov rax, 1000000
    vxorps xmm0, xmm0, xmm0
    vcvtsi2ss xmm2, xmm0, rax
    ; 1000000.0f = 0x49742400
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x49742400

    ; --- Test 22: vcvtsi2sd 64-bit ---
    TEST_CASE t22_name
    mov rax, 1000000
    vxorpd xmm0, xmm0, xmm0
    vcvtsi2sd xmm2, xmm0, rax
    ; 1000000.0 = 0x412E848000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x412E848000000000

    ; --- Test 23: vcvtss2si 64-bit result ---
    TEST_CASE t23_name
    vmovss xmm0, [rel val_ps_3]
    vcvtss2si rax, xmm0
    CHECK_EQ_64 rax, 3

    ; --- Test 24: vcvtsd2si 64-bit result ---
    TEST_CASE t24_name
    vmovsd xmm0, [rel val_pd_3]
    vcvtsd2si rax, xmm0
    CHECK_EQ_64 rax, 3

    ; --- Test 25: vcvtps2dq rounding (2.5 rounds to 2 with banker's rounding) ---
    TEST_CASE t25_name
    vmovaps xmm0, [rel vec_ps_round]  ; [2.5,-2.5,3.25,-3.25]
    vcvtps2dq xmm2, xmm0
    ; round-to-nearest-even: 2.5->2, -2.5->-2
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFE00000002

    ; --- Test 26: vcvtdq2ps large values ---
    TEST_CASE t26_name
    vmovdqa xmm0, [rel vec_dq_big]  ; [100000,-100000,42,-42]
    vcvtdq2ps xmm2, xmm0
    ; 100000.0f = 0x47C35000, -100000.0f = 0xC7C35000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xC7C3500047C35000

    ; --- Test 27: vcvtps2pd preserves dest high bits from VEX ---
    TEST_CASE t27_name
    vmovaps xmm0, [rel vec_ps_a]
    vcvtps2pd xmm2, xmm0
    ; Check high qword = 2.0 = 0x4000000000000000
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 28: vcvtpd2ps zeroes high 64 bits ---
    TEST_CASE t28_name
    vpcmpeqd xmm2, xmm2, xmm2  ; all ones
    vmovapd xmm0, [rel vec_pd_34]
    vcvtpd2ps xmm2, xmm0
    ; High 64 bits should be zero
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0

    ; --- Test 29: vcvtsi2ss zero ---
    TEST_CASE t29_name
    xor eax, eax
    vxorps xmm0, xmm0, xmm0
    vcvtsi2ss xmm2, xmm0, eax
    ; 0.0f = 0x00000000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0

    ; --- Test 30: vcvttps2dq negative ---
    TEST_CASE t30_name
    vmovaps xmm0, [rel vec_ps_neg]  ; [-3,-4,-5,-6]
    vcvttps2dq xmm2, xmm0
    ; truncate: -3.0->-3, -4.0->-4
    vmovq rax, xmm2
    ; d0=-3=0xFFFFFFFD, d1=-4=0xFFFFFFFC
    CHECK_EQ_64 rax, 0xFFFFFFFCFFFFFFFD

    END_TESTS
