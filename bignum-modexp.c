
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
//#define BIGNUM_DEBUG_ENABLED
#include "bignum-dbg.h"
#include "bignum-monty.h"
#include "handy.h"

error bignum_monty_modexp(bignum *A, const bignum *x, const bignum *e, const bignum *m,
                          monty_ctx *monty)
{
  /* 1. x' = Mont(x, R^2 mod m). */
  BIGNUM_TMP(tmp);
  BIGNUM_TMP(x_prime);
  bignum_setu(&tmp, 1);
  ER(bignum_monty_normalise2(&tmp, &tmp, m, monty));
  ER(bignum_monty_modmul_normalised(&x_prime, x, &tmp, m, monty));
  bignum_dump("x~", &x_prime);

  /* A = R mod m. */
  bignum_setu(A, 1);
  ER(bignum_monty_normalise(A, A, m, monty));
  bignum_dump("A", A);

  /* 2. For i from t down to 0 do the following: */
  for (size_t i = bignum_len_bits(e); i != 0; i--)
  {
    /* 2.1 A = Mont(A, A) */
    ER(bignum_monty_modmul_normalised(&tmp, A, A, m, monty));
    ER(bignum_dup(A, &tmp));

    /* 2.2 If ei == 1 then A = Mont(A, x') */
    if (bignum_get_bit(e, i - 1) == 1)
    {
      ER(bignum_monty_modmul_normalised(&tmp, A, &x_prime, m, monty));
      ER(bignum_dup(A, &tmp));
    }
  }

  /* 3. A = Mont(A, 1) */
  bignum_setu(&x_prime, 1);
  ER(bignum_monty_modmul_normalised(&tmp, A, &x_prime, m, monty));
  ER(bignum_dup(A, &tmp));
  return OK;
}

error bignum_slow_modexp(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
  BIGNUM_TMP(S);
  ER(bignum_dup(&S, a));
  bignum_setu(r, 1);
  for (size_t i = 0; i < bignum_len_bits(b); i++)
  {
    if (bignum_get_bit(b, i) == 1)
    {
      ER(bignum_modmul(r, r, &S, p));
    }
    ER(bignum_modmul(&S, &S, &S, p));
  }
  
  return OK;
}

error bignum_modexp(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(!bignum_check(b));
  assert(!bignum_check(p));
  
  monty_ctx monty;
  if (bignum_monty_setup(p, &monty))
    return bignum_monty_modexp(r, a, b, p, &monty);
  else
    return bignum_slow_modexp(r, a, b, p);
}
