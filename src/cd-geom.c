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

fer_cd_geom_t *ferCDGeomNew(void)
{
    fer_cd_geom_t *g;

    g = FER_ALLOC_ALIGN(fer_cd_geom_t, 16);
    ferVec3Set(&g->tr, FER_ZERO, FER_ZERO, FER_ZERO);
    ferMat3SetIdentity(&g->rot);
    ferListInit(&g->obbs);

    return g;
}

void ferCDGeomDel(fer_cd_geom_t *g)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    while (!ferListEmpty(&g->obbs)){
        item = ferListNext(&g->obbs);
        ferListDel(item);
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        ferCDOBBDel(obb);
    }

    free(g);
}

void ferCDGeomBuild(fer_cd_geom_t *g)
{
    // TODO
}

void ferCDGeomAddTriMesh(fer_cd_geom_t *g, const fer_vec3_t *pts,
                         const unsigned int *ids, size_t len)
{
    fer_cd_obb_t *obb;

    obb = ferCDOBBNewTriMesh(pts, ids, len, FER_CDOBB_TRIMESH_FAST);
    ferListAppend(&g->obbs, &obb->list);
}


int ferCDGeomCollide(const fer_cd_geom_t *g1, const fer_cd_geom_t *g2)
{
    fer_list_t *item1, *item2;
    fer_cd_obb_t *obb1, *obb2;
    fer_cd_obb_pair_t *pair;
    fer_list_t pairs;

    ferListInit(&pairs);

    FER_LIST_FOR_EACH(&g1->obbs, item1){
        obb1 = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);

        FER_LIST_FOR_EACH(&g2->obbs, item2){
            obb2 = FER_LIST_ENTRY(item2, fer_cd_obb_t, list);

            ferCDOBBOverlapPairs(obb1, &g1->rot, &g1->tr,
                                 obb2, &g2->rot, &g2->tr,
                                 &pairs);
        }
    }

    FER_LIST_FOR_EACH(&pairs, item1){
        pair = FER_LIST_ENTRY(item1, fer_cd_obb_pair_t, list);

        if (ferCDTriMeshTriCollide((fer_cd_trimesh_tri_t *)pair->obb1->shape,
                                   &g1->rot, &g1->tr,
                                   (fer_cd_trimesh_tri_t *)pair->obb2->shape,
                                   &g2->rot, &g2->tr)){
            ferCDOBBFreePairs(&pairs);
            return 1;
        }
    }

    ferCDOBBFreePairs(&pairs);
    return 0;
}


void ferCDGeomDumpSVT(const fer_cd_geom_t *g, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        if (obb->shape && obb->shape->type == FER_CD_SHAPE_TRIMESH){
            ferCDTriMeshDumpSVT2((const fer_cd_trimesh_t *)obb->shape,
                                 &g->rot, &g->tr, out, name, 1);
        }
    }
}
