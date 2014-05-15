
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bigmath.h"

error bignum_add(bignum *r, const bignum *a, const bignum *b)
{
  uint32_t *atop = a->vtop;
  uint32_t *btop = b->vtop;

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
    r->flags |= BIGNUM_F_NEG;
  } else if (nega ^ negb) {
    /* -a + b and a + -b cases. */
    if (nega)
    {
      /* Convert -a + b to b + -a. */
      const bignum *tmp = a;
      a = b;
      b = tmp;
    }

    //TODO return bignum_sub(r, a, b);
  } else {
    /* a + b case. */
    r->flags &= ~BIGNUM_F_NEG;
  }

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

  return OK;
}
