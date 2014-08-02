
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "bignum.h"
//#define BIGNUM_DEBUG_ENABLED
#include "bignum-dbg.h"
#include "bignum-monty.h"
#include "handy.h"

#define WINDOW_SIZE 2

error bignum_modexp(bignum *r, const bignum *a, const bignum *b, const bignum *p)
{
  assert(!bignum_check_mutable(r));
  assert(!bignum_check(a));
  assert(!bignum_check(b));
  assert(!bignum_check(p));

  monty_ctx monty;
  unsigned use_monty = bignum_monty_setup(p, &monty);
  printf("-----\nuse_monty = %u\n", use_monty);

  BIGNUM_TMP(tmp);
  (void) tmp;

  bignum_dump("a", a);
  bignum_dump("b", b);
  bignum_dump("p", p);

  size_t tabsize = 1 << WINDOW_SIZE;
  bignum tab[1 << WINDOW_SIZE];
  uint32_t tab_words[BIGNUM_MAX_WORDS * tabsize];
  for (size_t i = 0; i < tabsize; i++)
  {
    uint32_t *w = &tab_words[i * BIGNUM_MAX_WORDS];
    *w = 0;
    tab[i].v = w;
    tab[i].vtop = w;
    tab[i].words = BIGNUM_MAX_WORDS;
    tab[i].flags = 0;
  }

  bignum_setu(&tab[0], 1);
  if (use_monty)
    ER(bignum_monty_normalise(&tab[0], &tab[0], p, &monty));
  bignum_dump("tab_0", &tab[0]);

  for (size_t i = 1; i < tabsize; i++)
  {
    ER(bignum_modmul(&tab[i], &tab[i - 1], a, p));
    bignum_dump("tab", &tab[i]);
  }

  bignum_dump("b", b);

  ER(bignum_dup(r, &tab[0]));
  size_t j = bignum_len_bits(b);

  while (1)
  {
    size_t bits = (j >= WINDOW_SIZE) ? WINDOW_SIZE : j;
    uint32_t v = bignum_get_bits(b, j - bits, bits);

    for (size_t s = 0; s < bits; s++)
    {
      if (use_monty)
        ER(bignum_monty_sqr(r, r, p, &monty));
      else
        ER(bignum_modmul(r, r, r, p));
    }

    bignum_dump("after-sqr", r);

    if (use_monty)
      ER(bignum_monty_modmul_normalised(&tmp, r, &tab[v], p, &monty));
    else
      ER(bignum_modmul(&tmp, r, &tab[v], p));
    ER(bignum_dup(r, &tmp));

    bignum_dump("int", r);

    j -= bits; 
    if (j == 0)
      break;
  }

  bignum_dump("r", r);
  return OK;
}

