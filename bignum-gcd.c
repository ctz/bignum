
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "handy.h"

error bignum_gcd(bignum *v, const bignum *fx, const bignum *fy)
{
  assert(!bignum_check_mutable(v));
  assert(!bignum_check(fx));
  assert(!bignum_check(fy));

  BIGNUM_TMP(x);
  BIGNUM_TMP(y);

  ER(bignum_dup(&x, fx));
  ER(bignum_dup(&y, fy));

  bignum_abs(&x);
  bignum_abs(&y);

  if (bignum_lt(&x, &y))
    SWAP(x, y);

  /* This is HAC Algorithm 14.54.
   *
   * We don't store g, but instead count the number of
   * doublings we did of it. */

  /* 1. g <- 1. */
  size_t g_shifts = 0;

  /* 2. While both x and y are even do the following:
   *    x <- x / 2
   *    y <- y / 2
   *    g <- 2g
   */
  while (bignum_get_bit(&x, 0) == 0 && bignum_get_bit(&y, 0) == 0)
  {
    ER(bignum_shr(&x, 1));
    ER(bignum_shr(&y, 1));
    g_shifts++;
  }

  /* 3. While x != 0 do the following: */
  while (!bignum_eq32(&x, 0))
  {
    /* 3.1. While x is even do x <- x / 2. */
    while (bignum_get_bit(&x, 0) == 0)
      ER(bignum_shr(&x, 1));

    /* 3.2. While y is even do y <- y / 2. */
    while (bignum_get_bit(&y, 0) == 0)
      ER(bignum_shr(&y, 1));

    /* 3.3. t <- abs(x - y) / 2. */
    /* 3.4. If x >= y then x <- t; otherwise, y <- t. */
    if (bignum_gte(&x, &y))
    {
      ER(bignum_sub(&x, &x, &y));
      bignum_abs(&x);
      ER(bignum_shr(&x, 1));
    } else {
      ER(bignum_sub(&y, &x, &y));
      bignum_abs(&y);
      ER(bignum_shr(&y, 1));
    }
  }

  /* 4. Return (g . y).
   *
   * nb. we use x as a temporary here. */
  ER(bignum_dup(v, &y));
  return bignum_shl(v, g_shifts);
}

error bignum_extended_gcd(bignum *v, bignum *a, bignum *b,
                          const bignum *fx, const bignum *fy)
{
  assert(!bignum_check_mutable(v));
  assert(!bignum_check_mutable(a));
  assert(!bignum_check_mutable(b));
  assert(!bignum_check(fx));
  assert(!bignum_check(fy));

  BIGNUM_TMP(x);
  BIGNUM_TMP(y);

  ER(bignum_dup(&x, fx));
  ER(bignum_dup(&y, fy));

  bignum_abs(&x);
  bignum_abs(&y);

  /* This is HAC algorithm 14.61.
   *
   * See notes above about g. */

  /* 1. g <- 1. */
  size_t g_shifts = 0;
  
  /* 2. While both x and y are even do the following:
   *    x <- x / 2
   *    y <- y / 2
   *    g <- 2g
   */
  while (bignum_get_bit(&x, 0) == 0 && bignum_get_bit(&y, 0) == 0)
  {
    ER(bignum_shr(&x, 1));
    ER(bignum_shr(&y, 1));
    g_shifts++;
  }

  /* 3. u <- x
   *    v <- y
   *    A <- 1
   *    B <- 0
   *    C <- 0
   *    D <- 1
   */
  BIGNUM_TMP(tmpu);
  BIGNUM_TMP(tmpA);
  BIGNUM_TMP(tmpB);

  bignum *A = &tmpA,
         *B = &tmpB,
         *C = a,
         *D = b,
         *u = &tmpu;

  ER(bignum_dup(u, &x));
  ER(bignum_dup(v, &y));
  bignum_setu(A, 1);
  bignum_setu(B, 0);
  bignum_setu(C, 0);
  bignum_setu(D, 1);

step_4:
  /* 4. While u is even do the following: */
  while (bignum_get_bit(u, 0) == 0)
  {
    /* 4.1 u <- u / 2. */
    ER(bignum_shr(u, 1));

    /* 4.2 If A = B = 0 (mod 2)
     *     then
     *       A <- A / 2
     *       B <- B / 2
     *     else
     *       A <- (A + y) / 2
     *       B <- (B - x) / 2
     *
     */
    if (bignum_get_bit(A, 0) == 0 && bignum_get_bit(B, 0) == 0)
    {
      ER(bignum_shr(A, 1));
      ER(bignum_shr(B, 1));
    } else {
      ER(bignum_addl(A, &y));
      ER(bignum_shr(A, 1));

      ER(bignum_subl(B, &x));
      ER(bignum_shr(B, 1));
    }
  }

  /* 5. While v is even do the following: */
  while (bignum_get_bit(v, 0) == 0)
  {
    /* 5.1 v <- v / 2. */
    ER(bignum_shr(v, 1));

    /* 5.2 If C = D = 0 (mod 2)
     *     then
     *       C <- C / 2
     *       D <- D / 2
     *     else
     *       C <- (C + y) / 2
     *       D <- (D - x) / 2
     */
    if (bignum_get_bit(C, 0) == 0 && bignum_get_bit(D, 0) == 0)
    {
      ER(bignum_shr(C, 1));
      ER(bignum_shr(D, 1));
    } else {
      ER(bignum_addl(C, &y));
      ER(bignum_shr(C, 1));

      ER(bignum_subl(D, &x));
      ER(bignum_shr(D, 1));
    }
  }

  /* 6. If u >= v
   *    then
   *      u <- u - v
   *      A <- A - C
   *      B <- B - D
   *    else
   *      v <- v - u
   *      C <- C - A
   *      D <- D - B
   */
  if (bignum_gte(u, v))
  {
    ER(bignum_subl(u, v));
    ER(bignum_subl(A, C));
    ER(bignum_subl(B, D));
  } else {
    ER(bignum_subl(v, u));
    ER(bignum_subl(C, A));
    ER(bignum_subl(D, B));
  }

  /* 7. If u = 0
   *    then
   *      a <- C
   *      b <- D
   *      return (a, b, g * v)
   *    else
   *      goto step 4.
   */
  if (!bignum_eq32(u, 0))
    goto step_4;
 
  /* nb. we already arranged that a aliases C, and b aliases D. */
  ER(bignum_shl(v, g_shifts));
  return OK;
}
