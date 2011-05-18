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

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_geom_t;

/**
 * Collision Detection
 * ====================
 */
struct _fer_cd_t {
    uint32_t build_flags;
    fer_cd_collide_fn collide[FER_CD_SHAPE_LEN][FER_CD_SHAPE_LEN];
    fer_list_t geoms; /*!< List of all geoms */
};
typedef struct _fer_cd_t fer_cd_t;

/**
 * New instance of collision detection library.
 */
fer_cd_t *ferCDNew(void);

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
 * Callback function for ferCDCollide().
 */
typedef void (*fer_cd_collide_cb)(const fer_cd_t *cd,
                                  const struct _fer_cd_geom_t *g1,
                                  const struct _fer_cd_geom_t *g2,
                                  void *data);

/**
 * For each colliding pair of geoms callback {cb} is called (if non-NULL).
 * Function returns true if any colliding pair was found, false otherwise.
 */
int ferCDCollide(fer_cd_t *cd, fer_cd_collide_cb cb, void *data);


void ferCDDumpSVT(const fer_cd_t *cd, FILE *out, const char *name);

/** Returns true if two given shapes do collide */
int __ferCDShapeCollide(fer_cd_t *cd,
                        const fer_cd_shape_t *s1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_shape_t *s2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**** INLINES ****/
_fer_inline void ferCDSetBuildFlags(fer_cd_t *cd, uint32_t flags)
{
    cd->build_flags = flags;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_CD_H__ */
