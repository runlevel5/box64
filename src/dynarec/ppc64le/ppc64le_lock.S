// PPC64LE lock helper functions
// Atomic read/write operations using Load-and-Reserve / Store-Conditional
// PPC64LE uses: lbarx/stbcx. (byte), lharx/sthcx. (halfword),
//               lwarx/stwcx. (word), ldarx/stdcx. (doubleword)
// All with lwsync/sync barriers for memory ordering.

.text
.align 4

.global ppc64le_lock_read_b
.global ppc64le_lock_write_b
.global ppc64le_lock_read_h
.global ppc64le_lock_write_h
.global ppc64le_lock_read_d
.global ppc64le_lock_write_d
.global ppc64le_lock_read_dd
.global ppc64le_lock_write_dd
.global ppc64le_lock_xchg_dd
.global ppc64le_lock_xchg_d
.global ppc64le_lock_xchg_h
.global ppc64le_lock_xchg_b
.global ppc64le_lock_storeifnull
.global ppc64le_lock_storeifnull_d
.global ppc64le_lock_storeifref
.global ppc64le_lock_storeifref2
.global ppc64le_lock_storeifref_d
.global ppc64le_lock_storeifref2_d
.global ppc64le_lock_decifnot0b
.global ppc64le_lock_storeb
.global ppc64le_lock_incif0
.global ppc64le_lock_decifnot0
.global ppc64le_lock_store
.global ppc64le_lock_store_dd
.global ppc64le_lock_get_b
.global ppc64le_lock_get_d
.global ppc64le_lock_get_dd
.global ppc64le_crc

// ============================================================================
// Byte operations
// ============================================================================
.type ppc64le_lock_read_b, @function
ppc64le_lock_read_b:
    // address is r3, return is r3
    lwsync
    lbarx   3, 0, 3
    blr
.size ppc64le_lock_read_b, .-ppc64le_lock_read_b

.type ppc64le_lock_write_b, @function
ppc64le_lock_write_b:
    // address is r3, value is r4, return is r3 (0=success, 1=fail)
    mr      5, 3
    stbcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31     // extract CR0[EQ]: 1 if success
    xori    3, 3, 1              // invert: 0 = success, 1 = fail
    lwsync
    blr
.size ppc64le_lock_write_b, .-ppc64le_lock_write_b

// ============================================================================
// Halfword operations
// ============================================================================
.type ppc64le_lock_read_h, @function
ppc64le_lock_read_h:
    // address is r3, return is r3
    lwsync
    lharx   3, 0, 3
    blr
.size ppc64le_lock_read_h, .-ppc64le_lock_read_h

.type ppc64le_lock_write_h, @function
ppc64le_lock_write_h:
    // address is r3, value is r4, return is r3
    mr      5, 3
    sthcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31
    xori    3, 3, 1
    lwsync
    blr
.size ppc64le_lock_write_h, .-ppc64le_lock_write_h

// ============================================================================
// Word (32-bit) operations
// ============================================================================
.type ppc64le_lock_read_d, @function
ppc64le_lock_read_d:
    // address is r3, return is r3
    lwsync
    lwarx   3, 0, 3
    blr
.size ppc64le_lock_read_d, .-ppc64le_lock_read_d

.type ppc64le_lock_write_d, @function
ppc64le_lock_write_d:
    // address is r3, value is r4, return is r3
    mr      5, 3
    stwcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31
    xori    3, 3, 1
    lwsync
    blr
.size ppc64le_lock_write_d, .-ppc64le_lock_write_d

// ============================================================================
// Doubleword (64-bit) operations
// ============================================================================
.type ppc64le_lock_read_dd, @function
ppc64le_lock_read_dd:
    // address is r3, return is r3
    lwsync
    ldarx   3, 0, 3
    blr
.size ppc64le_lock_read_dd, .-ppc64le_lock_read_dd

.type ppc64le_lock_write_dd, @function
ppc64le_lock_write_dd:
    // address is r3, value is r4, return is r3
    mr      5, 3
    stdcx.  4, 0, 5
    mfcr    3
    rlwinm  3, 3, 3, 31, 31
    xori    3, 3, 1
    lwsync
    blr
.size ppc64le_lock_write_dd, .-ppc64le_lock_write_dd

// ============================================================================
// Exchange operations (atomic swap)
// ============================================================================
.type ppc64le_lock_xchg_dd, @function
ppc64le_lock_xchg_dd:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    ldarx   5, 0, 3
    stdcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_dd, .-ppc64le_lock_xchg_dd

.type ppc64le_lock_xchg_d, @function
ppc64le_lock_xchg_d:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lwarx   5, 0, 3
    stwcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_d, .-ppc64le_lock_xchg_d

.type ppc64le_lock_xchg_h, @function
ppc64le_lock_xchg_h:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lharx   5, 0, 3
    sthcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_h, .-ppc64le_lock_xchg_h

.type ppc64le_lock_xchg_b, @function
ppc64le_lock_xchg_b:
    // address is r3, value is r4, return old value in r3
    lwsync
1:
    lbarx   5, 0, 3
    stbcx.  4, 0, 3
    bne     1b
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_xchg_b, .-ppc64le_lock_xchg_b

// ============================================================================
// Conditional store operations
// ============================================================================
.type ppc64le_lock_storeifnull, @function
ppc64le_lock_storeifnull:
    // address is r3, value is r4
    // store r4 to [r3] only if [r3] is 0. return old [r3] value
    lwsync
1:
    ldarx   5, 0, 3
    cmpdi   5, 0
    bne     2f
    stdcx.  4, 0, 3
    bne     1b
2:
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_storeifnull, .-ppc64le_lock_storeifnull

.type ppc64le_lock_storeifnull_d, @function
ppc64le_lock_storeifnull_d:
    // address is r3, value is r4
    // store r4 (word) to [r3] only if [r3] is 0. return old [r3] value
    lwsync
1:
    lwarx   5, 0, 3
    cmpwi   5, 0
    bne     2f
    stwcx.  4, 0, 3
    bne     1b
2:
    lwsync
    mr      3, 5
    blr
.size ppc64le_lock_storeifnull_d, .-ppc64le_lock_storeifnull_d

.type ppc64le_lock_storeifref, @function
ppc64le_lock_storeifref:
    // address is r3, value is r4, ref is r5
    // store r4 to [r3] only if [r3] == r5. return new [r3] value (r4 or old)
    lwsync
1:
    ldarx   6, 0, 3
    cmpd    5, 6
    bne     2f
    stdcx.  4, 0, 3
    bne     1b
    mr      3, 4
    blr
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref, .-ppc64le_lock_storeifref

.type ppc64le_lock_storeifref2, @function
ppc64le_lock_storeifref2:
    // address is r3, value is r4, ref is r5
    // store r4 to [r3] only if [r3] == r5. return old [r3] value
    lwsync
1:
    ldarx   6, 0, 3
    cmpd    5, 6
    bne     2f
    stdcx.  4, 0, 3
    bne     1b
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref2, .-ppc64le_lock_storeifref2

.type ppc64le_lock_storeifref_d, @function
ppc64le_lock_storeifref_d:
    // address is r3, value is r4 (word), ref is r5 (word)
    // store r4 to [r3] only if [r3] == r5. return new [r3] value
    lwsync
1:
    lwarx   6, 0, 3
    cmpw    5, 6
    bne     2f
    stwcx.  4, 0, 3
    bne     1b
    mr      3, 4
    blr
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref_d, .-ppc64le_lock_storeifref_d

.type ppc64le_lock_storeifref2_d, @function
ppc64le_lock_storeifref2_d:
    // address is r3, value is r4 (word), ref is r5 (word)
    // store r4 to [r3] only if [r3] == r5. return old [r3] value
    lwsync
1:
    lwarx   6, 0, 3
    cmpw    5, 6
    bne     2f
    stwcx.  4, 0, 3
    bne     1b
2:
    mr      3, 6
    blr
.size ppc64le_lock_storeifref2_d, .-ppc64le_lock_storeifref2_d

// ============================================================================
// Misc lock operations
// ============================================================================
.type ppc64le_lock_decifnot0b, @function
ppc64le_lock_decifnot0b:
    // address is r3, decrement byte at [r3] if not 0
    lwsync
1:
    lbarx   4, 0, 3
    cmpwi   4, 0
    beq     2f
    addi    4, 4, -1
    stbcx.  4, 0, 3
    bne     1b
2:
    blr
.size ppc64le_lock_decifnot0b, .-ppc64le_lock_decifnot0b

.type ppc64le_lock_storeb, @function
ppc64le_lock_storeb:
    // address is r3, value is r4
    stb     4, 0(3)
    lwsync
    blr
.size ppc64le_lock_storeb, .-ppc64le_lock_storeb

.type ppc64le_lock_decifnot0, @function
ppc64le_lock_decifnot0:
    // address is r3, return old value in r3
    lwsync
1:
    lwarx   4, 0, 3
    cmpwi   4, 0
    beq     2f
    addi    5, 4, -1
    stwcx.  5, 0, 3
    bne     1b
2:
    mr      3, 4
    blr
.size ppc64le_lock_decifnot0, .-ppc64le_lock_decifnot0

.type ppc64le_lock_incif0, @function
ppc64le_lock_incif0:
    // address is r3, increment word at [r3] if 0. return old value in r3
    lwsync
1:
    lwarx   4, 0, 3
    cmpwi   4, 0
    bne     2f
    addi    5, 4, 1
    stwcx.  5, 0, 3
    bne     1b
2:
    mr      3, 4
    blr
.size ppc64le_lock_incif0, .-ppc64le_lock_incif0

.type ppc64le_lock_store, @function
ppc64le_lock_store:
    // address is r3, value is r4 (word)
    stw     4, 0(3)
    lwsync
    blr
.size ppc64le_lock_store, .-ppc64le_lock_store

.type ppc64le_lock_store_dd, @function
ppc64le_lock_store_dd:
    // address is r3, value is r4 (doubleword)
    std     4, 0(3)
    lwsync
    blr
.size ppc64le_lock_store_dd, .-ppc64le_lock_store_dd

.type ppc64le_lock_get_b, @function
ppc64le_lock_get_b:
    // address is r3, return byte in r3
    lbarx   3, 0, 3
    blr
.size ppc64le_lock_get_b, .-ppc64le_lock_get_b

.type ppc64le_lock_get_d, @function
ppc64le_lock_get_d:
    // address is r3, return word in r3
    lwarx   3, 0, 3
    blr
.size ppc64le_lock_get_d, .-ppc64le_lock_get_d

.type ppc64le_lock_get_dd, @function
ppc64le_lock_get_dd:
    // address is r3, return doubleword in r3
    ldarx   3, 0, 3
    blr
.size ppc64le_lock_get_dd, .-ppc64le_lock_get_dd

// ============================================================================
// CRC32 — software implementation (PPC64LE has no hardware CRC32 instruction)
// Uses polynomial 0x1EDC6F41 (CRC-32C / Castagnoli, matches x86 CRC32)
// ============================================================================
.type ppc64le_crc, @function
ppc64le_crc:
    // r3 = address, r4 = length (bytes)
    // return CRC in r3
    mr      5, 3                // address in r5
    li      3, 0                // crc = 0

    // Simple byte-at-a-time CRC32C for now
    // TODO: Optimize with table-based or VPMSUMW approach
    cmpwi   4, 0
    beq     .Lcrc_done

.Lcrc_byte_loop:
    lbz     6, 0(5)             // load byte
    xor     3, 3, 6             // crc ^= byte
    // Process 8 bits
    li      7, 8
.Lcrc_bit_loop:
    andi.   8, 3, 1             // test bit 0
    srwi    3, 3, 1             // crc >>= 1
    beq     .Lcrc_no_xor
    // XOR with polynomial 0x82F63B78 (bit-reversed 0x1EDC6F41)
    lis     8, 0x82F6
    ori     8, 8, 0x3B78
    xor     3, 3, 8
.Lcrc_no_xor:
    addi    7, 7, -1
    cmpwi   7, 0
    bne     .Lcrc_bit_loop

    addi    5, 5, 1
    addi    4, 4, -1
    cmpwi   4, 0
    bne     .Lcrc_byte_loop

.Lcrc_done:
    blr
.size ppc64le_crc, .-ppc64le_crc
