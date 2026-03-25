#ifndef __NEVERCLEAN_PROFILE_H_
#define __NEVERCLEAN_PROFILE_H_

// Compile-time flag: add -DNEVERCLEAN_PROFILE to cmake to enable
// e.g.: cmake .. -DPPC64LE_DYNAREC=ON -DBOX32=ON -DNEVERCLEAN_PROFILE
//
// Prints a summary every NEVERCLEAN_REPORT_INTERVAL LinkNext calls.
// Also prints on SIGUSR1.

#ifdef NEVERCLEAN_PROFILE

#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>

// --- Source classification counters (where PROT_NEVERCLEAN / always_test=1 is
// set) ---

// custommem.c: protectDB / protectDBJumpTable - large host page with mixed
// code+data
extern _Atomic uint64_t nc_src_largepage;
// custommem.c: neverprotectDB() called explicitly
extern _Atomic uint64_t nc_src_neverprotect;
// wrappedlibc.c: mmap with MAP_SHARED + O_RDWR
extern _Atomic uint64_t nc_src_mmap_shared;
// dynarec_native_pass.c: block crosses into NEVERCLEAN page during pass0
extern _Atomic uint64_t nc_src_pass0_cross;
// dynarec_native.c: post-compilation getProtection check
extern _Atomic uint64_t nc_src_postcomp;
// dynarec_native.c: self-loop without ARCH_NOP
extern _Atomic uint64_t nc_src_selfloop;

// --- Hash stability counters (in DBGetBlock) ---

// Total hash validations for always_test==1 blocks
extern _Atomic uint64_t nc_hash_total;
// Hash matched (block unchanged)
extern _Atomic uint64_t nc_hash_match;
// Hash mismatched (block code actually changed)
extern _Atomic uint64_t nc_hash_mismatch;

// --- Hash stability for always_test==2 (hot page) blocks ---
extern _Atomic uint64_t nc_hash_hotpage_total;
extern _Atomic uint64_t nc_hash_hotpage_match;
extern _Atomic uint64_t nc_hash_hotpage_mismatch;

// --- LinkNext dispatch ratio ---
extern _Atomic uint64_t nc_link_total;
extern _Atomic uint64_t nc_link_clean;      // always_test==0
extern _Atomic uint64_t nc_link_neverclean; // always_test==1
extern _Atomic uint64_t nc_link_hotpage;    // always_test==2
extern _Atomic uint64_t
    nc_link_largepage; // always_test==3 (NEVERCLEAN-largepage, cacheable)
extern _Atomic uint64_t nc_link_noblock; // block was NULL

// --- Unique block tracking ---
// We can't easily track unique blocks with atomics alone,
// so we'll just count total block creations with always_test set
extern _Atomic uint64_t nc_blocks_created_neverclean;
extern _Atomic uint64_t nc_blocks_created_hotpage;
extern _Atomic uint64_t nc_blocks_created_clean;
extern _Atomic uint64_t nc_blocks_created_largepage;

// Reporting interval (every N LinkNext calls)
#define NEVERCLEAN_REPORT_INTERVAL 2000000

void neverclean_profile_report(void);
void neverclean_profile_ensure_atexit(void);
void neverclean_profile_install_sigusr1(void);

// Convenience macros
#define NC_INC(counter)                                                        \
  atomic_fetch_add_explicit(&(counter), 1, memory_order_relaxed)

#else // !NEVERCLEAN_PROFILE

#define NC_INC(counter) ((void)0)
#define neverclean_profile_report() ((void)0)
#define neverclean_profile_install_sigusr1() ((void)0)

#endif // NEVERCLEAN_PROFILE

#endif // __NEVERCLEAN_PROFILE_H_
