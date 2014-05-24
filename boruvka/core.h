/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_CORE_H__
#define __BOR_CORE_H__

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include <boruvka/config.h>
#include <boruvka/compiler.h>

#ifdef BOR_SSE
# include <immintrin.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Core - Core Functions and Macros
 * =================================
 */


#ifndef BOR_SINGLE
# ifndef BOR_DOUBLE
#  error You must define BOR_SINGLE or BOR_DOUBLE
# endif /* BOR_DOUBLE */
#endif /* BOR_SINGLE */

#ifdef BOR_SSE
union _bor_sse_t {
    __m128 m;
    __m128d md;
    float f[4];
    double d[2];
} bor_aligned(16) bor_packed;
typedef union _bor_sse_t bor_sse_t;
#endif /* BOR_SSE */


#ifdef BOR_SINGLE
# ifdef BOR_DOUBLE
#  error You can define either BOR_SINGLE or BOR_DOUBLE, not both!
# endif /* BOR_DOUBLE */

typedef float bor_real_t;
typedef uint32_t bor_uint_t;

/*# define BOR_EPS 1E-6 */
# define BOR_EPS FLT_EPSILON  /*!< epsilon */
# define BOR_REAL_MAX FLT_MAX /*!< maximal real value */
# define BOR_REAL_MIN FLT_MIN /*!< minimal real value */

# define BOR_REAL_STR "%.30f"

# define BOR_REAL(x)     (x ## f)          /*!< form a constant */

# ifdef BOR_SSE
#  define BOR_SQRT(x)     (__borSqrt(x))   /*!< square root */
# else /* BOR_SSE */
#  define BOR_SQRT(x)     (sqrtf(x))
# endif /* BOR_SSE */

# define BOR_FABS(x)     (fabsf(x))        /*!< absolute value */
# define BOR_FMAX(x, y)  (fmaxf((x), (y))) /*!< maximum of two floats */
# define BOR_FMIN(x, y)  (fminf((x), (y))) /*!< minimum of two floats */
# define BOR_CBRT(x)     (cbrtf(x)         /*!< cube root */
# define BOR_COS(x)      (cosf(x))         /*!< cosine */
# define BOR_SIN(x)      (sinf(x))         /*!< sine */
# define BOR_ACOS(x)     (acosf(x))        /*!< arc cosine */
# define BOR_ASIN(x)     (asinf(x))        /*!< arc sine */
# define BOR_ATAN2(y, x) (atan2f(y, x))    /*!< arc tangent of y/x */
# define BOR_ATAN(x)     (atanf(x))        /*!< arc tangent of y/x */
# define BOR_POW(x, y)   (powf((x), (y)))  /*!< power function */
# define BOR_EXP(x)      (expf(x))         /*!< e exponential */
# define BOR_LOG(x)      (logf(x))         /*!< natural logarithm */

#endif /* BOR_SINGLE */

#ifdef BOR_DOUBLE
typedef double bor_real_t;
typedef uint64_t bor_uint_t;

/*# define BOR_EPS 1E-10*/
# define BOR_EPS DBL_EPSILON
# define BOR_REAL_MAX DBL_MAX
# define BOR_REAL_MIN DBL_MIN

# define BOR_REAL_STR "%.30lf"

# define BOR_REAL(x)     (x ## f)

# ifdef BOR_SSE
#  define BOR_SQRT(x)     (__borSqrt(x))
# else /* BOR_SSE */
#  define BOR_SQRT(x)     (sqrt(x))
# endif /* BOR_SSE */

# define BOR_FABS(x)     (fabs(x))
# define BOR_FMAX(x, y)  (fmax((x), (y)))
# define BOR_FMIN(x, y)  (fmin((x), (y)))
# define BOR_CBRT(x)     (cbrt(x)
# define BOR_COS(x)      (cos(x))
# define BOR_SIN(x)      (sin(x))
# define BOR_ACOS(x)     (acos(x))
# define BOR_ASIN(x)     (asin(x))
# define BOR_ATAN2(y, x) (atan2(y, x))
# define BOR_ATAN(x)     (atan(x))
# define BOR_POW(x, y)   (pow((x), (y)))
# define BOR_EXP(x)      (exp(x))
# define BOR_LOG(x)      (log(x))

#endif /* BOR_DOUBLE */

union _bor_real_uint_t {
    bor_real_t f;
    bor_uint_t i;
};
typedef union _bor_real_uint_t bor_real_uint_t;


#define BOR_MIN(x, y) ((x) < (y) ? (x) : (y)) /*!< minimum */
#define BOR_MAX(x, y) ((x) > (y) ? (x) : (y)) /*!< maximum */
#define BOR_SQ(x)     ((x) * (x))             /*!< square */
#define BOR_POWL(x, y) (powl((x), (y)))       /*!< power function */

#define BOR_ONE BOR_REAL(1.)  /*!< unit constant */
#define BOR_ZERO BOR_REAL(0.) /*!< zero contant */


/**
 * Swaps {a} and {b} using given temporary variable {tmp}.
 */
#define BOR_SWAP(a, b, tmp) \
    (tmp) = (a); \
    (a) = (b); \
    (b) = (tmp)


#ifdef BOR_SSE
_bor_inline bor_real_t __borSqrt(bor_real_t val);
#endif /* BOR_SSE */

/**
 * Returns sign of value.
 */
_bor_inline int borSign(bor_real_t val);

/**
 * Returns true if val is zero.
 */
_bor_inline int borIsZero(bor_real_t val);

/**
 * Returns true if a and b equal.
 */
_bor_inline int borEq(bor_real_t a, bor_real_t b);

/**
 * Returns true if a and b not equal.
 */
_bor_inline int borNEq(bor_real_t a, bor_real_t b);

/**
 * Returns 1 / d.
 */
_bor_inline bor_real_t borRecp(bor_real_t v);

/**
 * Returns 1 / sqrt(v)
 */
_bor_inline bor_real_t borRsqrt(bor_real_t v);


/**
 * Alignes given memory.
 */
_bor_inline void *borAlign(void *mem, int alignment);

/**
 * Returns integer representation of real number
 */
_bor_inline bor_uint_t borRealAsUInt(bor_real_t x);

/***** INLINES *****/
#ifdef BOR_SSE
_bor_inline bor_real_t __borSqrt(bor_real_t val)
{
    bor_sse_t m;

#ifdef BOR_SSE_SINGLE
    m.m = _mm_set1_ps(val);
    m.m = _mm_sqrt_ps(m.m);
    return m.f[0];
#else /* BOR_SSE_SINGLE */
    m.md = _mm_set1_pd(val);
    m.md = _mm_sqrt_pd(m.md);
    return m.d[0];
#endif /* BOR_SSE_SINGLE */
}
#endif /* BOR_SSE */

_bor_inline int borSign(bor_real_t val)
{
    if (borIsZero(val)){
        return 0;
    }else if (val < BOR_ZERO){
        return -1;
    }
    return 1;
}

_bor_inline int borIsZero(bor_real_t val)
{
    return BOR_FABS(val) < BOR_EPS;
}

_bor_inline int borEq(bor_real_t _a, bor_real_t _b)
{
    bor_real_t ab;

    ab = BOR_FABS(_a - _b);
    if (ab < BOR_EPS)
        return 1;

    bor_real_t a, b;
    a = BOR_FABS(_a);
    b = BOR_FABS(_b);
    if (b > a){
        return ab < BOR_EPS * b;
    }else{
        return ab < BOR_EPS * a;
    }
}

_bor_inline int borNEq(bor_real_t a, bor_real_t b)
{
    return !borEq(a, b);
}

_bor_inline bor_real_t borRecp(bor_real_t v)
{
#ifdef BOR_SSE_SINGLE
    bor_sse_t m;
    m.m = _mm_set1_ps(v);
    m.m = _mm_rcp_ps(m.m);
    return m.f[0];
#else /* BOR_SSE_SINGLE */
    return BOR_ONE / v;
#endif /* BOR_SSE_SINGLE */
}

_bor_inline bor_real_t borRsqrt(bor_real_t v)
{
#ifdef BOR_SSE_SINGLE
    bor_sse_t m;
    m.m = _mm_set1_ps(v);
    m.m = _mm_rsqrt_ps(m.m);
    return m.f[0];
#else /* BOR_SSE_SINGLE */
    return BOR_ONE / BOR_SQRT(v);
# endif /* BOR_SSE_SINGLE */
}

_bor_inline void *borAlign(void *mem, int align)
{
    long padding;

    if (align == 0)
        return mem;

    padding = align - (long)mem % align;
    return (void *)((long)mem + padding);
}

_bor_inline bor_uint_t borRealAsUInt(bor_real_t x)
{
    bor_real_uint_t v;
    v.f = x;
    return v.i;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_CORE_H__ */
