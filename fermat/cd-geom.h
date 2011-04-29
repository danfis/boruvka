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

#ifndef __FER_CD_GEOM_H__
#define __FER_CD_GEOM_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * CD - Geom - Geometry Objects
 * =============================
 */
struct _fer_cd_geom_t {
    fer_vec3_t tr;   /*!< Translation */
    fer_mat3_t rot;  /*!< Rotation */
    fer_list_t obbs; /*!< List of OBBs */
} fer_aligned(16) fer_packed;
typedef struct _fer_cd_geom_t fer_cd_geom_t;

/**
 * Creates new geom
 */
fer_cd_geom_t *ferCDGeomNew(void);

/**
 * Deletes geom
 */
void ferCDGeomDel(fer_cd_geom_t *g);

/**
 * Builds OBB hierarchy.
 */
void ferCDGeomBuild(fer_cd_geom_t *g);

/**
 * Sets translation.
 */
_fer_inline void ferCDGeomSetTr(fer_cd_geom_t *g, const fer_vec3_t *tr);

/**
 * Sets rotation.
 */
_fer_inline void ferCDGeomSetRot(fer_cd_geom_t *g, const fer_mat3_t *rot);

/**
 * Returns true if given geoms do collide.
 */
int ferCDGeomCollide(const fer_cd_geom_t *g1, const fer_cd_geom_t *g2);



/**
 * Adds triangular mesh to geom.
 */
void ferCDGeomAddTriMesh(fer_cd_geom_t *g, const fer_vec3_t *pts,
                         const unsigned int *ids, size_t len);

/**
 * Adds sphere to geom.
 */
void ferCDGeomAddSphere(fer_cd_geom_t *g, const fer_vec3_t *center,
                        fer_real_t radius);

/**
 * Adds box to geom.
 */
void ferCDGeomAddBox(fer_cd_geom_t *g,
                     fer_real_t lx, fer_real_t ly, fer_real_t lz,
                     const fer_vec3_t *center, const fer_mat3_t *rot);



void ferCDGeomDumpSVT(const fer_cd_geom_t *g, FILE *out, const char *name);


/**** INLINES ****/
_fer_inline void ferCDGeomSetTr(fer_cd_geom_t *g, const fer_vec3_t *tr)
{
    ferVec3Copy(&g->tr, tr);
}

_fer_inline void ferCDGeomSetRot(fer_cd_geom_t *g, const fer_mat3_t *rot)
{
    ferMat3Copy(&g->rot, rot);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_GEOM_H__ */

