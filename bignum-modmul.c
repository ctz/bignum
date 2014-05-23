
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"
#include "handy.h"

error bignum_modmul(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(!bignum_check(b));
  assert(!bignum_check(p));

  BIGNUM_TMP(tmp);

  ER(bignum_mul(&tmp, a, b));
  return bignum_mod(r, &tmp, p);
}

