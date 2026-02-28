; test_avx_mov.asm - Test AVX move/load/store operations (VEX-encoded)
; VMOVAPS/UPS/APD/UPD/DQA/DQU, VMOVSS/SD, VMOVD/Q, VMOVLPS/HPS/LPD/HPD,
; VMOVHLPS/LHPS, VMOVMSKPS/PD, VPMOVMSKB, VMOVDDUP/SLDUP/SHDUP, VLDDQU
%include "test_framework.inc"

section .data
    t1_name:  db "vmovaps load", 0
    t2_name:  db "vmovups load", 0
    t3_name:  db "vmovapd load", 0
    t4_name:  db "vmovdqa load", 0
    t5_name:  db "vmovdqu load", 0
    t6_name:  db "vmovss load", 0
    t7_name:  db "vmovsd load", 0
    t8_name:  db "vmovd load", 0
    t9_name:  db "vmovq load", 0
    t10_name: db "vmovd to gpr", 0
    t11_name: db "vmovq to gpr", 0
    t12_name: db "vmovd from gpr", 0
    t13_name: db "vmovq from gpr", 0
    t14_name: db "vmovhlps", 0
    t15_name: db "vmovlhps", 0
    t16_name: db "vmovlps load", 0
    t17_name: db "vmovhps load", 0
    t18_name: db "vmovmskps", 0
    t19_name: db "vmovmskpd", 0
    t20_name: db "vpmovmskb", 0
    t21_name: db "vmovddup", 0
    t22_name: db "vmovsldup", 0
    t23_name: db "vmovshdup", 0
    t24_name: db "vlddqu", 0
    t25_name: db "vmovss reg-reg merge", 0
    t26_name: db "vmovsd reg-reg merge", 0
    t27_name: db "vmovaps store+load", 0
    t28_name: db "vmovq xmm zeroes upper", 0
    t29_name: db "vmovlpd load", 0
    t30_name: db "vmovhpd load", 0

    align 16
    vec_a: dq 0x0123456789ABCDEF, 0xFEDCBA9876543210
    vec_b: dq 0x1111111111111111, 0x2222222222222222
    vec_c: dq 0xAAAAAAAABBBBBBBB, 0xCCCCCCCCDDDDDDDD
    vec_d: dq 0x3FF0000000000000, 0x4000000000000000  ; 1.0, 2.0 doubles
    val_dw: dd 0xDEADBEEF
    val_qw: dq 0xCAFEBABEDEADC0DE
    ; For movmskps: sign bits in [3,1,0,2] -> bits: elem3=neg,elem2=pos,elem1=neg,elem0=pos
    ; mask = bit3=1,bit2=0,bit1=1,bit0=0 = 0b1010 = 10
    vec_mskps: dd 0x80000000, 0x00000001, 0x80000000, 0x00000001  ; neg,pos,neg,pos -> mask should be 0b0101=5
    ; Actually: elem0=0x80000000(neg)=bit0=1, elem1=0x00000001(pos)=bit1=0, elem2=0x80000000(neg)=bit2=1, elem3=0x00000001(pos)=bit3=0
    ; mask = 0b0101 = 5
    vec_mskpd: dq 0x8000000000000000, 0x0000000000000001  ; elem0=neg, elem1=pos -> mask=0b01=1
    vec_mskb: db 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00
              db 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80
    ; mask = byte15..0 sign bits: 0x80=1, 0x00=0
    ; bytes 0-7: 1,0,1,0,1,0,1,0 = 0x55
    ; bytes 8-15: 0,1,0,1,0,1,0,1 = 0xAA
    ; total = 0xAA55
    ; VMOVSLDUP: [a,b,c,d] -> [a,a,c,c]
    vec_sldup: dd 0x11111111, 0x22222222, 0x33333333, 0x44444444
    ; VMOVSHDUP: [a,b,c,d] -> [b,b,d,d]
    vec_lps_data: dq 0xDEADDEADDEADDEAD  ; 8 bytes for movlps/hps
    vec_hps_data: dq 0xBEEFBEEFBEEFBEEF

    align 16
    store_buf: dq 0, 0, 0, 0

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vmovaps load ---
    TEST_CASE t1_name
    vmovaps xmm0, [rel vec_a]
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; --- Test 2: vmovups load ---
    TEST_CASE t2_name
    vmovups xmm1, [rel vec_b]
    vmovq rax, xmm1
    CHECK_EQ_64 rax, 0x1111111111111111

    ; --- Test 3: vmovapd load ---
    TEST_CASE t3_name
    vmovapd xmm2, [rel vec_d]
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 4: vmovdqa load ---
    TEST_CASE t4_name
    vmovdqa xmm3, [rel vec_c]
    vmovq rax, xmm3
    CHECK_EQ_64 rax, 0xAAAAAAAABBBBBBBB

    ; --- Test 5: vmovdqu load ---
    TEST_CASE t5_name
    vmovdqu xmm4, [rel vec_c]
    vpextrq rax, xmm4, 1
    CHECK_EQ_64 rax, 0xCCCCCCCCDDDDDDDD

    ; --- Test 6: vmovss load (zeroes upper) ---
    TEST_CASE t6_name
    vmovss xmm5, [rel val_dw]
    movd eax, xmm5
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; --- Test 7: vmovsd load (zeroes upper) ---
    TEST_CASE t7_name
    vmovsd xmm6, [rel val_qw]
    vmovq rax, xmm6
    CHECK_EQ_64 rax, 0xCAFEBABEDEADC0DE

    ; --- Test 8: vmovd load ---
    TEST_CASE t8_name
    vmovd xmm7, [rel val_dw]
    movd eax, xmm7
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; --- Test 9: vmovq load ---
    TEST_CASE t9_name
    vmovq xmm8, [rel val_qw]
    vmovq rax, xmm8
    CHECK_EQ_64 rax, 0xCAFEBABEDEADC0DE

    ; --- Test 10: vmovd xmm to gpr ---
    TEST_CASE t10_name
    vmovdqa xmm0, [rel vec_a]
    vmovd eax, xmm0
    CHECK_EQ_32 eax, 0x89ABCDEF

    ; --- Test 11: vmovq xmm to gpr ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_a]
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; --- Test 12: vmovd gpr to xmm ---
    TEST_CASE t12_name
    mov eax, 0x12345678
    vmovd xmm9, eax
    movd eax, xmm9
    CHECK_EQ_32 eax, 0x12345678

    ; --- Test 13: vmovq gpr to xmm ---
    TEST_CASE t13_name
    mov rax, 0xCAFE1234DEAD5678
    vmovq xmm10, rax
    vmovq rax, xmm10
    CHECK_EQ_64 rax, 0xCAFE1234DEAD5678

    ; --- Test 14: vmovhlps ---
    TEST_CASE t14_name
    vmovdqa xmm0, [rel vec_a]   ; [0123456789ABCDEF, FEDCBA9876543210]
    vmovdqa xmm1, [rel vec_b]   ; [1111111111111111, 2222222222222222]
    vmovhlps xmm2, xmm1, xmm0
    ; xmm2 low = xmm0 high, xmm2 high = xmm1 high
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xFEDCBA9876543210

    ; --- Test 15: vmovlhps ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_a]
    vmovdqa xmm1, [rel vec_b]
    vmovlhps xmm2, xmm0, xmm1
    ; xmm2 low = xmm0 low, xmm2 high = xmm1 low
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x1111111111111111

    ; --- Test 16: vmovlps load ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_a]  ; xmm0 = [0123..., FEDC...]
    vmovlps xmm2, xmm0, [rel vec_lps_data]
    ; xmm2 low = mem, xmm2 high = xmm0 high
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xDEADDEADDEADDEAD

    ; --- Test 17: vmovhps load ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_a]
    vmovhps xmm2, xmm0, [rel vec_hps_data]
    ; xmm2 low = xmm0 low, xmm2 high = mem
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0xBEEFBEEFBEEFBEEF

    ; --- Test 18: vmovmskps ---
    TEST_CASE t18_name
    vmovaps xmm0, [rel vec_mskps]
    vmovmskps eax, xmm0
    CHECK_EQ_32 eax, 5

    ; --- Test 19: vmovmskpd ---
    TEST_CASE t19_name
    vmovapd xmm0, [rel vec_mskpd]
    vmovmskpd eax, xmm0
    CHECK_EQ_32 eax, 1

    ; --- Test 20: vpmovmskb ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_mskb]
    vpmovmskb eax, xmm0
    CHECK_EQ_32 eax, 0xAA55

    ; --- Test 21: vmovddup ---
    TEST_CASE t21_name
    vmovq xmm0, [rel val_qw]
    vmovddup xmm2, xmm0
    ; xmm2 = [low64, low64]
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0xCAFEBABEDEADC0DE

    ; --- Test 22: vmovsldup ---
    TEST_CASE t22_name
    vmovaps xmm0, [rel vec_sldup]
    vmovsldup xmm2, xmm0
    ; [a,b,c,d] -> [a,a,c,c] = [0x11111111, 0x11111111, 0x33333333, 0x33333333]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x1111111111111111

    ; --- Test 23: vmovshdup ---
    TEST_CASE t23_name
    vmovaps xmm0, [rel vec_sldup]
    vmovshdup xmm2, xmm0
    ; [a,b,c,d] -> [b,b,d,d] = [0x22222222, 0x22222222, 0x44444444, 0x44444444]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x2222222222222222

    ; --- Test 24: vlddqu ---
    TEST_CASE t24_name
    vlddqu xmm2, [rel vec_a]
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; --- Test 25: vmovss reg-reg merges ---
    TEST_CASE t25_name
    vmovdqa xmm0, [rel vec_a]   ; src1
    vmovdqa xmm1, [rel vec_b]   ; src2
    vmovss xmm2, xmm0, xmm1    ; xmm2 = [xmm1[0], xmm0[1], xmm0[2], xmm0[3]]
    movd eax, xmm2
    ; low dword should be from xmm1 = 0x11111111
    CHECK_EQ_32 eax, 0x11111111

    ; --- Test 26: vmovsd reg-reg merges ---
    TEST_CASE t26_name
    vmovdqa xmm0, [rel vec_a]
    vmovdqa xmm1, [rel vec_b]
    vmovsd xmm2, xmm0, xmm1    ; xmm2 = [xmm1[low64], xmm0[high64]]
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0xFEDCBA9876543210

    ; --- Test 27: vmovaps store then reload ---
    TEST_CASE t27_name
    vmovdqa xmm0, [rel vec_c]
    vmovaps [rel store_buf], xmm0
    vmovaps xmm1, [rel store_buf]
    vmovq rax, xmm1
    CHECK_EQ_64 rax, 0xAAAAAAAABBBBBBBB

    ; --- Test 28: vmovq xmm zeroes upper 64 bits ---
    TEST_CASE t28_name
    vpcmpeqd xmm0, xmm0, xmm0  ; all ones
    vmovq xmm0, [rel val_qw]
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0

    ; --- Test 29: vmovlpd load ---
    TEST_CASE t29_name
    vmovapd xmm0, [rel vec_d]    ; [1.0, 2.0]
    vmovlpd xmm2, xmm0, [rel val_qw]
    ; xmm2 low = mem, high = xmm0 high = 2.0
    vpextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0x4000000000000000

    ; --- Test 30: vmovhpd load ---
    TEST_CASE t30_name
    vmovapd xmm0, [rel vec_d]    ; [1.0, 2.0]
    vmovhpd xmm2, xmm0, [rel val_qw]
    ; xmm2 low = xmm0 low = 1.0, high = mem
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x3FF0000000000000

    END_TESTS
