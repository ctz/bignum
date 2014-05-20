
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "handy.h"
#include "bignum.h"
#include "bignum-math.h"

#include "bignum-str.h"

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

static error find_k(uint32_t *k_out, bignum *tmp, const bignum *w, const bignum *y)
{
  /* TODO: this is dumb as shit. */
  uint32_t kmin = 1, kmax = 0xffffffff;
  uint32_t up = 0x7fffffff, down = up;

  assert(bignum_mag_lte(y, w));

  while (1)
  {
    if (kmin + 1 == kmax)
    {
      *k_out = kmin;
      return OK;
    }

    /* Move kmin up? */
    ER(bignum_mulw(tmp, y, kmin + up));
    if (bignum_mag_lte(tmp, w))
      kmin += up;
    up = (up + 1) / 2;

    /* Move kmax down? */
    ER(bignum_mulw(tmp, y, kmax - down));
    if (bignum_mag_lt(w, tmp))
      kmax -= down;
    down = (down + 1) / 2;
  }
}

/* This is basic schoolboy long multiplication:
 *
 * INPUT: integers x and y
 * OUTPUT: q = floor(x/y), r = x - yq.
 *
 * WITH: B = 2**32-1
 *
 * 1. q <- 0
 * 2. n <- ||x|| - ||y||
 * 3. r <- x
 * 4. for t from n down to 0 do:
 *   1. Find the largest k such that kyB^t <= r < (k+1)yB^t
 *   2. q <- q + kB^t
 *   3. r <- r - kyB^t
 */

error bignum_divmod(bignum *q, bignum *r, const bignum *x, const bignum *y)
{
  BIGNUM_TMP(tmp);

  if (bignum_is_zero(y))
    return error_div_zero;
  
  ER(bignum_dup(r, x));

  if (bignum_mag_lt(x, y))
  {
    /* x < y, so x / y := 0, x mod y := a. */
    bignum_setu(q, 0);
    /* r set to x above. */
    return OK;
  }

  /* Don't consider the top words where y can't divide the top of x. */
  size_t n = bignum_len_words(x) - bignum_len_words(y);

  bignum_set(q, 0);
  ER(bignum_cleartop(q, bignum_len_words(x)));

  for (size_t i = 0; i <= n; i++)
  {
    size_t t = n - i;

    /* Make our window: which is the top i+1 words of x. */
    bignum window = *r;
    window.v += t;
    bignum_abs(&window);
    
    /* If window < y, we can't divide here: expand window downwards. */
    if (bignum_mag_lt(&window, y))
      continue;
   
    /* Calculate the t'th word of q, k such that:
     *   k * y <= r < (k + 1) * y
     */
    ER(find_k(&q->v[t], &tmp, &window, y));

    /* reduce remainder by y * k shifted left into place. */
    ER(bignum_mulw(&tmp, y, q->v[t]));
    ER(bignum_shl(&tmp, t * 32));
    ER(bignum_subl(r, &tmp));
  }

  bignum_canon(q);
  bignum_canon(r);
  return OK;
}
