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

#include <fermat/cd.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

static fer_cd_shape_class_t shape = {
    .type          = FER_CD_SHAPE_PLANE,
    .del           = (fer_cd_shape_del_fn)ferCDPlaneDel,
    .support       = (fer_cd_shape_support_fn)ferCDPlaneSupport,
    .center        = (fer_cd_shape_center_fn)ferCDPlaneCenter,
    .fit_obb       = (fer_cd_shape_fit_obb_fn)ferCDPlaneFitOBB,
    .update_chull  = (fer_cd_shape_update_chull_fn)ferCDPlaneUpdateCHull,
    .update_minmax = (fer_cd_shape_update_minmax_fn)ferCDPlaneUpdateMinMax,
    .update_cov    = (fer_cd_shape_update_cov_fn)ferCDPlaneUpdateCov,
    .dump_svt      = (fer_cd_shape_dump_svt_fn)ferCDPlaneDumpSVT
};

fer_cd_plane_t *ferCDPlaneNew(void)
{
    fer_cd_plane_t *c;

    c = FER_ALLOC(fer_cd_plane_t);
    c->shape.cl = &shape;

    return c;
}

void ferCDPlaneDel(fer_cd_plane_t *c)
{
    FER_FREE(c);
}

void ferCDPlaneSupport(const fer_cd_plane_t *c, const fer_vec3_t *dir,
                       fer_vec3_t *p)
{
    // TODO
    fprintf(stderr, "CD Error: Support function is not available for plane shape.\n");
}

void ferCDPlaneCenter(const fer_cd_plane_t *s,
                      const fer_mat3_t *rot, const fer_vec3_t *tr,
                      fer_vec3_t *center)
{
    // TODO
    fprintf(stderr, "CD Error: Center function is not available for plane shape.\n");
}

void ferCDPlaneFitOBB(const fer_cd_plane_t *c,
                      fer_vec3_t *center,
                      fer_vec3_t *axis0,
                      fer_vec3_t *axis1,
                      fer_vec3_t *axis2,
                      fer_vec3_t *half_extents, int flags)
{
    ferVec3Set(center, FER_ZERO, FER_ZERO, FER_ZERO);
    ferVec3Set(axis0, FER_ONE,  FER_ZERO, FER_ZERO);
    ferVec3Set(axis1, FER_ZERO, FER_ONE,  FER_ZERO);
    ferVec3Set(axis2, FER_ZERO, FER_ZERO, FER_ONE);
    //ferVec3Set(half_extents, FER_REAL_MAX, FER_REAL_MAX, FER_REAL_MAX);
    ferVec3Set(half_extents, FER_REAL_MAX, FER_REAL_MAX, 0.1);
}

int ferCDPlaneUpdateCHull(const fer_cd_plane_t *c, fer_chull3_t *chull,
                          const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    // do nothing
    return 0;
}

void ferCDPlaneUpdateMinMax(const fer_cd_plane_t *c, const fer_vec3_t *_axis,
                          const fer_mat3_t *rot, const fer_vec3_t *tr,
                          fer_real_t *min, fer_real_t *max)
{
    *min = -FER_REAL_MAX;
    *max = FER_REAL_MAX;
}

void ferCDPlaneUpdateCov(const fer_cd_plane_t *s,
                         const fer_mat3_t *rot, const fer_vec3_t *tr,
                         fer_vec3_t *wcenter, fer_mat3_t *cov,
                         fer_real_t *area, int *num)
{
    // do nothing
}

void ferCDPlaneDumpSVT(const fer_cd_plane_t *c,
                       FILE *out, const char *name,
                       const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    // do nothing
}
