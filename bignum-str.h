#ifndef BIGNUM_STR_H
#define BIGNUM_STR_H

/*
 * Bignum library string formatting and parsing functions.
 */

#include <stddef.h>
#include <stdint.h>
#include "bignum.h"

/**
 * Formats the value of b in hex into buf.  buf is always
 * 0 terminated if OK is returned.
 *
 * Formatting:
 * - Negative numbers are prepended with '-',
 * - Numbers are always even length (eg, decimal 0 becomes hex 00),
 * - Hex is lower case.
 * - Hex is prepended with '0x'.
 *
 * Examples:
 *   dec 123 -> hex 0x7b
 *   dec -1  -> hex -0x01
 *
 * error_buffer_sz is returned if buf isn't big enough.
 * buf as NULL or len as 0 is meaningless and illegal.
 */
error bignum_fmt_hex(const bignum *b, char *buf, size_t len);

/**
 * Formats the value of b in decimal into buf.  buf is always
 * 0 terminated if OK is returned.
 *
 * Formatting:
 * - Negative numbers are prepended with '-'.
 *
 * error_buffer_sz is returned if buf isn't big enough.
 * buf as NULL or len as 0 is meaningless and illegal.
 */
error bignum_fmt_dec(const bignum *b, char *buf, size_t len);

/**
 * Parses the hex value from the characters at buf[:len].
 *
 * - If the string starts with '-', the number is negative.
 * - If the string starts with '0x', the rest is interpreted
 *   as hex.  Otherwise, decimal.
 * - Both cases of hex are allowed.
 */
error bignum_parse_strl(bignum *out, const char *buf, size_t len);

/**
 * Nul-terminated version of bignum_parse_strl.
 *
 * Equivalent to bignum_parse_strl(out, buf, strlen(buf));
 */
error bignum_parse_str(bignum *out, const char *buf);

#endif
