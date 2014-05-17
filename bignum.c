
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bigmath.h"

static uint32_t zero = 0, one = 1;
bignum bignum_0 = { &zero, &zero, 1, BIGNUM_F_IMMUTABLE };
bignum bignum_1 = { &one, &one, 1, BIGNUM_F_IMMUTABLE };
bignum bignum_neg1 = { &one, &one, 1, BIGNUM_F_IMMUTABLE | BIGNUM_F_NEG };

error bignum_check(const bignum *b)
{
  assert(b != NULL);
  if (b->v == NULL ||
      b->vtop == NULL ||
      b->v > b->vtop ||
      bignum_len_words(b) > b->words ||
      b->words == 0 ||
      b->words > BIGNUM_MAX_WORDS ||
      (b->flags & ~BIGNUM_F__ALL))
    return error_invalid_bignum;
  return OK;
}

error bignum_check_mutable(const bignum *b)
{
  error e = bignum_check(b);
  assert(!(b->flags & BIGNUM_F_IMMUTABLE));
  return e;
}

#define min(a, b) (((a) < (b)) ? (a) : (b))

void bignum_cleartop(bignum *b, size_t words)
{
  assert(!bignum_check_mutable(b));
  assert(words != 0);
  uint32_t *newtop = b->v + min(b->words, words) - 1;

  while (b->vtop != newtop)
  {
    *b->vtop = 0;
    b->vtop++;
  }
}

void bignum_canon(bignum *b)
{
  assert(!bignum_check_mutable(b));
  while (b->vtop != b->v && *b->vtop == 0)
    b->vtop--;
  
  if (b->vtop == b->v && *b->v == 0)
    b->flags &= ~BIGNUM_F_NEG;
}

void bignum_clear(bignum *b)
{
  assert(!bignum_check_mutable(b));
  for (uint32_t *v = b->v; v <= b->vtop; v++)
    *v = 0;
  memset(b, 0, sizeof *b);
}

error bignum_dup(bignum *r, const bignum *a)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));

  if (a->vtop - a->v >= r->words)
    return error_bignum_sz;

  uint32_t *rtop = r->v + r->words - 1;
  uint32_t *atop = a->vtop;

  for (uint32_t *vr = r->v, *va = a->v;
       vr <= rtop && va <= atop;
       vr++, va++)
  {
    *vr = *va;
    r->vtop = vr;
  }

  bignum_canon(r);
  return OK;
}

void bignum_setu(bignum *b, uint32_t l)
{
  assert(!bignum_check_mutable(b));
  memset(b->v, 0, b->words * BIGNUM_BYTES);
  *b->v = l;
  b->vtop = b->v;
  b->flags &= ~BIGNUM_F_NEG;
  bignum_canon(b);
}

void bignum_set(bignum *b, int32_t v)
{
  if (v < 0)
  {
    bignum_setu(b, (uint32_t) -v);
    b->flags |= BIGNUM_F_NEG;
  } else {
    bignum_setu(b, (uint32_t) v);
  }
}

void bignum_neg(bignum *b)
{
  assert(!bignum_check_mutable(b));
  if (b->flags & BIGNUM_F_NEG)
    b->flags &= ~BIGNUM_F_NEG;
  else
    b->flags |= BIGNUM_F_NEG;
  bignum_canon(b);
}

void bignum_abs(bignum *b)
{
  assert(!bignum_check_mutable(b));
  b->flags &= ~BIGNUM_F_NEG;
  bignum_canon(b);
}

unsigned bignum_eq32(const bignum *b, int32_t v)
{
  if (b->v != b->vtop)
    return 0;
  else if (v == 0 && *b->v == 0)
    return 1;
  else if (v < 0 && *b->v == (uint32_t) -v && (b->flags & BIGNUM_F_NEG))
    return 1;
  else if (v > 0 && *b->v == (uint32_t) v && !(b->flags & BIGNUM_F_NEG))
    return 1;
  else
    return 0;
}

int bignum_sign(const bignum *b)
{
  if (bignum_eq32(b, 0))
    return 1;
  else if (b->flags & BIGNUM_F_NEG)
    return -1;
  else
    return 1;
}

size_t bignum_len_bits(const bignum *b)
{
  assert(!bignum_check(b));
  uint32_t *v = b->vtop;
  while (v != b->v && *v == 0)
    v--;

  size_t whole_words = v - b->v;
  uint8_t extra_bits = bigmath_uint32_fls(*v);

  /* Zero: we need 1 bit to represent this. */
  if (extra_bits == 0 && whole_words == 0)
    return 1;

  return BYTES_TO_BITS(whole_words * BIGNUM_BYTES) + extra_bits;
}

size_t bignum_len_bytes(const bignum *b)
{
  return BITS_TO_BYTES(bignum_len_bits(b));
}

size_t bignum_len_words(const bignum *b)
{
  return b->vtop - b->v + 1;
}

size_t bignum_capacity_bits(const bignum *b)
{
  return b->words * BIGNUM_BYTES * 8;
}

uint8_t bignum_get_byte(const bignum *b, size_t n)
{
  assert(!bignum_check(b));

  size_t word = n / BIGNUM_BYTES;
  size_t byte = n - word * BIGNUM_BYTES;

  if (word > bignum_len_words(b))
    return 0;

  return (b->v[word] >> (byte * 8)) & 0xff;
}

error bignum_set_byte(bignum *b, uint8_t v, size_t n)
{
  assert(!bignum_check_mutable(b));

  size_t word = n / BIGNUM_BYTES;
  size_t byte = n - word * BIGNUM_BYTES;
  size_t bit = byte * 8;
  
  if (word >= b->words)
    return error_bignum_sz;

  bignum_cleartop(b, word + 1);
  uint32_t ww = b->v[word];
  ww &= ~(0xff << bit);
  ww |= v << bit;
  b->v[word] = ww;
  bignum_canon(b);
  return OK;
}

