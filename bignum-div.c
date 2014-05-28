
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

static uint32_t div64_32(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo)
{
  uint64_t x = ((uint64_t) xhi) << 32 | xlo;
  uint64_t y = ((uint64_t) yhi) << 32 | ylo;
  return x / y;
}

/* Sets *res = -1 if candidate * y <= w,
 *      *res = 1 if candidate * y > w. */
static error check_k(bignum *tmp, const bignum *w, const bignum *y, uint32_t candidate, int *res)
{
  ER(bignum_mulw(tmp, y, candidate));
  *res = bignum_mag_lte(tmp, w) ? -1 : 1;
  return OK;
}

static uint32_t div_top(const bignum *x, const bignum *y)
{
  /* Divides the top 64 bits (if available) of x by y. */
  size_t x_words = bignum_len_words(x);
  size_t y_words = bignum_len_words(y);

  if (x_words == y_words)
  {
    if (x_words == 1)
      return x->vtop[0] / y->vtop[0];
    else
      return div64_32(x->vtop[0], x->vtop[-1],
                      y->vtop[0], y->vtop[-1]);
  } else if (x_words > y_words) {
    return div64_32(x->vtop[0], x->vtop[-1],
                    0, y->vtop[0]);
  } else {
    return div64_32(0, x->vtop[0],
                    y->vtop[0], y->vtop[-1]);
  }
}

static error find_k(uint32_t *k_out, bignum *tmp, const bignum *w, const bignum *y)
{
  assert(bignum_mag_lte(y, w));

  /* Make an initial guess by dividing the top of w by y.
   * This might be an overestimate. */
  uint32_t guess = div_top(w, y);

  while (1)
  {
    int ltw, gtw;
    ER(check_k(tmp, w, y, guess, &ltw));
    ER(check_k(tmp, w, y, guess + 1, &gtw));
    if (ltw == -1 && gtw == 1)
    {
      *k_out = guess;
      return OK;
    }

    guess--;
  }

#if 0
  DELETEME
  /* Fall-back to dumb-as-shit binary search. */
  uint32_t kmin = 1, kmax = 0xffffffff;
  uint32_t up = 0x7fffffff, down = up;

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
#endif
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
  BIGNUM_TMP(yn);

  if (bignum_is_zero(y))
    return error_div_zero;
  
  if (bignum_mag_lt(x, y))
  {
    /* x < y, so x / y := 0, x mod y := a. */
    bignum_setu(q, 0);
    return bignum_dup(r, x);
  }
    
  ER(bignum_dup(r, x));
  ER(bignum_dup(&yn, y));

  /* Normalise x and y in r and yn respectively, to provide best results
   * when doing quotient estimation in find_k. */
  size_t y_topword_bits = bignum_len_bits(y) % BIGNUM_BITS;
  size_t norm_shift = 0;

  /* We want to make y >= 2**(32-1). */
  size_t norm_target_bits = BIGNUM_BITS - 1;
  if (y_topword_bits < norm_target_bits)
  {
    norm_shift = norm_target_bits - y_topword_bits;
    ER(bignum_shl(r, norm_shift));
    ER(bignum_shl(&yn, norm_shift));
  }

  /* Don't consider the top words where y can't divide the top of x. */
  size_t n = bignum_len_words(r) - bignum_len_words(&yn);

  bignum_set(q, 0);
  ER(bignum_cleartop(q, bignum_len_words(r)));

  for (size_t i = 0; i <= n; i++)
  {
    size_t t = n - i;

    /* Make our window: which is the top i+1 words of x. */
    bignum window = *r;
    window.v += t;
    bignum_abs(&window);
    
    /* If window < yn, we can't divide here: expand window downwards. */
    if (bignum_mag_lt(&window, &yn))
      continue;
   
    /* Calculate the t'th word of q, k such that:
     *   k * yn <= r < (k + 1) * yn
     */
    ER(find_k(&q->v[t], &tmp, &window, &yn));

    /* reduce remainder by yn * k shifted left into place. */
    ER(bignum_mulw(&tmp, &yn, q->v[t]));
    ER(bignum_shl(&tmp, t * 32));
    ER(bignum_subl(r, &tmp));
  }
  
  bignum_canon(q);
  bignum_canon(r);

  /* Denormalise remainder. */
  if (norm_shift)
    ER(bignum_shr(r, norm_shift));

  return OK;
}
