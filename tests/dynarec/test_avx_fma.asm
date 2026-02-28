; test_avx_fma.asm - Test AVX FMA3 operations (VEX-encoded)
; VFMADD132/213/231 PS/PD/SS/SD, VFMSUB132/213/231 PS/PD/SS/SD,
; VFNMADD132/213/231 PS/PD/SS/SD, VFNMSUB132/213/231 PS/PD/SS/SD
; FMA: fused multiply-add with various operand orderings
%include "test_framework.inc"

section .data
    t1_name:  db "vfmadd132ss", 0
    t2_name:  db "vfmadd213ss", 0
    t3_name:  db "vfmadd231ss", 0
    t4_name:  db "vfmadd132sd", 0
    t5_name:  db "vfmadd213sd", 0
    t6_name:  db "vfmadd231sd", 0
    t7_name:  db "vfmadd132ps", 0
    t8_name:  db "vfmadd213ps", 0
    t9_name:  db "vfmadd231ps", 0
    t10_name: db "vfmadd132pd", 0
    t11_name: db "vfmadd213pd", 0
    t12_name: db "vfmadd231pd", 0
    t13_name: db "vfmsub132ss", 0
    t14_name: db "vfmsub213ss", 0
    t15_name: db "vfmsub231ss", 0
    t16_name: db "vfmsub132sd", 0
    t17_name: db "vfnmadd132ss", 0
    t18_name: db "vfnmadd213ss", 0
    t19_name: db "vfnmadd231ss", 0
    t20_name: db "vfnmadd132sd", 0
    t21_name: db "vfnmsub132ss", 0
    t22_name: db "vfnmsub213ss", 0
    t23_name: db "vfnmsub231ss", 0
    t24_name: db "vfnmsub132sd", 0
    t25_name: db "vfmadd231ps neg", 0
    t26_name: db "vfmsub231pd", 0
    t27_name: db "vfnmadd231pd", 0
    t28_name: db "vfnmsub231pd", 0
    t29_name: db "vfmadd132ss zero", 0
    t30_name: db "vfmadd213ps large", 0

    align 16
    ; f32: 1.0=3F800000 2.0=40000000 3.0=40400000 4.0=40800000 5.0=40A00000
    ;      6.0=40C00000 7.0=40E00000 8.0=41000000 10.0=41200000
    ;      -1.0=BF800000 -2.0=C0000000
    val_ps_0: dd 0x00000000
    val_ps_1: dd 0x3F800000
    val_ps_2: dd 0x40000000
    val_ps_3: dd 0x40400000
    val_ps_4: dd 0x40800000
    val_ps_5: dd 0x40A00000
    val_ps_neg1: dd 0xBF800000
    val_ps_neg2: dd 0xC0000000
    ; f64: 1.0=3FF.. 2.0=400.. 3.0=4008.. 4.0=4010.. 5.0=4014..
    ;      10.0=4024.. -1.0=BFF.. -2.0=C00..
    val_pd_1: dq 0x3FF0000000000000
    val_pd_2: dq 0x4000000000000000
    val_pd_3: dq 0x4008000000000000
    val_pd_4: dq 0x4010000000000000
    val_pd_5: dq 0x4014000000000000
    val_pd_neg1: dq 0xBFF0000000000000
    val_pd_neg2: dq 0xC000000000000000
    ; Float vectors
    vec_ps_123: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_234: dd 0x40000000, 0x40400000, 0x40800000, 0x40A00000  ; 2,3,4,5
    vec_ps_345: dd 0x40400000, 0x40800000, 0x40A00000, 0x40C00000  ; 3,4,5,6
    vec_ps_neg: dd 0xBF800000, 0xC0000000, 0xC0400000, 0xC0800000  ; -1,-2,-3,-4
    vec_ps_large: dd 0x4E6E6B28, 0x4E6E6B28, 0x4E6E6B28, 0x4E6E6B28  ; 1e9, 1e9, 1e9, 1e9
    ; Double vectors
    vec_pd_12: dq 0x3FF0000000000000, 0x4000000000000000  ; 1, 2
    vec_pd_34: dq 0x4008000000000000, 0x4010000000000000  ; 3, 4
    vec_pd_56: dq 0x4014000000000000, 0x4018000000000000  ; 5, 6

section .text
global _start
_start:
    INIT_TESTS

    ; FMA naming: vfmadd{132,213,231}{ss,sd,ps,pd}
    ; 132: dest = dest*src2 + src1  (operands 1,3,2 -> dest=op1*op3+op2)
    ; 213: dest = src1*dest + src2  (operands 2,1,3 -> dest=op2*op1+op3)
    ; 231: dest = src1*src2 + dest  (operands 2,3,1 -> dest=op2*op3+op1)

    ; --- Test 1: vfmadd132ss: dest = dest*src2 + src1 ---
    TEST_CASE t1_name
    vmovss xmm0, [rel val_ps_2]   ; dest=2
    vmovss xmm1, [rel val_ps_3]   ; src1=3
    vmovss xmm2, [rel val_ps_4]   ; src2=4
    vfmadd132ss xmm0, xmm1, xmm2  ; xmm0 = 2*4 + 3 = 11
    ; 11.0f = 0x41300000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41300000

    ; --- Test 2: vfmadd213ss: dest = src1*dest + src2 ---
    TEST_CASE t2_name
    vmovss xmm0, [rel val_ps_2]   ; dest=2
    vmovss xmm1, [rel val_ps_3]   ; src1=3
    vmovss xmm2, [rel val_ps_4]   ; src2=4
    vfmadd213ss xmm0, xmm1, xmm2  ; xmm0 = 3*2 + 4 = 10
    ; 10.0f = 0x41200000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41200000

    ; --- Test 3: vfmadd231ss: dest = src1*src2 + dest ---
    TEST_CASE t3_name
    vmovss xmm0, [rel val_ps_5]   ; dest=5
    vmovss xmm1, [rel val_ps_2]   ; src1=2
    vmovss xmm2, [rel val_ps_3]   ; src2=3
    vfmadd231ss xmm0, xmm1, xmm2  ; xmm0 = 2*3 + 5 = 11
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41300000

    ; --- Test 4: vfmadd132sd: dest = dest*src2 + src1 ---
    TEST_CASE t4_name
    vmovsd xmm0, [rel val_pd_2]
    vmovsd xmm1, [rel val_pd_3]
    vmovsd xmm2, [rel val_pd_4]
    vfmadd132sd xmm0, xmm1, xmm2  ; 2*4+3=11
    ; 11.0 = 0x4026000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4026000000000000

    ; --- Test 5: vfmadd213sd: dest = src1*dest + src2 ---
    TEST_CASE t5_name
    vmovsd xmm0, [rel val_pd_2]
    vmovsd xmm1, [rel val_pd_3]
    vmovsd xmm2, [rel val_pd_4]
    vfmadd213sd xmm0, xmm1, xmm2  ; 3*2+4=10
    ; 10.0 = 0x4024000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4024000000000000

    ; --- Test 6: vfmadd231sd: dest = src1*src2 + dest ---
    TEST_CASE t6_name
    vmovsd xmm0, [rel val_pd_5]
    vmovsd xmm1, [rel val_pd_2]
    vmovsd xmm2, [rel val_pd_3]
    vfmadd231sd xmm0, xmm1, xmm2  ; 2*3+5=11
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4026000000000000

    ; --- Test 7: vfmadd132ps: dest = dest*src2 + src1 ---
    TEST_CASE t7_name
    vmovaps xmm0, [rel vec_ps_123]  ; dest=[1,2,3,4]
    vmovaps xmm1, [rel vec_ps_234]  ; src1=[2,3,4,5]
    vmovaps xmm2, [rel vec_ps_345]  ; src2=[3,4,5,6]
    vfmadd132ps xmm0, xmm1, xmm2   ; [1*3+2, 2*4+3, 3*5+4, 4*6+5] = [5,11,19,29]
    ; 5.0f = 0x40A00000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40A00000

    ; --- Test 8: vfmadd213ps: dest = src1*dest + src2 ---
    TEST_CASE t8_name
    vmovaps xmm0, [rel vec_ps_123]  ; dest=[1,2,3,4]
    vmovaps xmm1, [rel vec_ps_234]  ; src1=[2,3,4,5]
    vmovaps xmm2, [rel vec_ps_345]  ; src2=[3,4,5,6]
    vfmadd213ps xmm0, xmm1, xmm2   ; [2*1+3, 3*2+4, 4*3+5, 5*4+6] = [5,10,17,26]
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40A00000

    ; --- Test 9: vfmadd231ps: dest = src1*src2 + dest ---
    TEST_CASE t9_name
    vmovaps xmm0, [rel vec_ps_123]  ; dest=[1,2,3,4]
    vmovaps xmm1, [rel vec_ps_234]  ; src1=[2,3,4,5]
    vmovaps xmm2, [rel vec_ps_345]  ; src2=[3,4,5,6]
    vfmadd231ps xmm0, xmm1, xmm2   ; [2*3+1, 3*4+2, 4*5+3, 5*6+4] = [7,14,23,34]
    ; 7.0f = 0x40E00000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40E00000

    ; --- Test 10: vfmadd132pd ---
    TEST_CASE t10_name
    vmovapd xmm0, [rel vec_pd_12]  ; dest=[1,2]
    vmovapd xmm1, [rel vec_pd_34]  ; src1=[3,4]
    vmovapd xmm2, [rel vec_pd_56]  ; src2=[5,6]
    vfmadd132pd xmm0, xmm1, xmm2   ; [1*5+3, 2*6+4] = [8, 16]
    ; 8.0 = 0x4020000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4020000000000000

    ; --- Test 11: vfmadd213pd ---
    TEST_CASE t11_name
    vmovapd xmm0, [rel vec_pd_12]
    vmovapd xmm1, [rel vec_pd_34]
    vmovapd xmm2, [rel vec_pd_56]
    vfmadd213pd xmm0, xmm1, xmm2   ; [3*1+5, 4*2+6] = [8, 14]
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4020000000000000

    ; --- Test 12: vfmadd231pd ---
    TEST_CASE t12_name
    vmovapd xmm0, [rel vec_pd_12]
    vmovapd xmm1, [rel vec_pd_34]
    vmovapd xmm2, [rel vec_pd_56]
    vfmadd231pd xmm0, xmm1, xmm2   ; [3*5+1, 4*6+2] = [16, 26]
    ; 16.0 = 0x4030000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4030000000000000

    ; --- Test 13: vfmsub132ss: dest = dest*src2 - src1 ---
    TEST_CASE t13_name
    vmovss xmm0, [rel val_ps_3]  ; dest=3
    vmovss xmm1, [rel val_ps_1]  ; src1=1
    vmovss xmm2, [rel val_ps_4]  ; src2=4
    vfmsub132ss xmm0, xmm1, xmm2  ; 3*4-1=11
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41300000

    ; --- Test 14: vfmsub213ss: dest = src1*dest - src2 ---
    TEST_CASE t14_name
    vmovss xmm0, [rel val_ps_3]
    vmovss xmm1, [rel val_ps_4]
    vmovss xmm2, [rel val_ps_2]
    vfmsub213ss xmm0, xmm1, xmm2  ; 4*3-2=10
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41200000

    ; --- Test 15: vfmsub231ss: dest = src1*src2 - dest ---
    TEST_CASE t15_name
    vmovss xmm0, [rel val_ps_1]  ; dest=1
    vmovss xmm1, [rel val_ps_3]  ; src1=3
    vmovss xmm2, [rel val_ps_4]  ; src2=4
    vfmsub231ss xmm0, xmm1, xmm2  ; 3*4-1=11
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x41300000

    ; --- Test 16: vfmsub132sd ---
    TEST_CASE t16_name
    vmovsd xmm0, [rel val_pd_3]
    vmovsd xmm1, [rel val_pd_1]
    vmovsd xmm2, [rel val_pd_4]
    vfmsub132sd xmm0, xmm1, xmm2  ; 3*4-1=11
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4026000000000000

    ; --- Test 17: vfnmadd132ss: dest = -(dest*src2) + src1 ---
    TEST_CASE t17_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_5]  ; src1=5 (addend)
    vmovss xmm2, [rel val_ps_3]
    vfnmadd132ss xmm0, xmm1, xmm2  ; -(2*3)+5 = -6+5 = -1
    ; -1.0f = 0xBF800000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xBF800000

    ; --- Test 18: vfnmadd213ss: dest = -(src1*dest) + src2 ---
    TEST_CASE t18_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_3]
    vmovss xmm2, [rel val_ps_5]  ; addend
    vfnmadd213ss xmm0, xmm1, xmm2  ; -(3*2)+5 = -1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xBF800000

    ; --- Test 19: vfnmadd231ss: dest = -(src1*src2) + dest ---
    TEST_CASE t19_name
    vmovss xmm0, [rel val_ps_5]  ; dest=5 (addend)
    vmovss xmm1, [rel val_ps_2]
    vmovss xmm2, [rel val_ps_3]
    vfnmadd231ss xmm0, xmm1, xmm2  ; -(2*3)+5 = -1
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xBF800000

    ; --- Test 20: vfnmadd132sd ---
    TEST_CASE t20_name
    vmovsd xmm0, [rel val_pd_2]
    vmovsd xmm1, [rel val_pd_5]
    vmovsd xmm2, [rel val_pd_3]
    vfnmadd132sd xmm0, xmm1, xmm2  ; -(2*3)+5 = -1
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xBFF0000000000000

    ; --- Test 21: vfnmsub132ss: dest = -(dest*src2) - src1 ---
    TEST_CASE t21_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_1]
    vmovss xmm2, [rel val_ps_3]
    vfnmsub132ss xmm0, xmm1, xmm2  ; -(2*3)-1 = -7
    ; -7.0f = 0xC0E00000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0E00000

    ; --- Test 22: vfnmsub213ss: dest = -(src1*dest) - src2 ---
    TEST_CASE t22_name
    vmovss xmm0, [rel val_ps_2]
    vmovss xmm1, [rel val_ps_3]
    vmovss xmm2, [rel val_ps_1]
    vfnmsub213ss xmm0, xmm1, xmm2  ; -(3*2)-1 = -7
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0E00000

    ; --- Test 23: vfnmsub231ss: dest = -(src1*src2) - dest ---
    TEST_CASE t23_name
    vmovss xmm0, [rel val_ps_1]  ; dest=1
    vmovss xmm1, [rel val_ps_2]
    vmovss xmm2, [rel val_ps_3]
    vfnmsub231ss xmm0, xmm1, xmm2  ; -(2*3)-1 = -7
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xC0E00000

    ; --- Test 24: vfnmsub132sd ---
    TEST_CASE t24_name
    vmovsd xmm0, [rel val_pd_2]
    vmovsd xmm1, [rel val_pd_1]
    vmovsd xmm2, [rel val_pd_3]
    vfnmsub132sd xmm0, xmm1, xmm2  ; -(2*3)-1 = -7
    ; -7.0 = 0xC01C000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xC01C000000000000

    ; --- Test 25: vfmadd231ps with negatives ---
    TEST_CASE t25_name
    vmovaps xmm0, [rel vec_ps_123]  ; dest=[1,2,3,4]
    vmovaps xmm1, [rel vec_ps_neg]  ; src1=[-1,-2,-3,-4]
    vmovaps xmm2, [rel vec_ps_234]  ; src2=[2,3,4,5]
    vfmadd231ps xmm0, xmm1, xmm2   ; [(-1)*2+1, (-2)*3+2, (-3)*4+3, (-4)*5+4]
    ; = [-2+1, -6+2, -12+3, -20+4] = [-1, -4, -9, -16]
    ; -1.0f = 0xBF800000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xBF800000

    ; --- Test 26: vfmsub231pd ---
    TEST_CASE t26_name
    vmovapd xmm0, [rel vec_pd_12]  ; dest=[1,2]
    vmovapd xmm1, [rel vec_pd_34]  ; src1=[3,4]
    vmovapd xmm2, [rel vec_pd_56]  ; src2=[5,6]
    vfmsub231pd xmm0, xmm1, xmm2   ; [3*5-1, 4*6-2] = [14, 22]
    ; 14.0 = 0x402C000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x402C000000000000

    ; --- Test 27: vfnmadd231pd ---
    TEST_CASE t27_name
    vmovapd xmm0, [rel vec_pd_12]  ; dest=[1,2]
    vmovapd xmm1, [rel vec_pd_34]  ; src1=[3,4]
    vmovapd xmm2, [rel vec_pd_56]  ; src2=[5,6]
    vfnmadd231pd xmm0, xmm1, xmm2  ; [-(3*5)+1, -(4*6)+2] = [-14, -22]
    ; -14.0 = 0xC02C000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xC02C000000000000

    ; --- Test 28: vfnmsub231pd ---
    TEST_CASE t28_name
    vmovapd xmm0, [rel vec_pd_12]  ; dest=[1,2]
    vmovapd xmm1, [rel vec_pd_34]  ; src1=[3,4]
    vmovapd xmm2, [rel vec_pd_56]  ; src2=[5,6]
    vfnmsub231pd xmm0, xmm1, xmm2  ; [-(3*5)-1, -(4*6)-2] = [-16, -26]
    ; -16.0 = 0xC030000000000000
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xC030000000000000

    ; --- Test 29: vfmadd132ss with zero ---
    TEST_CASE t29_name
    vmovss xmm0, [rel val_ps_0]   ; dest=0
    vmovss xmm1, [rel val_ps_3]   ; src1=3
    vmovss xmm2, [rel val_ps_4]   ; src2=4
    vfmadd132ss xmm0, xmm1, xmm2  ; 0*4+3=3
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 30: vfmadd213ps with larger values ---
    TEST_CASE t30_name
    vmovaps xmm0, [rel vec_ps_123]  ; dest=[1,2,3,4]
    vmovaps xmm1, [rel vec_ps_345]  ; src1=[3,4,5,6]
    vmovaps xmm2, [rel vec_ps_123]  ; src2=[1,2,3,4]
    vfmadd213ps xmm0, xmm1, xmm2   ; [3*1+1, 4*2+2, 5*3+3, 6*4+4] = [4,10,18,28]
    ; 4.0f = 0x40800000
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x40800000

    END_TESTS
