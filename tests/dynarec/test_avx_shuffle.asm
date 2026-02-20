; test_avx_shuffle.asm - Test AVX shuffle/permute/unpack operations (VEX-encoded)
; VSHUFPS/PD, VPSHUFD, VPSHUFHW, VPSHUFLW, VPSHUFB, VPALIGNR,
; VUNPCKLPS/PD, VUNPCKHPS/PD, VPUNPCKL/H BW/WD/DQ/QDQ, VPACKSSWB/DW, VPACKUSWB/DW
%include "test_framework.inc"

section .data
    t1_name:  db "vshufps imm=0x00", 0
    t2_name:  db "vshufps imm=0x1B", 0
    t3_name:  db "vshufpd imm=0x01", 0
    t4_name:  db "vpshufd imm=0x1B", 0
    t5_name:  db "vpshufd imm=0x00", 0
    t6_name:  db "vpshufhw imm=0x1B", 0
    t7_name:  db "vpshuflw imm=0x1B", 0
    t8_name:  db "vpshufb", 0
    t9_name:  db "vpalignr imm=4", 0
    t10_name: db "vpalignr imm=8", 0
    t11_name: db "vunpcklps", 0
    t12_name: db "vunpckhps", 0
    t13_name: db "vunpcklpd", 0
    t14_name: db "vunpckhpd", 0
    t15_name: db "vpunpcklbw", 0
    t16_name: db "vpunpckhbw", 0
    t17_name: db "vpunpcklwd", 0
    t18_name: db "vpunpckhwd", 0
    t19_name: db "vpunpckldq", 0
    t20_name: db "vpunpckhdq", 0
    t21_name: db "vpunpcklqdq", 0
    t22_name: db "vpunpckhqdq", 0
    t23_name: db "vpacksswb", 0
    t24_name: db "vpackssdw", 0
    t25_name: db "vpackuswb", 0
    t26_name: db "vpackusdw", 0
    t27_name: db "vshufps imm=0xE4", 0
    t28_name: db "vpshufd identity", 0
    t29_name: db "vpalignr imm=0", 0
    t30_name: db "vpshufb zero mask", 0

    align 16
    ; Float vectors for shuffle
    vec_ps_a: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps_b: dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000  ; 5,6,7,8
    ; Double vectors
    vec_pd_a: dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0
    vec_pd_b: dq 0x4008000000000000, 0x4010000000000000  ; 3.0, 4.0
    ; Integer vectors
    vec_i_a: dd 0x11111111, 0x22222222, 0x33333333, 0x44444444
    vec_i_b: dd 0x55555555, 0x66666666, 0x77777777, 0x88888888
    ; Byte vectors for VPSHUFB
    vec_shufb_data: db 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0xFF
    vec_shufb_ctrl: db 0, 1, 2, 3, 0, 1, 2, 3, 0x80, 0x80, 0x80, 0x80, 4, 5, 6, 7
    ; For VPSHUFB zero mask: all 0x80 → all zero
    vec_shufb_zero: db 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
    ; Byte interleave vectors
    vec_b_a: db 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10
    vec_b_b: db 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0
    ; Word vectors for interleave
    vec_w_a: dw 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666, 0x7777, 0x8888
    vec_w_b: dw 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD, 0xEEEE, 0x1234, 0x5678, 0x9ABC
    ; For VPACKSSWB: signed word -> signed byte saturate
    vec_packsswb_a: dw 127, -128, 200, -200, 0, 1, -1, 50
    vec_packsswb_b: dw 100, -100, 300, -300, 10, -10, 0, 127
    ; For VPACKSSDW: signed dword -> signed word saturate
    vec_packssdw_a: dd 32767, -32768, 100000, -100000
    vec_packssdw_b: dd 0, 1, -1, 50000
    ; For VPACKUSWB: unsigned saturate word -> byte
    vec_packuswb_a: dw 0, 255, 256, 0xFFFF, 128, 1, 200, 100
    vec_packuswb_b: dw 50, 300, 0, 127, 255, 1, 0xFFFE, 200
    ; For VPACKUSDW: unsigned saturate dword -> word
    vec_packusdw_a: dd 0, 65535, 100000, 0xFFFFFFFF
    vec_packusdw_b: dd 1, 50000, 0, 32768

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vshufps imm=0x00 (all elem0) ---
    TEST_CASE t1_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vshufps xmm2, xmm0, xmm1, 0x00
    ; result[0]=src1[0]=1, result[1]=src1[0]=1, result[2]=src2[0]=5, result[3]=src2[0]=5
    ; low64: d0=1.0f=0x3F800000, d1=1.0f=0x3F800000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3F8000003F800000

    ; --- Test 2: vshufps imm=0x1B (reverse) ---
    TEST_CASE t2_name
    vmovaps xmm0, [rel vec_ps_a]
    vmovaps xmm1, [rel vec_ps_b]
    vshufps xmm2, xmm0, xmm1, 0x1B
    ; imm=0x1B=0b_00_01_10_11: result[0]=src1[3]=4, result[1]=src1[2]=3, result[2]=src2[1]=6, result[3]=src2[0]=5
    ; low64: d0=4.0f=0x40800000, d1=3.0f=0x40400000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4040000040800000

    ; --- Test 3: vshufpd imm=0x01 ---
    TEST_CASE t3_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vshufpd xmm2, xmm0, xmm1, 0x01
    ; imm bit0=1: result[0]=src1[1]=2.0, imm bit1=0: result[1]=src2[0]=3.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 4: vpshufd imm=0x1B (reverse dwords) ---
    TEST_CASE t4_name
    vmovdqa xmm0, [rel vec_i_a]  ; [0x11111111, 0x22222222, 0x33333333, 0x44444444]
    vpshufd xmm2, xmm0, 0x1B
    ; 0x1B = 0b_00_01_10_11: d0=src[3], d1=src[2], d2=src[1], d3=src[0]
    ; result: [0x44444444, 0x33333333, 0x22222222, 0x11111111]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3333333344444444

    ; --- Test 5: vpshufd imm=0x00 (broadcast d0) ---
    TEST_CASE t5_name
    vmovdqa xmm0, [rel vec_i_a]
    vpshufd xmm2, xmm0, 0x00
    ; all dwords = src[0] = 0x11111111
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x1111111111111111

    ; --- Test 6: vpshufhw imm=0x1B (reverse high words) ---
    TEST_CASE t6_name
    vmovdqa xmm0, [rel vec_w_a]  ; [0x1111,0x2222,0x3333,0x4444, 0x5555,0x6666,0x7777,0x8888]
    vpshufhw xmm2, xmm0, 0x1B
    ; Low 4 words unchanged: 0x1111,0x2222,0x3333,0x4444
    ; High 4 words reversed: 0x8888,0x7777,0x6666,0x5555
    ; low64 (words 0-3) = unchanged
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4444333322221111

    ; --- Test 7: vpshuflw imm=0x1B (reverse low words) ---
    TEST_CASE t7_name
    vmovdqa xmm0, [rel vec_w_a]
    vpshuflw xmm2, xmm0, 0x1B
    ; Low 4 words reversed: 0x4444,0x3333,0x2222,0x1111
    ; High 4 words unchanged
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x1111222233334444

    ; --- Test 8: vpshufb ---
    TEST_CASE t8_name
    vmovdqa xmm0, [rel vec_shufb_data]
    vmovdqa xmm1, [rel vec_shufb_ctrl]
    vpshufb xmm2, xmm0, xmm1
    ; ctrl[0..3]=0,1,2,3 -> data[0..3] = 0x10,0x20,0x30,0x40
    ; ctrl[4..7]=0,1,2,3 -> data[0..3] = 0x10,0x20,0x30,0x40
    ; low64 bytes: 0x10,0x20,0x30,0x40,0x10,0x20,0x30,0x40
    ; = 0x4030201040302010
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4030201040302010

    ; --- Test 9: vpalignr imm=4 ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_i_a]  ; [11111111, 22222222, 33333333, 44444444]
    vmovdqa xmm1, [rel vec_i_b]  ; [55555555, 66666666, 77777777, 88888888]
    vpalignr xmm2, xmm0, xmm1, 4
    ; Concatenate (src1:src2) = [11111111,22222222,33333333,44444444 : 55555555,66666666,77777777,88888888]
    ; Shift right by 4 bytes: take bytes [4..19] from the 32-byte concatenation
    ; src2 bytes 4-15 = 66666666,77777777,88888888 (3 dwords) + src1 byte 0-3 = 11111111
    ; result: [66666666, 77777777, 88888888, 11111111]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x7777777766666666

    ; --- Test 10: vpalignr imm=8 ---
    TEST_CASE t10_name
    vmovdqa xmm0, [rel vec_i_a]
    vmovdqa xmm1, [rel vec_i_b]
    vpalignr xmm2, xmm0, xmm1, 8
    ; Shift right 8 bytes: bytes [8..23]
    ; src2 high 8 bytes + src1 low 8 bytes
    ; = [77777777, 88888888, 11111111, 22222222]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x8888888877777777

    ; --- Test 11: vunpcklps (interleave low floats) ---
    TEST_CASE t11_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vunpcklps xmm2, xmm0, xmm1
    ; result: [src1[0],src2[0],src1[1],src2[1]] = [1,5,2,6]
    ; low64: d0=1.0f=0x3F800000, d1=5.0f=0x40A00000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x40A000003F800000

    ; --- Test 12: vunpckhps (interleave high floats) ---
    TEST_CASE t12_name
    vmovaps xmm0, [rel vec_ps_a]
    vmovaps xmm1, [rel vec_ps_b]
    vunpckhps xmm2, xmm0, xmm1
    ; result: [src1[2],src2[2],src1[3],src2[3]] = [3,7,4,8]
    ; low64: d0=3.0f=0x40400000, d1=7.0f=0x40E00000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x40E0000040400000

    ; --- Test 13: vunpcklpd (interleave low doubles) ---
    TEST_CASE t13_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vmovapd xmm1, [rel vec_pd_b]  ; [3.0, 4.0]
    vunpcklpd xmm2, xmm0, xmm1
    ; result: [src1[0], src2[0]] = [1.0, 3.0]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 14: vunpckhpd (interleave high doubles) ---
    TEST_CASE t14_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovapd xmm1, [rel vec_pd_b]
    vunpckhpd xmm2, xmm0, xmm1
    ; result: [src1[1], src2[1]] = [2.0, 4.0]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 15: vpunpcklbw (interleave low bytes) ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_b_a]  ; 01,02,...,10
    vmovdqa xmm1, [rel vec_b_b]  ; A1,A2,...,B0
    vpunpcklbw xmm2, xmm0, xmm1
    ; Interleave low 8 bytes: [a0,b0,a1,b1,...,a7,b7]
    ; = [01,A1,02,A2,03,A3,04,A4,05,A5,06,A6,07,A7,08,A8]
    ; low64 bytes: 01,A1,02,A2,03,A3,04,A4
    ; = 0xA404A303A202A101
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xA404A303A202A101

    ; --- Test 16: vpunpckhbw (interleave high bytes) ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_b_a]
    vmovdqa xmm1, [rel vec_b_b]
    vpunpckhbw xmm2, xmm0, xmm1
    ; Interleave high 8 bytes: [a8,b8,...,a15,b15]
    ; = [09,A9,0A,AA,0B,AB,0C,AC,0D,AD,0E,AE,0F,AF,10,B0]
    ; low64 bytes: 09,A9,0A,AA,0B,AB,0C,AC
    ; = 0xAC0CAB0BAA0AA909
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xAC0CAB0BAA0AA909

    ; --- Test 17: vpunpcklwd (interleave low words) ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_w_a]  ; 1111,2222,3333,4444,5555,6666,7777,8888
    vmovdqa xmm1, [rel vec_w_b]  ; AAAA,BBBB,CCCC,DDDD,...
    vpunpcklwd xmm2, xmm0, xmm1
    ; Interleave low 4 words: [a0,b0,a1,b1,a2,b2,a3,b3]
    ; = [1111,AAAA,2222,BBBB,3333,CCCC,4444,DDDD]
    ; low64: w0=1111, w1=AAAA, w2=2222, w3=BBBB
    ; = 0xBBBB2222AAAA1111
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xBBBB2222AAAA1111

    ; --- Test 18: vpunpckhwd (interleave high words) ---
    TEST_CASE t18_name
    vmovdqa xmm0, [rel vec_w_a]
    vmovdqa xmm1, [rel vec_w_b]
    vpunpckhwd xmm2, xmm0, xmm1
    ; Interleave high 4 words: [a4,b4,a5,b5,a6,b6,a7,b7]
    ; = [5555,EEEE,6666,1234,7777,5678,8888,9ABC]
    ; low64: w0=5555, w1=EEEE, w2=6666, w3=1234
    ; = 0x12346666EEEE5555
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x12346666EEEE5555

    ; --- Test 19: vpunpckldq (interleave low dwords) ---
    TEST_CASE t19_name
    vmovdqa xmm0, [rel vec_i_a]  ; [11111111,22222222,33333333,44444444]
    vmovdqa xmm1, [rel vec_i_b]  ; [55555555,66666666,77777777,88888888]
    vpunpckldq xmm2, xmm0, xmm1
    ; [a0,b0,a1,b1] = [11111111,55555555,22222222,66666666]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x5555555511111111

    ; --- Test 20: vpunpckhdq (interleave high dwords) ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_i_a]
    vmovdqa xmm1, [rel vec_i_b]
    vpunpckhdq xmm2, xmm0, xmm1
    ; [a2,b2,a3,b3] = [33333333,77777777,44444444,88888888]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x7777777733333333

    ; --- Test 21: vpunpcklqdq ---
    TEST_CASE t21_name
    vmovdqa xmm0, [rel vec_i_a]
    vmovdqa xmm1, [rel vec_i_b]
    vpunpcklqdq xmm2, xmm0, xmm1
    ; [a_low64, b_low64]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x2222222211111111

    ; --- Test 22: vpunpckhqdq ---
    TEST_CASE t22_name
    vmovdqa xmm0, [rel vec_i_a]
    vmovdqa xmm1, [rel vec_i_b]
    vpunpckhqdq xmm2, xmm0, xmm1
    ; [a_high64, b_high64]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4444444433333333

    ; --- Test 23: vpacksswb (signed word -> signed byte, saturate) ---
    TEST_CASE t23_name
    vmovdqa xmm0, [rel vec_packsswb_a]  ; 127,-128,200,-200,0,1,-1,50
    vmovdqa xmm1, [rel vec_packsswb_b]  ; 100,-100,300,-300,10,-10,0,127
    vpacksswb xmm2, xmm0, xmm1
    ; From xmm0: 127->0x7F, -128->0x80, 200->0x7F(clamp), -200->0x80(clamp), 0->0x00, 1->0x01, -1->0xFF, 50->0x32
    ; From xmm1: 100->0x64, -100->0x9C, 300->0x7F, -300->0x80, 10->0x0A, -10->0xF6, 0->0x00, 127->0x7F
    ; result bytes 0-7 from xmm0, 8-15 from xmm1
    ; low64 bytes: 7F,80,7F,80,00,01,FF,32
    ; = 0x32FF01007F807F80  ... wait let me redo
    ; Actually wait: -128 as byte = 0x80, -1 as byte = 0xFF
    ; byte0=0x7F, byte1=0x80, byte2=0x7F, byte3=0x80, byte4=0x00, byte5=0x01, byte6=0xFF, byte7=0x32
    ; little-endian qword: 0x32FF0100807F807F
    ; Hmm, let me be more careful. packed byte 0 = saturate(word0 of xmm0) = saturate(127) = 0x7F
    ; byte1 = saturate(-128) = 0x80, byte2 = saturate(200) = 0x7F, byte3 = saturate(-200) = 0x80
    ; byte4 = saturate(0) = 0x00, byte5 = saturate(1) = 0x01, byte6 = saturate(-1) = 0xFF, byte7 = saturate(50) = 0x32
    ; LE qword: byte7||...||byte0 = 0x32FF01008_07F807F ... 
    ; 0x32, 0xFF, 0x01, 0x00, 0x80, 0x7F, 0x80, 0x7F
    ; = 0x32FF010080 7F807F
    movd eax, xmm2
    ; low 4 bytes: 0x7F, 0x80, 0x7F, 0x80 -> 0x807F807F
    CHECK_EQ_32 eax, 0x807F807F

    ; --- Test 24: vpackssdw (signed dword -> signed word, saturate) ---
    TEST_CASE t24_name
    vmovdqa xmm0, [rel vec_packssdw_a]  ; 32767,-32768,100000,-100000
    vmovdqa xmm1, [rel vec_packssdw_b]  ; 0,1,-1,50000
    vpackssdw xmm2, xmm0, xmm1
    ; From xmm0: 32767->0x7FFF, -32768->0x8000, 100000->0x7FFF(clamp), -100000->0x8000(clamp)
    ; low64: w0=0x7FFF, w1=0x8000, w2=0x7FFF, w3=0x8000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x80007FFF80007FFF

    ; --- Test 25: vpackuswb (unsigned saturate word -> byte) ---
    TEST_CASE t25_name
    vmovdqa xmm0, [rel vec_packuswb_a]  ; 0,255,256,0xFFFF(=-1),128,1,200,100
    vmovdqa xmm1, [rel vec_packuswb_b]
    vpackuswb xmm2, xmm0, xmm1
    ; From xmm0: 0->0, 255->255, 256->255(clamp), -1(0xFFFF)->0(clamp), 128->128, 1->1, 200->200, 100->100
    ; low64 bytes: 0x00, 0xFF, 0xFF, 0x00, 0x80, 0x01, 0xC8, 0x64
    ; LE qword: 0x64C8018000FFFF00
    movd eax, xmm2
    ; low 4 bytes: 0x00, 0xFF, 0xFF, 0x00 -> 0x00FFFF00
    CHECK_EQ_32 eax, 0x00FFFF00

    ; --- Test 26: vpackusdw (unsigned saturate dword -> word) ---
    TEST_CASE t26_name
    vmovdqa xmm0, [rel vec_packusdw_a]  ; 0,65535,100000,0xFFFFFFFF(=-1)
    vmovdqa xmm1, [rel vec_packusdw_b]  ; 1,50000,0,32768
    vpackusdw xmm2, xmm0, xmm1
    ; From xmm0: 0->0, 65535->0xFFFF, 100000->0xFFFF(clamp), -1->0(clamp)
    ; low64: w0=0x0000, w1=0xFFFF, w2=0xFFFF, w3=0x0000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000FFFFFFFF0000

    ; --- Test 27: vshufps imm=0xE4 (identity) ---
    TEST_CASE t27_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps_b]  ; [5,6,7,8]
    vshufps xmm2, xmm0, xmm1, 0xE4
    ; 0xE4 = 0b_11_10_01_00: result[0]=src1[0]=1, [1]=src1[1]=2, [2]=src2[2]=7, [3]=src2[3]=8
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x400000003F800000

    ; --- Test 28: vpshufd identity (imm=0xE4) ---
    TEST_CASE t28_name
    vmovdqa xmm0, [rel vec_i_a]
    vpshufd xmm2, xmm0, 0xE4
    ; identity: [d0,d1,d2,d3]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x2222222211111111

    ; --- Test 29: vpalignr imm=0 (identity of src2) ---
    TEST_CASE t29_name
    vmovdqa xmm0, [rel vec_i_a]
    vmovdqa xmm1, [rel vec_i_b]
    vpalignr xmm2, xmm0, xmm1, 0
    ; shift 0: just src2
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x6666666655555555

    ; --- Test 30: vpshufb zero mask (all 0x80) ---
    TEST_CASE t30_name
    vmovdqa xmm0, [rel vec_shufb_data]
    vmovdqa xmm1, [rel vec_shufb_zero]
    vpshufb xmm2, xmm0, xmm1
    ; All control bytes have high bit set -> all result bytes = 0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    END_TESTS
