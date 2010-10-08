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

#ifndef __MG_VEC_H__
#define __MG_VEC_H__

/**
 * Implementation of vectors.
 * Functions defined should be optimized as possible, so if you see some
 * drawback in implementation feel free to fix it!
 */

#include <mg/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Structure representing 2D vector.
 */
struct _mg_vec2_t {
    mg_real_t v[2];
};
typedef struct _mg_vec2_t mg_vec2_t;

/**
 * Holds origin (0,0) - this variable is meant to be read-only!
 */
extern const mg_vec2_t *mg_vec2_origin;

#define MG_VEC2_STATIC(x, y) \
    { { (x), (y) } }

#define MG_VEC2(name, x, y) \
    mg_vec2_t name = MG_VEC2_STATIC((x), (y))


/**
 * Allocate and initialize new vector.
 */
mg_vec2_t *mgVec2New(mg_real_t x, mg_real_t y);

/**
 * Delete vector.
 */
void mgVec2Del(mg_vec2_t *);

/**
 * Clone given mg_vec2_t. This does deep copy.
 */
_mg_inline mg_vec2_t *mgVec2Clone(const mg_vec2_t *v);

/**
 * Copies w into v.
 */
_mg_inline void mgVec2Copy(mg_vec2_t *v, const mg_vec2_t *w);


_mg_inline mg_real_t mgVec2X(const mg_vec2_t *v);
_mg_inline mg_real_t mgVec2Y(const mg_vec2_t *v);
_mg_inline mg_real_t mgVec2Get(const mg_vec2_t *v, int d);


_mg_inline void mgVec2SetX(mg_vec2_t *v, mg_real_t val);
_mg_inline void mgVec2SetY(mg_vec2_t *v, mg_real_t val);
_mg_inline void mgVec2Set(mg_vec2_t *v, mg_real_t x, mg_real_t y);

_mg_inline int mgVec2Eq(const mg_vec2_t *x, const mg_vec2_t *y);
_mg_inline int mgVec2NEq(const mg_vec2_t *x, const mg_vec2_t *y);

_mg_inline int mgVec2Eq2(const mg_vec2_t *v, mg_real_t x, mg_real_t y);
_mg_inline int mgVec2NEq2(const mg_vec2_t *v, mg_real_t x, mg_real_t y);


/**
 * Compute squared distance between two points represented as vectors.
 */
_mg_inline mg_real_t mgVec2Dist2(const mg_vec2_t *v, const mg_vec2_t *w);

/**
 * Squared length of vector.
 */
_mg_inline mg_real_t mgVec2Len2(const mg_vec2_t *v);


/**
 * Adds vector W to vector V (and result is stored in V).
 */
_mg_inline void mgVec2Add(mg_vec2_t *v, const mg_vec2_t *w);

/**
 * Substracts coordinates of vector W from vector V.
 */
_mg_inline void mgVec2Sub(mg_vec2_t *v, const mg_vec2_t *w);

/**
 *  v = w - ww
 */
_mg_inline void mgVec2Sub2(mg_vec2_t *v, const mg_vec2_t *w, const mg_vec2_t *ww);


/**
 * Scales vector V using constant k:
 *  v = k * v
 */
_mg_inline void mgVec2Scale(mg_vec2_t *v, mg_real_t k);


/**
 * Returns twice area enclosed by given vectors.
 * a, b, c should be named in counterclockwise order to get positive
 * area and clockwise to get negative.
 */
_mg_inline mg_real_t mgVec2Area2(const mg_vec2_t *a,
                                 const mg_vec2_t *b,
                                 const mg_vec2_t *c);


/**
 * Returns angle in b formed by vectors a, b, c.
 * Returned value is between 0 and PI
 */
mg_real_t mgVec2Angle(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c);

/**
 * Returns angle formed by points a, b, c in this order, it means, that
 * is returned angle in point b and in direction from segment ab to cb.
 * Returned angle is from -PI to PI. Positive angle is in
 * counterclockwise direction.
 */
_mg_inline mg_real_t mgVec2SignedAngle(const mg_vec2_t *a,
                                       const mg_vec2_t *b,
                                       const mg_vec2_t *c);


/**
 * This function computes projection of point C onto segment AB. Point of
 * projection is returned in X.
 *
 * Returns 0 if there exists any projection, otherwise -1.
 */
int mgVec2ProjectionPointOntoSegment(const mg_vec2_t *A, const mg_vec2_t *B,
                                     const mg_vec2_t *C,
                                     mg_vec2_t *X);


/**
 * Returns true if point d is in circle formed by points a, b, c.
 * Vectors a, b, c must be in counterclockwise order.
 */
int mgVec2InCircle(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c,
                   const mg_vec2_t *d);

/**
 * Returns true if point a lies on segment formed by b c.
 */
int mgVec2LiesOn(const mg_vec2_t *a, const mg_vec2_t *b, const mg_vec2_t *c);


/**
 * Returns true if a is collinear with b and c.
 */
#define mgVec2Collinear(a, b, c) \
    mgEq(mgVec2Area2((b), (c), (a)), MG_ZERO)

/**
 * Returns true, if vector v is in cone formed by p1, c, p2 (in
 * counterclockwise order).
 */
int mgVec2InCone(const mg_vec2_t *v,
                 const mg_vec2_t *p1, const mg_vec2_t *c, const mg_vec2_t *p2);


/**
 * Returns true if segment ab properly intersects segement cd (they share
 * point interior to both segments).
 * Properness of intersection means that two segmensts intersect at a point
 * interior to both segments. Improper intersection (which is not covered
 * by this function) means that one of end point lies somewhere on other
 * segment.
 */
_mg_inline int mgVec2IntersectProp(const mg_vec2_t *a,
                                   const mg_vec2_t *b,
                                   const mg_vec2_t *c,
                                   const mg_vec2_t *d);



/**
 * Returns true if segment ab intersects segment cd properly or improperly.
 */
_mg_inline int mgVec2Intersect(const mg_vec2_t *a, const mg_vec2_t *b,
                               const mg_vec2_t *c, const mg_vec2_t *d);


/**
 * Compute intersection point of two segments - (a, b) and (c, d).
 * Returns 0 if there exists intersection, -1 otherwise.
 * Intersetion point is returned in p, where p must point to already
 * allocated mg_vec2_t.
 */
int mgVec2IntersectPoint(const mg_vec2_t *a, const mg_vec2_t *b,
                         const mg_vec2_t *c, const mg_vec2_t *d,
                         mg_vec2_t *p);

/**
 * Returns true iff vector v is on left side from segment formed by p1 and
 * p2 in this ordering.
 */
_mg_inline int mgVec2OnLeft(const mg_vec2_t *v,
                            const mg_vec2_t *p1, const mg_vec2_t *p2);


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
int mgVec2SegmentInRect(const mg_vec2_t *a, const mg_vec2_t *b,
                        const mg_vec2_t *c, const mg_vec2_t *d,
                        const mg_vec2_t *x, const mg_vec2_t *y,
                        mg_vec2_t *s1, mg_vec2_t *s2);


/**
 * Returns angle by which must be vector b rotated about origin to have
 * same direction as vector a.
 * Returned angle is in range -PI, PI.
 */
mg_real_t mgVec2AngleSameDir(const mg_vec2_t *a, const mg_vec2_t *b);



/**
 * Retuns angle by which must be rotated oriented segment CD to have same
 * direction as oriented segment AB. Directional vector of segment CD is
 * vector (D - C) and directional vector of segment AB is vector (B - A).
 * Returned angle will be between -PI and PI.
 */
mg_real_t mgVec2AngleSegsSameDir(const mg_vec2_t *A, const mg_vec2_t *B,
                               const mg_vec2_t *C, const mg_vec2_t *D);



/***** INLINES *****/
_mg_inline mg_real_t mgVec2Get(const mg_vec2_t *v, int d)
{
    return v->v[d];
}


_mg_inline mg_real_t mgVec2X(const mg_vec2_t *v) { return mgVec2Get(v, 0); }
_mg_inline mg_real_t mgVec2Y(const mg_vec2_t *v) { return mgVec2Get(v, 1); }


_mg_inline void mgVec2SetX(mg_vec2_t *v, mg_real_t val)
{
    v->v[0] = val;
}
_mg_inline void mgVec2SetY(mg_vec2_t *v, mg_real_t val)
{
    v->v[1] = val;
}


_mg_inline void mgVec2Set(mg_vec2_t *v, mg_real_t x, mg_real_t y)
{
    mgVec2SetX(v, x);
    mgVec2SetY(v, y);
}


_mg_inline mg_vec2_t *mgVec2Clone(const mg_vec2_t *v)
{
    return mgVec2New(mgVec2X(v), mgVec2Y(v));
}

_mg_inline void mgVec2Copy(mg_vec2_t *v, const mg_vec2_t *w)
{
    mgVec2Set(v, mgVec2X(w), mgVec2Y(w));
}

_mg_inline int mgVec2Eq(const mg_vec2_t *x, const mg_vec2_t *y)
{
    return mgEq(mgVec2X(x), mgVec2X(y)) && mgEq(mgVec2Y(x), mgVec2Y(y));
}
_mg_inline int mgVec2NEq(const mg_vec2_t *x, const mg_vec2_t *y)
{
    return !mgVec2Eq(x, y);
}

_mg_inline int mgVec2Eq2(const mg_vec2_t *v, mg_real_t x, mg_real_t y)
{
    return mgEq(mgVec2X(v), x) && mgEq(mgVec2Y(v), y);
}
_mg_inline int mgVec2NEq2(const mg_vec2_t *v, mg_real_t x, mg_real_t y)
{
    return !mgVec2Eq2(v, x, y);
}


_mg_inline mg_real_t mgVec2Dist2(const mg_vec2_t *v, const mg_vec2_t *w)
{
    return MG_CUBE(mgVec2X(v) - mgVec2X(w))
            + MG_CUBE(mgVec2Y(v) - mgVec2Y(w));
}

_mg_inline mg_real_t mgVec2Len2(const mg_vec2_t *v)
{
    return MG_CUBE(mgVec2X(v)) + MG_CUBE(mgVec2Y(v));
}


_mg_inline void mgVec2Add(mg_vec2_t *v, const mg_vec2_t *w)
{
    v->v[0] += w->v[0];
    v->v[1] += w->v[1];
}

_mg_inline void mgVec2Sub(mg_vec2_t *v, const mg_vec2_t *w)
{
    v->v[0] -= w->v[0];
    v->v[1] -= w->v[1];
}

_mg_inline void mgVec2Sub2(mg_vec2_t *v, const mg_vec2_t *w, const mg_vec2_t *ww)
{
    mgVec2Set(v, mgVec2X(w) - mgVec2X(ww), mgVec2Y(w) - mgVec2Y(ww));
}


_mg_inline void mgVec2Scale(mg_vec2_t *v, mg_real_t k)
{
    v->v[0] *= k;
    v->v[1] *= k;
}


_mg_inline mg_real_t mgVec2Area2(const mg_vec2_t *a,
                              const mg_vec2_t *b,
                              const mg_vec2_t *c)
{
    /* Area2 can be computed as determinant:
     * | a.x a.y 1 |
     * | b.x b.y 1 |
     * | c.x c.y 1 |
     */

    mg_real_t ax, ay, bx, by, cx, cy;

    ax = mgVec2X(a);
    ay = mgVec2Y(a);
    bx = mgVec2X(b);
    by = mgVec2Y(b);
    cx = mgVec2X(c);
    cy = mgVec2Y(c);
    return ax * by - ay * bx +
           ay * cx - ax * cy +
           bx * cy - by * cx;
}


_mg_inline mg_real_t mgVec2SignedAngle(const mg_vec2_t *a,
                                    const mg_vec2_t *b,
                                    const mg_vec2_t *c)
{
    mg_real_t ang = mgVec2Angle(a, b, c);

    /* angle holded in b is convex */
    if (mgVec2Area2(a, b, c) < 0)
        return ang;

    /* angle is concave */
    return -1 * ang;
}



_mg_inline int mgVec2IntersectProp(const mg_vec2_t *a,
                                  const mg_vec2_t *b,
                                  const mg_vec2_t *c,
                                  const mg_vec2_t *d)
{
    // area2(a, b, c) * area2(a, b, d) is less than zero only if
    // only one computed area is less than zero, so it means that it is the
    // same as XOR(pointArea2(a, b, c), pointArea2(a, b, d)) 
    if (mgVec2Area2(a, b, c) * mgVec2Area2(a, b, d) < 0
        && mgVec2Area2(c, d, a) * mgVec2Area2(c, d, b) < 0)
        return 1;
    return 0;
}



_mg_inline int mgVec2Intersect(const mg_vec2_t *a, const mg_vec2_t *b,
                              const mg_vec2_t *c, const mg_vec2_t *d)
{
    if (mgVec2IntersectProp(a, b, c, d))
        return 1;

    if (mgVec2LiesOn(a, c, d) || mgVec2LiesOn(b, c, d)
        || mgVec2LiesOn(c, a, b) || mgVec2LiesOn(d, a, b))
        return 1;
    return 0;
}


_mg_inline int mgVec2OnLeft(const mg_vec2_t *v,
                           const mg_vec2_t *p1, const mg_vec2_t *p2)
{
    return mgVec2Area2(p1, p2, v) > MG_ZERO;
}


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __MG_VEC_H__ */
