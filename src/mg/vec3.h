#ifndef __MG_VEC3_H__
#define __MG_VEC3_H__

#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <mg/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _mg_vec3_t {
    mg_real_t v[3];
};
typedef struct _mg_vec3_t mg_vec3_t;


/**
 * Holds origin (0,0,0) - this variable is meant to be read-only!
 */
extern mg_vec3_t *mg_vec3_origin;

/**
 * Array of points uniformly distributed on unit sphere.
 */
extern mg_vec3_t *mg_points_on_sphere;
extern size_t mg_points_on_sphere_len;

/** Returns sign of value. */
_mg_inline int mgSign(mg_real_t val);
/** Returns true if val is zero. **/
_mg_inline int mgIsZero(mg_real_t val);
/** Returns true if a and b equal. **/
_mg_inline int mgEq(mg_real_t a, mg_real_t b);


#define MG_VEC3_STATIC(x, y, z) \
    { { (x), (y), (z) } }

#define MG_VEC3(name, x, y, z) \
    mg_vec3_t name = MG_VEC3_STATIC((x), (y), (z))

_mg_inline mg_real_t mgVec3X(const mg_vec3_t *v);
_mg_inline mg_real_t mgVec3Y(const mg_vec3_t *v);
_mg_inline mg_real_t mgVec3Z(const mg_vec3_t *v);

/**
 * Returns true if a and b equal.
 */
_mg_inline int mgVec3Eq(const mg_vec3_t *a, const mg_vec3_t *b);

/**
 * Returns squared length of vector.
 */
_mg_inline mg_real_t mgVec3Len2(const mg_vec3_t *v);

/**
 * Returns distance between a and b.
 */
_mg_inline mg_real_t mgVec3Dist2(const mg_vec3_t *a, const mg_vec3_t *b);


_mg_inline void mgVec3Set(mg_vec3_t *v, mg_real_t x, mg_real_t y, mg_real_t z);

/**
 * v = w
 */
_mg_inline void mgVec3Copy(mg_vec3_t *v, const mg_vec3_t *w);

/**
 * Substracts coordinates of vector w from vector v. v = v - w
 */
_mg_inline void mgVec3Sub(mg_vec3_t *v, const mg_vec3_t *w);

/**
 * Adds coordinates of vector w to vector v. v = v + w
 */
_mg_inline void mgVec3Add(mg_vec3_t *v, const mg_vec3_t *w);

/**
 * d = v - w
 */
_mg_inline void mgVec3Sub2(mg_vec3_t *d, const mg_vec3_t *v, const mg_vec3_t *w);

/**
 * d = d * k;
 */
_mg_inline void mgVec3Scale(mg_vec3_t *d, mg_real_t k);

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
 * was computaed distance to point P.
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


/**** INLINES ****/
_mg_inline int mgSign(mg_real_t val)
{
    if (mgIsZero(val)){
        return 0;
    }else if (val < MG_ZERO){
        return -1;
    }
    return 1;
}

_mg_inline int mgIsZero(mg_real_t val)
{
    return MG_FABS(val) < MG_EPS;
}

_mg_inline int mgEq(mg_real_t _a, mg_real_t _b)
{
    mg_real_t ab;

    ab = MG_FABS(_a - _b);
    if (MG_FABS(ab) < MG_EPS)
        return 1;

    mg_real_t a, b;
    a = MG_FABS(_a);
    b = MG_FABS(_b);
    if (b > a){
        return ab < MG_EPS * b;
    }else{
        return ab < MG_EPS * a;
    }
}


_mg_inline mg_real_t mgVec3X(const mg_vec3_t *v)
{
    return v->v[0];
}

_mg_inline mg_real_t mgVec3Y(const mg_vec3_t *v)
{
    return v->v[1];
}

_mg_inline mg_real_t mgVec3Z(const mg_vec3_t *v)
{
    return v->v[2];
}

_mg_inline int mgVec3Eq(const mg_vec3_t *a, const mg_vec3_t *b)
{
    return mgEq(mgVec3X(a), mgVec3X(b))
            && mgEq(mgVec3Y(a), mgVec3Y(b))
            && mgEq(mgVec3Z(a), mgVec3Z(b));
}

_mg_inline mg_real_t mgVec3Len2(const mg_vec3_t *v)
{
    return mgVec3Dot(v, v);
}

_mg_inline mg_real_t mgVec3Dist2(const mg_vec3_t *a, const mg_vec3_t *b)
{
    mg_vec3_t ab;
    mgVec3Sub2(&ab, a, b);
    return mgVec3Len2(&ab);
}

_mg_inline void mgVec3Set(mg_vec3_t *v, mg_real_t x, mg_real_t y, mg_real_t z)
{
    v->v[0] = x;
    v->v[1] = y;
    v->v[2] = z;
}

_mg_inline void mgVec3Copy(mg_vec3_t *v, const mg_vec3_t *w)
{
    *v = *w;
}

_mg_inline void mgVec3Sub(mg_vec3_t *v, const mg_vec3_t *w)
{
    v->v[0] -= w->v[0];
    v->v[1] -= w->v[1];
    v->v[2] -= w->v[2];
}
_mg_inline void mgVec3Sub2(mg_vec3_t *d, const mg_vec3_t *v, const mg_vec3_t *w)
{
    d->v[0] = v->v[0] - w->v[0];
    d->v[1] = v->v[1] - w->v[1];
    d->v[2] = v->v[2] - w->v[2];
}

_mg_inline void mgVec3Add(mg_vec3_t *v, const mg_vec3_t *w)
{
    v->v[0] += w->v[0];
    v->v[1] += w->v[1];
    v->v[2] += w->v[2];
}

_mg_inline void mgVec3Scale(mg_vec3_t *d, mg_real_t k)
{
    d->v[0] *= k;
    d->v[1] *= k;
    d->v[2] *= k;
}

_mg_inline void mgVec3Normalize(mg_vec3_t *d)
{
    mg_real_t k = MG_ONE / MG_SQRT(mgVec3Len2(d));
    mgVec3Scale(d, k);
}

_mg_inline mg_real_t mgVec3Dot(const mg_vec3_t *a, const mg_vec3_t *b)
{
    mg_real_t dot;

    dot  = a->v[0] * b->v[0];
    dot += a->v[1] * b->v[1];
    dot += a->v[2] * b->v[2];
    return dot;
}

_mg_inline void mgVec3Cross(mg_vec3_t *d, const mg_vec3_t *a, const mg_vec3_t *b)
{
    d->v[0] = (a->v[1] * b->v[2]) - (a->v[2] * b->v[1]);
    d->v[1] = (a->v[2] * b->v[0]) - (a->v[0] * b->v[2]);
    d->v[2] = (a->v[0] * b->v[1]) - (a->v[1] * b->v[0]);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __MG_VEC3_H__ */
