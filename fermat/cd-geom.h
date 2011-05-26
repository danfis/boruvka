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

    fer_list_t list;       /*!< Reference to list of all geoms */
    fer_list_t list_dirty; /*!< Reference to list of dirty geoms */

    void *sap;
} fer_aligned(16) fer_packed;
typedef struct _fer_cd_geom_t fer_cd_geom_t;

/**
 * Creates new geom
 */
fer_cd_geom_t *ferCDGeomNew(fer_cd_t *cd);

/**
 * Deletes geom
 */
void ferCDGeomDel(fer_cd_t *cd, fer_cd_geom_t *g);

/**
 * Builds OBB hierarchy.
 */
void ferCDGeomBuild(fer_cd_t *cd, fer_cd_geom_t *g);

/**
 * Sets translation.
 */
_fer_inline void ferCDGeomSetTr(fer_cd_t *cd, fer_cd_geom_t *g,
                                const fer_vec3_t *tr);

/**
 * Sets translation using x, y, z coordinates.
 */
_fer_inline void ferCDGeomSetTr3(fer_cd_t *cd, fer_cd_geom_t *g,
                                 fer_real_t x, fer_real_t y, fer_real_t z);

/**
 * Sets rotation.
 */
_fer_inline void ferCDGeomSetRot(fer_cd_t *cd, fer_cd_geom_t *g,
                                 const fer_mat3_t *rot);

/**
 * Sets rotation using euler angles (x, y, z).
 */
_fer_inline void ferCDGeomSetRotEuler(fer_cd_t *cd, fer_cd_geom_t *g,
                                      fer_real_t xrot, fer_real_t yrot,
                                      fer_real_t zrot);

/**
 * Returns true if given geoms do collide.
 */
int ferCDGeomCollide(fer_cd_t *cd,
                     const fer_cd_geom_t *g1, const fer_cd_geom_t *g2);



/**
 * Adds sphere to geom.
 */
void ferCDGeomAddSphere(fer_cd_t *cd, fer_cd_geom_t *g, fer_real_t radius);

/**
 * Adds sphere to geom with specified offset.
 */
void ferCDGeomAddSphere2(fer_cd_t *cd, fer_cd_geom_t *g, fer_real_t radius,
                         const fer_vec3_t *tr);

/**
 * Adds box to geom.
 */
void ferCDGeomAddBox(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t lx, fer_real_t ly, fer_real_t lz);

/**
 * Adds box with offset
 */
void ferCDGeomAddBox2(fer_cd_t *cd, fer_cd_geom_t *g,
                      fer_real_t lx, fer_real_t ly, fer_real_t lz,
                      const fer_mat3_t *rot, const fer_vec3_t *tr);

/**
 * Adds cylinder to geom.
 */
void ferCDGeomAddCyl(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t radius, fer_real_t height);

/**
 * Adds cylinder with offset
 */
void ferCDGeomAddCyl2(fer_cd_t *cd, fer_cd_geom_t *g,
                      fer_real_t radius, fer_real_t height,
                      const fer_mat3_t *rot, const fer_vec3_t *tr);

/**
 * Adds capsule to geom.
 */
void ferCDGeomAddCap(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t radius, fer_real_t height);

/**
 * Adds capsule with offset
 */
void ferCDGeomAddCap2(fer_cd_t *cd, fer_cd_geom_t *g,
                      fer_real_t radius, fer_real_t height,
                      const fer_mat3_t *rot, const fer_vec3_t *tr);
/**
 * Adds plane to geom.
 */
void ferCDGeomAddPlane(fer_cd_t *cd, fer_cd_geom_t *g);

/**
 * Adds transformed plane
 */
void ferCDGeomAddPlane2(fer_cd_t *cd, fer_cd_geom_t *g,
                        const fer_mat3_t *rot, const fer_vec3_t *tr);

/**
 * Adds triangle ti geom.
 */
void ferCDGeomAddTri(fer_cd_t *cd, fer_cd_geom_t *g,
                     const fer_vec3_t *p0, const fer_vec3_t *p1,
                     const fer_vec3_t *p2);

/**
 * Adds triangular mesh to geom.
 */
void ferCDGeomAddTriMesh(fer_cd_t *cd, fer_cd_geom_t *g,
                         const fer_vec3_t *pts,
                         const unsigned int *ids, size_t num_tris);

/**
 * Adds triangular mesh with offset.
 */
void ferCDGeomAddTriMesh2(fer_cd_t *cd, fer_cd_geom_t *g,
                          const fer_vec3_t *pts,
                          const unsigned int *ids, size_t num_tris,
                          const fer_mat3_t *rot, const fer_vec3_t *tr);

/**
 * Adds triangules from file in "raw" format to geom.
 */
void ferCDGeomAddTrisFromRaw(fer_cd_t *cd, fer_cd_geom_t *g,
                             const char *filename);

/**
 * Similar to ferCDGeomAddTrisFromRaw() but triangles are scaled by given
 * factor.
 */
void ferCDGeomAddTrisFromRawScale(fer_cd_t *cd, fer_cd_geom_t *g,
                                  const char *filename, fer_real_t scale);


/**
 * Saves geom in Lisp-like format.
 * See ferCDGeomLoad().
 */
int ferCDGeomSave(fer_cd_t *cd, const fer_cd_geom_t *g,
                  const char *filename);

/**
 * Loads geom from given file.
 * See ferCDGeomSave().
 */
fer_cd_geom_t *ferCDGeomLoad(fer_cd_t *cd, const char *filename);

/**
 * Set geom as dirty
 */
void ferCDGeomSetDirty(fer_cd_t *cd, fer_cd_geom_t *g);

/**
 * Returns true if geom is dirty
 */
_fer_inline int ferCDGeomDirty(const fer_cd_t *cd, const fer_cd_geom_t *g);

void ferCDGeomDumpSVT(const fer_cd_geom_t *g, FILE *out, const char *name);
void ferCDGeomDumpOBBSVT(const fer_cd_geom_t *g, FILE *out, const char *name);
void ferCDGeomDumpTriSVT(const fer_cd_geom_t *g, FILE *out, const char *name);


/** Sets min/max values along given axis */
void __ferCDGeomSetMinMax(const fer_cd_geom_t *g,
                          const fer_vec3_t *axis,
                          fer_real_t *min, fer_real_t *max);

/** Reset geom as non-dirty */
void __ferCDGeomResetDirty(fer_cd_t *cd, fer_cd_geom_t *g);

/**** INLINES ****/
_fer_inline void ferCDGeomSetTr(fer_cd_t *cd, fer_cd_geom_t *g,
                                const fer_vec3_t *tr)
{
    ferVec3Copy(&g->tr, tr);
    ferCDGeomSetDirty(cd, g);
}

_fer_inline void ferCDGeomSetTr3(fer_cd_t *cd, fer_cd_geom_t *g,
                                 fer_real_t x, fer_real_t y, fer_real_t z)
{
    ferVec3Set(&g->tr, x, y, z);
    ferCDGeomSetDirty(cd, g);
}

_fer_inline void ferCDGeomSetRot(fer_cd_t *cd, fer_cd_geom_t *g,
                                 const fer_mat3_t *rot)
{
    ferMat3Copy(&g->rot, rot);
    ferCDGeomSetDirty(cd, g);
}

_fer_inline void ferCDGeomSetRotEuler(fer_cd_t *cd, fer_cd_geom_t *g,
                                      fer_real_t xrot, fer_real_t yrot,
                                      fer_real_t zrot)
{
    ferMat3SetRot3D(&g->rot, xrot, yrot, zrot);
    ferCDGeomSetDirty(cd, g);
}

_fer_inline int ferCDGeomDirty(const fer_cd_t *cd, const fer_cd_geom_t *g)
{
    return !ferListEmpty(&g->list_dirty);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_GEOM_H__ */

