
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"
#include "handy.h"

static error bignum_shl_words(bignum *r, size_t words)
{
  if (words == 0)
    return OK;

  ER(bignum_cleartop(r, bignum_len_words(r) + words));

  /* MSW to LSW. */
  for (uint32_t *dst = r->vtop, *src = r->vtop - words;
       dst >= r->v && src >= r->v;
       dst--, src--)
    *dst = *src;

  /* Shift in 'words' zeroes. */
  for (size_t i = 0; i < words; i++)
    r->v[i] = 0;

  bignum_canon(r);
  return OK;
}

error bignum_shl(bignum *r, size_t bits)
{
  assert(!bignum_check_mutable(r));

  if (bits >= 32)
    ER(bignum_shl_words(r, bits / 32));

  bits %= 32;

  if (bits == 0)
    return OK;

  ER(bignum_cleartop(r, bignum_len_words(r) + 1));

  /* Construct each dst word with at most two src words: hi and lo. */
  for (uint32_t *dst = r->vtop, *src_hi = r->vtop, *src_lo = r->vtop - 1;
       src_hi >= r->v;
       dst--, src_hi--, src_lo--)
  {
    /* nb. lo may be off the end; shift in zeroes */
    uint32_t lo = src_lo >= r->v ? *src_lo : 0;
    *dst = (*src_hi << bits) | (lo >> (32 - bits));
  }

  bignum_canon(r);
  return OK;
}

static error bignum_shr_words(bignum *r, size_t words)
{
  if (words == 0)
    return OK;

  for (uint32_t *dst = r->v, *src = r->v + words;
       dst <= r->vtop && src <= r->vtop;
       dst++, src++)
    *dst = *src;

  /* Shift in 'words' zeroes from right. */
  for (size_t i = 0; i < words; i++)
    if (r->vtop - i >= r->v)
      *(r->vtop - i) = 0;
  
  bignum_canon(r);
  return OK;
}

error bignum_shr(bignum *r, size_t bits)
{
  assert(!bignum_check_mutable(r));

  if (bits >= 32)
    ER(bignum_shr_words(r, bits / 32));

  bits %= 32;

  if (bits == 0)
    return OK;

  uint32_t mask = ~0;
  mask >>= (32 - bits);

  for (uint32_t *dst = r->v, *src_lo = r->v, *src_hi = r->v + 1;
       src_lo <= r->vtop;
       dst++, src_lo++, src_hi++)
  {
    uint32_t hi = src_hi <= r->vtop ? *src_hi : 0;
    *dst = (*src_lo >> bits) | (hi & mask) << (32 - bits);
  }

  bignum_canon(r);
  return OK;
}

error bignum_trunc(bignum *r, size_t bits)
{
  assert(!bignum_check_mutable(r));

  /* Anything to do? */
  if (bits >= bignum_len_bits(r))
    return OK;

  size_t word = bits / BIGNUM_BITS;
  uint32_t mask = (1 << (bits % BIGNUM_BITS)) - 1;

  for (uint32_t *top = r->v + word + 1;
       top <= r->vtop;
       top++)
    *top = 0;

  /* This access is OK because:
   * 1. On entry, r contained a bit set to the left of 'bits'.
   * 2. Therefore, r, contained a valid word at least at bits / 32.
   */
  r->v[word] &= mask;
  bignum_canon(r);
  return OK;

}
