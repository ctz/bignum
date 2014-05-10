#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bignum.h"
#include "bignum-str.h"

static bignum bignum_alloc(void)
{
  size_t words = 64;
  size_t bytes = words * BIGNUM_BYTES;
  uint32_t *v = malloc(bytes);
  assert(v);
  memset(v, 0, bytes);
  return (bignum) { v, v, words, 0 };
}

static void bignum_free(bignum *b)
{
  assert(bignum_check(b) == OK);
  free(b->v);
  memset(b, 0, sizeof *b);
}

static void print(const char *label, const bignum *b)
{
  char buf[256];
  error e = bignum_fmt_hex(b, buf, sizeof buf);
  assert(e == OK);
  printf("%s = %s\n", label, buf);
}

int main(void)
{
  bignum r = bignum_alloc();
  bignum a = bignum_alloc();
  bignum b = bignum_alloc();
  bignum_set(&b, 32);
  print("32", &b);
  bignum_neg(&b);
  print("-32", &b);
  bignum_abs(&b);
  print("abs(-32)", &b);

  bignum_set(&a, 0xffff);
  bignum_set(&b, 0x80003333);

  print("a", &a);
  print("b", &b);

  error err = bignum_add(&r, &a, &b);
  assert(err == OK);
  
  print("r0", &r);

  err = bignum_add(&r, &r, &b);
  assert(err == OK);
  print("r1", &r);
  
  err = bignum_add(&r, &r, &b);
  assert(err == OK);
  print("r2", &r);

  bignum_free(&r);
  bignum_free(&a);
  bignum_free(&b);
  printf("ok\n");
  return 0;
}
