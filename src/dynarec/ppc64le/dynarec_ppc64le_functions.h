#ifndef __DYNAREC_PPC64LE_FUNCTIONS_H__
#define __DYNAREC_PPC64LE_FUNCTIONS_H__

#include "../dynarec_native_functions.h"

// First VMX scratch register index
#define SCRATCH0    24

typedef struct x64emu_s x64emu_t;
typedef struct dynarec_ppc64le_s dynarec_ppc64le_t;

// Get an FPU scratch reg
int fpu_get_scratch(dynarec_ppc64le_t* dyn);
// Reset scratch regs counter
void fpu_reset_scratch(dynarec_ppc64le_t* dyn);
// Get an x87 double reg
int fpu_get_reg_x87(dynarec_ppc64le_t* dyn, int t, int n);
// Get an XMM quad reg
int fpu_get_reg_xmm(dynarec_ppc64le_t* dyn, int t, int xmm);
// Get an YMM quad reg
int fpu_get_reg_ymm(dynarec_ppc64le_t* dyn, int t, int ymm);
// Free a FPU/MMX/XMM reg
void fpu_free_reg(dynarec_ppc64le_t* dyn, int reg);
// Reset fpu regs counter
void fpu_reset_reg(dynarec_ppc64le_t* dyn);
// Get an MMX double reg
int fpu_get_reg_emm(dynarec_ppc64le_t* dyn, int emm);

// ---- VMX cache functions
// Get type for STx
int vmxcache_get_st(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get if STx is FLOAT or DOUBLE
int vmxcache_get_st_f(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get if STx is FLOAT or I64
int vmxcache_get_st_f_i64(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get actual type for STx
int vmxcache_get_current_st(dynarec_ppc64le_t* dyn, int ninst, int a);
// Get actual STx is FLOAT or DOUBLE
int vmxcache_get_current_st_f(dynarec_ppc64le_t* dyn, int a);
// Get actual STx is FLOAT or I64
int vmxcache_get_current_st_f_i64(dynarec_ppc64le_t* dyn, int a);
// Back-propagate a change float->double
void vmxcache_promote_double(dynarec_ppc64le_t* dyn, int ninst, int a);
// Combine and propagate if needed (pass 1 only)
int vmxcache_combine_st(dynarec_ppc64le_t* dyn, int ninst, int a, int b);  // with stack current dyn->v_stack*
// Do not allow i64 type
int vmxcache_no_i64(dynarec_ppc64le_t* dyn, int ninst, int st, int a);

// FPU Cache transformation (for loops) // Specific, need to be written by backend
int fpuCacheNeedsTransform(dynarec_ppc64le_t* dyn, int ninst);

// Undo the changes of a vmxcache to get the status before the instruction
void vmxcacheUnwind(vmxcache_t* cache);
void fpu_save_and_unwind(dynarec_ppc64le_t* dyn, int ninst, vmxcache_t* cache);
void fpu_unwind_restore(dynarec_ppc64le_t* dyn, int ninst, vmxcache_t* cache);

const char* getCacheName(int t, int n);

void inst_name_pass3(dynarec_native_t* dyn, int ninst, const char* name, rex_t rex);
void print_opcode(dynarec_native_t* dyn, int ninst, uint32_t opcode);

// reset the cache
void fpu_reset(dynarec_native_t* dyn);
void fpu_reset_ninst(dynarec_native_t* dyn, int ninst);
// is st freed
int fpu_is_st_freed(dynarec_native_t* dyn, int ninst, int st);

// propagate FPU_BARRIER to trigger it as soon as possible (avoiding fetching an FPU reg if it's unused)
void propagateFpuBarrier(dynarec_ppc64le_t* dyn);

// propagate the unneeded flags on XMM/YMM regs (done between step 0 and step 1)
void updateYmm0s(dynarec_ppc64le_t* dyn, int ninst, int max_ninst_reached);

// Update native flags fusion info
void updateNativeFlags(dynarec_ppc64le_t* dyn);

// Get free scratch registers avoiding native flag operands
void get_free_scratch(dynarec_ppc64le_t* dyn, int ninst, uint8_t* tmp1, uint8_t* tmp2, uint8_t* tmp3, uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5);

// Try to put FPU barrier earlier
void tryEarlyFpuBarrier(dynarec_ppc64le_t* dyn, int last_fpu_used, int ninst);

#endif //__DYNAREC_PPC64LE_FUNCTIONS_H__
