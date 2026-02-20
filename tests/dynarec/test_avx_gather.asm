; test_avx_gather.asm - Test AVX2 gather instructions
; VPGATHERDD, VPGATHERDQ, VPGATHERQD, VPGATHERQQ
; VGATHERDPS, VGATHERDPD, VGATHERQPS, VGATHERQPD
%include "test_framework.inc"

section .data
    ; Test names
    t1_name:  db "vpgatherdd xmm", 0
    t2_name:  db "vpgatherdd ymm", 0
    t3_name:  db "vpgatherdq xmm", 0
    t4_name:  db "vpgatherdq ymm", 0
    t5_name:  db "vpgatherqd xmm", 0
    t6_name:  db "vpgatherqd ymm", 0
    t7_name:  db "vpgatherqq xmm", 0
    t8_name:  db "vpgatherqq ymm", 0
    t9_name:  db "vgatherdps xmm", 0
    t10_name: db "vgatherdps ymm", 0
    t11_name: db "vgatherdpd xmm", 0
    t12_name: db "vgatherdpd ymm", 0
    t13_name: db "vgatherqps xmm", 0
    t14_name: db "vgatherqps ymm", 0
    t15_name: db "vgatherqpd xmm", 0
    t16_name: db "vgatherqpd ymm", 0
    t17_name: db "mask partial", 0
    t18_name: db "mask all zero", 0
    t19_name: db "scale x4", 0
    t20_name: db "neg index", 0
    t21_name: db "disp gather", 0
    t22_name: db "mask zeroed", 0

    align 16
    ; Source data table - 8 dwords (also usable as 4 qwords or 8 floats or 4 doubles)
    ; Dword view:  [0]=0x10, [1]=0x20, [2]=0x30, [3]=0x40, [4]=0x50, [5]=0x60, [6]=0x70, [7]=0x80
    gather_table: dd 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80
    ; Extended table for negative index tests (put before main table)
    ; We'll use gather_table as base and negative byte offsets to reach earlier data

    ; Qword view of gather_table:
    ; [0] = 0x0000002000000010, [1] = 0x0000004000000030
    ; [2] = 0x0000006000000050, [3] = 0x0000008000000070

    ; Float view (just dword patterns, not real floats - we check bit patterns)
    ; Same as dword view

    align 16
    ; Dword indices for VPGATHERDD 128-bit: gather elements [3,1,2,0]
    idx_dd_128: dd 3, 1, 2, 0
    ; Dword indices for VPGATHERDD 256-bit: gather [7,5,3,1,6,4,2,0]
    idx_dd_256: dd 7, 5, 3, 1, 6, 4, 2, 0
    ; Dword indices for VPGATHERDQ 128-bit: 2 qword gathers using dword indices [2,0]
    ; (only first 2 dword indices used when rex.w=1, vex.l=0)
    idx_dq_128: dd 2, 0, 99, 99
    ; Dword indices for VPGATHERDQ 256-bit: 4 qword gathers using dword indices [3,1,2,0]
    idx_dq_256: dd 3, 1, 2, 0
    ; Qword indices for VPGATHERQD 128-bit: 2 qwords used as indices [2,0]
    idx_qd_128: dq 2, 0
    ; Qword indices for VPGATHERQD 256-bit: 4 qwords used as indices [3,1,2,0]
    idx_qd_256: dq 3, 1, 2, 0
    ; Qword indices for VPGATHERQQ 128-bit: [1,0]
    idx_qq_128: dq 1, 0
    ; Qword indices for VPGATHERQQ 256-bit: [3,1,2,0]
    idx_qq_256: dq 3, 1, 2, 0

    ; All-ones mask (every sign bit set)
    align 16
    mask_all: dd 0x80000000, 0x80000000, 0x80000000, 0x80000000
              dd 0x80000000, 0x80000000, 0x80000000, 0x80000000
    ; Partial mask: only elements 0 and 2 enabled
    mask_partial: dd 0x80000000, 0x00000000, 0x80000000, 0x00000000
                  dd 0x00000000, 0x00000000, 0x00000000, 0x00000000
    ; All-zero mask
    mask_zero: dd 0, 0, 0, 0, 0, 0, 0, 0
    ; All-ones qword mask
    mask_all_q: dq 0x8000000000000000, 0x8000000000000000
                dq 0x8000000000000000, 0x8000000000000000

    ; Pre-filled destination (to verify partial mask leaves untouched)
    prefill_val: dd 0xDEAD0001, 0xDEAD0002, 0xDEAD0003, 0xDEAD0004

    ; Negative index test: index -1 means go back 1 element from base
    idx_neg: dd 0xFFFFFFFF, 0, 0, 0  ; index[0] = -1 (signed)

    ; Extended table for negative index tests
    ; Place some data before gather_table
    align 16
    neg_prefix: dd 0xAA, 0xBB, 0xCC, 0xDD
    neg_table:  dd 0x10, 0x20, 0x30, 0x40
    idx_neg_test: dd -1, 0, 1, 2    ; -1 reaches neg_prefix[3]=0xDD

    ; Scale test indices: scale=4 means index*4 bytes
    idx_scale: dd 0, 1, 2, 3        ; with scale=4 and dword table, stride=4 bytes * 4 = 16 bytes

    ; Displacement test
    idx_disp: dd 0, 1, 2, 3

section .text
global _start
_start:
    INIT_TESTS

    ; ==========================================
    ; Test 1: VPGATHERDD xmm (128-bit, 4 dword elements with dword indices)
    ; ==========================================
    TEST_CASE t1_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dd_128]      ; indices: [3,1,2,0]
    vmovdqa xmm2, [rel mask_all]         ; mask: all enabled
    vpxor xmm0, xmm0, xmm0              ; clear dest
    ; VPGATHERDD xmm0, [rbx + xmm1*4], xmm2
    ; scale=2 (x4) — each index * 4 = byte offset for dword table
    vpgatherdd xmm0, [rbx + xmm1*4], xmm2
    ; Expected: element[0]=table[3]=0x40, [1]=table[1]=0x20, [2]=table[2]=0x30, [3]=table[0]=0x10
    ; Low 64: dword[0]=0x40, dword[1]=0x20 -> qword = 0x0000002000000040
    ; High64: dword[2]=0x30, dword[3]=0x10 -> qword = 0x0000001000000030
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000002000000040
    TEST_CASE t1_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000001000000030

    ; ==========================================
    ; Test 2: VPGATHERDD ymm (256-bit, 8 dword elements with dword indices)
    ; ==========================================
    TEST_CASE t2_name
    lea rbx, [rel gather_table]
    vmovdqa ymm1, [rel idx_dd_256]       ; indices: [7,5,3,1,6,4,2,0]
    vmovdqa ymm2, [rel mask_all]         ; mask: all enabled
    vpxor ymm0, ymm0, ymm0
    vpgatherdd ymm0, [rbx + ymm1*4], ymm2
    ; element[0]=table[7]=0x80, [1]=table[5]=0x60, [2]=table[3]=0x40, [3]=table[1]=0x20
    ; element[4]=table[6]=0x70, [5]=table[4]=0x50, [6]=table[2]=0x30, [7]=table[0]=0x10
    ; Low 64: [0]=0x80, [1]=0x60 -> 0x0000006000000080
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000006000000080
    TEST_CASE t2_name
    pextrq rax, xmm0, 1
    ; High64 of lower 128: [2]=0x40, [3]=0x20 -> 0x0000002000000040
    CHECK_EQ_64 rax, 0x0000002000000040
    ; Also check upper 128
    TEST_CASE t2_name
    vextracti128 xmm3, ymm0, 1
    movq rax, xmm3
    ; Upper low 64: [4]=0x70, [5]=0x50 -> 0x0000005000000070
    CHECK_EQ_64 rax, 0x0000005000000070
    TEST_CASE t2_name
    pextrq rax, xmm3, 1
    ; Upper high 64: [6]=0x30, [7]=0x10 -> 0x0000001000000030
    CHECK_EQ_64 rax, 0x0000001000000030

    ; ==========================================
    ; Test 3: VPGATHERDQ xmm (128-bit, 2 qword elements with dword indices)
    ; ==========================================
    TEST_CASE t3_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dq_128]       ; dword indices: [2,0,99,99] — only first 2 used
    vmovdqa xmm2, [rel mask_all_q]       ; qword mask
    vpxor xmm0, xmm0, xmm0
    vpgatherdq xmm0, [rbx + xmm1*8], xmm2
    ; Qword index 2: table qword[2] at offset 16 = 0x0000006000000050
    ; Qword index 0: table qword[0] at offset 0  = 0x0000002000000010
    ; element[0] = qword at index 2 = 0x0000006000000050
    ; element[1] = qword at index 0 = 0x0000002000000010
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000006000000050
    TEST_CASE t3_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000002000000010

    ; ==========================================
    ; Test 4: VPGATHERDQ ymm (256-bit, 4 qword elements with dword indices)
    ; ==========================================
    TEST_CASE t4_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dq_256]       ; dword indices: [3,1,2,0] (only 4 dwords needed)
    vmovdqa ymm2, [rel mask_all_q]       ; all qword masks set
    vpxor ymm0, ymm0, ymm0
    vpgatherdq ymm0, [rbx + xmm1*8], ymm2
    ; element[0] = table qword[3] at offset 24 = 0x0000008000000070
    ; element[1] = table qword[1] at offset 8  = 0x0000004000000030
    ; element[2] = table qword[2] at offset 16 = 0x0000006000000050
    ; element[3] = table qword[0] at offset 0  = 0x0000002000000010
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000008000000070
    TEST_CASE t4_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000004000000030
    TEST_CASE t4_name
    vextracti128 xmm3, ymm0, 1
    movq rax, xmm3
    CHECK_EQ_64 rax, 0x0000006000000050
    TEST_CASE t4_name
    pextrq rax, xmm3, 1
    CHECK_EQ_64 rax, 0x0000002000000010

    ; ==========================================
    ; Test 5: VPGATHERQD xmm (128-bit, 2 dword elements with qword indices)
    ; ==========================================
    TEST_CASE t5_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_qd_128]       ; qword indices: [2,0]
    vmovdqa xmm2, [rel mask_all]          ; dword mask
    vpxor xmm0, xmm0, xmm0
    vpgatherqd xmm0, [rbx + xmm1*4], xmm2
    ; element[0] = table[2] = 0x30
    ; element[1] = table[0] = 0x10
    ; elements[2,3] zeroed
    ; Low 64: [0]=0x30, [1]=0x10 -> 0x0000001000000030
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000001000000030
    ; High 64 should be zero
    TEST_CASE t5_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0

    ; ==========================================
    ; Test 6: VPGATHERQD ymm (256-bit, 4 dword elements with qword indices)
    ; ==========================================
    TEST_CASE t6_name
    lea rbx, [rel gather_table]
    vmovdqa ymm1, [rel idx_qd_256]       ; qword indices: [3,1,2,0]
    vmovdqa xmm2, [rel mask_all]          ; dword mask (only lower 128 needed for 4 dwords)
    vpxor xmm0, xmm0, xmm0
    vpgatherqd xmm0, [rbx + ymm1*4], xmm2
    ; element[0] = table[3] = 0x40
    ; element[1] = table[1] = 0x20
    ; element[2] = table[2] = 0x30
    ; element[3] = table[0] = 0x10
    ; Low 64: [0]=0x40, [1]=0x20 -> 0x0000002000000040
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000002000000040
    TEST_CASE t6_name
    pextrq rax, xmm0, 1
    ; High 64: [2]=0x30, [3]=0x10 -> 0x0000001000000030
    CHECK_EQ_64 rax, 0x0000001000000030

    ; ==========================================
    ; Test 7: VPGATHERQQ xmm (128-bit, 2 qword elements with qword indices)
    ; ==========================================
    TEST_CASE t7_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_qq_128]       ; qword indices: [1,0]
    vmovdqa xmm2, [rel mask_all_q]       ; qword mask
    vpxor xmm0, xmm0, xmm0
    vpgatherqq xmm0, [rbx + xmm1*8], xmm2
    ; element[0] = table qword[1] = 0x0000004000000030
    ; element[1] = table qword[0] = 0x0000002000000010
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000004000000030
    TEST_CASE t7_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000002000000010

    ; ==========================================
    ; Test 8: VPGATHERQQ ymm (256-bit, 4 qword elements with qword indices)
    ; ==========================================
    TEST_CASE t8_name
    lea rbx, [rel gather_table]
    vmovdqa ymm1, [rel idx_qq_256]       ; qword indices: [3,1,2,0]
    vmovdqa ymm2, [rel mask_all_q]       ; all qword masks set
    vpxor ymm0, ymm0, ymm0
    vpgatherqq ymm0, [rbx + ymm1*8], ymm2
    ; element[0] = table qword[3] = 0x0000008000000070
    ; element[1] = table qword[1] = 0x0000004000000030
    ; element[2] = table qword[2] = 0x0000006000000050
    ; element[3] = table qword[0] = 0x0000002000000010
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000008000000070
    TEST_CASE t8_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000004000000030
    TEST_CASE t8_name
    vextracti128 xmm3, ymm0, 1
    movq rax, xmm3
    CHECK_EQ_64 rax, 0x0000006000000050
    TEST_CASE t8_name
    pextrq rax, xmm3, 1
    CHECK_EQ_64 rax, 0x0000002000000010

    ; ==========================================
    ; Test 9: VGATHERDPS xmm (same encoding as VPGATHERDD but with VEX.66.0F38.W0 0x92)
    ; Same bit pattern result as VPGATHERDD since it's dword gather
    ; ==========================================
    TEST_CASE t9_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dd_128]
    vmovdqa xmm2, [rel mask_all]
    vpxor xmm0, xmm0, xmm0
    vgatherdps xmm0, [rbx + xmm1*4], xmm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000002000000040
    TEST_CASE t9_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000001000000030

    ; ==========================================
    ; Test 10: VGATHERDPS ymm
    ; ==========================================
    TEST_CASE t10_name
    lea rbx, [rel gather_table]
    vmovdqa ymm1, [rel idx_dd_256]
    vmovdqa ymm2, [rel mask_all]
    vpxor ymm0, ymm0, ymm0
    vgatherdps ymm0, [rbx + ymm1*4], ymm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000006000000080
    TEST_CASE t10_name
    vextracti128 xmm3, ymm0, 1
    movq rax, xmm3
    CHECK_EQ_64 rax, 0x0000005000000070

    ; ==========================================
    ; Test 11: VGATHERDPD xmm (2 qwords with dword indices)
    ; ==========================================
    TEST_CASE t11_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dq_128]
    vmovdqa xmm2, [rel mask_all_q]
    vpxor xmm0, xmm0, xmm0
    vgatherdpd xmm0, [rbx + xmm1*8], xmm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000006000000050
    TEST_CASE t11_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000002000000010

    ; ==========================================
    ; Test 12: VGATHERDPD ymm (4 qwords with dword indices)
    ; ==========================================
    TEST_CASE t12_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dq_256]
    vmovdqa ymm2, [rel mask_all_q]
    vpxor ymm0, ymm0, ymm0
    vgatherdpd ymm0, [rbx + xmm1*8], ymm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000008000000070
    TEST_CASE t12_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000004000000030

    ; ==========================================
    ; Test 13: VGATHERQPS xmm (2 dwords with qword indices)
    ; ==========================================
    TEST_CASE t13_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_qd_128]
    vmovdqa xmm2, [rel mask_all]
    vpxor xmm0, xmm0, xmm0
    vgatherqps xmm0, [rbx + xmm1*4], xmm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000001000000030

    ; ==========================================
    ; Test 14: VGATHERQPS ymm (4 dwords with qword indices)
    ; ==========================================
    TEST_CASE t14_name
    lea rbx, [rel gather_table]
    vmovdqa ymm1, [rel idx_qd_256]
    vmovdqa xmm2, [rel mask_all]
    vpxor xmm0, xmm0, xmm0
    vgatherqps xmm0, [rbx + ymm1*4], xmm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000002000000040
    TEST_CASE t14_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000001000000030

    ; ==========================================
    ; Test 15: VGATHERQPD xmm (2 qwords with qword indices)
    ; ==========================================
    TEST_CASE t15_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_qq_128]
    vmovdqa xmm2, [rel mask_all_q]
    vpxor xmm0, xmm0, xmm0
    vgatherqpd xmm0, [rbx + xmm1*8], xmm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000004000000030
    TEST_CASE t15_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000002000000010

    ; ==========================================
    ; Test 16: VGATHERQPD ymm (4 qwords with qword indices)
    ; ==========================================
    TEST_CASE t16_name
    lea rbx, [rel gather_table]
    vmovdqa ymm1, [rel idx_qq_256]
    vmovdqa ymm2, [rel mask_all_q]
    vpxor ymm0, ymm0, ymm0
    vgatherqpd ymm0, [rbx + ymm1*8], ymm2
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000008000000070
    TEST_CASE t16_name
    vextracti128 xmm3, ymm0, 1
    movq rax, xmm3
    CHECK_EQ_64 rax, 0x0000006000000050

    ; ==========================================
    ; Test 17: Partial mask — only some elements gathered
    ; ==========================================
    TEST_CASE t17_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dd_128]       ; indices: [3,1,2,0]
    vmovdqa xmm2, [rel mask_partial]     ; mask: [set,0,set,0] -> only elem 0,2 gathered
    vmovdqa xmm0, [rel prefill_val]      ; pre-fill dest
    vpgatherdd xmm0, [rbx + xmm1*4], xmm2
    ; element[0] = table[3] = 0x40 (gathered, mask was set)
    ; element[1] = 0xDEAD0002 (NOT gathered, mask was 0)
    ; element[2] = table[2] = 0x30 (gathered, mask was set)
    ; element[3] = 0xDEAD0004 (NOT gathered, mask was 0)
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xDEAD000200000040
    TEST_CASE t17_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xDEAD000400000030

    ; ==========================================
    ; Test 18: All-zero mask — no elements gathered, dest unchanged
    ; ==========================================
    TEST_CASE t18_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dd_128]
    vmovdqa xmm2, [rel mask_zero]        ; all zero mask
    vmovdqa xmm0, [rel prefill_val]      ; pre-fill dest
    vpgatherdd xmm0, [rbx + xmm1*4], xmm2
    ; Nothing gathered, dest remains pre-fill
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xDEAD0002DEAD0001
    TEST_CASE t18_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xDEAD0004DEAD0003

    ; ==========================================
    ; Test 19: Scale factor x4 with VPGATHERDD
    ; Using scale=1 (index*1 byte) — we set up byte offsets directly
    ; Actually, let's use scale=1 with indices that are byte offsets
    ; indices = [0, 4, 8, 12] with scale=1 => same as [0,1,2,3] with scale=4
    ; ==========================================
    TEST_CASE t19_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_scale]        ; indices: [0,1,2,3]
    vmovdqa xmm2, [rel mask_all]
    vpxor xmm0, xmm0, xmm0
    ; scale=4: each index * 4 = byte offset for dword-spaced table
    vpgatherdd xmm0, [rbx + xmm1*4], xmm2
    ; element[0]=table[0]=0x10, [1]=table[1]=0x20, [2]=table[2]=0x30, [3]=table[3]=0x40
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000002000000010
    TEST_CASE t19_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000004000000030

    ; ==========================================
    ; Test 20: Negative index test
    ; ==========================================
    TEST_CASE t20_name
    lea rbx, [rel neg_table]
    vmovdqa xmm1, [rel idx_neg_test]     ; indices: [-1, 0, 1, 2]
    vmovdqa xmm2, [rel mask_all]
    vpxor xmm0, xmm0, xmm0
    ; scale=4: index[-1]*4 = -4 bytes from base = neg_prefix[3] = 0xDD
    vpgatherdd xmm0, [rbx + xmm1*4], xmm2
    ; element[0] = *(neg_table - 4) = neg_prefix[3] = 0xDD
    ; element[1] = *(neg_table + 0) = neg_table[0] = 0x10
    ; element[2] = *(neg_table + 4) = neg_table[1] = 0x20
    ; element[3] = *(neg_table + 8) = neg_table[2] = 0x30
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x00000010000000DD
    TEST_CASE t20_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000003000000020

    ; ==========================================
    ; Test 21: Gather with displacement (base + disp + index*scale)
    ; Use rbx pointing 16 bytes before table, with disp=16
    ; ==========================================
    TEST_CASE t21_name
    lea rbx, [rel gather_table]
    sub rbx, 16                           ; base points 16 bytes before table
    vmovdqa xmm1, [rel idx_disp]         ; indices: [0,1,2,3]
    vmovdqa xmm2, [rel mask_all]
    vpxor xmm0, xmm0, xmm0
    ; vpgatherdd xmm0, [rbx + 16 + xmm1*4], xmm2
    ; The displacement is encoded in the VSIB addressing
    vpgatherdd xmm0, [rbx + xmm1*4 + 16], xmm2
    ; Same result as test 19: sequential table access
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000002000000010
    TEST_CASE t21_name
    pextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000004000000030

    ; ==========================================
    ; Test 22: Verify mask is zeroed after gather
    ; ==========================================
    TEST_CASE t22_name
    lea rbx, [rel gather_table]
    vmovdqa xmm1, [rel idx_dd_128]
    vmovdqa xmm2, [rel mask_all]         ; mask starts with sign bits set
    vpxor xmm0, xmm0, xmm0
    vpgatherdd xmm0, [rbx + xmm1*4], xmm2
    ; After gather, xmm2 should be all zeros
    movq rax, xmm2
    CHECK_EQ_64 rax, 0
    TEST_CASE t22_name
    pextrq rax, xmm2, 1
    CHECK_EQ_64 rax, 0

    END_TESTS
