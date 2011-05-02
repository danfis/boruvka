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

fer_cd_geom_t *ferCDGeomNew(fer_cd_t *cd)
{
    fer_cd_geom_t *g;

    g = FER_ALLOC_ALIGN(fer_cd_geom_t, 16);
    ferVec3Set(&g->tr, FER_ZERO, FER_ZERO, FER_ZERO);
    ferMat3SetIdentity(&g->rot);
    ferListInit(&g->obbs);

    return g;
}

void ferCDGeomDel(fer_cd_t *cd, fer_cd_geom_t *g)
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

void ferCDGeomBuild(fer_cd_t *cd, fer_cd_geom_t *g)
{
    ferCDOBBMerge(&g->obbs, cd->build_flags);
}


void ferCDGeomAddSphere(fer_cd_t *cd, fer_cd_geom_t *g, fer_real_t radius)
{
    fer_cd_sphere_t *s;
    fer_cd_obb_t *obb;

    s = ferCDSphereNew(radius);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)s, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}

void ferCDGeomAddSphere2(fer_cd_t *cd, fer_cd_geom_t *g, fer_real_t radius,
                         const fer_vec3_t *tr)
{
    fer_cd_sphere_t *s;
    fer_cd_shape_off_t *off;
    fer_cd_obb_t *obb;

    s   = ferCDSphereNew(radius);
    off = ferCDShapeOffNew((fer_cd_shape_t *)s, fer_mat3_identity, tr);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)off, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}


void ferCDGeomAddBox(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t lx, fer_real_t ly, fer_real_t lz)
{
    fer_cd_box_t *b;
    fer_cd_obb_t *obb;

    b   = ferCDBoxNew(lx, ly, lz);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)b, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}

void ferCDGeomAddBox2(fer_cd_t *cd, fer_cd_geom_t *g,
                      fer_real_t lx, fer_real_t ly, fer_real_t lz,
                      const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_cd_box_t *b;
    fer_cd_shape_off_t *off;
    fer_cd_obb_t *obb;

    b   = ferCDBoxNew(lx, ly, lz);
    off = ferCDShapeOffNew((fer_cd_shape_t *)b, rot, tr);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)off, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}


void ferCDGeomAddCyl(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t radius, fer_real_t height)
{
    fer_cd_cyl_t *c;
    fer_cd_obb_t *obb;

    c   = ferCDCylNew(radius, height);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)c, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}

void ferCDGeomAddCyl2(fer_cd_t *cd, fer_cd_geom_t *g,
                      fer_real_t radius, fer_real_t height,
                      const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_cd_cyl_t *c;
    fer_cd_shape_off_t *off;
    fer_cd_obb_t *obb;

    c   = ferCDCylNew(radius, height);
    off = ferCDShapeOffNew((fer_cd_shape_t *)c, rot, tr);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)off, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}


void ferCDGeomAddTriMesh(fer_cd_t *cd, fer_cd_geom_t *g,
                         const fer_vec3_t *pts,
                         const unsigned int *ids, size_t len)
{
    ferCDGeomAddTriMesh2(cd, g, pts, ids, len, fer_mat3_identity, fer_vec3_origin);
}

void ferCDGeomAddTriMesh2(fer_cd_t *cd, fer_cd_geom_t *g,
                          const fer_vec3_t *pts,
                          const unsigned int *ids, size_t len,
                          const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_cd_trimesh_t *t;
    fer_cd_obb_t *obb;

    t   = ferCDTriMeshNew(pts, ids, len, rot, tr);
    obb = ferCDOBBNewTriMesh(t, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);
}



struct __collide_t {
    fer_cd_t *cd;
    const fer_cd_geom_t *g1;
    const fer_cd_geom_t *g2;
    int ret;
};

static int __ferCDGeomCollideCB(const fer_cd_obb_t *obb1,
                                const fer_cd_obb_t *obb2,
                                void *data)
{
    struct __collide_t *c = (struct __collide_t *)data;
    int type1, type2;

    type1 = obb1->shape->cl->type;
    type2 = obb2->shape->cl->type;

    if (c->cd->collide[type1][type2]){
        c->ret = c->cd->collide[type1][type2](c->cd,
                                              obb1->shape,
                                              &c->g1->rot, &c->g1->tr,
                                              obb2->shape,
                                              &c->g2->rot, &c->g2->tr);
    }else if (c->cd->collide[type2][type1]){
        c->ret = c->cd->collide[type1][type2](c->cd,
                                              obb2->shape,
                                              &c->g2->rot, &c->g2->tr,
                                              obb1->shape,
                                              &c->g1->rot, &c->g1->tr);
    }else{
        fprintf(stderr, "Error: No collider for %d-%d\n", type1, type2);
    }

    if (c->ret)
        return -1;
    return 0;
}

int ferCDGeomCollide(fer_cd_t *cd,
                     const fer_cd_geom_t *g1, const fer_cd_geom_t *g2)
{
    struct __collide_t c;
    fer_list_t *item1, *item2;
    fer_cd_obb_t *obb1, *obb2;

    c.cd = cd;
    c.g1 = g1;
    c.g2 = g2;

    FER_LIST_FOR_EACH(&g1->obbs, item1){
        obb1 = FER_LIST_ENTRY(item1, fer_cd_obb_t, list);

        FER_LIST_FOR_EACH(&g2->obbs, item2){
            obb2 = FER_LIST_ENTRY(item2, fer_cd_obb_t, list);

            c.ret = 0;
            ferCDOBBOverlapPairsCB(obb1, &g1->rot, &g1->tr,
                                   obb2, &g2->rot, &g2->tr,
                                   __ferCDGeomCollideCB, (void *)&c);
            if (c.ret)
                return 1;
        }
    }
    return 0;
}


void ferCDGeomDumpSVT(const fer_cd_geom_t *g, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        ferCDOBBDumpSVT(obb, out, name, &g->rot, &g->tr);
    }
}
