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

#ifndef __FER_CD_SHAPE_H__
#define __FER_CD_SHAPE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_cd_shape_t;

/**
 * Support function. Returns furthest point on shape in given direction.
 * It is assumed dir is unit vector.
 */
typedef void (*fer_cd_shape_support)(const struct _fer_cd_shape_t *shape,
                                     const fer_vec3_t *dir,
                                     fer_vec3_t *p);

/**
 * Returns axis and half extents of bounding box that tightly fit to shape.
 */
typedef void (*fer_cd_shape_fit_obb)(const struct _fer_cd_shape_t *shape,
                                     fer_vec3_t *center,
                                     fer_vec3_t *axis0,
                                     fer_vec3_t *axis1,
                                     fer_vec3_t *axis2,
                                     fer_vec3_t *half_extents,
                                     int flags);

/**
 * Updates given convex hull to cover the shape.
 * Returns 1 if shape is fully contained in convex hull.
 * Returns 0 if shape is not contained in convex hull but at least skelet
 * is conatined (and thus orientation can be obtained from convex hull).
 */
typedef int (*fer_cd_shape_update_chull)(const struct _fer_cd_shape_t *shape,
                                         fer_chull3_t *chull,
                                         const fer_mat3_t *rot,
                                         const fer_vec3_t *tr);

/**
 * Dump shape in SVT format
 */
typedef void (*fer_cd_shape_dump_svt)(const struct _fer_cd_shape_t *shape,
                                      FILE *out, const char *name,
                                      const fer_mat3_t *rot,
                                      const fer_vec3_t *tr);
/**
 * Shape type class.
 */
struct _fer_cd_shape_class_t {
    int type;
    fer_cd_shape_support support;
    fer_cd_shape_fit_obb fit_obb;
    fer_cd_shape_update_chull update_chull;
    fer_cd_shape_dump_svt dump_svt;
};
typedef struct _fer_cd_shape_class_t fer_cd_shape_class_t;


/**
 * Base shape struct.
 */
struct _fer_cd_shape_t {
    fer_cd_shape_class_t *cl;
};
typedef struct _fer_cd_shape_t fer_cd_shape_t;

/** Shape types */
#define FER_CD_SHAPE_SPHERE      0
#define FER_CD_SHAPE_BOX         1
#define FER_CD_SHAPE_CYL         2
#define FER_CD_SHAPE_CAP         3
#define FER_CD_SHAPE_TRIMESH_TRI 4
#define FER_CD_SHAPE_LEN         5

#define FER_CD_SHAPE_TRIMESH 20

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_SHAPE_H__ */

