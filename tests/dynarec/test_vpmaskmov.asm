; test_vpmaskmov.asm - Test VPMASKMOVD/Q and VMASKMOVPS/PD
; Tests both correctness and fault suppression (masked-off elements must not fault)
%include "test_framework.inc"

section .data
    t1_name:  db "vpmaskmovd {s,0,s,0} lo", 0
    t2_name:  db "vpmaskmovd {s,0,s,0} hi", 0
    t3_name:  db "vpmaskmovd all_set lo", 0
    t4_name:  db "vpmaskmovd all_set hi", 0
    t5_name:  db "vpmaskmovd all_clear lo", 0
    t6_name:  db "vpmaskmovd all_clear hi", 0
    t7_name:  db "vpmaskmovq {s,0} lo", 0
    t8_name:  db "vpmaskmovq {s,0} hi", 0
    t9_name:  db "vpmaskmovq {s,s} lo", 0
    t10_name: db "vpmaskmovq {s,s} hi", 0
    t11_name: db "vmaskmovps {s,0,s,0} lo", 0
    t12_name: db "vmaskmovps {s,0,s,0} hi", 0
    t13_name: db "vmaskmovpd {s,0} lo", 0
    t14_name: db "vmaskmovpd {s,0} hi", 0
    t15_name: db "vpmaskmovd store lo", 0
    t16_name: db "vpmaskmovd store hi", 0
    t17_name: db "vpmaskmovq store lo", 0
    t18_name: db "vpmaskmovq store hi", 0
    t19_name: db "vmaskmovps store lo", 0
    t20_name: db "vmaskmovps store hi", 0
    t21_name: db "vpmaskmovd {0,s,0,s} lo", 0
    t22_name: db "vpmaskmovd {0,s,0,s} hi", 0
    t23_name: db "vmaskmovpd store lo", 0
    t24_name: db "vmaskmovpd store hi", 0
    t25_name: db "vpmaskmovd fault sup lo", 0
    t26_name: db "vpmaskmovd fault sup hi", 0
    t27_name: db "vmaskmovps fault sup lo", 0
    t28_name: db "vmaskmovps fault sup hi", 0
    t29_name: db "vpmaskmovq fault sup lo", 0
    t30_name: db "vpmaskmovq fault sup hi", 0
    t31_name: db "vmaskmovpd fault sup lo", 0
    t32_name: db "vmaskmovpd fault sup hi", 0
    t33_name: db "vpmaskmovd st fault sup", 0
    t34_name: db "vmaskmovps st fault sup", 0

    align 16
    ; Source data for 32-bit tests
    src_d:  dd 0xAAAA0001, 0xBBBB0002, 0xCCCC0003, 0xDDDD0004
    ; Source data for 64-bit tests
    src_q:  dq 0xAAAAAAAA00000001, 0xBBBBBBBB00000002
    ; Float source {1.0, 2.0, 3.0, 4.0}
    src_ps: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000
    ; Double source {1.0, 2.0}
    src_pd: dq 0x3FF0000000000000, 0x4000000000000000

    ; Masks
    align 16
    mask_d_02:   dd 0x80000000, 0x00000000, 0x80000000, 0x00000000   ; elem 0,2 set
    mask_d_all:  dd 0x80000000, 0x80000000, 0x80000000, 0x80000000   ; all set
    mask_d_none: dd 0x00000000, 0x00000000, 0x00000000, 0x00000000   ; all clear
    mask_d_13:   dd 0x00000000, 0x80000000, 0x00000000, 0x80000000   ; elem 1,3 set
    mask_q_0:    dq 0x8000000000000000, 0x0000000000000000            ; elem 0 set
    mask_q_all:  dq 0x8000000000000000, 0x8000000000000000            ; both set

    ; Store source data
    store_d: dd 0x11111111, 0x22222222, 0x33333333, 0x44444444
    store_q: dq 0x1111111111111111, 0x2222222222222222
    ; {1.0, 2.0, 3.0, 4.0}
    store_ps: dd 0x3F800000, 0x40000000, 0x40400000, 0x40800000
    store_pd: dq 0x3FF0000000000000, 0x4000000000000000

section .bss
    align 16
    dst_buf: resb 64
    mmap_ptr: resq 1

section .text
global _start
_start:
    INIT_TESTS

    ; ---- vpmaskmovd mask={set,0,set,0} ----
    vmovdqa xmm1, [rel mask_d_02]
    vpmaskmovd xmm0, xmm1, [rel src_d]

    TEST_CASE t1_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x00000000AAAA0001

    TEST_CASE t2_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x00000000CCCC0003

    ; ---- vpmaskmovd mask=all_set ----
    vmovdqa xmm1, [rel mask_d_all]
    vpmaskmovd xmm0, xmm1, [rel src_d]

    TEST_CASE t3_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xBBBB0002AAAA0001

    TEST_CASE t4_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xDDDD0004CCCC0003

    ; ---- vpmaskmovd mask=all_clear ----
    vmovdqa xmm1, [rel mask_d_none]
    vpmaskmovd xmm0, xmm1, [rel src_d]

    TEST_CASE t5_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x0000000000000000

    TEST_CASE t6_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vpmaskmovq mask={set,clear} ----
    vmovdqa xmm1, [rel mask_q_0]
    vpmaskmovq xmm0, xmm1, [rel src_q]

    TEST_CASE t7_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xAAAAAAAA00000001

    TEST_CASE t8_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vpmaskmovq mask={set,set} ----
    vmovdqa xmm1, [rel mask_q_all]
    vpmaskmovq xmm0, xmm1, [rel src_q]

    TEST_CASE t9_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xAAAAAAAA00000001

    TEST_CASE t10_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xBBBBBBBB00000002

    ; ---- vmaskmovps mask={set,0,set,0} ----
    vmovdqa xmm1, [rel mask_d_02]
    vmaskmovps xmm0, xmm1, [rel src_ps]

    TEST_CASE t11_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x000000003F800000

    TEST_CASE t12_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000040400000

    ; ---- vmaskmovpd mask={set,clear} ----
    vmovdqa xmm1, [rel mask_q_0]
    vmaskmovpd xmm0, xmm1, [rel src_pd]

    TEST_CASE t13_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x3FF0000000000000

    TEST_CASE t14_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vpmaskmovd store mask={set,0,set,0} ----
    vpcmpeqd xmm0, xmm0, xmm0
    vmovdqa [rel dst_buf], xmm0
    vmovdqa xmm2, [rel store_d]
    vmovdqa xmm1, [rel mask_d_02]
    vpmaskmovd [rel dst_buf], xmm1, xmm2

    TEST_CASE t15_name
    mov rax, [rel dst_buf]
    CHECK_EQ_64 rax, 0xFFFFFFFF11111111

    TEST_CASE t16_name
    mov rax, [rel dst_buf+8]
    CHECK_EQ_64 rax, 0xFFFFFFFF33333333

    ; ---- vpmaskmovq store mask={set,clear} ----
    vpcmpeqd xmm0, xmm0, xmm0
    vmovdqa [rel dst_buf], xmm0
    vmovdqa xmm2, [rel store_q]
    vmovdqa xmm1, [rel mask_q_0]
    vpmaskmovq [rel dst_buf], xmm1, xmm2

    TEST_CASE t17_name
    mov rax, [rel dst_buf]
    CHECK_EQ_64 rax, 0x1111111111111111

    TEST_CASE t18_name
    mov rax, [rel dst_buf+8]
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ---- vmaskmovps store mask={set,0,set,0} ----
    vpcmpeqd xmm0, xmm0, xmm0
    vmovdqa [rel dst_buf], xmm0
    vmovaps xmm2, [rel store_ps]
    vmovdqa xmm1, [rel mask_d_02]
    vmaskmovps [rel dst_buf], xmm1, xmm2

    TEST_CASE t19_name
    mov rax, [rel dst_buf]
    CHECK_EQ_64 rax, 0xFFFFFFFF3F800000

    TEST_CASE t20_name
    mov rax, [rel dst_buf+8]
    CHECK_EQ_64 rax, 0xFFFFFFFF40400000

    ; ---- vpmaskmovd mask={0,set,0,set} ----
    vmovdqa xmm1, [rel mask_d_13]
    vpmaskmovd xmm0, xmm1, [rel src_d]

    TEST_CASE t21_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xBBBB000200000000

    TEST_CASE t22_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0xDDDD000400000000

    ; ---- vmaskmovpd store mask={set,clear} ----
    vpcmpeqd xmm0, xmm0, xmm0
    vmovdqa [rel dst_buf], xmm0
    vmovapd xmm2, [rel store_pd]
    vmovdqa xmm1, [rel mask_q_0]
    vmaskmovpd [rel dst_buf], xmm1, xmm2

    TEST_CASE t23_name
    mov rax, [rel dst_buf]
    CHECK_EQ_64 rax, 0x3FF0000000000000

    TEST_CASE t24_name
    mov rax, [rel dst_buf+8]
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ; ================================================================
    ; Fault suppression tests using mmap + mprotect
    ;
    ; Strategy: mmap 3 * 64KB (covers 64KB-page systems like ppc64le),
    ; mprotect the last 64KB as PROT_NONE. Place data at the end of
    ; the RW region, then do masked loads/stores where masked-off
    ; elements would fall in the guard region. Must NOT fault.
    ;
    ; Layout: [RW: 128KB][NONE: 64KB]
    ; Page boundary at base + 128KB
    ; ================================================================

    mov rax, 9                  ; sys_mmap
    xor rdi, rdi
    mov rsi, 196608             ; 3 * 64KB
    mov rdx, 3                  ; PROT_READ | PROT_WRITE
    mov r10, 0x22               ; MAP_PRIVATE | MAP_ANONYMOUS
    mov r8, -1
    xor r9, r9
    syscall
    mov [rel mmap_ptr], rax

    lea rdi, [rax + 131072]     ; base + 128KB
    push rax
    mov rax, 10                 ; sys_mprotect
    mov rsi, 65536              ; 64KB
    xor rdx, rdx                ; PROT_NONE
    syscall
    pop rbx

    lea r8, [rbx + 131072]      ; r8 = boundary (guard starts here)

    ; ---- vpmaskmovd fault suppression ----
    ; Load from page_end - 4 with mask={set,0,0,0}
    ; Only elem 0 (4 bytes) in valid mem; elems 1-3 in guard page
    mov dword [r8 - 4], 0xDEADBEEF
    lea rdi, [r8 - 4]
    vpxor xmm1, xmm1, xmm1
    mov eax, 0x80000000
    vmovd xmm1, eax
    vpmaskmovd xmm0, xmm1, [rdi]

    TEST_CASE t25_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x00000000DEADBEEF

    TEST_CASE t26_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vmaskmovps fault suppression ----
    mov dword [r8 - 4], 0x3F800000     ; 1.0f
    lea rdi, [r8 - 4]
    vpxor xmm1, xmm1, xmm1
    mov eax, 0x80000000
    vmovd xmm1, eax
    vmaskmovps xmm0, xmm1, [rdi]

    TEST_CASE t27_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x000000003F800000

    TEST_CASE t28_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vpmaskmovq fault suppression ----
    ; Load from page_end - 8 with mask={set,clear}
    mov rax, 0xCAFEBABE12345678
    mov [r8 - 8], rax
    lea rdi, [r8 - 8]
    vmovdqa xmm1, [rel mask_q_0]
    vpmaskmovq xmm0, xmm1, [rdi]

    TEST_CASE t29_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0xCAFEBABE12345678

    TEST_CASE t30_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vmaskmovpd fault suppression ----
    mov rax, 0x4000000000000000         ; 2.0
    mov [r8 - 8], rax
    lea rdi, [r8 - 8]
    vmovdqa xmm1, [rel mask_q_0]
    vmaskmovpd xmm0, xmm1, [rdi]

    TEST_CASE t31_name
    vmovq rax, xmm0
    CHECK_EQ_64 rax, 0x4000000000000000

    TEST_CASE t32_name
    vpextrq rax, xmm0, 1
    CHECK_EQ_64 rax, 0x0000000000000000

    ; ---- vpmaskmovd store fault suppression ----
    ; Store to page_end - 4 with mask={set,0,0,0}
    mov dword [r8 - 4], 0xFFFFFFFF
    lea rdi, [r8 - 4]
    mov eax, 0xBAADF00D
    vmovd xmm2, eax
    vpxor xmm1, xmm1, xmm1
    mov eax, 0x80000000
    vmovd xmm1, eax
    vpmaskmovd [rdi], xmm1, xmm2

    TEST_CASE t33_name
    mov eax, [r8 - 4]
    CHECK_EQ_64 rax, 0x00000000BAADF00D

    ; ---- vmaskmovps store fault suppression ----
    mov dword [r8 - 4], 0xFFFFFFFF
    lea rdi, [r8 - 4]
    mov eax, 0x3F800000
    vmovd xmm2, eax
    vpxor xmm1, xmm1, xmm1
    mov eax, 0x80000000
    vmovd xmm1, eax
    vmaskmovps [rdi], xmm1, xmm2

    TEST_CASE t34_name
    mov eax, [r8 - 4]
    CHECK_EQ_64 rax, 0x000000003F800000

    ; Cleanup: munmap
    mov rax, 11                 ; sys_munmap
    mov rdi, [rel mmap_ptr]
    mov rsi, 196608
    syscall

    END_TESTS
