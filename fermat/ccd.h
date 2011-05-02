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

#ifndef __FER_H__
#define __FER_H__

#include <fermat/vec3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Type of *support* function that takes pointer to 3D object and direction
 * and returns (via vec argument) furthest point from object in specified
 * direction.
 */
typedef void (*fer_support_fn)(const void *obj, const fer_vec3_t *dir,
                               fer_vec3_t *vec);

/**
 * Returns (via dir argument) first direction vector that will be used in
 * initialization of algorithm.
 */
typedef void (*fer_first_dir_fn)(const void *obj1, const void *obj2,
                                 fer_vec3_t *dir);


/**
 * Returns (via center argument) geometric center (some point near center)
 * of given object.
 */
typedef void (*fer_center_fn)(const void *obj1, fer_vec3_t *center);

/**
 * Main structure of CCD algorithm.
 */
struct _fer_t {
    fer_first_dir_fn first_dir; //!< Returns initial direction where first
                                //!< support point will be searched
    fer_support_fn support1; //!< Function that returns support point of
                             //!< first object
    fer_support_fn support2; //!< Function that returns support point of
                             //!< second object

    fer_center_fn center1; //!< Function that returns geometric center of
                           //!< first object
    fer_center_fn center2; //!< Function that returns geometric center of
                           //!< second object

    unsigned long max_iterations; //!< Maximal number of iterations
    fer_real_t epa_tolerance;
    fer_real_t mpr_tolerance; //!< Boundary tolerance for MPR algorithm
};
typedef struct _fer_t fer_t;

/**
 * Default first direction.
 */
void ferFirstDirDefault(const void *o1, const void *o2, fer_vec3_t *dir);

#define FER_INIT(ccd) \
    do { \
        (ccd)->first_dir = ferFirstDirDefault; \
        (ccd)->support1 = NULL; \
        (ccd)->support2 = NULL; \
        (ccd)->center1  = NULL; \
        (ccd)->center2  = NULL; \
        \
        (ccd)->max_iterations = (unsigned long)-1; \
        (ccd)->epa_tolerance = FER_REAL(0.0001); \
        (ccd)->mpr_tolerance = FER_REAL(0.0001); \
    } while(0)


/**
 * Returns true if two given objects interest.
 */
int ferGJKIntersect(const void *obj1, const void *obj2, const fer_t *ccd);

/**
 * This function computes separation vector of two objects. Separation
 * vector is minimal translation of obj2 to get obj1 and obj2 speparated
 * (without intersection).
 * Returns 0 if obj1 and obj2 intersect and sep is filled with translation
 * vector. If obj1 and obj2 don't intersect -1 is returned.
 */
int ferGJKSeparate(const void *obj1, const void *obj2, const fer_t *ccd,
                   fer_vec3_t *sep);

/**
 * Computes penetration of obj2 into obj1.
 * Depth of penetration, direction and position is returned. It means that
 * if obj2 is translated by distance depth in direction dir objects will
 * have touching contact, pos should be position in global coordinates
 * where force should take a place.
 *
 * CCD+EPA algorithm is used.
 *
 * Returns 0 if obj1 and obj2 intersect and depth, dir and pos are filled
 * if given non-NULL pointers.
 * If obj1 and obj2 don't intersect -1 is returned.
 */
int ferGJKPenetration(const void *obj1, const void *obj2, const fer_t *ccd,
                      fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);


/**
 * Returns true if two given objects intersect - MPR algorithm is used.
 */
int ferMPRIntersect(const void *obj1, const void *obj2, const fer_t *ccd);

/**
 * Computes penetration of obj2 into obj1.
 * Depth of penetration, direction and position is returned, i.e. if obj2
 * is translated by computed depth in resulting direction obj1 and obj2
 * would have touching contact. Position is point in global coordinates
 * where force should be take a place.
 *
 * Minkowski Portal Refinement algorithm is used (MPR, a.k.a. XenoCollide,
 * see Game Programming Gem 7).
 *
 * Returns 0 if obj1 and obj2 intersect, otherwise -1 is returned.
 */
int ferMPRPenetration(const void *obj1, const void *obj2, const fer_t *ccd,
                      fer_real_t *depth, fer_vec3_t *dir, fer_vec3_t *pos);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_H__ */
