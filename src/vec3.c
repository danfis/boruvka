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

static FER_VEC3(__fer_vec3_origin, FER_ZERO, FER_ZERO, FER_ZERO);
const bor_vec3_t *fer_vec3_origin = &__fer_vec3_origin;

static FER_VEC3(__fer_vec3_axis_x, FER_ONE,  FER_ZERO, FER_ZERO);
static FER_VEC3(__fer_vec3_axis_y, FER_ZERO, FER_ONE,  FER_ZERO);
static FER_VEC3(__fer_vec3_axis_z, FER_ZERO, FER_ZERO, FER_ONE);

const bor_vec3_t *fer_vec3_axis[3] = {
    &__fer_vec3_axis_x,
    &__fer_vec3_axis_y,
    &__fer_vec3_axis_z
};

static bor_vec3_t points_on_sphere[] = {
    FER_VEC3_STATIC(FER_REAL( 0.000000), FER_REAL(-0.000000), FER_REAL(-1.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.723608), FER_REAL(-0.525725), FER_REAL(-0.447219)),
    FER_VEC3_STATIC(FER_REAL(-0.276388), FER_REAL(-0.850649), FER_REAL(-0.447219)),
    FER_VEC3_STATIC(FER_REAL(-0.894426), FER_REAL(-0.000000), FER_REAL(-0.447216)),
    FER_VEC3_STATIC(FER_REAL(-0.276388), FER_REAL( 0.850649), FER_REAL(-0.447220)),
    FER_VEC3_STATIC(FER_REAL( 0.723608), FER_REAL( 0.525725), FER_REAL(-0.447219)),
    FER_VEC3_STATIC(FER_REAL( 0.276388), FER_REAL(-0.850649), FER_REAL( 0.447220)),
    FER_VEC3_STATIC(FER_REAL(-0.723608), FER_REAL(-0.525725), FER_REAL( 0.447219)),
    FER_VEC3_STATIC(FER_REAL(-0.723608), FER_REAL( 0.525725), FER_REAL( 0.447219)),
    FER_VEC3_STATIC(FER_REAL( 0.276388), FER_REAL( 0.850649), FER_REAL( 0.447219)),
    FER_VEC3_STATIC(FER_REAL( 0.894426), FER_REAL( 0.000000), FER_REAL( 0.447216)),
    FER_VEC3_STATIC(FER_REAL(-0.000000), FER_REAL( 0.000000), FER_REAL( 1.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.425323), FER_REAL(-0.309011), FER_REAL(-0.850654)),
    FER_VEC3_STATIC(FER_REAL(-0.162456), FER_REAL(-0.499995), FER_REAL(-0.850654)),
    FER_VEC3_STATIC(FER_REAL( 0.262869), FER_REAL(-0.809012), FER_REAL(-0.525738)),
    FER_VEC3_STATIC(FER_REAL( 0.425323), FER_REAL( 0.309011), FER_REAL(-0.850654)),
    FER_VEC3_STATIC(FER_REAL( 0.850648), FER_REAL(-0.000000), FER_REAL(-0.525736)),
    FER_VEC3_STATIC(FER_REAL(-0.525730), FER_REAL(-0.000000), FER_REAL(-0.850652)),
    FER_VEC3_STATIC(FER_REAL(-0.688190), FER_REAL(-0.499997), FER_REAL(-0.525736)),
    FER_VEC3_STATIC(FER_REAL(-0.162456), FER_REAL( 0.499995), FER_REAL(-0.850654)),
    FER_VEC3_STATIC(FER_REAL(-0.688190), FER_REAL( 0.499997), FER_REAL(-0.525736)),
    FER_VEC3_STATIC(FER_REAL( 0.262869), FER_REAL( 0.809012), FER_REAL(-0.525738)),
    FER_VEC3_STATIC(FER_REAL( 0.951058), FER_REAL( 0.309013), FER_REAL( 0.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.951058), FER_REAL(-0.309013), FER_REAL( 0.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.587786), FER_REAL(-0.809017), FER_REAL( 0.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.000000), FER_REAL(-1.000000), FER_REAL( 0.000000)),
    FER_VEC3_STATIC(FER_REAL(-0.587786), FER_REAL(-0.809017), FER_REAL( 0.000000)),
    FER_VEC3_STATIC(FER_REAL(-0.951058), FER_REAL(-0.309013), FER_REAL(-0.000000)),
    FER_VEC3_STATIC(FER_REAL(-0.951058), FER_REAL( 0.309013), FER_REAL(-0.000000)),
    FER_VEC3_STATIC(FER_REAL(-0.587786), FER_REAL( 0.809017), FER_REAL(-0.000000)),
    FER_VEC3_STATIC(FER_REAL(-0.000000), FER_REAL( 1.000000), FER_REAL(-0.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.587786), FER_REAL( 0.809017), FER_REAL(-0.000000)),
    FER_VEC3_STATIC(FER_REAL( 0.688190), FER_REAL(-0.499997), FER_REAL( 0.525736)),
    FER_VEC3_STATIC(FER_REAL(-0.262869), FER_REAL(-0.809012), FER_REAL( 0.525738)),
    FER_VEC3_STATIC(FER_REAL(-0.850648), FER_REAL( 0.000000), FER_REAL( 0.525736)),
    FER_VEC3_STATIC(FER_REAL(-0.262869), FER_REAL( 0.809012), FER_REAL( 0.525738)),
    FER_VEC3_STATIC(FER_REAL( 0.688190), FER_REAL( 0.499997), FER_REAL( 0.525736)),
    FER_VEC3_STATIC(FER_REAL( 0.525730), FER_REAL( 0.000000), FER_REAL( 0.850652)),
    FER_VEC3_STATIC(FER_REAL( 0.162456), FER_REAL(-0.499995), FER_REAL( 0.850654)),
    FER_VEC3_STATIC(FER_REAL(-0.425323), FER_REAL(-0.309011), FER_REAL( 0.850654)),
    FER_VEC3_STATIC(FER_REAL(-0.425323), FER_REAL( 0.309011), FER_REAL( 0.850654)),
    FER_VEC3_STATIC(FER_REAL( 0.162456), FER_REAL( 0.499995), FER_REAL( 0.850654))
};
bor_vec3_t *fer_points_on_sphere = points_on_sphere;
size_t fer_points_on_sphere_len = sizeof(points_on_sphere) / sizeof(bor_vec3_t);

_fer_inline bor_real_t _ferVec3ACos(bor_real_t angle)
{
    if (ferEq(angle, FER_ONE) || angle > FER_ONE)
        angle = FER_ONE;
    if (ferEq(angle, -FER_ONE) || angle < -FER_ONE)
        angle = -FER_ONE;

    /*
    if (isnan(angle) || isnan(FER_ACOS(angle))){
        DBG("NAN: %f", angle);
    }
    */

    return FER_ACOS(angle);
}


bor_vec3_t *ferVec3New(bor_real_t x, bor_real_t y, bor_real_t z)
{
    bor_vec3_t *v;

#ifdef FER_SSE
    v = FER_ALLOC_ALIGN(bor_vec3_t, 16);
#else /* FER_SSE */
    v = FER_ALLOC(bor_vec3_t);
#endif /* FER_SSE */
    ferVec3Set(v, x, y, z);
    return v;
}

void ferVec3Del(bor_vec3_t *v)
{
    FER_FREE(v);
}

bor_vec3_t *ferVec3ArrNew(size_t num_vecs)
{
    bor_vec3_t *vs;

#ifdef FER_SSE
    vs = FER_ALLOC_ALIGN_ARR(bor_vec3_t, num_vecs, sizeof(bor_vec3_t));
#else /* FER_SSE */
    vs = FER_ALLOC_ARR(bor_vec3_t, num_vecs);
#endif /* FER_SSE */

    return vs;
}

void ferVec3ArrDel(bor_vec3_t *v)
{
    FER_FREE(v);
}

_fer_inline bor_real_t __ferVec3PointSegmentDist2(const bor_vec3_t *P,
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
    ferVec3Sub2(&d, b, x0);

    // precompute vector from P to x0
    ferVec3Sub2(&a, x0, P);

    t  = -FER_REAL(1.) * ferVec3Dot(&a, &d);
    t /= ferVec3Len2(&d);

    if (t < FER_ZERO || ferIsZero(t)){
        dist = ferVec3Dist2(x0, P);
        if (witness)
            ferVec3Copy(witness, x0);
    }else if (t > FER_ONE || ferEq(t, FER_ONE)){
        dist = ferVec3Dist2(b, P);
        if (witness)
            ferVec3Copy(witness, b);
    }else{
        if (witness){
            ferVec3Copy(witness, &d);
            ferVec3Scale(witness, t);
            ferVec3Add(witness, x0);
            dist = ferVec3Dist2(witness, P);
        }else{
            // recycling variables
            ferVec3Scale(&d, t);
            ferVec3Add(&d, &a);
            dist = ferVec3Len2(&d);
        }
    }

    return dist;
}

bor_real_t ferVec3PointSegmentDist2(const bor_vec3_t *P,
                                    const bor_vec3_t *x0, const bor_vec3_t *b,
                                    bor_vec3_t *witness)
{
    return __ferVec3PointSegmentDist2(P, x0, b, witness);
}


_fer_inline bor_real_t __clamp(bor_real_t n, bor_real_t min, bor_real_t max)
{
    if (n < min)
        return min;
    if (n > max)
        return max;
    return n;
}

bor_real_t ferVec3SegmentSegmentDist2(const bor_vec3_t *A, const bor_vec3_t *B,
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
    ferVec3Sub2(&d1, B, A);
    ferVec3Sub2(&d2, D, C);
    ferVec3Sub2(&r, A, C);

    // square lengths
    a = ferVec3Len2(&d1);
    e = ferVec3Len2(&d1);
    f = ferVec3Dot(&d2, &r);

    if (parallel)
        *parallel = 0;

    // check if either or both segments degenerate into point
    if (ferIsZero(a) && ferIsZero(e)){
        // both segments degenerate into points
        if (witness1)
            ferVec3Copy(witness1, A);
        if (witness2)
            ferVec3Copy(witness2, C);

        return ferVec3Dist2(A, C);
    }

    if (ferIsZero(a)){
        // segment AB degenerates into point
        if (witness1)
            ferVec3Copy(witness1, A);

        t = f / e;
        if (witness2){
            ferVec3Scale(&d2, t);
            ferVec3Add2(witness2, C, &d2);
        }

        return __clamp(t, FER_ZERO, FER_ONE);
    }else{
        c = ferVec3Dot(&d1, &r);
        if (ferIsZero(e)){
            // segment CD degenerates into point
            t = FER_ZERO;
            s = __clamp(-c / a, FER_ZERO, FER_ONE);
        }else{
            // general non-degenerate case
            b = ferVec3Dot(&d1, &d2);
            denom = (a * e) - (b * b);

            if (!ferIsZero(denom)){
                s = __clamp((b * f) - (c * e) / denom, FER_ZERO, FER_ONE);
            }else{
                // segments parallel - choose arbitrary s.
                s = FER_ZERO;
                if (parallel)
                    *parallel = 1;
            }

            t = b * s + f;
            if (t < FER_ZERO){
                t = FER_ZERO;
                s = __clamp(-c / a, FER_ZERO, FER_ONE);
            }else if (t > e){
                t = FER_ONE;
                s = __clamp((b - c) / a, FER_ZERO, FER_ONE);
            }else{
                t = t / e;
            }
        }
    }

    ferVec3Scale(&d1, s);
    ferVec3Add(&d1, A);
    ferVec3Scale(&d2, t);
    ferVec3Add(&d2, C);

    if (witness1){
        ferVec3Copy(witness1, &d1);
    }
    if (witness2){
        ferVec3Copy(witness2, &d2);
    }

    return ferVec3Dist2(&d1, &d2);
}


bor_real_t ferVec3PointTriDist2(const bor_vec3_t *P,
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

    ferVec3Sub2(&d1, B, x0);
    ferVec3Sub2(&d2, C, x0);
    ferVec3Sub2(&a, x0, P);

    u = ferVec3Dot(&a, &a);
    v = ferVec3Dot(&d1, &d1);
    w = ferVec3Dot(&d2, &d2);
    p = ferVec3Dot(&a, &d1);
    q = ferVec3Dot(&a, &d2);
    r = ferVec3Dot(&d1, &d2);

    s = (q * r - w * p) / (w * v - r * r);
    t = (-s * r - q) / w;

    if ((ferIsZero(s) || s > FER_ZERO)
            && (ferEq(s, FER_ONE) || s < FER_ONE)
            && (ferIsZero(t) || t > FER_ZERO)
            && (ferEq(t, FER_ONE) || t < FER_ONE)
            && (ferEq(t + s, FER_ONE) || t + s < FER_ONE)){

        if (witness){
            ferVec3Scale(&d1, s);
            ferVec3Scale(&d2, t);
            ferVec3Copy(witness, x0);
            ferVec3Add(witness, &d1);
            ferVec3Add(witness, &d2);

            dist = ferVec3Dist2(witness, P);
        }else{
            dist  = s * s * v;
            dist += t * t * w;
            dist += FER_REAL(2.) * s * t * r;
            dist += FER_REAL(2.) * s * p;
            dist += FER_REAL(2.) * t * q;
            dist += u;
        }
    }else{
        dist = __ferVec3PointSegmentDist2(P, x0, B, witness);

        dist2 = __ferVec3PointSegmentDist2(P, x0, C, &witness2);
        if (dist2 < dist){
            dist = dist2;
            if (witness)
                ferVec3Copy(witness, &witness2);
        }

        dist2 = __ferVec3PointSegmentDist2(P, B, C, &witness2);
        if (dist2 < dist){
            dist = dist2;
            if (witness)
                ferVec3Copy(witness, &witness2);
        }
    }

    return dist;
}

int ferVec3PointInTri(const bor_vec3_t *p,
                     const bor_vec3_t *a, const bor_vec3_t *b,
                     const bor_vec3_t *c)
{
    bor_vec3_t v0, v1, v2;
    bor_real_t dot00, dot01, dot02, dot11, dot12;
    bor_real_t inv_denom, u, v;

    // compute vectors
    ferVec3Sub2(&v0, c, a);
    ferVec3Sub2(&v1, b, a);
    ferVec3Sub2(&v2, p, a);

    // compute dot products
    dot00 = ferVec3Dot(&v0, &v0);
    dot01 = ferVec3Dot(&v0, &v1);
    dot02 = ferVec3Dot(&v0, &v2);
    dot11 = ferVec3Dot(&v1, &v1);
    dot12 = ferVec3Dot(&v1, &v2);

    // compute barycentric coordinates
    inv_denom = FER_ONE / (dot00 * dot11 - dot01 * dot01);
    u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
    v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

    // check if point is in triangle
    return (u > FER_ZERO || ferIsZero(u))
            && (v > FER_ZERO || ferIsZero(v))
            && (u + v < FER_ONE || ferEq(u + v, FER_ONE));
}

bor_real_t ferVec3Angle(const bor_vec3_t *a, const bor_vec3_t *b, const bor_vec3_t *c)
{
    bor_real_t angle, div;
    bor_real_t abx, aby, abz, cbx, cby, cbz;

    abx = ferVec3X(a) - ferVec3X(b);
    aby = ferVec3Y(a) - ferVec3Y(b);
    abz = ferVec3Z(a) - ferVec3Z(b);

    cbx = ferVec3X(c) - ferVec3X(b);
    cby = ferVec3Y(c) - ferVec3Y(b);
    cbz = ferVec3Z(c) - ferVec3Z(b);

    div = FER_SQRT(abx * abx + aby * aby + abz * abz);
    div *= FER_SQRT(cbx * cbx + cby * cby + cbz * cbz);
    if (ferIsZero(div))
        return FER_ZERO;

    angle = abx * cbx + aby * cby + abz * cbz;
    angle /= div;

    return _ferVec3ACos(angle);
}

bor_real_t ferVec3DihedralAngle(const bor_vec3_t *a, const bor_vec3_t *b,
                              const bor_vec3_t *c, const bor_vec3_t *d)
{
    bor_vec3_t base, v;
    bor_vec3_t na, nb;
    bor_real_t angle;

    // get normal vec3tors of planes
    ferVec3Sub2(&base, c, b);
    ferVec3Sub2(&v, a, b);
    ferVec3Cross(&na, &v, &base);

    ferVec3Sub2(&v, d, b);
    ferVec3Cross(&nb, &v, &base);

    // normalize normals
    ferVec3Normalize(&na);
    ferVec3Normalize(&nb);

    angle = ferVec3Dot(&na, &nb);

    return _ferVec3ACos(angle);
}

bor_real_t ferVec3ProjToPlane(const bor_vec3_t *p,
                            const bor_vec3_t *u, const bor_vec3_t *v,
                            const bor_vec3_t *w, bor_vec3_t *d)
{
    bor_vec3_t uv, wv, normal;

    // uv = u - v, wv = w - v
    ferVec3Sub2(&uv, u, v);
    ferVec3Sub2(&wv, w, v);

    // scale uv and wv to get better normal
    if (ferVec3Len2(&uv) < FER_ONE)
        ferVec3Normalize(&uv);
    if (ferVec3Len2(&wv) < FER_ONE)
        ferVec3Normalize(&wv);

    // compute normal vec3tor
    ferVec3Cross(&normal, &uv, &wv);

    if (ferVec3IsZero(&normal))
        return FER_REAL(-1.);

    return ferVec3ProjToPlane2(p, v, &normal, d);
}

bor_real_t ferVec3ProjToPlane2(const bor_vec3_t *p,
                             const bor_vec3_t *x, const bor_vec3_t *normal,
                             bor_vec3_t *d)
{
    bor_real_t k;
    bor_vec3_t xp;

    ferVec3Sub2(&xp, x, p);
    k  = ferVec3Dot(&xp, normal);
    k /= ferVec3Len2(normal);

    ferVec3Copy(d, normal);
    ferVec3Scale(d, k);
    ferVec3Add(d, p);

    k = ferVec3Dist(p, d);

    return k;
}

bor_real_t ferVec3TriArea2(const bor_vec3_t *a, const bor_vec3_t *b,
                         const bor_vec3_t *c)
{
    bor_vec3_t ba, bc, babc;
    bor_real_t area;

    ferVec3Sub2(&ba, a, b);
    ferVec3Sub2(&bc, c, b);
    ferVec3Cross(&babc, &ba, &bc);

    area = ferVec3Len(&babc);
    return area;
}





#define __overlapMinMax(p1, q1, r1, p2, q2, r2) \
    ferVec3Sub2(&v1, p2, q1); \
    ferVec3Sub2(&v2, p1, q1); \
    ferVec3Cross(&n1, &v1, &v2); \
    \
    ferVec3Sub2(&v1, q2, q1); \
    if (ferVec3Dot(&v1, &n1) > FER_ZERO) \
        return 0; \
    \
    ferVec3Sub2(&v1, p2, p1); \
    ferVec3Sub2(&v2, r1, p1); \
    ferVec3Cross(&n1, &v1, &v2); \
    \
    ferVec3Sub2(&v1, r2, p1); \
    if (ferVec3Dot(&v1, &n1) > FER_ZERO) \
        return 0; \
    \
    return 1;

#define __overlapCheck(p1, q1, r1, p2, q2, r2, sp2, sq2, sr2) \
    if (ferIsZero(sp2)){ \
        if (ferIsZero(sq2)){ \
            if (ferIsZero(sr2)){ \
                return __overlapCoplanar(p1, q1, r1, p2, q2, r2, &n1, &n2); \
            }else if (sr2 > FER_ZERO){ \
                __overlapMinMax(p1, q1, r1, r2, p2, q2) \
            }else{ /* sr2 < FER_ZERO */ \
                __overlapMinMax(p1, r1, q1, r2, p2, q2) \
            } \
        }else if (sq2 < FER_ZERO){ \
            if (ferIsZero(sr2) || sr2 > FER_ZERO){ \
                __overlapMinMax(p1, r1, q1, q2, r2, p2) \
            }else{ \
                __overlapMinMax(p1, q1, r1, p2, q2, r2) \
            } \
        }else{ /* sq2 > FER_ZERO */ \
            if (sr2 > FER_ZERO){ \
                __overlapMinMax(p1, r1, q1, p2, q2, r2) \
            }else{ \
                __overlapMinMax(p1, q1, r1, q2, r2, p2) \
            } \
        } \
    }else if (sp2 > FER_ZERO){ \
        if (sq2 > FER_ZERO){ \
            __overlapMinMax(p1, r1, q1, r2, p2, q2) \
        }else if (sr2 > FER_ZERO){ \
            __overlapMinMax(p1, r1, q1, q2, r2, p2) \
        }else{ \
            __overlapMinMax(p1, q1, r1, p2, q2, r2) \
        } \
    }else{ /* sp2 < FER_ZERO */ \
        if (sq2 < FER_ZERO){ \
            __overlapMinMax(p1, q1, r1, r2, p2, q2) \
        }else if (sr2 < FER_ZERO){ \
            __overlapMinMax(p1, q1, r1, q2, r2, p2) \
        }else{ \
            __overlapMinMax(p1, r1, q1, p2, q2, r2) \
        } \
    }


_fer_inline int __overlapCoplanar(const bor_vec3_t *_p1,
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

    n_x = FER_FABS(ferVec3X(n1));
    n_y = FER_FABS(ferVec3Y(n1));
    n_z = FER_FABS(ferVec3Z(n1));

    // Projection of the triangles in 3D onto 2D such that the area of
    // the projection is maximized.

    if (n_x > n_z && n_x >= n_y){
        // Project onto plane YZ
        ferVec2Set(&p1, ferVec3Z(_q1), ferVec3Y(_q1));
        ferVec2Set(&q1, ferVec3Z(_p1), ferVec3Y(_p1));
        ferVec2Set(&r1, ferVec3Z(_r1), ferVec3Y(_r1));

        ferVec2Set(&p2, ferVec3Z(_q2), ferVec3Y(_q2));
        ferVec2Set(&q2, ferVec3Z(_p2), ferVec3Y(_p2));
        ferVec2Set(&r2, ferVec3Z(_r2), ferVec3Y(_r2));
    }else if (n_y > n_z && n_y >= n_x){
        // Project onto plane XZ
        ferVec2Set(&p1, ferVec3X(_q1), ferVec3Z(_q1));
        ferVec2Set(&q1, ferVec3X(_p1), ferVec3Z(_p1));
        ferVec2Set(&r1, ferVec3X(_r1), ferVec3Z(_r1));

        ferVec2Set(&p2, ferVec3X(_q2), ferVec3Z(_q2));
        ferVec2Set(&q2, ferVec3X(_p2), ferVec3Z(_p2));
        ferVec2Set(&r2, ferVec3X(_r2), ferVec3Z(_r2));
    }else {
        // Project onto plane XY
        ferVec2Set(&p1, ferVec3X(_q1), ferVec3Y(_q1));
        ferVec2Set(&q1, ferVec3X(_p1), ferVec3Y(_p1));
        ferVec2Set(&r1, ferVec3X(_r1), ferVec3Y(_r1));

        ferVec2Set(&p2, ferVec3X(_q2), ferVec3Y(_q2));
        ferVec2Set(&q2, ferVec3X(_p2), ferVec3Y(_p2));
        ferVec2Set(&r2, ferVec3X(_r2), ferVec3Y(_r2));
    }

    return ferVec2TriTriOverlap(&p1, &q1, &r1,
                                &p2, &q2, &r2);
}

int ferVec3TriTriOverlap(const bor_vec3_t *p1, const bor_vec3_t *q1,
                         const bor_vec3_t *r1,
                         const bor_vec3_t *p2, const bor_vec3_t *q2,
                         const bor_vec3_t *r2)
{
    bor_vec3_t n1, n2, v1, v2;
    bor_real_t sp1, sq1, sr1;
    bor_real_t sp2, sq2, sr2;

    // compute signs of first triangle to second triangle
    ferVec3Sub2(&v1, p2, r2);
    ferVec3Sub2(&v2, q2, r2);
    ferVec3Cross(&n2, &v1, &v2);
    ferVec3Normalize(&n2);

    ferVec3Sub2(&v1, p1, r2);
    sp1 = ferVec3Dot(&v1, &n2);
    ferVec3Sub2(&v1, q1, r2);
    sq1 = ferVec3Dot(&v1, &n2);
    ferVec3Sub2(&v1, r1, r2);
    sr1 = ferVec3Dot(&v1, &n2);

    // check if all points from tri1 aren't on same side
    if (sp1 * sq1 > FER_ZERO
            && sp1 * sr1 > FER_ZERO)
        return 0;


    // compute signs for second triangle
    ferVec3Sub2(&v1, q1, p1);
    ferVec3Sub2(&v2, r1, p1);
    ferVec3Cross(&n1, &v1, &v2);
    ferVec3Normalize(&n1);

    ferVec3Sub2(&v1, p2, r1);
    sp2 = ferVec3Dot(&v1, &n1);
    ferVec3Sub2(&v1, q2, r1);
    sq2 = ferVec3Dot(&v1, &n1);
    ferVec3Sub2(&v1, r2, r1);
    sr2 = ferVec3Dot(&v1, &n1);

    // check if all tri2s' points aren't on same side
    if (sp2 * sq2 > FER_ZERO
            && sp2 * sr2 > FER_ZERO)
        return 0;


    if (ferIsZero(sp1)){
        if (ferIsZero(sq1)){
            if (ferIsZero(sr1)){
                return __overlapCoplanar(p1, q1, r1, p2, q2, r2, &n1, &n2);
            }else if (sr1 > FER_ZERO){
                __overlapCheck(r1,p1,q1,p2,q2,r2,sp2,sq2,sr2)
            }else if (sr1 < FER_ZERO){
                __overlapCheck(r1,p1,q1,p2,r2,q2,sp2,sr2,sq2)
            }
        }else if (sq1 < FER_ZERO){
            if (ferIsZero(sr1) || sr1 > FER_ZERO){
                __overlapCheck(q1,r1,p1,p2,r2,q2,sp2,sr2,sq2)
            }else{
                __overlapCheck(p1,q1,r1,p2,q2,r2,sp2,sq2,sr2)
            }
        }else{ // sq1 > FER_ZERO
            if (sr1 > FER_ZERO){
                __overlapCheck(p1,q1,r1,p2,r2,q2,sp2,sr2,sq2)
            }else{
                __overlapCheck(q1,r1,p1,p2,q2,r2,sp2,sq2,sr2)
            }
        }
    }else if (sp1 > FER_ZERO){
        if (sq1 > FER_ZERO){
            __overlapCheck(r1, p1, q1, p2, r2, q2, sp2, sr2, sq2)
        }else if (sr1 > FER_ZERO){
            __overlapCheck(q1,r1,p1,p2,r2,q2,sp2,sr2,sq2)
        }else{
            __overlapCheck(p1,q1,r1,p2,q2,r2,sp2,sq2,sr2)
        }
    }else{ // sp1 < FER_ZERO
        if (sq1 < FER_ZERO){
            __overlapCheck(r1,p1,q1,p2,q2,r2,sp2,sq2,sr2)
        }else if (sr1 < FER_ZERO){
            __overlapCheck(q1,r1,p1,p2,q2,r2,sp2,sq2,sr2)
        }else{
            __overlapCheck(p1,q1,r1,p2,r2,q2,sp2,sr2,sq2)
        }
    }

    return 0;
}


/** ferVec3TriTriIntersect() adapted from http://jgt.akpeters.com/papers/Moller97/tritri.html */
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
  Bx=ferVec3Get(U0, i0)-ferVec3Get(U1, i0);           \
  By=ferVec3Get(U0, i1)-ferVec3Get(U1, i1);           \
  Cx=ferVec3Get(V0, i0)-ferVec3Get(U0, i0);           \
  Cy=ferVec3Get(V0, i1)-ferVec3Get(U0, i1);           \
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
  Ax=ferVec3Get(V1, i0)-ferVec3Get(V0, i0);    \
  Ay=ferVec3Get(V1, i1)-ferVec3Get(V0, i1);    \
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
  a=ferVec3Get(U1, i1)-ferVec3Get(U0, i1);                          \
  b=-(ferVec3Get(U1, i0)-ferVec3Get(U0, i0));                       \
  c=-a*ferVec3Get(U0, i0)-b*ferVec3Get(U0, i1);                     \
  d0=a*ferVec3Get(V0, i0)+b*ferVec3Get(V0, i1)+c;                   \
                                            \
  a=ferVec3Get(U2, i1)-ferVec3Get(U1, i1);                          \
  b=-(ferVec3Get(U2, i0)-ferVec3Get(U1, i0));                       \
  c=-a*ferVec3Get(U1, i0)-b*ferVec3Get(U1, i1);                     \
  d1=a*ferVec3Get(V0, i0)+b*ferVec3Get(V0, i1)+c;                   \
                                            \
  a=ferVec3Get(U0, i1)-ferVec3Get(U2, i1);                          \
  b=-(ferVec3Get(U0, i0)-ferVec3Get(U2, i0));                       \
  c=-a*ferVec3Get(U2, i0)-b*ferVec3Get(U2, i1);                     \
  d2=a*ferVec3Get(V0, i0)+b*ferVec3Get(V0, i1)+c;                   \
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
    A[0]=FER_FABS(ferVec3X(N));
    A[1]=FER_FABS(ferVec3Y(N));
    A[2]=FER_FABS(ferVec3Z(N));
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

_fer_inline void isect2(const bor_vec3_t *VTX0, const bor_vec3_t *VTX1,
                        const bor_vec3_t *VTX2,
                        bor_real_t VV0,bor_real_t VV1,bor_real_t VV2,
                        bor_real_t D0,bor_real_t D1,bor_real_t D2,bor_real_t *isect0,bor_real_t *isect1,
                        bor_vec3_t *isectpoint0, bor_vec3_t *isectpoint1)
{
    float tmp=D0/(D0-D1);
    bor_vec3_t diff;

    *isect0=VV0+(VV1-VV0)*tmp;
    ferVec3Sub2(&diff, VTX1, VTX0);
    ferVec3Scale(&diff, tmp);
    ferVec3Add2(isectpoint0,&diff,VTX0);
    tmp=D0/(D0-D2);
    *isect1=VV0+(VV2-VV0)*tmp;
    ferVec3Sub2(&diff,VTX2,VTX0);
    ferVec3Scale(&diff,tmp);
    ferVec3Add2(isectpoint1,VTX0,&diff);
}
_fer_inline int compute_intervals_isectline(const bor_vec3_t *VERT0,
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

int ferVec3TriTriIntersect(const bor_vec3_t *V0, const bor_vec3_t *V1,
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
    ferVec3Sub2(&E1,V1,V0);
    ferVec3Sub2(&E2,V2,V0);
    ferVec3Cross(&N1,&E1,&E2);
    d1=-ferVec3Dot(&N1,V0);
    /* plane equation 1: N1.X+d1=0 */

    /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
    du0=ferVec3Dot(&N1,U0)+d1;
    du1=ferVec3Dot(&N1,U1)+d1;
    du2=ferVec3Dot(&N1,U2)+d1;

    /* coplanarity robustness check */
#ifdef USE_EPSILON_TEST
    if(ferIsZero(du0)) du0 = FER_ZERO;
    if(ferIsZero(du1)) du1 = FER_ZERO;
    if(ferIsZero(du2)) du2 = FER_ZERO;
#endif
    du0du1=du0*du1;
    du0du2=du0*du2;

    if(du0du1>0.0f && du0du2>0.0f) /* same sign on all of them + not equal 0 ? */
        return 0;                    /* no intersection occurs */

    /* compute plane of triangle (U0,U1,U2) */
    ferVec3Sub2(&E1,U1,U0);
    ferVec3Sub2(&E2,U2,U0);
    ferVec3Cross(&N2,&E1,&E2);
    d2=-ferVec3Dot(&N2,U0);
    /* plane equation 2: N2.X+d2=0 */

    /* put V0,V1,V2 into plane equation 2 */
    dv0=ferVec3Dot(&N2,V0)+d2;
    dv1=ferVec3Dot(&N2,V1)+d2;
    dv2=ferVec3Dot(&N2,V2)+d2;

#ifdef USE_EPSILON_TEST
    if(ferIsZero(dv0)) dv0 = FER_ZERO;
    if(ferIsZero(dv1)) dv1 = FER_ZERO;
    if(ferIsZero(dv2)) dv2 = FER_ZERO;
#endif

    dv0dv1=dv0*dv1;
    dv0dv2=dv0*dv2;

    if(dv0dv1>0.0f && dv0dv2>0.0f) /* same sign on all of them + not equal 0 ? */
        return 0;                    /* no intersection occurs */

    /* compute direction of intersection line */
    ferVec3Cross(&D,&N1,&N2);

    /* compute and index to the largest component of D */
    max=FER_FABS(ferVec3X(&D));
    index=0;
    b=FER_FABS(ferVec3Y(&D));
    c=FER_FABS(ferVec3Z(&D));
    if(b>max) max=b,index=1;
    if(c>max) max=c,index=2;

    /* this is the simplified projection onto L*/
    vp0=ferVec3Get(V0, index);
    vp1=ferVec3Get(V1, index);
    vp2=ferVec3Get(V2, index);

    up0=ferVec3Get(U0, index);
    up1=ferVec3Get(U1, index);
    up2=ferVec3Get(U2, index);

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
        if(smallest1==0) { ferVec3Copy(isectpt1,&isectpointA1); }
        else { ferVec3Copy(isectpt1,&isectpointA2); }

        if(isect2[1]<isect1[1])
        {
            if(smallest2==0) { ferVec3Copy(isectpt2,&isectpointB2); }
            else { ferVec3Copy(isectpt2,&isectpointB1); }
        }
        else
        {
            if(smallest1==0) { ferVec3Copy(isectpt2,&isectpointA2); }
            else { ferVec3Copy(isectpt2,&isectpointA1); }
        }
    }
    else
    {
        if(smallest2==0) { ferVec3Copy(isectpt1,&isectpointB1); }
        else { ferVec3Copy(isectpt1,&isectpointB2); }

        if(isect2[1]>isect1[1])
        {
            if(smallest1==0) { ferVec3Copy(isectpt2,&isectpointA2); }
            else { ferVec3Copy(isectpt2,&isectpointA1); }
        }
        else
        {
            if(smallest2==0) { ferVec3Copy(isectpt2,&isectpointB2); }
            else { ferVec3Copy(isectpt2,&isectpointB1); }
        }
    }
    return 1;
}
