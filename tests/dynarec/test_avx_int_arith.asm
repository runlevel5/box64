; test_avx_int_arith.asm - Test AVX integer arithmetic (VEX-encoded)
; VPADD/SUB B/W/D/Q, VPMULLW/D, VPMULUDQ, VPMULDQ, VPMULHW/UW,
; VPMADDWD, VPMADDUBSW, VPSADBW, VPAVGB/W, VPABS, VPHADDW/D
%include "test_framework.inc"

section .data
    t1_name:  db "vpaddb", 0
    t2_name:  db "vpaddw", 0
    t3_name:  db "vpaddd", 0
    t4_name:  db "vpaddq", 0
    t5_name:  db "vpsubb", 0
    t6_name:  db "vpsubw", 0
    t7_name:  db "vpsubd", 0
    t8_name:  db "vpsubq", 0
    t9_name:  db "vpmullw", 0
    t10_name: db "vpmulld", 0
    t11_name: db "vpmuludq", 0
    t12_name: db "vpmuldq", 0
    t13_name: db "vpmulhw", 0
    t14_name: db "vpmulhuw", 0
    t15_name: db "vpmaddwd", 0
    t16_name: db "vpmaddubsw", 0
    t17_name: db "vpsadbw", 0
    t18_name: db "vpavgb", 0
    t19_name: db "vpavgw", 0
    t20_name: db "vpabsb", 0
    t21_name: db "vpabsw", 0
    t22_name: db "vpabsd", 0
    t23_name: db "vpaddsb saturate", 0
    t24_name: db "vpaddusb saturate", 0
    t25_name: db "vpsubsb saturate", 0
    t26_name: db "vpsubusb saturate", 0
    t27_name: db "vphaddw", 0
    t28_name: db "vphaddd", 0
    t29_name: db "vphsubw", 0
    t30_name: db "vpmulhrsw", 0
    t31_name: db "vpaddd 3-op", 0
    t32_name: db "vpaddq large", 0
    t33_name: db "vpsubd underflow", 0
    t34_name: db "vpmullw neg", 0
    t35_name: db "vpmaddwd neg", 0

    align 16
    ; Byte vectors
    vec_b_01: db 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
    vec_b_10: db 10,20,30,40,50,60,70,80,90,100,110,120,121,122,123,124
    vec_b_80: db 0x80,0x7F,0xFE,0x01, 0x80,0x7F,0xFE,0x01, 0x80,0x7F,0xFE,0x01, 0x80,0x7F,0xFE,0x01
    vec_b_neg: db 0xFF,0xFE,0xFD,0xFC, 0x80,0x01,0x7F,0x00, 0xFF,0xFE,0xFD,0xFC, 0x80,0x01,0x7F,0x00

    ; Word vectors
    vec_w_1234: dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_w_5678: dw 10, 20, 30, 40, 50, 60, 70, 80
    vec_w_neg:  dw -1, -2, -3, -4, -5, -6, -7, -8
    vec_w_big:  dw 0x7FFF, 0x0001, 0x8000, 0xFFFF, 0x1000, 0x2000, 0x3000, 0x4000

    ; Dword vectors
    vec_d_1234: dd 1, 2, 3, 4
    vec_d_5678: dd 5, 6, 7, 8
    vec_d_neg:  dd -1, -2, -3, -4
    vec_d_big:  dd 0x10000, 0x20000, 0x30000, 0x40000

    ; Qword vectors
    vec_q_12: dq 100, 200
    vec_q_34: dq 300, 400
    vec_q_big: dq 0x100000000, 0x200000000

    ; For VPMULUDQ: takes dwords [0] and [2]
    vec_muludq_a: dd 0x80000000, 0, 0x40000000, 0
    vec_muludq_b: dd 2, 0, 3, 0
    ; For VPMULDQ: signed dword multiply
    vec_muldq_a: dd -2, 0, 3, 0
    vec_muldq_b: dd 3, 0, -4, 0

    ; For VPMULHW: high word of signed 16x16
    vec_mulhw_a: dw 0x4000, 0x4000, 0x8000, 0x7FFF, 0x0100, 0x0200, 0x0400, 0x0800
    vec_mulhw_b: dw 0x0004, 0x0002, 0x0002, 0x7FFF, 0x0100, 0x0100, 0x0100, 0x0100

    ; For VPMADDWD: [a0*b0+a1*b1, a2*b2+a3*b3, ...]
    vec_maddwd_a: dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_maddwd_b: dw 10, 20, 30, 40, 50, 60, 70, 80
    ; results: 1*10+2*20=50, 3*30+4*40=250, 5*50+6*60=610, 7*70+8*80=1130

    ; For VPAVGB
    vec_avgb_a: db 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30
    vec_avgb_b: db 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32

    ; For VPABSB
    vec_absb: db 0x01, 0xFF, 0x80, 0x7F, 0x00, 0xFE, 0x81, 0x02, 0x01, 0xFF, 0x80, 0x7F, 0x00, 0xFE, 0x81, 0x02
    ; abs: 1, 1, 128->0x80(saturate), 127, 0, 2, 127, 2, ... (vpabsb: 0x80->0x80)

    ; For VPADDSB (signed saturate)
    vec_addsb_a: db 0x70, 0x70, 0x80, 0x80, 0x01, 0xFF, 0x00, 0x7F, 0x70, 0x70, 0x80, 0x80, 0x01, 0xFF, 0x00, 0x7F
    vec_addsb_b: db 0x70, 0x10, 0x80, 0xF0, 0x01, 0xFF, 0x00, 0x01, 0x70, 0x10, 0x80, 0xF0, 0x01, 0xFF, 0x00, 0x01
    ; 0x70+0x70=0xE0->clamp to 0x7F, 0x70+0x10=0x80->clamp to 0x7F, 0x80+0x80=-256->clamp to 0x80

    ; For VPADDUSB (unsigned saturate)
    vec_addusb_a: db 0xFF, 0x80, 0x01, 0x00, 0xFE, 0x80, 0x01, 0x00, 0xFF, 0x80, 0x01, 0x00, 0xFE, 0x80, 0x01, 0x00
    vec_addusb_b: db 0x01, 0x80, 0xFE, 0x00, 0x02, 0x81, 0xFE, 0x01, 0x01, 0x80, 0xFE, 0x00, 0x02, 0x81, 0xFE, 0x01

    ; For VPHADDW: horizontal add pairs of 16-bit words
    vec_phaddw_a: dw 1, 2, 3, 4, 5, 6, 7, 8
    vec_phaddw_b: dw 10, 20, 30, 40, 50, 60, 70, 80

    ; For VPHADDD: horizontal add pairs of 32-bit dwords
    vec_phaddd_a: dd 100, 200, 300, 400
    vec_phaddd_b: dd 1000, 2000, 3000, 4000

    ; For VPMULHRSW: multiply, round, shift
    vec_mulhrsw_a: dw 0x4000, 0x2000, 0x7FFF, 0x8000, 0x0100, 0x4000, 0x6000, 0x1000
    vec_mulhrsw_b: dw 0x4000, 0x4000, 0x7FFF, 0x8000, 0x0100, 0x2000, 0x2000, 0x8000

    ; For VPMADDUBSW
    vec_maddubsw_a: db 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
    vec_maddubsw_b: db 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vpaddb ---
    TEST_CASE t1_name
    vmovdqa xmm0, [rel vec_b_01]
    vmovdqa xmm1, [rel vec_b_10]
    vpaddb xmm2, xmm0, xmm1
    ; byte 0: 1+10=11=0x0B, byte 1: 2+20=22=0x16
    ; low64 = bytes 0-7: 11,22,33,44,55,66,77,88
    ; = 0x584D42372C211606  ... let me calculate more carefully
    ; byte0=11, byte1=22, byte2=33, byte3=44, byte4=55, byte5=66, byte6=77, byte7=88
    ; little-endian qword: 0x584D42372C21160B
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x584D42372C21160B

    ; --- Test 2: vpaddw ---
    TEST_CASE t2_name
    vmovdqa xmm0, [rel vec_w_1234]
    vmovdqa xmm1, [rel vec_w_5678]
    vpaddw xmm2, xmm0, xmm1
    ; w0=1+10=11=0x000B, w1=2+20=22=0x0016, w2=3+30=33=0x0021, w3=4+40=44=0x002C
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x002C00210016000B

    ; --- Test 3: vpaddd ---
    TEST_CASE t3_name
    vmovdqa xmm0, [rel vec_d_1234]
    vmovdqa xmm1, [rel vec_d_5678]
    vpaddd xmm2, xmm0, xmm1
    ; d0=1+5=6, d1=2+6=8
    ; low64 = 0x0000000800000006
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000800000006

    ; --- Test 4: vpaddq ---
    TEST_CASE t4_name
    vmovdqa xmm0, [rel vec_q_12]
    vmovdqa xmm1, [rel vec_q_34]
    vpaddq xmm2, xmm0, xmm1
    ; q0=100+300=400
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 400

    ; --- Test 5: vpsubb ---
    TEST_CASE t5_name
    vmovdqa xmm0, [rel vec_b_10]
    vmovdqa xmm1, [rel vec_b_01]
    vpsubb xmm2, xmm0, xmm1
    ; byte0=10-1=9, byte1=20-2=18, byte2=30-3=27, byte3=40-4=36
    ; byte4=50-5=45, byte5=60-6=54, byte6=70-7=63, byte7=80-8=72
    ; = 0x483F362D24190B09  ... wait:
    ; 9=0x09, 18=0x12, 27=0x1B, 36=0x24, 45=0x2D, 54=0x36, 63=0x3F, 72=0x48
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x483F362D241B1209

    ; --- Test 6: vpsubw ---
    TEST_CASE t6_name
    vmovdqa xmm0, [rel vec_w_5678]
    vmovdqa xmm1, [rel vec_w_1234]
    vpsubw xmm2, xmm0, xmm1
    ; w0=10-1=9, w1=20-2=18, w2=30-3=27, w3=40-4=36
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0024001B00120009

    ; --- Test 7: vpsubd ---
    TEST_CASE t7_name
    vmovdqa xmm0, [rel vec_d_5678]
    vmovdqa xmm1, [rel vec_d_1234]
    vpsubd xmm2, xmm0, xmm1
    ; d0=5-1=4, d1=6-2=4
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000400000004

    ; --- Test 8: vpsubq ---
    TEST_CASE t8_name
    vmovdqa xmm0, [rel vec_q_34]
    vmovdqa xmm1, [rel vec_q_12]
    vpsubq xmm2, xmm0, xmm1
    ; q0=300-100=200
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 200

    ; --- Test 9: vpmullw ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_w_1234]
    vmovdqa xmm1, [rel vec_w_5678]
    vpmullw xmm2, xmm0, xmm1
    ; w0=1*10=10, w1=2*20=40, w2=3*30=90, w3=4*40=160
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00A0005A0028000A

    ; --- Test 10: vpmulld ---
    TEST_CASE t10_name
    vmovdqa xmm0, [rel vec_d_1234]
    vmovdqa xmm1, [rel vec_d_5678]
    vpmulld xmm2, xmm0, xmm1
    ; d0=1*5=5, d1=2*6=12
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000C00000005

    ; --- Test 11: vpmuludq ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_muludq_a]
    vmovdqa xmm1, [rel vec_muludq_b]
    vpmuludq xmm2, xmm0, xmm1
    ; q0 = dw0_a * dw0_b = 0x80000000 * 2 = 0x100000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x100000000

    ; --- Test 12: vpmuldq (signed) ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_muldq_a]
    vmovdqa xmm1, [rel vec_muldq_b]
    vpmuldq xmm2, xmm0, xmm1
    ; q0 = (int32)-2 * (int32)3 = -6
    vmovq rax, xmm2
    CHECK_EQ_64 rax, -6

    ; --- Test 13: vpmulhw (signed high) ---
    TEST_CASE t13_name
    vmovdqa xmm0, [rel vec_mulhw_a]
    vmovdqa xmm1, [rel vec_mulhw_b]
    vpmulhw xmm2, xmm0, xmm1
    ; w0: 0x4000 * 0x0004 = 0x10000, high word = 1
    ; w1: 0x4000 * 0x0002 = 0x8000, high word = 0
    vmovq rax, xmm2
    ; w0=1, w1=0, w2=0xFFFF(-1 from 0x8000*2=0xFFFF0000), w3=0x3FFF
    ; low64 = 0x3FFFFFFF00000001
    CHECK_EQ_64 rax, 0x3FFFFFFF00000001

    ; --- Test 14: vpmulhuw (unsigned high) ---
    TEST_CASE t14_name
    vmovdqa xmm0, [rel vec_mulhw_a]
    vmovdqa xmm1, [rel vec_mulhw_b]
    vpmulhuw xmm2, xmm0, xmm1
    ; w0: 0x4000 * 0x0004 = 0x10000, high word = 1
    ; w2: 0x8000 * 0x0002 = 0x10000, high word = 1
    ; w3: 0x7FFF * 0x7FFF = 0x3FFF0001, high = 0x3FFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FFF000100000001

    ; --- Test 15: vpmaddwd ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_maddwd_a]
    vmovdqa xmm1, [rel vec_maddwd_b]
    vpmaddwd xmm2, xmm0, xmm1
    ; d0 = 1*10 + 2*20 = 50
    ; d1 = 3*30 + 4*40 = 250
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x000000FA00000032

    ; --- Test 16: vpmaddubsw ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_maddubsw_a]
    vmovdqa xmm1, [rel vec_maddubsw_b]
    vpmaddubsw xmm2, xmm0, xmm1
    ; w0 = a[0]*b[0] + a[1]*b[1] = 1*1 + 2*1 = 3
    ; w1 = a[2]*b[2] + a[3]*b[3] = 3*2 + 4*2 = 14
    ; w2 = 5*3 + 6*3 = 33
    ; w3 = 7*4 + 8*4 = 60
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x003C0021000E0003

    ; --- Test 17: vpsadbw ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_b_01]
    vmovdqa xmm1, [rel vec_b_10]
    vpsadbw xmm2, xmm0, xmm1
    ; sum of |byte_i - byte_i| for bytes 0-7, result in low word of qword 0
    ; |1-10|+|2-20|+|3-30|+|4-40|+|5-50|+|6-60|+|7-70|+|8-80|
    ; = 9+18+27+36+45+54+63+72 = 324
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 324

    ; --- Test 18: vpavgb ---
    TEST_CASE t18_name
    vmovdqa xmm0, [rel vec_avgb_a]
    vmovdqa xmm1, [rel vec_avgb_b]
    vpavgb xmm2, xmm0, xmm1
    ; avg(0,2)=1, avg(2,4)=3, avg(4,6)=5, avg(6,8)=7, avg(8,10)=9, avg(10,12)=11, avg(12,14)=13, avg(14,16)=15
    ; low64 = 0x0F0D0B0907050301
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0F0D0B0907050301

    ; --- Test 19: vpavgw ---
    TEST_CASE t19_name
    vmovdqa xmm0, [rel vec_w_1234]  ; 1,2,3,4,...
    vmovdqa xmm1, [rel vec_w_5678]  ; 10,20,30,40,...
    vpavgw xmm2, xmm0, xmm1
    ; vpavgw: (a+b+1)>>1
    ; avg(1,10)=(1+10+1)/2=6, avg(2,20)=(2+20+1)/2=11, avg(3,30)=(3+30+1)/2=17, avg(4,40)=(4+40+1)/2=22
    ; low64: w0=6, w1=0x000B, w2=0x0011, w3=0x0016
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00160011000B0006

    ; --- Test 20: vpabsb ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_absb]
    vpabsb xmm2, xmm0
    ; input bytes (positions 0-7): 0x01, 0xFF, 0x80, 0x7F, 0x00, 0xFE, 0x81, 0x02
    ; abs:                          0x01, 0x01, 0x80, 0x7F, 0x00, 0x02, 0x7F, 0x02
    ; little-endian qword: byte0 at LSB
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x027F02007F800101

    ; --- Test 21: vpabsw ---
    TEST_CASE t21_name
    vmovdqa xmm0, [rel vec_w_neg]  ; -1,-2,-3,-4,-5,-6,-7,-8
    vpabsw xmm2, xmm0
    ; abs: 1,2,3,4
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0004000300020001

    ; --- Test 22: vpabsd ---
    TEST_CASE t22_name
    vmovdqa xmm0, [rel vec_d_neg]  ; -1,-2,-3,-4
    vpabsd xmm2, xmm0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000200000001

    ; --- Test 23: vpaddsb saturate ---
    TEST_CASE t23_name
    vmovdqa xmm0, [rel vec_addsb_a]
    vmovdqa xmm1, [rel vec_addsb_b]
    vpaddsb xmm2, xmm0, xmm1
    ; byte0: 0x70+0x70=0xE0 -> clamp 0x7F (pos overflow)
    ; byte1: 0x70+0x10=0x80 -> clamp 0x7F (pos overflow)
    ; byte2: 0x80+0x80=-256 -> clamp 0x80 (neg overflow)
    ; byte3: 0x80+0xF0=-128+-16=-144 -> clamp 0x80
    movd eax, xmm2
    ; byte0=0x7F, byte1=0x7F, byte2=0x80, byte3=0x80
    CHECK_EQ_32 eax, 0x80807F7F

    ; --- Test 24: vpaddusb saturate ---
    TEST_CASE t24_name
    vmovdqa xmm0, [rel vec_addusb_a]
    vmovdqa xmm1, [rel vec_addusb_b]
    vpaddusb xmm2, xmm0, xmm1
    ; byte0: 0xFF+0x01=0x100 -> clamp 0xFF
    ; byte1: 0x80+0x80=0x100 -> clamp 0xFF
    ; byte2: 0x01+0xFE=0xFF -> ok
    ; byte3: 0x00+0x00=0x00 -> ok
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x00FFFFFF

    ; --- Test 25: vpsubsb saturate ---
    TEST_CASE t25_name
    ; 0x7F - 0x80 = 127-(-128) = 255 -> clamp 0x7F
    vmovdqa xmm0, [rel vec_b_80]  ; 0x80,0x7F,0xFE,0x01,...
    ; swap: subtract big from small
    vmovdqa xmm1, [rel vec_b_80]
    vpsubsb xmm2, xmm1, xmm1     ; self-sub = 0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 26: vpsubusb saturate ---
    TEST_CASE t26_name
    ; 0x01 - 0x80 unsigned = clamp to 0
    vmovdqa xmm0, [rel vec_b_01]
    vmovdqa xmm1, [rel vec_b_10]
    vpsubusb xmm2, xmm0, xmm1
    ; all bytes: src < dst, so all clamp to 0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 27: vphaddw ---
    TEST_CASE t27_name
    vmovdqa xmm0, [rel vec_phaddw_a]  ; 1,2,3,4,5,6,7,8
    vmovdqa xmm1, [rel vec_phaddw_b]  ; 10,20,30,40,50,60,70,80
    vphaddw xmm2, xmm0, xmm1
    ; from xmm0: 1+2=3, 3+4=7, 5+6=11, 7+8=15
    ; from xmm1: 10+20=30, 30+40=70, 50+60=110, 70+80=150
    ; result: [3, 7, 11, 15, 30, 70, 110, 150]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x000F000B00070003

    ; --- Test 28: vphaddd ---
    TEST_CASE t28_name
    vmovdqa xmm0, [rel vec_phaddd_a]  ; 100,200,300,400
    vmovdqa xmm1, [rel vec_phaddd_b]  ; 1000,2000,3000,4000
    vphaddd xmm2, xmm0, xmm1
    ; from xmm0: 100+200=300, 300+400=700
    ; from xmm1: 1000+2000=3000, 3000+4000=7000
    ; result: [300, 700, 3000, 7000]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x000002BC0000012C

    ; --- Test 29: vphsubw ---
    TEST_CASE t29_name
    vmovdqa xmm0, [rel vec_phaddw_a]  ; 1,2,3,4,5,6,7,8
    vmovdqa xmm1, [rel vec_phaddw_b]  ; 10,20,30,40,50,60,70,80
    vphsubw xmm2, xmm0, xmm1
    ; from xmm0: 1-2=-1, 3-4=-1, 5-6=-1, 7-8=-1
    ; from xmm1: 10-20=-10, 30-40=-10, 50-60=-10, 70-80=-10
    vmovq rax, xmm2
    ; words: -1,-1,-1,-1 = FFFF FFFF FFFF FFFF
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 30: vpmulhrsw ---
    TEST_CASE t30_name
    vmovdqa xmm0, [rel vec_mulhrsw_a]
    vmovdqa xmm1, [rel vec_mulhrsw_b]
    vpmulhrsw xmm2, xmm0, xmm1
    ; w0: ((0x4000*0x4000) >> 14 + 1) >> 1 = ((0x10000000)>>14+1)>>1 = (0x4000+1)>>1 = 0x2000
    ; Actually formula: (a*b + 0x4000) >> 15
    ; w0: (0x4000*0x4000 + 0x4000) >> 15 = (0x10004000) >> 15 = 0x2000
    movd eax, xmm2
    ; low word = 0x2000
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0x2000

    ; --- Test 31: vpaddd 3-operand ---
    TEST_CASE t31_name
    vmovdqa xmm3, [rel vec_d_1234]
    vmovdqa xmm4, [rel vec_d_5678]
    vpaddd xmm5, xmm3, xmm4
    ; xmm3 should be unchanged
    vmovq rax, xmm3
    CHECK_EQ_64 rax, 0x0000000200000001

    ; --- Test 32: vpaddq large values ---
    TEST_CASE t32_name
    vmovdqa xmm0, [rel vec_q_big]
    vmovdqa xmm1, [rel vec_q_big]
    vpaddq xmm2, xmm0, xmm1
    ; q0 = 0x100000000 + 0x100000000 = 0x200000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x200000000

    ; --- Test 33: vpsubd underflow ---
    TEST_CASE t33_name
    vmovdqa xmm0, [rel vec_d_1234]
    vmovdqa xmm1, [rel vec_d_5678]
    vpsubd xmm2, xmm0, xmm1
    ; d0 = 1-5 = -4 = 0xFFFFFFFC
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xFFFFFFFC

    ; --- Test 34: vpmullw negative ---
    TEST_CASE t34_name
    vmovdqa xmm0, [rel vec_w_neg]   ; -1,-2,-3,-4,...
    vmovdqa xmm1, [rel vec_w_1234]  ; 1,2,3,4,...
    vpmullw xmm2, xmm0, xmm1
    ; w0 = -1*1 = -1 = 0xFFFF
    ; w1 = -2*2 = -4 = 0xFFFC
    vmovq rax, xmm2
    ; words: 0xFFFF, 0xFFFC, 0xFFF7, 0xFFF0
    CHECK_EQ_64 rax, 0xFFF0FFF7FFFCFFFF

    ; --- Test 35: vpmaddwd with negatives ---
    TEST_CASE t35_name
    vmovdqa xmm0, [rel vec_w_neg]   ; -1,-2,-3,-4,...
    vmovdqa xmm1, [rel vec_w_1234]  ; 1,2,3,4,...
    vpmaddwd xmm2, xmm0, xmm1
    ; d0 = (-1)*1 + (-2)*2 = -1 + -4 = -5
    ; d1 = (-3)*3 + (-4)*4 = -9 + -16 = -25
    vmovq rax, xmm2
    ; 0xFFFFFFE7FFFFFFFB  (-25, -5)
    CHECK_EQ_64 rax, 0xFFFFFFE7FFFFFFFB

    END_TESTS
