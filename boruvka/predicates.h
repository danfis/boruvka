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


#ifndef __FER_PREDICATES_H__
#define __FER_PREDICATES_H__

#include <boruvka/core.h>
#include <boruvka/vec2.h>
#include <boruvka/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Predicates
 * ===========
 *
 * This is port of Jonathan Richard Shewchuk's robust predicates to Boruvka
 * library. See http://www.cs.cmu.edu/~quake/robust.html for papers,
 * original code and other informations.
 *
 * Only API of original code was changed to conform with rest of Boruvka
 * API. Predicates available are:
 *
 * 1) ferPredOrient2d() and ferPredOrient2dFast() find out relative
 *    orientation of three 2d points (counterclockwise, clockwise,
 *    collinear).
 *
 * 2) ferPredOrient3d() and ferPredOrient3dFast() find out relative
 *    position of point to plane (defined by triplet of points).
 *
 * 3) ferPredInCircle() and ferPredInCircleFast() find out if given point
 *    is inside, outside or lies on circle defined by triplet of points.
 *
 * 4) ferPredInSphere() and ferPredInSphereFast() find out if given point
 *    lies inside, outside or is cospherical with sphere defined by four
 *    points.
 *
 * The former version of function is always the robus and exact one and the
 * latter one is approximate and nonrobust (but fast) version.
 */

struct _bor_pred_t {
    bor_real_t splitter; /* = 2^ceiling(p / 2) + 1.  Used to split floats in half. */
    bor_real_t epsilon;  /* = 2^(-p). Used to estimate roundoff errors. */

    /* A set of coefficients used to calculate maximum roundoff errors. */
    bor_real_t resulterrbound;
    bor_real_t ccwerrboundA, ccwerrboundB, ccwerrboundC;
    bor_real_t o3derrboundA, o3derrboundB, o3derrboundC;
    bor_real_t iccerrboundA, iccerrboundB, iccerrboundC;
    bor_real_t isperrboundA, isperrboundB, isperrboundC;
};
typedef struct _bor_pred_t bor_pred_t;


/**
 * Functions
 * ----------
 */

/**
 * Initialize the variables used for exact arithmetic.
 * Call this once before calling any of exact versions of functions.
 */
void ferPredInit(bor_pred_t *pred);


/**
 * Return a positive value if the points pa, pb, and pc occur
 * in counterclockwise order; a negative value if they occur
 * in clockwise order; and zero if they are collinear.  The
 * result is also a rough approximation of twice the signed
 * area of the triangle defined by the three points.
 */
bor_real_t ferPredOrient2dFast(const bor_pred_t *pred,
                               const bor_vec2_t *pa,
                               const bor_vec2_t *pb,
                               const bor_vec2_t *pc);

/**
 * This is robust and exact version of ferPredOrient2dFast().
 */
bor_real_t ferPredOrient2d(const bor_pred_t *pred,
                           const bor_vec2_t *pa,
                           const bor_vec2_t *pb,
                           const bor_vec2_t *pc);


/**
 * Return a positive value if the point pd lies below the
 * plane passing through pa, pb, and pc; "below" is defined so
 * that pa, pb, and pc appear in counterclockwise order when
 * viewed from above the plane.  Returns a negative value if
 * pd lies above the plane.  Returns zero if the points are
 * coplanar.  The result is also a rough approximation of six
 * times the signed volume of the tetrahedron defined by the
 * four points.
 */
bor_real_t ferPredOrient3dFast(const bor_pred_t *pred,
                               const bor_vec3_t *pa,
                               const bor_vec3_t *pb,
                               const bor_vec3_t *pc,
                               const bor_vec3_t *pd);

/**
 * This is robust and exact version of ferPredOrient3dFast().
 */
bor_real_t ferPredOrient3d(const bor_pred_t *pred,
                           const bor_vec3_t *pa,
                           const bor_vec3_t *pb,
                           const bor_vec3_t *pc,
                           const bor_vec3_t *pd);


/**
 * Return a positive value if the point pd lies inside the
 * circle passing through pa, pb, and pc; a negative value if
 * it lies outside; and zero if the four points are cocircular.
 * The points pa, pb, and pc must be in counterclockwise
 * order, or the sign of the result will be reversed.
 */
bor_real_t ferPredInCircleFast(const bor_pred_t *pred,
                               const bor_vec2_t *pa,
                               const bor_vec2_t *pb,
                               const bor_vec2_t *pc,
                               const bor_vec2_t *pd);

/**
 * This is robust and exact version of ferPredInCircleFast().
 */
bor_real_t ferPredInCircle(const bor_pred_t *pred,
                           const bor_vec2_t *pa,
                           const bor_vec2_t *pb,
                           const bor_vec2_t *pc,
                           const bor_vec2_t *pd);

/**
 * Return a positive value if the point pe lies inside the
 * sphere passing through pa, pb, pc, and pd; a negative value
 * if it lies outside; and zero if the five points are
 * cospherical.  The points pa, pb, pc, and pd must be ordered
 * so that they have a positive orientation (as defined by
 * orient3d()), or the sign of the result will be reversed.
 */
bor_real_t ferPredInSphereFast(const bor_pred_t *pred,
                               const bor_vec3_t *pa,
                               const bor_vec3_t *pb,
                               const bor_vec3_t *pc,
                               const bor_vec3_t *pd,
                               const bor_vec3_t *pe);

/**
 * This is robust and exact version of ferPredInSphereFast().
 */
bor_real_t ferPredInSphere(const bor_pred_t *pred,
                           const bor_vec3_t *pa,
                           const bor_vec3_t *pb,
                           const bor_vec3_t *pc,
                           const bor_vec3_t *pd,
                           const bor_vec3_t *pe);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_PREDICATES_H__ */
