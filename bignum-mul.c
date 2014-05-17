
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"
#include "handy.h"

error bignum_mul(bignum *r, const bignum *a, const bignum *b)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(!bignum_check(b));

  size_t sza = bignum_len_bits(a);
  size_t szb = bignum_len_bits(b);

  /* Shortcuts? */

  /* x * 0 -> 0 */
  if ((sza == 1 && bignum_eq32(a, 0)) ||
      (szb == 1 && bignum_eq32(b, 0)))
  {
    bignum_set(r, 0);
    return OK;
  }

  /* 1 * b -> b */
  if (sza == 1 && bignum_eq32(a, 1))
    return bignum_dup(r, b);

  /* a * 1 -> a */
  if (szb == 1 && bignum_eq32(b, 1))
    return bignum_dup(r, a);

  if (bignum_capacity_bits(r) < sza + szb)
    return error_bignum_sz;

  /* Ensure a <= b. */
  if (sza > szb)
  {
    SWAP(a, b);
    SWAP(sza, szb);
  }

  /* We cannot alias. */
  assert(r != a && r != b);

  bignum_set(r, 0);
  bignum_cleartop(r, (sza + szb + 31) / 32);

  size_t nb = bignum_len_words(b);
  for (uint32_t *wr = r->v, *wa = a->v, *wb = b->v;
       wa <= a->vtop;
       wa++, wr++)
  {
    bignum_math_mul_accum(wr, wb, nb, *wa);
  }

  unsigned nega = bignum_is_negative(a),
           negb = bignum_is_negative(b);
  if (nega ^ negb)
    r->flags |= BIGNUM_F_NEG;

  bignum_canon(r);
  return OK;
}

error bignum_mulw(bignum *r, const bignum *a, uint32_t b)
{
  if (b == 0)
  {
    bignum_set(r, 0);
    return OK;
  } else if (b == 1) {
    return bignum_dup(r, a);
  }

  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(r != a);

  uint8_t sza = bignum_len_bits(a);
  uint8_t szb = bignum_math_uint32_fls(b);

  if (bignum_capacity_bits(r) < sza + szb)
    return error_bignum_sz;

  size_t words = bignum_len_words(a);
  
  bignum_set(r, 0);
  bignum_cleartop(r, words + 1);
  bignum_math_mul_accum(r->v, a->v, words, b);
  bignum_canon(r);
  return OK;
}

error bignum_mult(bignum *tmp, bignum *r, const bignum *a, const bignum *b)
{
  if (r == a || r == b)
  {
    error err = bignum_mul(tmp, a, b);
    if (err)
      return err;
    return bignum_dup(r, tmp);
  }

  return bignum_mul(r, a, b);
}

error bignum_multw(bignum *tmp, bignum *r, const bignum *a, uint32_t w)
{
  if (r == a)
  {
    error err = bignum_mulw(tmp, a, w);
    if (err)
      return err;
    return bignum_dup(r, tmp);
  }

  return bignum_mulw(r, a, w);
}
