#ifndef BIGNUM_STR_H
#define BIGNUM_STR_H

/*
 * Bignum library string formatting and parsing functions.
 *
 * In general these depend on bignum-alloc too, and hence
 * the existence of an allocator.
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
 * - Hex is NOT prepended with '0x'.
 *
 * Examples:
 *   dec 123 -> hex 7b
 *   dec -1  -> hex -01
 *
 * error_buffer_sz is returned if buf isn't big enough.
 * buf as NULL or len as 0 is illegal.
 */
error bignum_fmt_hex(const bignum *b, char *buf, size_t len);

#endif
