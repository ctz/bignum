
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-monty.h"
#include "bignum-dbg.h"
#include "handy.h"

static error bignum_modmul_slow(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(!bignum_check(b));
  assert(!bignum_check(p));

  BIGNUM_TMP(tmp);

  ER(bignum_mul(&tmp, a, b));
  return bignum_mod(r, &tmp, p);
}

error bignum_modmul(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
#ifdef WITH_MONTY
  monty_ctx monty;
  if (bignum_monty_setup(p, &monty))
  {
    return bignum_monty_modmul(r, a, b, p, &monty);
  } else {
    return bignum_modmul_slow(r, a, b, p);
  }
#else
  return bignum_modmul_slow(r, a, b, p);
#endif
}

