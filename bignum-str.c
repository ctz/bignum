
#include "bignum.h"
#include "bignum-str.h"
#include "sstr.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

static const char *hex_chars = "0123456789abcdef";

error bignum_fmt_hex(const bignum *b, char *buf, size_t len)
{
  assert(!bignum_check(b));
  assert(buf && len);

  sstr out = { buf, buf + len };

  size_t bytes = bignum_len_bytes(b);
  size_t sign = bignum_is_negative(b);

  if ((sign && sstr_putc(&out, '-')) ||
      sstr_putc(&out, '0') ||
      sstr_putc(&out, 'x'))
    return error_buffer_sz;

  for (size_t i = bytes; i > 0; i--)
  {
    uint8_t byte = bignum_get_byte(b, i - 1);

    if (sstr_putc(&out, hex_chars[(byte >> 4) & 0xf]) ||
        sstr_putc(&out, hex_chars[byte & 0xf]))
      return error_buffer_sz;
  }

  if (sstr_put0(&out))
    return error_buffer_sz;

  return OK;
}

error bignum_fmt_dec(const bignum *b, char *buf, size_t len)
{
  uint32_t ten = 10;
  bignum tenbn = { &ten, &ten, 1, 0 };

  BIGNUM_TMP(unitbn);

  if (len < 2)
    return error_buffer_sz;

  BIGNUM_TMP(tmp);
  BIGNUM_TMP(tmp2);
  bignum_dup(&tmp, b);

  /* Work from right to left */
  char *out = buf + len;
  *--out = 0;

  /* Output single 0 for zero. */
  if (bignum_eq(&tmp, &bignum_0))
  {
    if (out == buf)
      return error_buffer_sz;
    *--out = '0';
  }

  tmp.flags &= ~BIGNUM_F_NEG;

  /* Otherwise, repeatedly divide by 10 to obtain digits. */
  while (bignum_gt(&tmp, &bignum_0))
  {
    error err = bignum_divmod(&tmp2, &unitbn, &tmp, &tenbn);
    assert(err == OK);
    if (out == buf)
      return error_buffer_sz;
    assert(unitbn.v[0] < 10);
    *--out = '0' + unitbn.v[0];
    bignum_dup(&tmp, &tmp2);
  }

  /* Finally, add negative sign if necessary. */
  if (bignum_is_negative(b))
  {
    if (out == buf)
      return error_buffer_sz;

    *--out = '-';
  }

  /* Adjust left */
  memmove(buf, out, buf + len - out);

  return OK;
}

error bignum_parse_str(bignum *r, const char *buf)
{
  return bignum_parse_strl(r, buf, strlen(buf));
}

static unsigned parse_hexbyte(char x, uint8_t *out)
{
  if (x >= '0' && x <= '9')
  {
    *out = x - '0';
    return 0;
  } else if (x >= 'A' && x <= 'F') {
    *out = x - 'A' + 10;
    return 0;
  } else if (x >= 'a' && x <= 'f') {
    *out = x - 'a' + 10;
    return 0;
  } else {
    return 1;
  }
}

static unsigned take_hexbyte(sstr *s, uint8_t *b)
{
  char h, l;
  uint8_t rh, rl;
  
  if (sstr_takec(s, &h) ||
      sstr_takec(s, &l) ||
      parse_hexbyte(h, &rh) ||
      parse_hexbyte(l, &rl))
    return 1;

  *b = (rh << 4) | rl;
  return 0;
}

static error parse_hex(bignum *r, sstr *s)
{
  bignum_set(r, 0);

  size_t bytes = (sstr_left(s) + 1) / 2;

  /* If we have an odd-length input, interpret
   * the first character as the low nibble of the
   * first byte. */
  unsigned short_start = sstr_left(s) % 2 == 1;

  for (size_t i = bytes; i > 0; i--)
  {
    uint8_t byte;

    if (i == bytes && short_start)
    {
      /* Take a single nibble */
      char l;
      if (sstr_takec(s, &l) ||
          parse_hexbyte(l, &byte))
        return error_invalid_string;
    } else {
      /* Take a byte */
      if (take_hexbyte(s, &byte))
        return error_invalid_string;
    }

    error err = bignum_set_byte(r, byte, i - 1);
    if (!err)
      return err;
  }

  return OK;
}

error parse_dec(bignum *r, sstr *s)
{
  uint32_t digit = 0;
  bignum bignum_dig = { &digit, &digit, 1, BIGNUM_F_IMMUTABLE };
  BIGNUM_TMP(bignum_tmp);

  bignum_set(r, 0);

  char c;
  error err;

  while (!sstr_takec(s, &c))
  {
    if (c >= '0' && c <= '9')
      digit = c - '0';
    else
      return error_invalid_string;

    if ((err = bignum_multw(&bignum_tmp, r, r, 10)))
      return err;
    if ((err = bignum_addl(r, &bignum_dig)))
      return err;
  }

  return OK;
}

/* nb. this could be generalised to more bases quite nicely.
 * Don't need that right now, though. */
error bignum_parse_strl(bignum *r, const char *buf, size_t len)
{
  sstr s = { (char *) buf, (char*) buf + len };
  char hexmark[2], negmark;
  unsigned hex = 0, neg = 0;
  
  /* Slurp any '-' first. */
  if (!sstr_peekn(&s, &negmark, 1) &&
      negmark == '-' &&
      !sstr_skip(&s, 1))
    neg = 1;

  /* Slurp any '0x' next. */
  if (!sstr_peekn(&s, hexmark, sizeof hexmark) &&
      hexmark[0] == '0' &&
      hexmark[1] == 'x' &&
      !sstr_skip(&s, sizeof hexmark))
    hex = 1;

  error err;
  if (hex)
    err = parse_hex(r, &s);
  else
    err = parse_dec(r, &s);

  if (err == OK && neg)
    bignum_setsign(r, -1);

  bignum_canon(r);
  return err;
}
