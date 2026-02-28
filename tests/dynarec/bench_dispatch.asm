; bench_dispatch.asm - Block dispatch stress test for box64 dynarec
;
; Designed to create many small blocks with frequent transitions,
; mimicking the pattern that makes NEVERCLEAN overhead visible.
;
; Build:
;   nasm -f elf64 -o /tmp/bench_dispatch.o bench_dispatch.asm
;   x86_64-linux-gnu-ld -o /tmp/bench_dispatch /tmp/bench_dispatch.o
;
; Run:  box64 /tmp/bench_dispatch
; Profile: sudo perf record -g -F 4000 -- box64 /tmp/bench_dispatch

BITS 64

; ---- syscall helpers ----
%define SYS_write   1
%define SYS_exit    60
%define SYS_clock_gettime 228
%define CLOCK_MONOTONIC 1
%define STDOUT 1

section .data

; Data intermixed in same section - on large pages this will share
; pages with .text, triggering NEVERCLEAN
msg_start:  db "bench_dispatch: running...", 10
msg_start_len equ $ - msg_start

msg_phase1: db "Phase 1 done: indirect call dispatch", 10
msg_p1_len equ $ - msg_phase1

msg_phase2: db "Phase 2 done: switch/jump table dispatch", 10
msg_p2_len equ $ - msg_phase2

msg_phase3: db "Phase 3 done: nested call chains", 10
msg_p3_len equ $ - msg_phase3

msg_phase4: db "Phase 4 done: conditional branch storm", 10
msg_p4_len equ $ - msg_phase4

msg_done:   db "All phases complete.", 10
msg_done_len equ $ - msg_done

msg_elapsed: db "Elapsed: "
msg_el_len equ $ - msg_elapsed

msg_sec:    db " seconds", 10
msg_sec_len equ $ - msg_sec

; Mutable data that will share pages with code on 64KB page systems
align 8
counter:    dq 0
accumulator: dq 0
ts_start:   dq 0, 0    ; tv_sec, tv_nsec
ts_end:     dq 0, 0

; Function pointer table (16 entries)
; These pointers + the data below = code+data on same page = NEVERCLEAN
align 8
func_table:
    dq fn_add, fn_sub, fn_mul, fn_xor
    dq fn_and, fn_or,  fn_shl, fn_shr
    dq fn_rol, fn_ror, fn_not, fn_neg
    dq fn_inc, fn_dec, fn_bswap, fn_mix

; Per-function call counters (written during benchmark = dirty data on code pages)
align 8
call_counts: times 16 dq 0

; Total iterations for each phase
ITERATIONS equ 20000000
CHAIN_DEPTH equ 16
BRANCH_ITERATIONS equ 20000000

section .text
global _start

; ---- Print helper: write(1, msg, len) ----
; rdi = msg, rsi = len
print:
    push rax
    push rdi
    push rsi
    push rdx
    mov rdx, rsi        ; len
    mov rsi, rdi         ; msg
    mov rdi, STDOUT
    mov rax, SYS_write
    syscall
    pop rdx
    pop rsi
    pop rdi
    pop rax
    ret

; ---- Get monotonic time into [rdi] ----
get_time:
    push rax
    push rsi
    push rdx
    mov rsi, rdi         ; timespec pointer
    mov rdi, CLOCK_MONOTONIC
    mov rax, SYS_clock_gettime
    syscall
    pop rdx
    pop rsi
    pop rax
    ret

; ---- 16 small ALU functions (each = separate dynarec block) ----
; Input: edi = a, esi = b
; Output: eax = result

fn_add:
    lea eax, [rdi + rsi]
    ret

fn_sub:
    mov eax, edi
    sub eax, esi
    ret

fn_mul:
    mov eax, edi
    imul eax, esi
    ret

fn_xor:
    mov eax, edi
    xor eax, esi
    ret

fn_and:
    mov eax, edi
    and eax, esi
    ret

fn_or:
    mov eax, edi
    or eax, esi
    ret

fn_shl:
    mov eax, edi
    mov ecx, esi
    and ecx, 31
    shl eax, cl
    ret

fn_shr:
    mov eax, edi
    mov ecx, esi
    and ecx, 31
    shr eax, cl
    ret

fn_rol:
    mov eax, edi
    mov ecx, esi
    and ecx, 31
    rol eax, cl
    ret

fn_ror:
    mov eax, edi
    mov ecx, esi
    and ecx, 31
    ror eax, cl
    ret

fn_not:
    mov eax, edi
    not eax
    ret

fn_neg:
    mov eax, edi
    neg eax
    ret

fn_inc:
    lea eax, [rdi + 1]
    ret

fn_dec:
    lea eax, [rdi - 1]
    ret

fn_bswap:
    mov eax, edi
    bswap eax
    ret

fn_mix:
    mov eax, edi
    imul eax, 0x9E3779B1   ; golden ratio hash
    mov ecx, esi
    imul ecx, 0x85EBCA77
    xor eax, ecx
    ret

; ---- Nested call chain functions (4 mutually recursive) ----
; edi = value, esi = depth remaining
; Returns eax

chain_a:
    test esi, esi
    jle .done
    xor edi, 0xDEADBEEF    ; actually use lower 32 bits
    dec esi
    call chain_b
    inc eax
    ret
.done:
    mov eax, edi
    ret

chain_b:
    test esi, esi
    jle .done
    imul edi, edi, 31
    dec esi
    call chain_c
    add eax, 2
    ret
.done:
    mov eax, edi
    ret

chain_c:
    test esi, esi
    jle .done
    add edi, 0x12345678
    dec esi
    call chain_d
    add eax, 3
    ret
.done:
    mov eax, edi
    ret

chain_d:
    test esi, esi
    jle .done
    shr edi, 1
    dec esi
    call chain_a
    add eax, 4
    ret
.done:
    mov eax, edi
    ret

; ---- Print decimal number ----
; rdi = number to print
print_number:
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    sub rsp, 32          ; buffer on stack
    mov rax, rdi
    lea rsi, [rsp + 31]
    mov byte [rsi], 0
    mov rcx, 10
.loop:
    dec rsi
    xor edx, edx
    div rcx
    add dl, '0'
    mov [rsi], dl
    test rax, rax
    jnz .loop
    ; rsi points to start of digits
    lea rdx, [rsp + 31]
    sub rdx, rsi         ; length
    mov rdi, rsi
    mov rsi, rdx
    call print
    add rsp, 32
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    ret

; ======== MAIN ========
_start:
    ; Print start message
    mov rdi, msg_start
    mov rsi, msg_start_len
    call print

    ; Record start time
    lea rdi, [rel ts_start]
    call get_time

    ; ============================================================
    ; Phase 1: Indirect call dispatch via function pointer table
    ; Each iteration: pick a function from table based on accumulator,
    ; call it. This creates unpredictable indirect branches.
    ; ============================================================
    xor ebx, ebx           ; ebx = accumulator
    mov r12d, ITERATIONS    ; loop counter
.phase1_loop:
    ; Index = (acc ^ counter) & 0xF
    mov eax, ebx
    xor eax, r12d
    and eax, 0xF

    ; Load function pointer from table
    lea rcx, [rel func_table]
    mov rax, [rcx + rax*8]

    ; Call it: fn(acc, counter)
    mov edi, ebx
    mov esi, r12d
    call rax

    ; Update accumulator
    mov ebx, eax

    ; Write to data array (mutate data on code pages)
    mov eax, ebx
    and eax, 0xF
    lea rcx, [rel call_counts]
    inc qword [rcx + rax*8]

    dec r12d
    jnz .phase1_loop

    mov [rel accumulator], rbx

    ; Print phase 1 done
    mov rdi, msg_phase1
    mov rsi, msg_p1_len
    call print

    ; ============================================================
    ; Phase 2: Jump-table based dispatch (like a switch statement)
    ; A computed jump into 16 small blocks, each doing different ops.
    ; ============================================================
    xor ebx, ebx           ; accumulator
    mov r12d, ITERATIONS
.phase2_loop:
    mov eax, ebx
    xor eax, r12d
    and eax, 0xF

    ; Computed jump into case table
    lea rcx, [rel .jump_table]
    movsxd rax, dword [rcx + rax*4]
    add rax, rcx
    jmp rax

align 8
.jump_table:
    dd .case0  - .jump_table
    dd .case1  - .jump_table
    dd .case2  - .jump_table
    dd .case3  - .jump_table
    dd .case4  - .jump_table
    dd .case5  - .jump_table
    dd .case6  - .jump_table
    dd .case7  - .jump_table
    dd .case8  - .jump_table
    dd .case9  - .jump_table
    dd .case10 - .jump_table
    dd .case11 - .jump_table
    dd .case12 - .jump_table
    dd .case13 - .jump_table
    dd .case14 - .jump_table
    dd .case15 - .jump_table

.case0:  add ebx, r12d
         jmp .phase2_next
.case1:  sub ebx, r12d
         jmp .phase2_next
.case2:  imul ebx, r12d
         jmp .phase2_next
.case3:  xor ebx, r12d
         jmp .phase2_next
.case4:  and ebx, r12d
         jmp .phase2_next
.case5:  or  ebx, r12d
         jmp .phase2_next
.case6:  mov ecx, r12d
         and ecx, 31
         shl ebx, cl
         jmp .phase2_next
.case7:  mov ecx, r12d
         and ecx, 31
         shr ebx, cl
         jmp .phase2_next
.case8:  mov ecx, r12d
         and ecx, 31
         rol ebx, cl
         jmp .phase2_next
.case9:  mov ecx, r12d
         and ecx, 31
         ror ebx, cl
         jmp .phase2_next
.case10: not ebx
         jmp .phase2_next
.case11: neg ebx
         jmp .phase2_next
.case12: inc ebx
         jmp .phase2_next
.case13: dec ebx
         jmp .phase2_next
.case14: bswap ebx
         jmp .phase2_next
.case15: imul ebx, 0x9E3779B1
         xor ebx, r12d
         jmp .phase2_next

.phase2_next:
    dec r12d
    jnz .phase2_loop

    mov [rel accumulator], rbx

    mov rdi, msg_phase2
    mov rsi, msg_p2_len
    call print

    ; ============================================================
    ; Phase 3: Nested call chains (deep call/ret sequences)
    ; Each iteration does CHAIN_DEPTH levels of call/ret through
    ; 4 mutually recursive functions = lots of block transitions.
    ; ============================================================
    xor ebx, ebx
    mov r12d, ITERATIONS / CHAIN_DEPTH  ; fewer iterations but deeper
.phase3_loop:
    mov edi, ebx
    add edi, r12d
    mov esi, CHAIN_DEPTH
    call chain_a
    xor ebx, eax

    dec r12d
    jnz .phase3_loop

    mov [rel accumulator], rbx

    mov rdi, msg_phase3
    mov rsi, msg_p3_len
    call print

    ; ============================================================
    ; Phase 4: Conditional branch storm
    ; Many unpredictable branches that create lots of small blocks.
    ; Uses a simple PRNG to make branches hard to predict.
    ; ============================================================
    mov ebx, 0x12345678     ; PRNG state
    xor r13d, r13d          ; counter for "taken" branches
    mov r12d, BRANCH_ITERATIONS
.phase4_loop:
    ; xorshift32 PRNG
    mov eax, ebx
    shl eax, 13
    xor ebx, eax
    mov eax, ebx
    shr eax, 17
    xor ebx, eax
    mov eax, ebx
    shl eax, 5
    xor ebx, eax

    ; Branch on various bits (each creates a different block transition)
    test ebx, 1
    jz .p4_skip1
    add r13d, 1
.p4_skip1:
    test ebx, 2
    jz .p4_skip2
    add r13d, 2
.p4_skip2:
    test ebx, 4
    jz .p4_skip3
    xor r13d, ebx
.p4_skip3:
    test ebx, 8
    jz .p4_skip4
    sub r13d, 1
.p4_skip4:
    test ebx, 16
    jz .p4_skip5
    ror r13d, 3
.p4_skip5:
    test ebx, 32
    jz .p4_skip6
    add r13d, ebx
.p4_skip6:
    test ebx, 64
    jz .p4_skip7
    xor r13d, 0xCAFEBABE
.p4_skip7:
    test ebx, 128
    jz .p4_skip8
    not r13d
.p4_skip8:

    dec r12d
    jnz .phase4_loop

    mov [rel accumulator], r13

    mov rdi, msg_phase4
    mov rsi, msg_p4_len
    call print

    ; ============================================================
    ; End timing and print elapsed
    ; ============================================================
    lea rdi, [rel ts_end]
    call get_time

    ; Compute elapsed seconds
    mov rax, [rel ts_end]
    sub rax, [rel ts_start]

    ; Print "Elapsed: N seconds"
    mov rdi, msg_elapsed
    mov rsi, msg_el_len
    call print

    mov rax, [rel ts_end]
    sub rax, [rel ts_start]
    mov rdi, rax
    call print_number

    mov rdi, msg_sec
    mov rsi, msg_sec_len
    call print

    ; Print done message
    mov rdi, msg_done
    mov rsi, msg_done_len
    call print

    ; Exit
    mov rdi, 0
    mov rax, SYS_exit
    syscall
