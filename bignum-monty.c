
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-monty.h"
#include "bignum-dbg.h"
#include "handy.h"

static uint32_t word(const bignum *x, size_t i)
{
  if (i >= x->words)
    return 0;
  else
    return x->v[i];
}

/* Tricksy computation of -1/n mod 2 ** 32. */
static uint32_t modinv_u32(uint32_t n)
{
  assert(n & 1); /* must be odd */

  /* Obviously we could just use bignum_modinv with bignum_base,
   * but this trick is way quicker.
   *
   * Cribbed from Tom St Denis' tommath. */
  uint32_t r = (((n + 2) & 4) << 1) + n;
  r *= 2 - n * r;
  r *= 2 - n * r;
  r *= 2 - n * r;
  
  /* Now we have r = 1/n mod 2 ** 32. */
  r ^= 0xfffffffe;
  return r;
}

/* Fills in *mont and returns 1 if montgomery reduction will work. */
unsigned bignum_monty_setup(const bignum *m, monty_ctx *mont)
{
  if (bignum_is_odd(m))
  {
    /* R = b^n, where n is the number of digits in modulus. */
    size_t n = bignum_len_words(m);
    mont->R_shift = n * BIGNUM_BITS;
    
    /* m' = -m ^ -1 mod b. */
    mont->mprime = modinv_u32(m->v[0]);
    return 1;
  }

  return 0;
}
  
error monty_normalise_n(bignum *xR, const bignum *x, const bignum *m, size_t R_shift)
{
  BIGNUM_TMP(tmp);
  ER(bignum_dup(&tmp, x));
  ER(bignum_shl(&tmp, R_shift));
  ER(bignum_mod(xR, &tmp, m));
  return OK;
}

error bignum_monty_normalise2(bignum *xR, const bignum *x, const bignum *m, const monty_ctx *monty)
{
  return monty_normalise_n(xR, x, m, monty->R_shift * 2);
}

/* Sets xR = xR mod m. */
error bignum_monty_normalise(bignum *xR, const bignum *x, const bignum *m, const monty_ctx *monty)
{
  return monty_normalise_n(xR, x, m, monty->R_shift);
}

static error bignum_monty_modmul_normalised_reduce(bignum *A, const bignum *x, const bignum *y, const bignum *m,
                                     const monty_ctx *monty)
{
  BIGNUM_TMP(rx);
  BIGNUM_TMP(ry);
  ER(bignum_mod(&rx, x, m));
  ER(bignum_mod(&ry, y, m));
  return bignum_monty_modmul_normalised(A, &rx, &ry, m, monty);
}

error bignum_monty_modmul_normalised(bignum *A, const bignum *x, const bignum *y, const bignum *m,
                                     const monty_ctx *monty)
{
  assert(!bignum_check_mutable(A));
  assert(!bignum_check(x));
  assert(!bignum_check(y));
  assert(!bignum_check(m));
  
  bignum_dump("  x", x);
  bignum_dump("  y", y);
  bignum_dump("  m", m);


  if (!(bignum_gte(x, &bignum_0) && bignum_lt(x, m)) ||
      !(bignum_gte(y, &bignum_0) && bignum_lt(y, m)))
    return bignum_monty_modmul_normalised_reduce(A, x, y, m, monty);

  assert(A != x && A != y);

  size_t n = bignum_len_words(m);

  /* 1. A <- 0. */
  bignum_setu(A, 0);

  BIGNUM_TMP(tmp);

  /* 2. For i from 0 to (n - 1): */
  for (size_t i = 0; i < n; i++)
  {
    /* u_i <- (a_0 + x_i * y_0) m' mod b. */
    uint32_t u = (word(A, 0) + word(x, i) * word(y, 0)) * monty->mprime;

    /* A <- (A + x_i * y + u_i * m) / b. */

    /* + x_i * y */
    ER(bignum_mulw(&tmp, y, word(x, i)));
    ER(bignum_addl(A, &tmp));

    /* + u_i * m */
    ER(bignum_mulw(&tmp, m, u));
    ER(bignum_addl(A, &tmp));

    /* / b */
    ER(bignum_shr(A, BIGNUM_BITS));
  }

  /* 3. If A >= m then A <- A - m. */
  if (bignum_gte(A, m))
    ER(bignum_subl(A, m));

  bignum_dump("  result-m", A);
  return OK;
}

static error bignum_monty_reduce(bignum *A, const bignum *T, const bignum *m,
                                 const monty_ctx *monty)
{
  size_t n = bignum_len_words(m);

  BIGNUM_TMP(tmp);

  /* 1. A <- T */
  ER(bignum_dup(A, T));

  /* 2. For i from 0 to (n - 1) do the following: */
  for (size_t i = 0; i < n; i++)
  {
    /* 2.1 u_i <- a_i m' mod b */
    uint32_t u = word(A, i) * monty->mprime;

    /* 2.2 A <- A + u_i m b^i */
    ER(bignum_mulw(&tmp, m, u));
    ER(bignum_shl(&tmp, i * BIGNUM_BITS));
    ER(bignum_addl(A, &tmp));
  }

  /* 3. A <- A / b^n */
  bignum_dump("  A-pre-div", A);
  ER(bignum_shr(A, monty->R_shift));
  
  /* 4. If A >= m then A <- A - m. */
  if (bignum_gte(A, m))
    ER(bignum_subl(A, m));

  return OK;
}

static error bignum_monty_modmul_noalias(bignum *A, const bignum *x, const bignum *y, const bignum *m,
                                         const monty_ctx *monty)
{
  /* Make x < y. */
  if (!bignum_lt(x, y))
    SWAP(x, y);

  BIGNUM_TMP(xR);
  ER(bignum_monty_normalise(&xR, x, m, monty));
  return bignum_monty_modmul_normalised(A, &xR, y, m, monty);
}

static error bignum_monty_modmul_alias(bignum *r, const bignum *a, const bignum *b, const bignum *p,
                                       const monty_ctx *monty)
{
  BIGNUM_TMP(rt);
  ER(bignum_monty_modmul_noalias(&rt, a, b, p, monty));
  ER(bignum_dup(r, &rt));
  return OK;
}

error bignum_monty_modmul(bignum *A, const bignum *x, const bignum *y, const bignum *m,
                          const monty_ctx *monty)
{
  if (A == x || A == y || A == m)
    return bignum_monty_modmul_alias(A, x, y, m, monty);
  else
    return bignum_monty_modmul_noalias(A, x, y, m, monty);
}

error bignum_monty_sqr_normalised(bignum *A, const bignum *x, const bignum *m,
                                  const monty_ctx *monty)
{
  BIGNUM_TMP(r);
  ER(bignum_monty_modmul_normalised(&r, x, x, m, monty));
  return bignum_dup(A, &r);
  (void) bignum_monty_reduce;
}
