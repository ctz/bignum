#ifndef BIGNUM_DBG
#define BIGNUM_DBG

#include "bignum.h"

//#define BIGNUM_DEBUG_ENABLED

#ifdef BIGNUM_DEBUG_ENABLED
void bignum_dump(const char *label, const bignum *b);
#else
# define bignum_dump(lbl, b) do {} while (0)
#endif

#endif
