#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/time.h>

#include "bignum.h"
#include "bignum-str.h"
#include "ext/cutest.h"

static bignum bignum_alloc(void)
{
  size_t words = 256;
  size_t bytes = words * BIGNUM_BYTES;
  uint32_t *v = malloc(bytes);
  assert(v);
  memset(v, 0, bytes);
  return (bignum) { v, v, words, 0 };
}

/* Compact heap allocation to improve valgrind sensitivity. */
static void bignum_compact(bignum *b)
{
  assert(b);
  size_t words = bignum_len_words(b);
  uint32_t *new_storage = malloc(words * BIGNUM_BYTES);
  memcpy(new_storage, b->v, words * BIGNUM_BYTES);
  free(b->v);
  b->v = new_storage;
  b->vtop = b->v + words - 1;
  b->words = words;
  assert(!bignum_check(b));
}

static void bignum_free(bignum *b)
{
  if (!b)
    return;
  assert(bignum_check(b) == OK);
  free(b->v);
  memset(b, 0, sizeof *b);
}

static double microtime(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);

  double rv = tv.tv_sec;
  rv += ((double) tv.tv_usec) / 1e6;
  return rv;
}

static void print(const char *label, const bignum *b)
{
  char buf[4096];
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

/* --- Equalities --- */
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

/* --- Functions --- */
typedef void (*evalfn)(bignum *r, const bignum *a, const bignum *b, const bignum *c);

static void eval_mul(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 != NULL && arg2 != NULL && arg3 == NULL);
  error err = bignum_mul(r, arg1, arg2);
  assert(err == OK);
}

static void eval_add(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  error err;
  err = bignum_add(r, arg1, arg2);
  assert(err == OK);
  if (arg3)
  {
    err = bignum_addl(r, arg3);
    assert(err == OK);
  }
}

static void eval_sub(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  error err;
  err = bignum_sub(r, arg1, arg2);
  assert(err == OK);
  if (arg3)
  {
    err = bignum_subl(r, arg3);
    assert(err == OK);
  }
}

static void eval_sqr(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && !arg2 && !arg3);
  error err = bignum_sqr(r, arg1);
  assert(err == OK);
}

static void eval_mod(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);
  error err = bignum_mod(r, arg1, arg2);
  assert(err == OK);
}

static void eval_div(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);
  error err = bignum_div(r, arg1, arg2);
  assert(err == OK);
}

static void eval_shl(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);
  assert(bignum_len_words(arg2) == 1 && !bignum_is_negative(arg2));
  error err = bignum_dup(r, arg1);
  assert(err == OK);
  err = bignum_shl(r, (size_t) *arg2->vtop);
  assert(err == OK);
}

static void eval_shr(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);
  assert(bignum_len_words(arg2) == 1 && !bignum_is_negative(arg2));
  error err = bignum_dup(r, arg1);
  assert(err == OK);
  err = bignum_shr(r, (size_t) *arg2->vtop);
  assert(err == OK);
}

static void eval_modmul(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && arg3);
  error err = bignum_modmul(r, arg1, arg2, arg3);
  assert(err == OK);
}

static void eval_modexp(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && arg3);
  double start = microtime();
  error err = bignum_modexp(r, arg1, arg2, arg3);
  printf("modexp(%zu,%zu,%zu bits) = %dms\n",
         bignum_len_bits(arg1),
         bignum_len_bits(arg2),
         bignum_len_bits(arg3),
         (int) ((microtime() - start) * 1000));
  assert(err == OK);
}

static void eval_gcd(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);
  error err = bignum_gcd(r, arg1, arg2);
  assert(err == OK);
}

static unsigned check_egcd(const bignum *gcd, const bignum *a, const bignum *b,
                           const bignum *x, const bignum *y)
{
  BIGNUM_TMP(v1);
  BIGNUM_TMP(v2);

  error err = bignum_mul(&v1, a, x);
  assert(err == OK);
  err = bignum_mul(&v2, b, y);
  assert(err == OK);
  err = bignum_addl(&v1, &v2);
  assert(err == OK);
  return bignum_eq(gcd, &v1);
}

static void eval_egcd_v(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);

  BIGNUM_TMP(a);
  BIGNUM_TMP(b);
  error err = bignum_extended_gcd(r, &a, &b, arg1, arg2);
  assert(err == OK);

  assert(check_egcd(r, &a, &b, arg1, arg2));
}

static void eval_egcd_a(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);

  BIGNUM_TMP(v);
  BIGNUM_TMP(b);
  error err = bignum_extended_gcd(&v, r, &b, arg1, arg2);
  assert(err == OK);
  
  assert(check_egcd(&v, r, &b, arg1, arg2));
}

static void eval_egcd_b(bignum *r, const bignum *arg1, const bignum *arg2, const bignum *arg3)
{
  assert(arg1 && arg2 && !arg3);

  BIGNUM_TMP(v);
  BIGNUM_TMP(a);
  error err = bignum_extended_gcd(&v, &a, r, arg1, arg2);
  assert(err == OK);
  
  assert(check_egcd(&v, &a, r, arg1, arg2));
}

typedef struct
{
  const char *str;
  evalfn fn;
} eval;

static const eval evaluators[] = {
  { "modmul", eval_modmul },
  { "modexp", eval_modexp },
  { "egcd-v", eval_egcd_v },
  { "egcd-a", eval_egcd_a },
  { "egcd-b", eval_egcd_b },
  { "gcd", eval_gcd },
  { "mul", eval_mul },
  { "add", eval_add },
  { "sub", eval_sub },
  { "sqr", eval_sqr },
  { "mod", eval_mod },
  { "div", eval_div },
  { "shl", eval_shl },
  { "shr", eval_shr },
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

  TEST_CHECK_(0, "Expression '%s' does not contain equality operator", expr);
  abort();
}

static void convert_bignum(bignum *num, const char *str, size_t len)
{
  error err = bignum_parse_strl(num, str, len);
  assert(err == OK);
}

static void convert_eval_params(const char *str, const char *end,
                                bignum *arg0, bignum *arg1, bignum *arg2)
{
  const char *ptr = str;
  const char *start = str;
  size_t arg = 0;
  bignum *args[3] = { arg0, arg1, arg2 };

#define PROCESS_ARG                                 \
  {                                                 \
    const char *end = ptr;                          \
    trim(&start, &end);                             \
    *(args[arg]) = bignum_alloc();                  \
    convert_bignum(args[arg], start, end - start);  \
    arg++;                                          \
    assert(arg <= 3);                               \
  }

  while (ptr != end)
  {
    if (*ptr == ',')
    {
      PROCESS_ARG;
      start = ptr + 1;
    }

    ptr++;
  }

  PROCESS_ARG;
}

static void convert_arg(bignum *num, const char *str, size_t len)
{
  for (const eval *e = evaluators;
       e->str;
       e++)
  {
    if (strstr(str, e->str) == str)
    {
      bignum arg0, arg1, arg2;
      arg0.v = arg1.v = arg2.v = NULL;
      convert_eval_params(str + strlen(e->str) + 1, str + len - 1,
                          &arg0, &arg1, &arg2);

      if (arg0.v) bignum_compact(&arg0);
      if (arg1.v) bignum_compact(&arg1);
      if (arg2.v) bignum_compact(&arg2);

      e->fn(num,
            arg0.v ? &arg0 : NULL,
            arg1.v ? &arg1 : NULL,
            arg2.v ? &arg2 : NULL);

      if (arg0.v) bignum_free(&arg0);
      if (arg1.v) bignum_free(&arg1);
      if (arg2.v) bignum_free(&arg2);
      return;
    }
  }

  convert_bignum(num, str, len);
}

static void check(const char *expr)
{
  const char *left, *right;
  const char *endleft, *endright;
  const eq *equality;
  split_eq(expr, &left, &endleft, &right, &endright, &equality);

  bignum a = bignum_alloc(), b = bignum_alloc();
  convert_arg(&a, left, endleft - left);
  convert_arg(&b, right, endright - right);
  bignum_compact(&a);
  bignum_compact(&b);
  if (!TEST_CHECK_(equality->fn(&a, &b) == 1, "Expression '%s' is not true", expr))
  {
    print("lhs", &a);
    print("rhs", &b);
  }
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

static void addsign(void)
{
  check("sub(0,1) == -1");
  check("sub(1,2) == -1");
  check("add(1,-1) == 0");
  check("add(-1,1) == 0");
  check("add(-1,2) == 1");
  check("add(-1,-1) == -2");
}

static void test_stdin(void)
{
  char line[8192];
  const char *callheader = "check(\"";
  const char *calltrailer = "\");";
  size_t lines = 0;

  if (isatty(fileno(stdin)))
    return;

  while (1)
  {
    if (fgets(line, sizeof line, stdin) == NULL)
      break;

    char *end = line + strlen(line) - 1;
    while (*end == '\r' || *end == '\n')
      *end-- = '\0';

    if (strstr(line, callheader) == line &&
        strstr(line, calltrailer) == line + strlen(line) - strlen(calltrailer))
    {
      line[strlen(line) - strlen(calltrailer)] = '\0';
      char *expr = line + strlen(callheader);
      check(expr);
      lines++;

      if (lines % 1000 == 0)
        printf("%zu lines\n", lines);
    } else {
      printf("ignored unhandled line: %s\n", line);
    }
  }
}

static void test_add(void)
{
#include "test-add.inc"
}

static void test_mul(void)
{
#include "test-mul.inc"
}

static void test_sub(void)
{
#include "test-sub.inc"
}

static void test_sqr(void)
{
#include "test-sqr.inc"
}

static void test_mod(void)
{
#include "test-mod.inc"
}

static void test_div(void)
{
#include "test-div.inc"
}

static void test_shl(void)
{
#include "test-shl.inc"
}

static void test_shr(void)
{
#include "test-shr.inc"
}

static void test_modmul(void)
{
#include "test-modmul.inc"
}

static void test_modexp(void)
{
#include "test-modexp.inc"
}

static void test_gcd(void)
{
#include "test-gcd.inc"
}

static void test_egcd_v(void)
{
#include "test-egcd-v.inc"
}

static void test_egcd_a(void)
{
#include "test-egcd-a.inc"
}

static void test_egcd_b(void)
{
#include "test-egcd-b.inc"
}

TEST_LIST = {
  { "basic_test", basic_test },
  { "inequality", inequality },
  { "addsign", addsign },
  { "stdin", test_stdin },
  { "add", test_add },
  { "sub", test_sub },
  { "mul", test_mul },
  { "sqr", test_sqr },
  { "div", test_div },
  { "mod", test_mod },
  { "shl", test_shl },
  { "shr", test_shr },
  { "modmul", test_modmul },
  { "modexp", test_modexp },
  { "gcd", test_gcd },
  { "egcd-v", test_egcd_v },
  { "egcd-a", test_egcd_a },
  { "egcd-b", test_egcd_b },
  { 0 }
};
