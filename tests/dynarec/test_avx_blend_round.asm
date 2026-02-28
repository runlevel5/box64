; test_avx_blend_round.asm - Test AVX blend, round, hadd, dpps operations (VEX-encoded)
; VBLENDPS/PD, VBLENDVPS/VPD, VPBLENDW, VPBLENDVB, VROUNDPS/PD/SS/SD,
; VDPPS, VDPPD, VHADDPS/PD, VHSUBPS/PD
%include "test_framework.inc"

section .data
    t1_name:  db "vblendps imm=0x5", 0
    t2_name:  db "vblendps imm=0xA", 0
    t3_name:  db "vblendpd imm=0x1", 0
    t4_name:  db "vblendpd imm=0x2", 0
    t5_name:  db "vpblendw imm=0xAA", 0
    t6_name:  db "vpblendw imm=0x55", 0
    t7_name:  db "vblendvps", 0
    t8_name:  db "vblendvpd", 0
    t9_name:  db "vpblendvb", 0
    t10_name: db "vroundss floor", 0
    t11_name: db "vroundss ceil", 0
    t12_name: db "vroundss trunc", 0
    t13_name: db "vroundss nearest", 0
    t14_name: db "vroundsd floor", 0
    t15_name: db "vroundsd ceil", 0
    t16_name: db "vroundps floor", 0
    t17_name: db "vroundps ceil", 0
    t18_name: db "vroundpd floor", 0
    t19_name: db "vroundpd ceil", 0
    t20_name: db "vdpps full", 0
    t21_name: db "vdppd full", 0
    t22_name: db "vhaddps", 0
    t23_name: db "vhaddpd", 0
    t24_name: db "vhsubps", 0
    t25_name: db "vhsubpd", 0

    align 16
    ; Float vectors
    vec_ps_a: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_b: dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000  ; 5,6,7,8
    ; Double vectors
    vec_pd_a: dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0
    vec_pd_b: dq 0x4008000000000000, 0x4010000000000000  ; 3.0, 4.0
    ; Word vectors for VPBLENDW
    vec_w_a: dw 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888
    vec_w_b: dw 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE, 0x1234, 0x5678, 0x9ABC
    ; Byte vectors for VPBLENDVB
    vec_byte_a: db 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00
    vec_byte_b: db 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99
    vec_blend_mask: db 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00
    ; VBLENDVPS mask: sign bits select
    vec_blendvps_mask: dd 0x80000000, 0x00000000, 0x80000000, 0x00000000
    vec_blendvpd_mask: dq 0x8000000000000000, 0x0000000000000000
    ; Rounding test values
    val_ps_2p3: dd 0x40133333  ; 2.3f
    val_ps_2p7: dd 0x402CCCCD  ; 2.7f
    val_ps_neg2p3: dd 0xC0133333  ; -2.3f
    val_pd_2p3: dq 0x4002666666666666  ; 2.3
    val_pd_2p7: dq 0x4005999999999999  ; 2.7 (approximate)
    vec_ps_round: dd 0x40133333, 0x402CCCCD, 0xC0133333, 0xC02CCCCD  ; 2.3,2.7,-2.3,-2.7
    vec_pd_round: dq 0x4002666666666666, 0x4005999999999999  ; 2.3, 2.7 (approx)
    ; For VDPPS: dot product [1,2,3,4].[5,6,7,8] = 5+12+21+32 = 70
    ; For VDPPD: dot product [1,2].[3,4] = 3+8 = 11
    ; For VHADDPS: [1,2,3,4] -> [1+2, 3+4, ...] and [5,6,7,8] -> [5+6, 7+8, ...]
    ; result: [1+2, 3+4, 5+6, 7+8] = [3, 7, 11, 15]

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vblendps imm=0x5 (elem 0,2 from src2) ---
    TEST_CASE t1_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vblendps xmm2, xmm0, xmm1, 0x5
    ; bits 0,2 set: elem0 from src2=5, elem1 from src1=2, elem2 from src2=7, elem3 from src1=4
    ; low64: d0=5.0f=0x40A00000, d1=2.0f=0x40000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000040A00000

    ; --- Test 2: vblendps imm=0xA (elem 1,3 from src2) ---
    TEST_CASE t2_name
    vmovaps xmm0, [rel vec_ps_a]
    vmovaps xmm1, [rel vec_ps_b]
    vblendps xmm2, xmm0, xmm1, 0xA
    ; bits 1,3 set: elem0 from src1=1, elem1 from src2=6, elem2 from src1=3, elem3 from src2=8
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x40C000003F800000

    ; --- Test 3: vblendpd imm=0x1 (elem0 from src2) ---
    TEST_CASE t3_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vblendpd xmm2, xmm0, xmm1, 0x1
    ; q0 from src2=3.0, q1 from src1=2.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 4: vblendpd imm=0x2 (elem1 from src2) ---
    TEST_CASE t4_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovapd xmm1, [rel vec_pd_b]
    vblendpd xmm2, xmm0, xmm1, 0x2
    ; q0 from src1=1.0, q1 from src2=4.0
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4010000000000000

    ; --- Test 5: vpblendw imm=0xAA (even words from src1, odd from src2) ---
    TEST_CASE t5_name
    vmovdqa xmm0, [rel vec_w_a]
    vmovdqa xmm1, [rel vec_w_b]
    vpblendw xmm2, xmm0, xmm1, 0xAA
    ; 0xAA = bits 1,3,5,7 set -> words 1,3,5,7 from src2
    ; w0=src1=0x1111, w1=src2=0xBBBB, w2=src1=0x3333, w3=src2=0xDDDD
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xDDDD3333BBBB1111

    ; --- Test 6: vpblendw imm=0x55 (odd words from src1, even from src2) ---
    TEST_CASE t6_name
    vmovdqa xmm0, [rel vec_w_a]
    vmovdqa xmm1, [rel vec_w_b]
    vpblendw xmm2, xmm0, xmm1, 0x55
    ; 0x55 = bits 0,2,4,6 set -> words 0,2,4,6 from src2
    ; w0=src2=0xAAAA, w1=src1=0x2222, w2=src2=0xCCCC, w3=src1=0x4444
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4444CCCC2222AAAA

    ; --- Test 7: vblendvps (variable blend) ---
    TEST_CASE t7_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vmovaps xmm3, [rel vec_blendvps_mask]  ; [sign,0,sign,0]
    vblendvps xmm2, xmm0, xmm1, xmm3
    ; mask sign bits: elem0=set->src2=5, elem1=clear->src1=2, elem2=set->src2=7, elem3=clear->src1=4
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000040A00000

    ; --- Test 8: vblendvpd (variable blend) ---
    TEST_CASE t8_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vmovapd xmm3, [rel vec_blendvpd_mask]  ; [sign, 0]
    vblendvpd xmm2, xmm0, xmm1, xmm3
    ; q0: sign set -> src2=3.0, q1: sign clear -> src1=2.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 9: vpblendvb (variable byte blend) ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_byte_a]
    vmovdqa xmm1, [rel vec_byte_b]
    vmovdqa xmm3, [rel vec_blend_mask]  ; alternating 0x80,0x00
    vpblendvb xmm2, xmm0, xmm1, xmm3
    ; byte0: mask=0x80 -> src2=0xAA, byte1: mask=0x00 -> src1=0x22
    ; byte2: mask=0x80 -> src2=0xCC, byte3: mask=0x00 -> src1=0x44
    ; low64 bytes: 0xAA, 0x22, 0xCC, 0x44, 0xEE, 0x66, 0x00, 0x88
    ; LE qword: 0x8800660xEE4400xCC2200xAA ... let me compute carefully
    ; byte7=0x88, byte6=0x00, byte5=0x66, byte4=0xEE, byte3=0x44, byte2=0xCC, byte1=0x22, byte0=0xAA
    ; = 0x880066EE44CC22AA
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x880066EE44CC22AA

    ; --- Test 10: vroundss floor (imm=1) ---
    TEST_CASE t10_name
    vmovss xmm0, [rel val_ps_2p3]  ; 2.3
    vxorps xmm1, xmm1, xmm1
    vroundss xmm2, xmm1, xmm0, 1  ; floor
    ; floor(2.3) = 2.0f = 0x40000000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40000000

    ; --- Test 11: vroundss ceil (imm=2) ---
    TEST_CASE t11_name
    vmovss xmm0, [rel val_ps_2p3]
    vxorps xmm1, xmm1, xmm1
    vroundss xmm2, xmm1, xmm0, 2  ; ceil
    ; ceil(2.3) = 3.0f = 0x40400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 12: vroundss truncate (imm=3) ---
    TEST_CASE t12_name
    vmovss xmm0, [rel val_ps_neg2p3]  ; -2.3
    vxorps xmm1, xmm1, xmm1
    vroundss xmm2, xmm1, xmm0, 3  ; truncate
    ; trunc(-2.3) = -2.0f = 0xC0000000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xC0000000

    ; --- Test 13: vroundss nearest (imm=0) ---
    TEST_CASE t13_name
    vmovss xmm0, [rel val_ps_2p7]  ; 2.7
    vxorps xmm1, xmm1, xmm1
    vroundss xmm2, xmm1, xmm0, 0  ; nearest
    ; round(2.7) = 3.0f = 0x40400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 14: vroundsd floor (imm=1) ---
    TEST_CASE t14_name
    vmovsd xmm0, [rel val_pd_2p3]  ; 2.3
    vxorpd xmm1, xmm1, xmm1
    vroundsd xmm2, xmm1, xmm0, 1  ; floor
    ; floor(2.3) = 2.0 = 0x4000000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 15: vroundsd ceil (imm=2) ---
    TEST_CASE t15_name
    vmovsd xmm0, [rel val_pd_2p3]
    vxorpd xmm1, xmm1, xmm1
    vroundsd xmm2, xmm1, xmm0, 2  ; ceil
    ; ceil(2.3) = 3.0 = 0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 16: vroundps floor ---
    TEST_CASE t16_name
    vmovaps xmm0, [rel vec_ps_round]  ; [2.3, 2.7, -2.3, -2.7]
    vroundps xmm2, xmm0, 1  ; floor
    ; floor(2.3)=2, floor(2.7)=2, floor(-2.3)=-3, floor(-2.7)=-3
    ; low64: d0=2.0f=0x40000000, d1=2.0f=0x40000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000040000000

    ; --- Test 17: vroundps ceil ---
    TEST_CASE t17_name
    vmovaps xmm0, [rel vec_ps_round]
    vroundps xmm2, xmm0, 2  ; ceil
    ; ceil(2.3)=3, ceil(2.7)=3, ceil(-2.3)=-2, ceil(-2.7)=-2
    vmovq rax, xmm2
    ; d0=3.0f=0x40400000, d1=3.0f=0x40400000
    CHECK_EQ_64 rax, 0x4040000040400000

    ; --- Test 18: vroundpd floor ---
    TEST_CASE t18_name
    vmovapd xmm0, [rel vec_pd_round]  ; [2.3, 2.7]
    vroundpd xmm2, xmm0, 1  ; floor
    ; floor(2.3)=2.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 19: vroundpd ceil ---
    TEST_CASE t19_name
    vmovapd xmm0, [rel vec_pd_round]
    vroundpd xmm2, xmm0, 2  ; ceil
    ; ceil(2.3)=3.0 = 0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 20: vdpps full dot product ---
    TEST_CASE t20_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vdpps xmm2, xmm0, xmm1, 0xFF
    ; imm=0xFF: multiply all 4 pairs, store result in all 4 positions
    ; 1*5+2*6+3*7+4*8 = 5+12+21+32 = 70
    ; 70.0f = 0x428C0000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x428C0000

    ; --- Test 21: vdppd full dot product ---
    TEST_CASE t21_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vdppd xmm2, xmm0, xmm1, 0x33
    ; imm=0x33: multiply both pairs (bits 4,5), store in both positions (bits 0,1)
    ; 1*3+2*4 = 3+8 = 11
    ; 11.0 = 0x4026000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4026000000000000

    ; --- Test 22: vhaddps (horizontal add) ---
    TEST_CASE t22_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vhaddps xmm2, xmm0, xmm1
    ; from xmm0: 1+2=3, 3+4=7; from xmm1: 5+6=11, 7+8=15
    ; result: [3, 7, 11, 15]
    ; low64: d0=3.0f=0x40400000, d1=7.0f=0x40E00000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x40E0000040400000

    ; --- Test 23: vhaddpd (horizontal add double) ---
    TEST_CASE t23_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vhaddpd xmm2, xmm0, xmm1
    ; from xmm0: 1+2=3; from xmm1: 3+4=7
    ; result: [3.0, 7.0]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 24: vhsubps ---
    TEST_CASE t24_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vhsubps xmm2, xmm0, xmm1
    ; from xmm0: 1-2=-1, 3-4=-1; from xmm1: 5-6=-1, 7-8=-1
    ; result: [-1, -1, -1, -1]
    ; -1.0f = 0xBF800000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xBF800000BF800000

    ; --- Test 25: vhsubpd ---
    TEST_CASE t25_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vhsubpd xmm2, xmm0, xmm1
    ; from xmm0: 1-2=-1; from xmm1: 3-4=-1
    ; result: [-1.0, -1.0]
    ; -1.0 = 0xBFF0000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xBFF0000000000000

    END_TESTS
