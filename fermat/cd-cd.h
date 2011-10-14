/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_CD_CD_H__
#define __FER_CD_CD_H__

#ifdef FER_CD_TIME_MEASURE
# include <fermat/timer.h>
#endif /* FER_CD_TIME_MEASURE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_geom_t;
struct _fer_cd_sap_t;
struct _fer_cd_cp_t;

/**
 * Collision Detection
 * ====================
 *
 * See fer_cd_t.
 */

struct _fer_cd_params_t {
    uint32_t build_flags; /*!< Flags that alter building methods.
                               Available flags are:
                                 - FER_CD_TOP_DOWN, FER_CD_BOTTOM_UP
                                 - FER_CD_FIT_COVARIANCE,
                                   FER_CD_FIT_CALIPERS,
                                   FER_CD_FIT_CALIPERS_NUM_ROT(),
                                   FER_CD_FIT_POLYHEDRAL_MASS,
                                   FER_CD_FIT_NAIVE,
                                   FER_CD_FIT_NAIVE_NUM_ROT(),
                                   FER_CD_FIT_COVARIANCE_FAST
                                 - FER_CD_BUILD_PARALLEL()
                               Default: FER_CD_TOP_DOWN
                                            | FER_CD_FIT_COVARIANCE */

    int use_sap; /*!< True if SAP should be used. Default: true */
    int sap_gpu; /*!< True if SAP should run on GPU. Default: false */
    size_t sap_hashsize; /*!< Size of SAP's hash table.
                              Set it to reasonable high number (consider
                              prime number). Default: 1023 */

    size_t max_contacts; /*!< Maximal number of contacts.
                              Default: 20 */
    size_t num_threads; /*!< Number of threads used in parallel versions of
                             some functions. If set >1 then parallelization
                             usign threads will be used whenever possible
                             (if not documented otherwise).
                             Default: 1 */

    unsigned long mpr_max_iterations; /*!< Maximal number of iterations MPR
                                           algorithm can perform.
                                           Default: ULONG_MAX */
    fer_real_t mpr_tolerance;         /*!< Boundary tolerance of MPR
                                           algorithm.
                                           Default: 1E-4 */


    int use_cp;         /*!< If set to true, contact persistence is used.
                             Note that contact persistence is used only for
                             those geoms that were explicitly enabled by
                             [see ferCDGeomContactPersistence() function].
                             Default: true */
    size_t cp_hashsize; /*!< Size of hash table used as register for
                             contact persistence.  If set to 0,
                             {.sap_hashsize} is used instead.
                             Default: 0 */
    fer_real_t cp_max_dist; /*!< Maximal squared distance a contact point
                                 can move to be considered "still" */
};
typedef struct _fer_cd_params_t fer_cd_params_t;

/**
 * Initializes parameters to default values.
 */
void ferCDParamsInit(fer_cd_params_t *params);


struct _fer_cd_t {
    uint32_t build_flags;
    fer_cd_collide_fn collide[FER_CD_SHAPE_LEN][FER_CD_SHAPE_LEN];

    fer_tasks_t *tasks;   /*!< Pool of available threads */
    pthread_mutex_t lock; /*!< Global lock */

    size_t max_contacts;
    fer_cd_separate_fn separate[FER_CD_SHAPE_LEN][FER_CD_SHAPE_LEN];

    fer_cd_contacts_t **contacts;
    size_t contacts_len;

    fer_list_t geoms;          /*!< List of all geoms */
    size_t geoms_len;          /*!< Size of .geoms list */
    fer_list_t geoms_dirty;    /*!< List of dirty geoms */
    size_t geoms_dirty_len;    /*!< Size of .geoms_dirty list */

    unsigned long mpr_max_iterations;
    fer_real_t mpr_tolerance;

    struct _fer_cd_sap_t *sap; /*!< SAP solver */
    struct _fer_cd_cp_t *cp;   /*!< Struct for contact persistence */
#ifdef FER_CD_TIME_MEASURE
    fer_timer_t timer;
    fer_timer_t timer_all;
    fer_real_t time_radix[3];
    fer_real_t time_remove_pairs;
    fer_real_t time_find_pairs;
    fer_real_t time_update_dirty;
    fer_real_t time_separate;
#endif /* FER_CD_TIME_MEASURE */
};
typedef struct _fer_cd_t fer_cd_t;

/**
 * New instance of collision detection library.
 * If {params} is set to NULL, default parameters are used.
 */
fer_cd_t *ferCDNew(const fer_cd_params_t *params);

/**
 * Destructor.
 */
void ferCDDel(fer_cd_t *cd);

/**
 * Sets build flags.
 * These flags modifies building of OBB hierarchy.
 *
 * See macros:
 * - FER_CD_FIT_COVARIANCE
 * - FER_CD_FIT_CALIPERS
 * - FER_CD_FIT_CALIPERS_NUM_ROT()
 *
 * Default is FER_CD_COVARIANCE.
 */
_fer_inline void ferCDSetBuildFlags(fer_cd_t *cd, uint32_t flags);

/**
 * Sets collider between shape1 and shape2 (in this order).
 */
void ferCDSetCollideFn(fer_cd_t *cd, int shape1, int shape2,
                       fer_cd_collide_fn collider);

/**
 * Sets separater between shape1 and shape2 (in this order).
 */
void ferCDSetSeparateFn(fer_cd_t *cd, int shape1, int shape2,
                        fer_cd_separate_fn sep);

/**
 * Callback function for ferCDCollide().
 */
typedef int (*fer_cd_collide_cb)(const fer_cd_t *cd,
                                 const struct _fer_cd_geom_t *g1,
                                 const struct _fer_cd_geom_t *g2,
                                 void *data);

/**
 * Function returns true if any colliding pair was found, false otherwise.
 * Also for each colliding pair is called callback {cb} is non-NULL. If
 * this callback returns -1 ferCDCollide() is terminated prematurely.
 */
int ferCDCollide(fer_cd_t *cd, fer_cd_collide_cb cb, void *data);

/**
 * Callback function for ferCDSeparate()
 */
typedef int (*fer_cd_separate_cb)(const fer_cd_t *cd,
                                  const struct _fer_cd_geom_t *g1,
                                  const struct _fer_cd_geom_t *g2,
                                  const fer_cd_contacts_t *contacts,
                                  void *data);

/**
 * For each colliding pair is called callback {cb}.
 * If this callback returns -1 ferCDSeparate() is terminated prematurely.
 */
void ferCDSeparate(fer_cd_t *cd, fer_cd_separate_cb cb, void *data);


/**
 * Callback function for ferCDEachGeom().
 */
typedef void (*fer_cd_each_geom_cb)(fer_cd_t *cd, struct _fer_cd_geom_t *g,
                                   void *data);

/**
 * Calls {cb} for each geom.
 */
void ferCDEachGeom(fer_cd_t *cd, fer_cd_each_geom_cb cb, void *data);

/**
 * Returns number of geoms
 */
_fer_inline size_t ferCDNumGeoms(const fer_cd_t *cd);


void ferCDDumpSVT(const fer_cd_t *cd, FILE *out, const char *name);

/** Returns true if two given shapes do collide */
int __ferCDShapeCollide(fer_cd_t *cd,
                        const fer_cd_shape_t *s1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_shape_t *s2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/** Dispatch separator according to shape's type */
int __ferCDShapeSeparate(struct _fer_cd_t *cd,
                         const fer_cd_shape_t *s1,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_shape_t *s2,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                         fer_cd_contacts_t *con);

/**** INLINES ****/
_fer_inline void ferCDSetBuildFlags(fer_cd_t *cd, uint32_t flags)
{
    cd->build_flags = flags;
}

_fer_inline size_t ferCDNumGeoms(const fer_cd_t *cd)
{
    return cd->geoms_len;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CD_H__ */
