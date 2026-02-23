; test_fma_double_round.asm - Test scalar SS FMA double-rounding fix
; Verifies that all 12 VFMADD/VFMSUB/VFNMADD/VFNMSUB 132/213/231 SS opcodes
; produce correctly-rounded single-precision results, not double-rounded
; (DP FMA then narrow to SP, which gives wrong results at SP midpoints).
;
; Test values chosen so that fmaf(a,b,c) != (float)fma((double)a,(double)b,(double)c):
;   va = 0x3F800001  (~1 + ulp)
;   vb = 0x3F7FFFFE  (~1 - ulp)
;   vc = 0x4B800001  (16777218.0)
;
; Correct (single-round SP FMA):
;   FMADD:  va*vb + vc  = 0x4B800001
;   FMSUB:  va*vb - vc  = 0xCB800001
;   FNMADD: -(va*vb)+vc = 0x4B800001
;   FNMSUB: -(va*vb)-vc = 0xCB800001
;
; Buggy (double-rounded DP→SP):
;   FMADD:  0x4B800002 (+1 ULP error)
;   FMSUB:  0xCB800000 (+1 ULP error)
;   FNMADD: 0x4B800000 (-1 ULP error)
;   FNMSUB: 0xCB800002 (+1 ULP error)

%include "test_framework.inc"

section .data
    t1_name:  db "vfmadd132ss dblrnd", 0
    t2_name:  db "vfmadd213ss dblrnd", 0
    t3_name:  db "vfmadd231ss dblrnd", 0
    t4_name:  db "vfmsub132ss dblrnd", 0
    t5_name:  db "vfmsub213ss dblrnd", 0
    t6_name:  db "vfmsub231ss dblrnd", 0
    t7_name:  db "vfnmadd132ss dblrnd", 0
    t8_name:  db "vfnmadd213ss dblrnd", 0
    t9_name:  db "vfnmadd231ss dblrnd", 0
    t10_name: db "vfnmsub132ss dblrnd", 0
    t11_name: db "vfnmsub213ss dblrnd", 0
    t12_name: db "vfnmsub231ss dblrnd", 0

    align 16
    ; Test values that trigger double-rounding divergence
    val_a: dd 0x3F800001   ; ~1.0000001192 (1 + 2^-23)
    val_b: dd 0x3F7FFFFE   ; ~0.9999998808 (1 - 2^-22)
    val_c: dd 0x4B800001   ; 16777218.0

section .text
global _start
_start:
    INIT_TESTS

    ; Load test values into known registers for reuse
    ; We'll reload before each test to ensure clean state.

    ; --- Test 1: vfmadd132ss: xmm0 = xmm0*xmm2 + xmm1 ---
    ; Want: va*vb + vc = 0x4B800001
    ; Setup: xmm0=va, xmm1=vc, xmm2=vb
    TEST_CASE t1_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_c]
    vmovss xmm2, [rel val_b]
    vfmadd132ss xmm0, xmm1, xmm2   ; xmm0 = va*vb + vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x4B800001

    ; --- Test 2: vfmadd213ss: xmm0 = xmm1*xmm0 + xmm2 ---
    ; Want: vb*va + vc = 0x4B800001
    ; Setup: xmm0=va, xmm1=vb, xmm2=vc
    TEST_CASE t2_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_b]
    vmovss xmm2, [rel val_c]
    vfmadd213ss xmm0, xmm1, xmm2   ; xmm0 = vb*va + vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x4B800001

    ; --- Test 3: vfmadd231ss: xmm0 = xmm1*xmm2 + xmm0 ---
    ; Want: va*vb + vc = 0x4B800001
    ; Setup: xmm0=vc, xmm1=va, xmm2=vb
    TEST_CASE t3_name
    vmovss xmm0, [rel val_c]
    vmovss xmm1, [rel val_a]
    vmovss xmm2, [rel val_b]
    vfmadd231ss xmm0, xmm1, xmm2   ; xmm0 = va*vb + vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x4B800001

    ; --- Test 4: vfmsub132ss: xmm0 = xmm0*xmm2 - xmm1 ---
    ; Want: va*vb - vc = 0xCB800001
    ; Setup: xmm0=va, xmm1=vc, xmm2=vb
    TEST_CASE t4_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_c]
    vmovss xmm2, [rel val_b]
    vfmsub132ss xmm0, xmm1, xmm2   ; xmm0 = va*vb - vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xCB800001

    ; --- Test 5: vfmsub213ss: xmm0 = xmm1*xmm0 - xmm2 ---
    ; Want: vb*va - vc = 0xCB800001
    ; Setup: xmm0=va, xmm1=vb, xmm2=vc
    TEST_CASE t5_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_b]
    vmovss xmm2, [rel val_c]
    vfmsub213ss xmm0, xmm1, xmm2   ; xmm0 = vb*va - vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xCB800001

    ; --- Test 6: vfmsub231ss: xmm0 = xmm1*xmm2 - xmm0 ---
    ; Want: va*vb - vc = 0xCB800001
    ; Setup: xmm0=vc, xmm1=va, xmm2=vb
    TEST_CASE t6_name
    vmovss xmm0, [rel val_c]
    vmovss xmm1, [rel val_a]
    vmovss xmm2, [rel val_b]
    vfmsub231ss xmm0, xmm1, xmm2   ; xmm0 = va*vb - vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xCB800001

    ; --- Test 7: vfnmadd132ss: xmm0 = -(xmm0*xmm2) + xmm1 ---
    ; Want: -(va*vb) + vc = 0x4B800001
    ; Setup: xmm0=va, xmm1=vc, xmm2=vb
    TEST_CASE t7_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_c]
    vmovss xmm2, [rel val_b]
    vfnmadd132ss xmm0, xmm1, xmm2  ; xmm0 = -(va*vb) + vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x4B800001

    ; --- Test 8: vfnmadd213ss: xmm0 = -(xmm1*xmm0) + xmm2 ---
    ; Want: -(vb*va) + vc = 0x4B800001
    ; Setup: xmm0=va, xmm1=vb, xmm2=vc
    TEST_CASE t8_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_b]
    vmovss xmm2, [rel val_c]
    vfnmadd213ss xmm0, xmm1, xmm2  ; xmm0 = -(vb*va) + vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x4B800001

    ; --- Test 9: vfnmadd231ss: xmm0 = -(xmm1*xmm2) + xmm0 ---
    ; Want: -(va*vb) + vc = 0x4B800001
    ; Setup: xmm0=vc, xmm1=va, xmm2=vb
    TEST_CASE t9_name
    vmovss xmm0, [rel val_c]
    vmovss xmm1, [rel val_a]
    vmovss xmm2, [rel val_b]
    vfnmadd231ss xmm0, xmm1, xmm2  ; xmm0 = -(va*vb) + vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0x4B800001

    ; --- Test 10: vfnmsub132ss: xmm0 = -(xmm0*xmm2) - xmm1 ---
    ; Want: -(va*vb) - vc = 0xCB800001
    ; Setup: xmm0=va, xmm1=vc, xmm2=vb
    TEST_CASE t10_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_c]
    vmovss xmm2, [rel val_b]
    vfnmsub132ss xmm0, xmm1, xmm2  ; xmm0 = -(va*vb) - vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xCB800001

    ; --- Test 11: vfnmsub213ss: xmm0 = -(xmm1*xmm0) - xmm2 ---
    ; Want: -(vb*va) - vc = 0xCB800001
    ; Setup: xmm0=va, xmm1=vb, xmm2=vc
    TEST_CASE t11_name
    vmovss xmm0, [rel val_a]
    vmovss xmm1, [rel val_b]
    vmovss xmm2, [rel val_c]
    vfnmsub213ss xmm0, xmm1, xmm2  ; xmm0 = -(vb*va) - vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xCB800001

    ; --- Test 12: vfnmsub231ss: xmm0 = -(xmm1*xmm2) - xmm0 ---
    ; Want: -(va*vb) - vc = 0xCB800001
    ; Setup: xmm0=vc, xmm1=va, xmm2=vb
    TEST_CASE t12_name
    vmovss xmm0, [rel val_c]
    vmovss xmm1, [rel val_a]
    vmovss xmm2, [rel val_b]
    vfnmsub231ss xmm0, xmm1, xmm2  ; xmm0 = -(va*vb) - vc
    movd eax, xmm0
    CHECK_EQ_32 eax, 0xCB800001

    END_TESTS
