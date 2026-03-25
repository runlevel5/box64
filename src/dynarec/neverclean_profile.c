#ifdef NEVERCLEAN_PROFILE

#include "neverclean_profile.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

// --- Source classification counters ---
_Atomic uint64_t nc_src_largepage = 0;
_Atomic uint64_t nc_src_neverprotect = 0;
_Atomic uint64_t nc_src_mmap_shared = 0;
_Atomic uint64_t nc_src_pass0_cross = 0;
_Atomic uint64_t nc_src_postcomp = 0;
_Atomic uint64_t nc_src_selfloop = 0;

// --- Hash stability counters ---
_Atomic uint64_t nc_hash_total = 0;
_Atomic uint64_t nc_hash_match = 0;
_Atomic uint64_t nc_hash_mismatch = 0;

_Atomic uint64_t nc_hash_hotpage_total = 0;
_Atomic uint64_t nc_hash_hotpage_match = 0;
_Atomic uint64_t nc_hash_hotpage_mismatch = 0;

// --- LinkNext dispatch ratio ---
_Atomic uint64_t nc_link_total = 0;
_Atomic uint64_t nc_link_clean = 0;
_Atomic uint64_t nc_link_neverclean = 0;
_Atomic uint64_t nc_link_hotpage = 0;
_Atomic uint64_t nc_link_largepage = 0;
_Atomic uint64_t nc_link_noblock = 0;

// --- Block creation counters ---
_Atomic uint64_t nc_blocks_created_neverclean = 0;
_Atomic uint64_t nc_blocks_created_hotpage = 0;
_Atomic uint64_t nc_blocks_created_clean = 0;
_Atomic uint64_t nc_blocks_created_largepage = 0;

static int nc_atexit_installed = 0;

static void nc_atexit_handler(void) {
  fprintf(stderr, "\n=== FINAL NEVERCLEAN PROFILE (atexit) ===\n");
  neverclean_profile_report();
}

void neverclean_profile_ensure_atexit(void) {
  if (!nc_atexit_installed) {
    nc_atexit_installed = 1;
    atexit(nc_atexit_handler);
  }
}

static void sigusr1_handler(int sig) {
  (void)sig;
  neverclean_profile_report();
}

void neverclean_profile_install_sigusr1(void) {
  struct sigaction sa;
  sa.sa_handler = sigusr1_handler;
  sa.sa_flags = SA_RESTART;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR1, &sa, NULL);
}

void neverclean_profile_report(void) {
  uint64_t lt = atomic_load_explicit(&nc_link_total, memory_order_relaxed);
  uint64_t lc = atomic_load_explicit(&nc_link_clean, memory_order_relaxed);
  uint64_t ln = atomic_load_explicit(&nc_link_neverclean, memory_order_relaxed);
  uint64_t lh = atomic_load_explicit(&nc_link_hotpage, memory_order_relaxed);
  uint64_t ll = atomic_load_explicit(&nc_link_largepage, memory_order_relaxed);
  uint64_t lb = atomic_load_explicit(&nc_link_noblock, memory_order_relaxed);

  uint64_t ht = atomic_load_explicit(&nc_hash_total, memory_order_relaxed);
  uint64_t hm = atomic_load_explicit(&nc_hash_match, memory_order_relaxed);
  uint64_t hx = atomic_load_explicit(&nc_hash_mismatch, memory_order_relaxed);

  uint64_t hpt =
      atomic_load_explicit(&nc_hash_hotpage_total, memory_order_relaxed);
  uint64_t hpm =
      atomic_load_explicit(&nc_hash_hotpage_match, memory_order_relaxed);
  uint64_t hpx =
      atomic_load_explicit(&nc_hash_hotpage_mismatch, memory_order_relaxed);

  uint64_t sl = atomic_load_explicit(&nc_src_largepage, memory_order_relaxed);
  uint64_t sn =
      atomic_load_explicit(&nc_src_neverprotect, memory_order_relaxed);
  uint64_t sm = atomic_load_explicit(&nc_src_mmap_shared, memory_order_relaxed);
  uint64_t sp = atomic_load_explicit(&nc_src_pass0_cross, memory_order_relaxed);
  uint64_t sc = atomic_load_explicit(&nc_src_postcomp, memory_order_relaxed);
  uint64_t ss = atomic_load_explicit(&nc_src_selfloop, memory_order_relaxed);

  uint64_t bn =
      atomic_load_explicit(&nc_blocks_created_neverclean, memory_order_relaxed);
  uint64_t bh =
      atomic_load_explicit(&nc_blocks_created_hotpage, memory_order_relaxed);
  uint64_t bc =
      atomic_load_explicit(&nc_blocks_created_clean, memory_order_relaxed);
  uint64_t bl =
      atomic_load_explicit(&nc_blocks_created_largepage, memory_order_relaxed);

  fprintf(stderr,
          "\n========== NEVERCLEAN PROFILE ==========\n"
          "LinkNext dispatch: total=%lu\n"
          "  clean(at=0):      %lu (%.1f%%)\n"
          "  neverclean(at=1): %lu (%.1f%%)\n"
          "  hotpage(at=2):    %lu (%.1f%%)\n"
          "  largepage(at=3):  %lu (%.1f%%)\n"
          "  no-block:         %lu (%.1f%%)\n"
          "\n"
          "Hash validation (always_test==1 or 3):\n"
          "  total:    %lu\n"
          "  match:    %lu (%.2f%%)\n"
          "  mismatch: %lu (%.2f%%)\n"
          "\n"
          "Hash validation (always_test==2, hotpage):\n"
          "  total:    %lu\n"
          "  match:    %lu (%.2f%%)\n"
          "  mismatch: %lu (%.2f%%)\n"
          "\n"
          "NEVERCLEAN source (page-level, cumulative):\n"
          "  large-page mixed code+data: %lu\n"
          "  neverprotectDB() explicit:  %lu\n"
          "  mmap MAP_SHARED+O_RDWR:     %lu\n"
          "  pass0 page crossing:        %lu\n"
          "  post-compilation check:     %lu\n"
          "  self-loop (no ARCH_NOP):    %lu\n"
          "\n"
          "Block creation:\n"
          "  clean:      %lu\n"
          "  neverclean: %lu\n"
          "  hotpage:    %lu\n"
          "  largepage:  %lu\n"
          "========================================\n\n",
          (unsigned long)lt, (unsigned long)lc, lt ? (100.0 * lc / lt) : 0.0,
          (unsigned long)ln, lt ? (100.0 * ln / lt) : 0.0, (unsigned long)lh,
          lt ? (100.0 * lh / lt) : 0.0, (unsigned long)ll,
          lt ? (100.0 * ll / lt) : 0.0, (unsigned long)lb,
          lt ? (100.0 * lb / lt) : 0.0, (unsigned long)ht, (unsigned long)hm,
          ht ? (100.0 * hm / ht) : 0.0, (unsigned long)hx,
          ht ? (100.0 * hx / ht) : 0.0, (unsigned long)hpt, (unsigned long)hpm,
          hpt ? (100.0 * hpm / hpt) : 0.0, (unsigned long)hpx,
          hpt ? (100.0 * hpx / hpt) : 0.0, (unsigned long)sl, (unsigned long)sn,
          (unsigned long)sm, (unsigned long)sp, (unsigned long)sc,
          (unsigned long)ss, (unsigned long)bc, (unsigned long)bn,
          (unsigned long)bh, (unsigned long)bl);
}

#endif // NEVERCLEAN_PROFILE
