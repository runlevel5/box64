; test_avx_extend.asm - Test AVX sign/zero extend and min/max (VEX-encoded)
; VPMOVSXBW/BD/BQ/WD/WQ/DQ, VPMOVZXBW/BD/BQ/WD/WQ/DQ, VPMINUB/SW/SD/UD, VPMAXUB/SW/SD/UD
%include "test_framework.inc"

section .data
    t1_name:  db "vpmovsxbw", 0
    t2_name:  db "vpmovsxbd", 0
    t3_name:  db "vpmovsxbq", 0
    t4_name:  db "vpmovsxwd", 0
    t5_name:  db "vpmovsxwq", 0
    t6_name:  db "vpmovsxdq", 0
    t7_name:  db "vpmovzxbw", 0
    t8_name:  db "vpmovzxbd", 0
    t9_name:  db "vpmovzxbq", 0
    t10_name: db "vpmovzxwd", 0
    t11_name: db "vpmovzxwq", 0
    t12_name: db "vpmovzxdq", 0
    t13_name: db "vpminub", 0
    t14_name: db "vpminsb", 0
    t15_name: db "vpminuw", 0
    t16_name: db "vpminsw", 0
    t17_name: db "vpminud", 0
    t18_name: db "vpminsd", 0
    t19_name: db "vpmaxub", 0
    t20_name: db "vpmaxsb", 0
    t21_name: db "vpmaxuw", 0
    t22_name: db "vpmaxsw", 0
    t23_name: db "vpmaxud", 0
    t24_name: db "vpmaxsd", 0
    t25_name: db "vpmovsxbw neg", 0
    t26_name: db "vpmovzxbw neg byte", 0
    t27_name: db "vpminub zero", 0
    t28_name: db "vpmaxsd neg", 0
    t29_name: db "vpmovsxdq neg", 0
    t30_name: db "vpmovzxdq", 0

    align 16
    ; For sign/zero extend: source bytes
    vec_sx_bytes: db 1, 2, -1, -2, 3, -3, 127, -128, 0, 10, -10, 50, -50, 100, -100, 0
    ; = db 1, 2, 0xFF, 0xFE, 3, 0xFD, 0x7F, 0x80, 0, 10, 0xF6, 50, 0xCE, 100, 0x9C, 0
    vec_zx_bytes: db 0x01, 0x02, 0xFF, 0xFE, 0x80, 0x7F, 0x00, 0x55, 0xAA, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
    ; For word extend
    vec_sx_words: dw 1, -1, 2, -2, 32767, -32768, 100, -100
    vec_zx_words: dw 0x0001, 0xFFFF, 0x8000, 0x7FFF, 0x1234, 0x5678, 0xABCD, 0xEF01
    ; For dword extend
    vec_sx_dwords: dd 1, -1, 2, -2
    vec_zx_dwords: dd 0x00000001, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF
    ; For min/max
    vec_ub_a: db 0, 255, 128, 1, 200, 50, 100, 150, 0, 255, 128, 1, 200, 50, 100, 150
    vec_ub_b: db 255, 0, 127, 2, 100, 200, 150, 100, 255, 0, 127, 2, 100, 200, 150, 100
    vec_sb_a: db 0, 127, -128, 1, -1, 50, -50, 100, 0, 127, -128, 1, -1, 50, -50, 100
    vec_sb_b: db -1, 0, 127, -1, 1, -50, 50, -100, -1, 0, 127, -1, 1, -50, 50, -100
    vec_uw_a: dw 0, 65535, 32768, 1, 200, 50000, 100, 60000
    vec_uw_b: dw 65535, 0, 32767, 2, 100, 60000, 200, 50000
    vec_sw_a: dw 0, 32767, -32768, 1, -1, 100, -100, 200
    vec_sw_b: dw -1, 0, 32767, -1, 1, -100, 100, -200
    vec_ud_a: dd 0, 0xFFFFFFFF, 0x80000000, 1
    vec_ud_b: dd 0xFFFFFFFF, 0, 0x7FFFFFFF, 2
    vec_sd_a: dd 0, 0x7FFFFFFF, 0x80000000, 1
    vec_sd_b: dd -1, 0, 0x7FFFFFFF, -1

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vpmovsxbw (sign-extend 8 bytes -> 8 words) ---
    TEST_CASE t1_name
    vmovdqa xmm0, [rel vec_sx_bytes]
    vpmovsxbw xmm2, xmm0
    ; byte0=1 -> word0=0x0001, byte1=2 -> word1=0x0002
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFE00FF00020001

    ; --- Test 2: vpmovsxbd (sign-extend 4 bytes -> 4 dwords) ---
    TEST_CASE t2_name
    vmovdqa xmm0, [rel vec_sx_bytes]
    vpmovsxbd xmm2, xmm0
    ; byte0=1 -> dw0=1, byte1=2 -> dw1=2
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000200000001

    ; --- Test 3: vpmovsxbq (sign-extend 2 bytes -> 2 qwords) ---
    TEST_CASE t3_name
    vmovdqa xmm0, [rel vec_sx_bytes]
    vpmovsxbq xmm2, xmm0
    ; byte0=1 -> q0=1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 1

    ; --- Test 4: vpmovsxwd (sign-extend 4 words -> 4 dwords) ---
    TEST_CASE t4_name
    vmovdqa xmm0, [rel vec_sx_words]  ; 1,-1,2,-2,...
    vpmovsxwd xmm2, xmm0
    ; w0=1 -> d0=1, w1=-1 -> d1=-1=0xFFFFFFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFF00000001

    ; --- Test 5: vpmovsxwq (sign-extend 2 words -> 2 qwords) ---
    TEST_CASE t5_name
    vmovdqa xmm0, [rel vec_sx_words]  ; 1,-1,...
    vpmovsxwq xmm2, xmm0
    ; w0=1 -> q0=1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 1

    ; --- Test 6: vpmovsxdq (sign-extend 2 dwords -> 2 qwords) ---
    TEST_CASE t6_name
    vmovdqa xmm0, [rel vec_sx_dwords]  ; 1,-1,2,-2
    vpmovsxdq xmm2, xmm0
    ; d0=1 -> q0=1, d1=-1 -> q1=-1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 1

    ; --- Test 7: vpmovzxbw (zero-extend 8 bytes -> 8 words) ---
    TEST_CASE t7_name
    vmovdqa xmm0, [rel vec_zx_bytes]
    vpmovzxbw xmm2, xmm0
    ; byte0=0x01 -> w0=0x0001, byte1=0x02 -> w1=0x0002, byte2=0xFF -> w2=0x00FF, byte3=0xFE -> w3=0x00FE
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00FE00FF00020001

    ; --- Test 8: vpmovzxbd (zero-extend 4 bytes -> 4 dwords) ---
    TEST_CASE t8_name
    vmovdqa xmm0, [rel vec_zx_bytes]
    vpmovzxbd xmm2, xmm0
    ; byte0=0x01 -> d0=1, byte1=0x02 -> d1=2
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000200000001

    ; --- Test 9: vpmovzxbq (zero-extend 2 bytes -> 2 qwords) ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_zx_bytes]
    vpmovzxbq xmm2, xmm0
    ; byte0=0x01 -> q0=1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 1

    ; --- Test 10: vpmovzxwd (zero-extend 4 words -> 4 dwords) ---
    TEST_CASE t10_name
    vmovdqa xmm0, [rel vec_zx_words]  ; 0x0001, 0xFFFF, ...
    vpmovzxwd xmm2, xmm0
    ; w0=0x0001 -> d0=0x00000001, w1=0xFFFF -> d1=0x0000FFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000FFFF00000001

    ; --- Test 11: vpmovzxwq (zero-extend 2 words -> 2 qwords) ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_zx_words]
    vpmovzxwq xmm2, xmm0
    ; w0=0x0001 -> q0=1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 1

    ; --- Test 12: vpmovzxdq (zero-extend 2 dwords -> 2 qwords) ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_zx_dwords]  ; 0x00000001, 0xFFFFFFFF, ...
    vpmovzxdq xmm2, xmm0
    ; d0=1 -> q0=1, d1=0xFFFFFFFF -> q1=0x00000000FFFFFFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 1

    ; --- Test 13: vpminub (unsigned byte min) ---
    TEST_CASE t13_name
    vmovdqa xmm0, [rel vec_ub_a]
    vmovdqa xmm1, [rel vec_ub_b]
    vpminub xmm2, xmm0, xmm1
    ; byte0: min(0,255)=0, byte1: min(255,0)=0, byte2: min(128,127)=127, byte3: min(1,2)=1
    ; low32 bytes: 0x00, 0x00, 0x7F, 0x01
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x017F0000

    ; --- Test 14: vpminsb (signed byte min) ---
    TEST_CASE t14_name
    vmovdqa xmm0, [rel vec_sb_a]
    vmovdqa xmm1, [rel vec_sb_b]
    vpminsb xmm2, xmm0, xmm1
    ; byte0: min(0,-1)=-1=0xFF, byte1: min(127,0)=0, byte2: min(-128,127)=-128=0x80, byte3: min(1,-1)=-1=0xFF
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xFF8000FF

    ; --- Test 15: vpminuw (unsigned word min) ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_uw_a]
    vmovdqa xmm1, [rel vec_uw_b]
    vpminuw xmm2, xmm0, xmm1
    ; w0: min(0,65535)=0, w1: min(65535,0)=0
    vmovq rax, xmm2
    ; w0=0, w1=0, w2=min(32768,32767)=32767=0x7FFF, w3=min(1,2)=1
    CHECK_EQ_64 rax, 0x00017FFF00000000

    ; --- Test 16: vpminsw (signed word min) ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_sw_a]
    vmovdqa xmm1, [rel vec_sw_b]
    vpminsw xmm2, xmm0, xmm1
    ; w0: min(0,-1)=-1=0xFFFF, w1: min(32767,0)=0, w2: min(-32768,32767)=-32768=0x8000, w3: min(1,-1)=-1=0xFFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFF80000000FFFF

    ; --- Test 17: vpminud (unsigned dword min) ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_ud_a]
    vmovdqa xmm1, [rel vec_ud_b]
    vpminud xmm2, xmm0, xmm1
    ; d0: min(0, 0xFFFFFFFF)=0, d1: min(0xFFFFFFFF, 0)=0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 18: vpminsd (signed dword min) ---
    TEST_CASE t18_name
    vmovdqa xmm0, [rel vec_sd_a]
    vmovdqa xmm1, [rel vec_sd_b]
    vpminsd xmm2, xmm0, xmm1
    ; d0: min(0,-1)=-1=0xFFFFFFFF, d1: min(0x7FFFFFFF,0)=0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    ; --- Test 19: vpmaxub (unsigned byte max) ---
    TEST_CASE t19_name
    vmovdqa xmm0, [rel vec_ub_a]
    vmovdqa xmm1, [rel vec_ub_b]
    vpmaxub xmm2, xmm0, xmm1
    ; byte0: max(0,255)=255, byte1: max(255,0)=255, byte2: max(128,127)=128, byte3: max(1,2)=2
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x0280FFFF

    ; --- Test 20: vpmaxsb (signed byte max) ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_sb_a]
    vmovdqa xmm1, [rel vec_sb_b]
    vpmaxsb xmm2, xmm0, xmm1
    ; byte0: max(0,-1)=0, byte1: max(127,0)=127=0x7F, byte2: max(-128,127)=127=0x7F, byte3: max(1,-1)=1
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x017F7F00

    ; --- Test 21: vpmaxuw (unsigned word max) ---
    TEST_CASE t21_name
    vmovdqa xmm0, [rel vec_uw_a]
    vmovdqa xmm1, [rel vec_uw_b]
    vpmaxuw xmm2, xmm0, xmm1
    ; w0: max(0,65535)=65535, w1: max(65535,0)=65535, w2: max(32768,32767)=32768, w3: max(1,2)=2
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00028000FFFFFFFF

    ; --- Test 22: vpmaxsw (signed word max) ---
    TEST_CASE t22_name
    vmovdqa xmm0, [rel vec_sw_a]
    vmovdqa xmm1, [rel vec_sw_b]
    vpmaxsw xmm2, xmm0, xmm1
    ; w0: max(0,-1)=0, w1: max(32767,0)=32767=0x7FFF, w2: max(-32768,32767)=32767, w3: max(1,-1)=1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x00017FFF7FFF0000

    ; --- Test 23: vpmaxud (unsigned dword max) ---
    TEST_CASE t23_name
    vmovdqa xmm0, [rel vec_ud_a]
    vmovdqa xmm1, [rel vec_ud_b]
    vpmaxud xmm2, xmm0, xmm1
    ; d0: max(0, 0xFFFFFFFF)=0xFFFFFFFF, d1: max(0xFFFFFFFF, 0)=0xFFFFFFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 24: vpmaxsd (signed dword max) ---
    TEST_CASE t24_name
    vmovdqa xmm0, [rel vec_sd_a]
    vmovdqa xmm1, [rel vec_sd_b]
    vpmaxsd xmm2, xmm0, xmm1
    ; d0: max(0,-1)=0, d1: max(0x7FFFFFFF,0)=0x7FFFFFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x7FFFFFFF00000000

    ; --- Test 25: vpmovsxbw negative ---
    TEST_CASE t25_name
    vmovdqa xmm0, [rel vec_sx_bytes]  ; byte2=0xFF(-1), byte3=0xFE(-2)
    vpmovsxbw xmm2, xmm0
    ; w2 = sign-extend(-1) = 0xFFFF, w3 = sign-extend(-2) = 0xFFFE
    vpextrq rax, xmm2, 1
    ; high64 has words 4-7: byte4=3->0x0003, byte5=-3(0xFD)->0xFFFD, byte6=127->0x007F, byte7=-128(0x80)->0xFF80
    CHECK_EQ_64 rax, 0xFF80007FFFFD0003

    ; --- Test 26: vpmovzxbw 0xFF byte ---
    TEST_CASE t26_name
    vmovdqa xmm0, [rel vec_zx_bytes]  ; byte2=0xFF
    vpmovzxbw xmm2, xmm0
    ; w2 = zero-extend(0xFF) = 0x00FF
    ; Check word 2 specifically
    vpextrw eax, xmm2, 2
    CHECK_EQ_32 eax, 0x00FF

    ; --- Test 27: vpminub with zero ---
    TEST_CASE t27_name
    vmovdqa xmm0, [rel vec_ub_a]
    vpxor xmm1, xmm1, xmm1  ; all zeros
    vpminub xmm2, xmm0, xmm1
    ; min(anything, 0) = 0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 28: vpmaxsd with negatives ---
    TEST_CASE t28_name
    vmovdqa xmm0, [rel vec_sd_a]  ; d2=0x80000000(-2147483648)
    vmovdqa xmm1, [rel vec_sd_b]  ; d2=0x7FFFFFFF(2147483647)
    vpmaxsd xmm2, xmm0, xmm1
    ; d2: max(-2147483648, 2147483647) = 2147483647
    vpextrd eax, xmm2, 2
    CHECK_EQ_32 eax, 0x7FFFFFFF

    ; --- Test 29: vpmovsxdq negative ---
    TEST_CASE t29_name
    vmovdqa xmm0, [rel vec_sx_dwords]  ; d1=-1
    vpmovsxdq xmm2, xmm0
    ; d1=-1 -> q1=0xFFFFFFFFFFFFFFFF
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 30: vpmovzxdq ---
    TEST_CASE t30_name
    vmovdqa xmm0, [rel vec_zx_dwords]  ; d1=0xFFFFFFFF
    vpmovzxdq xmm2, xmm0
    ; d1=0xFFFFFFFF -> q1=0x00000000FFFFFFFF
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x00000000FFFFFFFF

    END_TESTS
