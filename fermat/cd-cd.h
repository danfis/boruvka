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
 * First thing that must be done in order to start using FermatCD library
 * is to create instance of {fer_cd_t} structure. This is done by a function
 * {ferCDNew()} which requires as parameter parameters already set up:
 * ~~~~
 * fer_cd_params_t params;
 * fer_cd_t *cd;
 *
 * // Init parameters
 * ferCDParamsInit(&params);
 * // and alter them if you need
 * params.max_contacts = 30;
 *
 * // And now create an instance
 * cd = ferCDNew(&params);
 * ~~~~
 *
 * At the and of the work all allocated memory should be freed by function
 * {ferCDDel()}. This function frees all memory including all geometry
 * objects created since.
 */
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

/** vvvv */
/** Callback function for ferCDCollide(). */
typedef int (*fer_cd_collide_cb)(const fer_cd_t *cd,
                                 const struct _fer_cd_geom_t *g1,
                                 const struct _fer_cd_geom_t *g2,
                                 void *data);
/** ^^^^ */

/**
 * Function returns true if any colliding pair was found, false otherwise.
 * Also for each colliding pair is called callback {cb} is non-NULL. If
 * this callback returns -1 ferCDCollide() is terminated prematurely.
 */
int ferCDCollide(fer_cd_t *cd, fer_cd_collide_cb cb, void *data);

/** vvvv */
/** Callback function for ferCDSeparate() */
typedef int (*fer_cd_separate_cb)(const fer_cd_t *cd,
                                  const struct _fer_cd_geom_t *g1,
                                  const struct _fer_cd_geom_t *g2,
                                  const fer_cd_contacts_t *contacts,
                                  void *data);
/** ^^^^ */

/**
 * For each colliding pair is called callback {cb}.
 * If this callback returns -1 ferCDSeparate() is terminated prematurely.
 */
void ferCDSeparate(fer_cd_t *cd, fer_cd_separate_cb cb, void *data);


/** vvvv */
/** Callback function for ferCDEachGeom(). */
typedef void (*fer_cd_each_geom_cb)(fer_cd_t *cd, struct _fer_cd_geom_t *g,
                                   void *data);
/** ^^^^ */

/**
 * Calls {cb} for each geom.
 */
void ferCDEachGeom(fer_cd_t *cd, fer_cd_each_geom_cb cb, void *data);

/**
 * Returns number of geoms
 */
_fer_inline size_t ferCDNumGeoms(const fer_cd_t *cd);

/**
 * Sets collider between shape1 and shape2 (in this order).
 * *Use this function only if you know what are you doing.*
 */
void ferCDSetCollideFn(fer_cd_t *cd, int shape1, int shape2,
                       fer_cd_collide_fn collider);

/**
 * Sets separater between shape1 and shape2 (in this order).
 * *Use this function only if you know what are you doing.*
 */
void ferCDSetSeparateFn(fer_cd_t *cd, int shape1, int shape2,
                        fer_cd_separate_fn sep);



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
