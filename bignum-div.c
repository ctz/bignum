
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"

error bignum_mod(bignum *r, const bignum *a, const bignum *b)
{
  BIGNUM_TMP(q_tmp);
  return bignum_divmod(&q_tmp, r, a, b);
}

error bignum_div(bignum *q, const bignum *a, const bignum *b)
{
  BIGNUM_TMP(r_tmp);
  return bignum_divmod(q, &r_tmp, a, b);
}

error bignum_divmod(bignum *q, bignum *r, const bignum *a, const bignum *b)
{
  if (bignum_is_zero(b))
    return error_div_zero;

  if (bignum_lt(a, b))
  {
    /* a < b, so a / b = 0, a mod b = a. */
    bignum_setu(q, 0);
    return bignum_dup(r, a);
  }

  return error_invalid_bignum;
}
