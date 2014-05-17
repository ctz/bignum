
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bigmath.h"

unsigned bignum_lt(const bignum *a, const bignum *b)
{
  assert(!bignum_check(a));
  assert(!bignum_check(b));

  /* Check sign */
  if (bignum_sign(a) < bignum_sign(b))
    return 1;
  else if (bignum_sign(a) > bignum_sign(b))
    return 0;

  /* Check sizes */
  size_t sza = bignum_len_bits(a);
  size_t szb = bignum_len_bits(b);
  
  if (sza < szb)
    return 1;
  else if (sza > szb)
    return 0;
  
  /* Now run through word values. */
  for (uint32_t *va = a->vtop, *vb = b->vtop;
       va >= a->v && vb >= b->v;
       va--, vb--)
  {
    uint32_t wa = *va, wb = *vb;
    if (wa < wb)
      return 1;
    else if (wa > wb)
      return 0;
  }

  return 0;
}

unsigned bignum_lte(const bignum *a, const bignum *b)
{
  return bignum_lt(a, b) || bignum_eq(a, b);
}

unsigned bignum_gt(const bignum *a, const bignum *b)
{
  return !bignum_lte(a, b);
}

unsigned bignum_gte(const bignum *a, const bignum *b)
{
  return !bignum_lt(a, b);
}

unsigned bignum_eq(const bignum *a, const bignum *b)
{
  if (bignum_sign(a) != bignum_sign(b))
    return 0;

  if (bignum_len_bits(a) != bignum_len_bits(b))
    return 0;

  for (uint32_t *va = a->vtop, *vb = b->vtop;
       va >= a->v && vb >= b->v;
       va--, vb--)
  {
    if (*va != *vb)
      return 0;
  }

  return 1;
}

unsigned bignum_const_eq(const bignum *a, const bignum *b)
{
  uint32_t neq = (bignum_sign(a) ^ bignum_sign(b));
  neq |= (bignum_len_bits(a) ^ bignum_len_bits(b));

  for (uint32_t *va = a->vtop, *vb = b->vtop;
       va >= a->v && vb >= b->v;
       va--, vb--)
  {
    neq |= (*va ^ *vb);
  }
  
  return !neq;
}
