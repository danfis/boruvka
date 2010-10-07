#ifndef __MG_CORE_H__
#define __MG_CORE_H__

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <mg/config.h>
#include <mg/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifndef MG_SINGLE
# ifndef MG_DOUBLE
#  error You must define MG_SINGLE or MG_DOUBLE
# endif /* MG_DOUBLE */
#endif /* MG_SINGLE */


#ifdef MG_SINGLE
# ifdef MG_DOUBLE
#  error You can define either MG_SINGLE or MG_DOUBLE, not both!
# endif /* MG_DOUBLE */

typedef float mg_real_t;

//# define MG_EPS 1E-6
# define MG_EPS FLT_EPSILON
# define MG_REAL_MAX FLT_MAX

# define MG_REAL_STR "%f"

# define MG_REAL(x)     (x ## f)          /*!< form a constant */

# define MG_SQRT(x)     (sqrtf(x))        /*!< square root */
# define MG_FABS(x)     (fabsf(x))        /*!< absolute value */
# define MG_FMAX(x, y)  (fmaxf((x), (y))) /*!< maximum of two floats */
# define MG_FMIN(x, y)  (fminf((x), (y))) /*!< minimum of two floats */
# define MG_CBRT(x)     (cbrtf(x)         /*!< cube root */
# define MG_COS(x)      (cosf(x))         /*!< cosine */
# define MG_SIN(x)      (sinf(x))         /*!< sine */
# define MG_ACOS(x)     (acosf(x))        /*!< arc cosine */
# define MG_ASIN(x)     (asinf(x))        /*!< arc sine */
# define MG_ATAN2(y, x) (atan2f(ym x))    /*!< arc tangent of y/x */

#endif /* MG_SINGLE */

#ifdef MG_DOUBLE
typedef double mg_real_t;

//# define MG_EPS 1E-10
# define MG_EPS DBL_EPSILON
# define MG_REAL_MAX DBL_MAX

# define MG_REAL_STR "%lf"

# define MG_REAL(x)     (x ## f)         /*!< form a constant */

# define MG_SQRT(x)     (sqrt(x))        /*!< square root */
# define MG_FABS(x)     (fabs(x))        /*!< absolute value */
# define MG_FMAX(x, y)  (fmax((x), (y))) /*!< maximum of two floats */
# define MG_FMIN(x, y)  (fmin((x), (y))) /*!< minimum of two floats */
# define MG_CBRT(x)     (cbrt(x)         /*!< cube root */
# define MG_COS(x)      (cos(x))         /*!< cosine */
# define MG_SIN(x)      (sin(x))         /*!< sine */
# define MG_ACOS(x)     (acos(x))        /*!< arc cosine */
# define MG_ASIN(x)     (asin(x))        /*!< arc sine */
# define MG_ATAN2(y, x) (atan2(ym x))    /*!< arc tangent of y/x */
#endif /* MG_DOUBLE */

#define MG_ONE MG_REAL(1.)
#define MG_ZERO MG_REAL(0.)


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __MG_CORE_H__ */
