
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"

/* TODO: make this twice as fast by memoising the
 * intermiediate products. */
error bignum_sqr(bignum *r, const bignum *a)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));

  return bignum_mul(r, a, a);
}

