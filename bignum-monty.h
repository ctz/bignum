#ifndef BIGNUM_MONTY
#define BIGNUM_MONTY

#include "bignum.h"

/** Montgomery reduction context. */
typedef struct
{
  /* R = 1 << R_shift. */
  size_t R_shift;

  /* m' = -1/m mod b. */
  uint32_t mprime;
} monty_ctx;

/** Fills in *mont and returns 1 if montgomery reduction will work.
 *  Otherwise leaves *mont untouched and returns 0. */
unsigned bignum_monty_setup(const bignum *m, monty_ctx *mont);

/** Multiplies x by R mod m. */
error bignum_monty_normalise(bignum *xR, const bignum *x, const bignum *m,
                             const monty_ctx *monty);
                             
/** Sets A = xy mod m.
 */
error bignum_monty_modmul(bignum *A, const bignum *x, const bignum *y, const bignum *m,
                          const monty_ctx *monty);

/** Sets A = xyR^-1 mod m.
 * 
 *  That means you need to bignum_monty_normalise the output or one
 *  of the inputs. */
error bignum_monty_modmul_normalised(bignum *A, const bignum *x, const bignum *y, const bignum *m,
                                     const monty_ctx *monty);

/** Sets A = x^2 mod m.
 */
error bignum_monty_sqr(bignum *A, const bignum *x, const bignum *m,
                       const monty_ctx *monty);

#endif
