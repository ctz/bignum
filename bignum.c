
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
#include "bignum-math.h"
#include "handy.h"

static uint32_t zero = 0, one = 1, base[2] = { 0, 1 };
bignum bignum_0 = { &zero, &zero, 1, BIGNUM_F_IMMUTABLE };
bignum bignum_1 = { &one, &one, 1, BIGNUM_F_IMMUTABLE };
bignum bignum_neg1 = { &one, &one, 1, BIGNUM_F_IMMUTABLE | BIGNUM_F_NEG };
bignum bignum_base = { &base[0], &base[1], 2, BIGNUM_F_IMMUTABLE };

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

error bignum_cleartop(bignum *b, size_t words)
{
  assert(!bignum_check_mutable(b));
  assert(words != 0);

  if (words == 0 || words > b->words)
    return error_bignum_sz;

  uint32_t *newtop = b->v + words - 1;

  for (uint32_t *ptr = b->vtop + 1;
       ptr <= newtop;
       ptr++)
    *ptr = 0;
  b->vtop = newtop;

  return OK;
}

void bignum_canon(bignum *b)
{
  assert(!bignum_check_mutable(b));
  while (b->vtop != b->v && *b->vtop == 0)
    b->vtop--;
  
  if (bignum_is_zero(b))
    bignum_setsign(b, 0);
}

void bignum_clear(bignum *b)
{
  assert(!bignum_check_mutable(b));
  mem_clean(b->v, b->words * BIGNUM_BYTES);
  mem_clean(b, sizeof *b);
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

  r->flags = 0;
  bignum_setsign(r, bignum_getsign(a));
  bignum_canon(r);
  return OK;
}

void bignum_setu(bignum *b, uint32_t l)
{
  assert(!bignum_check_mutable(b));
  memset(b->v, 0, b->words * BIGNUM_BYTES);
  *b->v = l;
  b->vtop = b->v;
  bignum_setsign(b, 1);
  bignum_canon(b);
}

void bignum_set(bignum *b, int32_t v)
{
  if (v < 0)
  {
    bignum_setu(b, (uint32_t) -v);
    bignum_setsign(b, -1);
  } else {
    bignum_setu(b, (uint32_t) v);
  }
}

void bignum_neg(bignum *b)
{
  assert(!bignum_check_mutable(b));
  bignum_setsign(b, -bignum_getsign(b));
  bignum_canon(b);
}

void bignum_abs(bignum *b)
{
  assert(!bignum_check_mutable(b));
  bignum_setsign(b, 1);
  bignum_canon(b);
}

unsigned bignum_eq32(const bignum *b, int32_t v)
{
  if (b->v != b->vtop)
    return 0;
  else if (v == 0 && *b->v == 0)
    return 1;
  else if (v < 0 && *b->v == (uint32_t) -v && bignum_is_negative(b))
    return 1;
  else if (v > 0 && *b->v == (uint32_t) v && !bignum_is_negative(b))
    return 1;
  else
    return 0;
}

unsigned bignum_is_zero(const bignum *b)
{
  return bignum_eq32(b, 0);
}

unsigned bignum_is_even(const bignum *b)
{
  assert(!bignum_check(b));
  return (b->v[0] & 1) == 0;
}

unsigned bignum_is_odd(const bignum *b)
{
  assert(!bignum_check(b));
  return (b->v[0] & 1) == 1;
}

int bignum_getsign(const bignum *b)
{
  if (bignum_is_zero(b))
    return 1;
  else if (b->flags & BIGNUM_F_NEG)
    return -1;
  else
    return 1;
}

void bignum_setsign(bignum *b, int sign)
{
  assert(!bignum_check_mutable(b));
  if (sign < 0)
    b->flags |= BIGNUM_F_NEG;
  else
    b->flags &= ~BIGNUM_F_NEG;
}

size_t bignum_len_bits(const bignum *b)
{
  assert(!bignum_check(b));
  uint32_t *v = b->vtop;
  while (v != b->v && *v == 0)
    v--;

  size_t whole_words = v - b->v;
  uint8_t extra_bits = bignum_math_uint32_fls(*v);

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
  return b->words * BIGNUM_BITS;
}

uint8_t bignum_get_byte(const bignum *b, size_t n)
{
  assert(!bignum_check(b));

  size_t word = n / BIGNUM_BYTES;
  size_t byte = n % BIGNUM_BYTES;

  if (word > bignum_len_words(b))
    return 0;

  return (b->v[word] >> (byte * 8)) & 0xff;
}

uint8_t bignum_get_bit(const bignum *b, size_t i)
{
  uint8_t byte = bignum_get_byte(b, i / 8);
  i %= 8;
  return (byte >> i) & 1;
}

uint32_t bignum_get_bits(const bignum *b, size_t i, size_t n)
{
  uint32_t r = 0;
  assert(n <= 32);

  for (size_t j = 0; j < n; j++)
  {
    r |= ((uint32_t) bignum_get_bit(b, i + j)) << j;
  }

  return r;
}

static void edit_word(bignum *b, size_t word, uint32_t and, uint32_t or)
{
  uint32_t ww = b->v[word];
  ww &= and;
  ww |= or;
  b->v[word] = ww;
}

error bignum_set_byte(bignum *b, uint8_t v, size_t n)
{
  assert(!bignum_check_mutable(b));

  size_t word = n / BIGNUM_BYTES;
  size_t byte = n % BIGNUM_BYTES;
  size_t bit = byte * 8;
  
  ER(bignum_cleartop(b, word + 1));
  edit_word(b, word,
            ~(0xff << bit),
            v << bit);
  bignum_canon(b);
  return OK;
}

error bignum_set_bit(bignum *b, uint8_t v, size_t n)
{
  assert(!bignum_check_mutable(b));

  size_t word = n / BIGNUM_BITS;
  size_t bit = n % BIGNUM_BITS;

  v = !!v;

  ER(bignum_cleartop(b, word + 1));
  edit_word(b, word,
            ~(1 << bit),
            v << bit);
  bignum_canon(b);
  return OK;
}
