
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"

static error bignum_sqr_tmp(bignum *r, const bignum *a)
{
  BIGNUM_TMP(tmp);
  return bignum_mult(&tmp, r, a, a);
}

/* TODO: make this twice as fast by memoising the
 * intermediate products. */
error bignum_sqr(bignum *r, const bignum *a)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));

  if (r == a)
    return bignum_sqr_tmp(r, a);
  else
    return bignum_mul(r, a, a);
}

