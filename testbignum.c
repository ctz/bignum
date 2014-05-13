#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "bignum.h"
#include "bignum-str.h"
#include "ext/cutest.h"

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

void basic_test(void)
{
  bignum r = bignum_alloc();
  bignum a = bignum_alloc();
  bignum b = bignum_alloc();
  bignum_setu(&b, 32);
  print("32", &b);
  bignum_neg(&b);
  print("-32", &b);
  bignum_abs(&b);
  print("abs(-32)", &b);

  bignum_setu(&a, 0xffffffff);
  bignum_setu(&r, 0);

  print("a", &a);
  print("b", &b);

  error err = bignum_add(&r, &a, &bignum_1);
  assert(err == OK);
  print("r0", &r);

  err = bignum_add(&r, &r, &a);
  assert(err == OK);
  print("r1", &r);
  
  err = bignum_add(&r, &r, &bignum_1);
  assert(err == OK);
  print("r2", &r);

  bignum_free(&r);
  bignum_free(&a);
  bignum_free(&b);
  printf("ok\n");
}

typedef unsigned (*eqfn)(const bignum *a, const bignum *b);

static unsigned both_eq(const bignum *a, const bignum *b)
{
  unsigned r = bignum_eq(a, b);
  TEST_CHECK_(r == bignum_const_eq(a, b), "bignum_const_eq does not agree with bignum_eq");
  return r;
}

static unsigned not_eq(const bignum *a, const bignum *b)
{
  return !both_eq(a, b);
}

typedef struct
{
  const char *str;
  eqfn fn;
} eq;

/* nb. must be longest first. */
static const eq equalities[] = {
  { "==", both_eq },
  { "!=", not_eq },
  { "<=", bignum_lte },
  { ">=", bignum_gte },
  { "<", bignum_lt },
  { ">", bignum_gt },
  { NULL }
};

static void trim(const char **start,
                 const char **end)
{
  while (isspace(**start))
    (*start)++;

  while (isspace(*(*end - 1)))
    (*end)--;
}

static void split_eq(const char *expr,
                     const char **left,
                     const char **endleft,
                     const char **right,
                     const char **endright,
                     const eq **equality)
{
  for (const eq *e = equalities;
       e->str;
       e++)
  {
    const char *found = strstr(expr, e->str);
    if (found)
    {
      *left = expr;
      *endleft = found;
      *right = found + strlen(e->str);
      *endright = *right + strlen(*right);
      trim(left, endleft);
      trim(right, endright);
      *equality = e;
      return;
    }
  }

  TEST_CHECK_(0, "Expression '%s' does not contain operator", expr);
  abort();
}

static void check(const char *expr)
{
  const char *left, *right;
  const char *endleft, *endright;
  const eq *equality;
  error err;
  split_eq(expr, &left, &endleft, &right, &endright, &equality);

  bignum a = bignum_alloc(), b = bignum_alloc();
  err = bignum_parse_strl(&a, left, endleft - left);
  assert(err == OK);
  err = bignum_parse_strl(&b, right, endright - right);
  assert(err == OK);
  TEST_CHECK_(equality->fn(&a, &b) == 1, "Expression '%s' is not true", expr);
  print("a", &a);
  print("b", &b);
  printf("%s -> %u\n", expr, equality->fn(&a, &b));
  bignum_free(&a);
  bignum_free(&b);

}

static void inequality(void)
{
  check("0x1 == 0x1");
  check("0x0 != 0x1");
  check("1 == 1");
  check("0 != 1");
  check("-1 < 1");
  check("-1 <= -1");
  check("1 >= 1");
  check("0 <= 0");
  check("1 >= 0");
  check("1 > -1");
  check("1234567890123456789 > 1234567890123456788");
}

TEST_LIST = {
  { "basic_test", basic_test },
  { "inequality", inequality },
  { 0 }
};
