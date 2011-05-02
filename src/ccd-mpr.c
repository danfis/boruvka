/***
 * libccd
 * ---------------------------------
 * Copyright (c)2010,2011 Daniel Fiser <danfis@danfis.cz>
 *
 *
 *  This file is part of libccd.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <stdlib.h>
#include <fermat/ccd.h>
#include <fermat/ccd-simplex.h>
#include <fermat/dbg.h>

/** Finds origin (center) of Minkowski difference (actually it can be any
 *  interior point of Minkowski difference. */
_fer_inline void findOrigin(const void *obj1, const void *obj2, const fer_t *ccd,
                            fer_support_t *center);

/** Discovers initial portal - that is tetrahedron that intersects with
 *  origin ray (ray from center of Minkowski diff to (0,0,0).
 *
 *  Returns -1 if already recognized that origin is outside Minkowski
 *  portal.
 *  Returns 1 if origin lies on v1 of simplex (only v0 and v1 are present
 *  in simplex).
 *  Returns 2 if origin lies on v0-v1 segment.
 *  Returns 0 if portal was built.
 */
static int discoverPortal(const void *obj1, const void *obj2,
                          const fer_t *ccd, fer_simplex_t *portal);


/** Expands portal towards origin and determine if objects intersect.
 *  Already established portal must be given as argument.
 *  If intersection is found 0 is returned, -1 otherwise */
static int refinePortal(const void *obj1, const void *obj2,
                        const fer_t *ccd, fer_simplex_t *portal);

/** Finds penetration info by expanding provided portal. */
static void findPenetr(const void *obj1, const void *obj2, const fer_t *ccd,
                       fer_simplex_t *portal,
                       fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);

/** Finds penetration info if origin lies on portal's v1 */
static void findPenetrTouch(const void *obj1, const void *obj2, const fer_t *ccd,
                            fer_simplex_t *portal,
                            fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);

/** Find penetration info if origin lies on portal's segment v0-v1 */
static void findPenetrSegment(const void *obj1, const void *obj2, const fer_t *ccd,
                              fer_simplex_t *portal,
                              fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);

/** Finds position vector from fully established portal */
static void findPos(const void *obj1, const void *obj2, const fer_t *ccd,
                    const fer_simplex_t *portal, fer_vec3_t *pos);

/** Extends portal with new support point.
 *  Portal must have face v1-v2-v3 arranged to face outside portal. */
_fer_inline void expandPortal(fer_simplex_t *portal,
                              const fer_support_t *v4);

/** Fill dir with direction outside portal. Portal's v1-v2-v3 face must be
 *  arranged in correct order! */
_fer_inline void portalDir(const fer_simplex_t *portal, fer_vec3_t *dir);

/** Returns true if portal encapsules origin (0,0,0), dir is direction of
 *  v1-v2-v3 face. */
_fer_inline int portalEncapsulesOrigin(const fer_simplex_t *portal,
                                       const fer_vec3_t *dir);

/** Returns true if portal with new point v4 would reach specified
 *  tolerance (i.e. returns true if portal can _not_ significantly expand
 *  within Minkowski difference).
 *
 *  v4 is candidate for new point in portal, dir is direction in which v4
 *  was obtained. */
_fer_inline int portalReachTolerance(const fer_simplex_t *portal,
                                     const fer_support_t *v4,
                                     const fer_vec3_t *dir,
                                     const fer_t *ccd);

/** Returns true if portal expanded by new point v4 could possibly contain
 *  origin, dir is direction in which v4 was obtained. */
_fer_inline int portalCanEncapsuleOrigin(const fer_simplex_t *portal,   
                                         const fer_support_t *v4,
                                         const fer_vec3_t *dir);


int ferMPRIntersect(const void *obj1, const void *obj2, const fer_t *ccd)
{
    fer_simplex_t portal;
    int res;

    // Phase 1: Portal discovery - find portal that intersects with origin
    // ray (ray from center of Minkowski diff to origin of coordinates)
    res = discoverPortal(obj1, obj2, ccd, &portal);
    if (res < 0)
        return 0;
    if (res > 0)
        return 1;

    // Phase 2: Portal refinement
    res = refinePortal(obj1, obj2, ccd, &portal);
    return (res == 0 ? 1 : 0);
}

int ferMPRPenetration(const void *obj1, const void *obj2, const fer_t *ccd,
                      fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos)
{
    fer_simplex_t portal;
    int res;

    // Phase 1: Portal discovery
    res = discoverPortal(obj1, obj2, ccd, &portal);
    if (res < 0){
        // Origin isn't inside portal - no collision.
        return -1;

    }else if (res == 1){
        // Touching contact on portal's v1.
        findPenetrTouch(obj1, obj2, ccd, &portal, depth, dir, pos);

    }else if (res == 2){
        // Origin lies on v0-v1 segment.
        findPenetrSegment(obj1, obj2, ccd, &portal, depth, dir, pos);

    }else if (res == 0){
        // Phase 2: Portal refinement
        res = refinePortal(obj1, obj2, ccd, &portal);
        if (res < 0)
            return -1;

        // Phase 3. Penetration info
        findPenetr(obj1, obj2, ccd, &portal, depth, dir, pos);
    }

    return 0;
}



_fer_inline void findOrigin(const void *obj1, const void *obj2, const fer_t *ccd,
                            fer_support_t *center)
{
    ccd->center1(obj1, &center->v1);
    ccd->center2(obj2, &center->v2);
    ferVec3Sub2(&center->v, &center->v1, &center->v2);
}

static int discoverPortal(const void *obj1, const void *obj2,
                          const fer_t *ccd, fer_simplex_t *portal)
{
    fer_vec3_t dir, va, vb;
    fer_real_t dot;
    int cont;

    // vertex 0 is center of portal
    findOrigin(obj1, obj2, ccd, ferSimplexPointW(portal, 0));
    ferSimplexSetSize(portal, 1);

    if (ferVec3Eq(&ferSimplexPoint(portal, 0)->v, fer_vec3_origin)){
        // Portal's center lies on origin (0,0,0) => we know that objects
        // intersect but we would need to know penetration info.
        // So move center little bit...
        ferVec3Set(&va, FER_EPS * FER_REAL(10.), FER_ZERO, FER_ZERO);
        ferVec3Add(&ferSimplexPointW(portal, 0)->v, &va);
    }


    // vertex 1 = support in direction of origin
    ferVec3Copy(&dir, &ferSimplexPoint(portal, 0)->v);
    ferVec3Scale(&dir, FER_REAL(-1.));
    ferVec3Normalize(&dir);
    __ferSupport(obj1, obj2, &dir, ccd, ferSimplexPointW(portal, 1));
    ferSimplexSetSize(portal, 2);

    // test if origin isn't outside of v1
    dot = ferVec3Dot(&ferSimplexPoint(portal, 1)->v, &dir);
    if (ferIsZero(dot) || dot < FER_ZERO)
        return -1;


    // vertex 2
    ferVec3Cross(&dir, &ferSimplexPoint(portal, 0)->v,
                       &ferSimplexPoint(portal, 1)->v);
    if (ferIsZero(ferVec3Len2(&dir))){
        if (ferVec3Eq(&ferSimplexPoint(portal, 1)->v, fer_vec3_origin)){
            // origin lies on v1
            return 1;
        }else{
            // origin lies on v0-v1 segment
            return 2;
        }
    }

    ferVec3Normalize(&dir);
    __ferSupport(obj1, obj2, &dir, ccd, ferSimplexPointW(portal, 2));
    dot = ferVec3Dot(&ferSimplexPoint(portal, 2)->v, &dir);
    if (ferIsZero(dot) || dot < FER_ZERO)
        return -1;

    ferSimplexSetSize(portal, 3);

    // vertex 3 direction
    ferVec3Sub2(&va, &ferSimplexPoint(portal, 1)->v,
                     &ferSimplexPoint(portal, 0)->v);
    ferVec3Sub2(&vb, &ferSimplexPoint(portal, 2)->v,
                     &ferSimplexPoint(portal, 0)->v);
    ferVec3Cross(&dir, &va, &vb);
    ferVec3Normalize(&dir);

    // it is better to form portal faces to be oriented "outside" origin
    dot = ferVec3Dot(&dir, &ferSimplexPoint(portal, 0)->v);
    if (dot > FER_ZERO){
        ferSimplexSwap(portal, 1, 2);
        ferVec3Scale(&dir, FER_REAL(-1.));
    }

    while (ferSimplexSize(portal) < 4){
        __ferSupport(obj1, obj2, &dir, ccd, ferSimplexPointW(portal, 3));
        dot = ferVec3Dot(&ferSimplexPoint(portal, 3)->v, &dir);
        if (ferIsZero(dot) || dot < FER_ZERO)
            return -1;

        cont = 0;

        // test if origin is outside (v1, v0, v3) - set v2 as v3 and
        // continue
        ferVec3Cross(&va, &ferSimplexPoint(portal, 1)->v,
                          &ferSimplexPoint(portal, 3)->v);
        dot = ferVec3Dot(&va, &ferSimplexPoint(portal, 0)->v);
        if (dot < FER_ZERO && !ferIsZero(dot)){
            ferSimplexSet(portal, 2, ferSimplexPoint(portal, 3));
            cont = 1;
        }

        if (!cont){
            // test if origin is outside (v3, v0, v2) - set v1 as v3 and
            // continue
            ferVec3Cross(&va, &ferSimplexPoint(portal, 3)->v,
                              &ferSimplexPoint(portal, 2)->v);
            dot = ferVec3Dot(&va, &ferSimplexPoint(portal, 0)->v);
            if (dot < FER_ZERO && !ferIsZero(dot)){
                ferSimplexSet(portal, 1, ferSimplexPoint(portal, 3));
                cont = 1;
            }
        }

        if (cont){
            ferVec3Sub2(&va, &ferSimplexPoint(portal, 1)->v,
                             &ferSimplexPoint(portal, 0)->v);
            ferVec3Sub2(&vb, &ferSimplexPoint(portal, 2)->v,
                             &ferSimplexPoint(portal, 0)->v);
            ferVec3Cross(&dir, &va, &vb);
            ferVec3Normalize(&dir);
        }else{
            ferSimplexSetSize(portal, 4);
        }
    }

    return 0;
}

static int refinePortal(const void *obj1, const void *obj2,
                        const fer_t *ccd, fer_simplex_t *portal)
{
    fer_vec3_t dir;
    fer_support_t v4;

    while (1){
        // compute direction outside the portal (from v0 throught v1,v2,v3
        // face)
        portalDir(portal, &dir);

        // test if origin is inside the portal
        if (portalEncapsulesOrigin(portal, &dir))
            return 0;

        // get next support point
        __ferSupport(obj1, obj2, &dir, ccd, &v4);

        // test if v4 can expand portal to contain origin and if portal
        // expanding doesn't reach given tolerance
        if (!portalCanEncapsuleOrigin(portal, &v4, &dir)
                || portalReachTolerance(portal, &v4, &dir, ccd)){
            return -1;
        }

        // v1-v2-v3 triangle must be rearranged to face outside Minkowski
        // difference (direction from v0).
        expandPortal(portal, &v4);
    }

    return -1;
}


static void findPenetr(const void *obj1, const void *obj2, const fer_t *ccd,
                       fer_simplex_t *portal,
                       fer_real_t *depth, fer_vec3_t *pdir, fer_vec3_t *pos)
{
    fer_vec3_t dir;
    fer_support_t v4;
    unsigned long iterations;

    iterations = 0UL;
    while (1){
        // compute portal direction and obtain next support point
        portalDir(portal, &dir);
        __ferSupport(obj1, obj2, &dir, ccd, &v4);

        // reached tolerance -> find penetration info
        if (portalReachTolerance(portal, &v4, &dir, ccd)
                || iterations > ccd->max_iterations){
            *depth = ferVec3PointTriDist2(fer_vec3_origin,
                                          &ferSimplexPoint(portal, 1)->v,
                                          &ferSimplexPoint(portal, 2)->v,
                                          &ferSimplexPoint(portal, 3)->v,
                                          pdir);
            *depth = FER_SQRT(*depth);
            ferVec3Normalize(pdir);

            // barycentric coordinates:
            findPos(obj1, obj2, ccd, portal, pos);

            return;
        }

        expandPortal(portal, &v4);

        iterations++;
    }
}

static void findPenetrTouch(const void *obj1, const void *obj2, const fer_t *ccd,
                            fer_simplex_t *portal,
                            fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos)
{
    // Touching contact on portal's v1 - so depth is zero and direction
    // is unimportant and pos can be guessed
    *depth = FER_REAL(0.);
    ferVec3Copy(dir, fer_vec3_origin);

    ferVec3Copy(pos, &ferSimplexPoint(portal, 1)->v1);
    ferVec3Add(pos, &ferSimplexPoint(portal, 1)->v2);
    ferVec3Scale(pos, 0.5);
}

static void findPenetrSegment(const void *obj1, const void *obj2, const fer_t *ccd,
                              fer_simplex_t *portal,
                              fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos)
{
    /*
    fer_vec3_t vec;
    fer_real_t k;
    */

    // Origin lies on v0-v1 segment.
    // Depth is distance to v1, direction also and position must be
    // computed

    ferVec3Copy(pos, &ferSimplexPoint(portal, 1)->v1);
    ferVec3Add(pos, &ferSimplexPoint(portal, 1)->v2);
    ferVec3Scale(pos, FER_REAL(0.5));

    /*
    ferVec3Sub2(&vec, &ferSimplexPoint(portal, 1)->v,
                      &ferSimplexPoint(portal, 0)->v);
    k  = FER_SQRT(ferVec3Len2(&ferSimplexPoint(portal, 0)->v));
    k /= FER_SQRT(ferVec3Len2(&vec));
    ferVec3Scale(&vec, -k);
    ferVec3Add(pos, &vec);
    */

    ferVec3Copy(dir, &ferSimplexPoint(portal, 1)->v);
    *depth = FER_SQRT(ferVec3Len2(dir));
    ferVec3Normalize(dir);
}


static void findPos(const void *obj1, const void *obj2, const fer_t *ccd,
                    const fer_simplex_t *portal, fer_vec3_t *pos)
{
    fer_vec3_t dir;
    size_t i;
    fer_real_t b[4], sum, inv;
    fer_vec3_t vec, p1, p2;

    portalDir(portal, &dir);

    // use barycentric coordinates of tetrahedron to find origin
    ferVec3Cross(&vec, &ferSimplexPoint(portal, 1)->v,
                       &ferSimplexPoint(portal, 2)->v);
    b[0] = ferVec3Dot(&vec, &ferSimplexPoint(portal, 3)->v);

    ferVec3Cross(&vec, &ferSimplexPoint(portal, 3)->v,
                       &ferSimplexPoint(portal, 2)->v);
    b[1] = ferVec3Dot(&vec, &ferSimplexPoint(portal, 0)->v);

    ferVec3Cross(&vec, &ferSimplexPoint(portal, 0)->v,
                       &ferSimplexPoint(portal, 1)->v);
    b[2] = ferVec3Dot(&vec, &ferSimplexPoint(portal, 3)->v);

    ferVec3Cross(&vec, &ferSimplexPoint(portal, 2)->v,
                       &ferSimplexPoint(portal, 1)->v);
    b[3] = ferVec3Dot(&vec, &ferSimplexPoint(portal, 0)->v);

	sum = b[0] + b[1] + b[2] + b[3];

    if (ferIsZero(sum) || sum < FER_ZERO){
		b[0] = FER_REAL(0.);

        ferVec3Cross(&vec, &ferSimplexPoint(portal, 2)->v,
                           &ferSimplexPoint(portal, 3)->v);
        b[1] = ferVec3Dot(&vec, &dir);
        ferVec3Cross(&vec, &ferSimplexPoint(portal, 3)->v,
                           &ferSimplexPoint(portal, 1)->v);
        b[2] = ferVec3Dot(&vec, &dir);
        ferVec3Cross(&vec, &ferSimplexPoint(portal, 1)->v,
                           &ferSimplexPoint(portal, 2)->v);
        b[3] = ferVec3Dot(&vec, &dir);

		sum = b[1] + b[2] + b[3];
	}

	inv = FER_REAL(1.) / sum;

    ferVec3Copy(&p1, fer_vec3_origin);
    ferVec3Copy(&p2, fer_vec3_origin);
    for (i = 0; i < 4; i++){
        ferVec3Copy(&vec, &ferSimplexPoint(portal, i)->v1);
        ferVec3Scale(&vec, b[i]);
        ferVec3Add(&p1, &vec);

        ferVec3Copy(&vec, &ferSimplexPoint(portal, i)->v2);
        ferVec3Scale(&vec, b[i]);
        ferVec3Add(&p2, &vec);
    }
    ferVec3Scale(&p1, inv);
    ferVec3Scale(&p2, inv);

    ferVec3Copy(pos, &p1);
    ferVec3Add(pos, &p2);
    ferVec3Scale(pos, 0.5);
}

_fer_inline void expandPortal(fer_simplex_t *portal,
                              const fer_support_t *v4)
{
    fer_real_t dot;
    fer_vec3_t v4v0;

    ferVec3Cross(&v4v0, &v4->v, &ferSimplexPoint(portal, 0)->v);
    dot = ferVec3Dot(&ferSimplexPoint(portal, 1)->v, &v4v0);
    if (dot > FER_ZERO){
        dot = ferVec3Dot(&ferSimplexPoint(portal, 2)->v, &v4v0);
        if (dot > FER_ZERO){
            ferSimplexSet(portal, 1, v4);
        }else{
            ferSimplexSet(portal, 3, v4);
        }
    }else{
        dot = ferVec3Dot(&ferSimplexPoint(portal, 3)->v, &v4v0);
        if (dot > FER_ZERO){
            ferSimplexSet(portal, 2, v4);
        }else{
            ferSimplexSet(portal, 1, v4);
        }
    }
}

_fer_inline void portalDir(const fer_simplex_t *portal, fer_vec3_t *dir)
{
    fer_vec3_t v2v1, v3v1;

    ferVec3Sub2(&v2v1, &ferSimplexPoint(portal, 2)->v,
                       &ferSimplexPoint(portal, 1)->v);
    ferVec3Sub2(&v3v1, &ferSimplexPoint(portal, 3)->v,
                       &ferSimplexPoint(portal, 1)->v);
    ferVec3Cross(dir, &v2v1, &v3v1);
    ferVec3Normalize(dir);
}

_fer_inline int portalEncapsulesOrigin(const fer_simplex_t *portal,
                                       const fer_vec3_t *dir)
{
    fer_real_t dot;
    dot = ferVec3Dot(dir, &ferSimplexPoint(portal, 1)->v);
    return ferIsZero(dot) || dot > FER_ZERO;
}

_fer_inline int portalReachTolerance(const fer_simplex_t *portal,
                                     const fer_support_t *v4,
                                     const fer_vec3_t *dir,
                                     const fer_t *ccd)
{
    fer_real_t dv1, dv2, dv3, dv4;
    fer_real_t dot1, dot2, dot3;

    // find the smallest dot product of dir and {v1-v4, v2-v4, v3-v4}

    dv1 = ferVec3Dot(&ferSimplexPoint(portal, 1)->v, dir);
    dv2 = ferVec3Dot(&ferSimplexPoint(portal, 2)->v, dir);
    dv3 = ferVec3Dot(&ferSimplexPoint(portal, 3)->v, dir);
    dv4 = ferVec3Dot(&v4->v, dir);

    dot1 = dv4 - dv1;
    dot2 = dv4 - dv2;
    dot3 = dv4 - dv3;

    dot1 = FER_FMIN(dot1, dot2);
    dot1 = FER_FMIN(dot1, dot3);

    return ferEq(dot1, ccd->mpr_tolerance) || dot1 < ccd->mpr_tolerance;
}

_fer_inline int portalCanEncapsuleOrigin(const fer_simplex_t *portal,   
                                         const fer_support_t *v4,
                                         const fer_vec3_t *dir)
{
    fer_real_t dot;
    dot = ferVec3Dot(&v4->v, dir);
    return ferIsZero(dot) || dot > FER_ZERO;
}
