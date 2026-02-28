; test_avx_insert_extract.asm - Test AVX insert/extract operations (VEX-encoded)
; VPINSRB/W/D/Q, VPEXTRB/W/D/Q, VINSERTPS, VEXTRACTPS, VINSERTF128, VEXTRACTF128
%include "test_framework.inc"

section .data
    t1_name:  db "vpinsrb", 0
    t2_name:  db "vpinsrw", 0
    t3_name:  db "vpinsrd", 0
    t4_name:  db "vpinsrq", 0
    t5_name:  db "vpextrb", 0
    t6_name:  db "vpextrw", 0
    t7_name:  db "vpextrd", 0
    t8_name:  db "vpextrq", 0
    t9_name:  db "vinsertps", 0
    t10_name: db "vextractps", 0
    t11_name: db "vpinsrb pos 5", 0
    t12_name: db "vpinsrw pos 3", 0
    t13_name: db "vpinsrd pos 2", 0
    t14_name: db "vpextrb pos 15", 0
    t15_name: db "vpextrw pos 7", 0
    t16_name: db "vpextrd pos 3", 0
    t17_name: db "vpextrq pos 1", 0
    t18_name: db "vinsertps zero", 0
    t19_name: db "vextractps pos 3", 0
    t20_name: db "vinsertf128", 0
    t21_name: db "vextractf128", 0
    t22_name: db "vpinsrb chain", 0
    t23_name: db "vpinsrw roundtrip", 0
    t24_name: db "vpinsrd from mem", 0
    t25_name: db "vpextrq store", 0

    align 16
    vec_a: dq 0x0123456789ABCDEF, 0xFEDCBA9876543210
    vec_b: dq 0x1111111111111111, 0x2222222222222222
    vec_zero: dq 0, 0
    vec_ps: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000  ; 1,2,3,4
    vec_ps2: dd 0x40A00000, 0x40C00000, 0x40E00000, 0x41000000  ; 5,6,7,8
    val_dw: dd 0xDEADBEEF
    val_qw: dq 0xCAFEBABEDEADC0DE

    align 16
    store_buf: dq 0, 0, 0, 0

section .text
global _start
_start:
    INIT_TESTS

    ; --- Test 1: vpinsrb pos 0 ---
    TEST_CASE t1_name
    vmovdqa xmm0, [rel vec_zero]
    mov eax, 0x42
    vpinsrb xmm2, xmm0, eax, 0
    ; byte 0 = 0x42, rest = 0
    movd eax, xmm2
    and eax, 0xFF
    CHECK_EQ_32 eax, 0x42

    ; --- Test 2: vpinsrw pos 0 ---
    TEST_CASE t2_name
    vmovdqa xmm0, [rel vec_zero]
    mov eax, 0x1234
    vpinsrw xmm2, xmm0, eax, 0
    movd eax, xmm2
    and eax, 0xFFFF
    CHECK_EQ_32 eax, 0x1234

    ; --- Test 3: vpinsrd pos 0 ---
    TEST_CASE t3_name
    vmovdqa xmm0, [rel vec_zero]
    mov eax, 0xDEADBEEF
    vpinsrd xmm2, xmm0, eax, 0
    movd eax, xmm2
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; --- Test 4: vpinsrq pos 0 ---
    TEST_CASE t4_name
    vmovdqa xmm0, [rel vec_zero]
    mov rax, 0xCAFEBABE12345678
    vpinsrq xmm2, xmm0, rax, 0
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xCAFEBABE12345678

    ; --- Test 5: vpextrb pos 0 ---
    TEST_CASE t5_name
    vmovdqa xmm0, [rel vec_a]  ; low byte = 0xEF
    vpextrb eax, xmm0, 0
    CHECK_EQ_32 eax, 0xEF

    ; --- Test 6: vpextrw pos 0 ---
    TEST_CASE t6_name
    vmovdqa xmm0, [rel vec_a]  ; low word = 0xCDEF
    vpextrw eax, xmm0, 0
    CHECK_EQ_32 eax, 0xCDEF

    ; --- Test 7: vpextrd pos 0 ---
    TEST_CASE t7_name
    vmovdqa xmm0, [rel vec_a]  ; low dword = 0x89ABCDEF
    vpextrd eax, xmm0, 0
    CHECK_EQ_32 eax, 0x89ABCDEF

    ; --- Test 8: vpextrq pos 0 ---
    TEST_CASE t8_name
    vmovdqa xmm0, [rel vec_a]
    vpextrq rax, xmm0, 0
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; --- Test 9: vinsertps (insert float from src2 elem to dest elem) ---
    TEST_CASE t9_name
    vmovaps xmm0, [rel vec_ps]   ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps2]  ; [5,6,7,8]
    ; imm = 0x10: src2[0]=5 -> dest[1], no zero
    ; imm bits: [7:6]=count_s(src2 select), [5:4]=count_d(dest position), [3:0]=zmask
    ; 0x10 = 0b_00_01_0000: src2[0]->dest[1], no zeroing
    vinsertps xmm2, xmm0, xmm1, 0x10
    ; result: [1, 5, 3, 4]
    ; d1 = 5.0f = 0x40A00000
    ; Extract dword 1
    vpextrd eax, xmm2, 1
    CHECK_EQ_32 eax, 0x40A00000

    ; --- Test 10: vextractps ---
    TEST_CASE t10_name
    vmovaps xmm0, [rel vec_ps]  ; [1,2,3,4]
    vextractps eax, xmm0, 2
    ; elem 2 = 3.0f = 0x40400000
    CHECK_EQ_32 eax, 0x40400000

    ; --- Test 11: vpinsrb pos 5 ---
    TEST_CASE t11_name
    vmovdqa xmm0, [rel vec_a]
    mov eax, 0xFF
    vpinsrb xmm2, xmm0, eax, 5
    ; byte 5 replaced with 0xFF
    ; Original bytes 4-7 of vec_a: 0x45, 0x67, 0x89, 0xAB... wait
    ; vec_a low64 = 0x0123456789ABCDEF
    ; byte0=0xEF, byte1=0xCD, byte2=0xAB, byte3=0x89, byte4=0x67, byte5=0x45, byte6=0x23, byte7=0x01
    ; After insert byte5=0xFF: byte5=0xFF
    ; low64 = 0x01_23_FF_67_89_AB_CD_EF = 0x0123FF6789ABCDEF
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0x0123FF6789ABCDEF

    ; --- Test 12: vpinsrw pos 3 ---
    TEST_CASE t12_name
    vmovdqa xmm0, [rel vec_zero]
    mov eax, 0xBEEF
    vpinsrw xmm2, xmm0, eax, 3
    ; word 3 = 0xBEEF, rest 0
    ; low64: w0=0, w1=0, w2=0, w3=0xBEEF = 0xBEEF000000000000
    vmovq rax, xmm2
    CHECK_EQ_64 rax, 0xBEEF000000000000

    ; --- Test 13: vpinsrd pos 2 ---
    TEST_CASE t13_name
    vmovdqa xmm0, [rel vec_zero]
    mov eax, 0xCAFECAFE
    vpinsrd xmm2, xmm0, eax, 2
    ; dword 2 = 0xCAFECAFE (in high 64 bits)
    vpextrq rax, xmm2, 1
    ; high64: d2=0xCAFECAFE, d3=0
    CHECK_EQ_64 rax, 0x00000000CAFECAFE

    ; --- Test 14: vpextrb pos 15 ---
    TEST_CASE t14_name
    vmovdqa xmm0, [rel vec_a]
    vpextrb eax, xmm0, 15
    ; vec_a high64 = 0xFEDCBA9876543210
    ; byte8=0x10, byte9=0x32, ..., byte15=0xFE
    CHECK_EQ_32 eax, 0xFE

    ; --- Test 15: vpextrw pos 7 ---
    TEST_CASE t15_name
    vmovdqa xmm0, [rel vec_a]
    vpextrw eax, xmm0, 7
    ; word7 = top word of high64 = 0xFEDC
    CHECK_EQ_32 eax, 0xFEDC

    ; --- Test 16: vpextrd pos 3 ---
    TEST_CASE t16_name
    vmovdqa xmm0, [rel vec_a]
    vpextrd eax, xmm0, 3
    ; dword3 = top dword of high64 = 0xFEDCBA98
    CHECK_EQ_32 eax, 0xFEDCBA98

    ; --- Test 17: vpextrq pos 1 ---
    TEST_CASE t17_name
    vmovdqa xmm0, [rel vec_a]
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xFEDCBA9876543210

    ; --- Test 18: vinsertps with zero mask ---
    TEST_CASE t18_name
    vmovaps xmm0, [rel vec_ps]  ; [1,2,3,4]
    vmovaps xmm1, [rel vec_ps2] ; [5,6,7,8]
    ; imm = 0x07: src2[0]->dest[0], zero mask=0b0111 (zero elems 0,1,2)
    ; Wait: zmask bits [3:0] zero the corresponding dest elements
    ; 0x07 = 0b_00_00_0111: src2[0]->dest[0], zmask zeroes elem 0,1,2
    ; So elem0=0 (zeroed by zmask even though inserted), elem1=0, elem2=0, elem3=4
    vinsertps xmm2, xmm0, xmm1, 0x07
    ; Check elem3 = 4.0f = 0x40800000
    vpextrd eax, xmm2, 3
    CHECK_EQ_32 eax, 0x40800000

    ; --- Test 19: vextractps pos 3 ---
    TEST_CASE t19_name
    vmovaps xmm0, [rel vec_ps]  ; [1,2,3,4]
    vextractps eax, xmm0, 3
    ; elem3 = 4.0f = 0x40800000
    CHECK_EQ_32 eax, 0x40800000

    ; --- Test 20: vinsertf128 (insert 128-bit into upper ymm) ---
    TEST_CASE t20_name
    vmovdqa xmm0, [rel vec_a]
    vmovdqa xmm1, [rel vec_b]
    ; Insert xmm1 into ymm0 upper 128 bits
    vinsertf128 ymm2, ymm0, xmm1, 1
    ; upper 128 = vec_b, lower 128 = vec_a
    vextractf128 xmm3, ymm2, 1
    vmovq rax, xmm3
    CHECK_EQ_64 rax, 0x1111111111111111

    ; --- Test 21: vextractf128 ---
    TEST_CASE t21_name
    ; First set up a known ymm value
    vmovdqa xmm0, [rel vec_a]
    vmovdqa xmm1, [rel vec_b]
    vinsertf128 ymm2, ymm0, xmm1, 1
    ; Extract lower 128
    vextractf128 xmm3, ymm2, 0
    vmovq rax, xmm3
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    ; --- Test 22: vpinsrb chain (build a vector byte by byte) ---
    TEST_CASE t22_name
    vpxor xmm0, xmm0, xmm0
    mov eax, 0xAA
    vpinsrb xmm0, xmm0, eax, 0
    mov eax, 0xBB
    vpinsrb xmm0, xmm0, eax, 1
    mov eax, 0xCC
    vpinsrb xmm0, xmm0, eax, 2
    mov eax, 0xDD
    vpinsrb xmm0, xmm0, eax, 3
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xDDCCBBAA

    ; --- Test 23: vpinsrw + vpextrw roundtrip ---
    TEST_CASE t23_name
    vpxor xmm0, xmm0, xmm0
    mov eax, 0x5678
    vpinsrw xmm0, xmm0, eax, 4
    vpextrw eax, xmm0, 4
    CHECK_EQ_32 eax, 0x5678

    ; --- Test 24: vpinsrd from memory ---
    TEST_CASE t24_name
    vpxor xmm0, xmm0, xmm0
    vpinsrd xmm2, xmm0, [rel val_dw], 1
    vpextrd eax, xmm2, 1
    CHECK_EQ_32 eax, 0xDEADBEEF

    ; --- Test 25: vpextrq store to memory ---
    TEST_CASE t25_name
    vmovdqa xmm0, [rel vec_a]
    vpextrq [rel store_buf], xmm0, 0
    mov rax, [rel store_buf]
    CHECK_EQ_64 rax, 0x0123456789ABCDEF

    END_TESTS
