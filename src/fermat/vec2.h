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

#ifndef __FER_VEC_H__
#define __FER_VEC_H__

/**
 * Implementation of vectors.
 * Functions defined should be optimized as possible, so if you see some
 * drawback in implementation feel free to fix it!
 */

#include <fermat/core.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Structure representing 2D vector.
 */
struct _fer_vec2_t {
    fer_real_t f[2];
};
typedef struct _fer_vec2_t fer_vec2_t;

/**
 * Holds origin (0,0) - this variable is meant to be read-only!
 */
extern const fer_vec2_t *fer_vec2_origin;

#define FER_VEC2_STATIC(x, y) \
    { { (x), (y) } }

#define FER_VEC2(name, x, y) \
    fer_vec2_t name = FER_VEC2_STATIC((x), (y))


/**
 * Allocate and initialize new vector.
 */
fer_vec2_t *ferVec2New(fer_real_t x, fer_real_t y);

/**
 * Delete vector.
 */
void ferVec2Del(fer_vec2_t *);

/**
 * Clone given fer_vec2_t. This does deep copy.
 */
_fer_inline fer_vec2_t *ferVec2Clone(const fer_vec2_t *v);

/**
 * Copies w into v.
 */
_fer_inline void ferVec2Copy(fer_vec2_t *v, const fer_vec2_t *w);


_fer_inline fer_real_t ferVec2X(const fer_vec2_t *v);
_fer_inline fer_real_t ferVec2Y(const fer_vec2_t *v);
_fer_inline fer_real_t ferVec2Get(const fer_vec2_t *v, int d);


_fer_inline void ferVec2SetX(fer_vec2_t *v, fer_real_t val);
_fer_inline void ferVec2SetY(fer_vec2_t *v, fer_real_t val);
_fer_inline void ferVec2Set(fer_vec2_t *v, fer_real_t x, fer_real_t y);

_fer_inline int ferVec2Eq(const fer_vec2_t *x, const fer_vec2_t *y);
_fer_inline int ferVec2NEq(const fer_vec2_t *x, const fer_vec2_t *y);

_fer_inline int ferVec2Eq2(const fer_vec2_t *v, fer_real_t x, fer_real_t y);
_fer_inline int ferVec2NEq2(const fer_vec2_t *v, fer_real_t x, fer_real_t y);


/**
 * Compute squared distance between two points represented as vectors.
 */
_fer_inline fer_real_t ferVec2Dist2(const fer_vec2_t *v, const fer_vec2_t *w);

/**
 * Distance of two vectors.
 */
_fer_inline fer_real_t ferVec2Dist(const fer_vec2_t *v, const fer_vec2_t *w);

/**
 * Squared length of vector.
 */
_fer_inline fer_real_t ferVec2Len2(const fer_vec2_t *v);

/**
 * Length of vector.
 */
_fer_inline fer_real_t ferVec2Len(const fer_vec2_t *v);


/**
 * Adds vector W to vector V (and result is stored in V):
 * v = v + w
 */
_fer_inline void ferVec2Add(fer_vec2_t *v, const fer_vec2_t *w);

/**
 * d = v + w
 */
_fer_inline void ferVec2Add2(fer_vec2_t *d, const fer_vec2_t *v, const fer_vec2_t *w);

/**
 * Substracts coordinates of vector W from vector V:
 * v = v - w
 */
_fer_inline void ferVec2Sub(fer_vec2_t *v, const fer_vec2_t *w);

/**
 *  v = w - ww
 */
_fer_inline void ferVec2Sub2(fer_vec2_t *v, const fer_vec2_t *w, const fer_vec2_t *ww);

/**
 * Scales vector to given length.
 */
_fer_inline void ferVec2ScaleToLen(fer_vec2_t *v, fer_real_t len);

/**
 * Normalizes vector to unit vector.
 */
_fer_inline void ferVec2Normalize(fer_vec2_t *v);

/**
 * Dot product of two vectors.
 */
_fer_inline fer_real_t ferVec2Dot(const fer_vec2_t *v, const fer_vec2_t *w);

/**
 * Multiplies vectors by components:
 * a.x = a.x * b.x
 * a.y = a.y * b.y
 */
_fer_inline void ferVec2MulComp(fer_vec2_t *a, const fer_vec2_t *b);

/**
 * Multiplies vectors by components:
 * d.x = a.x * b.x
 * d.y = a.y * b.y
 */
_fer_inline void ferVec2MulComp2(fer_vec2_t *d, const fer_vec2_t *a, const fer_vec2_t *b);



/**
 * Scales vector V using constant k:
 *  v = k * v
 */
_fer_inline void ferVec2Scale(fer_vec2_t *v, fer_real_t k);


/**
 * Returns twice area enclosed by given vectors.
 * a, b, c should be named in counterclockwise order to get positive
 * area and clockwise to get negative.
 */
_fer_inline fer_real_t ferVec2Area2(const fer_vec2_t *a,
                                    const fer_vec2_t *b,
                                    const fer_vec2_t *c);


/**
 * Returns angle in b formed by vectors a, b, c.
 * Returned value is between 0 and PI
 */
fer_real_t ferVec2Angle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c);

/**
 * Returns angle formed by points a, b, c in this order, it means, that
 * is returned angle in point b and in direction from segment ab to cb.
 * Returned angle is from -PI to PI. Positive angle is in
 * counterclockwise direction.
 */
_fer_inline fer_real_t ferVec2SignedAngle(const fer_vec2_t *a,
                                          const fer_vec2_t *b,
                                          const fer_vec2_t *c);


/**
 * This function computes projection of point C onto segment AB. Point of
 * projection is returned in X.
 *
 * Returns 0 if there exists any projection, otherwise -1.
 */
int ferVec2ProjectionPointOntoSegment(const fer_vec2_t *A, const fer_vec2_t *B,
                                      const fer_vec2_t *C,
                                      fer_vec2_t *X);


/**
 * Returns true if point d is in circle formed by points a, b, c.
 * Vectors a, b, c must be in counterclockwise order.
 */
int ferVec2InCircle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c,
                    const fer_vec2_t *d);

/**
 * Returns true if point a lies on segment formed by b c.
 */
int ferVec2LiesOn(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c);


/**
 * Returns true if a is collinear with b and c.
 */
#define ferVec2Collinear(a, b, c) \
    ferEq(ferVec2Area2((b), (c), (a)), FER_ZERO)

/**
 * Returns true, if vector v is in cone formed by p1, c, p2 (in
 * counterclockwise order).
 */
int ferVec2InCone(const fer_vec2_t *v,
                  const fer_vec2_t *p1, const fer_vec2_t *c, const fer_vec2_t *p2);


/**
 * Returns true if segment ab properly intersects segement cd (they share
 * point interior to both segments).
 * Properness of intersection means that two segmensts intersect at a point
 * interior to both segments. Improper intersection (which is not covered
 * by this function) means that one of end point lies somewhere on other
 * segment.
 */
_fer_inline int ferVec2IntersectProp(const fer_vec2_t *a,
                                     const fer_vec2_t *b,
                                     const fer_vec2_t *c,
                                     const fer_vec2_t *d);



/**
 * Returns true if segment ab intersects segment cd properly or improperly.
 */
_fer_inline int ferVec2Intersect(const fer_vec2_t *a, const fer_vec2_t *b,
                                 const fer_vec2_t *c, const fer_vec2_t *d);


/**
 * Compute intersection point of two segments - (a, b) and (c, d).
 * Returns 0 if there exists intersection, -1 otherwise.
 * Intersetion point is returned in p, where p must point to already
 * allocated fer_vec2_t.
 */
int ferVec2IntersectPoint(const fer_vec2_t *a, const fer_vec2_t *b,
                          const fer_vec2_t *c, const fer_vec2_t *d,
                          fer_vec2_t *p);

/**
 * Returns true iff vector v is on left side from segment formed by p1 and
 * p2 in this ordering.
 */
_fer_inline int ferVec2OnLeft(const fer_vec2_t *v,
                              const fer_vec2_t *p1, const fer_vec2_t *p2);


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
int ferVec2SegmentInRect(const fer_vec2_t *a, const fer_vec2_t *b,
                         const fer_vec2_t *c, const fer_vec2_t *d,
                         const fer_vec2_t *x, const fer_vec2_t *y,
                         fer_vec2_t *s1, fer_vec2_t *s2);


/**
 * Returns angle by which must be vector b rotated about origin to have
 * same direction as vector a.
 * Returned angle is in range -PI, PI.
 */
fer_real_t ferVec2AngleSameDir(const fer_vec2_t *a, const fer_vec2_t *b);



/**
 * Retuns angle by which must be rotated oriented segment CD to have same
 * direction as oriented segment AB. Directional vector of segment CD is
 * vector (D - C) and directional vector of segment AB is vector (B - A).
 * Returned angle will be between -PI and PI.
 */
fer_real_t ferVec2AngleSegsSameDir(const fer_vec2_t *A, const fer_vec2_t *B,
                                   const fer_vec2_t *C, const fer_vec2_t *D);



/***** INLINES *****/
_fer_inline fer_real_t ferVec2Get(const fer_vec2_t *v, int d)
{
    return v->f[d];
}


_fer_inline fer_real_t ferVec2X(const fer_vec2_t *v) { return ferVec2Get(v, 0); }
_fer_inline fer_real_t ferVec2Y(const fer_vec2_t *v) { return ferVec2Get(v, 1); }


_fer_inline void ferVec2SetX(fer_vec2_t *v, fer_real_t val)
{
    v->f[0] = val;
}
_fer_inline void ferVec2SetY(fer_vec2_t *v, fer_real_t val)
{
    v->f[1] = val;
}


_fer_inline void ferVec2Set(fer_vec2_t *v, fer_real_t x, fer_real_t y)
{
    ferVec2SetX(v, x);
    ferVec2SetY(v, y);
}


_fer_inline fer_vec2_t *ferVec2Clone(const fer_vec2_t *v)
{
    return ferVec2New(ferVec2X(v), ferVec2Y(v));
}

_fer_inline void ferVec2Copy(fer_vec2_t *v, const fer_vec2_t *w)
{
    ferVec2Set(v, ferVec2X(w), ferVec2Y(w));
}

_fer_inline int ferVec2Eq(const fer_vec2_t *x, const fer_vec2_t *y)
{
    return ferEq(ferVec2X(x), ferVec2X(y)) && ferEq(ferVec2Y(x), ferVec2Y(y));
}
_fer_inline int ferVec2NEq(const fer_vec2_t *x, const fer_vec2_t *y)
{
    return !ferVec2Eq(x, y);
}

_fer_inline int ferVec2Eq2(const fer_vec2_t *v, fer_real_t x, fer_real_t y)
{
    return ferEq(ferVec2X(v), x) && ferEq(ferVec2Y(v), y);
}
_fer_inline int ferVec2NEq2(const fer_vec2_t *v, fer_real_t x, fer_real_t y)
{
    return !ferVec2Eq2(v, x, y);
}


_fer_inline fer_real_t ferVec2Dist2(const fer_vec2_t *v, const fer_vec2_t *w)
{
    return FER_CUBE(ferVec2X(v) - ferVec2X(w))
            + FER_CUBE(ferVec2Y(v) - ferVec2Y(w));
}

_fer_inline fer_real_t ferVec2Dist(const fer_vec2_t *v, const fer_vec2_t *w)
{
    return FER_SQRT(ferVec2Dist2(v, w));
}

_fer_inline fer_real_t ferVec2Len2(const fer_vec2_t *v)
{
    return FER_CUBE(ferVec2X(v)) + FER_CUBE(ferVec2Y(v));
}

_fer_inline fer_real_t ferVec2Len(const fer_vec2_t *v)
{
    return FER_SQRT(ferVec2Len2(v));
}


_fer_inline void ferVec2Add(fer_vec2_t *v, const fer_vec2_t *w)
{
    v->f[0] += w->f[0];
    v->f[1] += w->f[1];
}

_fer_inline void ferVec2Add2(fer_vec2_t *d, const fer_vec2_t *v, const fer_vec2_t *w)
{
    d->f[0] = v->f[0] + w->f[0];
    d->f[0] = v->f[1] + w->f[1];
}

_fer_inline void ferVec2Sub(fer_vec2_t *v, const fer_vec2_t *w)
{
    v->f[0] -= w->f[0];
    v->f[1] -= w->f[1];
}

_fer_inline void ferVec2Sub2(fer_vec2_t *v, const fer_vec2_t *w, const fer_vec2_t *ww)
{
    ferVec2Set(v, ferVec2X(w) - ferVec2X(ww), ferVec2Y(w) - ferVec2Y(ww));
}

_fer_inline void ferVec2Scale(fer_vec2_t *v, fer_real_t k)
{
    v->f[0] *= k;
    v->f[1] *= k;
}

_fer_inline void ferVec2ScaleToLen(fer_vec2_t *v, fer_real_t len)
{
    fer_real_t k = len * ferRsqrt(ferVec2Len2(v));
    ferVec2Scale(v, k);
}

_fer_inline void ferVec2Normalize(fer_vec2_t *v)
{
    fer_real_t k = ferRsqrt(ferVec2Len2(v));
    ferVec2Scale(v, k);
}

_fer_inline fer_real_t ferVec2Dot(const fer_vec2_t *v, const fer_vec2_t *w)
{
    fer_real_t dot;
    dot  = v->f[0] * w->f[0];
    dot += v->f[1] * w->f[1];
    return dot;
}

_fer_inline void ferVec2MulComp(fer_vec2_t *a, const fer_vec2_t *b)
{
    a->f[0] *= b->f[0];
    a->f[1] *= b->f[1];
}

_fer_inline void ferVec2MulComp2(fer_vec2_t *d, const fer_vec2_t *a, const fer_vec2_t *b)
{
    d->f[0] = a->f[0] * b->f[0];
    d->f[1] = a->f[1] * b->f[1];
}



_fer_inline fer_real_t ferVec2Area2(const fer_vec2_t *a,
                              const fer_vec2_t *b,
                              const fer_vec2_t *c)
{
    /* Area2 can be computed as determinant:
     * | a.x a.y 1 |
     * | b.x b.y 1 |
     * | c.x c.y 1 |
     */

    fer_real_t ax, ay, bx, by, cx, cy;

    ax = ferVec2X(a);
    ay = ferVec2Y(a);
    bx = ferVec2X(b);
    by = ferVec2Y(b);
    cx = ferVec2X(c);
    cy = ferVec2Y(c);
    return ax * by - ay * bx +
           ay * cx - ax * cy +
           bx * cy - by * cx;
}


_fer_inline fer_real_t ferVec2SignedAngle(const fer_vec2_t *a,
                                    const fer_vec2_t *b,
                                    const fer_vec2_t *c)
{
    fer_real_t ang = ferVec2Angle(a, b, c);

    /* angle holded in b is convex */
    if (ferVec2Area2(a, b, c) < 0)
        return ang;

    /* angle is concave */
    return -1 * ang;
}



_fer_inline int ferVec2IntersectProp(const fer_vec2_t *a,
                                  const fer_vec2_t *b,
                                  const fer_vec2_t *c,
                                  const fer_vec2_t *d)
{
    // area2(a, b, c) * area2(a, b, d) is less than zero only if
    // only one computed area is less than zero, so it means that it is the
    // same as XOR(pointArea2(a, b, c), pointArea2(a, b, d)) 
    if (ferVec2Area2(a, b, c) * ferVec2Area2(a, b, d) < 0
        && ferVec2Area2(c, d, a) * ferVec2Area2(c, d, b) < 0)
        return 1;
    return 0;
}



_fer_inline int ferVec2Intersect(const fer_vec2_t *a, const fer_vec2_t *b,
                              const fer_vec2_t *c, const fer_vec2_t *d)
{
    if (ferVec2IntersectProp(a, b, c, d))
        return 1;

    if (ferVec2LiesOn(a, c, d) || ferVec2LiesOn(b, c, d)
        || ferVec2LiesOn(c, a, b) || ferVec2LiesOn(d, a, b))
        return 1;
    return 0;
}


_fer_inline int ferVec2OnLeft(const fer_vec2_t *v,
                           const fer_vec2_t *p1, const fer_vec2_t *p2)
{
    return ferVec2Area2(p1, p2, v) > FER_ZERO;
}


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_VEC_H__ */
