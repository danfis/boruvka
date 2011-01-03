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

#include <stdio.h>
#include <fermat/alloc.h>
#include <fermat/vec3.h>
#include <fermat/dbg.h>

static FER_VEC3(__fer_vec3_origin, FER_ZERO, FER_ZERO, FER_ZERO);
const fer_vec3_t *fer_vec3_origin = &__fer_vec3_origin;

static fer_vec3_t points_on_sphere[] = {
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
fer_vec3_t *fer_points_on_sphere = points_on_sphere;
size_t fer_points_on_sphere_len = sizeof(points_on_sphere) / sizeof(fer_vec3_t);

_fer_inline fer_real_t _ferVec3ACos(fer_real_t angle)
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


fer_vec3_t *ferVec3New(fer_real_t x, fer_real_t y, fer_real_t z)
{
    fer_vec3_t *v;

#ifdef FER_SSE
    v = FER_ALLOC_ALIGN(fer_vec3_t, sizeof(fer_vec3_t));
#else /* FER_SSE */
    v = FER_ALLOC(fer_vec3_t);
#endif /* FER_SSE */
    ferVec3Set(v, x, y, z);
    return v;
}

void ferVec3Del(fer_vec3_t *v)
{
    free(v);
}

fer_vec3_t *ferVec3ArrNew(size_t num_vecs)
{
    fer_vec3_t *vs;

#ifdef FER_SSE
    vs = FER_ALLOC_ALIGN_ARR(fer_vec3_t, num_vecs, sizeof(fer_vec3_t));
#else /* FER_SSE */
    vs = FER_ALLOC_ARR(fer_vec3_t, num_vecs);
#endif /* FER_SSE */

    return vs;
}

void ferVec3ArrDel(fer_vec3_t *v)
{
    free(v);
}

_fer_inline fer_real_t __ferVec3PointSegmentDist2(const fer_vec3_t *P,
                                               const fer_vec3_t *x0,
                                               const fer_vec3_t *b,
                                               fer_vec3_t *witness)
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

    fer_real_t dist, t;
    fer_vec3_t d, a;

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

fer_real_t ferVec3PointSegmentDist2(const fer_vec3_t *P,
                                    const fer_vec3_t *x0, const fer_vec3_t *b,
                                    fer_vec3_t *witness)
{
    return __ferVec3PointSegmentDist2(P, x0, b, witness);
}

fer_real_t ferVec3PointTriDist2(const fer_vec3_t *P,
                              const fer_vec3_t *x0, const fer_vec3_t *B,
                              const fer_vec3_t *C,
                              fer_vec3_t *witness)
{
    // Computation comes from analytic expression for triangle (x0, B, C)
    //      T(s, t) = x0 + s.d1 + t.d2, where d1 = B - x0 and d2 = C - x0 and
    // Then equation for distance is:
    //      D(s, t) = | T(s, t) - P |^2
    // This leads to minimization of quadratic function of two variables.
    // The solution from is taken only if s is between 0 and 1, t is
    // between 0 and 1 and t + s < 1, otherwise distance from segment is
    // computed.

    fer_vec3_t d1, d2, a;
    fer_real_t u, v, w, p, q, r;
    fer_real_t s, t, dist, dist2;
    fer_vec3_t witness2;

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

int ferVec3PointInTri(const fer_vec3_t *p,
                     const fer_vec3_t *a, const fer_vec3_t *b,
                     const fer_vec3_t *c)
{
    fer_vec3_t v0, v1, v2;
    fer_real_t dot00, dot01, dot02, dot11, dot12;
    fer_real_t inv_denom, u, v;

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

fer_real_t ferVec3Angle(const fer_vec3_t *a, const fer_vec3_t *b, const fer_vec3_t *c)
{
    fer_real_t angle, div;
    fer_real_t abx, aby, abz, cbx, cby, cbz;

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

fer_real_t ferVec3DihedralAngle(const fer_vec3_t *a, const fer_vec3_t *b,
                              const fer_vec3_t *c, const fer_vec3_t *d)
{
    fer_vec3_t base, v;
    fer_vec3_t na, nb;
    fer_real_t angle;

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

fer_real_t ferVec3ProjToPlane(const fer_vec3_t *p,
                            const fer_vec3_t *u, const fer_vec3_t *v,
                            const fer_vec3_t *w, fer_vec3_t *d)
{
    fer_vec3_t uv, wv, normal;

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

fer_real_t ferVec3ProjToPlane2(const fer_vec3_t *p,
                             const fer_vec3_t *x, const fer_vec3_t *normal,
                             fer_vec3_t *d)
{
    fer_real_t k;
    fer_vec3_t xp;

    ferVec3Sub2(&xp, x, p);
    k  = ferVec3Dot(&xp, normal);
    k /= ferVec3Len2(normal);

    ferVec3Copy(d, normal);
    ferVec3Scale(d, k);
    ferVec3Add(d, p);

    k = ferVec3Dist(p, d);

    return k;
}

fer_real_t ferVec3TriArea2(const fer_vec3_t *a, const fer_vec3_t *b,
                         const fer_vec3_t *c)
{
    fer_vec3_t ba, bc, babc;
    fer_real_t area;

    ferVec3Sub2(&ba, a, b);
    ferVec3Sub2(&bc, c, b);
    ferVec3Cross(&babc, &ba, &bc);

    area = ferVec3Len(&babc);
    return area;
}
