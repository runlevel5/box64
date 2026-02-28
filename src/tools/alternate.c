#include <stdint.h>

#include "alternate.h"
#include "custommem.h"
#include "khash.h"

// Alternate address handling
KHASH_MAP_INIT_INT64(alternate, void*)
static kh_alternate_t *my_alternates = NULL;

// Bloom filter for fast negative lookups in hasAlternate().
// 256 bits (4 x uint64_t) with 3 hash probes per key.
// The alternate table is small (typically <100 entries) and populated
// only at init/library-load time, so the bloom filter eliminates
// virtually all khash lookups on the hot path (>99.9% are misses).
static uint64_t alternate_bloom[4] = {0, 0, 0, 0};

static inline void bloom_set(uintptr_t key) {
    // Three independent hash probes from different bit ranges of the key.
    // Each selects one of 256 bits (2 bits for word index, 6 bits for bit position).
    uint32_t h1 = (uint32_t)(key ^ (key >> 17));
    uint32_t h2 = (uint32_t)(key ^ (key >> 31));
    uint32_t h3 = (uint32_t)((key >> 11) ^ (key >> 47));
    alternate_bloom[(h1 >> 6) & 3] |= (uint64_t)1 << (h1 & 63);
    alternate_bloom[(h2 >> 6) & 3] |= (uint64_t)1 << (h2 & 63);
    alternate_bloom[(h3 >> 6) & 3] |= (uint64_t)1 << (h3 & 63);
}

static inline int bloom_test(uintptr_t key) {
    uint32_t h1 = (uint32_t)(key ^ (key >> 17));
    uint32_t h2 = (uint32_t)(key ^ (key >> 31));
    uint32_t h3 = (uint32_t)((key >> 11) ^ (key >> 47));
    if(!(alternate_bloom[(h1 >> 6) & 3] & ((uint64_t)1 << (h1 & 63)))) return 0;
    if(!(alternate_bloom[(h2 >> 6) & 3] & ((uint64_t)1 << (h2 & 63)))) return 0;
    if(!(alternate_bloom[(h3 >> 6) & 3] & ((uint64_t)1 << (h3 & 63)))) return 0;
    return 1;  // maybe present — must check khash to confirm
}

int hasAlternate(void* addr) {
    if(!my_alternates)
        return 0;
    if(!bloom_test((uintptr_t)addr))
        return 0;
    khint_t k = kh_get(alternate, my_alternates, (uintptr_t)addr);
    if(k==kh_end(my_alternates))
        return 0;
    return 1;
}

void* getAlternate(void* addr) {
    if(!my_alternates)
        return addr;
    khint_t k = kh_get(alternate, my_alternates, (uintptr_t)addr);
    if(k!=kh_end(my_alternates))
        return kh_value(my_alternates, k);
    return addr;
}
void addAlternate(void* addr, void* alt) {
    if(!my_alternates) {
        my_alternates = kh_init(alternate);
    }
    int ret;
    khint_t k = kh_put(alternate, my_alternates, (uintptr_t)addr, &ret);
    if(!ret)    // already there
        return;
    kh_value(my_alternates, k) = alt;
    bloom_set((uintptr_t)addr);
}

void addCheckAlternate(void* addr, void* alt) {
    if(!hasAlternate(addr))
        addAlternate(addr, alt);
}

void cleanAlternate() {
    if(my_alternates) {
        kh_destroy(alternate, my_alternates);
        my_alternates = NULL;
    }
    alternate_bloom[0] = 0;
    alternate_bloom[1] = 0;
    alternate_bloom[2] = 0;
    alternate_bloom[3] = 0;
}
