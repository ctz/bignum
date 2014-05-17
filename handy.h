#ifndef HANDY_H
#define HANDY_H

/*
 * Handy CPP defines and C inline functions.
 */

/* Evaluates to the number of items in array-type variable arr. */
#define ARRAYCOUNT(arr) (sizeof arr / sizeof arr[0])

/* Normal MIN/MAX macros.  Evaluate argument expressions only once. */
#define MIN(x, y) \
  ({ typeof (x) __x = (x); \
     typeof (y) __y = (y); \
     __x < __y ? __x : __y; })
#define MAX(x, y) \
  ({ typeof (x) __x = (x); \
     typeof (y) __y = (y); \
     __x > __y ? __x : __y; })

/* Swap two values.  Uses GCC type inference magic. */
#define SWAP(x, y) \
  do { \
    typeof (x) __tmp = (x); \
    (x) = (y); \
    (y) = __tmp; \
  } while (0)

/* Error handling macros.
 *
 * These expect a numeric 'error' typedef, where non-zero
 * indicates an error. */

/** Error: return. 
 *  
 *  If the expression fails, return the error from this function. */
#define ER(expr) do { error err_ = (expr); if (err_) return err_; } while (0)

/** Error: goto.
 *
 *  If the expression fails, goto x_err.  Assumes defn of label
 *  x_err and 'error err'. */
#define EG(expr) do { err = (expr); if (err) goto x_err; } while (0)



#endif
