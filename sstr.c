#include "sstr.h"

#include <stdio.h>
#include <string.h>

unsigned sstr_putc(sstr *s, char c)
{
  if (s->start < s->end)
  {
    *s->start = c;
    s->start++;
    return 1;
  }

  return 0;
}

unsigned sstr_put0(sstr *s)
{
  return sstr_putc(s, 0);
}

unsigned sstr_puts(sstr *s, const char *str)
{
  while (*str)
  {
    if (!sstr_putc(s, *str++))
      return 0;
  }

  return 1;
}

unsigned sstr_takec(sstr *s, char *c)
{
  if (s->start < s->end)
  {
    *c = *s->start;
    s->start++;
    return 1;
  }

  return 0;
}

char sstr_take0(sstr *s)
{
  char r = 0;
  sstr_takec(s, &r);
  return r;
}

unsigned sstr_taken(sstr *s, char *c, size_t n)
{
  while (--n)
  {
    if (!sstr_takec(s, c))
      return 0;
    c++;
  }

  return 1;
}

unsigned sstr_peekn(sstr *s, char *c, size_t n)
{
  char *bound = s->start + n;
  if (bound >= s->end || bound < s->start)
    return 0;
  memcpy(c, s->start, bound - s->start);
  return 1;
}

char sstr_peek0(sstr *s)
{
  char r = 0;
  sstr_peekn(s, &r, 1);
  return r;
}

unsigned sstr_skip(sstr *s, size_t n)
{
  char *bound = s->start + n;
  if (bound >= s->end || bound < s->start)
    return 0;
  s->start += n;
  return 1;
}

size_t sstr_left(sstr *s)
{
  if (s->start <= s->end)
    return (size_t) (s->end - s->start);
  else
    return 0;
}
