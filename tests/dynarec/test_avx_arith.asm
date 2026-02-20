; test_avx_arith.asm - Test AVX floating-point arithmetic (VEX-encoded)
; VADDPS/PD/SS/SD, VSUBPS/PD/SS/SD, VMULPS/PD/SS/SD, VDIVPS/PD/SS/SD,
; VSQRTPS/PD/SS/SD, VMINPS/PD/SS/SD, VMAXPS/PD/SS/SD, VRCPPS, VRSQRTPS
%include "test_framework.inc"

section .data
    t1_name:  db "vaddss 1+2=3", 0
    t2_name:  db "vaddsd 1+2=3", 0
    t3_name:  db "vaddps [1,2,3,4]+[5,6,7,8]", 0
    t4_name:  db "vaddpd [1,2]+[3,4]", 0
    t5_name:  db "vsubss 5-3=2", 0
    t6_name:  db "vsubsd 5-3=2", 0
    t7_name:  db "vsubps [5,6,7,8]-[1,2,3,4]", 0
    t8_name:  db "vsubpd [5,6]-[1,2]", 0
    t9_name:  db "vmulss 3*4=12", 0
    t10_name: db "vmulsd 3*4=12", 0
    t11_name: db "vmulps [1,2,3,4]*[5,6,7,8]", 0
    t12_name: db "vmulpd [2,3]*[4,5]", 0
    t13_name: db "vdivss 12/4=3", 0
    t14_name: db "vdivsd 12/4=3", 0
    t15_name: db "vdivps [12,24,36,48]/[4,8,12,16]", 0
    t16_name: db "vdivpd [12,24]/[4,8]", 0
    t17_name: db "vsqrtss 16=4", 0
    t18_name: db "vsqrtsd 16=4", 0
    t19_name: db "vsqrtps [4,9,16,25]", 0
    t20_name: db "vsqrtpd [4,16]", 0
    t21_name: db "vminss 3,7=3", 0
    t22_name: db "vminsd 3,7=3", 0
    t23_name: db "vminps [3,1,7,2],[5,0,4,8]", 0
    t24_name: db "vminpd [3,7],[5,1]", 0
    t25_name: db "vmaxss 3,7=7", 0
    t26_name: db "vmaxsd 3,7=7", 0
    t27_name: db "vmaxps [3,1,7,2],[5,0,4,8]", 0
    t28_name: db "vmaxpd [3,7],[5,1]", 0
    t29_name: db "vaddss 3-op preserves src", 0
    t30_name: db "vaddps upper xmm zeroed", 0
    t31_name: db "vrcpps [1,4,0.25,2]", 0
    t32_name: db "vrsqrtps [1,4,16,25]", 0
    t33_name: db "vsqrtsd preserves hi", 0
    t34_name: db "vmulss neg*neg=pos", 0
    t35_name: db "vdivpd [1,1]/[3,7]", 0

    align 16
    vec_ps_1234:  dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_5678:  dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000  ; 5,6,7,8
    vec_ps_res_add: dd 0x40C00000, 0x41000000, 0x41200000, 0x41400000  ; 6,8,10,12
    vec_ps_res_sub: dd 0x40800000, 0x40800000, 0x40800000, 0x40800000  ; 4,4,4,4
    vec_ps_res_mul: dd 0x40A00000, 0x41400000, 0x41A80000, 0x42000000  ; 5,12,21,32
    vec_ps_div_a:   dd 0x41400000, 0x41C00000, 0x42100000, 0x42400000  ; 12,24,36,48
    vec_ps_div_b:   dd 0x40800000, 0x41000000, 0x41400000, 0x41800000  ; 4,8,12,16
    vec_ps_sqrt_in: dd 0x40800000, 0x41100000, 0x41800000, 0x41C80000  ; 4,9,16,25
    vec_ps_min_a:   dd 0x40400000, 0x3F800000, 0x40E00000, 0x40000000  ; 3,1,7,2
    vec_ps_min_b:   dd 0x40A00000, 0x00000000, 0x40800000, 0x41000000  ; 5,0,4,8
    vec_ps_rcp_in:  dd 0x3F800000, 0x40800000, 0x3E800000, 0x40000000  ; 1,4,0.25,2
    vec_ps_rsqrt_in: dd 0x3F800000, 0x40800000, 0x41800000, 0x41C80000 ; 1,4,16,25
    ; f32: 1.0=3F800000 2.0=40000000 3.0=40400000 4.0=40800000 5.0=40A00000
    ;      7.0=40E00000 12.0=41400000 16.0=41800000 -3.0=C0400000
    val_ps_1: dd 0x3F800000  ; 1.0f
    val_ps_2: dd 0x40000000
    val_ps_3: dd 0x40400000
    val_ps_4: dd 0x40800000
    val_ps_5: dd 0x40A00000
    val_ps_7: dd 0x40E00000
    val_ps_12: dd 0x41400000
    val_ps_16: dd 0x41800000
    val_ps_neg3: dd 0xC0400000  ; -3.0f

    align 16
    ; Double-precision vectors
    ; f64: 1.0=3FF0000000000000 2.0=4000000000000000 3.0=4008000000000000
    ;      4.0=4010000000000000 5.0=4014000000000000 6.0=4018000000000000
    ;      7.0=401C000000000000 12.0=4028000000000000 16.0=4030000000000000
    ;      24.0=4038000000000000
    vec_pd_12: dq 0x3FF0000000000000, 0x4000000000000000  ; 1,2
    vec_pd_34: dq 0x4008000000000000, 0x4010000000000000  ; 3,4
    vec_pd_56: dq 0x4014000000000000, 0x4018000000000000  ; 5,6
    vec_pd_23: dq 0x4000000000000000, 0x4008000000000000  ; 2,3
    vec_pd_45: dq 0x4010000000000000, 0x4014000000000000  ; 4,5
    vec_pd_37: dq 0x4008000000000000, 0x401C000000000000  ; 3,7
    vec_pd_51: dq 0x4014000000000000, 0x3FF0000000000000  ; 5,1
    vec_pd_12_24: dq 0x4028000000000000, 0x4038000000000000 ; 12,24
    vec_pd_4_8:   dq 0x4010000000000000, 0x4020000000000000 ; 4,8
    vec_pd_4_16:  dq 0x4010000000000000, 0x4030000000000000 ; 4,16
    vec_pd_11:    dq 0x3FF0000000000000, 0x3FF0000000000000 ; 1,1
    vec_pd_3_7:   dq 0x4008000000000000, 0x401C000000000000 ; 3,7
    val_pd_1: dq 0x3FF0000000000000
    val_pd_2: dq 0x4000000000000000
    val_pd_3: dq 0x4008000000000000
    val_pd_4: dq 0x4010000000000000
    val_pd_5: dq 0x4014000000000000
    val_pd_7: dq 0x401C000000000000
    val_pd_12: dq 0x4028000000000000
    val_pd_16: dq 0x4030000000000000

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vaddss ---
    TEST_CASE t1_name
    vmovss xmm0, [rel val_ps_1]
    vmovss xmm1, [rel val_ps_2]
    vaddss xmm2, xmm0, xmm1
    ; result should be 3.0f = 0x40400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 2: vaddsd ---
    TEST_CASE t2_name
    vmovsd xmm0, [rel val_pd_1]
    vmovsd xmm1, [rel val_pd_2]
    vaddsd xmm2, xmm0, xmm1
    ; result = 3.0 = 0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 3: vaddps ---
    TEST_CASE t3_name
    vmovaps xmm0, [rel vec_ps_1234]
    vmovaps xmm1, [rel vec_ps_5678]
    vaddps xmm2, xmm0, xmm1
    ; result = [6,8,10,12] low64 = 0x4100000040C00000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4100000040C00000

    ; --- Test 4: vaddpd ---
    TEST_CASE t4_name
    vmovapd xmm0, [rel vec_pd_12]
    vmovapd xmm1, [rel vec_pd_34]
    vaddpd xmm2, xmm0, xmm1
    ; result = [1+3,2+4] = [4,6] low = 0x4010000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4010000000000000

    ; --- Test 5: vsubss ---
    TEST_CASE t5_name
    vmovss xmm0, [rel val_ps_5]
    vmovss xmm1, [rel val_ps_3]
    vsubss xmm2, xmm0, xmm1
    ; 5-3=2 = 0x40000000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40000000

    ; --- Test 6: vsubsd ---
    TEST_CASE t6_name
    vmovsd xmm0, [rel val_pd_5]
    vmovsd xmm1, [rel val_pd_3]
    vsubsd xmm2, xmm0, xmm1
    ; 5-3=2 = 0x4000000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 7: vsubps ---
    TEST_CASE t7_name
    vmovaps xmm0, [rel vec_ps_5678]
    vmovaps xmm1, [rel vec_ps_1234]
    vsubps xmm2, xmm0, xmm1
    ; [5-1,6-2,7-3,8-4] = [4,4,4,4] low64 = 0x4080000040800000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4080000040800000

    ; --- Test 8: vsubpd ---
    TEST_CASE t8_name
    vmovapd xmm0, [rel vec_pd_56]
    vmovapd xmm1, [rel vec_pd_12]
    vsubpd xmm2, xmm0, xmm1
    ; [5-1,6-2] = [4,4] low = 0x4010000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4010000000000000

    ; --- Test 9: vmulss ---
    TEST_CASE t9_name
    vmovss xmm0, [rel val_ps_3]
    vmovss xmm1, [rel val_ps_4]
    vmulss xmm2, xmm0, xmm1
    ; 3*4=12 = 0x41400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x41400000

    ; --- Test 10: vmulsd ---
    TEST_CASE t10_name
    vmovsd xmm0, [rel val_pd_3]
    vmovsd xmm1, [rel val_pd_4]
    vmulsd xmm2, xmm0, xmm1
    ; 3*4=12 = 0x4028000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4028000000000000

    ; --- Test 11: vmulps ---
    TEST_CASE t11_name
    vmovaps xmm0, [rel vec_ps_1234]
    vmovaps xmm1, [rel vec_ps_5678]
    vmulps xmm2, xmm0, xmm1
    ; [1*5,2*6,3*7,4*8]=[5,12,21,32] low64=0x4140000040A00000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4140000040A00000

    ; --- Test 12: vmulpd ---
    TEST_CASE t12_name
    vmovapd xmm0, [rel vec_pd_23]
    vmovapd xmm1, [rel vec_pd_45]
    vmulpd xmm2, xmm0, xmm1
    ; [2*4,3*5]=[8,15] low=0x4020000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4020000000000000

    ; --- Test 13: vdivss ---
    TEST_CASE t13_name
    vmovss xmm0, [rel val_ps_12]
    vmovss xmm1, [rel val_ps_4]
    vdivss xmm2, xmm0, xmm1
    ; 12/4=3 = 0x40400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 14: vdivsd ---
    TEST_CASE t14_name
    vmovsd xmm0, [rel val_pd_12]
    vmovsd xmm1, [rel val_pd_4]
    vdivsd xmm2, xmm0, xmm1
    ; 12/4=3 = 0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 15: vdivps ---
    TEST_CASE t15_name
    vmovaps xmm0, [rel vec_ps_div_a]
    vmovaps xmm1, [rel vec_ps_div_b]
    vdivps xmm2, xmm0, xmm1
    ; [12/4,24/8,36/12,48/16]=[3,3,3,3] low64=0x4040000040400000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4040000040400000

    ; --- Test 16: vdivpd ---
    TEST_CASE t16_name
    vmovapd xmm0, [rel vec_pd_12_24]
    vmovapd xmm1, [rel vec_pd_4_8]
    vdivpd xmm2, xmm0, xmm1
    ; [12/4,24/8]=[3,3] low=0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 17: vsqrtss ---
    TEST_CASE t17_name
    vmovss xmm0, [rel val_ps_16]
    vsqrtss xmm2, xmm0, xmm0
    ; sqrt(16)=4 = 0x40800000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40800000

    ; --- Test 18: vsqrtsd ---
    TEST_CASE t18_name
    vmovsd xmm0, [rel val_pd_16]
    vsqrtsd xmm2, xmm0, xmm0
    ; sqrt(16)=4 = 0x4010000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4010000000000000

    ; --- Test 19: vsqrtps ---
    TEST_CASE t19_name
    vmovaps xmm0, [rel vec_ps_sqrt_in]
    vsqrtps xmm2, xmm0
    ; sqrt([4,9,16,25])=[2,3,4,5] low64=0x4040000040000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4040000040000000

    ; --- Test 20: vsqrtpd ---
    TEST_CASE t20_name
    vmovapd xmm0, [rel vec_pd_4_16]
    vsqrtpd xmm2, xmm0
    ; sqrt([4,16])=[2,4] low=0x4000000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 21: vminss ---
    TEST_CASE t21_name
    vmovss xmm0, [rel val_ps_3]
    vmovss xmm1, [rel val_ps_7]
    vminss xmm2, xmm0, xmm1
    ; min(3,7)=3 = 0x40400000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 22: vminsd ---
    TEST_CASE t22_name
    vmovsd xmm0, [rel val_pd_3]
    vmovsd xmm1, [rel val_pd_7]
    vminsd xmm2, xmm0, xmm1
    ; min(3,7)=3 = 0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 23: vminps ---
    TEST_CASE t23_name
    vmovaps xmm0, [rel vec_ps_min_a]
    vmovaps xmm1, [rel vec_ps_min_b]
    vminps xmm2, xmm0, xmm1
    ; min([3,1,7,2],[5,0,4,8])=[3,0,4,2] low64=0x0000000040400000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000040400000

    ; --- Test 24: vminpd ---
    TEST_CASE t24_name
    vmovapd xmm0, [rel vec_pd_37]
    vmovapd xmm1, [rel vec_pd_51]
    vminpd xmm2, xmm0, xmm1
    ; min([3,7],[5,1])=[3,1] low=0x4008000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4008000000000000

    ; --- Test 25: vmaxss ---
    TEST_CASE t25_name
    vmovss xmm0, [rel val_ps_3]
    vmovss xmm1, [rel val_ps_7]
    vmaxss xmm2, xmm0, xmm1
    ; max(3,7)=7 = 0x40E00000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x40E00000

    ; --- Test 26: vmaxsd ---
    TEST_CASE t26_name
    vmovsd xmm0, [rel val_pd_3]
    vmovsd xmm1, [rel val_pd_7]
    vmaxsd xmm2, xmm0, xmm1
    ; max(3,7)=7 = 0x401C000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x401C000000000000

    ; --- Test 27: vmaxps ---
    TEST_CASE t27_name
    vmovaps xmm0, [rel vec_ps_min_a]
    vmovaps xmm1, [rel vec_ps_min_b]
    vmaxps xmm2, xmm0, xmm1
    ; max([3,1,7,2],[5,0,4,8])=[5,1,7,8] low64=0x3F80000040A00000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3F80000040A00000

    ; --- Test 28: vmaxpd ---
    TEST_CASE t28_name
    vmovapd xmm0, [rel vec_pd_37]
    vmovapd xmm1, [rel vec_pd_51]
    vmaxpd xmm2, xmm0, xmm1
    ; max([3,7],[5,1])=[5,7] low=0x4014000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4014000000000000

    ; --- Test 29: vaddss 3-operand preserves src1 ---
    TEST_CASE t29_name
    vmovss xmm3, [rel val_ps_1]
    vmovss xmm4, [rel val_ps_2]
    vaddss xmm5, xmm3, xmm4
    ; xmm3 should still be 1.0
    movd eax, xmm3
    CHECK_EQ_32 eax, 0x3F800000

    ; --- Test 30: vaddps 128-bit zeroes upper xmm bits ---
    TEST_CASE t30_name
    ; Fill xmm6 with all 1s first (using 128-bit op)
    vpcmpeqd xmm6, xmm6, xmm6     ; all ones
    ; Now do 128-bit vaddps — upper bits of ymm6 should be zeroed
    vmovaps xmm0, [rel vec_ps_1234]
    vmovaps xmm1, [rel vec_ps_5678]
    vaddps xmm6, xmm0, xmm1
    ; Extract high 128 bits — should be zero
    ; Use vextractf128 to get upper half of ymm6
    vextractf128 xmm7, ymm6, 1
    vmovq rax, xmm7
    CHECK_EQ_64 rax, 0

    ; --- Test 31: vrcpps approximate reciprocal ---
    TEST_CASE t31_name
    vmovaps xmm0, [rel vec_ps_rcp_in]
    vrcpps xmm2, xmm0
    ; rcp([1,4,0.25,2]) ~ [1, 0.25, 4, 0.5]
    ; Check low dword (rcp(1.0) ~= 1.0 = 0x3F800000 approximately)
    ; RCPPS is approximate, allow +-1 ULP. Just check it's not totally wrong.
    ; rcp(1.0) should be very close to 0x3F800000
    movd eax, xmm2
    ; Approximate: 0x3F7FFF00 to 0x3F800100 range
    ; Just check top 16 bits = 0x3F80 (within ~0.1%)
    shr eax, 16
    CHECK_EQ_32 eax, 0x3F7F  ; top bits of ~1.0 (RCPPS gives slightly under 1.0)

    ; --- Test 32: vrsqrtps approximate reciprocal sqrt ---
    TEST_CASE t32_name
    vmovaps xmm0, [rel vec_ps_rsqrt_in]
    vrsqrtps xmm2, xmm0
    ; rsqrt([1,4,16,25]) ~ [1, 0.5, 0.25, 0.2]
    ; rsqrt(1)~=1.0f, check top 16 bits
    movd eax, xmm2
    shr eax, 16
    CHECK_EQ_32 eax, 0x3F7F  ; ~1.0

    ; --- Test 33: vsqrtsd preserves high qword of src1 ---
    TEST_CASE t33_name
    vmovapd xmm0, [rel vec_pd_37]  ; xmm0 = [3.0, 7.0]
    vmovsd xmm1, [rel val_pd_16]   ; xmm1 low = 16.0
    vsqrtsd xmm2, xmm0, xmm1
    ; xmm2 low = sqrt(16)=4, high = xmm0 high = 7.0
    ; Check high qword
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x401C000000000000

    ; --- Test 34: vmulss neg*neg=pos ---
    TEST_CASE t34_name
    vmovss xmm0, [rel val_ps_neg3]
    vmovss xmm1, [rel val_ps_neg3]
    vmulss xmm2, xmm0, xmm1
    ; (-3)*(-3) = 9 = 0x41100000
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x41100000

    ; --- Test 35: vdivpd [1,1]/[3,7] ---
    TEST_CASE t35_name
    vmovapd xmm0, [rel vec_pd_11]
    vmovapd xmm1, [rel vec_pd_3_7]
    vdivpd xmm2, xmm0, xmm1
    ; [1/3, 1/7] low = 1/3 = 0x3FD5555555555555
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FD5555555555555

    END_TESTS
