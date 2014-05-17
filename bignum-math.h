#ifndef BIGNUM_MATH_H
#define BIGNUM_MATH_H

/*
 * Bignum maths functions.
 *
 * These are straightforward low-level functions (like
 * multiply-and-accumulate) which are usefully implemented
 * in assembly.
 */

#include <stddef.h>
#include <stdint.h>
#include "bignum.h"

/** Multiply a by m, adding the result to r.
 *  r and a have w words. */
void bignum_math_mul_accum(uint32_t *r, uint32_t *a, size_t words, uint32_t m);

/** Returns the index of the top set bit of w.
 *
 *  Returns 0 if w is 0, 32 if w is 0xffffffff, 1 if w is 1,
 *  2 if w is 3, etc. */
uint8_t bignum_math_uint32_fls(uint32_t w);

#endif
