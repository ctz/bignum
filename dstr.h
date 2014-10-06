/*
 * dstr: dynamically expanding string handling.
 *
 * This is a simple type containing start and end pointers defining
 * the allocated buffer, and a write pointer which is where the next
 * write will start.
 *
 * Compare 'sstr', which depends on an underlying buffer of fixed
 * size.
 *
 * All functions return 1 on success, 0 on allocation failure.
 */

#ifndef DSTR_H
#define DSTR_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

typedef struct
{
  /* Start and end of allocated buffer.
   * Invariant: start <= end */
  char *start, *end;

  /* Current writing position.
   * Invariant: start <= wr <= end */
  char *wr;
} dstr;

/** Initialises d to be empty.
 *  Such a dstr does not need to be freed,
 *  (but it doesn't hurt). */
void dstr_init(dstr *d);

/** Frees all storage associated with d.
 *  Leaves d empty (with start = end = wr = NULL). */
void dstr_free(dstr *d);

/** Append a single character c to d. */
unsigned dstr_putc(dstr *d, char c);

/** Append a single zero to d (for 0 termination, perhaps). */
unsigned dstr_put0(dstr *d);

/** Append the 0-terminated string str to d (not including 0 termination). */
unsigned dstr_puts(dstr *d, const char *str);

/** Append a sprintf-like formatted expression. */
unsigned dstr_putf(dstr *d, const char *fmt, ...)
  __attribute__ ((format (printf, 2, 3)));
  ;

/** Append a vsprintf-like formatted expression. */
unsigned dstr_vputf(dstr *d, const char *fmt, va_list arg);

/** Difference between start and end pointers. */
size_t dstr_allocated(dstr *d);

/** Difference between wr and start pointers. */
size_t dstr_used(dstr *d);

#endif
