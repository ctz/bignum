
#include <assert.h>
#include <string.h>

#include "bignum.h"

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
      b->vtop - b->v >= b->words ||
      b->words >= BIGNUM_MAX_WORDS ||
      (b->flags & ~BIGNUM_F__ALL))
    return error_invalid_bignum;
  return OK;
}

static error bignum_check_mutable(const bignum *b)
{
  error e = bignum_check(b);
  assert(!(b->flags & BIGNUM_F_IMMUTABLE));
  return e;
}

void bignum_canon(bignum *b)
{
  assert(!bignum_check_mutable(b));
  while (b->vtop != b->v && *b->vtop != 0)
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

void bignum_set(bignum *b, uint32_t l)
{
  assert(!bignum_check_mutable(b));
  memset(b->v, 0, b->words * BIGNUM_BYTES);
  *b->v = l;
  b->vtop = b->v;
  bignum_canon(b);
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

static int bignum_eq32(const bignum *b, int32_t v)
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

unsigned bignum_is_negative(const bignum *b)
{
  return bignum_sign(b) == -1;
}

static uint8_t topbit_index(uint32_t v)
{
  uint8_t r = 0;

  for (; v != 0; v >>= 1)
    r++;

  return r;
}

size_t bignum_len_bits(const bignum *b)
{
  assert(!bignum_check(b));
  uint32_t *v = b->vtop;
  while (v != b->v && *v == 0)
    v--;

  size_t words = v - b->v;
  return BYTES_TO_BITS(words * BIGNUM_BYTES) + topbit_index(*v);
}

size_t bignum_len_bytes(const bignum *b)
{
  return BITS_TO_BYTES(bignum_len_bits(b));
}

uint8_t bignum_get_byte(const bignum *b, size_t n)
{
  assert(!bignum_check(b));

  size_t word = n / BIGNUM_BYTES;
  size_t byte = n - word * BIGNUM_BYTES;
  size_t word_count = (size_t) (b->vtop - b->v);

  if (word > word_count)
    return 0;

  return (b->v[word] >> (byte * 8)) & 0xff;
}

error bignum_add(bignum *r, const bignum *a, const bignum *b)
{
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

    uint32_t aw = 0, bw = 0, rw;

    if (have_a)
      aw = *av++;
    if (have_b)
      bw = *bv++;

    rw = aw + bw + carry;
    carry = ((aw != 0 && rw < aw) || (bw != 0 && rw < bw));
    *rv = rw;
  }

  return OK;
}

error bignum_addl(bignum *a, const bignum *b)
{
  return bignum_add(a, a, b);
}
