; test_avx_256lane.asm - Test 256-bit AVX lane operations
; VPERMQ, VPERMPD, VPERM2F128, VPERM2I128, VINSERTF128, VINSERTI128, VEXTRACTF128, VEXTRACTI128
%include "test_framework.inc"

section .data
    test_name: db "=== test_avx_256lane ===", 10, 0

    ; Test names
    t_vpermq_id:   db "VPERMQ identity (0xE4)", 0
    t_vpermq_rev:  db "VPERMQ reverse (0x1B)", 0
    t_vpermq_bcast: db "VPERMQ broadcast q0 (0x00)", 0
    t_vpermq_swap: db "VPERMQ swap halves (0x4E)", 0

    t_vpermpd_id:  db "VPERMPD identity (0xE4)", 0
    t_vpermpd_rev: db "VPERMPD reverse (0x1B)", 0

    t_vperm2f_vxlo_exlo:  db "VPERM2F128 Vx.lo,Ex.lo (0x20)", 0
    t_vperm2f_vxhi_exhi:  db "VPERM2F128 Vx.hi,Ex.hi (0x31)", 0
    t_vperm2f_zero_lo:    db "VPERM2F128 zero_lo (0x08)", 0
    t_vperm2f_zero_both:  db "VPERM2F128 zero_both (0x88)", 0
    t_vperm2f_vxhi_vxlo:  db "VPERM2F128 Vx.hi,Vx.lo (0x01)", 0

    t_vperm2i_basic:      db "VPERM2I128 Vx.lo,Ex.hi (0x30)", 0

    t_vinsertf_lo: db "VINSERTF128 insert low (0x00)", 0
    t_vinsertf_hi: db "VINSERTF128 insert high (0x01)", 0
    t_vinserti_lo: db "VINSERTI128 insert low (0x00)", 0
    t_vinserti_hi: db "VINSERTI128 insert high (0x01)", 0

    t_vextractf_lo: db "VEXTRACTF128 extract low (0x00)", 0
    t_vextractf_hi: db "VEXTRACTF128 extract high (0x01)", 0
    t_vextracti_lo: db "VEXTRACTI128 extract low (0x00)", 0
    t_vextracti_hi: db "VEXTRACTI128 extract high (0x01)", 0

    t_vextractf_mem_lo: db "VEXTRACTF128 mem low (0x00)", 0
    t_vextractf_mem_hi: db "VEXTRACTF128 mem high (0x01)", 0

    t_vinsertf_mem: db "VINSERTF128 from mem (0x01)", 0

    t_vperm2f_mem:  db "VPERM2F128 Ex from mem (0x20)", 0

    t_vpermq_mem:   db "VPERMQ from mem (0x1B)", 0

    align 32
    ; Source data for 256-bit operations
    ; YMM layout: low 128 bits at lower address, high 128 bits at higher address
    ; Within 128 bits: q0 at bytes 0-7, q1 at bytes 8-15 (little-endian)
    src1_ymm:
        dq 0x1111111111111111   ; q0
        dq 0x2222222222222222   ; q1
        dq 0x3333333333333333   ; q2
        dq 0x4444444444444444   ; q3

    src2_ymm:
        dq 0xAAAAAAAAAAAAAAAA   ; q0
        dq 0xBBBBBBBBBBBBBBBB   ; q1
        dq 0xCCCCCCCCCCCCCCCC   ; q2
        dq 0xDDDDDDDDDDDDDDDD   ; q3

    ; Float source data
    src1_pd:
        dq 1.0, 2.0, 3.0, 4.0
    src2_pd:
        dq 10.0, 20.0, 30.0, 40.0

    ; 128-bit source for insert operations
    insert_src:
        dq 0xFEDCBA9876543210
        dq 0x0123456789ABCDEF

section .bss
    align 32
    result_buf: resb 32
    extract_buf: resb 16

section .text
    global _start

_start:
    lea rsi, [rel test_name]
    call _fw_print_str
    INIT_TESTS

    ;; ====== VPERMQ tests ======

    ; Load ymm1 = src1_ymm {q0=0x1111.., q1=0x2222.., q2=0x3333.., q3=0x4444..}
    vmovdqa ymm1, [rel src1_ymm]

    ; Test VPERMQ identity (0xE4 = 11 10 01 00 = q3,q2,q1,q0 → identity)
    TEST_CASE t_vpermq_id
    vpermq ymm0, ymm1, 0xE4
    ; Result: q0=0x1111.., q1=0x2222.., q2=0x3333.., q3=0x4444.. (unchanged)
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vpermq_id
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x2222222222222222
    TEST_CASE t_vpermq_id
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vpermq_id
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x4444444444444444

    ; Test VPERMQ reverse (0x1B = 00 01 10 11 = q0,q1,q2,q3)
    TEST_CASE t_vpermq_rev
    vpermq ymm0, ymm1, 0x1B
    ; Result: q0=q3=0x4444.., q1=q2=0x3333.., q2=q1=0x2222.., q3=q0=0x1111..
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4444444444444444
    TEST_CASE t_vpermq_rev
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x3333333333333333
    TEST_CASE t_vpermq_rev
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x2222222222222222
    TEST_CASE t_vpermq_rev
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x1111111111111111

    ; Test VPERMQ broadcast q0 (0x00 = 00 00 00 00 = q0,q0,q0,q0)
    TEST_CASE t_vpermq_bcast
    vpermq ymm0, ymm1, 0x00
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vpermq_bcast
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x1111111111111111
    TEST_CASE t_vpermq_bcast
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vpermq_bcast
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x1111111111111111

    ; Test VPERMQ swap halves (0x4E = 01 00 11 10 = q1,q0,q3,q2)
    TEST_CASE t_vpermq_swap
    vpermq ymm0, ymm1, 0x4E
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vpermq_swap
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4444444444444444
    TEST_CASE t_vpermq_swap
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vpermq_swap
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ;; ====== VPERMPD tests ======

    ; Load ymm1 = src1_pd {1.0, 2.0, 3.0, 4.0}
    vmovapd ymm1, [rel src1_pd]

    ; Test VPERMPD identity (0xE4)
    TEST_CASE t_vpermpd_id
    vpermpd ymm0, ymm1, 0xE4
    ; Check q0=1.0 (0x3FF0000000000000)
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3FF0000000000000
    TEST_CASE t_vpermpd_id
    ; Check q1=2.0 (0x4000000000000000)
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4000000000000000

    ; Test VPERMPD reverse (0x1B)
    TEST_CASE t_vpermpd_rev
    vpermpd ymm0, ymm1, 0x1B
    ; Result: q0=4.0, q1=3.0, q2=2.0, q3=1.0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4010000000000000   ; 4.0
    TEST_CASE t_vpermpd_rev
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4008000000000000   ; 3.0

    ;; ====== VPERM2F128 tests ======

    vmovdqa ymm1, [rel src1_ymm]    ; Vx: q0=0x1111.., q1=0x2222.., q2=0x3333.., q3=0x4444..
    vmovdqa ymm2, [rel src2_ymm]    ; Ex: q0=0xAAAA.., q1=0xBBBB.., q2=0xCCCC.., q3=0xDDDD..

    ; Test VPERM2F128: low=Ex.lo(0x2), high=Vx.lo(0x00) → imm8=0x02 (result_lo=Ex.lo, result_hi=Vx.lo)
    ; Correction: imm8[1:0]=src for lo, imm8[5:4]=src for hi
    ; We want: result_lo=Ex.lo (sel=2), result_hi=Vx.lo (sel=0)
    ; imm8 = (0 << 4) | 2 = 0x02
    TEST_CASE t_vperm2f_vxlo_exlo
    vperm2f128 ymm0, ymm1, ymm2, 0x02
    ; result low = Ex.lo = {0xAAAA.., 0xBBBB..}
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xAAAAAAAAAAAAAAAA
    TEST_CASE t_vperm2f_vxlo_exlo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0xBBBBBBBBBBBBBBBB
    TEST_CASE t_vperm2f_vxlo_exlo
    ; result high = Vx.lo = {0x1111.., 0x2222..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vperm2f_vxlo_exlo
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ; Test VPERM2F128: low=Vx.hi(1), high=Ex.hi(3) → imm8=(3<<4)|1 = 0x31
    TEST_CASE t_vperm2f_vxhi_exhi
    vperm2f128 ymm0, ymm1, ymm2, 0x31
    ; result low = Vx.hi = {0x3333.., 0x4444..}
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vperm2f_vxhi_exhi
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4444444444444444
    TEST_CASE t_vperm2f_vxhi_exhi
    ; result high = Ex.hi = {0xCCCC.., 0xDDDD..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0xCCCCCCCCCCCCCCCC
    TEST_CASE t_vperm2f_vxhi_exhi
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0xDDDDDDDDDDDDDDDD

    ; Test VPERM2F128: zero low, high=Vx.lo → imm8 = 0x08 | (0x00 << 4) = 0x08
    TEST_CASE t_vperm2f_zero_lo
    vperm2f128 ymm0, ymm1, ymm2, 0x08
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000000
    TEST_CASE t_vperm2f_zero_lo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x0000000000000000
    TEST_CASE t_vperm2f_zero_lo
    ; high = Vx.lo = {0x1111.., 0x2222..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vperm2f_zero_lo
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ; Test VPERM2F128: zero both → imm8 = 0x88
    TEST_CASE t_vperm2f_zero_both
    vperm2f128 ymm0, ymm1, ymm2, 0x88
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000000
    TEST_CASE t_vperm2f_zero_both
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x0000000000000000

    ; Test VPERM2F128: low=Vx.hi(1), high=Vx.lo(0) → imm8=(0<<4)|1=0x01
    TEST_CASE t_vperm2f_vxhi_vxlo
    vperm2f128 ymm0, ymm1, ymm2, 0x01
    ; result low = Vx.hi = {0x3333.., 0x4444..}
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vperm2f_vxhi_vxlo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4444444444444444
    TEST_CASE t_vperm2f_vxhi_vxlo
    ; result high = Vx.lo = {0x1111.., 0x2222..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vperm2f_vxhi_vxlo
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ;; ====== VPERM2I128 tests ======

    vmovdqa ymm1, [rel src1_ymm]
    vmovdqa ymm2, [rel src2_ymm]

    ; Test VPERM2I128: low=Vx.lo(0), high=Ex.hi(3) → imm8=(3<<4)|0=0x30
    TEST_CASE t_vperm2i_basic
    vperm2i128 ymm0, ymm1, ymm2, 0x30
    ; result low = Vx.lo = {0x1111.., 0x2222..}
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vperm2i_basic
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x2222222222222222
    TEST_CASE t_vperm2i_basic
    ; result high = Ex.hi = {0xCCCC.., 0xDDDD..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0xCCCCCCCCCCCCCCCC
    TEST_CASE t_vperm2i_basic
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0xDDDDDDDDDDDDDDDD

    ;; ====== VINSERTF128 / VINSERTI128 tests ======

    vmovdqa ymm1, [rel src1_ymm]    ; {0x1111.., 0x2222.., 0x3333.., 0x4444..}
    vmovdqa xmm3, [rel insert_src]   ; {0xFEDCBA.., 0x012345..}

    ; Test VINSERTF128 insert low (imm8=0): Gx.lo=Ex, Gx.hi=Vx.hi
    TEST_CASE t_vinsertf_lo
    vinsertf128 ymm0, ymm1, xmm3, 0
    ; lo = insert_src
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFEDCBA9876543210
    TEST_CASE t_vinsertf_lo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x0123456789ABCDEF
    TEST_CASE t_vinsertf_lo
    ; hi = Vx.hi = {0x3333.., 0x4444..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vinsertf_lo
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x4444444444444444

    ; Test VINSERTF128 insert high (imm8=1): Gx.lo=Vx.lo, Gx.hi=Ex
    TEST_CASE t_vinsertf_hi
    vinsertf128 ymm0, ymm1, xmm3, 1
    ; lo = Vx.lo = {0x1111.., 0x2222..}
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vinsertf_hi
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x2222222222222222
    TEST_CASE t_vinsertf_hi
    ; hi = insert_src
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0xFEDCBA9876543210
    TEST_CASE t_vinsertf_hi
    pextrq rbx, xmm5, 1
    CHECK_EQ_64 rbx, 0x0123456789ABCDEF

    ; Test VINSERTI128 insert low (imm8=0)
    TEST_CASE t_vinserti_lo
    vinserti128 ymm0, ymm1, xmm3, 0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0xFEDCBA9876543210
    TEST_CASE t_vinserti_lo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x0123456789ABCDEF

    ; Test VINSERTI128 insert high (imm8=1)
    TEST_CASE t_vinserti_hi
    vinserti128 ymm0, ymm1, xmm3, 1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vinserti_hi
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ;; ====== VEXTRACTF128 / VEXTRACTI128 tests ======

    vmovdqa ymm1, [rel src1_ymm]    ; {0x1111.., 0x2222.., 0x3333.., 0x4444..}

    ; Test VEXTRACTF128 extract low (imm8=0)
    TEST_CASE t_vextractf_lo
    vextractf128 xmm0, ymm1, 0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vextractf_lo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ; Test VEXTRACTF128 extract high (imm8=1)
    TEST_CASE t_vextractf_hi
    vextractf128 xmm0, ymm1, 1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vextractf_hi
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4444444444444444

    ; Test VEXTRACTI128 extract low (imm8=0)
    TEST_CASE t_vextracti_lo
    vextracti128 xmm0, ymm1, 0
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vextracti_lo
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x2222222222222222

    ; Test VEXTRACTI128 extract high (imm8=1)
    TEST_CASE t_vextracti_hi
    vextracti128 xmm0, ymm1, 1
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vextracti_hi
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x4444444444444444

    ;; ====== Memory operand tests ======

    vmovdqa ymm1, [rel src1_ymm]

    ; Test VEXTRACTF128 to memory (low)
    TEST_CASE t_vextractf_mem_lo
    vextractf128 [rel extract_buf], ymm1, 0
    mov rax, [rel extract_buf]
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vextractf_mem_lo
    mov rbx, [rel extract_buf + 8]
    CHECK_EQ_64 rbx, 0x2222222222222222

    ; Test VEXTRACTF128 to memory (high)
    TEST_CASE t_vextractf_mem_hi
    vextractf128 [rel extract_buf], ymm1, 1
    mov rax, [rel extract_buf]
    CHECK_EQ_64 rax, 0x3333333333333333
    TEST_CASE t_vextractf_mem_hi
    mov rbx, [rel extract_buf + 8]
    CHECK_EQ_64 rbx, 0x4444444444444444

    ; Test VINSERTF128 from memory
    TEST_CASE t_vinsertf_mem
    vinsertf128 ymm0, ymm1, [rel insert_src], 1
    ; lo = Vx.lo = {0x1111.., 0x2222..}
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vinsertf_mem
    ; hi = insert_src = {0xFEDCBA.., 0x012345..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0xFEDCBA9876543210

    ; Test VPERM2F128 with memory operand
    vmovdqa ymm1, [rel src1_ymm]
    TEST_CASE t_vperm2f_mem
    ; low=Vx.lo(0), high=Ex.lo(2) where Ex is from memory
    ; imm8 = (2<<4)|0 = 0x20
    vperm2f128 ymm0, ymm1, [rel src2_ymm], 0x20
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x1111111111111111
    TEST_CASE t_vperm2f_mem
    ; high = Ex.lo = {0xAAAA.., 0xBBBB..}
    vextracti128 xmm5, ymm0, 1
    movq rax, xmm5
    CHECK_EQ_64 rax, 0xAAAAAAAAAAAAAAAA

    ; Test VPERMQ from memory
    TEST_CASE t_vpermq_mem
    vpermq ymm0, [rel src1_ymm], 0x1B
    ; reverse: q0=q3=0x4444.., q1=q2=0x3333..
    movq rax, xmm0
    CHECK_EQ_64 rax, 0x4444444444444444
    TEST_CASE t_vpermq_mem
    pextrq rbx, xmm0, 1
    CHECK_EQ_64 rbx, 0x3333333333333333

    vzeroupper
    END_TESTS
