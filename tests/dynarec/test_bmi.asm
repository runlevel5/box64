; test_bmi.asm - Test BMI1/BMI2 instructions
; BMI1: ANDN, BLSR, BLSMSK, BLSI, BZHI, BEXTR
; BMI2: PDEP, PEXT, MULX, SHRX, SARX, SHLX, RORX
%include "test_framework.inc"

section .data
    ; --- ANDN ---
    t_andn32:       db "andn r32", 0
    t_andn64:       db "andn r64", 0
    t_andn_zf:      db "andn ZF=1 (result 0)", 0
    t_andn_sf:      db "andn SF=1 (neg result)", 0

    ; --- BLSR ---
    t_blsr32:       db "blsr r32", 0
    t_blsr64:       db "blsr r64", 0
    t_blsr_zf:      db "blsr ZF=1 (single bit)", 0
    t_blsr_cf:      db "blsr CF=1 (input 0)", 0

    ; --- BLSMSK ---
    t_blsmsk32:     db "blsmsk r32", 0
    t_blsmsk64:     db "blsmsk r64", 0

    ; --- BLSI ---
    t_blsi32:       db "blsi r32", 0
    t_blsi64:       db "blsi r64", 0
    t_blsi_zf:      db "blsi ZF=1 (input 0)", 0

    ; --- BZHI ---
    t_bzhi32:       db "bzhi r32 idx=8", 0
    t_bzhi64:       db "bzhi r64 idx=16", 0
    t_bzhi32_0:     db "bzhi r32 idx=0", 0
    t_bzhi32_32:    db "bzhi r32 idx=32 (all)", 0

    ; --- BEXTR ---
    t_bextr32:      db "bextr r32 [4:8]", 0
    t_bextr64:      db "bextr r64 [0:16]", 0
    t_bextr_zf:     db "bextr ZF=1 (len=0)", 0

    ; --- PDEP ---
    t_pdep32:       db "pdep r32", 0
    t_pdep64:       db "pdep r64", 0
    t_pdep_zero:    db "pdep mask=0", 0

    ; --- PEXT ---
    t_pext32:       db "pext r32", 0
    t_pext64:       db "pext r64", 0
    t_pext_zero:    db "pext mask=0", 0

    ; --- MULX ---
    t_mulx32:       db "mulx r32", 0
    t_mulx64:       db "mulx r64", 0
    t_mulx32_big:   db "mulx r32 overflow", 0

    ; --- SHRX ---
    t_shrx32:       db "shrx r32", 0
    t_shrx64:       db "shrx r64", 0

    ; --- SARX ---
    t_sarx32:       db "sarx r32", 0
    t_sarx64:       db "sarx r64", 0

    ; --- SHLX ---
    t_shlx32:       db "shlx r32", 0
    t_shlx64:       db "shlx r64", 0

    ; --- RORX ---
    t_rorx32:       db "rorx r32", 0
    t_rorx64:       db "rorx r64", 0
    t_rorx32_0:     db "rorx r32 by 0", 0

section .text
global _start

_start:
    INIT_TESTS

    ;; ================================================================
    ;; ANDN: dest = src2 AND NOT(src1)
    ;; Sets SF, ZF; clears OF, CF
    ;; ================================================================

    ; andn r32: ANDN eax, ecx, ebx = ebx & ~ecx
    TEST_CASE t_andn32
    mov ebx, 0xFF00FF00
    mov ecx, 0xF0F0F0F0
    andn eax, ecx, ebx         ; 0xFF00FF00 & ~0xF0F0F0F0 = 0x0F000F00
    CHECK_EQ_32 eax, 0x0F000F00

    ; andn r64: ANDN rax, rcx, rbx = rbx & ~rcx
    TEST_CASE t_andn64
    mov rbx, 0xFFFFFFFF00000000
    mov rcx, 0xFF00FF00FF00FF00
    andn rax, rcx, rbx         ; 0xFFFFFFFF00000000 & ~0xFF00FF00FF00FF00 = 0x00FF00FF00000000
    CHECK_EQ_64 rax, 0x00FF00FF00000000

    ; andn ZF=1: result is 0
    mov ebx, 0xF0F0F0F0
    mov ecx, 0xFFFFFFFF
    andn eax, ecx, ebx         ; 0xF0F0F0F0 & ~0xFFFFFFFF = 0
    SAVE_FLAGS
    TEST_CASE t_andn_zf
    ; ZF should be set, SF/OF/CF clear
    CHECK_FLAGS_EQ (ZF|SF|OF|CF), ZF

    ; andn SF=1: negative result (bit 31 set)
    mov ebx, 0x80000000
    mov ecx, 0x00000000
    andn eax, ecx, ebx         ; 0x80000000 & ~0 = 0x80000000 (SF=1)
    SAVE_FLAGS
    TEST_CASE t_andn_sf
    CHECK_FLAGS_EQ (ZF|SF|OF|CF), SF

    ;; ================================================================
    ;; BLSR: dest = src AND (src - 1)  -- clears lowest set bit
    ;; Sets SF, ZF, CF; clears OF
    ;; CF=1 if src==0
    ;; ================================================================

    ; blsr r32: 0b1100 -> 0b1000
    TEST_CASE t_blsr32
    mov ebx, 0x0C              ; 0b00001100
    blsr eax, ebx              ; 0b00001100 & 0b00001011 = 0b00001000
    CHECK_EQ_32 eax, 0x08

    ; blsr r64
    TEST_CASE t_blsr64
    mov rbx, 0x8000000000000000
    blsr rax, rbx              ; 0x8000... & 0x7FFF... = 0
    CHECK_EQ_64 rax, 0

    ; blsr ZF=1: single bit input -> result is 0
    mov ebx, 0x00000010        ; single bit set
    blsr eax, ebx              ; 0x10 & 0x0F = 0
    SAVE_FLAGS
    TEST_CASE t_blsr_zf
    CHECK_FLAGS_EQ (ZF|CF|OF), ZF

    ; blsr CF=1: input is 0
    mov ebx, 0
    blsr eax, ebx              ; 0 & (0-1) = 0 & 0xFFFFFFFF = 0, but CF=1
    SAVE_FLAGS
    TEST_CASE t_blsr_cf
    CHECK_FLAGS_EQ (ZF|CF|OF), (ZF|CF)

    ;; ================================================================
    ;; BLSMSK: dest = src XOR (src - 1)  -- mask up to lowest set bit
    ;; Sets SF, CF; clears OF, ZF depends
    ;; CF=1 if src==0
    ;; ================================================================

    ; blsmsk r32: 0b1100 -> 0b0111
    TEST_CASE t_blsmsk32
    mov ebx, 0x0C              ; 0b00001100
    blsmsk eax, ebx            ; 0b00001100 ^ 0b00001011 = 0b00000111
    CHECK_EQ_32 eax, 0x07

    ; blsmsk r64: bit 63 set -> 0xFFFFFFFFFFFFFFFF
    TEST_CASE t_blsmsk64
    mov rbx, 0x8000000000000000
    blsmsk rax, rbx            ; 0x8000... ^ 0x7FFF... = 0xFFFF...
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ;; ================================================================
    ;; BLSI: dest = src AND (-src)  -- isolate lowest set bit
    ;; Sets SF, ZF, CF; clears OF
    ;; CF=1 if src!=0
    ;; ================================================================

    ; blsi r32: 0b1100 -> 0b0100
    TEST_CASE t_blsi32
    mov ebx, 0x0C              ; 0b00001100
    blsi eax, ebx              ; 0b00001100 & 0b11110100 = 0b00000100
    CHECK_EQ_32 eax, 0x04

    ; blsi r64
    TEST_CASE t_blsi64
    mov rbx, 0x0F00000000000000
    blsi rax, rbx              ; isolate lowest bit in 0x0F00... = 0x0100...
    CHECK_EQ_64 rax, 0x0100000000000000

    ; blsi ZF=1: input 0 -> result 0, CF=0
    mov ebx, 0
    blsi eax, ebx              ; 0 & 0 = 0
    SAVE_FLAGS
    TEST_CASE t_blsi_zf
    CHECK_FLAGS_EQ (ZF|CF|OF), ZF

    ;; ================================================================
    ;; BZHI: zero high bits starting at bit index
    ;; dest = src AND ((1 << idx) - 1), if idx < operand_size
    ;; Sets SF, ZF, CF (CF=1 if idx > operand_size)
    ;; ================================================================

    ; bzhi r32 idx=8: keep bits 0-7
    TEST_CASE t_bzhi32
    mov ebx, 0xDEADBEEF
    mov ecx, 8
    bzhi eax, ebx, ecx         ; keep low 8 bits = 0xEF
    CHECK_EQ_32 eax, 0xEF

    ; bzhi r64 idx=16: keep bits 0-15
    TEST_CASE t_bzhi64
    mov rbx, 0xDEADBEEFCAFEBABE
    mov ecx, 16
    bzhi rax, rbx, rcx         ; keep low 16 bits = 0xBABE
    CHECK_EQ_64 rax, 0xBABE

    ; bzhi r32 idx=0: all bits zeroed
    TEST_CASE t_bzhi32_0
    mov ebx, 0xFFFFFFFF
    mov ecx, 0
    bzhi eax, ebx, ecx
    CHECK_EQ_32 eax, 0

    ; bzhi r32 idx=32: all bits kept (idx >= operand size, CF=1)
    TEST_CASE t_bzhi32_32
    mov ebx, 0xDEADBEEF
    mov ecx, 32
    bzhi eax, ebx, ecx         ; idx >= 32, so result = src unchanged
    CHECK_EQ_32 eax, 0xDEADBEEF

    ;; ================================================================
    ;; BEXTR: extract bit field
    ;; start = ctrl[7:0], len = ctrl[15:8]
    ;; dest = (src >> start) & ((1 << len) - 1)
    ;; Sets ZF; clears OF, CF
    ;; ================================================================

    ; bextr r32 [4:8]: extract 8 bits starting at bit 4
    TEST_CASE t_bextr32
    mov ebx, 0xDEADBEEF
    mov ecx, 0x0804            ; len=8, start=4
    bextr eax, ebx, ecx        ; (0xDEADBEEF >> 4) & 0xFF = 0xEADBEEF & 0xFF = 0xEF
    ; Actually: 0xDEADBEEF >> 4 = 0x0DEADBEE, then & 0xFF = 0xEE
    CHECK_EQ_32 eax, 0xEE

    ; bextr r64 [0:16]: extract low 16 bits
    TEST_CASE t_bextr64
    mov rbx, 0xDEADBEEFCAFEBABE
    mov ecx, 0x1000            ; len=16, start=0
    bextr rax, rbx, rcx        ; 0xDEADBEEFCAFEBABE & 0xFFFF = 0xBABE
    CHECK_EQ_64 rax, 0xBABE

    ; bextr ZF=1: len=0 -> result always 0
    mov ebx, 0xFFFFFFFF
    mov ecx, 0x0000            ; len=0, start=0
    bextr eax, ebx, ecx
    SAVE_FLAGS
    TEST_CASE t_bextr_zf
    CHECK_FLAGS_EQ (ZF|OF|CF), ZF

    ;; ================================================================
    ;; PDEP: parallel bit deposit
    ;; Scatter consecutive bits of src into mask positions
    ;; ================================================================

    ; pdep r32: src=0xFF, mask=0x0F0F0F0F
    ;   Bits of src: 7,6,5,4,3,2,1,0
    ;   Deposited into mask positions:
    ;   mask bit 0 <- src bit 0 = 1 -> 0x01
    ;   mask bit 1 <- src bit 1 = 1 -> 0x02
    ;   mask bit 2 <- src bit 2 = 1 -> 0x04
    ;   mask bit 3 <- src bit 3 = 1 -> 0x08
    ;   mask bit 8 <- src bit 4 = 1 -> 0x100
    ;   mask bit 9 <- src bit 5 = 1 -> 0x200
    ;   mask bit 10 <- src bit 6 = 1 -> 0x400
    ;   mask bit 11 <- src bit 7 = 1 -> 0x800
    ;   Result: 0x00000F0F
    TEST_CASE t_pdep32
    mov ebx, 0xFF
    mov ecx, 0x0F0F0F0F
    pdep eax, ebx, ecx         ; deposit 0xFF into mask 0x0F0F0F0F
    CHECK_EQ_32 eax, 0x00000F0F

    ; pdep r64
    TEST_CASE t_pdep64
    mov rbx, 0x0000000000000003    ; src = 0b11
    mov rcx, 0x0000000000000005    ; mask = 0b101
    pdep rax, rbx, rcx             ; bit0->pos0, bit1->pos2 = 0b101 = 5
    CHECK_EQ_64 rax, 5

    ; pdep mask=0: result always 0
    TEST_CASE t_pdep_zero
    mov ebx, 0xFFFFFFFF
    mov ecx, 0
    pdep eax, ebx, ecx
    CHECK_EQ_32 eax, 0

    ;; ================================================================
    ;; PEXT: parallel bit extract
    ;; Gather bits from src at mask positions into consecutive low bits
    ;; ================================================================

    ; pext r32: src=0xDEADBEEF, mask=0x0000000F -> extract low 4 bits = 0xF
    TEST_CASE t_pext32
    mov ebx, 0xDEADBEEF
    mov ecx, 0x0000000F
    pext eax, ebx, ecx
    CHECK_EQ_32 eax, 0x0F

    ; pext r64: src=0xFF, mask=0xAA -> extract bits at positions 1,3,5,7
    ;   bit1=1, bit3=1, bit5=1, bit7=1 -> 0b1111 = 0xF
    TEST_CASE t_pext64
    mov rbx, 0xFF
    mov rcx, 0xAA                  ; mask=10101010
    pext rax, rbx, rcx
    CHECK_EQ_64 rax, 0x0F

    ; pext mask=0: result always 0
    TEST_CASE t_pext_zero
    mov ebx, 0xFFFFFFFF
    mov ecx, 0
    pext eax, ebx, ecx
    CHECK_EQ_32 eax, 0

    ;; ================================================================
    ;; MULX: unsigned multiply EDX * src -> hi:lo (gd:vd)
    ;; No flags modified
    ;; ================================================================

    ; mulx r32: 3 * 7 = 21 (lo=21, hi=0)
    TEST_CASE t_mulx32
    mov edx, 3
    mov ebx, 7
    mulx ecx, eax, ebx         ; ecx=hi, eax=lo
    ; 3*7=21, fits in 32 bits: lo=21, hi=0
    CHECK_EQ_32 eax, 21

    ; mulx r64: large multiply
    TEST_CASE t_mulx64
    mov rdx, 0x100000000        ; 2^32
    mov rbx, 0x100000000        ; 2^32
    mulx rcx, rax, rbx          ; rcx=hi, rax=lo; 2^32 * 2^32 = 2^64
    ; Result: hi=1, lo=0
    CHECK_EQ_64 rcx, 1

    ; mulx r32 overflow: 0xFFFFFFFF * 0xFFFFFFFF = 0xFFFFFFFE00000001
    TEST_CASE t_mulx32_big
    mov edx, 0xFFFFFFFF
    mov ebx, 0xFFFFFFFF
    mulx ecx, eax, ebx         ; hi:lo of 32-bit multiply
    ; 0xFFFFFFFF^2 = 0xFFFFFFFE_00000001
    ; lo = 0x00000001, hi = 0xFFFFFFFE
    CHECK_EQ_32 eax, 0x00000001

    ;; ================================================================
    ;; SHRX: logical shift right, no flags
    ;; ================================================================

    TEST_CASE t_shrx32
    mov ebx, 0xFF000000
    mov ecx, 24
    shrx eax, ebx, ecx         ; 0xFF000000 >> 24 = 0xFF
    CHECK_EQ_32 eax, 0xFF

    TEST_CASE t_shrx64
    mov rbx, 0x8000000000000000
    mov ecx, 63
    shrx rax, rbx, rcx         ; >> 63 = 1
    CHECK_EQ_64 rax, 1

    ;; ================================================================
    ;; SARX: arithmetic shift right, no flags
    ;; ================================================================

    TEST_CASE t_sarx32
    mov ebx, 0x80000000        ; -2^31
    mov ecx, 4
    sarx eax, ebx, ecx         ; arithmetic right shift by 4
    CHECK_EQ_32 eax, 0xF8000000

    TEST_CASE t_sarx64
    mov rbx, 0x8000000000000000
    mov ecx, 63
    sarx rax, rbx, rcx         ; >> 63 = 0xFFFFFFFFFFFFFFFF (-1)
    CHECK_EQ_64 rax, 0xFFFFFFFFFFFFFFFF

    ;; ================================================================
    ;; SHLX: logical shift left, no flags
    ;; ================================================================

    TEST_CASE t_shlx32
    mov ebx, 0x000000FF
    mov ecx, 24
    shlx eax, ebx, ecx         ; 0xFF << 24 = 0xFF000000
    CHECK_EQ_32 eax, 0xFF000000

    TEST_CASE t_shlx64
    mov rbx, 1
    mov ecx, 63
    shlx rax, rbx, rcx         ; 1 << 63
    CHECK_EQ_64 rax, 0x8000000000000000

    ;; ================================================================
    ;; RORX: rotate right by immediate, no flags
    ;; ================================================================

    TEST_CASE t_rorx32
    mov ebx, 0x00000001
    rorx eax, ebx, 1           ; rotate right by 1: 0x80000000
    CHECK_EQ_32 eax, 0x80000000

    TEST_CASE t_rorx64
    mov rbx, 0x0000000000000001
    rorx rax, rbx, 1           ; rotate right by 1: 0x8000000000000000
    CHECK_EQ_64 rax, 0x8000000000000000

    TEST_CASE t_rorx32_0
    mov ebx, 0xDEADBEEF
    rorx eax, ebx, 0           ; rotate by 0 = identity
    CHECK_EQ_32 eax, 0xDEADBEEF

    END_TESTS
