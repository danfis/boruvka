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

#ifndef __BOR_VEC3_H__
#define __BOR_VEC3_H__

#include <stdio.h>
#include <boruvka/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Vec3 - 3D vector
 * =================
 *
 *
 * .. c:type:: bor_vec3_t
 */

/** vvvvv */
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
union _bor_vec3_t {
    __m128 v;
    float f[4];
} bor_aligned(16) bor_packed;
typedef union _bor_vec3_t bor_vec3_t;
# else /* BOR_SSE_SINGLE */
union _bor_vec3_t {
    __m128d v[2];
    double f[4];
} bor_aligned(16) bor_packed;
typedef union _bor_vec3_t bor_vec3_t;
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
struct _bor_vec3_t {
    bor_real_t f[4];
};
typedef struct _bor_vec3_t bor_vec3_t;
#endif /* BOR_SSE */
/** ^^^^^ */


/**
 * Holds origin (0,0,0) - this variable is meant to be read-only!
 */
extern const bor_vec3_t *bor_vec3_origin;

/**
 * X, Y and Z axis - read-only.
 */
extern const bor_vec3_t *bor_vec3_axis[3];

/**
 * Array of points uniformly distributed on unit sphere.
 */
extern bor_vec3_t *bor_points_on_sphere;
extern size_t bor_points_on_sphere_len;

# define BOR_VEC3_STATIC(x, y, z) \
    { .f = { (x), (y), (z), BOR_ZERO } }

#define BOR_VEC3(name, x, y, z) \
    bor_vec3_t name = BOR_VEC3_STATIC((x), (y), (z))

/**
 * Functions
 * ----------
 */

/**
 * Allocate and initialize new vector.
 */
bor_vec3_t *borVec3New(bor_real_t x, bor_real_t y, bor_real_t z);

/**
 * Delete vector.
 */
void borVec3Del(bor_vec3_t *);

/**
 * Allocates array of vectors.
 * Use this function instead of classical malloc() due to alignement that
 * could be required if you use SSE version of library.
 * Use borVec3ArrDel() to free allocated memory.
 */
bor_vec3_t *borVec3ArrNew(size_t num_vecs);

/**
 * Deletes array created by borVec3ArrNew().
 */
void borVec3ArrDel(bor_vec3_t *);

/**
 * Returns memory aligned to vector size.
 * This can be useful if you have somewhere allocated memory but you want
 * to use it for vectors and you use SSE version of library.
 */
_bor_inline bor_vec3_t *borVec3Align(void *mem);

/**
 * Clone given bor_vec3_t. This does deep copy.
 */
_bor_inline bor_vec3_t *borVec3Clone(const bor_vec3_t *v);

/**
 * v = w
 */
_bor_inline void borVec3Copy(bor_vec3_t *v, const bor_vec3_t *w);


/**
 * Returns x coordinate
 */
_bor_inline bor_real_t borVec3X(const bor_vec3_t *v);

/**
 * Returns y coordinate
 */
_bor_inline bor_real_t borVec3Y(const bor_vec3_t *v);

/**
 * Returns z coordinate
 */
_bor_inline bor_real_t borVec3Z(const bor_vec3_t *v);

/**
 * Returns d'th coordinate
 */
_bor_inline bor_real_t borVec3Get(const bor_vec3_t *v, int d);

/**
 * a.x = v
 */
_bor_inline void borVec3SetX(bor_vec3_t *a, bor_real_t v);

/**
 * a.y = v
 */
_bor_inline void borVec3SetY(bor_vec3_t *a, bor_real_t v);

/**
 * a.z = v
 */
_bor_inline void borVec3SetZ(bor_vec3_t *a, bor_real_t v);

/**
 * a = (x, y, z)
 */
_bor_inline void borVec3Set(bor_vec3_t *v, bor_real_t x, bor_real_t y, bor_real_t z);

/**
 * Sets {d}'th coordinate to {val}.
 */
_bor_inline void borVec3SetCoord(bor_vec3_t *v, int d, bor_real_t val);

/**
 * Sets {d}'th coordinate to {val}.
 */
_bor_inline void borVec3Set1(bor_vec3_t *v, int d, bor_real_t val);


/**
 * Returns true if {a} and {b} equal.
 */
_bor_inline int borVec3Eq(const bor_vec3_t *a, const bor_vec3_t *b);

/**
 * Returns true if {a} and {b} are not equal
 */
_bor_inline int borVec3NEq(const bor_vec3_t *a, const bor_vec3_t *b);

/**
 * Returns true if {a} equals to ({x}, {y}, {z})
 */
_bor_inline int borVec3Eq2(const bor_vec3_t *a, bor_real_t x, bor_real_t y, bor_real_t z);

/**
 * Returns true if {a} doesn't equal to ({x}, {y}, {z})
 */
_bor_inline int borVec3NEq2(const bor_vec3_t *a, bor_real_t x, bor_real_t y, bor_real_t z);

/**
 * Returns true if {a} is zero vector.
 */
_bor_inline int borVec3IsZero(const bor_vec3_t *a);


/**
 * Returns squared length of vector.
 */
_bor_inline bor_real_t borVec3Len2(const bor_vec3_t *v);

/**
 * Returns length of vector
 */
_bor_inline bor_real_t borVec3Len(const bor_vec3_t *v);

/**
 * Returns squared distance between a and b.
 */
_bor_inline bor_real_t borVec3Dist2(const bor_vec3_t *a, const bor_vec3_t *b);

/**
 * Returns distance between {a} and {b}
 */
_bor_inline bor_real_t borVec3Dist(const bor_vec3_t *a, const bor_vec3_t *b);


/**
 * Adds coordinates of vector w to vector v.
 * v = v + w
 */
_bor_inline void borVec3Add(bor_vec3_t *v, const bor_vec3_t *w);

/**
 * d = v + w
 */
_bor_inline void borVec3Add2(bor_vec3_t *d, const bor_vec3_t *v, const bor_vec3_t *w);

/**
 * d = f * w
 */
_bor_inline void borVec3AddScaled(bor_vec3_t *d, bor_real_t f, const bor_vec3_t *w);


/**
 * Substracts coordinates of vector w from vector v.
 * v = v - w
 */
_bor_inline void borVec3Sub(bor_vec3_t *v, const bor_vec3_t *w);

/**
 * d = v - w
 */
_bor_inline void borVec3Sub2(bor_vec3_t *d, const bor_vec3_t *v, const bor_vec3_t *w);

/**
 * v = v + (f, f, f)
 */
_bor_inline void borVec3AddConst(bor_vec3_t *v, bor_real_t f);

/**
 * d = v + (f, f, f)
 */
_bor_inline void borVec3AddConst2(bor_vec3_t *d, const bor_vec3_t *v, bor_real_t f);

/**
 * v = v - (f, f, f)
 */
_bor_inline void borVec3SubConst(bor_vec3_t *v, bor_real_t f);

/**
 * d = v - (f, f, f)
 */
_bor_inline void borVec3SubConst2(bor_vec3_t *d, const bor_vec3_t *v, bor_real_t f);


/**
 * d = d * k;
 */
_bor_inline void borVec3Scale(bor_vec3_t *d, bor_real_t k);

/**
 * d = v * k;
 */
_bor_inline void borVec3Scale2(bor_vec3_t *d, const bor_vec3_t *v, bor_real_t k);

/**
 * Scales vector v to given length.
 */
_bor_inline void borVec3ScaleToLen(bor_vec3_t *v, bor_real_t len);

/**
 * Normalizes given vector to unit length.
 */
_bor_inline void borVec3Normalize(bor_vec3_t *d);


/**
 * Dot product of two vectors.
 */
_bor_inline bor_real_t borVec3Dot(const bor_vec3_t *a, const bor_vec3_t *b);

/**
 * Multiplies vectors component wise:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 * a.z = a.z * b.z
 */
_bor_inline void borVec3MulComp(bor_vec3_t *a, const bor_vec3_t *b);

/**
 * a.x = b.x * c.x
 * a.y = b.y * c.y
 * a.z = b.z * c.z
 */
_bor_inline void borVec3MulComp2(bor_vec3_t *a, const bor_vec3_t *b, const bor_vec3_t *c);

/**
 * Cross product:
 * d = a x b.
 */
_bor_inline void borVec3Cross(bor_vec3_t *d, const bor_vec3_t *a, const bor_vec3_t *b);


/**
 * Prints vector to *out* in form "x y z".
 */
_bor_inline void borVec3Print(const bor_vec3_t *v, FILE *out);

/**
 * Returns distance^2 of point P to segment ab.
 * If witness is non-NULL it is filled with coordinates of point from which
 * was computed distance to point P.
 */
bor_real_t borVec3PointSegmentDist2(const bor_vec3_t *P,
                                    const bor_vec3_t *a, const bor_vec3_t *b,
                                    bor_vec3_t *witness);

/**
 * Returns distance^2 of two segments ab and cd.
 * If {witness1} (or {witness2}) is non-NULL it is filled with coordinates of
 * closes points. If {parallel} is non-NULL it is filled with 1 if the
 * segments are parallel.
 */
bor_real_t borVec3SegmentSegmentDist2(const bor_vec3_t *a, const bor_vec3_t *b,
                                      const bor_vec3_t *c, const bor_vec3_t *d,
                                      bor_vec3_t *witness1,
                                      bor_vec3_t *witness2,
                                      int *parallel);

/**
 * Returns distance^2 of point P from triangle formed by triplet a, b, c.
 * If witness vector is provided it is filled with coordinates of point
 * from which was computed distance to point P.
 */
bor_real_t borVec3PointTriDist2(const bor_vec3_t *P,
                                const bor_vec3_t *a, const bor_vec3_t *b,
                                const bor_vec3_t *c,
                                bor_vec3_t *witness);

/**
 * Returns true if point p lies on triangle abc. If witness is non-NULL it
 * will be filled with witness point if p lies on triangle.
 */
int borVec3PointInTri(const bor_vec3_t *p,
                      const bor_vec3_t *a, const bor_vec3_t *b,
                      const bor_vec3_t *c);

/**
 * Returns angle in b between points a and b.
 * Returned value is between 0 and PI.
 */
bor_real_t borVec3Angle(const bor_vec3_t *a, const bor_vec3_t *b, const bor_vec3_t *c);

/**
 * Returns dihedral angle between planes abc and bcd.
 */
bor_real_t borVec3DihedralAngle(const bor_vec3_t *a, const bor_vec3_t *b,
                                const bor_vec3_t *c, const bor_vec3_t *d);

/**
 * Stores in d projection of point p onto plane defined by three points
 * u, v, w.
 * Returns distance of p from plane or negative number on error (i.e. when
 * u, v, w does not define plane)
 */
bor_real_t borVec3ProjToPlane(const bor_vec3_t *p,
                              const bor_vec3_t *u, const bor_vec3_t *v,
                              const bor_vec3_t *w, bor_vec3_t *d);

/**
 * Stores in d projection of point p onto plane defined by its normal and
 * point x belonging to.
 * Returns distance of p from plane or negative number on error (i.e. when
 * normal is zero vector).
 */
bor_real_t borVec3ProjToPlane2(const bor_vec3_t *p,
                               const bor_vec3_t *x, const bor_vec3_t *normal,
                               bor_vec3_t *d);


/**
 * Returns twice of area of triangle.
 */
bor_real_t borVec3TriArea2(const bor_vec3_t *a, const bor_vec3_t *b,
                           const bor_vec3_t *c);

/**
 * Stores in d centroid of triangle abc.
 */
_bor_inline void borVec3TriCentroid(const bor_vec3_t *a, const bor_vec3_t *b,
                                    const bor_vec3_t *c, bor_vec3_t *d);

/**
 * Returns true if three given points are collinear.
 */
_bor_inline int borVec3Collinear(const bor_vec3_t *a, const bor_vec3_t *b,
                                 const bor_vec3_t *c);


/**
 * Returns 6-times *signed* volume of tetrahedron formed by points
 * a, b, c, d. If a, b, c are given in counterclockwise order then sign of
 * volume is:
 *     1) positive if d is below a plane formed by a, b, c
 *     2) negative if d is above
 *     3) zero if points are coplanar
 */
_bor_inline bor_real_t borVec3Volume6(const bor_vec3_t *a,
                                      const bor_vec3_t *b,
                                      const bor_vec3_t *c,
                                      const bor_vec3_t *d);


/**
 * Returns true if two given triangles (p1, q1, r1) and (p2, q2, r2) are
 * overlapping.
 */
int borVec3TriTriOverlap(const bor_vec3_t *p1, const bor_vec3_t *q1,
                         const bor_vec3_t *r1,
                         const bor_vec3_t *p2, const bor_vec3_t *q2,
                         const bor_vec3_t *r2);

/**
 * Computes intersection line of two triangles.
 * Returns 1 if triangles are intersecting and are not coplanar.
 * Returns 2 if triangles are intersecting but are coplanar and {s} and {t}
 * are not set.
 * Returns 0 if triangles does _not_ intersect.
 */
int borVec3TriTriIntersect(const bor_vec3_t *p1, const bor_vec3_t *q1,
                           const bor_vec3_t *r1,
                           const bor_vec3_t *p2, const bor_vec3_t *q2,
                           const bor_vec3_t *r2,
                           bor_vec3_t *s, bor_vec3_t *t);


/**** INLINES ****/
_bor_inline bor_vec3_t *borVec3Align(void *mem)
{
#ifdef BOR_SSE
    return (bor_vec3_t *)borAlign(mem, 16);
#else /* BOR_SSE */
    return (bor_vec3_t *)mem;
#endif /* BOR_SSE */
}

_bor_inline bor_vec3_t *borVec3Clone(const bor_vec3_t *v)
{
    return borVec3New(borVec3X(v), borVec3Y(v), borVec3Z(v));
}

_bor_inline bor_real_t borVec3Get(const bor_vec3_t *v, int d)
{
    return v->f[d];
}

_bor_inline bor_real_t borVec3X(const bor_vec3_t *v)
{
    return v->f[0];
}

_bor_inline bor_real_t borVec3Y(const bor_vec3_t *v)
{
    return v->f[1];
}

_bor_inline bor_real_t borVec3Z(const bor_vec3_t *v)
{
    return v->f[2];
}

_bor_inline int borVec3Eq(const bor_vec3_t *a, const bor_vec3_t *b)
{
    return borEq(borVec3X(a), borVec3X(b))
            && borEq(borVec3Y(a), borVec3Y(b))
            && borEq(borVec3Z(a), borVec3Z(b));
}

_bor_inline int borVec3NEq(const bor_vec3_t *a, const bor_vec3_t *b)
{
    return !borVec3Eq(a, b);
}

_bor_inline int borVec3Eq2(const bor_vec3_t *a, bor_real_t x, bor_real_t y, bor_real_t z)
{
    return borEq(borVec3X(a), x)
            && borEq(borVec3Y(a), y)
            && borEq(borVec3Z(a), z);
}

_bor_inline int borVec3NEq2(const bor_vec3_t *a, bor_real_t x, bor_real_t y, bor_real_t z)
{
    return !borVec3Eq2(a, x, y, z);
}

_bor_inline int borVec3IsZero(const bor_vec3_t *a)
{
    return borIsZero(borVec3X(a))
            && borIsZero(borVec3Y(a))
            && borIsZero(borVec3Z(a));
}

_bor_inline bor_real_t borVec3Len2(const bor_vec3_t *v)
{
    return borVec3Dot(v, v);
}
_bor_inline bor_real_t borVec3Len(const bor_vec3_t *v)
{
    return BOR_SQRT(borVec3Len2(v));
}

_bor_inline bor_real_t borVec3Dist2(const bor_vec3_t *a, const bor_vec3_t *b)
{
    bor_vec3_t ab;
    borVec3Sub2(&ab, a, b);
    return borVec3Len2(&ab);
}
_bor_inline bor_real_t borVec3Dist(const bor_vec3_t *a, const bor_vec3_t *b)
{
    return BOR_SQRT(borVec3Dist2(a, b));
}

_bor_inline void borVec3Set(bor_vec3_t *v, bor_real_t x, bor_real_t y, bor_real_t z)
{
    v->f[0] = x;
    v->f[1] = y;
    v->f[2] = z;
}
_bor_inline void borVec3SetCoord(bor_vec3_t *v, int d, bor_real_t val)
{
    v->f[d] = val;
}

_bor_inline void borVec3Set1(bor_vec3_t *v, int d, bor_real_t val)
{
    borVec3SetCoord(v, d, val);
}

_bor_inline void borVec3SetX(bor_vec3_t *v, bor_real_t val)
{
    v->f[0] = val;
}

_bor_inline void borVec3SetY(bor_vec3_t *v, bor_real_t val)
{
    v->f[1] = val;
}

_bor_inline void borVec3SetZ(bor_vec3_t *v, bor_real_t val)
{
    v->f[2] = val;
}

_bor_inline void borVec3Copy(bor_vec3_t *v, const bor_vec3_t *w)
{
    *v = *w;
}

_bor_inline void borVec3Add(bor_vec3_t *v, const bor_vec3_t *w)
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
#endif /* BOR_SSE */
}

_bor_inline void borVec3Add2(bor_vec3_t *d, const bor_vec3_t *v, const bor_vec3_t *w)
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
#endif /* BOR_SSE */
}

_bor_inline void borVec3AddScaled(bor_vec3_t *d, bor_real_t f, const bor_vec3_t *w)
{
    d->f[0] += f * w->f[0];
    d->f[1] += f * w->f[1];
    d->f[2] += f * w->f[2];
}

_bor_inline void borVec3Sub(bor_vec3_t *v, const bor_vec3_t *w)
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
#endif /* BOR_SSE */
}
_bor_inline void borVec3Sub2(bor_vec3_t *d, const bor_vec3_t *v, const bor_vec3_t *w)
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
#endif /* BOR_SSE */
}

_bor_inline void borVec3AddConst(bor_vec3_t *v, bor_real_t f)
{
    v->f[0] += f;
    v->f[1] += f;
    v->f[2] += f;
}

_bor_inline void borVec3AddConst2(bor_vec3_t *d, const bor_vec3_t *v, bor_real_t f)
{
    d->f[0] = v->f[0] + f;
    d->f[1] = v->f[1] + f;
    d->f[2] = v->f[2] + f;
}

_bor_inline void borVec3SubConst(bor_vec3_t *v, bor_real_t f)
{
    v->f[0] -= f;
    v->f[1] -= f;
    v->f[2] -= f;
}

_bor_inline void borVec3SubConst2(bor_vec3_t *d, const bor_vec3_t *v, bor_real_t f)
{
    d->f[0] = v->f[0] - f;
    d->f[1] = v->f[1] - f;
    d->f[2] = v->f[2] - f;
}

_bor_inline void borVec3Scale(bor_vec3_t *d, bor_real_t _k)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec3_t k;
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
#endif /* BOR_SSE */
}

_bor_inline void borVec3Scale2(bor_vec3_t *d, const bor_vec3_t *a, bor_real_t _k)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec3_t k;
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
#endif /* BOR_SSE */
}

_bor_inline void borVec3ScaleToLen(bor_vec3_t *v, bor_real_t len)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec3_t k, l;

    k.v = _mm_set1_ps(borVec3Len2(v));
    k.v = _mm_sqrt_ps(k.v);
    l.v = _mm_set1_ps(len);
    k.v = _mm_div_ps(k.v, l.v);
    v->v = _mm_div_ps(v->v, k.v);
# else /* BOR_SSE_SINGLE */
    __m128d k, l;

    k = _mm_set1_pd(borVec3Len2(v));
    k = _mm_sqrt_pd(k);
    l = _mm_set1_pd(len);
    k = _mm_div_pd(k, l);
    v->v[0] = _mm_div_pd(v->v[0], k);
    v->v[1] = _mm_div_pd(v->v[1], k);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    bor_real_t k = len * borRsqrt(borVec3Len2(v));
    borVec3Scale(v, k);
#endif /* BOR_SSE */
}

_bor_inline void borVec3Normalize(bor_vec3_t *d)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec3_t k;

    k.v = _mm_set1_ps(borVec3Len2(d));
    k.v = _mm_sqrt_ps(k.v);
    d->v = _mm_div_ps(d->v, k.v);
# else /* BOR_SSE_SINGLE */
    __m128d k;

    k = _mm_set1_pd(borVec3Len2(d));
    k = _mm_sqrt_pd(k);
    d->v[0] = _mm_div_pd(d->v[0], k);
    d->v[1] = _mm_div_pd(d->v[1], k);
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    bor_real_t k = borRsqrt(borVec3Len2(d));
    borVec3Scale(d, k);
#endif /* BOR_SSE */
}

_bor_inline bor_real_t borVec3Dot(const bor_vec3_t *a, const bor_vec3_t *b)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec3_t dot, t;

    dot.v = _mm_mul_ps(a->v, b->v);
    dot.f[3] = BOR_ZERO;
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(2, 3, 0, 1));
    dot.v = _mm_add_ps(dot.v, t.v);
    t.v = _mm_shuffle_ps(dot.v, dot.v, _MM_SHUFFLE(1, 0, 3, 2));
    dot.v = _mm_add_ps(dot.v, t.v);

    return dot.f[0];
# else /* BOR_SSE_SINGLE */
    bor_vec3_t dot;

    dot.v[0] = _mm_mul_pd(a->v[0], b->v[0]);
    dot.v[1] = _mm_mul_pd(a->v[1], b->v[1]);
    dot.f[3] = BOR_ZERO;
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

    return dot;
#endif /* BOR_SSE */
}

_bor_inline void borVec3MulComp(bor_vec3_t *a, const bor_vec3_t *b)
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
#endif /* BOR_SSE */
}

_bor_inline void borVec3MulComp2(bor_vec3_t *a, const bor_vec3_t *b, const bor_vec3_t *c)
{
    borVec3Copy(a, b);
    borVec3MulComp(a, c);
}

_bor_inline void borVec3Cross(bor_vec3_t *d, const bor_vec3_t *a, const bor_vec3_t *b)
{
#ifdef BOR_SSE
# ifdef BOR_SSE_SINGLE
    bor_vec3_t v0, v1, v2, v3;

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
# else /* BOR_SSE_SINGLE */
    bor_vec3_t v0, v1, v2, v3;

    borVec3Copy(&v0, a);
    borVec3Copy(&v1, b);
    borVec3Copy(&v2, a);
    borVec3Copy(&v3, b);

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
# endif /* BOR_SSE_SINGLE */
#else /* BOR_SSE */
    d->f[0] = (a->f[1] * b->f[2]) - (a->f[2] * b->f[1]);
    d->f[1] = (a->f[2] * b->f[0]) - (a->f[0] * b->f[2]);
    d->f[2] = (a->f[0] * b->f[1]) - (a->f[1] * b->f[0]);
#endif /* BOR_SSE */
}

_bor_inline void borVec3Print(const bor_vec3_t *v, FILE *out)
{
    fprintf(out, "%g %g %g",
            (double)borVec3X(v), (double)borVec3Y(v), (double)borVec3Z(v));
}

_bor_inline void borVec3TriCentroid(const bor_vec3_t *a, const bor_vec3_t *b,
                                 const bor_vec3_t *c,
                                 bor_vec3_t *d)
{
    size_t i;

    for (i = 0; i < 3; i++){
        borVec3SetCoord(d, i, (borVec3Get(a, i) + borVec3Get(b, i) + borVec3Get(c, i)) / 3.);
    }
}

_bor_inline int borVec3Collinear(const bor_vec3_t *a, const bor_vec3_t *b,
                                 const bor_vec3_t *c)
{
    bor_vec3_t ab, ac, cross;

    borVec3Sub2(&ab, b, a);
    borVec3Sub2(&ac, c, a);
    borVec3Cross(&cross, &ab, &ac);

    return borVec3IsZero(&cross);
}

_bor_inline bor_real_t borVec3Volume6(const bor_vec3_t *a,
                                      const bor_vec3_t *b,
                                      const bor_vec3_t *c,
                                      const bor_vec3_t *d)
{
    bor_real_t adx, bdx, cdx;
    bor_real_t ady, bdy, cdy;
    bor_real_t adz, bdz, cdz;

    adx = borVec3X(a) - borVec3X(d);
    bdx = borVec3X(b) - borVec3X(d);
    cdx = borVec3X(c) - borVec3X(d);
    ady = borVec3Y(a) - borVec3Y(d);
    bdy = borVec3Y(b) - borVec3Y(d);
    cdy = borVec3Y(c) - borVec3Y(d);
    adz = borVec3Z(a) - borVec3Z(d);
    bdz = borVec3Z(b) - borVec3Z(d);
    cdz = borVec3Z(c) - borVec3Z(d);

    return adx * (bdy * cdz - bdz * cdy)
            + bdx * (cdy * adz - cdz * ady)
            + cdx * (ady * bdz - adz * bdy);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_VEC3_H__ */
