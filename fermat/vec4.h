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

#ifndef __FER_VEC4_H__
#define __FER_VEC4_H__

#include <stdio.h>
#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec4 - 4D vector
 * =================
 *
 *
 * .. c:type:: fer_vec4_t
 */

/** vvvvv */
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
union _fer_vec4_t {
    __m128 v;
    float f[4];
} fer_aligned(16) fer_packed;
typedef union _fer_vec4_t fer_vec4_t;
# else /* FER_SSE_SINGLE */
union _fer_vec4_t {
    __m128d v[2];
    double f[4];
} fer_aligned(16) fer_packed;
typedef union _fer_vec4_t fer_vec4_t;
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
struct _fer_vec4_t {
    fer_real_t f[4];
};
typedef struct _fer_vec4_t fer_vec4_t;
#endif /* FER_SSE */
/** ^^^^^ */


/**
 * Holds origin (0,0,0,0) - this variable is meant to be read-only!
 */
extern const fer_vec4_t *fer_vec4_origin;


# define FER_VEC4_STATIC(x, y, z, w) \
    { .f = { (x), (y), (z), (w) } }

#define FER_VEC4(name, x, y, z, w) \
    fer_vec4_t name = FER_VEC4_STATIC((x), (y), (z), (w))

/**
 * Functions
 * ----------
 */

/**
 * Allocate and initialize new vector.
 */
fer_vec4_t *ferVec4New(fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w);

/**
 * Delete vector.
 */
void ferVec4Del(fer_vec4_t *);

/**
 * Allocates array of vectors.
 * Use this function instead of classical malloc() due to alignement that
 * could be required if you use SSE version of library.
 * Use ferVec4ArrDel() to free allocated memory.
 */
fer_vec4_t *ferVec4ArrNew(size_t num_vecs);
void ferVec4ArrDel(fer_vec4_t *);

/**
 * Returns memory aligned to vector size.
 * This can be useful if you have somewhere allocated memory but you want
 * to use it for vectors and you use SSE version of library.
 */
_fer_inline fer_vec4_t *ferVec4Align(void *mem);

/**
 * Clone given fer_vec4_t. This does deep copy.
 */
_fer_inline fer_vec4_t *ferVec4Clone(const fer_vec4_t *v);

/**
 * v = w
 */
_fer_inline void ferVec4Copy(fer_vec4_t *v, const fer_vec4_t *w);


_fer_inline fer_real_t ferVec4X(const fer_vec4_t *v);
_fer_inline fer_real_t ferVec4Y(const fer_vec4_t *v);
_fer_inline fer_real_t ferVec4Z(const fer_vec4_t *v);
_fer_inline fer_real_t ferVec4W(const fer_vec4_t *v);
_fer_inline fer_real_t ferVec4Get(const fer_vec4_t *v, int d);

_fer_inline void ferVec4SetX(fer_vec4_t *v, fer_real_t val);
_fer_inline void ferVec4SetY(fer_vec4_t *v, fer_real_t val);
_fer_inline void ferVec4SetZ(fer_vec4_t *v, fer_real_t val);
_fer_inline void ferVec4SetW(fer_vec4_t *v, fer_real_t val);
_fer_inline void ferVec4Set(fer_vec4_t *v, fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w);
_fer_inline void ferVec4SetCoord(fer_vec4_t *v, int d, fer_real_t val);


/**
 * Returns true if a and b equal.
 */
_fer_inline int ferVec4Eq(const fer_vec4_t *a, const fer_vec4_t *b);
_fer_inline int ferVec4NEq(const fer_vec4_t *a, const fer_vec4_t *b);
_fer_inline int ferVec4Eq2(const fer_vec4_t *a, fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w);
_fer_inline int ferVec4NEq2(const fer_vec4_t *a, fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w);
_fer_inline int ferVec4IsZero(const fer_vec4_t *a);


/**
 * Returns squared length of vector.
 */
_fer_inline fer_real_t ferVec4Len2(const fer_vec4_t *v);
_fer_inline fer_real_t ferVec4Len(const fer_vec4_t *v);

/**
 * Returns squared distance between a and b.
 */
_fer_inline fer_real_t ferVec4Dist2(const fer_vec4_t *a, const fer_vec4_t *b);
_fer_inline fer_real_t ferVec4Dist(const fer_vec4_t *a, const fer_vec4_t *b);


/**
 * Adds coordinates of vector w to vector v.
 * v = v + w
 */
_fer_inline void ferVec4Add(fer_vec4_t *v, const fer_vec4_t *w);

/**
 * d = v + w
 */
_fer_inline void ferVec4Add2(fer_vec4_t *d, const fer_vec4_t *v, const fer_vec4_t *w);


/**
 * Substracts coordinates of vector w from vector v.
 * v = v - w
 */
_fer_inline void ferVec4Sub(fer_vec4_t *v, const fer_vec4_t *w);

/**
 * d = v - w
 */
_fer_inline void ferVec4Sub2(fer_vec4_t *d, const fer_vec4_t *v, const fer_vec4_t *w);

_fer_inline void ferVec4AddConst(fer_vec4_t *v, fer_real_t f);
_fer_inline void ferVec4AddConst2(fer_vec4_t *d, const fer_vec4_t *v, fer_real_t f);
_fer_inline void ferVec4SubConst(fer_vec4_t *v, fer_real_t f);
_fer_inline void ferVec4SubConst2(fer_vec4_t *d, const fer_vec4_t *v, fer_real_t f);

/**
 * d = d * k;
 */
_fer_inline void ferVec4Scale(fer_vec4_t *d, fer_real_t k);

/**
 * d = v * k;
 */
_fer_inline void ferVec4Scale2(fer_vec4_t *d, const fer_vec4_t *v, fer_real_t k);

/**
 * Scales vector v to given length.
 */
_fer_inline void ferVec4ScaleToLen(fer_vec4_t *v, fer_real_t len);

/**
 * Normalizes given vector to unit length.
 */
_fer_inline void ferVec4Normalize(fer_vec4_t *d);


/**
 * Dot product of two vectors.
 */
_fer_inline fer_real_t ferVec4Dot(const fer_vec4_t *a, const fer_vec4_t *b);

/**
 * Multiplies vectors component wise:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 * a.z = a.z * b.z
 * a.w = a.w * b.w
 */
_fer_inline void ferVec4MulComp(fer_vec4_t *a, const fer_vec4_t *b);

/**
 * a.x = b.x * c.x
 * a.y = b.y * c.y
 * a.z = b.z * c.z
 * a.w = b.w * c.w
 */
_fer_inline void ferVec4MulComp2(fer_vec4_t *a, const fer_vec4_t *b, const fer_vec4_t *c);


/**
 * Prints vector to *out* in form "x y z w".
 */
_fer_inline void ferVec4Print(const fer_vec4_t *v, FILE *out);



/**** INLINES ****/
_fer_inline fer_vec4_t *ferVec4Align(void *mem)
{
#ifdef FER_SSE
    return (fer_vec4_t *)ferAlign(mem, 16);
#else /* FER_SSE */
    return (fer_vec4_t *)mem;
#endif /* FER_SSE */
}

_fer_inline fer_vec4_t *ferVec4Clone(const fer_vec4_t *v)
{
    return ferVec4New(ferVec4X(v), ferVec4Y(v), ferVec4Z(v), ferVec4W(v));
}

_fer_inline fer_real_t ferVec4Get(const fer_vec4_t *v, int d)
{
    return v->f[d];
}

_fer_inline fer_real_t ferVec4X(const fer_vec4_t *v)
{
    return v->f[0];
}

_fer_inline fer_real_t ferVec4Y(const fer_vec4_t *v)
{
    return v->f[1];
}

_fer_inline fer_real_t ferVec4Z(const fer_vec4_t *v)
{
    return v->f[2];
}

_fer_inline fer_real_t ferVec4W(const fer_vec4_t *v)
{
    return v->f[3];
}

_fer_inline int ferVec4Eq(const fer_vec4_t *a, const fer_vec4_t *b)
{
    return ferEq(ferVec4X(a), ferVec4X(b))
            && ferEq(ferVec4Y(a), ferVec4Y(b))
            && ferEq(ferVec4Z(a), ferVec4Z(b))
            && ferEq(ferVec4W(a), ferVec4W(b));
}

_fer_inline int ferVec4NEq(const fer_vec4_t *a, const fer_vec4_t *b)
{
    return !ferVec4Eq(a, b);
}

_fer_inline int ferVec4Eq2(const fer_vec4_t *a, fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w)
{
    return ferEq(ferVec4X(a), x)
            && ferEq(ferVec4Y(a), y)
            && ferEq(ferVec4Z(a), z)
            && ferEq(ferVec4W(a), w);
}

_fer_inline int ferVec4NEq2(const fer_vec4_t *a, fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w)
{
    return !ferVec4Eq2(a, x, y, z, w);
}

_fer_inline int ferVec4IsZero(const fer_vec4_t *a)
{
    return ferIsZero(ferVec4X(a))
            && ferIsZero(ferVec4Y(a))
            && ferIsZero(ferVec4Z(a))
            && ferIsZero(ferVec4W(a));
}

_fer_inline fer_real_t ferVec4Len2(const fer_vec4_t *v)
{
    return ferVec4Dot(v, v);
}
_fer_inline fer_real_t ferVec4Len(const fer_vec4_t *v)
{
    return FER_SQRT(ferVec4Len2(v));
}

_fer_inline fer_real_t ferVec4Dist2(const fer_vec4_t *a, const fer_vec4_t *b)
{
    fer_vec4_t ab;
    ferVec4Sub2(&ab, a, b);
    return ferVec4Len2(&ab);
}
_fer_inline fer_real_t ferVec4Dist(const fer_vec4_t *a, const fer_vec4_t *b)
{
    return FER_SQRT(ferVec4Dist2(a, b));
}

_fer_inline void ferVec4Set(fer_vec4_t *v, fer_real_t x, fer_real_t y, fer_real_t z, fer_real_t w)
{
    v->f[0] = x;
    v->f[1] = y;
    v->f[2] = z;
    v->f[3] = w;
}
_fer_inline void ferVec4SetCoord(fer_vec4_t *v, int d, fer_real_t val)
{
    v->f[d] = val;
}

_fer_inline void ferVec4SetX(fer_vec4_t *v, fer_real_t val)
{
    v->f[0] = val;
}

_fer_inline void ferVec4SetY(fer_vec4_t *v, fer_real_t val)
{
    v->f[1] = val;
}

_fer_inline void ferVec4SetZ(fer_vec4_t *v, fer_real_t val)
{
    v->f[2] = val;
}

_fer_inline void ferVec4SetW(fer_vec4_t *v, fer_real_t val)
{
    v->f[3] = val;
}

_fer_inline void ferVec4Copy(fer_vec4_t *v, const fer_vec4_t *w)
{
    *v = *w;
}

_fer_inline void ferVec4Add(fer_vec4_t *v, const fer_vec4_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    v->v = _mm_add_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    v->v[0] = _mm_add_pd(v->v[0], w->v[0]);
    v->v[1] = _mm_add_pd(v->v[1], w->v[1]);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    v->f[0] += w->f[0];
    v->f[1] += w->f[1];
    v->f[2] += w->f[2];
    v->f[3] += w->f[3];
#endif /* FER_SSE */
}

_fer_inline void ferVec4Add2(fer_vec4_t *d, const fer_vec4_t *v, const fer_vec4_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    d->v = _mm_add_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    d->v[0] = _mm_add_pd(v->v[0], w->v[0]);
    d->v[1] = _mm_add_pd(v->v[1], w->v[1]);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = v->f[0] + w->f[0];
    d->f[1] = v->f[1] + w->f[1];
    d->f[2] = v->f[2] + w->f[2];
    d->f[3] = v->f[3] + w->f[3];
#endif /* FER_SSE */
}

_fer_inline void ferVec4Sub(fer_vec4_t *v, const fer_vec4_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    v->v = _mm_sub_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    v->v[0] = _mm_sub_pd(v->v[0], w->v[0]);
    v->v[1] = _mm_sub_pd(v->v[1], w->v[1]);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    v->f[0] -= w->f[0];
    v->f[1] -= w->f[1];
    v->f[2] -= w->f[2];
    v->f[3] -= w->f[3];
#endif /* FER_SSE */
}
_fer_inline void ferVec4Sub2(fer_vec4_t *d, const fer_vec4_t *v, const fer_vec4_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    d->v = _mm_sub_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    d->v[0] = _mm_sub_pd(v->v[0], w->v[0]);
    d->v[1] = _mm_sub_pd(v->v[1], w->v[1]);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = v->f[0] - w->f[0];
    d->f[1] = v->f[1] - w->f[1];
    d->f[2] = v->f[2] - w->f[2];
    d->f[3] = v->f[3] - w->f[3];
#endif /* FER_SSE */
}

_fer_inline void ferVec4AddConst(fer_vec4_t *v, fer_real_t f)
{
    v->f[0] += f;
    v->f[1] += f;
    v->f[2] += f;
    v->f[3] += f;
}

_fer_inline void ferVec4AddConst2(fer_vec4_t *d, const fer_vec4_t *v, fer_real_t f)
{
    d->f[0] = v->f[0] + f;
    d->f[1] = v->f[1] + f;
    d->f[2] = v->f[2] + f;
    d->f[3] = v->f[3] + f;
}

_fer_inline void ferVec4SubConst(fer_vec4_t *v, fer_real_t f)
{
    v->f[0] -= f;
    v->f[1] -= f;
    v->f[2] -= f;
    v->f[3] -= f;
}

_fer_inline void ferVec4SubConst2(fer_vec4_t *d, const fer_vec4_t *v, fer_real_t f)
{
    d->f[0] = v->f[0] - f;
    d->f[1] = v->f[1] - f;
    d->f[2] = v->f[2] - f;
    d->f[3] = v->f[3] - f;
}


_fer_inline void ferVec4Scale(fer_vec4_t *d, fer_real_t _k)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec4_t k;
    k.v = _mm_set1_ps(_k);
    d->v = _mm_mul_ps(d->v, k.v);
# else /* FER_SSE_SINGLE */
    __m128d k;
    k = _mm_set1_pd(_k);
    d->v[0] = _mm_mul_pd(d->v[0], k);
    d->v[1] = _mm_mul_pd(d->v[1], k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] *= _k;
    d->f[1] *= _k;
    d->f[2] *= _k;
    d->f[3] *= _k;
#endif /* FER_SSE */
}

_fer_inline void ferVec4Scale2(fer_vec4_t *d, const fer_vec4_t *a, fer_real_t _k)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec4_t k;
    k.v = _mm_set1_ps(_k);
    d->v = _mm_mul_ps(a->v, k.v);
# else /* FER_SSE_SINGLE */
    __m128d k;
    k = _mm_set1_pd(_k);
    d->v[0] = _mm_mul_pd(a->v[0], k);
    d->v[1] = _mm_mul_pd(a->v[1], k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = a->f[0] * _k;
    d->f[1] = a->f[1] * _k;
    d->f[2] = a->f[2] * _k;
    d->f[3] = a->f[3] * _k;
#endif /* FER_SSE */
}

_fer_inline void ferVec4ScaleToLen(fer_vec4_t *v, fer_real_t len)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec4_t k, l;

    k.v = _mm_set1_ps(ferVec4Len2(v));
    k.v = _mm_sqrt_ps(k.v);
    l.v = _mm_set1_ps(len);
    k.v = _mm_div_ps(k.v, l.v);
    v->v = _mm_div_ps(v->v, k.v);
# else /* FER_SSE_SINGLE */
    __m128d k, l;

    k = _mm_set1_pd(ferVec4Len2(v));
    k = _mm_sqrt_pd(k);
    l = _mm_set1_pd(len);
    k = _mm_div_pd(k, l);
    v->v[0] = _mm_div_pd(v->v[0], k);
    v->v[1] = _mm_div_pd(v->v[1], k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    fer_real_t k = len * ferRsqrt(ferVec4Len2(v));
    ferVec4Scale(v, k);
#endif /* FER_SSE */
}

_fer_inline void ferVec4Normalize(fer_vec4_t *d)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec4_t k;

    k.v = _mm_set1_ps(ferVec4Len2(d));
    k.v = _mm_sqrt_ps(k.v);
    d->v = _mm_div_ps(d->v, k.v);
# else /* FER_SSE_SINGLE */
    __m128d k;

    k = _mm_set1_pd(ferVec4Len2(d));
    k = _mm_sqrt_pd(k);
    d->v[0] = _mm_div_pd(d->v[0], k);
    d->v[1] = _mm_div_pd(d->v[1], k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    fer_real_t k = ferRsqrt(ferVec4Len2(d));
    ferVec4Scale(d, k);
#endif /* FER_SSE */
}

_fer_inline fer_real_t ferVec4Dot(const fer_vec4_t *a, const fer_vec4_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec4_t dot, t;

    dot.v = _mm_mul_ps(a->v, b->v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(2, 3, 0, 1));
    dot.v = _mm_add_ps(dot.v, t.v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(1, 0, 3, 2));
    dot.v = _mm_add_ps(dot.v, t.v);

    return dot.f[0];
# else /* FER_SSE_SINGLE */
    fer_vec4_t dot;

    dot.v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    dot.v[1] = _mm_mul_pd(a->v[1], b->v[1]);
    dot.v[0] = _mm_add_pd(dot.v[0], dot.v[1]);
    dot.v[1] = _mm_shuffle_pd(dot.v[0], dot.v[0], _MM_SHUFFLE2(0, 1));
    dot.v[0] = _mm_add_pd(dot.v[0], dot.v[1]);

    return dot.f[0];
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    fer_real_t dot;

    dot  = a->f[0] * b->f[0];
    dot += a->f[1] * b->f[1];
    dot += a->f[2] * b->f[2];
    dot += a->f[3] * b->f[3];

    return dot;
#endif /* FER_SSE */
}

_fer_inline void ferVec4MulComp(fer_vec4_t *a, const fer_vec4_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    a->v = _mm_mul_ps(a->v, b->v);
# else /* FER_SSE_SINGLE */
    a->v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    a->v[1] = _mm_mul_pd(a->v[1], b->v[1]);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    a->f[0] *= b->f[0];
    a->f[1] *= b->f[1];
    a->f[2] *= b->f[2];
    a->f[3] *= b->f[3];
#endif /* FER_SSE */
}

_fer_inline void ferVec4MulComp2(fer_vec4_t *a, const fer_vec4_t *b, const fer_vec4_t *c)
{
    ferVec4Copy(a, b);
    ferVec4MulComp(a, c);
}


_fer_inline void ferVec4Print(const fer_vec4_t *v, FILE *out)
{
    fprintf(out, "%lg %lg %lg %lg",
            (double)ferVec4X(v), (double)ferVec4Y(v),
            (double)ferVec4Z(v), (double)ferVec4W(v));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_VEC4_H__ */
