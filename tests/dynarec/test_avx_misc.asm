; test_avx_misc.asm - Test miscellaneous AVX operations (VEX-encoded)
; VPERMILPS/PD, VBROADCASTSS/SD, VPBROADCASTB/W/D/Q (AVX2 but box64 supports),
; VPABS, VPSIGN, VZEROUPPER, VPSLLW/D/Q, VPSRLW/D/Q, VPSRAW/D, VPSLLDQ, VPSRLDQ
%include "test_framework.inc"

section .data
    t1_name:  db "vpermilps imm", 0
    t2_name:  db "vpermilps reg", 0
    t3_name:  db "vpermilpd imm", 0
    t4_name:  db "vpermilpd reg", 0
    t5_name:  db "vbroadcastss", 0
    t6_name:  db "vzeroupper", 0
    t7_name:  db "vpsllw imm", 0
    t8_name:  db "vpslld imm", 0
    t9_name:  db "vpsllq imm", 0
    t10_name: db "vpsrlw imm", 0
    t11_name: db "vpsrld imm", 0
    t12_name: db "vpsrlq imm", 0
    t13_name: db "vpsraw imm", 0
    t14_name: db "vpsrad imm", 0
    t15_name: db "vpslldq imm=2", 0
    t16_name: db "vpsrldq imm=2", 0
    t17_name: db "vpsllw xmm", 0
    t18_name: db "vpslld xmm", 0
    t19_name: db "vpsllq xmm", 0
    t20_name: db "vpsrlw xmm", 0
    t21_name: db "vpsignd", 0
    t22_name: db "vpsignw", 0
    t23_name: db "vpsignb", 0
    t24_name: db "vpslldq imm=8", 0
    t25_name: db "vpsrldq imm=8", 0
    t26_name: db "vpermilpd [0,0] q0", 0
    t27_name: db "vpermilpd [0,0] q1", 0
    t28_name: db "vpermilpd [2,2] q0", 0
    t29_name: db "vpermilpd [2,2] q1", 0
    t30_name: db "vpermilpd [0,2] q0", 0
    t31_name: db "vpermilpd [0,2] q1", 0
    t32_name: db "vpermilpd [2,0] q0", 0
    t33_name: db "vpermilpd [2,0] q1", 0
    t34_name: db "vtestps sign ZF", 0
    t35_name: db "vtestps sign CF", 0
    t36_name: db "vtestpd sign ZF", 0

    align 16
    ; Float vectors
    vec_ps_a: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    ; For VPERMILPS reg: control dwords specify source element index (bits 1:0)
    vec_perm_ctrl_ps: dd 3, 2, 1, 0  ; reverse
    ; Double vectors
    vec_pd_a: dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0
    vec_perm_ctrl_pd: dq 1, 0  ; swap (bit 0 of each qword controls selection)
    ; For broadcast
    val_ps_42: dd 0x42280000  ; 42.0f
    ; Integer vectors
    vec_i16: dw 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080
    vec_i32: dd 0x00000001, 0x00000002, 0x00000004, 0x00000008
    vec_i64: dq 0x0000000000000001, 0x0000000000000002
    vec_ab: dq 0x0123456789ABCDEF, 0xFEDCBA9876543210
    ; Shift amount in xmm (low 64 bits)
    vec_shift4: dq 4, 0
    ; VPSIGN test vectors
    vec_sign_data: dd 10, -20, 30, -40
    vec_sign_ctrl: dd 1, -1, 0, 1    ; positive->keep, negative->negate, zero->zero
    vec_signw_data: dw 10, -20, 30, -40, 50, -60, 70, -80
    vec_signw_ctrl: dw 1, -1, 0, 1, -1, 0, 1, -1
    vec_signb_data: db 10, 20, 30, 40, 50, 60, 70, 80, 10, 20, 30, 40, 50, 60, 70, 80
    vec_signb_ctrl: db 1, 0xFF, 0, 1, 0xFF, 0, 1, 0xFF, 1, 0xFF, 0, 1, 0xFF, 0, 1, 0xFF
    ; VPERMILPD variable control vectors (bit 1 of each qword selects source)
    vec_vpermilpd_00: dq 0, 0           ; both bit1=0 → both select src qw0
    vec_vpermilpd_22: dq 2, 2           ; both bit1=1 → both select src qw1
    vec_vpermilpd_02: dq 0, 2           ; qw0: bit1=0→src[0], qw1: bit1=1→src[1] (identity)
    vec_vpermilpd_20: dq 2, 0           ; qw0: bit1=1→src[1], qw1: bit1=0→src[0] (full swap)
    ; VTESTPS/VTESTPD data (positive values with non-zero bits but zero sign bits)
    vec_pos_ps: dd 0x3F800000, 0x3F800000, 0x3F800000, 0x3F800000  ; +1.0f x4 (sign=0)
    vec_neg_ps: dd 0xBF800000, 0xBF800000, 0xBF800000, 0xBF800000  ; -1.0f x4 (sign=1)
    vec_pos_pd: dq 0x3FF0000000000000, 0x3FF0000000000000  ; +1.0 x2 (sign=0)
    vec_neg_pd: dq 0xBFF0000000000000, 0xBFF0000000000000  ; -1.0 x2 (sign=1)

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vpermilps with immediate ---
    TEST_CASE t1_name
    vmovaps xmm0, [rel vec_ps_a]  ; [1,2,3,4]
    vpermilps xmm2, xmm0, 0x1B
    ; 0x1B = 0b_00_01_10_11: d0=src[3]=4, d1=src[2]=3, d2=src[1]=2, d3=src[0]=1
    ; low64: d0=4.0f=0x40800000, d1=3.0f=0x40400000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4040000040800000

    ; --- Test 2: vpermilps with register control ---
    TEST_CASE t2_name
    vmovaps xmm0, [rel vec_ps_a]
    vmovdqa xmm1, [rel vec_perm_ctrl_ps]  ; [3,2,1,0]
    vpermilps xmm2, xmm0, xmm1
    ; Same as reverse: [4,3,2,1]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4040000040800000

    ; --- Test 3: vpermilpd with immediate ---
    TEST_CASE t3_name
    vmovapd xmm0, [rel vec_pd_a]  ; [1.0, 2.0]
    vpermilpd xmm2, xmm0, 0x01
    ; bit0=1: d0=src[1]=2.0, bit1=0: d1=src[0]=1.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 4: vpermilpd with register control ---
    TEST_CASE t4_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_perm_ctrl_pd]  ; [1,0] (bit 1 of each qword)
    vpermilpd xmm2, xmm0, xmm1
    ; ctrl[0] bit1=0 -> src[0]=1.0, ctrl[1] bit1=0 -> src[0]=1.0
    ; Wait: vpermilpd uses bit 1 of each qword control. vec_perm_ctrl_pd = [1, 0]
    ; qword0 ctrl=1: bit1=0 -> select src[0]=1.0
    ; qword1 ctrl=0: bit1=0 -> select src[0]=1.0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 5: vbroadcastss (broadcast single float to all 4 positions) ---
    TEST_CASE t5_name
    vbroadcastss xmm2, [rel val_ps_42]
    ; All 4 dwords = 42.0f = 0x42280000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4228000042280000

    ; --- Test 6: vzeroupper ---
    TEST_CASE t6_name
    ; Set upper 128 bits of ymm0
    vpcmpeqd ymm0, ymm0, ymm0  ; all ones in ymm0
    vzeroupper
    ; Upper 128 bits of all ymm regs should be zeroed
    vextractf128 xmm1, ymm0, 1
    vmovq rax, xmm1
    CHECK_EQ_64 rax, 0

    ; --- Test 7: vpsllw imm (shift words left) ---
    TEST_CASE t7_name
    vmovdqa xmm0, [rel vec_i16]  ; [1,2,4,8,...]
    vpsllw xmm2, xmm0, 4
    ; 1<<4=16, 2<<4=32
    vmovq rax, xmm2
    ; w0=0x0010, w1=0x0020, w2=0x0040, w3=0x0080
    CHECK_EQ_64 rax, 0x0080004000200010

    ; --- Test 8: vpslld imm ---
    TEST_CASE t8_name
    vmovdqa xmm0, [rel vec_i32]  ; [1,2,4,8]
    vpslld xmm2, xmm0, 4
    ; 1<<4=16, 2<<4=32
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000002000000010

    ; --- Test 9: vpsllq imm ---
    TEST_CASE t9_name
    vmovdqa xmm0, [rel vec_i64]  ; [1, 2]
    vpsllq xmm2, xmm0, 4
    ; 1<<4=16, 2<<4=32
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 16

    ; --- Test 10: vpsrlw imm (logical shift words right) ---
    TEST_CASE t10_name
    vmovdqa xmm0, [rel vec_i16]
    vpsrlw xmm2, xmm0, 1
    ; w0: 1>>1=0, w1: 2>>1=1, w2: 4>>1=2, w3: 8>>1=4
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0004000200010000

    ; --- Test 11: vpsrld imm ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_i32]
    vpsrld xmm2, xmm0, 1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000000100000000

    ; --- Test 12: vpsrlq imm ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_i64]
    vpsrlq xmm2, xmm0, 1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 13: vpsraw imm (arithmetic shift words right) ---
    TEST_CASE t13_name
    ; Use a vector with high bit set
    vmovdqa xmm0, [rel vec_i16]  ; w7=0x0080
    ; Let's create a negative word test: shift a word with bit 15 set
    vpcmpeqd xmm0, xmm0, xmm0  ; all 0xFFFF words
    vpsraw xmm2, xmm0, 1
    ; 0xFFFF >> 1 arithmetic = 0xFFFF (sign extended)
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 14: vpsrad imm ---
    TEST_CASE t14_name
    vpcmpeqd xmm0, xmm0, xmm0  ; all 0xFFFFFFFF
    vpsrad xmm2, xmm0, 1
    ; 0xFFFFFFFF >> 1 arithmetic = 0xFFFFFFFF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; --- Test 15: vpslldq imm=2 (byte shift left whole register) ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_ab]  ; [0123456789ABCDEF, FEDCBA9876543210]
    vpslldq xmm2, xmm0, 2
    ; Shift left by 2 bytes: bytes shift up, low 2 bytes become 0
    ; Original: EF CD AB 89 67 45 23 01 | 10 32 54 76 98 BA DC FE (byte0..15)
    ; After shift left 2: 00 00 EF CD AB 89 67 45 23 01 10 32 54 76 98 BA
    ; low64 after shift: bytes 0-7 = 00,00,EF,CD,AB,89,67,45 = 0x456789ABCDEF0000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x456789ABCDEF0000

    ; --- Test 16: vpsrldq imm=2 (byte shift right whole register) ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_ab]
    vpsrldq xmm2, xmm0, 2
    ; Shift right by 2 bytes: bytes shift down, high 2 bytes become 0
    ; After: AB 89 67 45 23 01 10 32 54 76 98 BA DC FE 00 00
    ; low64: bytes 0-7 = AB,89,67,45,23,01,10,32 = 0x32100123456789AB
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x32100123456789AB

    ; --- Test 17: vpsllw with xmm shift count ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_i16]
    vmovdqa xmm1, [rel vec_shift4]  ; shift by 4
    vpsllw xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0080004000200010

    ; --- Test 18: vpslld with xmm shift count ---
    TEST_CASE t18_name
    vmovdqa xmm0, [rel vec_i32]
    vmovdqa xmm1, [rel vec_shift4]
    vpslld xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0000002000000010

    ; --- Test 19: vpsllq with xmm shift count ---
    TEST_CASE t19_name
    vmovdqa xmm0, [rel vec_i64]
    vmovdqa xmm1, [rel vec_shift4]
    vpsllq xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 16

    ; --- Test 20: vpsrlw with xmm shift count ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_i16]
    vmovdqa xmm3, [rel vec_shift4]  ; shift by 4
    vpsrlw xmm2, xmm0, xmm3
    ; w0: 1>>4=0, w1: 2>>4=0, w2: 4>>4=0, w3: 8>>4=0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0

    ; --- Test 21: vpsignd ---
    TEST_CASE t21_name
    vmovdqa xmm0, [rel vec_sign_data]  ; [10,-20,30,-40]
    vmovdqa xmm1, [rel vec_sign_ctrl]  ; [1,-1,0,1]
    vpsignd xmm2, xmm0, xmm1
    ; d0: ctrl=1(pos) -> keep 10, d1: ctrl=-1(neg) -> negate -20 -> 20
    vmovq rax, xmm2
    ; d0=10=0x0000000A, d1=20=0x00000014
    CHECK_EQ_64 rax, 0x000000140000000A

    ; --- Test 22: vpsignw ---
    TEST_CASE t22_name
    vmovdqa xmm0, [rel vec_signw_data]  ; [10,-20,30,-40,50,-60,70,-80]
    vmovdqa xmm1, [rel vec_signw_ctrl]  ; [1,-1,0,1,-1,0,1,-1]
    vpsignw xmm2, xmm0, xmm1
    ; w0: ctrl=1 -> 10, w1: ctrl=-1 -> 20, w2: ctrl=0 -> 0, w3: ctrl=1 -> -40
    ; low64: w0=10=0x000A, w1=20=0x0014, w2=0, w3=-40=0xFFD8
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFFD800000014000A

    ; --- Test 23: vpsignb ---
    TEST_CASE t23_name
    vmovdqa xmm0, [rel vec_signb_data]  ; [10,20,30,40,...]
    vmovdqa xmm1, [rel vec_signb_ctrl]  ; [1,-1,0,1,-1,0,1,-1]
    vpsignb xmm2, xmm0, xmm1
    ; byte0: ctrl=1 -> 10, byte1: ctrl=-1 -> -20=0xEC, byte2: ctrl=0 -> 0, byte3: ctrl=1 -> 40
    ; low32 bytes: 0x0A, 0xEC, 0x00, 0x28
    movd eax, xmm2
    CHECK_EQ_32 eax, 0x2800EC0A

    ; --- Test 24: vpslldq imm=8 ---
    TEST_CASE t24_name
    vmovdqa xmm0, [rel vec_ab]
    vpslldq xmm2, xmm0, 8
    ; Shift left 8 bytes: low 8 bytes become 0, high 8 bytes get old low 8
    ; Original low64=0x0123456789ABCDEF -> goes to high64
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; --- Test 25: vpsrldq imm=8 ---
    TEST_CASE t25_name
    vmovdqa xmm0, [rel vec_ab]
    vpsrldq xmm2, xmm0, 8
    ; Shift right 8 bytes: high 8 bytes become 0, low 8 bytes get old high 8
    ; Original high64=0xFEDCBA9876543210 -> goes to low64
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFEDCBA9876543210

    ; --- Test 26: vpermilpd [0,0] q0 — both ctrl=0, check low qword ---
    TEST_CASE t26_name
    vmovapd xmm0, [rel vec_pd_a]              ; qw0=1.0, qw1=2.0
    vmovdqa xmm1, [rel vec_vpermilpd_00]      ; both bit1=0 → both select src qw0
    vpermilpd xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 27: vpermilpd [0,0] q1 — both ctrl=0, check high qword ---
    TEST_CASE t27_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_00]
    vpermilpd xmm2, xmm0, xmm1
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 28: vpermilpd [2,2] q0 — both ctrl=2 (bit1=1), check low qword ---
    TEST_CASE t28_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_22]      ; both bit1=1 → both select src qw1
    vpermilpd xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 29: vpermilpd [2,2] q1 — both ctrl=2, check high qword ---
    TEST_CASE t29_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_22]
    vpermilpd xmm2, xmm0, xmm1
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 30: vpermilpd [0,2] q0 — identity pattern, check low qword ---
    TEST_CASE t30_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_02]      ; qw0:bit1=0→src[0], qw1:bit1=1→src[1]
    vpermilpd xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 31: vpermilpd [0,2] q1 — identity pattern, check high qword ---
    TEST_CASE t31_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_02]
    vpermilpd xmm2, xmm0, xmm1
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 32: vpermilpd [2,0] q0 — full swap, check low qword ---
    TEST_CASE t32_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_20]      ; qw0:bit1=1→src[1], qw1:bit1=0→src[0]
    vpermilpd xmm2, xmm0, xmm1
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 33: vpermilpd [2,0] q1 — full swap, check high qword ---
    TEST_CASE t33_name
    vmovapd xmm0, [rel vec_pd_a]
    vmovdqa xmm1, [rel vec_vpermilpd_20]
    vpermilpd xmm2, xmm0, xmm1
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x3FF0000000000000

    ; --- Test 34: vtestps sign-bit ZF (positive values: ZF=1 despite non-zero AND) ---
    TEST_CASE t34_name
    vmovaps xmm0, [rel vec_pos_ps]            ; +1.0 x4 (sign bits all 0)
    vmovaps xmm1, [rel vec_pos_ps]            ; +1.0 x4
    vtestps xmm0, xmm1
    ; AND result = 0x3F800000 per dword (non-zero, but sign bits = 0)
    ; So ZF should be 1 (sign bits of AND are all zero)
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    ; --- Test 35: vtestps sign-bit CF (negative values: CF=1) ---
    TEST_CASE t35_name
    vmovaps xmm0, [rel vec_neg_ps]            ; -1.0 x4 (sign bits all 1)
    vmovaps xmm1, [rel vec_neg_ps]            ; -1.0 x4
    vtestps xmm0, xmm1
    ; ZF: AND(-1,-1) sign bits = all 1 → ZF=0
    ; CF: AND(NOT(-1), -1) sign bits: NOT(sign=1)=0, AND(0,1)=0 → all zero → CF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ (ZF|CF), CF

    ; --- Test 36: vtestpd sign-bit ZF (positive doubles: ZF=1) ---
    TEST_CASE t36_name
    vmovapd xmm0, [rel vec_pos_pd]            ; +1.0 x2
    vmovapd xmm1, [rel vec_pos_pd]            ; +1.0 x2
    vtestpd xmm0, xmm1
    ; Sign bits of AND = all 0 → ZF=1
    SAVE_FLAGS
    CHECK_FLAGS_EQ ZF, ZF

    END_TESTS
