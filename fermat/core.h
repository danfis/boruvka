/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __FER_CORE_H__
#define __FER_CORE_H__

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include <fermat/config.h>
#include <fermat/compiler.h>

#ifdef FER_SSE
# include <immintrin.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Core - Core Functions and Macros
 * =================================
 */


#ifndef FER_SINGLE
# ifndef FER_DOUBLE
#  error You must define FER_SINGLE or FER_DOUBLE
# endif /* FER_DOUBLE */
#endif /* FER_SINGLE */

#ifdef FER_SSE
union _fer_sse_t {
    __m128 m;
    __m128d md;
    float f[4];
    double d[2];
} fer_aligned(16) fer_packed;
typedef union _fer_sse_t fer_sse_t;
#endif /* FER_SSE */


#ifdef FER_SINGLE
# ifdef FER_DOUBLE
#  error You can define either FER_SINGLE or FER_DOUBLE, not both!
# endif /* FER_DOUBLE */

typedef float fer_real_t;
typedef uint32_t fer_uint_t;

/*# define FER_EPS 1E-6 */
# define FER_EPS FLT_EPSILON  /*!< epsilon */
# define FER_REAL_MAX FLT_MAX /*!< maximal real value */
# define FER_REAL_MIN FLT_MIN /*!< minimal real value */

# define FER_REAL_STR "%.30f"

# define FER_REAL(x)     (x ## f)          /*!< form a constant */

# ifdef FER_SSE
#  define FER_SQRT(x)     (__ferSqrt(x))   /*!< square root */
# else /* FER_SSE */
#  define FER_SQRT(x)     (sqrtf(x))
# endif /* FER_SSE */

# define FER_FABS(x)     (fabsf(x))        /*!< absolute value */
# define FER_FMAX(x, y)  (fmaxf((x), (y))) /*!< maximum of two floats */
# define FER_FMIN(x, y)  (fminf((x), (y))) /*!< minimum of two floats */
# define FER_CBRT(x)     (cbrtf(x)         /*!< cube root */
# define FER_COS(x)      (cosf(x))         /*!< cosine */
# define FER_SIN(x)      (sinf(x))         /*!< sine */
# define FER_ACOS(x)     (acosf(x))        /*!< arc cosine */
# define FER_ASIN(x)     (asinf(x))        /*!< arc sine */
# define FER_ATAN2(y, x) (atan2f(y, x))    /*!< arc tangent of y/x */
# define FER_ATAN(x)     (atanf(x))        /*!< arc tangent of y/x */
# define FER_POW(x, y)   (powf((x), (y)))  /*!< power function */

#endif /* FER_SINGLE */

#ifdef FER_DOUBLE
typedef double fer_real_t;
typedef uint64_t fer_uint_t;

/*# define FER_EPS 1E-10*/
# define FER_EPS DBL_EPSILON
# define FER_REAL_MAX DBL_MAX
# define FER_REAL_MIN DBL_MIN

# define FER_REAL_STR "%.30lf"

# define FER_REAL(x)     (x ## f)

# ifdef FER_SSE
#  define FER_SQRT(x)     (__ferSqrt(x))
# else /* FER_SSE */
#  define FER_SQRT(x)     (sqrt(x))
# endif /* FER_SSE */

# define FER_FABS(x)     (fabs(x))
# define FER_FMAX(x, y)  (fmax((x), (y)))
# define FER_FMIN(x, y)  (fmin((x), (y)))
# define FER_CBRT(x)     (cbrt(x)
# define FER_COS(x)      (cos(x))
# define FER_SIN(x)      (sin(x))
# define FER_ACOS(x)     (acos(x))
# define FER_ASIN(x)     (asin(x))
# define FER_ATAN2(y, x) (atan2(y, x))
# define FER_ATAN(x)     (atan(x))
# define FER_POW(x, y)   (pow((x), (y)))

#endif /* FER_DOUBLE */

union _fer_real_uint_t {
    fer_real_t f;
    fer_uint_t i;
};
typedef union _fer_real_uint_t fer_real_uint_t;


#define FER_MIN(x, y) ((x) < (y) ? (x) : (y)) /*!< minimum */
#define FER_MAX(x, y) ((x) > (y) ? (x) : (y)) /*!< maximum */
#define FER_CUBE(x)   ((x) * (x))             /*!< cube */
#define FER_POWL(x, y) (powl((x), (y)))       /*!< power function */

#define FER_ONE FER_REAL(1.)  /*!< unit constant */
#define FER_ZERO FER_REAL(0.) /*!< zero contant */


/**
 * Swaps {a} and {b} using given temporary variable {tmp}.
 */
#define FER_SWAP(a, b, tmp) \
    (tmp) = (a); \
    (a) = (b); \
    (b) = (tmp)


#ifdef FER_SSE
_fer_inline fer_real_t __ferSqrt(fer_real_t val);
#endif /* FER_SSE */

/**
 * Returns sign of value.
 */
_fer_inline int ferSign(fer_real_t val);

/**
 * Returns true if val is zero.
 */
_fer_inline int ferIsZero(fer_real_t val);

/**
 * Returns true if a and b equal.
 */
_fer_inline int ferEq(fer_real_t a, fer_real_t b);

/**
 * Returns true if a and b not equal.
 */
_fer_inline int ferNEq(fer_real_t a, fer_real_t b);

/**
 * Returns 1 / d.
 */
_fer_inline fer_real_t ferRecp(fer_real_t v);

/**
 * Returns 1 / sqrt(v)
 */
_fer_inline fer_real_t ferRsqrt(fer_real_t v);


/**
 * Alignes given memory.
 */
_fer_inline void *ferAlign(void *mem, int alignment);

/**
 * Returns integer representation of real number
 */
_fer_inline fer_uint_t ferRealAsUInt(fer_real_t x);

/***** INLINES *****/
#ifdef FER_SSE
_fer_inline fer_real_t __ferSqrt(fer_real_t val)
{
    fer_sse_t m;

#ifdef FER_SSE_SINGLE
    m.m = _mm_set1_ps(val);
    m.m = _mm_sqrt_ps(m.m);
    return m.f[0];
#else /* FER_SSE_SINGLE */
    m.md = _mm_set1_pd(val);
    m.md = _mm_sqrt_pd(m.md);
    return m.d[0];
#endif /* FER_SSE_SINGLE */
}
#endif /* FER_SSE */

_fer_inline int ferSign(fer_real_t val)
{
    if (ferIsZero(val)){
        return 0;
    }else if (val < FER_ZERO){
        return -1;
    }
    return 1;
}

_fer_inline int ferIsZero(fer_real_t val)
{
    return FER_FABS(val) < FER_EPS;
}

_fer_inline int ferEq(fer_real_t _a, fer_real_t _b)
{
    fer_real_t ab;

    ab = FER_FABS(_a - _b);
    if (ab < FER_EPS)
        return 1;

    fer_real_t a, b;
    a = FER_FABS(_a);
    b = FER_FABS(_b);
    if (b > a){
        return ab < FER_EPS * b;
    }else{
        return ab < FER_EPS * a;
    }
}

_fer_inline int ferNEq(fer_real_t a, fer_real_t b)
{
    return !ferEq(a, b);
}

_fer_inline fer_real_t ferRecp(fer_real_t v)
{
#ifdef FER_SSE_SINGLE
    fer_sse_t m;
    m.m = _mm_set1_ps(v);
    m.m = _mm_rcp_ps(m.m);
    return m.f[0];
#else /* FER_SSE_SINGLE */
    return FER_ONE / v;
#endif /* FER_SSE_SINGLE */
}

_fer_inline fer_real_t ferRsqrt(fer_real_t v)
{
#ifdef FER_SSE_SINGLE
    fer_sse_t m;
    m.m = _mm_set1_ps(v);
    m.m = _mm_rsqrt_ps(m.m);
    return m.f[0];
#else /* FER_SSE_SINGLE */
    return FER_ONE / FER_SQRT(v);
# endif /* FER_SSE_SINGLE */
}

_fer_inline void *ferAlign(void *mem, int align)
{
    long padding;

    if (align == 0)
        return mem;

    padding = align - (long)mem % align;
    return (void *)((long)mem + padding);
}

_fer_inline fer_uint_t ferRealAsUInt(fer_real_t x)
{
    fer_real_uint_t v;
    v.f = x;
    return v.i;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CORE_H__ */
