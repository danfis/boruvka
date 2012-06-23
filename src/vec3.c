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

#include <stdio.h>
#include <boruvka/alloc.h>
#include <boruvka/vec3.h>
#include <boruvka/vec2.h>
#include <boruvka/dbg.h>

static BOR_VEC3(__bor_vec3_origin, BOR_ZERO, BOR_ZERO, BOR_ZERO);
const bor_vec3_t *bor_vec3_origin = &__bor_vec3_origin;

static BOR_VEC3(__bor_vec3_axis_x, BOR_ONE,  BOR_ZERO, BOR_ZERO);
static BOR_VEC3(__bor_vec3_axis_y, BOR_ZERO, BOR_ONE,  BOR_ZERO);
static BOR_VEC3(__bor_vec3_axis_z, BOR_ZERO, BOR_ZERO, BOR_ONE);

const bor_vec3_t *bor_vec3_axis[3] = {
    &__bor_vec3_axis_x,
    &__bor_vec3_axis_y,
    &__bor_vec3_axis_z
};

static bor_vec3_t points_on_sphere[] = {
    BOR_VEC3_STATIC(BOR_REAL( 0.000000), BOR_REAL(-0.000000), BOR_REAL(-1.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.723608), BOR_REAL(-0.525725), BOR_REAL(-0.447219)),
    BOR_VEC3_STATIC(BOR_REAL(-0.276388), BOR_REAL(-0.850649), BOR_REAL(-0.447219)),
    BOR_VEC3_STATIC(BOR_REAL(-0.894426), BOR_REAL(-0.000000), BOR_REAL(-0.447216)),
    BOR_VEC3_STATIC(BOR_REAL(-0.276388), BOR_REAL( 0.850649), BOR_REAL(-0.447220)),
    BOR_VEC3_STATIC(BOR_REAL( 0.723608), BOR_REAL( 0.525725), BOR_REAL(-0.447219)),
    BOR_VEC3_STATIC(BOR_REAL( 0.276388), BOR_REAL(-0.850649), BOR_REAL( 0.447220)),
    BOR_VEC3_STATIC(BOR_REAL(-0.723608), BOR_REAL(-0.525725), BOR_REAL( 0.447219)),
    BOR_VEC3_STATIC(BOR_REAL(-0.723608), BOR_REAL( 0.525725), BOR_REAL( 0.447219)),
    BOR_VEC3_STATIC(BOR_REAL( 0.276388), BOR_REAL( 0.850649), BOR_REAL( 0.447219)),
    BOR_VEC3_STATIC(BOR_REAL( 0.894426), BOR_REAL( 0.000000), BOR_REAL( 0.447216)),
    BOR_VEC3_STATIC(BOR_REAL(-0.000000), BOR_REAL( 0.000000), BOR_REAL( 1.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.425323), BOR_REAL(-0.309011), BOR_REAL(-0.850654)),
    BOR_VEC3_STATIC(BOR_REAL(-0.162456), BOR_REAL(-0.499995), BOR_REAL(-0.850654)),
    BOR_VEC3_STATIC(BOR_REAL( 0.262869), BOR_REAL(-0.809012), BOR_REAL(-0.525738)),
    BOR_VEC3_STATIC(BOR_REAL( 0.425323), BOR_REAL( 0.309011), BOR_REAL(-0.850654)),
    BOR_VEC3_STATIC(BOR_REAL( 0.850648), BOR_REAL(-0.000000), BOR_REAL(-0.525736)),
    BOR_VEC3_STATIC(BOR_REAL(-0.525730), BOR_REAL(-0.000000), BOR_REAL(-0.850652)),
    BOR_VEC3_STATIC(BOR_REAL(-0.688190), BOR_REAL(-0.499997), BOR_REAL(-0.525736)),
    BOR_VEC3_STATIC(BOR_REAL(-0.162456), BOR_REAL( 0.499995), BOR_REAL(-0.850654)),
    BOR_VEC3_STATIC(BOR_REAL(-0.688190), BOR_REAL( 0.499997), BOR_REAL(-0.525736)),
    BOR_VEC3_STATIC(BOR_REAL( 0.262869), BOR_REAL( 0.809012), BOR_REAL(-0.525738)),
    BOR_VEC3_STATIC(BOR_REAL( 0.951058), BOR_REAL( 0.309013), BOR_REAL( 0.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.951058), BOR_REAL(-0.309013), BOR_REAL( 0.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.587786), BOR_REAL(-0.809017), BOR_REAL( 0.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.000000), BOR_REAL(-1.000000), BOR_REAL( 0.000000)),
    BOR_VEC3_STATIC(BOR_REAL(-0.587786), BOR_REAL(-0.809017), BOR_REAL( 0.000000)),
    BOR_VEC3_STATIC(BOR_REAL(-0.951058), BOR_REAL(-0.309013), BOR_REAL(-0.000000)),
    BOR_VEC3_STATIC(BOR_REAL(-0.951058), BOR_REAL( 0.309013), BOR_REAL(-0.000000)),
    BOR_VEC3_STATIC(BOR_REAL(-0.587786), BOR_REAL( 0.809017), BOR_REAL(-0.000000)),
    BOR_VEC3_STATIC(BOR_REAL(-0.000000), BOR_REAL( 1.000000), BOR_REAL(-0.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.587786), BOR_REAL( 0.809017), BOR_REAL(-0.000000)),
    BOR_VEC3_STATIC(BOR_REAL( 0.688190), BOR_REAL(-0.499997), BOR_REAL( 0.525736)),
    BOR_VEC3_STATIC(BOR_REAL(-0.262869), BOR_REAL(-0.809012), BOR_REAL( 0.525738)),
    BOR_VEC3_STATIC(BOR_REAL(-0.850648), BOR_REAL( 0.000000), BOR_REAL( 0.525736)),
    BOR_VEC3_STATIC(BOR_REAL(-0.262869), BOR_REAL( 0.809012), BOR_REAL( 0.525738)),
    BOR_VEC3_STATIC(BOR_REAL( 0.688190), BOR_REAL( 0.499997), BOR_REAL( 0.525736)),
    BOR_VEC3_STATIC(BOR_REAL( 0.525730), BOR_REAL( 0.000000), BOR_REAL( 0.850652)),
    BOR_VEC3_STATIC(BOR_REAL( 0.162456), BOR_REAL(-0.499995), BOR_REAL( 0.850654)),
    BOR_VEC3_STATIC(BOR_REAL(-0.425323), BOR_REAL(-0.309011), BOR_REAL( 0.850654)),
    BOR_VEC3_STATIC(BOR_REAL(-0.425323), BOR_REAL( 0.309011), BOR_REAL( 0.850654)),
    BOR_VEC3_STATIC(BOR_REAL( 0.162456), BOR_REAL( 0.499995), BOR_REAL( 0.850654))
};
bor_vec3_t *bor_points_on_sphere = points_on_sphere;
size_t bor_points_on_sphere_len = sizeof(points_on_sphere) / sizeof(bor_vec3_t);

_bor_inline bor_real_t _borVec3ACos(bor_real_t angle)
{
    if (borEq(angle, BOR_ONE) || angle > BOR_ONE)
        angle = BOR_ONE;
    if (borEq(angle, -BOR_ONE) || angle < -BOR_ONE)
        angle = -BOR_ONE;

    /*
    if (isnan(angle) || isnan(BOR_ACOS(angle))){
        DBG("NAN: %f", angle);
    }
    */

    return BOR_ACOS(angle);
}


bor_vec3_t *borVec3New(bor_real_t x, bor_real_t y, bor_real_t z)
{
    bor_vec3_t *v;

#ifdef BOR_SSE
    v = BOR_ALLOC_ALIGN(bor_vec3_t, 16);
#else /* BOR_SSE */
    v = BOR_ALLOC(bor_vec3_t);
#endif /* BOR_SSE */
    borVec3Set(v, x, y, z);
    return v;
}

void borVec3Del(bor_vec3_t *v)
{
    BOR_FREE(v);
}

bor_vec3_t *borVec3ArrNew(size_t num_vecs)
{
    bor_vec3_t *vs;

#ifdef BOR_SSE
    vs = BOR_ALLOC_ALIGN_ARR(bor_vec3_t, num_vecs, sizeof(bor_vec3_t));
#else /* BOR_SSE */
    vs = BOR_ALLOC_ARR(bor_vec3_t, num_vecs);
#endif /* BOR_SSE */

    return vs;
}

void borVec3ArrDel(bor_vec3_t *v)
{
    BOR_FREE(v);
}

_bor_inline bor_real_t __borVec3PointSegmentDist2(const bor_vec3_t *P,
                                               const bor_vec3_t *x0,
                                               const bor_vec3_t *b,
                                               bor_vec3_t *witness)
{
    // The computation comes from solving equation of segment:
    //      S(t) = x0 + t.d
    //          where - x0 is initial point of segment
    //                - d is direction of segment from x0 (|d| > 0)
    //                - t belongs to <0, 1> interval
    //
    // Than, distance from a segment to some point P can be expressed:
    //      D(t) = |x0 + t.d - P|^2
    //          which is distance from any point on segment. Minimization
    //          of this function brings distance from P to segment.
    // Minimization of D(t) leads to simple quadratic equation that's
    // solving is straightforward.
    //
    // Bonus of this method is witness point for free.

    bor_real_t dist, t;
    bor_vec3_t d, a;

    // direction of segment
    borVec3Sub2(&d, b, x0);

    // precompute vector from P to x0
    borVec3Sub2(&a, x0, P);

    t  = -BOR_REAL(1.) * borVec3Dot(&a, &d);
    t /= borVec3Len2(&d);

    if (t < BOR_ZERO || borIsZero(t)){
        dist = borVec3Dist2(x0, P);
        if (witness)
            borVec3Copy(witness, x0);
    }else if (t > BOR_ONE || borEq(t, BOR_ONE)){
        dist = borVec3Dist2(b, P);
        if (witness)
            borVec3Copy(witness, b);
    }else{
        if (witness){
            borVec3Copy(witness, &d);
            borVec3Scale(witness, t);
            borVec3Add(witness, x0);
            dist = borVec3Dist2(witness, P);
        }else{
            // recycling variables
            borVec3Scale(&d, t);
            borVec3Add(&d, &a);
            dist = borVec3Len2(&d);
        }
    }

    return dist;
}

bor_real_t borVec3PointSegmentDist2(const bor_vec3_t *P,
                                    const bor_vec3_t *x0, const bor_vec3_t *b,
                                    bor_vec3_t *witness)
{
    return __borVec3PointSegmentDist2(P, x0, b, witness);
}


_bor_inline bor_real_t __clamp(bor_real_t n, bor_real_t min, bor_real_t max)
{
    if (n < min)
        return min;
    if (n > max)
        return max;
    return n;
}

bor_real_t borVec3SegmentSegmentDist2(const bor_vec3_t *A, const bor_vec3_t *B,
                                      const bor_vec3_t *C, const bor_vec3_t *D,
                                      bor_vec3_t *witness1,
                                      bor_vec3_t *witness2,
                                      int *parallel)
{
    /** Taken from "orange" book, 5.1.9 */

    bor_vec3_t d1, d2, r;
    bor_real_t a, b, c, denom, e, f;
    bor_real_t s, t;

    // direction vector of segment AB and segment CD
    borVec3Sub2(&d1, B, A);
    borVec3Sub2(&d2, D, C);
    borVec3Sub2(&r, A, C);

    // square lengths
    a = borVec3Len2(&d1);
    e = borVec3Len2(&d1);
    f = borVec3Dot(&d2, &r);

    if (parallel)
        *parallel = 0;

    // check if either or both segments degenerate into point
    if (borIsZero(a) && borIsZero(e)){
        // both segments degenerate into points
        if (witness1)
            borVec3Copy(witness1, A);
        if (witness2)
            borVec3Copy(witness2, C);

        return borVec3Dist2(A, C);
    }

    if (borIsZero(a)){
        // segment AB degenerates into point
        if (witness1)
            borVec3Copy(witness1, A);

        t = f / e;
        if (witness2){
            borVec3Scale(&d2, t);
            borVec3Add2(witness2, C, &d2);
        }

        return __clamp(t, BOR_ZERO, BOR_ONE);
    }else{
        c = borVec3Dot(&d1, &r);
        if (borIsZero(e)){
            // segment CD degenerates into point
            t = BOR_ZERO;
            s = __clamp(-c / a, BOR_ZERO, BOR_ONE);
        }else{
            // general non-degenerate case
            b = borVec3Dot(&d1, &d2);
            denom = (a * e) - (b * b);

            if (!borIsZero(denom)){
                s = __clamp((b * f) - (c * e) / denom, BOR_ZERO, BOR_ONE);
            }else{
                // segments parallel - choose arbitrary s.
                s = BOR_ZERO;
                if (parallel)
                    *parallel = 1;
            }

            t = b * s + f;
            if (t < BOR_ZERO){
                t = BOR_ZERO;
                s = __clamp(-c / a, BOR_ZERO, BOR_ONE);
            }else if (t > e){
                t = BOR_ONE;
                s = __clamp((b - c) / a, BOR_ZERO, BOR_ONE);
            }else{
                t = t / e;
            }
        }
    }

    borVec3Scale(&d1, s);
    borVec3Add(&d1, A);
    borVec3Scale(&d2, t);
    borVec3Add(&d2, C);

    if (witness1){
        borVec3Copy(witness1, &d1);
    }
    if (witness2){
        borVec3Copy(witness2, &d2);
    }

    return borVec3Dist2(&d1, &d2);
}


bor_real_t borVec3PointTriDist2(const bor_vec3_t *P,
                              const bor_vec3_t *x0, const bor_vec3_t *B,
                              const bor_vec3_t *C,
                              bor_vec3_t *witness)
{
    // Computation comes from analytic expression for triangle (x0, B, C)
    //      T(s, t) = x0 + s.d1 + t.d2, where d1 = B - x0 and d2 = C - x0 and
    // Then equation for distance is:
    //      D(s, t) = | T(s, t) - P |^2
    // This leads to minimization of quadratic function of two variables.
    // The solution from is taken only if s is between 0 and 1, t is
    // between 0 and 1 and t + s < 1, otherwise distance from segment is
    // computed.

    bor_vec3_t d1, d2, a;
    bor_real_t u, v, w, p, q, r;
    bor_real_t s, t, dist, dist2;
    bor_vec3_t witness2;

    borVec3Sub2(&d1, B, x0);
    borVec3Sub2(&d2, C, x0);
    borVec3Sub2(&a, x0, P);

    u = borVec3Dot(&a, &a);
    v = borVec3Dot(&d1, &d1);
    w = borVec3Dot(&d2, &d2);
    p = borVec3Dot(&a, &d1);
    q = borVec3Dot(&a, &d2);
    r = borVec3Dot(&d1, &d2);

    s = (q * r - w * p) / (w * v - r * r);
    t = (-s * r - q) / w;

    if ((borIsZero(s) || s > BOR_ZERO)
            && (borEq(s, BOR_ONE) || s < BOR_ONE)
            && (borIsZero(t) || t > BOR_ZERO)
            && (borEq(t, BOR_ONE) || t < BOR_ONE)
            && (borEq(t + s, BOR_ONE) || t + s < BOR_ONE)){

        if (witness){
            borVec3Scale(&d1, s);
            borVec3Scale(&d2, t);
            borVec3Copy(witness, x0);
            borVec3Add(witness, &d1);
            borVec3Add(witness, &d2);

            dist = borVec3Dist2(witness, P);
        }else{
            dist  = s * s * v;
            dist += t * t * w;
            dist += BOR_REAL(2.) * s * t * r;
            dist += BOR_REAL(2.) * s * p;
            dist += BOR_REAL(2.) * t * q;
            dist += u;
        }
    }else{
        dist = __borVec3PointSegmentDist2(P, x0, B, witness);

        dist2 = __borVec3PointSegmentDist2(P, x0, C, &witness2);
        if (dist2 < dist){
            dist = dist2;
            if (witness)
                borVec3Copy(witness, &witness2);
        }

        dist2 = __borVec3PointSegmentDist2(P, B, C, &witness2);
        if (dist2 < dist){
            dist = dist2;
            if (witness)
                borVec3Copy(witness, &witness2);
        }
    }

    return dist;
}

int borVec3PointInTri(const bor_vec3_t *p,
                     const bor_vec3_t *a, const bor_vec3_t *b,
                     const bor_vec3_t *c)
{
    bor_vec3_t v0, v1, v2;
    bor_real_t dot00, dot01, dot02, dot11, dot12;
    bor_real_t inv_denom, u, v;

    // compute vectors
    borVec3Sub2(&v0, c, a);
    borVec3Sub2(&v1, b, a);
    borVec3Sub2(&v2, p, a);

    // compute dot products
    dot00 = borVec3Dot(&v0, &v0);
    dot01 = borVec3Dot(&v0, &v1);
    dot02 = borVec3Dot(&v0, &v2);
    dot11 = borVec3Dot(&v1, &v1);
    dot12 = borVec3Dot(&v1, &v2);

    // compute barycentric coordinates
    inv_denom = BOR_ONE / (dot00 * dot11 - dot01 * dot01);
    u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
    v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

    // check if point is in triangle
    return (u > BOR_ZERO || borIsZero(u))
            && (v > BOR_ZERO || borIsZero(v))
            && (u + v < BOR_ONE || borEq(u + v, BOR_ONE));
}

bor_real_t borVec3Angle(const bor_vec3_t *a, const bor_vec3_t *b, const bor_vec3_t *c)
{
    bor_real_t angle, div;
    bor_real_t abx, aby, abz, cbx, cby, cbz;

    abx = borVec3X(a) - borVec3X(b);
    aby = borVec3Y(a) - borVec3Y(b);
    abz = borVec3Z(a) - borVec3Z(b);

    cbx = borVec3X(c) - borVec3X(b);
    cby = borVec3Y(c) - borVec3Y(b);
    cbz = borVec3Z(c) - borVec3Z(b);

    div = BOR_SQRT(abx * abx + aby * aby + abz * abz);
    div *= BOR_SQRT(cbx * cbx + cby * cby + cbz * cbz);
    if (borIsZero(div))
        return BOR_ZERO;

    angle = abx * cbx + aby * cby + abz * cbz;
    angle /= div;

    return _borVec3ACos(angle);
}

bor_real_t borVec3DihedralAngle(const bor_vec3_t *a, const bor_vec3_t *b,
                              const bor_vec3_t *c, const bor_vec3_t *d)
{
    bor_vec3_t base, v;
    bor_vec3_t na, nb;
    bor_real_t angle;

    // get normal vec3tors of planes
    borVec3Sub2(&base, c, b);
    borVec3Sub2(&v, a, b);
    borVec3Cross(&na, &v, &base);

    borVec3Sub2(&v, d, b);
    borVec3Cross(&nb, &v, &base);

    // normalize normals
    borVec3Normalize(&na);
    borVec3Normalize(&nb);

    angle = borVec3Dot(&na, &nb);

    return _borVec3ACos(angle);
}

bor_real_t borVec3ProjToPlane(const bor_vec3_t *p,
                            const bor_vec3_t *u, const bor_vec3_t *v,
                            const bor_vec3_t *w, bor_vec3_t *d)
{
    bor_vec3_t uv, wv, normal;

    // uv = u - v, wv = w - v
    borVec3Sub2(&uv, u, v);
    borVec3Sub2(&wv, w, v);

    // scale uv and wv to get better normal
    if (borVec3Len2(&uv) < BOR_ONE)
        borVec3Normalize(&uv);
    if (borVec3Len2(&wv) < BOR_ONE)
        borVec3Normalize(&wv);

    // compute normal vec3tor
    borVec3Cross(&normal, &uv, &wv);

    if (borVec3IsZero(&normal))
        return BOR_REAL(-1.);

    return borVec3ProjToPlane2(p, v, &normal, d);
}

bor_real_t borVec3ProjToPlane2(const bor_vec3_t *p,
                             const bor_vec3_t *x, const bor_vec3_t *normal,
                             bor_vec3_t *d)
{
    bor_real_t k;
    bor_vec3_t xp;

    borVec3Sub2(&xp, x, p);
    k  = borVec3Dot(&xp, normal);
    k /= borVec3Len2(normal);

    borVec3Copy(d, normal);
    borVec3Scale(d, k);
    borVec3Add(d, p);

    k = borVec3Dist(p, d);

    return k;
}

bor_real_t borVec3TriArea2(const bor_vec3_t *a, const bor_vec3_t *b,
                         const bor_vec3_t *c)
{
    bor_vec3_t ba, bc, babc;
    bor_real_t area;

    borVec3Sub2(&ba, a, b);
    borVec3Sub2(&bc, c, b);
    borVec3Cross(&babc, &ba, &bc);

    area = borVec3Len(&babc);
    return area;
}





#define __overlapMinMax(p1, q1, r1, p2, q2, r2) \
    borVec3Sub2(&v1, p2, q1); \
    borVec3Sub2(&v2, p1, q1); \
    borVec3Cross(&n1, &v1, &v2); \
    \
    borVec3Sub2(&v1, q2, q1); \
    if (borVec3Dot(&v1, &n1) > BOR_ZERO) \
        return 0; \
    \
    borVec3Sub2(&v1, p2, p1); \
    borVec3Sub2(&v2, r1, p1); \
    borVec3Cross(&n1, &v1, &v2); \
    \
    borVec3Sub2(&v1, r2, p1); \
    if (borVec3Dot(&v1, &n1) > BOR_ZERO) \
        return 0; \
    \
    return 1;

#define __overlapCheck(p1, q1, r1, p2, q2, r2, sp2, sq2, sr2) \
    if (borIsZero(sp2)){ \
        if (borIsZero(sq2)){ \
            if (borIsZero(sr2)){ \
                return __overlapCoplanar(p1, q1, r1, p2, q2, r2, &n1, &n2); \
            }else if (sr2 > BOR_ZERO){ \
                __overlapMinMax(p1, q1, r1, r2, p2, q2) \
            }else{ /* sr2 < BOR_ZERO */ \
                __overlapMinMax(p1, r1, q1, r2, p2, q2) \
            } \
        }else if (sq2 < BOR_ZERO){ \
            if (borIsZero(sr2) || sr2 > BOR_ZERO){ \
                __overlapMinMax(p1, r1, q1, q2, r2, p2) \
            }else{ \
                __overlapMinMax(p1, q1, r1, p2, q2, r2) \
            } \
        }else{ /* sq2 > BOR_ZERO */ \
            if (sr2 > BOR_ZERO){ \
                __overlapMinMax(p1, r1, q1, p2, q2, r2) \
            }else{ \
                __overlapMinMax(p1, q1, r1, q2, r2, p2) \
            } \
        } \
    }else if (sp2 > BOR_ZERO){ \
        if (sq2 > BOR_ZERO){ \
            __overlapMinMax(p1, r1, q1, r2, p2, q2) \
        }else if (sr2 > BOR_ZERO){ \
            __overlapMinMax(p1, r1, q1, q2, r2, p2) \
        }else{ \
            __overlapMinMax(p1, q1, r1, p2, q2, r2) \
        } \
    }else{ /* sp2 < BOR_ZERO */ \
        if (sq2 < BOR_ZERO){ \
            __overlapMinMax(p1, q1, r1, r2, p2, q2) \
        }else if (sr2 < BOR_ZERO){ \
            __overlapMinMax(p1, q1, r1, q2, r2, p2) \
        }else{ \
            __overlapMinMax(p1, r1, q1, p2, q2, r2) \
        } \
    }


_bor_inline int __overlapCoplanar(const bor_vec3_t *_p1,
                                  const bor_vec3_t *_q1,
                                  const bor_vec3_t *_r1,
                                  const bor_vec3_t *_p2,
                                  const bor_vec3_t *_q2,
                                  const bor_vec3_t *_r2,
                                  const bor_vec3_t *n1,
                                  const bor_vec3_t *n2)
{
    bor_vec2_t p1, q1, r1;
    bor_vec2_t p2, q2, r2;
    bor_real_t n_x, n_y, n_z;

    n_x = BOR_FABS(borVec3X(n1));
    n_y = BOR_FABS(borVec3Y(n1));
    n_z = BOR_FABS(borVec3Z(n1));

    // Projection of the triangles in 3D onto 2D such that the area of
    // the projection is maximized.

    if (n_x > n_z && n_x >= n_y){
        // Project onto plane YZ
        borVec2Set(&p1, borVec3Z(_q1), borVec3Y(_q1));
        borVec2Set(&q1, borVec3Z(_p1), borVec3Y(_p1));
        borVec2Set(&r1, borVec3Z(_r1), borVec3Y(_r1));

        borVec2Set(&p2, borVec3Z(_q2), borVec3Y(_q2));
        borVec2Set(&q2, borVec3Z(_p2), borVec3Y(_p2));
        borVec2Set(&r2, borVec3Z(_r2), borVec3Y(_r2));
    }else if (n_y > n_z && n_y >= n_x){
        // Project onto plane XZ
        borVec2Set(&p1, borVec3X(_q1), borVec3Z(_q1));
        borVec2Set(&q1, borVec3X(_p1), borVec3Z(_p1));
        borVec2Set(&r1, borVec3X(_r1), borVec3Z(_r1));

        borVec2Set(&p2, borVec3X(_q2), borVec3Z(_q2));
        borVec2Set(&q2, borVec3X(_p2), borVec3Z(_p2));
        borVec2Set(&r2, borVec3X(_r2), borVec3Z(_r2));
    }else {
        // Project onto plane XY
        borVec2Set(&p1, borVec3X(_q1), borVec3Y(_q1));
        borVec2Set(&q1, borVec3X(_p1), borVec3Y(_p1));
        borVec2Set(&r1, borVec3X(_r1), borVec3Y(_r1));

        borVec2Set(&p2, borVec3X(_q2), borVec3Y(_q2));
        borVec2Set(&q2, borVec3X(_p2), borVec3Y(_p2));
        borVec2Set(&r2, borVec3X(_r2), borVec3Y(_r2));
    }

    return borVec2TriTriOverlap(&p1, &q1, &r1,
                                &p2, &q2, &r2);
}

int borVec3TriTriOverlap(const bor_vec3_t *p1, const bor_vec3_t *q1,
                         const bor_vec3_t *r1,
                         const bor_vec3_t *p2, const bor_vec3_t *q2,
                         const bor_vec3_t *r2)
{
    bor_vec3_t n1, n2, v1, v2;
    bor_real_t sp1, sq1, sr1;
    bor_real_t sp2, sq2, sr2;

    // compute signs of first triangle to second triangle
    borVec3Sub2(&v1, p2, r2);
    borVec3Sub2(&v2, q2, r2);
    borVec3Cross(&n2, &v1, &v2);
    borVec3Normalize(&n2);

    borVec3Sub2(&v1, p1, r2);
    sp1 = borVec3Dot(&v1, &n2);
    borVec3Sub2(&v1, q1, r2);
    sq1 = borVec3Dot(&v1, &n2);
    borVec3Sub2(&v1, r1, r2);
    sr1 = borVec3Dot(&v1, &n2);

    // check if all points from tri1 aren't on same side
    if (sp1 * sq1 > BOR_ZERO
            && sp1 * sr1 > BOR_ZERO)
        return 0;


    // compute signs for second triangle
    borVec3Sub2(&v1, q1, p1);
    borVec3Sub2(&v2, r1, p1);
    borVec3Cross(&n1, &v1, &v2);
    borVec3Normalize(&n1);

    borVec3Sub2(&v1, p2, r1);
    sp2 = borVec3Dot(&v1, &n1);
    borVec3Sub2(&v1, q2, r1);
    sq2 = borVec3Dot(&v1, &n1);
    borVec3Sub2(&v1, r2, r1);
    sr2 = borVec3Dot(&v1, &n1);

    // check if all tri2s' points aren't on same side
    if (sp2 * sq2 > BOR_ZERO
            && sp2 * sr2 > BOR_ZERO)
        return 0;


    if (borIsZero(sp1)){
        if (borIsZero(sq1)){
            if (borIsZero(sr1)){
                return __overlapCoplanar(p1, q1, r1, p2, q2, r2, &n1, &n2);
            }else if (sr1 > BOR_ZERO){
                __overlapCheck(r1,p1,q1,p2,q2,r2,sp2,sq2,sr2)
            }else if (sr1 < BOR_ZERO){
                __overlapCheck(r1,p1,q1,p2,r2,q2,sp2,sr2,sq2)
            }
        }else if (sq1 < BOR_ZERO){
            if (borIsZero(sr1) || sr1 > BOR_ZERO){
                __overlapCheck(q1,r1,p1,p2,r2,q2,sp2,sr2,sq2)
            }else{
                __overlapCheck(p1,q1,r1,p2,q2,r2,sp2,sq2,sr2)
            }
        }else{ // sq1 > BOR_ZERO
            if (sr1 > BOR_ZERO){
                __overlapCheck(p1,q1,r1,p2,r2,q2,sp2,sr2,sq2)
            }else{
                __overlapCheck(q1,r1,p1,p2,q2,r2,sp2,sq2,sr2)
            }
        }
    }else if (sp1 > BOR_ZERO){
        if (sq1 > BOR_ZERO){
            __overlapCheck(r1, p1, q1, p2, r2, q2, sp2, sr2, sq2)
        }else if (sr1 > BOR_ZERO){
            __overlapCheck(q1,r1,p1,p2,r2,q2,sp2,sr2,sq2)
        }else{
            __overlapCheck(p1,q1,r1,p2,q2,r2,sp2,sq2,sr2)
        }
    }else{ // sp1 < BOR_ZERO
        if (sq1 < BOR_ZERO){
            __overlapCheck(r1,p1,q1,p2,q2,r2,sp2,sq2,sr2)
        }else if (sr1 < BOR_ZERO){
            __overlapCheck(q1,r1,p1,p2,q2,r2,sp2,sq2,sr2)
        }else{
            __overlapCheck(p1,q1,r1,p2,r2,q2,sp2,sr2,sq2)
        }
    }

    return 0;
}


/** borVec3TriTriIntersect() adapted from http://jgt.akpeters.com/papers/Moller97/tritri.html */
/* if USE_EPSILON_TEST is true then we do a check:
         if |dv|<EPSILON then dv=0.0;
   else no check is done (which is less robust)
*/
//#define USE_EPSILON_TEST 1

/* sort so that a<=b */
#define SORT2(a,b,smallest)       \
             if(a>b)       \
             {             \
               float c;    \
               c=a;        \
               a=b;        \
               b=c;        \
               smallest=1; \
             }             \
             else smallest=0;

/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=borVec3Get(U0, i0)-borVec3Get(U1, i0);           \
  By=borVec3Get(U0, i1)-borVec3Get(U1, i1);           \
  Cx=borVec3Get(V0, i0)-borVec3Get(U0, i0);           \
  Cy=borVec3Get(V0, i1)-borVec3Get(U0, i1);           \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return 1;                      \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return 1;                      \
    }                                                 \
  }

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  bor_real_t Ax,Ay,Bx,By,Cx,Cy,e,d,f;          \
  Ax=borVec3Get(V1, i0)-borVec3Get(V0, i0);    \
  Ay=borVec3Get(V1, i1)-borVec3Get(V0, i1);    \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  bor_real_t a,b,c,d0,d1,d2;                \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=borVec3Get(U1, i1)-borVec3Get(U0, i1);                          \
  b=-(borVec3Get(U1, i0)-borVec3Get(U0, i0));                       \
  c=-a*borVec3Get(U0, i0)-b*borVec3Get(U0, i1);                     \
  d0=a*borVec3Get(V0, i0)+b*borVec3Get(V0, i1)+c;                   \
                                            \
  a=borVec3Get(U2, i1)-borVec3Get(U1, i1);                          \
  b=-(borVec3Get(U2, i0)-borVec3Get(U1, i0));                       \
  c=-a*borVec3Get(U1, i0)-b*borVec3Get(U1, i1);                     \
  d1=a*borVec3Get(V0, i0)+b*borVec3Get(V0, i1)+c;                   \
                                            \
  a=borVec3Get(U0, i1)-borVec3Get(U2, i1);                          \
  b=-(borVec3Get(U0, i0)-borVec3Get(U2, i0));                       \
  c=-a*borVec3Get(U2, i0)-b*borVec3Get(U2, i1);                     \
  d2=a*borVec3Get(V0, i0)+b*borVec3Get(V0, i1)+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return 1;                 \
  }                                         \
}

static int coplanar_tri_tri(const bor_vec3_t *N, const bor_vec3_t *V0, const bor_vec3_t *V1, const bor_vec3_t *V2,
                            const bor_vec3_t *U0, const bor_vec3_t *U1, const bor_vec3_t *U2)
{
    bor_real_t A[3];
    short i0,i1;

    /* first project onto an axis-aligned plane, that maximizes the area */
    /* of the triangles, compute indices: i0,i1. */
    A[0]=BOR_FABS(borVec3X(N));
    A[1]=BOR_FABS(borVec3Y(N));
    A[2]=BOR_FABS(borVec3Z(N));
    if(A[0]>A[1])
    {
        if(A[0]>A[2])
        {
            i0=1;      /* A[0] is greatest */
            i1=2;
        }
        else
        {
            i0=0;      /* A[2] is greatest */
            i1=1;
        }
    }
    else   /* A[0]<=A[1] */
    {
        if(A[2]>A[1])
        {
            i0=0;      /* A[2] is greatest */
            i1=1;
        }
        else
        {
            i0=0;      /* A[1] is greatest */
            i1=2;
        }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(V0,U0,U1,U2);
    POINT_IN_TRI(U0,V0,V1,V2);

    return 0;
}

_bor_inline void isect2(const bor_vec3_t *VTX0, const bor_vec3_t *VTX1,
                        const bor_vec3_t *VTX2,
                        bor_real_t VV0,bor_real_t VV1,bor_real_t VV2,
                        bor_real_t D0,bor_real_t D1,bor_real_t D2,bor_real_t *isect0,bor_real_t *isect1,
                        bor_vec3_t *isectpoint0, bor_vec3_t *isectpoint1)
{
    float tmp=D0/(D0-D1);
    bor_vec3_t diff;

    *isect0=VV0+(VV1-VV0)*tmp;
    borVec3Sub2(&diff, VTX1, VTX0);
    borVec3Scale(&diff, tmp);
    borVec3Add2(isectpoint0,&diff,VTX0);
    tmp=D0/(D0-D2);
    *isect1=VV0+(VV2-VV0)*tmp;
    borVec3Sub2(&diff,VTX2,VTX0);
    borVec3Scale(&diff,tmp);
    borVec3Add2(isectpoint1,VTX0,&diff);
}
_bor_inline int compute_intervals_isectline(const bor_vec3_t *VERT0,
                                            const bor_vec3_t *VERT1,
                                            const bor_vec3_t *VERT2,
                                            bor_real_t VV0, bor_real_t VV1,
                                            bor_real_t VV2, bor_real_t D0,
                                            bor_real_t D1, bor_real_t D2,
                                            bor_real_t D0D1, bor_real_t D0D2,
                                            bor_real_t *isect0,
                                            bor_real_t *isect1,
                                            bor_vec3_t *isectpoint0,
                                            bor_vec3_t *isectpoint1)
{
    if(D0D1>0.0f)
    {
        /* here we know that D0D2<=0.0 */
        /* that is D0, D1 are on the same side, D2 on the other or on the plane */
        isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,isect0,isect1,isectpoint0,isectpoint1);
    }
    else if(D0D2>0.0f)
    {
        /* here we know that d0d1<=0.0 */
        isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,isect0,isect1,isectpoint0,isectpoint1);
    }
    else if(D1*D2>0.0f || D0!=0.0f)
    {
        /* here we know that d0d1<=0.0 or that D0!=0.0 */
        isect2(VERT0,VERT1,VERT2,VV0,VV1,VV2,D0,D1,D2,isect0,isect1,isectpoint0,isectpoint1);
    }
    else if(D1!=0.0f)
    {
        isect2(VERT1,VERT0,VERT2,VV1,VV0,VV2,D1,D0,D2,isect0,isect1,isectpoint0,isectpoint1);
    }
    else if(D2!=0.0f)
    {
        isect2(VERT2,VERT0,VERT1,VV2,VV0,VV1,D2,D0,D1,isect0,isect1,isectpoint0,isectpoint1);
    }
    else
    {
        /* triangles are coplanar */
        return 1;
    }
    return 0;
}

int borVec3TriTriIntersect(const bor_vec3_t *V0, const bor_vec3_t *V1,
                           const bor_vec3_t *V2,
                           const bor_vec3_t *U0, const bor_vec3_t *U1,
                           const bor_vec3_t *U2,
                           bor_vec3_t *isectpt1, bor_vec3_t *isectpt2)
{
    bor_vec3_t E1, E2, N1, N2;
    bor_real_t d1, d2;
    bor_real_t du0,du1,du2,dv0,dv1,dv2;
    bor_vec3_t D;
    bor_real_t isect1[2], isect2[2];
    bor_vec3_t isectpointA1,isectpointA2;
    bor_vec3_t isectpointB1,isectpointB2;
    bor_real_t du0du1,du0du2,dv0dv1,dv0dv2;
    short index;
    bor_real_t vp0,vp1,vp2;
    bor_real_t up0,up1,up2;
    bor_real_t b,c,max;
    int smallest1,smallest2;

    /* compute plane equation of triangle(V0,V1,V2) */
    borVec3Sub2(&E1,V1,V0);
    borVec3Sub2(&E2,V2,V0);
    borVec3Cross(&N1,&E1,&E2);
    d1=-borVec3Dot(&N1,V0);
    /* plane equation 1: N1.X+d1=0 */

    /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
    du0=borVec3Dot(&N1,U0)+d1;
    du1=borVec3Dot(&N1,U1)+d1;
    du2=borVec3Dot(&N1,U2)+d1;

    /* coplanarity robustness check */
#ifdef USE_EPSILON_TEST
    if(borIsZero(du0)) du0 = BOR_ZERO;
    if(borIsZero(du1)) du1 = BOR_ZERO;
    if(borIsZero(du2)) du2 = BOR_ZERO;
#endif
    du0du1=du0*du1;
    du0du2=du0*du2;

    if(du0du1>0.0f && du0du2>0.0f) /* same sign on all of them + not equal 0 ? */
        return 0;                    /* no intersection occurs */

    /* compute plane of triangle (U0,U1,U2) */
    borVec3Sub2(&E1,U1,U0);
    borVec3Sub2(&E2,U2,U0);
    borVec3Cross(&N2,&E1,&E2);
    d2=-borVec3Dot(&N2,U0);
    /* plane equation 2: N2.X+d2=0 */

    /* put V0,V1,V2 into plane equation 2 */
    dv0=borVec3Dot(&N2,V0)+d2;
    dv1=borVec3Dot(&N2,V1)+d2;
    dv2=borVec3Dot(&N2,V2)+d2;

#ifdef USE_EPSILON_TEST
    if(borIsZero(dv0)) dv0 = BOR_ZERO;
    if(borIsZero(dv1)) dv1 = BOR_ZERO;
    if(borIsZero(dv2)) dv2 = BOR_ZERO;
#endif

    dv0dv1=dv0*dv1;
    dv0dv2=dv0*dv2;

    if(dv0dv1>0.0f && dv0dv2>0.0f) /* same sign on all of them + not equal 0 ? */
        return 0;                    /* no intersection occurs */

    /* compute direction of intersection line */
    borVec3Cross(&D,&N1,&N2);

    /* compute and index to the largest component of D */
    max=BOR_FABS(borVec3X(&D));
    index=0;
    b=BOR_FABS(borVec3Y(&D));
    c=BOR_FABS(borVec3Z(&D));
    if(b>max) max=b,index=1;
    if(c>max) max=c,index=2;

    /* this is the simplified projection onto L*/
    vp0=borVec3Get(V0, index);
    vp1=borVec3Get(V1, index);
    vp2=borVec3Get(V2, index);

    up0=borVec3Get(U0, index);
    up1=borVec3Get(U1, index);
    up2=borVec3Get(U2, index);

    /* compute interval for triangle 1 */
    if (compute_intervals_isectline(V0,V1,V2,vp0,vp1,vp2,dv0,dv1,dv2,
            dv0dv1,dv0dv2,&isect1[0],&isect1[1],&isectpointA1,&isectpointA2)){
        // triangles are coplanar

        if (coplanar_tri_tri(&N1,V0,V1,V2,U0,U1,U2))
            return 2;
        return 0;
    }


    /* compute interval for triangle 2 */
    if (compute_intervals_isectline(U0,U1,U2,up0,up1,up2,du0,du1,du2,
            du0du1,du0du2,&isect2[0],&isect2[1],&isectpointB1,&isectpointB2)){
        fprintf(stderr, "Error (vec3.c): this shouldn't report coplanar triangles!\n");
        return 2;
    }

    SORT2(isect1[0],isect1[1],smallest1);
    SORT2(isect2[0],isect2[1],smallest2);

    if(isect1[1]<isect2[0] || isect2[1]<isect1[0]) return 0;

    /* at this point, we know that the triangles intersect */

    if(isect2[0]<isect1[0])
    {
        if(smallest1==0) { borVec3Copy(isectpt1,&isectpointA1); }
        else { borVec3Copy(isectpt1,&isectpointA2); }

        if(isect2[1]<isect1[1])
        {
            if(smallest2==0) { borVec3Copy(isectpt2,&isectpointB2); }
            else { borVec3Copy(isectpt2,&isectpointB1); }
        }
        else
        {
            if(smallest1==0) { borVec3Copy(isectpt2,&isectpointA2); }
            else { borVec3Copy(isectpt2,&isectpointA1); }
        }
    }
    else
    {
        if(smallest2==0) { borVec3Copy(isectpt1,&isectpointB1); }
        else { borVec3Copy(isectpt1,&isectpointB2); }

        if(isect2[1]>isect1[1])
        {
            if(smallest1==0) { borVec3Copy(isectpt2,&isectpointA2); }
            else { borVec3Copy(isectpt2,&isectpointA1); }
        }
        else
        {
            if(smallest2==0) { borVec3Copy(isectpt2,&isectpointB2); }
            else { borVec3Copy(isectpt2,&isectpointB1); }
        }
    }
    return 1;
}
