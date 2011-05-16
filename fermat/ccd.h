/***
 * fermat
 * -------
 * Copyright (c)2010,2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#ifndef __FER_CCD_H__
#define __FER_CCD_H__

#include <fermat/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Type of *support* function that takes pointer to 3D object and direction
 * and returns (via {vec} argument) furthest point from object in specified
 * direction.
 */
typedef void (*fer_ccd_support_fn)(const void *obj, const fer_vec3_t *dir,
                                   fer_vec3_t *vec);

/**
 * Returns (via {dir} argument) first direction vector that will be used in
 * initialization of algorithm.
 */
typedef void (*fer_ccd_first_dir_fn)(const void *obj1, const void *obj2,
                                     fer_vec3_t *dir);


/**
 * Returns (via {center} argument) geometric center (or some point near center)
 * of given object.
 */
typedef void (*fer_ccd_center_fn)(const void *obj1, fer_vec3_t *center);

/**
 * Main structure of CCD algorithm.
 */
struct _fer_ccd_t {
    fer_ccd_first_dir_fn first_dir; /*!< Returns initial direction where
                                         first support point will be
                                         searched */
    fer_ccd_support_fn support1; /*!< Function that returns support point
                                      of first object */
    fer_ccd_support_fn support2; /*!< Function that returns support point
                                      of second object */

    fer_ccd_center_fn center1; /*!< Function that returns geometric center
                                    of first object */
    fer_ccd_center_fn center2; /*!< Function that returns geometric center
                                    of second object */

    unsigned long max_iterations; /*!< Maximal number of iterations */
    fer_real_t epa_tolerance;
    fer_real_t mpr_tolerance; /*!< Boundary tolerance for MPR algorithm */
};
typedef struct _fer_ccd_t fer_ccd_t;

/**
 * Default first direction.
 */
void ferCCDFirstDirDefault(const void *o1, const void *o2, fer_vec3_t *dir);

/**
 * Initializes ccd struct.
 * Only default {.first_dir} member is set to default one.
 */
void ferCCDInit(fer_ccd_t *ccd);

/**
 * Static initializer of {fer_ccd_t} struct.
 *
 * Example:
 * ~~~~~~~~~~
 * fer_ccd_t ccd = FER_CCD_INITIALIZER;
 */
#define FER_CCD_INITIALIZER \
    { .first_dir = ferCCDFirstDirDefault, \
      .support1 = NULL, \
      .support2 = NULL, \
      .center1  = NULL, \
      .center2  = NULL, \
      \
      .max_iterations = (unsigned long)-1, \
      .epa_tolerance = FER_REAL(0.0001), \
      .mpr_tolerance = FER_REAL(0.0001), \
    }


/**
 * Returns true if two given objects do collide.
 * *GJK* algorithm is used.
 */
int ferCCDGJKCollide(const fer_ccd_t *ccd, const void *obj1, const void *obj2);

/**
 * This function computes separation vector of two objects. Separation
 * vector is minimal translation of {obj2} to get {obj1} and {obj2}
 * separated (without intersection).
 * Returns 0 if {obj1} and {obj2} intersect and {sep} is filled with
 * separation vector. If {obj1} and {obj2} don't intersect -1 is returned.
 * *GJK* algorithm is used.
 */
int ferCCDGJKSeparate(const fer_ccd_t *ccd,
                      const void *obj1, const void *obj2,
                      fer_vec3_t *sep);

/**
 * Computes penetration of {obj2} into {obj1}.
 * Depth of penetration, direction and position is returned. It means that
 * if {obj2} is translated by distance {depth} in direction {dir} objects
 * will have touching contact, {pos} should be position in global
 * coordinates where force should take a place.
 *
 * *GJK+EPA* algorithm is used.
 *
 * Returns 0 if {obj1} and {obj2} intersect and {depth}, {dir} and {pos}
 * are filled if given non-NULL pointers.
 * If {obj1} and {obj2} don't intersect -1 is returned.
 */
int ferCCDGJKPenetration(const fer_ccd_t *ccd,
                         const void *obj1, const void *obj2,
                         fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);


/**
 * Returns true if two given objects collide.
 * *MPR* algorithm is used.
 */
int ferCCDMPRCollide(const fer_ccd_t *ccd, const void *obj1, const void *obj2);

/**
 * Function similar to ferCCDGJKPenetration but *MPR* algorithm is used.
 */
int ferCCDMPRPenetration(const fer_ccd_t *ccd,
                         const void *obj1, const void *obj2,
                         fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_CCD_H__ */
