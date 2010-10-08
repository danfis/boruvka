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

#include <stdio.h>
#include <mg/alloc.h>
#include <mg/vec3.h>
#include <mg/dbg.h>

static MG_VEC3(__mg_vec3_origin, MG_ZERO, MG_ZERO, MG_ZERO);
const mg_vec3_t *mg_vec3_origin = &__mg_vec3_origin;

static mg_vec3_t points_on_sphere[] = {
	MG_VEC3_STATIC(MG_REAL( 0.000000), MG_REAL(-0.000000), MG_REAL(-1.000000)),
	MG_VEC3_STATIC(MG_REAL( 0.723608), MG_REAL(-0.525725), MG_REAL(-0.447219)),
	MG_VEC3_STATIC(MG_REAL(-0.276388), MG_REAL(-0.850649), MG_REAL(-0.447219)),
	MG_VEC3_STATIC(MG_REAL(-0.894426), MG_REAL(-0.000000), MG_REAL(-0.447216)),
	MG_VEC3_STATIC(MG_REAL(-0.276388), MG_REAL( 0.850649), MG_REAL(-0.447220)),
	MG_VEC3_STATIC(MG_REAL( 0.723608), MG_REAL( 0.525725), MG_REAL(-0.447219)),
	MG_VEC3_STATIC(MG_REAL( 0.276388), MG_REAL(-0.850649), MG_REAL( 0.447220)),
	MG_VEC3_STATIC(MG_REAL(-0.723608), MG_REAL(-0.525725), MG_REAL( 0.447219)),
	MG_VEC3_STATIC(MG_REAL(-0.723608), MG_REAL( 0.525725), MG_REAL( 0.447219)),
	MG_VEC3_STATIC(MG_REAL( 0.276388), MG_REAL( 0.850649), MG_REAL( 0.447219)),
	MG_VEC3_STATIC(MG_REAL( 0.894426), MG_REAL( 0.000000), MG_REAL( 0.447216)),
	MG_VEC3_STATIC(MG_REAL(-0.000000), MG_REAL( 0.000000), MG_REAL( 1.000000)), 
	MG_VEC3_STATIC(MG_REAL( 0.425323), MG_REAL(-0.309011), MG_REAL(-0.850654)),
	MG_VEC3_STATIC(MG_REAL(-0.162456), MG_REAL(-0.499995), MG_REAL(-0.850654)),
	MG_VEC3_STATIC(MG_REAL( 0.262869), MG_REAL(-0.809012), MG_REAL(-0.525738)),
	MG_VEC3_STATIC(MG_REAL( 0.425323), MG_REAL( 0.309011), MG_REAL(-0.850654)),
	MG_VEC3_STATIC(MG_REAL( 0.850648), MG_REAL(-0.000000), MG_REAL(-0.525736)),
	MG_VEC3_STATIC(MG_REAL(-0.525730), MG_REAL(-0.000000), MG_REAL(-0.850652)),
	MG_VEC3_STATIC(MG_REAL(-0.688190), MG_REAL(-0.499997), MG_REAL(-0.525736)),
	MG_VEC3_STATIC(MG_REAL(-0.162456), MG_REAL( 0.499995), MG_REAL(-0.850654)),
	MG_VEC3_STATIC(MG_REAL(-0.688190), MG_REAL( 0.499997), MG_REAL(-0.525736)),
	MG_VEC3_STATIC(MG_REAL( 0.262869), MG_REAL( 0.809012), MG_REAL(-0.525738)),
	MG_VEC3_STATIC(MG_REAL( 0.951058), MG_REAL( 0.309013), MG_REAL( 0.000000)),
	MG_VEC3_STATIC(MG_REAL( 0.951058), MG_REAL(-0.309013), MG_REAL( 0.000000)),
	MG_VEC3_STATIC(MG_REAL( 0.587786), MG_REAL(-0.809017), MG_REAL( 0.000000)),
	MG_VEC3_STATIC(MG_REAL( 0.000000), MG_REAL(-1.000000), MG_REAL( 0.000000)),
	MG_VEC3_STATIC(MG_REAL(-0.587786), MG_REAL(-0.809017), MG_REAL( 0.000000)),
	MG_VEC3_STATIC(MG_REAL(-0.951058), MG_REAL(-0.309013), MG_REAL(-0.000000)),
	MG_VEC3_STATIC(MG_REAL(-0.951058), MG_REAL( 0.309013), MG_REAL(-0.000000)),
	MG_VEC3_STATIC(MG_REAL(-0.587786), MG_REAL( 0.809017), MG_REAL(-0.000000)),
	MG_VEC3_STATIC(MG_REAL(-0.000000), MG_REAL( 1.000000), MG_REAL(-0.000000)),
	MG_VEC3_STATIC(MG_REAL( 0.587786), MG_REAL( 0.809017), MG_REAL(-0.000000)),
	MG_VEC3_STATIC(MG_REAL( 0.688190), MG_REAL(-0.499997), MG_REAL( 0.525736)),
	MG_VEC3_STATIC(MG_REAL(-0.262869), MG_REAL(-0.809012), MG_REAL( 0.525738)),
	MG_VEC3_STATIC(MG_REAL(-0.850648), MG_REAL( 0.000000), MG_REAL( 0.525736)),
	MG_VEC3_STATIC(MG_REAL(-0.262869), MG_REAL( 0.809012), MG_REAL( 0.525738)),
	MG_VEC3_STATIC(MG_REAL( 0.688190), MG_REAL( 0.499997), MG_REAL( 0.525736)),
	MG_VEC3_STATIC(MG_REAL( 0.525730), MG_REAL( 0.000000), MG_REAL( 0.850652)),
	MG_VEC3_STATIC(MG_REAL( 0.162456), MG_REAL(-0.499995), MG_REAL( 0.850654)),
	MG_VEC3_STATIC(MG_REAL(-0.425323), MG_REAL(-0.309011), MG_REAL( 0.850654)),
	MG_VEC3_STATIC(MG_REAL(-0.425323), MG_REAL( 0.309011), MG_REAL( 0.850654)),
	MG_VEC3_STATIC(MG_REAL( 0.162456), MG_REAL( 0.499995), MG_REAL( 0.850654))
};
mg_vec3_t *mg_points_on_sphere = points_on_sphere;
size_t mg_points_on_sphere_len = sizeof(points_on_sphere) / sizeof(mg_vec3_t);

mg_vec3_t *mgVec3New(mg_real_t x, mg_real_t y, mg_real_t z)
{
    mg_vec3_t *v = MG_ALLOC(mg_vec3_t);
    mgVec3Set(v, x, y, z);
    return v;
}

void mgVec3Del(mg_vec3_t *v)
{
    free(v);
}

_mg_inline mg_real_t __mgVec3PointSegmentDist2(const mg_vec3_t *P,
                                                  const mg_vec3_t *x0,
                                                  const mg_vec3_t *b,
                                                  mg_vec3_t *witness)
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

    mg_real_t dist, t;
    mg_vec3_t d, a;

    // direction of segment
    mgVec3Sub2(&d, b, x0);

    // precompute vector from P to x0
    mgVec3Sub2(&a, x0, P);

    t  = -MG_REAL(1.) * mgVec3Dot(&a, &d);
    t /= mgVec3Len2(&d);

    if (t < MG_ZERO || mgIsZero(t)){
        dist = mgVec3Dist2(x0, P);
        if (witness)
            mgVec3Copy(witness, x0);
    }else if (t > MG_ONE || mgEq(t, MG_ONE)){
        dist = mgVec3Dist2(b, P);
        if (witness)
            mgVec3Copy(witness, b);
    }else{
        if (witness){
            mgVec3Copy(witness, &d);
            mgVec3Scale(witness, t);
            mgVec3Add(witness, x0);
            dist = mgVec3Dist2(witness, P);
        }else{
            // recycling variables
            mgVec3Scale(&d, t);
            mgVec3Add(&d, &a);
            dist = mgVec3Len2(&d);
        }
    }

    return dist;
}

mg_real_t mgVec3PointSegmentDist2(const mg_vec3_t *P,
                                    const mg_vec3_t *x0, const mg_vec3_t *b,
                                    mg_vec3_t *witness)
{
    return __mgVec3PointSegmentDist2(P, x0, b, witness);
}

mg_real_t mgVec3PointTriDist2(const mg_vec3_t *P,
                                const mg_vec3_t *x0, const mg_vec3_t *B,
                                const mg_vec3_t *C,
                                mg_vec3_t *witness)
{
    // Computation comes from analytic expression for triangle (x0, B, C)
    //      T(s, t) = x0 + s.d1 + t.d2, where d1 = B - x0 and d2 = C - x0 and
    // Then equation for distance is:
    //      D(s, t) = | T(s, t) - P |^2
    // This leads to minimization of quadratic function of two variables.
    // The solution from is taken only if s is between 0 and 1, t is
    // between 0 and 1 and t + s < 1, otherwise distance from segment is
    // computed.

    mg_vec3_t d1, d2, a;
    mg_real_t u, v, w, p, q, r;
    mg_real_t s, t, dist, dist2;
    mg_vec3_t witness2;

    mgVec3Sub2(&d1, B, x0);
    mgVec3Sub2(&d2, C, x0);
    mgVec3Sub2(&a, x0, P);

    u = mgVec3Dot(&a, &a);
    v = mgVec3Dot(&d1, &d1);
    w = mgVec3Dot(&d2, &d2);
    p = mgVec3Dot(&a, &d1);
    q = mgVec3Dot(&a, &d2);
    r = mgVec3Dot(&d1, &d2);

    s = (q * r - w * p) / (w * v - r * r);
    t = (-s * r - q) / w;

    if ((mgIsZero(s) || s > MG_ZERO)
            && (mgEq(s, MG_ONE) || s < MG_ONE)
            && (mgIsZero(t) || t > MG_ZERO)
            && (mgEq(t, MG_ONE) || t < MG_ONE)
            && (mgEq(t + s, MG_ONE) || t + s < MG_ONE)){

        if (witness){
            mgVec3Scale(&d1, s);
            mgVec3Scale(&d2, t);
            mgVec3Copy(witness, x0);
            mgVec3Add(witness, &d1);
            mgVec3Add(witness, &d2);

            dist = mgVec3Dist2(witness, P);
        }else{
            dist  = s * s * v;
            dist += t * t * w;
            dist += MG_REAL(2.) * s * t * r;
            dist += MG_REAL(2.) * s * p;
            dist += MG_REAL(2.) * t * q;
            dist += u;
        }
    }else{
        dist = __mgVec3PointSegmentDist2(P, x0, B, witness);

        dist2 = __mgVec3PointSegmentDist2(P, x0, C, &witness2);
        if (dist2 < dist){
            dist = dist2;
            if (witness)
                mgVec3Copy(witness, &witness2);
        }

        dist2 = __mgVec3PointSegmentDist2(P, B, C, &witness2);
        if (dist2 < dist){
            dist = dist2;
            if (witness)
                mgVec3Copy(witness, &witness2);
        }
    }

    return dist;
}
