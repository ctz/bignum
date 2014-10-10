#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/time.h>

#include "sstr.h"
#include "dstr.h"
#include "ext/cutest.h"

static void test_sstr_take(void)
{
  char buf[1] = { 'a' };
  char tmp;
  sstr init_one = { buf, buf + 1 };
  sstr s = init_one;

  /* take0 */
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_take0(&s) == 'a');
  TEST_CHECK(sstr_left(&s) == 0);
  TEST_CHECK(sstr_take0(&s) == 0);

  /* takec */
  s = init_one;
  TEST_CHECK(sstr_takec(&s, &tmp) == 0);
  TEST_CHECK(tmp == 'a');
  TEST_CHECK(sstr_takec(&s, &tmp) == 1);

  /* taken */
  s = init_one;
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_taken(&s, &tmp, 1) == 0);
  TEST_CHECK(tmp == 'a');
  TEST_CHECK(sstr_left(&s) == 0);
  TEST_CHECK(sstr_taken(&s, &tmp, 1) == 1);

  s = init_one;
  char buf3[3];

  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_taken(&s, buf3, 3) == 1);
  TEST_CHECK(sstr_left(&s) == 1);
}

static void test_sstr_peek(void)
{
  char buf1[1] = { 'a' };
  char buf3[3] = "123";
  char tmp;
  char tmp3[3];
  sstr init_one = { buf1, buf1 + 1 };
  sstr init_three = { buf3, buf3 + 3 };

  sstr s = init_one;

  /* Basic */
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_peekn(&s, &tmp, 1) == 0);
  TEST_CHECK(tmp == 'a');
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_take0(&s) == 'a');
  TEST_CHECK(sstr_left(&s) == 0);
  TEST_CHECK(sstr_peekn(&s, &tmp, 1) == 1);

  s = init_three;
  TEST_CHECK(sstr_left(&s) == 3);
  TEST_CHECK(sstr_peekn(&s, tmp3, 2) == 0);
  TEST_CHECK(tmp3[0] == '1' && tmp3[1] == '2');
  TEST_CHECK(sstr_skip(&s, 2) == 0);
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_peekn(&s, tmp3, 2) == 1);
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_peekn(&s, tmp3, 1) == 0);
  TEST_CHECK(tmp3[0] == '3');
  TEST_CHECK(sstr_left(&s) == 1);
  TEST_CHECK(sstr_skip(&s, 1) == 0);
  TEST_CHECK(sstr_left(&s) == 0);
}

static void test_dstr_basic(void)
{
  dstr d;
  dstr_init(&d);
  dstr_puts(&d, "hello world");
  dstr_putf(&d, "\nhello earth %d %d %x\n", 1, 2, 0xffff);
  dstr_put0(&d);
  TEST_CHECK(strcmp("hello world\nhello earth 1 2 ffff\n", d.start) == 0);
  dstr_free(&d);

  dstr_init(&d);
  for (size_t i = 0; i < 1024; i++)
  {
    dstr_putc(&d, 'A');
  }
  TEST_CHECK(dstr_used(&d) == 1024);
  dstr_free(&d);

  dstr_init(&d);
  for (size_t i = 0; i < 128; i++)
  {
    char buf[257] = { 'a' };
    dstr_put(&d, buf, sizeof buf);
  }
  TEST_CHECK(dstr_used(&d) == 257 * 128);
  dstr_free(&d);
}

static void test_dstr_hex(void)
{
  dstr d;
  dstr_init(&d);
  dstr_puthex(&d, (void *) "1234:", 5);
  dstr_put0(&d);
  TEST_CHECK(strcmp("313233343a", d.start) == 0);
  dstr_free(&d);
}

TEST_LIST = {
  { "sstr-take", test_sstr_take },
  { "sstr-peek", test_sstr_peek },
  { "dstr-basic", test_dstr_basic },
  { "dstr-hex", test_dstr_hex },
  { 0 }
};
