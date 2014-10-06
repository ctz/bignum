#include "dstr.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>

void dstr_init(dstr *d)
{
  memset(d, 0, sizeof *d);
}

void dstr_free(dstr *d)
{
  free(d->start);
  dstr_init(d);
}

/** Size of allocation blocks. Tunable. */
#define ALLOC_SIZE 0x200

unsigned require(dstr *d, size_t extra)
{
  char *newwr = d->wr + extra;

  if (newwr < d->wr)
    return 1;

  if (newwr + extra > d->end)
  {
    size_t current_offset = dstr_used(d); /* nb. wr pointer is not stable over realloc */
    size_t current_sz = dstr_allocated(d);
    size_t new_sz = (current_sz + extra + ALLOC_SIZE - 1) / ALLOC_SIZE * ALLOC_SIZE;
   
    /* overflow? */
    if (new_sz < current_sz)
      return 1;
    
    /* nb. dstr is still valid if realloc fails. */
    char *new_start = realloc(d->start, new_sz);
    if (!new_start)
      return 1;

    /* clear new memory between offset and end */
    memset(new_start + current_offset, 0, new_sz - current_offset);
    d->start = new_start;
    d->end = new_start + new_sz;
    d->wr = new_start + current_offset;
  }

  return 0;
}

unsigned dstr_put(dstr *d, const char *buf, size_t len)
{
  if (require(d, len))
    return 1;

  memcpy(d->wr, buf, len);
  d->wr += len;
  return 0;
}

unsigned dstr_putc(dstr *d, char c)
{
  return dstr_put(d, &c, 1);
}

unsigned dstr_put0(dstr *d)
{
  return dstr_putc(d, 0);
}

unsigned dstr_puts(dstr *d, const char *str)
{
  return dstr_put(d, str, strlen(str));
}

unsigned dstr_vputf(dstr *d, const char *fmt, va_list ap)
{
  /* First, we do a 'counting' format operation.
   * This may actually work if we have enough buffer available. */
  va_list count_ap;
  va_copy(count_ap, ap);

  size_t available = d->end - d->wr;
  int used_i = vsnprintf(d->wr, available, fmt, count_ap);
  va_end(count_ap);
  assert(used_i >= 0);
  if (used_i < 0)
    return 1;

  size_t used = (size_t) used_i;

  /* That worked.  Adjust wr and report success. */
  if (used < available)
  {
    d->wr += used;
    return 0;
  }

  /* Otherwise, we need available - used more bytes. */
  if (require(d, available - used))
    return 1;

  /* Now, try the format again. */
  available = d->end - d->wr;
  used_i = vsnprintf(d->wr, available, fmt, ap);
  assert(used_i >= 0);
  if (used_i < 0)
    return 1;

  used = (size_t) used_i;
  d->wr += used;
  return 0;
}

unsigned dstr_putf(dstr *d, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  unsigned e = dstr_vputf(d, fmt, ap);
  va_end(ap);
  return e;
}

size_t dstr_allocated(dstr *d)
{
  if (d->end >= d->start)
    return d->end - d->start;
  return 0;
}

size_t dstr_used(dstr *d)
{
  if (d->wr >= d->start)
    return d->wr - d->start;
  return 0;
}

