
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"
#include "handy.h"

error bignum_modinv(bignum *z, const bignum *a, const bignum *m)
{
  assert(!bignum_check_mutable(z));
  assert(!bignum_check(a));
  assert(!bignum_check(m));

  BIGNUM_TMP(gcd);
  BIGNUM_TMP(y);

  ER(bignum_extended_gcd(&gcd, z, &y, a, m));

  if (!bignum_eq32(&gcd, 1))
    return error_no_inverse;

  if (bignum_is_negative(z))
    ER(bignum_addl(z, m));

  return OK;
}

