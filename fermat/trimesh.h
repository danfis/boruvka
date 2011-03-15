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


#ifndef __FER_TRIMESH_H__
#define __FER_TRIMESH_H__

#include <fermat/vec3.h>
#include <fermat/quat.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_trimesh_t {
    fer_vec3_t pos; /*!< Position */
    fer_quat_t rot; /*!< Rotation */

    void *data;
};
typedef struct _fer_trimesh_t fer_trimesh_t;

/**
 * Creates new trimesh.
 */
fer_trimesh_t *ferTriMeshNew(void);

/**
 * Deletes previously created trimesh.
 */
void ferTriMeshDel(fer_trimesh_t *tm);

/**
 * Load trimesh data from given file.
 */
int ferTriMeshLoad(fer_trimesh_t *tm, const char *fn);

/**
 * Returns true if tm1 collide with tm2.
 */
int ferTriMeshCollide(fer_trimesh_t *tm1, fer_trimesh_t *tm2);

/**
 * Dumps triangular mesh in SVT format.
 */
int ferTriMeshDumpSVT(const fer_trimesh_t *tm, FILE *out, const char *name);

/**
 * Returns position of trimesh.
 */
_fer_inline const fer_vec3_t *ferTriMeshPos(const fer_trimesh_t *tm);

/**
 * Sets position of trimesh.
 */
_fer_inline void ferTriMeshSetPos(fer_trimesh_t *tm, const fer_vec3_t *p);

/**
 * Returns rotation of trimesh.
 */
_fer_inline const fer_quat_t *ferTriMeshRot(const fer_trimesh_t *tm);

/**
 * Sets rotation of trimesh.
 */
_fer_inline void ferTriMeshSetRot(fer_trimesh_t *tm, const fer_quat_t *r);

/**** INLINES ****/
_fer_inline const fer_vec3_t *ferTriMeshPos(const fer_trimesh_t *tm)
{
    return &tm->pos;
}

_fer_inline void ferTriMeshSetPos(fer_trimesh_t *tm, const fer_vec3_t *p)
{
    ferVec3Copy(&tm->pos, p);
}

_fer_inline const fer_quat_t *ferTriMeshRot(const fer_trimesh_t *tm)
{
    return &tm->rot;
}

_fer_inline void ferTriMeshSetRot(fer_trimesh_t *tm, const fer_quat_t *r)
{
    ferQuatCopy(&tm->rot, r);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_TRIMESH_H__ */
