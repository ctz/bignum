
#include "bignum.h"
#include "bignum-str.h"

#include <assert.h>

static const char *hex_chars = "0123456789abcdef";

error bignum_fmt_hex(const bignum *b, char *buf, size_t len)
{
  assert(!bignum_check(b));
  assert(buf && len);
  size_t usable = len - 1;

  size_t bytes = bignum_len_bytes(b);
  size_t sign = bignum_is_negative(b);

  if (usable < bytes * 2 + sign)
    return error_buffer_sz;

  if (sign)
    *buf++ = '-';

  for (size_t i = bytes; i > 0; i--)
  {
    uint8_t byte = bignum_get_byte(b, i - 1);
    *buf++ = hex_chars[(byte >> 4) & 0xf];
    *buf++ = hex_chars[byte & 0xf];
  }

  *buf++ = 0;
  return OK;
}
