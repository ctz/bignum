#ifndef BIGMATH_H
#define BIGMATH_H

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
void bigmath_mul_accum(uint32_t *r, uint32_t *a, size_t words, uint32_t m);

#endif
