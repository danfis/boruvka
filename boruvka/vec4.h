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

#ifndef __BOR_VEC4_H__
#define __BOR_VEC4_H__

#include <stdio.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec4 - 4D vector
 * =================
 *
 *
 * .. c:type:: bor_vec4_t
 */

/** vvvvv */
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
union _bor_vec4_t {
    __m128 v;
    float f[4];
} bor_aligned(16) bor_packed;
typedef union _bor_vec4_t bor_vec4_t;
# else /* BOR_SSE_SINGLE */
union _bor_vec4_t {
    __m128d v[2];
    double f[4];
} bor_aligned(16) bor_packed;
typedef union _bor_vec4_t bor_vec4_t;
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
struct _bor_vec4_t {
    bor_real_t f[4];
};
typedef struct _bor_vec4_t bor_vec4_t;
#endif /* BOR_SSE */
/** ^^^^^ */


/**
 * Holds origin (0,0,0,0) - this variable is meant to be read-only!
 */
extern const bor_vec4_t *bor_vec4_origin;


# define BOR_VEC4_STATIC(x, y, z, w) \
    { .f = { (x), (y), (z), (w) } }

#define BOR_VEC4(name, x, y, z, w) \
    bor_vec4_t name = BOR_VEC4_STATIC((x), (y), (z), (w))

/**
 * Functions
 * ----------
 */

/**
 * Allocate and initialize new vector.
 */
bor_vec4_t *borVec4New(bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w);

/**
 * Delete vector.
 */
void borVec4Del(bor_vec4_t *);

/**
 * Allocates array of vectors.
 * Use this function instead of classical malloc() due to alignement that
 * could be required if you use SSE version of library.
 * Use borVec4ArrDel() to free allocated memory.
 */
bor_vec4_t *borVec4ArrNew(size_t num_vecs);
void borVec4ArrDel(bor_vec4_t *);

/**
 * Returns memory aligned to vector size.
 * This can be useful if you have somewhere allocated memory but you want
 * to use it for vectors and you use SSE version of library.
 */
_bor_inline bor_vec4_t *borVec4Align(void *mem);

/**
 * Clone given bor_vec4_t. This does deep copy.
 */
_bor_inline bor_vec4_t *borVec4Clone(const bor_vec4_t *v);

/**
 * v = w
 */
_bor_inline void borVec4Copy(bor_vec4_t *v, const bor_vec4_t *w);


_bor_inline bor_real_t borVec4X(const bor_vec4_t *v);
_bor_inline bor_real_t borVec4Y(const bor_vec4_t *v);
_bor_inline bor_real_t borVec4Z(const bor_vec4_t *v);
_bor_inline bor_real_t borVec4W(const bor_vec4_t *v);
_bor_inline bor_real_t borVec4Get(const bor_vec4_t *v, int d);

_bor_inline void borVec4SetX(bor_vec4_t *v, bor_real_t val);
_bor_inline void borVec4SetY(bor_vec4_t *v, bor_real_t val);
_bor_inline void borVec4SetZ(bor_vec4_t *v, bor_real_t val);
_bor_inline void borVec4SetW(bor_vec4_t *v, bor_real_t val);
_bor_inline void borVec4Set(bor_vec4_t *v, bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w);
_bor_inline void borVec4SetCoord(bor_vec4_t *v, int d, bor_real_t val);


/**
 * Returns true if a and b equal.
 */
_bor_inline int borVec4Eq(const bor_vec4_t *a, const bor_vec4_t *b);
_bor_inline int borVec4NEq(const bor_vec4_t *a, const bor_vec4_t *b);
_bor_inline int borVec4Eq2(const bor_vec4_t *a, bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w);
_bor_inline int borVec4NEq2(const bor_vec4_t *a, bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w);
_bor_inline int borVec4IsZero(const bor_vec4_t *a);


/**
 * Returns squared length of vector.
 */
_bor_inline bor_real_t borVec4Len2(const bor_vec4_t *v);
_bor_inline bor_real_t borVec4Len(const bor_vec4_t *v);

/**
 * Returns squared distance between a and b.
 */
_bor_inline bor_real_t borVec4Dist2(const bor_vec4_t *a, const bor_vec4_t *b);
_bor_inline bor_real_t borVec4Dist(const bor_vec4_t *a, const bor_vec4_t *b);


/**
 * Adds coordinates of vector w to vector v.
 * v = v + w
 */
_bor_inline void borVec4Add(bor_vec4_t *v, const bor_vec4_t *w);

/**
 * d = v + w
 */
_bor_inline void borVec4Add2(bor_vec4_t *d, const bor_vec4_t *v, const bor_vec4_t *w);


/**
 * Substracts coordinates of vector w from vector v.
 * v = v - w
 */
_bor_inline void borVec4Sub(bor_vec4_t *v, const bor_vec4_t *w);

/**
 * d = v - w
 */
_bor_inline void borVec4Sub2(bor_vec4_t *d, const bor_vec4_t *v, const bor_vec4_t *w);

_bor_inline void borVec4AddConst(bor_vec4_t *v, bor_real_t f);
_bor_inline void borVec4AddConst2(bor_vec4_t *d, const bor_vec4_t *v, bor_real_t f);
_bor_inline void borVec4SubConst(bor_vec4_t *v, bor_real_t f);
_bor_inline void borVec4SubConst2(bor_vec4_t *d, const bor_vec4_t *v, bor_real_t f);

/**
 * d = d * k;
 */
_bor_inline void borVec4Scale(bor_vec4_t *d, bor_real_t k);

/**
 * d = v * k;
 */
_bor_inline void borVec4Scale2(bor_vec4_t *d, const bor_vec4_t *v, bor_real_t k);

/**
 * Scales vector v to given length.
 */
_bor_inline void borVec4ScaleToLen(bor_vec4_t *v, bor_real_t len);

/**
 * Normalizes given vector to unit length.
 */
_bor_inline void borVec4Normalize(bor_vec4_t *d);


/**
 * Dot product of two vectors.
 */
_bor_inline bor_real_t borVec4Dot(const bor_vec4_t *a, const bor_vec4_t *b);

/**
 * Multiplies vectors component wise:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 * a.z = a.z * b.z
 * a.w = a.w * b.w
 */
_bor_inline void borVec4MulComp(bor_vec4_t *a, const bor_vec4_t *b);

/**
 * a.x = b.x * c.x
 * a.y = b.y * c.y
 * a.z = b.z * c.z
 * a.w = b.w * c.w
 */
_bor_inline void borVec4MulComp2(bor_vec4_t *a, const bor_vec4_t *b, const bor_vec4_t *c);


/**
 * Prints vector to *out* in form "x y z w".
 */
_bor_inline void borVec4Print(const bor_vec4_t *v, FILE *out);



/**** INLINES ****/
_bor_inline bor_vec4_t *borVec4Align(void *mem)
{
#ifdef BOR_SSE
    return (bor_vec4_t *)borAlign(mem, 16);
#else /* BOR_SSE */
    return (bor_vec4_t *)mem;
#endif /* BOR_SSE */
}

_bor_inline bor_vec4_t *borVec4Clone(const bor_vec4_t *v)
{
    return borVec4New(borVec4X(v), borVec4Y(v), borVec4Z(v), borVec4W(v));
}

_bor_inline bor_real_t borVec4Get(const bor_vec4_t *v, int d)
{
    return v->f[d];
}

_bor_inline bor_real_t borVec4X(const bor_vec4_t *v)
{
    return v->f[0];
}

_bor_inline bor_real_t borVec4Y(const bor_vec4_t *v)
{
    return v->f[1];
}

_bor_inline bor_real_t borVec4Z(const bor_vec4_t *v)
{
    return v->f[2];
}

_bor_inline bor_real_t borVec4W(const bor_vec4_t *v)
{
    return v->f[3];
}

_bor_inline int borVec4Eq(const bor_vec4_t *a, const bor_vec4_t *b)
{
    return borEq(borVec4X(a), borVec4X(b))
            && borEq(borVec4Y(a), borVec4Y(b))
            && borEq(borVec4Z(a), borVec4Z(b))
            && borEq(borVec4W(a), borVec4W(b));
}

_bor_inline int borVec4NEq(const bor_vec4_t *a, const bor_vec4_t *b)
{
    return !borVec4Eq(a, b);
}

_bor_inline int borVec4Eq2(const bor_vec4_t *a, bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w)
{
    return borEq(borVec4X(a), x)
            && borEq(borVec4Y(a), y)
            && borEq(borVec4Z(a), z)
            && borEq(borVec4W(a), w);
}

_bor_inline int borVec4NEq2(const bor_vec4_t *a, bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w)
{
    return !borVec4Eq2(a, x, y, z, w);
}

_bor_inline int borVec4IsZero(const bor_vec4_t *a)
{
    return borIsZero(borVec4X(a))
            && borIsZero(borVec4Y(a))
            && borIsZero(borVec4Z(a))
            && borIsZero(borVec4W(a));
}

_bor_inline bor_real_t borVec4Len2(const bor_vec4_t *v)
{
    return borVec4Dot(v, v);
}
_bor_inline bor_real_t borVec4Len(const bor_vec4_t *v)
{
    return BOR_SQRT(borVec4Len2(v));
}

_bor_inline bor_real_t borVec4Dist2(const bor_vec4_t *a, const bor_vec4_t *b)
{
    bor_vec4_t ab;
    borVec4Sub2(&ab, a, b);
    return borVec4Len2(&ab);
}
_bor_inline bor_real_t borVec4Dist(const bor_vec4_t *a, const bor_vec4_t *b)
{
    return BOR_SQRT(borVec4Dist2(a, b));
}

_bor_inline void borVec4Set(bor_vec4_t *v, bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w)
{
    v->f[0] = x;
    v->f[1] = y;
    v->f[2] = z;
    v->f[3] = w;
}
_bor_inline void borVec4SetCoord(bor_vec4_t *v, int d, bor_real_t val)
{
    v->f[d] = val;
}

_bor_inline void borVec4SetX(bor_vec4_t *v, bor_real_t val)
{
    v->f[0] = val;
}

_bor_inline void borVec4SetY(bor_vec4_t *v, bor_real_t val)
{
    v->f[1] = val;
}

_bor_inline void borVec4SetZ(bor_vec4_t *v, bor_real_t val)
{
    v->f[2] = val;
}

_bor_inline void borVec4SetW(bor_vec4_t *v, bor_real_t val)
{
    v->f[3] = val;
}

_bor_inline void borVec4Copy(bor_vec4_t *v, const bor_vec4_t *w)
{
    *v = *w;
}

_bor_inline void borVec4Add(bor_vec4_t *v, const bor_vec4_t *w)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    v->v = _mm_add_ps(v->v, w->v);
# else /* BOR_SSE_SINGLE */
    v->v[0] = _mm_add_pd(v->v[0], w->v[0]);
    v->v[1] = _mm_add_pd(v->v[1], w->v[1]);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    v->f[0] += w->f[0];
    v->f[1] += w->f[1];
    v->f[2] += w->f[2];
    v->f[3] += w->f[3];
#endif /* BOR_SSE */
}

_bor_inline void borVec4Add2(bor_vec4_t *d, const bor_vec4_t *v, const bor_vec4_t *w)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    d->v = _mm_add_ps(v->v, w->v);
# else /* BOR_SSE_SINGLE */
    d->v[0] = _mm_add_pd(v->v[0], w->v[0]);
    d->v[1] = _mm_add_pd(v->v[1], w->v[1]);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    d->f[0] = v->f[0] + w->f[0];
    d->f[1] = v->f[1] + w->f[1];
    d->f[2] = v->f[2] + w->f[2];
    d->f[3] = v->f[3] + w->f[3];
#endif /* BOR_SSE */
}

_bor_inline void borVec4Sub(bor_vec4_t *v, const bor_vec4_t *w)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    v->v = _mm_sub_ps(v->v, w->v);
# else /* BOR_SSE_SINGLE */
    v->v[0] = _mm_sub_pd(v->v[0], w->v[0]);
    v->v[1] = _mm_sub_pd(v->v[1], w->v[1]);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    v->f[0] -= w->f[0];
    v->f[1] -= w->f[1];
    v->f[2] -= w->f[2];
    v->f[3] -= w->f[3];
#endif /* BOR_SSE */
}
_bor_inline void borVec4Sub2(bor_vec4_t *d, const bor_vec4_t *v, const bor_vec4_t *w)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    d->v = _mm_sub_ps(v->v, w->v);
# else /* BOR_SSE_SINGLE */
    d->v[0] = _mm_sub_pd(v->v[0], w->v[0]);
    d->v[1] = _mm_sub_pd(v->v[1], w->v[1]);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    d->f[0] = v->f[0] - w->f[0];
    d->f[1] = v->f[1] - w->f[1];
    d->f[2] = v->f[2] - w->f[2];
    d->f[3] = v->f[3] - w->f[3];
#endif /* BOR_SSE */
}

_bor_inline void borVec4AddConst(bor_vec4_t *v, bor_real_t f)
{
    v->f[0] += f;
    v->f[1] += f;
    v->f[2] += f;
    v->f[3] += f;
}

_bor_inline void borVec4AddConst2(bor_vec4_t *d, const bor_vec4_t *v, bor_real_t f)
{
    d->f[0] = v->f[0] + f;
    d->f[1] = v->f[1] + f;
    d->f[2] = v->f[2] + f;
    d->f[3] = v->f[3] + f;
}

_bor_inline void borVec4SubConst(bor_vec4_t *v, bor_real_t f)
{
    v->f[0] -= f;
    v->f[1] -= f;
    v->f[2] -= f;
    v->f[3] -= f;
}

_bor_inline void borVec4SubConst2(bor_vec4_t *d, const bor_vec4_t *v, bor_real_t f)
{
    d->f[0] = v->f[0] - f;
    d->f[1] = v->f[1] - f;
    d->f[2] = v->f[2] - f;
    d->f[3] = v->f[3] - f;
}


_bor_inline void borVec4Scale(bor_vec4_t *d, bor_real_t _k)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec4_t k;
    k.v = _mm_set1_ps(_k);
    d->v = _mm_mul_ps(d->v, k.v);
# else /* BOR_SSE_SINGLE */
    __m128d k;
    k = _mm_set1_pd(_k);
    d->v[0] = _mm_mul_pd(d->v[0], k);
    d->v[1] = _mm_mul_pd(d->v[1], k);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    d->f[0] *= _k;
    d->f[1] *= _k;
    d->f[2] *= _k;
    d->f[3] *= _k;
#endif /* BOR_SSE */
}

_bor_inline void borVec4Scale2(bor_vec4_t *d, const bor_vec4_t *a, bor_real_t _k)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec4_t k;
    k.v = _mm_set1_ps(_k);
    d->v = _mm_mul_ps(a->v, k.v);
# else /* BOR_SSE_SINGLE */
    __m128d k;
    k = _mm_set1_pd(_k);
    d->v[0] = _mm_mul_pd(a->v[0], k);
    d->v[1] = _mm_mul_pd(a->v[1], k);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    d->f[0] = a->f[0] * _k;
    d->f[1] = a->f[1] * _k;
    d->f[2] = a->f[2] * _k;
    d->f[3] = a->f[3] * _k;
#endif /* BOR_SSE */
}

_bor_inline void borVec4ScaleToLen(bor_vec4_t *v, bor_real_t len)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec4_t k, l;

    k.v = _mm_set1_ps(borVec4Len2(v));
    k.v = _mm_sqrt_ps(k.v);
    l.v = _mm_set1_ps(len);
    k.v = _mm_div_ps(k.v, l.v);
    v->v = _mm_div_ps(v->v, k.v);
# else /* BOR_SSE_SINGLE */
    __m128d k, l;

    k = _mm_set1_pd(borVec4Len2(v));
    k = _mm_sqrt_pd(k);
    l = _mm_set1_pd(len);
    k = _mm_div_pd(k, l);
    v->v[0] = _mm_div_pd(v->v[0], k);
    v->v[1] = _mm_div_pd(v->v[1], k);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    bor_real_t k = len * borRsqrt(borVec4Len2(v));
    borVec4Scale(v, k);
#endif /* BOR_SSE */
}

_bor_inline void borVec4Normalize(bor_vec4_t *d)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec4_t k;

    k.v = _mm_set1_ps(borVec4Len2(d));
    k.v = _mm_sqrt_ps(k.v);
    d->v = _mm_div_ps(d->v, k.v);
# else /* BOR_SSE_SINGLE */
    __m128d k;

    k = _mm_set1_pd(borVec4Len2(d));
    k = _mm_sqrt_pd(k);
    d->v[0] = _mm_div_pd(d->v[0], k);
    d->v[1] = _mm_div_pd(d->v[1], k);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    bor_real_t k = borRsqrt(borVec4Len2(d));
    borVec4Scale(d, k);
#endif /* BOR_SSE */
}

_bor_inline bor_real_t borVec4Dot(const bor_vec4_t *a, const bor_vec4_t *b)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec4_t dot, t;

    dot.v = _mm_mul_ps(a->v, b->v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(2, 3, 0, 1));
    dot.v = _mm_add_ps(dot.v, t.v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(1, 0, 3, 2));
    dot.v = _mm_add_ps(dot.v, t.v);

    return dot.f[0];
# else /* BOR_SSE_SINGLE */
    bor_vec4_t dot;

    dot.v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    dot.v[1] = _mm_mul_pd(a->v[1], b->v[1]);
    dot.v[0] = _mm_add_pd(dot.v[0], dot.v[1]);
    dot.v[1] = _mm_shuffle_pd(dot.v[0], dot.v[0], _MM_SHUFFLE2(0, 1));
    dot.v[0] = _mm_add_pd(dot.v[0], dot.v[1]);

    return dot.f[0];
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    bor_real_t dot;

    dot  = a->f[0] * b->f[0];
    dot += a->f[1] * b->f[1];
    dot += a->f[2] * b->f[2];
    dot += a->f[3] * b->f[3];

    return dot;
#endif /* BOR_SSE */
}

_bor_inline void borVec4MulComp(bor_vec4_t *a, const bor_vec4_t *b)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    a->v = _mm_mul_ps(a->v, b->v);
# else /* BOR_SSE_SINGLE */
    a->v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    a->v[1] = _mm_mul_pd(a->v[1], b->v[1]);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    a->f[0] *= b->f[0];
    a->f[1] *= b->f[1];
    a->f[2] *= b->f[2];
    a->f[3] *= b->f[3];
#endif /* BOR_SSE */
}

_bor_inline void borVec4MulComp2(bor_vec4_t *a, const bor_vec4_t *b, const bor_vec4_t *c)
{
    borVec4Copy(a, b);
    borVec4MulComp(a, c);
}


_bor_inline void borVec4Print(const bor_vec4_t *v, FILE *out)
{
    fprintf(out, "%g %g %g %g",
            (double)borVec4X(v), (double)borVec4Y(v),
            (double)borVec4Z(v), (double)borVec4W(v));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_VEC4_H__ */
