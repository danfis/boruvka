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

#ifndef __FER_CD_SPHERE_GRID_H__
#define __FER_CD_SPHERE_GRID_H__

#include <fermat/vec3.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * CD-Sphere-Grid
 * ===============
 *
 * See fer_cd_sphere_grid_t.
 *
 * Example:
 * ~~~~~
 * fer_vec3_t center;
 * fer_cd_sphere_grid_t *g;
 *
 * // Create empty sphere grid
 * g = ferCDSphereGridNew();
 *
 * // Add new sphere with center at (0, 1, 2) and radius 0.1
 * ferVec3Set(&center, 0, 1, 2);
 * ferCDSphereGridAdd(g, 0.1, &center);
 *
 * // Add another sphere
 * ferVec3Set(&center, 0, 1, 0);
 * ferCDSphereGridAdd(g, 0.3, &center);
 *
 * // Build grid - cell size will be determined automatically
 * ferCDSphereGridBuild(g, -1.);
 *
 * // Test sphere with center at (0.5, 0.5, 0.5) and radius 0.3 on
 * // collision.
 * ferVec3Set(&c, 0.5, 0.5, 0.5);
 * ret = ferCDSphereGridCollide(g, 0.3, &c);
 */

struct _fer_cd_sphere_grid_cell_t {
    fer_list_t spheres;
};
typedef struct _fer_cd_sphere_grid_cell_t fer_cd_sphere_grid_cell_t;

struct _fer_cd_sphere_grid_t {
    fer_vec3_t origin; /*!< Origin of grid in global coordinates */
    fer_real_t edge;   /*!< Length of cell's edge */
    size_t dim[3];     /*!< Number of cells along x, y, z axis */

    fer_cd_sphere_grid_cell_t *grid; /*!< Grid of cells */

    fer_list_t spheres; /*!< List of all spheres */
} fer_packed fer_aligned(16);
typedef struct _fer_cd_sphere_grid_t fer_cd_sphere_grid_t;


/**
 * Creates new sphere grid
 */
fer_cd_sphere_grid_t *ferCDSphereGridNew(void);

/**
 * Deletes sphere grid
 */
void ferCDSphereGridDel(fer_cd_sphere_grid_t *g);

/**
 * Adds sphere to grid
 */
void ferCDSphereGridAdd(fer_cd_sphere_grid_t *g,
                        fer_real_t radius, const fer_vec3_t *center);

/**
 * Builds grid for all spheres added by ferCDSphereGridAddSphere().
 */
void ferCDSphereGridBuild(fer_cd_sphere_grid_t *g, fer_real_t edge);

/**
 * Returns true if specified sphere collides with any sphere in grid
 */
int ferCDSphereGridCollide(const fer_cd_sphere_grid_t *g,
                           fer_real_t radius, const fer_vec3_t *center);

/**
 * Brute force method for collision detection
 */
int ferCDSphereGridCollideBruteForce(const fer_cd_sphere_grid_t *g,
                                     fer_real_t radius,
                                     const fer_vec3_t *center);



void ferCDSphereGridCollDumpSVT(const fer_cd_sphere_grid_t *g,
                                fer_real_t radius, const fer_vec3_t *center,
                                FILE *out, const char *name);
                                
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SPHERE_GRID_H__ */


