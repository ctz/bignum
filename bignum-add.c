
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "handy.h"

error bignum_add(bignum *r, const bignum *a, const bignum *b)
{
  assert(!bignum_check_mutable(r));

  /* Sort out signs:
   *
   *  a +  b ->   a + b
   * -a +  b ->   b - a
   *  a + -b ->   a - b
   * -a + -b -> -(a + b)
   */

  unsigned nega = bignum_is_negative(a),
           negb = bignum_is_negative(b);
  if (nega && negb)
  {
    /* -a + -b case. */
    bignum_setsign(r, -1);
  } else if (nega ^ negb) {
    /* -a + b and a + -b cases. */
    if (nega)
    {
      /* Convert -a + b to b + -a. */
      SWAP(a, b);
    }

    unsigned result_negative = bignum_mag_lt(a, b);

    error e = bignum_sub_unsigned(r, a, b);
    bignum_setsign(r, result_negative ? -1 : 1);
    return e;
  } else {
    /* a + b case. */
    bignum_setsign(r, 1);
  }

  return bignum_add_unsigned(r, a, b);
}

error bignum_add_unsigned(bignum *r, const bignum *a, const bignum *b)
{
  assert(!bignum_check_mutable(r));

  uint32_t *atop = a->vtop;
  uint32_t *btop = b->vtop;

  for (uint32_t *rv = r->v, *av = a->v, *bv = b->v, carry = 0;
       ;
       r->vtop = rv, rv++)
  {
    uint8_t have_a = av <= atop;
    uint8_t have_b = bv <= btop;

    if (!have_a && !have_b && !carry)
      break;

    if (rv - r->v >= r->words)
      return error_bignum_sz;

    uint32_t rw = carry;
    carry = 0;

    if (have_a)
    {
      uint32_t aw = *av++;
      rw += aw;
      if (rw < aw)
        carry = 1;
    }

    if (have_b)
    {
      uint32_t bw = *bv++;
      rw += bw;
      if (rw < bw)
        carry = 1;
    }

    *rv = rw;
  }

  bignum_canon(r);
  return OK;
}
