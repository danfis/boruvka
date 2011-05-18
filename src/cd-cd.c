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


fer_cd_t *ferCDNew(void)
{
    fer_cd_t *cd;
    size_t i, j;

    cd = FER_ALLOC(fer_cd_t);
    cd->build_flags = FER_CD_TOP_DOWN
                        | FER_CD_FIT_COVARIANCE;

    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = 0; j < FER_CD_SHAPE_LEN; j++){
            cd->collide[i][j] = NULL;
        }
    }

    ferCDSetCollideFn(cd, FER_CD_SHAPE_BOX, FER_CD_SHAPE_BOX,
                      (fer_cd_collide_fn)ferCDCollideBoxBox);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_SPHERE,
                      (fer_cd_collide_fn)ferCDCollideSphereSphere);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_SPHERE, FER_CD_SHAPE_BOX,
                      (fer_cd_collide_fn)ferCDCollideSphereBox);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRI, FER_CD_SHAPE_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);
    ferCDSetCollideFn(cd, FER_CD_SHAPE_TRIMESH_TRI, FER_CD_SHAPE_TRIMESH_TRI,
                      (fer_cd_collide_fn)ferCDCollideTriTri);

    ferCDSetCollideFn(cd, FER_CD_SHAPE_OFF, FER_CD_SHAPE_OFF,
                      (fer_cd_collide_fn)ferCDCollideOffOff);
    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        if (i != FER_CD_SHAPE_OFF)
            ferCDSetCollideFn(cd, FER_CD_SHAPE_OFF, i,
                              (fer_cd_collide_fn)ferCDCollideOffAny);
    }

    for (i = 0; i < FER_CD_SHAPE_LEN; i++){
        for (j = i; j < FER_CD_SHAPE_LEN; j++){
            if (!cd->collide[i][j]){
                ferCDSetCollideFn(cd, i, j, (fer_cd_collide_fn)ferCDCollideCCD);
            }
        }
    }


    ferListInit(&cd->geoms);

    return cd;
}

void ferCDDel(fer_cd_t *cd)
{
    fer_list_t *item;
    fer_cd_geom_t *g;

    while (!ferListEmpty(&cd->geoms)){
        item = ferListNext(&cd->geoms);
        g    = FER_LIST_ENTRY(item, fer_cd_geom_t, list);
        ferCDGeomDel(cd, g);
    }

    free(cd);
}

void ferCDSetCollideFn(fer_cd_t *cd, int shape1, int shape2,
                       fer_cd_collide_fn collider)
{
    cd->collide[shape1][shape2] = collider;
}

int __ferCDShapeCollide(fer_cd_t *cd,
                        const fer_cd_shape_t *s1,
                        const fer_mat3_t *rot1, const fer_vec3_t *tr1,
                        const fer_cd_shape_t *s2,
                        const fer_mat3_t *rot2, const fer_vec3_t *tr2)
{
    int type1, type2, ret;

    type1 = s1->cl->type;
    type2 = s2->cl->type;

    ret = 0;
    if (cd->collide[type1][type2]){
        ret = cd->collide[type1][type2](cd, s1, rot1, tr1, s2, rot2, tr2);
    }else if (cd->collide[type2][type1]){
        ret = cd->collide[type2][type1](cd, s2, rot2, tr2, s1, rot1, tr1);
    }else{
        fprintf(stderr, "Error: No collider for %d-%d\n", type1, type2);
    }

    return ret;
}
