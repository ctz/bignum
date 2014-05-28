
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"
#include "handy.h"

error bignum_modexp(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(!bignum_check(b));
  assert(!bignum_check(p));

  bignum_setu(r, 1);

  for (size_t i = bignum_len_bits(b); i > 0; i--)
  {
    ER(bignum_modmul(r, r, r, p));
    if (bignum_get_bit(b, i - 1))
      ER(bignum_modmul(r, r, a, p));
  }
  return OK;
}

