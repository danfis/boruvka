/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __MG_VEC4_H__
#define __MG_VEC4_H__

#include <mg/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MG_SSE

# ifdef MG_SSE_SINGLE

union _mg_vec4_t {
    __m128 v;
    float f[4];
} mg_aligned(16) mg_packed;
typedef union _mg_vec4_t mg_vec4_t;

# else /* MG_SSE_SINGLE */

union _mg_vec4_t {
    __m128d v[2];
    double f[4];
} mg_aligned(32) mg_packed;
typedef union _mg_vec4_t mg_vec4_t;

# endif /* MG_SSE_SINGLE */

#else /* MG_SSE */
struct _mg_vec4_t {
    mg_real_t f[4];
};
typedef struct _mg_vec4_t mg_vec4_t;
#endif /* MG_SSE */


/**
 * Holds origin (0,0,0,0) - this variable is meant to be read-only!
 */
extern const mg_vec4_t *mg_vec4_origin;


# define MG_VEC4_STATIC(x, y, z, w) \
    { .f = { (x), (y), (z), (w) } }

#define MG_VEC4(name, x, y, z, w) \
    mg_vec4_t name = MG_VEC4_STATIC((x), (y), (z), (w))

/**
 * Allocate and initialize new vector.
 */
mg_vec4_t *mgVec4New(mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w);

/**
 * Delete vector.
 */
void mgVec4Del(mg_vec4_t *);

/**
 * Allocates array of vectors.
 * Use this function instead of classical malloc() due to alignement that
 * could be required if you use SSE version of library.
 * Use mgVec4ArrDel() to free allocated memory.
 */
mg_vec4_t *mgVec4ArrNew(size_t num_vecs);
void mgVec4ArrDel(mg_vec4_t *);

/**
 * Returns memory aligned to vector size.
 * This can be useful if you have somewhere allocated memory but you want
 * to use it for vectors and you use SSE version of library.
 */
_mg_inline mg_vec4_t *mgVec4Align(void *mem);

/**
 * Clone given mg_vec4_t. This does deep copy.
 */
_mg_inline mg_vec4_t *mgVec4Clone(const mg_vec4_t *v);

/**
 * v = w
 */
_mg_inline void mgVec4Copy(mg_vec4_t *v, const mg_vec4_t *w);


_mg_inline mg_real_t mgVec4X(const mg_vec4_t *v);
_mg_inline mg_real_t mgVec4Y(const mg_vec4_t *v);
_mg_inline mg_real_t mgVec4Z(const mg_vec4_t *v);
_mg_inline mg_real_t mgVec4W(const mg_vec4_t *v);
_mg_inline mg_real_t mgVec4Get(const mg_vec4_t *v, int d);

_mg_inline void mgVec4SetX(mg_vec4_t *v, mg_real_t val);
_mg_inline void mgVec4SetY(mg_vec4_t *v, mg_real_t val);
_mg_inline void mgVec4SetZ(mg_vec4_t *v, mg_real_t val);
_mg_inline void mgVec4SetW(mg_vec4_t *v, mg_real_t val);
_mg_inline void mgVec4Set(mg_vec4_t *v, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w);
_mg_inline void mgVec4SetCoord(mg_vec4_t *v, int d, mg_real_t val);


/**
 * Returns true if a and b equal.
 */
_mg_inline int mgVec4Eq(const mg_vec4_t *a, const mg_vec4_t *b);
_mg_inline int mgVec4NEq(const mg_vec4_t *a, const mg_vec4_t *b);
_mg_inline int mgVec4Eq2(const mg_vec4_t *a, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w);
_mg_inline int mgVec4NEq2(const mg_vec4_t *a, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w);
_mg_inline int mgVec4IsZero(const mg_vec4_t *a);


/**
 * Returns squared length of vector.
 */
_mg_inline mg_real_t mgVec4Len2(const mg_vec4_t *v);
_mg_inline mg_real_t mgVec4Len(const mg_vec4_t *v);

/**
 * Returns squared distance between a and b.
 */
_mg_inline mg_real_t mgVec4Dist2(const mg_vec4_t *a, const mg_vec4_t *b);
_mg_inline mg_real_t mgVec4Dist(const mg_vec4_t *a, const mg_vec4_t *b);


/**
 * Adds coordinates of vector w to vector v. v = v + w
 */
_mg_inline void mgVec4Add(mg_vec4_t *v, const mg_vec4_t *w);

/**
 * d = v + w
 */
_mg_inline void mgVec4Add2(mg_vec4_t *d, const mg_vec4_t *v, const mg_vec4_t *w);


/**
 * Substracts coordinates of vector w from vector v. v = v - w
 */
_mg_inline void mgVec4Sub(mg_vec4_t *v, const mg_vec4_t *w);

/**
 * d = v - w
 */
_mg_inline void mgVec4Sub2(mg_vec4_t *d, const mg_vec4_t *v, const mg_vec4_t *w);

/**
 * d = d * k;
 */
_mg_inline void mgVec4Scale(mg_vec4_t *d, mg_real_t k);

/**
 * Scales vector v to given length.
 */
_mg_inline void mgVec4ScaleToLen(mg_vec4_t *v, mg_real_t len);

/**
 * Normalizes given vector to unit length.
 */
_mg_inline void mgVec4Normalize(mg_vec4_t *d);


/**
 * Dot product of two vectors.
 */
_mg_inline mg_real_t mgVec4Dot(const mg_vec4_t *a, const mg_vec4_t *b);

/**
 * Multiplies vectors component wise:
 *  a.x = a.x * b.x
 *  a.y = a.y * b.y
 *  a.z = a.z * b.z
 */
_mg_inline void mgVec4Mul(mg_vec4_t *a, const mg_vec4_t *b);

/**
 * a = [ b.x * c.x, b.y * c.y, b.z * c.z ]
 */
_mg_inline void mgVec4Mul2(mg_vec4_t *a, const mg_vec4_t *b, const mg_vec4_t *c);




/**** INLINES ****/
_mg_inline mg_vec4_t *mgVec4Align(void *mem)
{
#ifdef MG_SSE
    long padding;
    long align;

    align = (long)sizeof(mg_vec4_t);
    padding = align - (long)mem % align;
    return (mg_vec4_t *)((long)mem + padding);
#else /* MG_SSE */
    return (mg_vec4_t *)mem;
#endif /* MG_SSE */
}

_mg_inline mg_vec4_t *mgVec4Clone(const mg_vec4_t *v)
{
    return mgVec4New(mgVec4X(v), mgVec4Y(v), mgVec4Z(v), mgVec4W(v));
}

_mg_inline mg_real_t mgVec4Get(const mg_vec4_t *v, int d)
{
    return v->f[d];
}

_mg_inline mg_real_t mgVec4X(const mg_vec4_t *v)
{
    return v->f[0];
}

_mg_inline mg_real_t mgVec4Y(const mg_vec4_t *v)
{
    return v->f[1];
}

_mg_inline mg_real_t mgVec4Z(const mg_vec4_t *v)
{
    return v->f[2];
}

_mg_inline mg_real_t mgVec4W(const mg_vec4_t *v)
{
    return v->f[3];
}

_mg_inline int mgVec4Eq(const mg_vec4_t *a, const mg_vec4_t *b)
{
    return mgEq(mgVec4X(a), mgVec4X(b))
            && mgEq(mgVec4Y(a), mgVec4Y(b))
            && mgEq(mgVec4Z(a), mgVec4Z(b))
            && mgEq(mgVec4W(a), mgVec4W(b));
}

_mg_inline int mgVec4NEq(const mg_vec4_t *a, const mg_vec4_t *b)
{
    return !mgVec4Eq(a, b);
}

_mg_inline int mgVec4Eq2(const mg_vec4_t *a, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    return mgEq(mgVec4X(a), x)
            && mgEq(mgVec4Y(a), y)
            && mgEq(mgVec4Z(a), z)
            && mgEq(mgVec4W(a), w);
}

_mg_inline int mgVec4NEq2(const mg_vec4_t *a, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    return !mgVec4Eq2(a, x, y, z, w);
}

_mg_inline int mgVec4IsZero(const mg_vec4_t *a)
{
    return mgIsZero(mgVec4X(a))
            && mgIsZero(mgVec4Y(a))
            && mgIsZero(mgVec4Z(a))
            && mgIsZero(mgVec4W(a));
}

_mg_inline mg_real_t mgVec4Len2(const mg_vec4_t *v)
{
    return mgVec4Dot(v, v);
}
_mg_inline mg_real_t mgVec4Len(const mg_vec4_t *v)
{
    return MG_SQRT(mgVec4Len2(v));
}

_mg_inline mg_real_t mgVec4Dist2(const mg_vec4_t *a, const mg_vec4_t *b)
{
    mg_vec4_t ab;
    mgVec4Sub2(&ab, a, b);
    return mgVec4Len2(&ab);
}
_mg_inline mg_real_t mgVec4Dist(const mg_vec4_t *a, const mg_vec4_t *b)
{
    return MG_SQRT(mgVec4Dist2(a, b));
}

_mg_inline void mgVec4Set(mg_vec4_t *v, mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    v->f[0] = x;
    v->f[1] = y;
    v->f[2] = z;
    v->f[3] = w;
}
_mg_inline void mgVec4SetCoord(mg_vec4_t *v, int d, mg_real_t val)
{
    v->f[d] = val;
}

_mg_inline void mgVec4SetX(mg_vec4_t *v, mg_real_t val)
{
    v->f[0] = val;
}

_mg_inline void mgVec4SetY(mg_vec4_t *v, mg_real_t val)
{
    v->f[1] = val;
}

_mg_inline void mgVec4SetZ(mg_vec4_t *v, mg_real_t val)
{
    v->f[2] = val;
}

_mg_inline void mgVec4SetW(mg_vec4_t *v, mg_real_t val)
{
    v->f[3] = val;
}

_mg_inline void mgVec4Copy(mg_vec4_t *v, const mg_vec4_t *w)
{
    *v = *w;
}

_mg_inline void mgVec4Add(mg_vec4_t *v, const mg_vec4_t *w)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    v->v = _mm_add_ps(v->v, w->v);
# else /* MG_SSE_SINGLE */
    v->v[0] = _mm_add_pd(v->v[0], w->v[0]);
    v->v[1] = _mm_add_pd(v->v[1], w->v[1]);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    v->f[0] += w->f[0];
    v->f[1] += w->f[1];
    v->f[2] += w->f[2];
    v->f[3] += w->f[3];
#endif /* MG_SSE */
}

_mg_inline void mgVec4Add2(mg_vec4_t *d, const mg_vec4_t *v, const mg_vec4_t *w)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    d->v = _mm_add_ps(v->v, w->v);
# else /* MG_SSE_SINGLE */
    d->v[0] = _mm_add_pd(v->v[0], w->v[0]);
    d->v[1] = _mm_add_pd(v->v[1], w->v[1]);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    d->f[0] = v->f[0] + w->f[0];
    d->f[1] = v->f[1] + w->f[1];
    d->f[2] = v->f[2] + w->f[2];
    d->f[3] = v->f[3] + w->f[3];
#endif /* MG_SSE */
}

_mg_inline void mgVec4Sub(mg_vec4_t *v, const mg_vec4_t *w)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    v->v = _mm_sub_ps(v->v, w->v);
# else /* MG_SSE_SINGLE */
    v->v[0] = _mm_sub_pd(v->v[0], w->v[0]);
    v->v[1] = _mm_sub_pd(v->v[1], w->v[1]);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    v->f[0] -= w->f[0];
    v->f[1] -= w->f[1];
    v->f[2] -= w->f[2];
    v->f[3] -= w->f[3];
#endif /* MG_SSE */
}
_mg_inline void mgVec4Sub2(mg_vec4_t *d, const mg_vec4_t *v, const mg_vec4_t *w)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    d->v = _mm_sub_ps(v->v, w->v);
# else /* MG_SSE_SINGLE */
    d->v[0] = _mm_sub_pd(v->v[0], w->v[0]);
    d->v[1] = _mm_sub_pd(v->v[1], w->v[1]);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    d->f[0] = v->f[0] - w->f[0];
    d->f[1] = v->f[1] - w->f[1];
    d->f[2] = v->f[2] - w->f[2];
    d->f[3] = v->f[3] - w->f[3];
#endif /* MG_SSE */
}

_mg_inline void mgVec4Scale(mg_vec4_t *d, mg_real_t _k)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    mg_vec4_t k;
    k.v = _mm_set1_ps(_k);
    d->v = _mm_mul_ps(d->v, k.v);
# else /* MG_SSE_SINGLE */
    __m128d k;
    k = _mm_set1_pd(_k);
    d->v[0] = _mm_mul_pd(d->v[0], k);
    d->v[1] = _mm_mul_pd(d->v[1], k);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    d->f[0] *= _k;
    d->f[1] *= _k;
    d->f[2] *= _k;
    d->f[3] *= _k;
#endif /* MG_SSE */
}

_mg_inline void mgVec4ScaleToLen(mg_vec4_t *v, mg_real_t len)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    mg_vec4_t k, l;

    k.v = _mm_set1_ps(mgVec4Len2(v));
    k.v = _mm_sqrt_ps(k.v);
    l.v = _mm_set1_ps(len);
    k.v = _mm_div_ps(k.v, l.v);
    v->v = _mm_div_ps(v->v, k.v);
# else /* MG_SSE_SINGLE */
    __m128d k, l;

    k = _mm_set1_pd(mgVec4Len2(v));
    k = _mm_sqrt_pd(k);
    l = _mm_set1_pd(len);
    k = _mm_div_pd(k, l);
    v->v[0] = _mm_div_pd(v->v[0], k);
    v->v[1] = _mm_div_pd(v->v[1], k);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    mg_real_t k = len * mgRsqrt(mgVec4Len(v));
    mgVec4Scale(v, k);
#endif /* MG_SSE */
}

_mg_inline void mgVec4Normalize(mg_vec4_t *d)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    mg_vec4_t k;

    k.v = _mm_set1_ps(mgVec4Len2(d));
    k.v = _mm_sqrt_ps(k.v);
    d->v = _mm_div_ps(d->v, k.v);
# else /* MG_SSE_SINGLE */
    __m128d k;

    k = _mm_set1_pd(mgVec4Len2(d));
    k = _mm_sqrt_pd(k);
    d->v[0] = _mm_div_pd(d->v[0], k);
    d->v[1] = _mm_div_pd(d->v[1], k);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    mg_real_t k = mgRsqrt(mgVec4Len2(d));
    mgVec4Scale(d, k);
#endif /* MG_SSE */
}

_mg_inline mg_real_t mgVec4Dot(const mg_vec4_t *a, const mg_vec4_t *b)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    mg_vec4_t dot, t;

    dot.v = _mm_mul_ps(a->v, b->v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(2, 3, 0, 1));
    dot.v = _mm_add_ps(dot.v, t.v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(1, 0, 3, 2));
    dot.v = _mm_add_ps(dot.v, t.v);

    return dot.f[0];
# else /* MG_SSE_SINGLE */
    mg_vec4_t dot;

    dot.v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    dot.v[1] = _mm_mul_pd(a->v[1], b->v[1]);
    dot.v[0] = _mm_add_pd(dot.v[0], dot.v[1]);
    dot.v[1] = _mm_shuffle_pd(dot.v[0], dot.v[0], _MM_SHUFFLE2(0, 1));
    dot.v[0] = _mm_add_pd(dot.v[0], dot.v[1]);

    return dot.f[0];
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    mg_real_t dot;

    dot  = a->f[0] * b->f[0];
    dot += a->f[1] * b->f[1];
    dot += a->f[2] * b->f[2];
    dot += a->f[3] * b->f[3];

    return dot;
#endif /* MG_SSE */
}

_mg_inline void mgVec4Mul(mg_vec4_t *a, const mg_vec4_t *b)
{
#ifdef MG_SSE
# ifdef MG_SSE_SINGLE
    a->v = _mm_mul_ps(a->v, b->v);
# else /* MG_SSE_SINGLE */
    a->v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    a->v[1] = _mm_mul_pd(a->v[1], b->v[1]);
# endif /* MG_SSE_SINGLE */
#else /* MG_SSE */
    a->f[0] *= b->f[0];
    a->f[1] *= b->f[1];
    a->f[2] *= b->f[2];
    a->f[3] *= b->f[3];
#endif /* MG_SSE */
}

_mg_inline void mgVec4Mul2(mg_vec4_t *a, const mg_vec4_t *b, const mg_vec4_t *c)
{
    mgVec4Copy(a, b);
    mgVec4Mul(a, c);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MG_VEC4_H__ */
