#ifndef BIGNUM_H
#define BIGNUM_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
  OK = 0,
  error_invalid_bignum,
  error_buffer_sz,
  error_bignum_sz,
  error_invalid_string
} error;

#define BIGNUM_BYTES 4
#define BIGNUM_BITS 32
#define BITS_TO_BYTES(bits) (((bits) + 7) >> 3)
#define BYTES_TO_BITS(bytes) ((bytes) << 3)

/** We have a fairly arbitrary limit on the size of bignum we'll
 *  process, for sanity checking rather than because of any
 *  underlying limitation.
 *
 *  In fact, the structure can handle up to 0xffff word-bignums
 *  so, 2-million-odd bits.
 */
#define BIGNUM_MAX_WORDS 256

/**
 * Arbitrary sized integer type.
 *
 * This type stores everything in a vector of 32-bit words.
 */
typedef struct
{
  /** Magnitude:
   *  LSW first vector of words, with possibly trailing zeroes. */
  uint32_t *v;
  
  /** MSW of v. vtop - v < words. */
  uint32_t *vtop;

  /** The number of words available for use pointed to by v. */
  uint16_t words;

  /** Number is negative. Negative zeros are allowed, but are not canonical. */
#define BIGNUM_F_NEG        0x0001
  /** Number is immutable.  Trying to mutate this number will cause an assert to fail. */
#define BIGNUM_F_IMMUTABLE  0x0002
  /** All valid flags. */
#define BIGNUM_F__ALL       0x0003
  uint16_t flags;
} bignum;

/** Pre-canned immutable bignums. */
extern bignum bignum_0, bignum_1, bignum_neg1;

/** Sanity check b.
 *
 * Returns an error if the bignum is internally consistent, OK otherwise.
 *
 * Example: assert(!bignum_check(b))
 */
error bignum_check(const bignum *b);

/** Canonicalise b.  Fails assert if b is immutable.
 *
 * Canonicalisation involves, for example:
 * - Converting negative zeroes to positive.
 * - Adjust vtop down if it points to zero words.
 */
void bignum_canon(bignum *b);

/** Zeroes all digits, and leave structure b invalid. */
void bignum_clear(bignum *b);

/** Copies the value of a into r. */
error bignum_dup(bignum *r, const bignum *a);

/** Returns the number of bits needed to store the magnitude of b
 *  in binary.
 *  Zero needs 1 bit. */
size_t bignum_len_bits(const bignum *b);

/** Returns the number of bytes needed to store the magnitude of b
 *  in binary. */
size_t bignum_len_bytes(const bignum *b);

/** Returns the value of the nth byte in the bignum.
 *
 *  n = 0 gives the rightmost (LSB) byte.
 *  n = bignum_len_bytes(b)-1 gives the leftmost (MSB) byte.
 *  
 *  Out of range n (ie >= bignum_len_bytes(b)) returns zero.
 */
uint8_t bignum_get_byte(const bignum *b, size_t n);

/** Sets the value of the nth byte in the bignum to v.
 *
 *  Same semantics for n as bignum_get_byte.
 *
 *  Out of range n returns error_bignum_sz.
 */
error bignum_set_byte(bignum *b, uint8_t v, size_t n);

/** Set b to have value l.
 *
 * This cannot fail, because even zero needs one word available.
 * However, as normal it will fail an assert if b is immutable. */
void bignum_set(bignum *b, int32_t v);

/** As bignum_set, but with an unsigned value.  The result is
 *  positive. */
void bignum_setu(bignum *b, uint32_t v);

/** b = -b. */
void bignum_neg(bignum *b);

/** b = abs(b). */
void bignum_abs(bignum *b);

/** (b < 0) ? -1 : 1 */
int bignum_sign(const bignum *b);

/** Returns 1 if b is negative, 0 otherwise. */
static inline unsigned bignum_is_negative(const bignum *b)
{
  return bignum_sign(b) == -1;
}

/** Returns 1 if a == b, 0 otherwise. */
unsigned bignum_eq(const bignum *a, const bignum *b);

/** Returns 1 if a == b, 0 otherwise.  In constant time. */
unsigned bignum_const_eq(const bignum *a, const bignum *b);

/** Returns a < b. */
unsigned bignum_lt(const bignum *a, const bignum *b);

/** Returns a <= b. */
unsigned bignum_lte(const bignum *a, const bignum *b);

/** Returns a > b. */
unsigned bignum_gt(const bignum *a, const bignum *b);

/** Returns a >= b. */
unsigned bignum_gte(const bignum *a, const bignum *b);

/** r = a + b.
 *
 * Any of r, a and b can alias. */
error bignum_add(bignum *r, const bignum *a, const bignum *b);

/** a += b.
 *
 * a and b can alias. */
static inline error bignum_addl(bignum *a, const bignum *b)
{
  return bignum_add(a, a, b);
}

/** r = a - b.
 *
 * Any of r, a and b can alias. */
error bignum_sub(bignum *r, const bignum *a, const bignum *b);

/** a -= b
 *
 * a and b can alias. */
static inline error bignum_subl(bignum *a, const bignum *b)
{
  return bignum_sub(a, a, b);
}

/** r = a * b.
 *
 * Any of r, a and b can alias. */
error bignum_mul(bignum *r, const bignum *a, const bignum *b);

/** a *= b;
 *
 * a and b can alias. */
static inline error bignum_mull(bignum *a, const bignum *b)
{
  return bignum_mul(a, a, b);
}

#endif
