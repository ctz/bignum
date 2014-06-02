
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "handy.h"

/* This is r = a - b. */
error bignum_sub(bignum *r, const bignum *a, const bignum *b)
{
  assert(!bignum_check_mutable(r));

  /*  a -  b -> a - b
   * -a -  b -> -(a + b)
   *  a - -b -> a + b
   * -a - -b -> b - a
   */

  unsigned nega = bignum_is_negative(a),
           negb = bignum_is_negative(b);

  if (nega && negb)
  {
    return bignum_sub_unsigned(r, b, a);
  } else if (nega ^ negb) {
    error err = bignum_add_unsigned(r, a, b);
    if (nega)
      bignum_setsign(r, -1);
    return err;
  }

  return bignum_sub_unsigned(r, a, b);
}

error bignum_sub_unsigned(bignum *r, const bignum *a, const bignum *b)
{
  if (bignum_mag_lt(a, b))
  {
    SWAP(a, b);
    bignum_setsign(r, -1);
  } else {
    bignum_abs(r);
  }

  uint32_t *atop = a->vtop,
           *btop = b->vtop;

  for (uint32_t *rv = r->v, *av = a->v, *bv = b->v, borrow = 0;
       ;
       r->vtop = rv, rv++)
  {
    uint8_t have_a = av <= atop;
    uint8_t have_b = bv <= btop;

    if (!have_a)
      break;
    
    if (rv - r->v >= r->words)
      return error_bignum_sz;

    uint32_t rw = *av++;

    if (borrow)
    {
      borrow = !rw;
      rw--;
    }

    if (have_b)
    {
      uint32_t bw = *bv++;

      if (bw > rw)
        borrow = 1;

      rw -= bw;
    }

    *rv = rw;
  }

  bignum_canon(r);
  return OK;
}

