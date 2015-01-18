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
  error_invalid_string,
  error_div_zero,
  error_no_inverse
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

/** Pre-canned immutable bignums: 0, 1, -1 and 2 ** BIGNUM_BITS. */
extern bignum bignum_0, bignum_1, bignum_neg1, bignum_base;

#define BIGNUM_TMP_SZ(var, words) \
  uint32_t var ## _words[words] = { 0 }; \
  bignum var = { var ## _words, var ## _words, words, 0 }

/** Defines a bignum with identifier var, suitable for providing as a
 *  temporary for functions which need it.
 *
 *  This uses quite a lot of stack, so consider doing it once per thread. */
#define BIGNUM_TMP(var) BIGNUM_TMP_SZ(var, BIGNUM_MAX_WORDS)

/** Sanity check b.
 *
 * Returns an error if the bignum is internally consistent, OK otherwise.
 *
 * Example: assert(!bignum_check(b))
 */
error bignum_check(const bignum *b);

/** Sanity check b, and fail an assert if it is immutable. */
error bignum_check_mutable(const bignum *b);

/** Canonicalise b.  Fails assert if b is immutable.
 *
 * Canonicalisation involves, for example:
 * - Converting negative zeroes to positive.
 * - Adjust vtop down if it points to zero words.
 */
void bignum_canon(bignum *b);

/** Moves b->vtop to b->v + words, zeroing as it goes.
 *
 *  Use this to prepare a bignum for arbitrary writes
 *  to storage, then use bignum_canon afterwards to
 *  adjust vtop back down.
 *
 *  Fails if there isn't enough storage. */
error bignum_cleartop(bignum *b, size_t words);

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

/** Returns the number of words needed to store the magnitude of b. */
size_t bignum_len_words(const bignum *b);

/** Returns the number of bits b is able to store (irrespective
 *  of its current magnitude). */
size_t bignum_capacity_bits(const bignum *b);

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

/** Returns the value of the i-th bit in the bignum.
 *
 *  i = 0 gives the rightmost (LSB) bit.
 *  i = bignum_len_bits(b)-1 gives the leftmost (MSB) bit.
 *
 *  Out of range i (ie >= bignum_len_bits(b)) returns zero.
 */
uint8_t bignum_get_bit(const bignum *b, size_t i);

/** Sets the value of the i-th bit in the bignum to !!v
 *  (in other words, any non-zero value of v results in
 *  a set bit.)
 *
 *  Out of range n returns error_bignum_sz. */
error bignum_set_bit(bignum *b, uint8_t v, size_t i);

/** Returns the value of the [i,i+n) bits from b.
 *  n <= 32.  n = 1 is obviously equivalent to bignum_get_bit.
 *
 *  Bits out of range are zero. */
uint32_t bignum_get_bits(const bignum *b, size_t i, size_t n);

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
int bignum_getsign(const bignum *b);

/** Set b = -abs(b) if sign < 0, else b = abs(b). */
void bignum_setsign(bignum *b, int sign);

/** Returns 1 if b is negative, 0 otherwise. */
static inline unsigned bignum_is_negative(const bignum *b)
{
  return bignum_getsign(b) == -1;
}

/** Returns 1 if b is zero, 0 otherwise. */
unsigned bignum_is_zero(const bignum *b);

/** Returns 1 if b is even, 0 otherwise. */
unsigned bignum_is_even(const bignum *b);

/** Returns 1 if b is odd, 0 otherwise. */
unsigned bignum_is_odd(const bignum *b);

/** Returns 1 if abs(a) == abs(b), 0 otherwise. */
unsigned bignum_mag_eq(const bignum *a, const bignum *b);

/** Returns 1 if a == b, 0 otherwise. */
unsigned bignum_eq(const bignum *a, const bignum *b);

/** Returns 1 if a == b, 0 otherwise. */
unsigned bignum_eq32(const bignum *a, int32_t b);

/** Returns 1 if a == b, 0 otherwise.  In constant time. */
unsigned bignum_const_eq(const bignum *a, const bignum *b);

/** Returns abs(a) < abs(b). */
unsigned bignum_mag_lt(const bignum *a, const bignum *b);

/** Returns abs(a) <= abs(b). */
unsigned bignum_mag_lte(const bignum *a, const bignum *b);

/** Returns abs(a) > abs(b). */
unsigned bignum_mag_gt(const bignum *a, const bignum *b);

/** Returns abs(a) >= abs(b). */
unsigned bignum_mag_gte(const bignum *a, const bignum *b);

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

/** r = a + b, ignoring the sign of a and b.  Hence r is always positive.
 *
 * Any of r, a and b can alias. */
error bignum_add_unsigned(bignum *r, const bignum *a, const bignum *b);

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

/** r = a - b, ignoring sign of a and b.  r is positive if a >= b.
 *
 * Any of r, a and b can alias. */
error bignum_sub_unsigned(bignum *r, const bignum *a, const bignum *b);

/** r = a * b.
 *
 * r MUST NOT alias a or b. */
error bignum_mul(bignum *r, const bignum *a, const bignum *b);

/** r = a * b.
 *
 * r may alias a or b.  tmp must not alias anything else. */
error bignum_mult(bignum *tmp, bignum *r, const bignum *a, const bignum *b);

/** r = a * w.
 *
 * r MUST NOT alias a. */
error bignum_mulw(bignum *r, const bignum *a, uint32_t w);

/** r = a * w.
 *
 * r may alias a.  tmp must not alias anything else. */
error bignum_multw(bignum *tmp, bignum *r, const bignum *a, uint32_t w);

/** Shifts r left by the given number of bits.
 *
 *  Fails with error_bignum_sz if the resulting value is too large. */
error bignum_shl(bignum *r, size_t bits);

/** Shifts r right by the given number of bits.
 *
 *  Does not fail. */
error bignum_shr(bignum *r, size_t bits);

/** r = a ^ 2. */
error bignum_sqr(bignum *r, const bignum *a);

/** Truncates r by removing set bits over the given
 *  threshold.  So r &= (2 ** bits) - 1, or
 *  equivalently r %= 2 ** bits.
 */
error bignum_trunc(bignum *r, size_t bits);

/** r = a / b.
 *
 * r MUST NOT alias a or b.
 * if b is zero, error_div_zero is returned.
 */
error bignum_div(bignum *r, const bignum *a, const bignum *b);

/** r = a mod b.
 *
 *  r may not alias a or b.
 *  if b is zero, error_div_zero is returned.
 */
error bignum_mod(bignum *r, const bignum *a, const bignum *b);

/** q = a / b
 *  r = a mod b
 *
 *  q may not alias r, a or b.
 *  r may not alias q, a or b.
 *  if b is zero, error_div_zero is returned.
 */
error bignum_divmod(bignum *q, bignum *r, const bignum *a, const bignum *b);

/** Return a * b mod p.
 *
 *  Arguments may alias in any combination. */
error bignum_modmul(bignum *r, const bignum *a, const bignum *b, const bignum *p);

/** Return a ^ b mod p.
 *
 *  Arguments may alias in any combination. */
error bignum_modexp(bignum *r, const bignum *a, const bignum *b, const bignum *p);

/** v = gcd(x, y)
 *
 *  Arguments may alias in any combination. */
error bignum_gcd(bignum *v, const bignum *x, const bignum *y);

/** v = gcd(x, y), with ax + by = v.
 *
 *  Arguments may alias in any combination.
 */
error bignum_extended_gcd(bignum *v, bignum *a, bignum *b,
                          const bignum *x, const bignum *y);

/** Finds z such that az mod m = 1.  In other words, find the
 *  multiplicitive inverse of a mod m.
 *
 *  Returns error_no_inverse if gcd(a, m) != 1.
 *
 *  Arguments may alias in any combination. */
error bignum_modinv(bignum *z, const bignum *a, const bignum *m);

#endif
