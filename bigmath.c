
#include "bigmath.h"

#include <inttypes.h>
#include <stdio.h>

void bigmath_add_word(uint32_t *r, uint32_t v)
{
  if (v == 0)
    return;

  uint32_t old = *r;
  *r += v;

  /* Carry up. */
  if (*r < old)
  {
    r++;
    do
    {
      *r += 1;
    } while (*r++ == 0);
  }
}

void bigmath_add_uint64(uint32_t *r, uint64_t v)
{
  bigmath_add_word(r, v & 0xffffffff);
  bigmath_add_word(r + 1, (v >> 32) & 0xffffffff);
}

void bigmath_mul_accum(uint32_t *r, uint32_t *a, size_t w, uint32_t m)
{
  for (size_t i = 0; i < w; i++, r++, a++)
  {
    uint64_t mm = (uint64_t) *a * m;
    //printf("mul-accum: %" PRIx32 " * %" PRIx32 " = %" PRIx64 "\n", *a, m, mm);
    bigmath_add_uint64(r, mm);
  }
}
