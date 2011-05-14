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

#ifndef __FER_VEC3_H__
#define __FER_VEC3_H__

#include <stdio.h>
#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec3 - 3D vector
 * =================
 *
 *
 * .. c:type:: fer_vec3_t
 */

/** vvvvv */
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
union _fer_vec3_t {
    __m128 v;
    float f[4];
} fer_aligned(16) fer_packed;
typedef union _fer_vec3_t fer_vec3_t;
# else /* FER_SSE_SINGLE */
union _fer_vec3_t {
    __m128d v[2];
    double f[4];
} fer_aligned(16) fer_packed;
typedef union _fer_vec3_t fer_vec3_t;
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
struct _fer_vec3_t {
    fer_real_t f[4];
};
typedef struct _fer_vec3_t fer_vec3_t;
#endif /* FER_SSE */
/** ^^^^^ */


/**
 * Holds origin (0,0,0) - this variable is meant to be read-only!
 */
extern const fer_vec3_t *fer_vec3_origin;

/**
 * X, Y and Z axis - read-only.
 */
extern const fer_vec3_t *fer_vec3_axis[3];

/**
 * Array of points uniformly distributed on unit sphere.
 */
extern fer_vec3_t *fer_points_on_sphere;
extern size_t fer_points_on_sphere_len;

# define FER_VEC3_STATIC(x, y, z) \
    { .f = { (x), (y), (z), FER_ZERO } }

#define FER_VEC3(name, x, y, z) \
    fer_vec3_t name = FER_VEC3_STATIC((x), (y), (z))

/**
 * Functions
 * ----------
 */

/**
 * Allocate and initialize new vector.
 */
fer_vec3_t *ferVec3New(fer_real_t x, fer_real_t y, fer_real_t z);

/**
 * Delete vector.
 */
void ferVec3Del(fer_vec3_t *);

/**
 * Allocates array of vectors.
 * Use this function instead of classical malloc() due to alignement that
 * could be required if you use SSE version of library.
 * Use ferVec3ArrDel() to free allocated memory.
 */
fer_vec3_t *ferVec3ArrNew(size_t num_vecs);

/**
 * Deletes array created by ferVec3ArrNew().
 */
void ferVec3ArrDel(fer_vec3_t *);

/**
 * Returns memory aligned to vector size.
 * This can be useful if you have somewhere allocated memory but you want
 * to use it for vectors and you use SSE version of library.
 */
_fer_inline fer_vec3_t *ferVec3Align(void *mem);

/**
 * Clone given fer_vec3_t. This does deep copy.
 */
_fer_inline fer_vec3_t *ferVec3Clone(const fer_vec3_t *v);

/**
 * v = w
 */
_fer_inline void ferVec3Copy(fer_vec3_t *v, const fer_vec3_t *w);


_fer_inline fer_real_t ferVec3X(const fer_vec3_t *v);
_fer_inline fer_real_t ferVec3Y(const fer_vec3_t *v);
_fer_inline fer_real_t ferVec3Z(const fer_vec3_t *v);
_fer_inline fer_real_t ferVec3Get(const fer_vec3_t *v, int d);

_fer_inline void ferVec3SetX(fer_vec3_t *v, fer_real_t val);
_fer_inline void ferVec3SetY(fer_vec3_t *v, fer_real_t val);
_fer_inline void ferVec3SetZ(fer_vec3_t *v, fer_real_t val);
_fer_inline void ferVec3Set(fer_vec3_t *v, fer_real_t x, fer_real_t y, fer_real_t z);
_fer_inline void ferVec3SetCoord(fer_vec3_t *v, int d, fer_real_t val);
_fer_inline void ferVec3Set1(fer_vec3_t *v, int d, fer_real_t val);


/**
 * Returns true if a and b equal.
 */
_fer_inline int ferVec3Eq(const fer_vec3_t *a, const fer_vec3_t *b);
_fer_inline int ferVec3NEq(const fer_vec3_t *a, const fer_vec3_t *b);
_fer_inline int ferVec3Eq2(const fer_vec3_t *a, fer_real_t x, fer_real_t y, fer_real_t z);
_fer_inline int ferVec3NEq2(const fer_vec3_t *a, fer_real_t x, fer_real_t y, fer_real_t z);
_fer_inline int ferVec3IsZero(const fer_vec3_t *a);


/**
 * Returns squared length of vector.
 */
_fer_inline fer_real_t ferVec3Len2(const fer_vec3_t *v);
_fer_inline fer_real_t ferVec3Len(const fer_vec3_t *v);

/**
 * Returns squared distance between a and b.
 */
_fer_inline fer_real_t ferVec3Dist2(const fer_vec3_t *a, const fer_vec3_t *b);
_fer_inline fer_real_t ferVec3Dist(const fer_vec3_t *a, const fer_vec3_t *b);


/**
 * Adds coordinates of vector w to vector v.
 * v = v + w
 */
_fer_inline void ferVec3Add(fer_vec3_t *v, const fer_vec3_t *w);

/**
 * d = v + w
 */
_fer_inline void ferVec3Add2(fer_vec3_t *d, const fer_vec3_t *v, const fer_vec3_t *w);


/**
 * Substracts coordinates of vector w from vector v.
 * v = v - w
 */
_fer_inline void ferVec3Sub(fer_vec3_t *v, const fer_vec3_t *w);

/**
 * d = v - w
 */
_fer_inline void ferVec3Sub2(fer_vec3_t *d, const fer_vec3_t *v, const fer_vec3_t *w);

_fer_inline void ferVec3AddConst(fer_vec3_t *v, fer_real_t f);
_fer_inline void ferVec3AddConst2(fer_vec3_t *d, const fer_vec3_t *v, fer_real_t f);
_fer_inline void ferVec3SubConst(fer_vec3_t *v, fer_real_t f);
_fer_inline void ferVec3SubConst2(fer_vec3_t *d, const fer_vec3_t *v, fer_real_t f);


/**
 * d = d * k;
 */
_fer_inline void ferVec3Scale(fer_vec3_t *d, fer_real_t k);

/**
 * d = v * k;
 */
_fer_inline void ferVec3Scale2(fer_vec3_t *d, const fer_vec3_t *v, fer_real_t k);

/**
 * Scales vector v to given length.
 */
_fer_inline void ferVec3ScaleToLen(fer_vec3_t *v, fer_real_t len);

/**
 * Normalizes given vector to unit length.
 */
_fer_inline void ferVec3Normalize(fer_vec3_t *d);


/**
 * Dot product of two vectors.
 */
_fer_inline fer_real_t ferVec3Dot(const fer_vec3_t *a, const fer_vec3_t *b);

/**
 * Multiplies vectors component wise:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 * a.z = a.z * b.z
 */
_fer_inline void ferVec3MulComp(fer_vec3_t *a, const fer_vec3_t *b);

/**
 * a.x = b.x * c.x
 * a.y = b.y * c.y
 * a.z = b.z * c.z
 */
_fer_inline void ferVec3MulComp2(fer_vec3_t *a, const fer_vec3_t *b, const fer_vec3_t *c);

/**
 * Cross product:
 * d = a x b.
 */
_fer_inline void ferVec3Cross(fer_vec3_t *d, const fer_vec3_t *a, const fer_vec3_t *b);


/**
 * Prints vector to *out* in form "x y z".
 */
_fer_inline void ferVec3Print(const fer_vec3_t *v, FILE *out);

/**
 * Returns distance^2 of point P to segment ab.
 * If witness is non-NULL it is filled with coordinates of point from which
 * was computed distance to point P.
 */
fer_real_t ferVec3PointSegmentDist2(const fer_vec3_t *P,
                                    const fer_vec3_t *a, const fer_vec3_t *b,
                                    fer_vec3_t *witness);

/**
 * Returns distance^2 of point P from triangle formed by triplet a, b, c.
 * If witness vector is provided it is filled with coordinates of point
 * from which was computed distance to point P.
 */
fer_real_t ferVec3PointTriDist2(const fer_vec3_t *P,
                                const fer_vec3_t *a, const fer_vec3_t *b,
                                const fer_vec3_t *c,
                                fer_vec3_t *witness);

/**
 * Returns true if point p lies on triangle abc. If witness is non-NULL it
 * will be filled with witness point if p lies on triangle.
 */
int ferVec3PointInTri(const fer_vec3_t *p,
                      const fer_vec3_t *a, const fer_vec3_t *b,
                      const fer_vec3_t *c);

/**
 * Returns angle in b between points a and b.
 * Returned value is between 0 and PI.
 */
fer_real_t ferVec3Angle(const fer_vec3_t *a, const fer_vec3_t *b, const fer_vec3_t *c);

/**
 * Returns dihedral angle between planes abc and bcd.
 */
fer_real_t ferVec3DihedralAngle(const fer_vec3_t *a, const fer_vec3_t *b,
                                const fer_vec3_t *c, const fer_vec3_t *d);

/**
 * Stores in d projection of point p onto plane defined by three points
 * u, v, w.
 * Returns distance of p from plane or negative number on error (i.e. when
 * u, v, w does not define plane)
 */
fer_real_t ferVec3ProjToPlane(const fer_vec3_t *p,
                              const fer_vec3_t *u, const fer_vec3_t *v,
                              const fer_vec3_t *w, fer_vec3_t *d);

/**
 * Stores in d projection of point p onto plane defined by its normal and
 * point x belonging to.
 * Returns distance of p from plane or negative number on error (i.e. when
 * normal is zero vector).
 */
fer_real_t ferVec3ProjToPlane2(const fer_vec3_t *p,
                               const fer_vec3_t *x, const fer_vec3_t *normal,
                               fer_vec3_t *d);


/**
 * Returns twice of area of triangle.
 */
fer_real_t ferVec3TriArea2(const fer_vec3_t *a, const fer_vec3_t *b,
                           const fer_vec3_t *c);

/**
 * Stores in d centroid of triangle abc.
 */
_fer_inline void ferVec3TriCentroid(const fer_vec3_t *a, const fer_vec3_t *b,
                                    const fer_vec3_t *c, fer_vec3_t *d);

/**
 * Returns true if three given points are collinear.
 */
_fer_inline int ferVec3Collinear(const fer_vec3_t *a, const fer_vec3_t *b,
                                 const fer_vec3_t *c);


/**
 * Returns 6-times *signed* volume of tetrahedron formed by points
 * a, b, c, d. If a, b, c are given in counterclockwise order then sign of
 * volume is:
 *     1) positive if d is below a plane formed by a, b, c
 *     2) negative if d is above
 *     3) zero if points are coplanar
 */
_fer_inline fer_real_t ferVec3Volume6(const fer_vec3_t *a,
                                      const fer_vec3_t *b,
                                      const fer_vec3_t *c,
                                      const fer_vec3_t *d);


/**
 * Returns true if two given triangles (p1, q1, r1) and (p2, q2, r2) are
 * overlapping.
 */
int ferVec3TriTriOverlap(const fer_vec3_t *p1, const fer_vec3_t *q1,
                         const fer_vec3_t *r1,
                         const fer_vec3_t *p2, const fer_vec3_t *q2,
                         const fer_vec3_t *r2);


/**** INLINES ****/
_fer_inline fer_vec3_t *ferVec3Align(void *mem)
{
#ifdef FER_SSE
    return (fer_vec3_t *)ferAlign(mem, 16);
#else /* FER_SSE */
    return (fer_vec3_t *)mem;
#endif /* FER_SSE */
}

_fer_inline fer_vec3_t *ferVec3Clone(const fer_vec3_t *v)
{
    return ferVec3New(ferVec3X(v), ferVec3Y(v), ferVec3Z(v));
}

_fer_inline fer_real_t ferVec3Get(const fer_vec3_t *v, int d)
{
    return v->f[d];
}

_fer_inline fer_real_t ferVec3X(const fer_vec3_t *v)
{
    return v->f[0];
}

_fer_inline fer_real_t ferVec3Y(const fer_vec3_t *v)
{
    return v->f[1];
}

_fer_inline fer_real_t ferVec3Z(const fer_vec3_t *v)
{
    return v->f[2];
}

_fer_inline int ferVec3Eq(const fer_vec3_t *a, const fer_vec3_t *b)
{
    return ferEq(ferVec3X(a), ferVec3X(b))
            && ferEq(ferVec3Y(a), ferVec3Y(b))
            && ferEq(ferVec3Z(a), ferVec3Z(b));
}

_fer_inline int ferVec3NEq(const fer_vec3_t *a, const fer_vec3_t *b)
{
    return !ferVec3Eq(a, b);
}

_fer_inline int ferVec3Eq2(const fer_vec3_t *a, fer_real_t x, fer_real_t y, fer_real_t z)
{
    return ferEq(ferVec3X(a), x)
            && ferEq(ferVec3Y(a), y)
            && ferEq(ferVec3Z(a), z);
}

_fer_inline int ferVec3NEq2(const fer_vec3_t *a, fer_real_t x, fer_real_t y, fer_real_t z)
{
    return !ferVec3Eq2(a, x, y, z);
}

_fer_inline int ferVec3IsZero(const fer_vec3_t *a)
{
    return ferIsZero(ferVec3X(a))
            && ferIsZero(ferVec3Y(a))
            && ferIsZero(ferVec3Z(a));
}

_fer_inline fer_real_t ferVec3Len2(const fer_vec3_t *v)
{
    return ferVec3Dot(v, v);
}
_fer_inline fer_real_t ferVec3Len(const fer_vec3_t *v)
{
    return FER_SQRT(ferVec3Len2(v));
}

_fer_inline fer_real_t ferVec3Dist2(const fer_vec3_t *a, const fer_vec3_t *b)
{
    fer_vec3_t ab;
    ferVec3Sub2(&ab, a, b);
    return ferVec3Len2(&ab);
}
_fer_inline fer_real_t ferVec3Dist(const fer_vec3_t *a, const fer_vec3_t *b)
{
    return FER_SQRT(ferVec3Dist2(a, b));
}

_fer_inline void ferVec3Set(fer_vec3_t *v, fer_real_t x, fer_real_t y, fer_real_t z)
{
    v->f[0] = x;
    v->f[1] = y;
    v->f[2] = z;
}
_fer_inline void ferVec3SetCoord(fer_vec3_t *v, int d, fer_real_t val)
{
    v->f[d] = val;
}

_fer_inline void ferVec3Set1(fer_vec3_t *v, int d, fer_real_t val)
{
    ferVec3SetCoord(v, d, val);
}

_fer_inline void ferVec3SetX(fer_vec3_t *v, fer_real_t val)
{
    v->f[0] = val;
}

_fer_inline void ferVec3SetY(fer_vec3_t *v, fer_real_t val)
{
    v->f[1] = val;
}

_fer_inline void ferVec3SetZ(fer_vec3_t *v, fer_real_t val)
{
    v->f[2] = val;
}

_fer_inline void ferVec3Copy(fer_vec3_t *v, const fer_vec3_t *w)
{
    *v = *w;
}

_fer_inline void ferVec3Add(fer_vec3_t *v, const fer_vec3_t *w)
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
#endif /* FER_SSE */
}

_fer_inline void ferVec3Add2(fer_vec3_t *d, const fer_vec3_t *v, const fer_vec3_t *w)
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
#endif /* FER_SSE */
}

_fer_inline void ferVec3Sub(fer_vec3_t *v, const fer_vec3_t *w)
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
#endif /* FER_SSE */
}
_fer_inline void ferVec3Sub2(fer_vec3_t *d, const fer_vec3_t *v, const fer_vec3_t *w)
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
#endif /* FER_SSE */
}

_fer_inline void ferVec3AddConst(fer_vec3_t *v, fer_real_t f)
{
    v->f[0] += f;
    v->f[1] += f;
    v->f[2] += f;
}

_fer_inline void ferVec3AddConst2(fer_vec3_t *d, const fer_vec3_t *v, fer_real_t f)
{
    d->f[0] = v->f[0] + f;
    d->f[1] = v->f[1] + f;
    d->f[2] = v->f[2] + f;
}

_fer_inline void ferVec3SubConst(fer_vec3_t *v, fer_real_t f)
{
    v->f[0] -= f;
    v->f[1] -= f;
    v->f[2] -= f;
}

_fer_inline void ferVec3SubConst2(fer_vec3_t *d, const fer_vec3_t *v, fer_real_t f)
{
    d->f[0] = v->f[0] - f;
    d->f[1] = v->f[1] - f;
    d->f[2] = v->f[2] - f;
}

_fer_inline void ferVec3Scale(fer_vec3_t *d, fer_real_t _k)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec3_t k;
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
#endif /* FER_SSE */
}

_fer_inline void ferVec3Scale2(fer_vec3_t *d, const fer_vec3_t *a, fer_real_t _k)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec3_t k;
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
#endif /* FER_SSE */
}

_fer_inline void ferVec3ScaleToLen(fer_vec3_t *v, fer_real_t len)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec3_t k, l;

    k.v = _mm_set1_ps(ferVec3Len2(v));
    k.v = _mm_sqrt_ps(k.v);
    l.v = _mm_set1_ps(len);
    k.v = _mm_div_ps(k.v, l.v);
    v->v = _mm_div_ps(v->v, k.v);
# else /* FER_SSE_SINGLE */
    __m128d k, l;

    k = _mm_set1_pd(ferVec3Len2(v));
    k = _mm_sqrt_pd(k);
    l = _mm_set1_pd(len);
    k = _mm_div_pd(k, l);
    v->v[0] = _mm_div_pd(v->v[0], k);
    v->v[1] = _mm_div_pd(v->v[1], k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    fer_real_t k = len * ferRsqrt(ferVec3Len2(v));
    ferVec3Scale(v, k);
#endif /* FER_SSE */
}

_fer_inline void ferVec3Normalize(fer_vec3_t *d)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec3_t k;

    k.v = _mm_set1_ps(ferVec3Len2(d));
    k.v = _mm_sqrt_ps(k.v);
    d->v = _mm_div_ps(d->v, k.v);
# else /* FER_SSE_SINGLE */
    __m128d k;

    k = _mm_set1_pd(ferVec3Len2(d));
    k = _mm_sqrt_pd(k);
    d->v[0] = _mm_div_pd(d->v[0], k);
    d->v[1] = _mm_div_pd(d->v[1], k);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    fer_real_t k = ferRsqrt(ferVec3Len2(d));
    ferVec3Scale(d, k);
#endif /* FER_SSE */
}

_fer_inline fer_real_t ferVec3Dot(const fer_vec3_t *a, const fer_vec3_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec3_t dot, t;

    dot.v = _mm_mul_ps(a->v, b->v);
    dot.f[3] = FER_ZERO;
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(2, 3, 0, 1));
    dot.v = _mm_add_ps(dot.v, t.v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(1, 0, 3, 2));
    dot.v = _mm_add_ps(dot.v, t.v);

    return dot.f[0];
# else /* FER_SSE_SINGLE */
    fer_vec3_t dot;

    dot.v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    dot.v[1] = _mm_mul_pd(a->v[1], b->v[1]);
    dot.f[3] = FER_ZERO;
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

    return dot;
#endif /* FER_SSE */
}

_fer_inline void ferVec3MulComp(fer_vec3_t *a, const fer_vec3_t *b)
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
#endif /* FER_SSE */
}

_fer_inline void ferVec3MulComp2(fer_vec3_t *a, const fer_vec3_t *b, const fer_vec3_t *c)
{
    ferVec3Copy(a, b);
    ferVec3MulComp(a, c);
}

_fer_inline void ferVec3Cross(fer_vec3_t *d, const fer_vec3_t *a, const fer_vec3_t *b)
{
#ifdef FER_SSE
# ifdef FER_SSE_SINGLE
    fer_vec3_t v0, v1, v2, v3;

    v0.v = a->v;
    v1.v = b->v;
    v2.v = a->v;
    v3.v = b->v;

    v0.v = _mm_shuffle_ps(v0.v, v0.v, _MM_SHUFFLE(3, 0, 2, 1));
    v1.v = _mm_shuffle_ps(v1.v, v1.v, _MM_SHUFFLE(3, 1, 0, 2));
    v0.v = _mm_mul_ps(v0.v, v1.v);

    v2.v = _mm_shuffle_ps(v2.v, v2.v, _MM_SHUFFLE(3, 1, 0, 2));
    v3.v = _mm_shuffle_ps(v3.v, v3.v, _MM_SHUFFLE(3, 0, 2, 1));
    v2.v = _mm_mul_ps(v2.v, v3.v);

    d->v = _mm_sub_ps(v0.v, v2.v);
# else /* FER_SSE_SINGLE */
    fer_vec3_t v0, v1, v2, v3;

    ferVec3Copy(&v0, a);
    ferVec3Copy(&v1, b);
    ferVec3Copy(&v2, a);
    ferVec3Copy(&v3, b);

    v0.v[0] = _mm_shuffle_pd(a->v[0], a->v[1], _MM_SHUFFLE2(0, 1));
    v0.v[1] = _mm_shuffle_pd(a->v[0], a->v[1], _MM_SHUFFLE2(1, 0));
    v1.v[0] = _mm_shuffle_pd(b->v[1], b->v[0], _MM_SHUFFLE2(0, 0));
    v1.v[1] = _mm_shuffle_pd(b->v[0], b->v[1], _MM_SHUFFLE2(1, 1));
    v0.v[0] = _mm_mul_pd(v0.v[0], v1.v[0]);
    v0.v[1] = _mm_mul_pd(v0.v[1], v1.v[1]);

    v2.v[0] = _mm_shuffle_pd(a->v[1], a->v[0], _MM_SHUFFLE2(0, 0));
    v2.v[1] = _mm_shuffle_pd(a->v[0], a->v[1], _MM_SHUFFLE2(1, 1));
    v3.v[0] = _mm_shuffle_pd(b->v[0], b->v[1], _MM_SHUFFLE2(0, 1));
    v3.v[1] = _mm_shuffle_pd(b->v[0], b->v[1], _MM_SHUFFLE2(1, 0));
    v2.v[0] = _mm_mul_pd(v2.v[0], v3.v[0]);
    v2.v[1] = _mm_mul_pd(v2.v[1], v3.v[1]);

    d->v[0] = _mm_sub_pd(v0.v[0], v2.v[0]);
    d->v[1] = _mm_sub_pd(v0.v[1], v2.v[1]);
# endif /* FER_SSE_SINGLE */
#else /* FER_SSE */
    d->f[0] = (a->f[1] * b->f[2]) - (a->f[2] * b->f[1]);
    d->f[1] = (a->f[2] * b->f[0]) - (a->f[0] * b->f[2]);
    d->f[2] = (a->f[0] * b->f[1]) - (a->f[1] * b->f[0]);
#endif /* FER_SSE */
}

_fer_inline void ferVec3Print(const fer_vec3_t *v, FILE *out)
{
    fprintf(out, "%g %g %g",
            (double)ferVec3X(v), (double)ferVec3Y(v), (double)ferVec3Z(v));
}

_fer_inline void ferVec3TriCentroid(const fer_vec3_t *a, const fer_vec3_t *b,
                                 const fer_vec3_t *c,
                                 fer_vec3_t *d)
{
    size_t i;

    for (i = 0; i < 3; i++){
        ferVec3SetCoord(d, i, (ferVec3Get(a, i) + ferVec3Get(b, i) + ferVec3Get(c, i)) / 3.);
    }
}

_fer_inline int ferVec3Collinear(const fer_vec3_t *a, const fer_vec3_t *b,
                                 const fer_vec3_t *c)
{
    fer_vec3_t ab, ac, cross;

    ferVec3Sub2(&ab, b, a);
    ferVec3Sub2(&ac, c, a);
    ferVec3Cross(&cross, &ab, &ac);

    return ferVec3IsZero(&cross);
}

_fer_inline fer_real_t ferVec3Volume6(const fer_vec3_t *a,
                                      const fer_vec3_t *b,
                                      const fer_vec3_t *c,
                                      const fer_vec3_t *d)
{
    fer_real_t adx, bdx, cdx;
    fer_real_t ady, bdy, cdy;
    fer_real_t adz, bdz, cdz;

    adx = ferVec3X(a) - ferVec3X(d);
    bdx = ferVec3X(b) - ferVec3X(d);
    cdx = ferVec3X(c) - ferVec3X(d);
    ady = ferVec3Y(a) - ferVec3Y(d);
    bdy = ferVec3Y(b) - ferVec3Y(d);
    cdy = ferVec3Y(c) - ferVec3Y(d);
    adz = ferVec3Z(a) - ferVec3Z(d);
    bdz = ferVec3Z(b) - ferVec3Z(d);
    cdz = ferVec3Z(c) - ferVec3Z(d);

    return adx * (bdy * cdz - bdz * cdy)
            + bdx * (cdy * adz - cdz * ady)
            + cdx * (ady * bdz - adz * bdy);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_VEC3_H__ */
