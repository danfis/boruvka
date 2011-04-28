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

#ifndef __FER_CD_OBB_H__
#define __FER_CD_OBB_H__

#include <fermat/cd.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * OBB - Oriented Bounding Box
 * ============================
 */
struct _fer_cd_obb_t {
    /* R = { x | x = center + r[0] * axis[0] + r[1] * axis[1] + r[2] * axis[2] },
     *   where |r[i]| <= half_extents[i] */
    fer_vec3_t center;       /*!< Center of bounding box */
    fer_vec3_t axis[3];      /*!< Axis - local coordinate system */
    fer_vec3_t half_extents; /*!< Half extents along box's axis */

    fer_cd_shape_t *shape; /*!< Reference to primitive that is holded by bounding box */
    fer_list_t obbs;       /*!< List of children obbs */
    fer_list_t list;       /*!< Reference to parent's list of obbs */
} fer_aligned(16) fer_packed;
typedef struct _fer_cd_obb_t fer_cd_obb_t;


/**
 * Keeps pair of OBBs
 */
struct _fer_cd_obb_pair_t {
    fer_cd_obb_t *obb1;
    fer_cd_obb_t *obb2;

    fer_list_t list;
};
typedef struct _fer_cd_obb_pair_t  fer_cd_obb_pair_t;


/**
 * Creates new oriented bounding box.
 */
fer_cd_obb_t *ferCDOBBNew(const fer_vec3_t *c, const fer_vec3_t *a1,
                          const fer_vec3_t *a2, const fer_vec3_t *a3,
                          const fer_vec3_t *half_extents,
                          fer_cd_shape_t *shape);

/**
 * Creates new OBB for sphere.
 */
fer_cd_obb_t *ferCDOBBNewSphere(const fer_vec3_t *center, fer_real_t radius);


/**
 * Flag for ferCDOBBNewTriMesh() function.
 * If used, fast (but less accurate) method is used for building tree of
 * bounding boxes.
 */
#define FER_CDOBB_TRIMESH_FAST 0x1

/**
 * Flag for ferCDOBBNewTriMesh() function.
 * It specifies accuracy of slow (but accurate) method for fitting bounding
 * boxes, the higher value is, the more accurate (and slower) method is
 * used. Reasonable value are 10, 20, ..., 50.
 * Default value is 20.
 */
#define FER_CDOBB_TRIMESH_ACCURACY(num) (num << 8)

/**
 * Creates bounding box for triangular mesh. Parameter {num_tri} is number
 * of triangles and {ids} hence must have {num_tri} * 3 elements.
 *
 * Using parameter {flags} can be changed (or altered) method used for
 * fitting bounding boxes. Currently, FER_CDOBB_TRIMESH_FAST or
 * FER_CDOBB_TRIMESH_ACCURACY() flags can be used. Set it to zero for default
 * behaviour.
 */
fer_cd_obb_t *ferCDOBBNewTriMesh(const fer_vec3_t *pts,
                                 const unsigned *ids, size_t num_tri,
                                 int flags);

/**
 * Frees allocated memory.
 */
void ferCDOBBDel(fer_cd_obb_t *obb);

/**
 * Returns true if given obbs are disjoint, relative rotation and
 * translation are also given.
 *
 * Note that no recursion is performed, only CDOBBs itself are checked for
 * collision.
 */
int ferCDOBBDisjoint(const fer_cd_obb_t *obb1,
                     const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                     const fer_cd_obb_t *obb2,
                     const fer_mat3_t *rot2, const fer_vec3_t *tr2);

/**
 * Recursively test obb1 and obb2 if they overlap. List {pairs} is filled
 * with fer_obb_pair_t items that contain leaf CDOBBs that overlap.
 * Returns number of overlaping pairs found.
 *
 * Note that members of list {pairs} are allocated on heap and caller is
 * responsible to free the memory. Best practice would be to use function
 * ferCDOBBFreePairs() for that.
 * Also note that list {pairs} will be only updated, i.e., list won't be
 * cleared, only new member could be added.
 */
int ferCDOBBOverlapPairs(const fer_cd_obb_t *obb1,
                         const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                         const fer_cd_obb_t *obb2,
                         const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                         fer_list_t *pairs);

/**
 * Callback for ferCDOBBOverlapPairsCB() function.
 */
typedef int (*fer_cd_obb_overlap)(const fer_cd_obb_t *obb1,
                                  const fer_cd_obb_t *obb2,
                                  void *data);

/**
 * Recursively test obb1 and obb2 if they overlap. For each overlaping
 * _leaf_ OBBs is called given callback. If callback returns -1 testing is
 * terminated.
 */
void ferCDOBBOverlapPairsCB(const fer_cd_obb_t *obb1,
                            const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                            const fer_cd_obb_t *obb2,
                            const fer_mat3_t *rot2, const fer_vec3_t *tr2,
                            fer_cd_obb_overlap cb, void *data);

/**
 * Frees all fer_obb_pair_t members stored in given list.
 */
void ferCDOBBFreePairs(fer_list_t *pairs);

void ferCDOBBDumpSVT(const fer_cd_obb_t *obb, FILE *out, const char *name);
void ferCDOBBDumpSVT2(const fer_cd_obb_t *obb,
                      const fer_mat3_t *rot, const fer_vec3_t *tr,
                      FILE *out, const char *name);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_CD_OBB_H__ */

