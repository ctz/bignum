#include "bignum.h"
#include "bignum-str.h"

#include <stdio.h>
#include <assert.h>

void bignum_dump(const char *label, const bignum *b)
{
  char buf[2048];
  error err = bignum_fmt_hex(b, buf, sizeof buf);
  assert(err == OK);
  printf("%s = %s%s +%uw\n",
         label, buf,
         b->flags & BIGNUM_F_IMMUTABLE ? " +immutable" : "",
         b->words);
}

