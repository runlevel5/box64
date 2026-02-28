; test_avx_cmp.asm - Test AVX comparison operations (VEX-encoded)
; VCMPPS/PD/SS/SD, VPCMPEQB/W/D/Q, VPCMPGTB/W/D/Q, VUCOMISS/SD, VCOMISD
%include "test_framework.inc"

section .data
    t1_name:  db "vcmpss eq", 0
    t2_name:  db "vcmpss lt", 0
    t3_name:  db "vcmpss le", 0
    t4_name:  db "vcmpsd eq", 0
    t5_name:  db "vcmpsd lt", 0
    t6_name:  db "vcmpps eq", 0
    t7_name:  db "vcmpps lt", 0
    t8_name:  db "vcmpps neq", 0
    t9_name:  db "vcmppd eq", 0
    t10_name: db "vcmppd lt", 0
    t11_name: db "vpcmpeqb match", 0
    t12_name: db "vpcmpeqb nomatch", 0
    t13_name: db "vpcmpeqw match", 0
    t14_name: db "vpcmpeqd match", 0
    t15_name: db "vpcmpeqq match", 0
    t16_name: db "vpcmpgtb", 0
    t17_name: db "vpcmpgtw", 0
    t18_name: db "vpcmpgtd", 0
    t19_name: db "vpcmpgtq", 0
    t20_name: db "vucomiss eq ZF", 0
    t21_name: db "vucomiss gt CF", 0
    t22_name: db "vucomiss lt CF", 0
    t23_name: db "vucomisd eq ZF", 0
    t24_name: db "vucomisd lt CF", 0
    t25_name: db "vcmpps ord", 0
    t26_name: db "vcmpss nlt", 0
    t27_name: db "vpcmpeqd self", 0
    t28_name: db "vpcmpgtb neg", 0
    t29_name: db "vcomisd gt", 0
    t30_name: db "vcmpps unord NaN", 0

    align 16
    ; Float values
    val_ps_1: dd 0x3F800000  ; 1.0
    val_ps_2: dd 0x40000000  ; 2.0
    val_ps_3: dd 0x40400000  ; 3.0
    val_pd_1: dq 0x3FF0000000000000  ; 1.0
    val_pd_2: dq 0x4000000000000000  ; 2.0
    val_pd_3: dq 0x4008000000000000  ; 3.0
    ; Float vectors
    vec_ps_a: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_b: dd 0x3F800000, 0x40400000, 0x40000000, 0x40800000  ; 1,3,2,4
    vec_pd_a: dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0
    vec_pd_b: dq 0x3FF0000000000000, 0x4008000000000000  ; 1.0, 3.0
    ; NaN vector
    vec_ps_nan: dd 0x7FC00000, 0x3F800000, 0x7FC00000, 0x40000000  ; NaN,1,NaN,2
    vec_ps_reg: dd 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000  ; 1,1,1,1
    ; Integer byte vectors
    vec_ib_a: db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
    vec_ib_b: db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
    vec_ib_c: db 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    vec_ib_neg: db 0xFF, 0x80, 0x01, 0x00, 0x7F, 0xFE, 0x02, 0x03, 0xFF, 0x80, 0x01, 0x00, 0x7F, 0xFE, 0x02, 0x03
    ; Integer word vectors
    vec_iw_a: dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_iw_b: dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_iw_c: dw 0, 1, 2, 3, 4, 5, 6, 7
    ; Integer dword vectors
    vec_id_a: dd 10, 20, 30, 40
    vec_id_b: dd 10, 20, 30, 40
    vec_id_c: dd 5, 15, 25, 35
    ; Integer qword vectors
    vec_iq_a: dq 100, 200
    vec_iq_b: dq 100, 200
    vec_iq_c: dq 50, 150

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vcmpss eq (imm=0) ---
    TEST_CASE t1_name
    vmovss xmm0, [rel val_ps_1]
    vmovss xmm1, [rel val_ps_1]
    vcmpss xmm2, xmm0, xmm1, 0  ; EQ
    ; 1.0 == 1.0 -> true -> low dword = 0xFFFFFFFF
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; --- Test 2: vcmpss lt (imm=1) ---
    TEST_CASE t2_name
    vmovss xmm0, [rel val_ps_1]
    vmovss xmm1, [rel val_ps_2]
    vcmpss xmm2, xmm0, xmm1, 1  ; LT
    ; 1.0 < 2.0 -> true
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; --- Test 3: vcmpss le (imm=2) ---
    TEST_CASE t3_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_1]
    vcmpss xmm2, xmm0, xmm1, 2  ; LE
    ; 2.0 <= 1.0 -> false
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x00000000

    ; --- Test 4: vcmpsd eq (imm=0) ---
    TEST_CASE t4_name
    vmovsd xmm0, [rel val_pd_1]
    vmovsd xmm1, [rel val_pd_1]
    vcmpsd xmm2, xmm0, xmm1, 0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 5: vcmpsd lt (imm=1) ---
    TEST_CASE t5_name
    vmovsd xmm0, [rel val_pd_2]
    vmovsd xmm1, [rel val_pd_1]
    vcmpsd xmm2, xmm0, xmm1, 1  ; LT
    ; 2.0 < 1.0 -> false
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 6: vcmpps eq (imm=0) ---
    TEST_CASE t6_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [1,3,2,4]
    vcmpps xmm2, xmm0, xmm1, 0   ; EQ per element
    ; elem0: 1==1 -> 0xFFFFFFFF, elem1: 2==3 -> 0, elem2: 3==2 -> 0, elem3: 4==4 -> 0xFFFFFFFF
    ; low64: d0=0xFFFFFFFF, d1=0x00000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    ; --- Test 7: vcmpps lt (imm=1) ---
    TEST_CASE t7_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [1,3,2,4]
    vcmpps xmm2, xmm0, xmm1, 1   ; LT
    ; elem0: 1<1 -> false, elem1: 2<3 -> true, elem2: 3<2 -> false, elem3: 4<4 -> false
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFF00000000

    ; --- Test 8: vcmpps neq (imm=4) ---
    TEST_CASE t8_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [1,3,2,4]
    vcmpps xmm2, xmm0, xmm1, 4   ; NEQ
    ; elem0: 1!=1->false, elem1: 2!=3->true, elem2: 3!=2->true, elem3: 4!=4->false
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFF00000000

    ; --- Test 9: vcmppd eq ---
    TEST_CASE t9_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [1.0, 3.0]
    vcmppd xmm2, xmm0, xmm1, 0   ; EQ
    ; q0: 1.0==1.0 -> all 1s, q1: 2.0==3.0 -> all 0s
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 10: vcmppd lt ---
    TEST_CASE t10_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [1.0, 3.0]
    vcmppd xmm2, xmm0, xmm1, 1   ; LT
    ; q0: 1.0<1.0 -> false, q1: 2.0<3.0 -> true
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 11: vpcmpeqb match ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_ib_a]
    vmovdqa xmm1, [rel vec_ib_b]  ; same values
    vpcmpeqb xmm2, xmm0, xmm1
    ; all bytes equal -> all 0xFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 12: vpcmpeqb nomatch ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_ib_a]  ; 1,2,3,...
    vmovdqa xmm1, [rel vec_ib_c]  ; 0,1,2,...
    vpcmpeqb xmm2, xmm0, xmm1
    ; no bytes match -> all 0x00
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 13: vpcmpeqw match ---
    TEST_CASE t13_name
    vmovdqa xmm0, [rel vec_iw_a]
    vmovdqa xmm1, [rel vec_iw_b]
    vpcmpeqw xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 14: vpcmpeqd match ---
    TEST_CASE t14_name
    vmovdqa xmm0, [rel vec_id_a]
    vmovdqa xmm1, [rel vec_id_b]
    vpcmpeqd xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 15: vpcmpeqq match ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_iq_a]
    vmovdqa xmm1, [rel vec_iq_b]
    vpcmpeqq xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 16: vpcmpgtb ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_ib_a]  ; 1,2,3,...
    vmovdqa xmm1, [rel vec_ib_c]  ; 0,1,2,...
    vpcmpgtb xmm2, xmm0, xmm1
    ; a[i] > c[i] for all bytes -> all 0xFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 17: vpcmpgtw ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_iw_a]  ; 1,2,3,...
    vmovdqa xmm1, [rel vec_iw_c]  ; 0,1,2,...
    vpcmpgtw xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 18: vpcmpgtd ---
    TEST_CASE t18_name
    vmovdqa xmm0, [rel vec_id_a]  ; 10,20,30,40
    vmovdqa xmm1, [rel vec_id_c]  ; 5,15,25,35
    vpcmpgtd xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 19: vpcmpgtq ---
    TEST_CASE t19_name
    vmovdqa xmm0, [rel vec_iq_a]  ; 100,200
    vmovdqa xmm1, [rel vec_iq_c]  ; 50,150
    vpcmpgtq xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 20: vucomiss equal -> ZF=1 ---
    TEST_CASE t20_name
    vmovss xmm0, [rel val_ps_1]
    vmovss xmm1, [rel val_ps_1]
    vucomiss xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; --- Test 21: vucomiss src1 > src2 -> CF=0 ---
    TEST_CASE t21_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_1]
    vucomiss xmm0, xmm1
    ; 2.0 > 1.0: CF=0, ZF=0
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; --- Test 22: vucomiss src1 < src2 -> CF=1 ---
    TEST_CASE t22_name
    vmovss xmm0, [rel val_ps_1]
    vmovss xmm1, [rel val_ps_2]
    vucomiss xmm0, xmm1
    ; 1.0 < 2.0: CF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test 23: vucomisd equal -> ZF=1 ---
    TEST_CASE t23_name
    vmovsd xmm0, [rel val_pd_1]
    vmovsd xmm1, [rel val_pd_1]
    vucomisd xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; --- Test 24: vucomisd src1 < src2 -> CF=1 ---
    TEST_CASE t24_name
    vmovsd xmm0, [rel val_pd_1]
    vmovsd xmm1, [rel val_pd_2]
    vucomisd xmm0, xmm1
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, CF

    ; --- Test 25: vcmpps ordered (imm=7) ---
    TEST_CASE t25_name
    vmovaps xmm0, [rel vec_ps_a]
    vmovaps xmm1, [rel vec_ps_b]
    vcmpps xmm2, xmm0, xmm1, 7  ; ORD
    ; All non-NaN -> all true
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 26: vcmpss NLT (imm=5, not-less-than) ---
    TEST_CASE t26_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_1]
    vcmpss xmm2, xmm0, xmm1, 5  ; NLT (>=)
    ; 2.0 >= 1.0 -> true
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; --- Test 27: vpcmpeqd self -> all ones ---
    TEST_CASE t27_name
    vmovdqa xmm0, [rel vec_id_a]
    vpcmpeqd xmm2, xmm0, xmm0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 28: vpcmpgtb with negative values ---
    TEST_CASE t28_name
    vmovdqa xmm0, [rel vec_ib_a]   ; 1,2,3,...
    vmovdqa xmm1, [rel vec_ib_neg] ; 0xFF(-1),0x80(-128),0x01,0x00,...
    vpcmpgtb xmm2, xmm0, xmm1
    ; byte0: 1 > -1 (0xFF signed) -> true (0xFF)
    ; byte1: 2 > -128 (0x80 signed) -> true (0xFF)
    ; byte2: 3 > 1 -> true
    ; byte3: 4 > 0 -> true
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xFFFFFFFF

    ; --- Test 29: vcomisd src1 > src2 ---
    TEST_CASE t29_name
    vmovsd xmm0, [rel val_pd_3]
    vmovsd xmm1, [rel val_pd_1]
    vcomisd xmm0, xmm1
    ; 3.0 > 1.0: CF=0, ZF=0
    SAVE_FLAGS
    CHECK_FLAGS_EQ CF, 0

    ; --- Test 30: vcmpps unordered with NaN (imm=3) ---
    TEST_CASE t30_name
    vmovaps xmm0, [rel vec_ps_nan]  ; [NaN,1,NaN,2]
    vmovaps xmm1, [rel vec_ps_reg]  ; [1,1,1,1]
    vcmpps xmm2, xmm0, xmm1, 3    ; UNORD
    ; elem0: NaN involved -> true, elem1: both normal -> false
    ; elem2: NaN involved -> true, elem3: both normal -> false
    ; low64: d0=0xFFFFFFFF, d1=0x00000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    END_TESTS
