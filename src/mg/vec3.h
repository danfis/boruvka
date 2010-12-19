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

#ifndef __MG_VEC3_H__
#define __MG_VEC3_H__

#include <mg/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MG_SSE_SINGLE
#include <immintrin.h>

union _mg_vec3_t {
    __m128 v;
    float f[4];
} mg_aligned(16) mg_packed;
typedef union _mg_vec3_t mg_vec3_t;

#else /* MG_SSE_SINGLE */
struct _mg_vec3_t {
    mg_real_t v[4];
};
typedef struct _mg_vec3_t mg_vec3_t;
#endif /* MG_SSE_SINGLE */


/**
 * Holds origin (0,0,0) - this variable is meant to be read-only!
 */
extern const mg_vec3_t *mg_vec3_origin;

/**
 * Array of points uniformly distributed on unit sphere.
 */
extern mg_vec3_t *mg_points_on_sphere;
extern size_t mg_points_on_sphere_len;

#define MG_VEC3_STATIC(x, y, z) \
    { { (x), (y), (z) } }

#define MG_VEC3(name, x, y, z) \
    mg_vec3_t name = MG_VEC3_STATIC((x), (y), (z))

/**
 * Allocate and initialize new vector.
 */
mg_vec3_t *mgVec3New(mg_real_t x, mg_real_t y, mg_real_t z);

/**
 * Delete vector.
 */
void mgVec3Del(mg_vec3_t *);

/**
 * Clone given mg_vec3_t. This does deep copy.
 */
_mg_inline mg_vec3_t *mgVec3Clone(const mg_vec3_t *v);

/**
 * v = w
 */
_mg_inline void mgVec3Copy(mg_vec3_t *v, const mg_vec3_t *w);


_mg_inline mg_real_t mgVec3X(const mg_vec3_t *v);
_mg_inline mg_real_t mgVec3Y(const mg_vec3_t *v);
_mg_inline mg_real_t mgVec3Z(const mg_vec3_t *v);
_mg_inline mg_real_t mgVec3Get(const mg_vec3_t *v, int d);

_mg_inline void mgVec3SetX(mg_vec3_t *v, mg_real_t val);
_mg_inline void mgVec3SetY(mg_vec3_t *v, mg_real_t val);
_mg_inline void mgVec3SetZ(mg_vec3_t *v, mg_real_t val);
_mg_inline void mgVec3Set(mg_vec3_t *v, mg_real_t x, mg_real_t y, mg_real_t z);
_mg_inline void mgVec3SetCoord(mg_vec3_t *v, int d, mg_real_t val);


/**
 * Returns true if a and b equal.
 */
_mg_inline int mgVec3Eq(const mg_vec3_t *a, const mg_vec3_t *b);
_mg_inline int mgVec3NEq(const mg_vec3_t *a, const mg_vec3_t *b);
_mg_inline int mgVec3Eq2(const mg_vec3_t *a, mg_real_t x, mg_real_t y, mg_real_t z);
_mg_inline int mgVec3NEq2(const mg_vec3_t *a, mg_real_t x, mg_real_t y, mg_real_t z);


/**
 * Returns squared length of vector.
 */
_mg_inline mg_real_t mgVec3Len2(const mg_vec3_t *v);
_mg_inline mg_real_t mgVec3Len(const mg_vec3_t *v);

/**
 * Returns squared distance between a and b.
 */
_mg_inline mg_real_t mgVec3Dist2(const mg_vec3_t *a, const mg_vec3_t *b);
_mg_inline mg_real_t mgVec3Dist(const mg_vec3_t *a, const mg_vec3_t *b);


/**
 * Adds coordinates of vector w to vector v. v = v + w
 */
_mg_inline void mgVec3Add(mg_vec3_t *v, const mg_vec3_t *w);

/**
 * d = v + w
 */
_mg_inline void mgVec3Add2(mg_vec3_t *d, const mg_vec3_t *v, const mg_vec3_t *w);


/**
 * Substracts coordinates of vector w from vector v. v = v - w
 */
_mg_inline void mgVec3Sub(mg_vec3_t *v, const mg_vec3_t *w);

/**
 * d = v - w
 */
_mg_inline void mgVec3Sub2(mg_vec3_t *d, const mg_vec3_t *v, const mg_vec3_t *w);

/**
 * d = d * k;
 */
_mg_inline void mgVec3Scale(mg_vec3_t *d, mg_real_t k);

/**
 * Scales vector v to given length.
 */
_mg_inline void mgVec3ScaleToLen(mg_vec3_t *v, mg_real_t len);

/**
 * Normalizes given vector to unit length.
 */
_mg_inline void mgVec3Normalize(mg_vec3_t *d);


/**
 * Dot product of two vectors.
 */
_mg_inline mg_real_t mgVec3Dot(const mg_vec3_t *a, const mg_vec3_t *b);

/**
 * Cross product: d = a x b.
 */
_mg_inline void mgVec3Cross(mg_vec3_t *d, const mg_vec3_t *a, const mg_vec3_t *b);


/**
 * Returns distance^2 of point P to segment ab.
 * If witness is non-NULL it is filled with coordinates of point from which
 * was computed distance to point P.
 */
mg_real_t mgVec3PointSegmentDist2(const mg_vec3_t *P,
                                  const mg_vec3_t *a, const mg_vec3_t *b,
                                  mg_vec3_t *witness);

/**
 * Returns distance^2 of point P from triangle formed by triplet a, b, c.
 * If witness vector is provided it is filled with coordinates of point
 * from which was computed distance to point P.
 */
mg_real_t mgVec3PointTriDist2(const mg_vec3_t *P,
                              const mg_vec3_t *a, const mg_vec3_t *b,
                              const mg_vec3_t *c,
                              mg_vec3_t *witness);

/**
 * Returns true if point p lies on triangle abc. If witness is non-NULL it
 * will be filled with witness point if p lies on triangle.
 */
int mgVec3PointInTri(const mg_vec3_t *p,
                     const mg_vec3_t *a, const mg_vec3_t *b,
                     const mg_vec3_t *c);

/**
 * Returns angle in b between points a and b.
 * Returned value is between 0 and PI.
 */
mg_real_t mgVec3Angle(const mg_vec3_t *a, const mg_vec3_t *b, const mg_vec3_t *c);

/**
 * Returns dihedral angle between planes abc and bcd.
 */
mg_real_t mgVec3DihedralAngle(const mg_vec3_t *a, const mg_vec3_t *b,
                              const mg_vec3_t *c, const mg_vec3_t *d);

/**
 * Stores in d projection of point p onto plane defined by three points
 * u, v, w.
 * Returns distance of p from plane or negative number on error (i.e. when
 * u, v, w does not define plane)
 */
mg_real_t mgVec3ProjToPlane(const mg_vec3_t *p,
                            const mg_vec3_t *u, const mg_vec3_t *v,
                            const mg_vec3_t *w, mg_vec3_t *d);

/**
 * Stores in d projection of point p onto plane defined by its normal and
 * point x belonging to.
 * Returns distance of p from plane or negative number on error (i.e. when
 * normal is zero vector).
 */
mg_real_t mgVec3ProjToPlane2(const mg_vec3_t *p,
                             const mg_vec3_t *x, const mg_vec3_t *normal,
                             mg_vec3_t *d);


/**
 * Returns twice of area of triangle.
 */
mg_real_t mgVec3TriArea2(const mg_vec3_t *a, const mg_vec3_t *b,
                         const mg_vec3_t *c);

/**
 * Stores in d centroid of triangle abc.
 */
_mg_inline void mgVec3TriCentroid(const mg_vec3_t *a, const mg_vec3_t *b,
                                  const mg_vec3_t *c, mg_vec3_t *d);


/**** INLINES ****/
_mg_inline mg_vec3_t *mgVec3Clone(const mg_vec3_t *v)
{
    return mgVec3New(mgVec3X(v), mgVec3Y(v), mgVec3Z(v));
}

_mg_inline mg_real_t mgVec3Get(const mg_vec3_t *v, int d)
{
#ifdef MG_SSE_SINGLE
    return v->f[d];
#else /* MG_SSE_SINGLE */
    return v->v[d];
#endif /* MG_SSE_SINGLE */
}

_mg_inline mg_real_t mgVec3X(const mg_vec3_t *v)
{
#ifdef MG_SSE_SINGLE
    return v->f[0];
#else /* MG_SSE_SINGLE */
    return v->v[0];
#endif /* MG_SSE_SINGLE */
}

_mg_inline mg_real_t mgVec3Y(const mg_vec3_t *v)
{
#ifdef MG_SSE_SINGLE
    return v->f[1];
#else /* MG_SSE_SINGLE */
    return v->v[1];
#endif /* MG_SSE_SINGLE */
}

_mg_inline mg_real_t mgVec3Z(const mg_vec3_t *v)
{
#ifdef MG_SSE_SINGLE
    return v->f[2];
#else /* MG_SSE_SINGLE */
    return v->v[2];
#endif /* MG_SSE_SINGLE */
}

_mg_inline int mgVec3Eq(const mg_vec3_t *a, const mg_vec3_t *b)
{
    return mgEq(mgVec3X(a), mgVec3X(b))
            && mgEq(mgVec3Y(a), mgVec3Y(b))
            && mgEq(mgVec3Z(a), mgVec3Z(b));
}

_mg_inline int mgVec3NEq(const mg_vec3_t *a, const mg_vec3_t *b)
{
    return !mgVec3Eq(a, b);
}

_mg_inline int mgVec3Eq2(const mg_vec3_t *a, mg_real_t x, mg_real_t y, mg_real_t z)
{
    return mgEq(mgVec3X(a), x)
            && mgEq(mgVec3Y(a), y)
            && mgEq(mgVec3Z(a), z);
}

_mg_inline int mgVec3NEq2(const mg_vec3_t *a, mg_real_t x, mg_real_t y, mg_real_t z)
{
    return !mgVec3Eq2(a, x, y, z);
}

_mg_inline mg_real_t mgVec3Len2(const mg_vec3_t *v)
{
    return mgVec3Dot(v, v);
}
_mg_inline mg_real_t mgVec3Len(const mg_vec3_t *v)
{
    return MG_SQRT(mgVec3Len2(v));
}

_mg_inline mg_real_t mgVec3Dist2(const mg_vec3_t *a, const mg_vec3_t *b)
{
    mg_vec3_t ab;
    mgVec3Sub2(&ab, a, b);
    return mgVec3Len2(&ab);
}
_mg_inline mg_real_t mgVec3Dist(const mg_vec3_t *a, const mg_vec3_t *b)
{
    return MG_SQRT(mgVec3Dist2(a, b));
}

_mg_inline void mgVec3Set(mg_vec3_t *v, mg_real_t x, mg_real_t y, mg_real_t z)
{
#ifdef MG_SSE_SINGLE
    //v->v = _mm_set_ps(x, y, z, MG_ZERO);
    v->f[0] = x;
    v->f[1] = y;
    v->f[2] = z;
#else /* MG_SSE_SINGLE */
    v->v[0] = x;
    v->v[1] = y;
    v->v[2] = z;
#endif /* MG_SSE_SINGLE */
}
_mg_inline void mgVec3SetCoord(mg_vec3_t *v, int d, mg_real_t val)
{
#ifdef MG_SSE_SINGLE
    v->f[d] = val;
#else /* MG_SSE_SINGLE */
    v->v[d] = val;
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3SetX(mg_vec3_t *v, mg_real_t val)
{
#ifdef MG_SSE_SINGLE
    //v->v = _mm_set_ps(val, v->f[1], v->f[2], MG_ZERO);
    v->f[0] = val;
#else /* MG_SSE_SINGLE */
    v->v[0] = val;
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3SetY(mg_vec3_t *v, mg_real_t val)
{
#ifdef MG_SSE_SINGLE
    //v->v = _mm_set_ps(v->f[0], val, v->f[1], MG_ZERO);
    v->f[1] = val;
#else /* MG_SSE_SINGLE */
    v->v[1] = val;
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3SetZ(mg_vec3_t *v, mg_real_t val)
{
#ifdef MG_SSE_SINGLE
    //v->v = _mm_set_ps(v->f[0], v->f[1], val, MG_ZERO);
    v->f[2] = val;
#else /* MG_SSE_SINGLE */
    v->v[2] = val;
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3Copy(mg_vec3_t *v, const mg_vec3_t *w)
{
    *v = *w;
}

_mg_inline void mgVec3Add(mg_vec3_t *v, const mg_vec3_t *w)
{
#ifdef MG_SSE_SINGLE
    v->v = _mm_add_ps(v->v, w->v);
#else /* MG_SSE_SINGLE */
    v->v[0] += w->v[0];
    v->v[1] += w->v[1];
    v->v[2] += w->v[2];
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3Add2(mg_vec3_t *d, const mg_vec3_t *v, const mg_vec3_t *w)
{
#ifdef MG_SSE_SINGLE
    d->v = _mm_add_ps(v->v, w->v);
#else /* MG_SSE_SINGLE */
    d->v[0] = v->v[0] + w->v[0];
    d->v[1] = v->v[1] + w->v[1];
    d->v[2] = v->v[2] + w->v[2];
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3Sub(mg_vec3_t *v, const mg_vec3_t *w)
{
#ifdef MG_SSE_SINGLE
    mgVec3Add(v, w);
    //v->v = _mm_sub_ps(v->v, w->v);
#else /* MG_SSE_SINGLE */
    v->v[0] -= w->v[0];
    v->v[1] -= w->v[1];
    v->v[2] -= w->v[2];
#endif /* MG_SSE_SINGLE */
}
_mg_inline void mgVec3Sub2(mg_vec3_t *d, const mg_vec3_t *v, const mg_vec3_t *w)
{
#ifdef MG_SSE_SINGLE
    d->v = _mm_sub_ps(v->v, w->v);
#else /* MG_SSE_SINGLE */
    d->v[0] = v->v[0] - w->v[0];
    d->v[1] = v->v[1] - w->v[1];
    d->v[2] = v->v[2] - w->v[2];
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3Scale(mg_vec3_t *d, mg_real_t _k)
{
#ifdef MG_SSE_SINGLE
    mg_vec3_t k;
    k.v = _mm_set1_ps(_k);
    d->v = _mm_mul_ps(d->v, k.v);
#else /* MG_SSE_SINGLE */
    d->v[0] *= _k;
    d->v[1] *= _k;
    d->v[2] *= _k;
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3ScaleToLen(mg_vec3_t *v, mg_real_t len)
{
#ifdef MG_SSE_SINGLE
    mg_vec3_t k, l;

    k.v = _mm_set1_ps(mgVec3Len2(v));
    k.v = _mm_sqrt_ps(k.v);
    l.v = _mm_set1_ps(len);
    k.v = _mm_div_ps(k.v, l.v);
    v->v = _mm_div_ps(v->v, k.v);
#else /* MG_SSE_SINGLE */
    mg_real_t k = len / MG_SQRT(mgVec3Len2(v));
    mgVec3Scale(v, k);
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3Normalize(mg_vec3_t *d)
{
#ifdef MG_SSE_SINGLE
    mg_vec3_t k;

    k.v = _mm_set1_ps(mgVec3Len2(d));
    k.v = _mm_sqrt_ps(k.v);
    d->v = _mm_div_ps(d->v, k.v);
#else /* MG_SSE_SINGLE */
    mg_real_t k = MG_ONE / MG_SQRT(mgVec3Len2(d));
    mgVec3Scale(d, k);
#endif /* MG_SSE_SINGLE */
}

_mg_inline mg_real_t mgVec3Dot(const mg_vec3_t *a, const mg_vec3_t *b)
{
#ifdef MG_SSE_SINGLE
    mg_vec3_t dot;
    /* TODO: SSE 4.1
    static const int mask = ~0;

    dot->v = _mm_dp_ps(a->v, b->v, mask);
    */
    dot.v = _mm_mul_ps(a->v, b->v);
    dot.v = _mm_hadd_ps(dot.v, mg_vec3_origin->v);
    dot.v = _mm_hadd_ps(dot.v, mg_vec3_origin->v);

    return mgVec3X(&dot);
#else /* MG_SSE_SINGLE */
    mg_real_t dot;

    dot  = a->v[0] * b->v[0];
    dot += a->v[1] * b->v[1];
    dot += a->v[2] * b->v[2];

    return dot;
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3Cross(mg_vec3_t *d, const mg_vec3_t *a, const mg_vec3_t *b)
{
#ifdef MG_SSE_SINGLE
    mg_vec3_t v0, v1, v2, v3;

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
#else /* MG_SSE_SINGLE */
    d->v[0] = (a->v[1] * b->v[2]) - (a->v[2] * b->v[1]);
    d->v[1] = (a->v[2] * b->v[0]) - (a->v[0] * b->v[2]);
    d->v[2] = (a->v[0] * b->v[1]) - (a->v[1] * b->v[0]);
#endif /* MG_SSE_SINGLE */
}

_mg_inline void mgVec3TriCentroid(const mg_vec3_t *a, const mg_vec3_t *b,
                                 const mg_vec3_t *c,
                                 mg_vec3_t *d)
{
    size_t i;

    for (i = 0; i < 3; i++){
        mgVec3SetCoord(d, i, (mgVec3Get(a, i) + mgVec3Get(b, i) + mgVec3Get(c, i)) / 3.);
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __MG_VEC3_H__ */
