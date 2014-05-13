/*
 * sstr: fixed-sized buffer management for constructing strings
 *
 * This is a simple type containing start and end pointers, and
 * functions which write to/read from and advance the start pointer.
 * The start pointer cannot advance past the end pointer.
 *
 * Compare 'dstr', which depends on an allocator and grows the
 * underlying storage as necessary.
 *
 * All functions return 1 on success, 0 on overflow.
 */

#ifndef SSTR_H
#define SSTR_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
  char *start, *end;
} sstr;

/** Append a single character c to s. */
unsigned sstr_putc(sstr *s, char c);

/** Append a single zero to s (for 0 termination, perhaps). */
unsigned sstr_put0(sstr *s);

/** Append the 0-terminated string str to s (not including 0 termination). */
unsigned sstr_puts(sstr *s, const char *str);

/** Take a character from s into c, advancing s by one. */
unsigned sstr_takec(sstr *s, char *c);

/** Take n characters from s into c, leaving s where it is. */
unsigned sstr_peekn(sstr *s, char *c, size_t n);

/** Advance s by n characters. */
unsigned sstr_skip(sstr *s, size_t n);

/** Difference between start and end pointers. */
size_t sstr_left(sstr *s);

#endif
