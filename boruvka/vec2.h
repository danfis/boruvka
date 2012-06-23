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

#ifndef __FER_VEC2_H__
#define __FER_VEC2_H__

#include <stdio.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec2 - 2D vector
 * =================
 *
 * .. c:type:: bor_vec2_t
 */

/** vvvvv */
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
union _bor_vec2_t {
    __m128 v;
    bor_real_t f[4];
} fer_aligned(16) fer_packed;
typedef union _bor_vec2_t bor_vec2_t;
# else /* FER_SSE_SINGLE */
union _bor_vec2_t {
    __m128d v;
    bor_real_t f[2];
} fer_aligned(16) fer_packed;
typedef union _bor_vec2_t bor_vec2_t;
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
struct _bor_vec2_t {
    bor_real_t f[2];
};
typedef struct _bor_vec2_t bor_vec2_t;
#endif /* FER_SSE */
/** ^^^^^ */

/**
 * Holds origin (0,0) - this variable is meant to be read-only!
 */
extern const bor_vec2_t *fer_vec2_origin;

/**
 * Holds vector (0, 1) - readonly
 */
extern const bor_vec2_t *fer_vec2_01;

/**
 * Holds vector (1, 0) - readonly
 */
extern const bor_vec2_t *fer_vec2_10;

/**
 * Holds vector (1, 1) - readonly
 */
extern const bor_vec2_t *fer_vec2_11;

#define FER_VEC2_STATIC(x, y) \
    { .f = { (x), (y) } }

#define FER_VEC2(name, x, y) \
    bor_vec2_t name = FER_VEC2_STATIC((x), (y))


/**
 * Functions
 * ----------
 */


/**
 * Allocate and initialize new vector.
 */
bor_vec2_t *ferVec2New(bor_real_t x, bor_real_t y);

/**
 * Delete vector.
 */
void ferVec2Del(bor_vec2_t *);

/**
 * Clone given bor_vec2_t. This does deep copy.
 */
_fer_inline bor_vec2_t *ferVec2Clone(const bor_vec2_t *v);

/**
 * Copies w into v.
 */
_fer_inline void ferVec2Copy(bor_vec2_t *v, const bor_vec2_t *w);

/**
 * Returns X coordinate
 */
_fer_inline bor_real_t ferVec2X(const bor_vec2_t *v);

/**
 * Returns Y coordinate
 */
_fer_inline bor_real_t ferVec2Y(const bor_vec2_t *v);

/**
 * Returns coordinate according to argument d (0 or 1).
 */
_fer_inline bor_real_t ferVec2Get(const bor_vec2_t *v, int d);

/**
 * Sets X coordinate.
 */
_fer_inline void ferVec2SetX(bor_vec2_t *v, bor_real_t val);

/**
 * Sets Y coordinate
 */
_fer_inline void ferVec2SetY(bor_vec2_t *v, bor_real_t val);

/**
 * Sets X and Y coordinates.
 */
_fer_inline void ferVec2Set(bor_vec2_t *v, bor_real_t x, bor_real_t y);

/**
 * Sets i'th coordinate (0 or 1)
 */
_fer_inline void ferVec2SetCoord(bor_vec2_t *v, size_t i, bor_real_t val);

/**
 * Returns true if two given vectors equals to each other.
 */
_fer_inline int ferVec2Eq(const bor_vec2_t *x, const bor_vec2_t *y);

/**
 * Opposite of ferVec2Eq().
 */
_fer_inline int ferVec2NEq(const bor_vec2_t *x, const bor_vec2_t *y);

/**
 * Returns true if vector v equals to [x, y]
 */
_fer_inline int ferVec2Eq2(const bor_vec2_t *v, bor_real_t x, bor_real_t y);

/**
 * Opposite of ferVec2Eq2().
 */
_fer_inline int ferVec2NEq2(const bor_vec2_t *v, bor_real_t x, bor_real_t y);


/**
 * Compute squared distance between two points represented as vectors.
 */
_fer_inline bor_real_t ferVec2Dist2(const bor_vec2_t *v, const bor_vec2_t *w);

/**
 * Distance of two vectors.
 */
_fer_inline bor_real_t ferVec2Dist(const bor_vec2_t *v, const bor_vec2_t *w);

/**
 * Squared length of vector.
 */
_fer_inline bor_real_t ferVec2Len2(const bor_vec2_t *v);

/**
 * Length of vector.
 */
_fer_inline bor_real_t ferVec2Len(const bor_vec2_t *v);


/**
 * Adds vector W to vector V (and result is stored in V):
 * v = v + w
 */
_fer_inline void ferVec2Add(bor_vec2_t *v, const bor_vec2_t *w);

/**
 * d = v + w
 */
_fer_inline void ferVec2Add2(bor_vec2_t *d, const bor_vec2_t *v, const bor_vec2_t *w);

/**
 * Substracts coordinates of vector W from vector V:
 * v = v - w
 */
_fer_inline void ferVec2Sub(bor_vec2_t *v, const bor_vec2_t *w);

/**
 * d = v - w
 */
_fer_inline void ferVec2Sub2(bor_vec2_t *d, const bor_vec2_t *v, const bor_vec2_t *w);

/**
 * Adds constant to vector v:
 * v = v + f.
 */
_fer_inline void ferVec2AddConst(bor_vec2_t *v, bor_real_t f);

/**
 * d = v + f
 */
_fer_inline void ferVec2AddConst2(bor_vec2_t *d, const bor_vec2_t *v, bor_real_t f);

/**
 * v = v - f
 */
_fer_inline void ferVec2SubConst(bor_vec2_t *v, bor_real_t f);

/**
 * d = v - f
 */
_fer_inline void ferVec2SubConst2(bor_vec2_t *d, const bor_vec2_t *v, bor_real_t f);


/**
 * Scales vector to given length.
 */
_fer_inline void ferVec2ScaleToLen(bor_vec2_t *v, bor_real_t len);

/**
 * Normalizes vector to unit vector.
 */
_fer_inline void ferVec2Normalize(bor_vec2_t *v);

/**
 * Dot product of two vectors.
 */
_fer_inline bor_real_t ferVec2Dot(const bor_vec2_t *v, const bor_vec2_t *w);

/**
 * Multiplies vectors by components:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 */
_fer_inline void ferVec2MulComp(bor_vec2_t *a, const bor_vec2_t *b);

/**
 * Multiplies vectors by components:
 * d.x = a.x * b.x
 * d.y = a.y * b.y
 */
_fer_inline void ferVec2MulComp2(bor_vec2_t *d, const bor_vec2_t *a, const bor_vec2_t *b);



/**
 * Scales vector V using constant k:
 * v = k * v
 */
_fer_inline void ferVec2Scale(bor_vec2_t *v, bor_real_t k);


/**
 * Rotates vector by given angle (counterclockwise).
 */
void ferVec2Rot(bor_vec2_t *v, bor_real_t angle);

/**
 * Rotates vector {v} by given angle and stores the result in {w}.
 */
void ferVec2Rot2(bor_vec2_t *w, const bor_vec2_t *v, bor_real_t angle);

/**
 * Prints vector to *out* in form "x y".
 */
_fer_inline void ferVec2Print(const bor_vec2_t *v, FILE *out);

/**
 * Returns twice area enclosed by given vectors.
 * a, b, c should be named in counterclockwise order to get positive
 * area and clockwise to get negative.
 */
_fer_inline bor_real_t ferVec2Area2(const bor_vec2_t *a,
                                    const bor_vec2_t *b,
                                    const bor_vec2_t *c);


/**
 * Returns angle in b formed by vectors a, b, c.
 * Returned value is between 0 and PI
 */
bor_real_t ferVec2Angle(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c);

/**
 * Returns angle formed by points a, b, c in this order, it means, that
 * is returned angle in point b and in direction from segment ab to cb.
 * Returned angle is from -PI to PI. Positive angle is in
 * counterclockwise direction.
 */
_fer_inline bor_real_t ferVec2SignedAngle(const bor_vec2_t *a,
                                          const bor_vec2_t *b,
                                          const bor_vec2_t *c);


/**
 * This function computes projection of point C onto segment AB. Point of
 * projection is returned in X.
 *
 * Returns 0 if there exists any projection, otherwise -1.
 */
int ferVec2ProjectionPointOntoSegment(const bor_vec2_t *A, const bor_vec2_t *B,
                                      const bor_vec2_t *C,
                                      bor_vec2_t *X);


/**
 * Returns true if point d is in circle formed by points a, b, c.
 * Vectors a, b, c must be in counterclockwise order.
 */
int ferVec2InCircle(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c,
                    const bor_vec2_t *d);

/**
 * Returns true if point a lies on segment formed by b c.
 */
int ferVec2LiesOn(const bor_vec2_t *a, const bor_vec2_t *b, const bor_vec2_t *c);


/**
 * Returns true if a is collinear with b and c.
 */
_fer_inline int ferVec2Collinear(const bor_vec2_t *a, const bor_vec2_t *b,
                                 const bor_vec2_t *c);

/**
 * Returns true, if vector v is in cone formed by p1, c, p2 (in
 * counterclockwise order).
 */
int ferVec2InCone(const bor_vec2_t *v,
                  const bor_vec2_t *p1, const bor_vec2_t *c, const bor_vec2_t *p2);


/**
 * Returns true if segment ab properly intersects segement cd (they share
 * point interior to both segments).
 * Properness of intersection means that two segmensts intersect at a point
 * interior to both segments. Improper intersection (which is not covered
 * by this function) means that one of end point lies somewhere on other
 * segment.
 */
_fer_inline int ferVec2IntersectProp(const bor_vec2_t *a,
                                     const bor_vec2_t *b,
                                     const bor_vec2_t *c,
                                     const bor_vec2_t *d);



/**
 * Returns true if segment ab intersects segment cd properly or improperly.
 */
_fer_inline int ferVec2Intersect(const bor_vec2_t *a, const bor_vec2_t *b,
                                 const bor_vec2_t *c, const bor_vec2_t *d);


/**
 * Compute intersection point of two segments - (a, b) and (c, d).
 * Returns 0 if there exists intersection, -1 otherwise.
 * Intersetion point is returned in p, where p must point to already
 * allocated bor_vec2_t.
 */
int ferVec2IntersectPoint(const bor_vec2_t *a, const bor_vec2_t *b,
                          const bor_vec2_t *c, const bor_vec2_t *d,
                          bor_vec2_t *p);

/**
 * Returns true iff vector v is on left side from segment formed by p1 and
 * p2 in this ordering.
 */
_fer_inline int ferVec2OnLeft(const bor_vec2_t *v,
                              const bor_vec2_t *p1, const bor_vec2_t *p2);


/**
 * This function takes as arguments rectangle (a, b, c, d) and segment
 * (x, y) and tries to find which part of segment (x, y) is enclosed by
 * rectangle (is within rectangle). This segment is returned via (s1, s2).
 * In fact, (a, b, c, d) does not have to be rectangle, but it can be any
 * convex polygon formed by four sides.
 *
 * If any part of segment (x, y) does not lies within given rectangle,
 * -1 is returned, 0 if segment is found.
 */
int ferVec2SegmentInRect(const bor_vec2_t *a, const bor_vec2_t *b,
                         const bor_vec2_t *c, const bor_vec2_t *d,
                         const bor_vec2_t *x, const bor_vec2_t *y,
                         bor_vec2_t *s1, bor_vec2_t *s2);


/**
 * Returns angle by which must be vector b rotated about origin to have
 * same direction as vector a.
 * Returned angle is in range -PI, PI.
 */
bor_real_t ferVec2AngleSameDir(const bor_vec2_t *a, const bor_vec2_t *b);



/**
 * Retuns angle by which must be rotated oriented segment CD to have same
 * direction as oriented segment AB. Directional vector of segment CD is
 * vector (D - C) and directional vector of segment AB is vector (B - A).
 * Returned angle will be between -PI and PI.
 */
bor_real_t ferVec2AngleSegsSameDir(const bor_vec2_t *A, const bor_vec2_t *B,
                                   const bor_vec2_t *C, const bor_vec2_t *D);


/**
 * Returns true if given triangles (p1, q1, r1) and (p2, q2, r2) overlap.
 */
int ferVec2TriTriOverlap(const bor_vec2_t *p1, const bor_vec2_t *q1,
                         const bor_vec2_t *r1,
                         const bor_vec2_t *p2, const bor_vec2_t *q2,
                         const bor_vec2_t *r2);

/**
 * Returns true if point {P} is inside triangle {pqr}.
 */
_fer_inline int ferVec2PointInTri(const bor_vec2_t *P,
                                  const bor_vec2_t *p, const bor_vec2_t *q,
                                  const bor_vec2_t *r);

/**
 * Returns true if boxes overlap.
 */
int ferVec2BoxBoxOverlap(const bor_vec2_t *half_edges1,
                         const bor_vec2_t *pos1, bor_real_t rot1,
                         const bor_vec2_t *half_edges2,
                         const bor_vec2_t *pos2, bor_real_t rot2);

/***** INLINES *****/
_fer_inline bor_real_t ferVec2Get(const bor_vec2_t *v, int d)
{
    return v->f[d];
}


_fer_inline bor_real_t ferVec2X(const bor_vec2_t *v)
{
    return v->f[0];
}
_fer_inline bor_real_t ferVec2Y(const bor_vec2_t *v)
{
    return v->f[1];
}


_fer_inline void ferVec2SetX(bor_vec2_t *v, bor_real_t val)
{
    v->f[0] = val;
}
_fer_inline void ferVec2SetY(bor_vec2_t *v, bor_real_t val)
{
    v->f[1] = val;
}


_fer_inline void ferVec2Set(bor_vec2_t *v, bor_real_t x, bor_real_t y)
{
    ferVec2SetX(v, x);
    ferVec2SetY(v, y);
}

_fer_inline void ferVec2SetCoord(bor_vec2_t *v, size_t i, bor_real_t val)
{
    v->f[i] = val;
}


_fer_inline bor_vec2_t *ferVec2Clone(const bor_vec2_t *v)
{
    return ferVec2New(ferVec2X(v), ferVec2Y(v));
}

_fer_inline void ferVec2Copy(bor_vec2_t *v, const bor_vec2_t *w)
{
    ferVec2Set(v, ferVec2X(w), ferVec2Y(w));
}

_fer_inline int ferVec2Eq(const bor_vec2_t *x, const bor_vec2_t *y)
{
    return ferEq(ferVec2X(x), ferVec2X(y)) && ferEq(ferVec2Y(x), ferVec2Y(y));
}
_fer_inline int ferVec2NEq(const bor_vec2_t *x, const bor_vec2_t *y)
{
    return !ferVec2Eq(x, y);
}

_fer_inline int ferVec2Eq2(const bor_vec2_t *v, bor_real_t x, bor_real_t y)
{
    return ferEq(ferVec2X(v), x) && ferEq(ferVec2Y(v), y);
}
_fer_inline int ferVec2NEq2(const bor_vec2_t *v, bor_real_t x, bor_real_t y)
{
    return !ferVec2Eq2(v, x, y);
}


_fer_inline bor_real_t ferVec2Dist2(const bor_vec2_t *a, const bor_vec2_t *b)
{
    bor_vec2_t ab;
    ferVec2Sub2(&ab, a, b);
    return ferVec2Len2(&ab);
}

_fer_inline bor_real_t ferVec2Dist(const bor_vec2_t *v, const bor_vec2_t *w)
{
    return FER_SQRT(ferVec2Dist2(v, w));
}

_fer_inline bor_real_t ferVec2Len2(const bor_vec2_t *v)
{
    return ferVec2Dot(v, v);
}

_fer_inline bor_real_t ferVec2Len(const bor_vec2_t *v)
{
    return FER_SQRT(ferVec2Len2(v));
}


_fer_inline void ferVec2Add(bor_vec2_t *v, const bor_vec2_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    v->v = _mm_add_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    v->v = _mm_add_pd(v->v, w->v);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    v->f[0] += w->f[0];
    v->f[1] += w->f[1];
#endif /* FER_SSE */
}

_fer_inline void ferVec2Add2(bor_vec2_t *d, const bor_vec2_t *v, const bor_vec2_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    d->v = _mm_add_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    d->v = _mm_add_pd(v->v, w->v);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = v->f[0] + w->f[0];
    d->f[1] = v->f[1] + w->f[1];
#endif /* FER_SSE */
}

_fer_inline void ferVec2Sub(bor_vec2_t *v, const bor_vec2_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    v->v = _mm_sub_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    v->v = _mm_sub_pd(v->v, w->v);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    v->f[0] -= w->f[0];
    v->f[1] -= w->f[1];
#endif /* FER_SSE */
}

_fer_inline void ferVec2Sub2(bor_vec2_t *d, const bor_vec2_t *v, const bor_vec2_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    d->v = _mm_sub_ps(v->v, w->v);
# else /* FER_SSE_SINGLE */
    d->v = _mm_sub_pd(v->v, w->v);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = v->f[0] - w->f[0];
    d->f[1] = v->f[1] - w->f[1];
#endif /* FER_SSE */
}

_fer_inline void ferVec2AddConst(bor_vec2_t *v, bor_real_t f)
{
    v->f[0] += f;
    v->f[1] += f;
}

_fer_inline void ferVec2AddConst2(bor_vec2_t *d, const bor_vec2_t *v, bor_real_t f)
{
    d->f[0] = v->f[0] + f;
    d->f[1] = v->f[1] + f;
}

_fer_inline void ferVec2SubConst(bor_vec2_t *v, bor_real_t f)
{
    v->f[0] -= f;
    v->f[1] -= f;
}

_fer_inline void ferVec2SubConst2(bor_vec2_t *d, const bor_vec2_t *v, bor_real_t f)
{
    d->f[0] = v->f[0] - f;
    d->f[1] = v->f[1] - f;
}


_fer_inline void ferVec2Scale(bor_vec2_t *v, bor_real_t k)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    __m128 l;
    l = _mm_set1_ps(k);
    v->v = _mm_mul_ps(v->v, l);
# else /* FER_SSE_SINGLE */
    __m128d l;
    l = _mm_set1_pd(k);
    v->v = _mm_mul_pd(v->v, l);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    v->f[0] *= k;
    v->f[1] *= k;
#endif /* FER_SSE */
}

_fer_inline void ferVec2ScaleToLen(bor_vec2_t *v, bor_real_t len)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    __m128 k, l;

    k = _mm_set1_ps(ferVec2Len2(v));
    k = _mm_sqrt_ps(k);
    l = _mm_set1_ps(len);
    k = _mm_div_ps(k, l);
    v->v = _mm_div_ps(v->v, k);
# else /* FER_SSE_SINGLE */
    __m128d k, l;

    k = _mm_set1_pd(ferVec2Len2(v));
    k = _mm_sqrt_pd(k);
    l = _mm_set1_pd(len);
    k = _mm_div_pd(k, l);
    v->v = _mm_div_pd(v->v, k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    bor_real_t k = len * ferRsqrt(ferVec2Len2(v));
    ferVec2Scale(v, k);
#endif /* FER_SSE */
}

_fer_inline void ferVec2Normalize(bor_vec2_t *v)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    __m128 k;

    k = _mm_set1_ps(ferVec2Len2(v));
    k = _mm_rsqrt_ps(k);
    v->v = _mm_mul_ps(v->v, k);
# else /* FER_SSE_SINGLE */
    __m128d k;

    k = _mm_set1_pd(ferVec2Len2(v));
    k = _mm_sqrt_pd(k);
    v->v = _mm_div_pd(v->v, k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    bor_real_t k = ferRsqrt(ferVec2Len2(v));
    ferVec2Scale(v, k);
#endif /* FER_SSE */
}

_fer_inline bor_real_t ferVec2Dot(const bor_vec2_t *v, const bor_vec2_t *w)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    bor_vec2_t dot, t;

    dot.v = _mm_mul_ps(v->v, w->v);
    dot.f[2] = dot.f[3] = FER_ZERO;
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(1, 1, 1, 1));
    dot.v = _mm_add_ps(dot.v, t.v);

    return dot.f[0];
# else /* FER_SSE_SINGLE */
    bor_vec2_t dot, t;

    dot.v = _mm_mul_pd(v->v, w->v);
    t.v = _mm_shuffle_pd(dot.v, dot.v, _MM_SHUFFLE2(1, 1));
    dot.v = _mm_add_pd(dot.v, t.v);

    return dot.f[0];
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    bor_real_t dot;
    dot  = v->f[0] * w->f[0];
    dot += v->f[1] * w->f[1];
    return dot;
#endif /* FER_SSE */
}

_fer_inline void ferVec2MulComp(bor_vec2_t *a, const bor_vec2_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    a->v = _mm_mul_ps(a->v, b->v);
# else /* FER_SSE_SINGLE */
    a->v = _mm_mul_pd(a->v, b->v);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    a->f[0] *= b->f[0];
    a->f[1] *= b->f[1];
#endif /* FER_SSE */
}

_fer_inline void ferVec2MulComp2(bor_vec2_t *d, const bor_vec2_t *a, const bor_vec2_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    d->v = _mm_mul_ps(a->v, b->v);
# else /* FER_SSE_SINGLE */
    d->v = _mm_mul_pd(a->v, b->v);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = a->f[0] * b->f[0];
    d->f[1] = a->f[1] * b->f[1];
#endif /* FER_SSE */
}


_fer_inline void ferVec2Print(const bor_vec2_t *v, FILE *out)
{
    fprintf(out, "%g %g", (double)ferVec2X(v), (double)ferVec2Y(v));
}

#include <stdio.h>
_fer_inline bor_real_t ferVec2Area2(const bor_vec2_t *a,
                                    const bor_vec2_t *b,
                                    const bor_vec2_t *c)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    __m128 bybx, cycx, x1, x2, x3, x4, x5, x6;

    /* TODO: all 4 items of __m128 can be used - it will reduce number of */
    /*       instructions. */
    bybx = _mm_shuffle_ps(b->v, b->v, _MM_SHUFFLE(0, 0, 0, 1));
    cycx = _mm_shuffle_ps(c->v, c->v, _MM_SHUFFLE(0, 0, 0, 1));

    x1 = _mm_mul_ps(a->v, bybx);
    x2 = _mm_shuffle_ps(x1, x1, _MM_SHUFFLE(0, 0, 0, 1));
    x3 = _mm_mul_ps(a->v, cycx);
    x4 = _mm_shuffle_ps(x3, x3, _MM_SHUFFLE(0, 0, 0, 1));
    x5 = _mm_mul_ps(b->v, cycx);
    x6 = _mm_shuffle_ps(x5, x5, _MM_SHUFFLE(0, 0, 0, 1));

    x1 = _mm_sub_ps(x1, x2);
    x3 = _mm_sub_ps(x4, x3);
    x5 = _mm_sub_ps(x5, x6);
    x1 = _mm_add_ps(x1, x3);
    x1 = _mm_add_ps(x1, x5);

    return ((bor_vec2_t *)&x1)->f[0];
# else /* FER_SSE_SINGLE */
    __m128d bybx, cycx, x1, x2, x3, x4, x5, x6;

    bybx = _mm_shuffle_pd(b->v, b->v, _MM_SHUFFLE2(0, 1));
    cycx = _mm_shuffle_pd(c->v, c->v, _MM_SHUFFLE2(0, 1));

    x1 = _mm_mul_pd(a->v, bybx);
    x2 = _mm_shuffle_pd(x1, x1, _MM_SHUFFLE(0, 0, 0, 1));
    x3 = _mm_mul_pd(a->v, cycx);
    x4 = _mm_shuffle_pd(x3, x3, _MM_SHUFFLE(0, 0, 0, 1));
    x5 = _mm_mul_pd(b->v, cycx);
    x6 = _mm_shuffle_pd(x5, x5, _MM_SHUFFLE(0, 0, 0, 1));

    x1 = _mm_sub_pd(x1, x2);
    x3 = _mm_sub_pd(x4, x3);
    x5 = _mm_sub_pd(x5, x6);
    x1 = _mm_add_pd(x1, x3);
    x1 = _mm_add_pd(x1, x5);

    return ((bor_vec2_t *)&x1)->f[0];
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    /* Area2 can be computed as determinant:
     * | a.x a.y 1 |
     * | b.x b.y 1 |
     * | c.x c.y 1 |
     */

    bor_real_t ax, ay, bx, by, cx, cy;

    ax = ferVec2X(a);
    ay = ferVec2Y(a);
    bx = ferVec2X(b);
    by = ferVec2Y(b);
    cx = ferVec2X(c);
    cy = ferVec2Y(c);
    return ax * by - ay * bx +
           ay * cx - ax * cy +
           bx * cy - by * cx;
#endif /* FER_SSE */
}


_fer_inline bor_real_t ferVec2SignedAngle(const bor_vec2_t *a,
                                    const bor_vec2_t *b,
                                    const bor_vec2_t *c)
{
    bor_real_t ang = ferVec2Angle(a, b, c);

    /* angle holded in b is convex */
    if (ferVec2Area2(a, b, c) < 0)
        return ang;

    /* angle is concave */
    return -1 * ang;
}

_fer_inline int ferVec2Collinear(const bor_vec2_t *a, const bor_vec2_t *b,
                                 const bor_vec2_t *c)
{
    return ferEq(ferVec2Area2((b), (c), (a)), FER_ZERO);
}


_fer_inline int ferVec2IntersectProp(const bor_vec2_t *a,
                                  const bor_vec2_t *b,
                                  const bor_vec2_t *c,
                                  const bor_vec2_t *d)
{
    /* area2(a, b, c) * area2(a, b, d) is less than zero only if */
    /* only one computed area is less than zero, so it means that it is the */
    /* same as XOR(pointArea2(a, b, c), pointArea2(a, b, d)) */
    if (ferVec2Area2(a, b, c) * ferVec2Area2(a, b, d) < 0
        && ferVec2Area2(c, d, a) * ferVec2Area2(c, d, b) < 0)
        return 1;
    return 0;
}



_fer_inline int ferVec2Intersect(const bor_vec2_t *a, const bor_vec2_t *b,
                              const bor_vec2_t *c, const bor_vec2_t *d)
{
    if (ferVec2IntersectProp(a, b, c, d))
        return 1;

    if (ferVec2LiesOn(a, c, d) || ferVec2LiesOn(b, c, d)
        || ferVec2LiesOn(c, a, b) || ferVec2LiesOn(d, a, b))
        return 1;
    return 0;
}


_fer_inline int ferVec2OnLeft(const bor_vec2_t *v,
                           const bor_vec2_t *p1, const bor_vec2_t *p2)
{
    return ferVec2Area2(p1, p2, v) > FER_ZERO;
}


_fer_inline int ferVec2PointInTri(const bor_vec2_t *P,
                                  const bor_vec2_t *p, const bor_vec2_t *q,
                                  const bor_vec2_t *r)
{
    int count = 0;
    count += ferVec2OnLeft(P, p, q);
    count += ferVec2OnLeft(P, q, r);
    count += ferVec2OnLeft(P, r, p);
    return count == 3 || count == 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_VEC2_H__ */
