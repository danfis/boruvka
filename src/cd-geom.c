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

fer_cd_geom_t *ferCDGeomNew(fer_cd_t *cd)
{
    fer_cd_geom_t *g;

    g = FER_ALLOC_ALIGN(fer_cd_geom_t, 16);
    ferVec3Set(&g->tr, FER_ZERO, FER_ZERO, FER_ZERO);
    ferMat3SetIdentity(&g->rot);
    ferListInit(&g->obbs);

    // add to list of all gemos
    ferListAppend(&cd->geoms, &g->list);
    ferListAppend(&cd->geoms_dirty, &g->list_dirty);

    g->sap = NULL;

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

    ferListDel(&g->list);
    ferListDel(&g->list_dirty);

    if (cd->sap && g->sap)
        ferCDSAPRemove(cd->sap, g);

    free(g);
}

void ferCDGeomBuild(fer_cd_t *cd, fer_cd_geom_t *g)
{
    ferCDOBBMerge(&g->obbs, cd->build_flags);
    ferCDGeomSetDirty(cd, g);
}


void ferCDGeomAddSphere(fer_cd_t *cd, fer_cd_geom_t *g, fer_real_t radius)
{
    fer_cd_sphere_t *s;
    fer_cd_obb_t *obb;

    s = ferCDSphereNew(radius);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)s, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);

    ferCDGeomSetDirty(cd, g);
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

    ferCDGeomSetDirty(cd, g);
}


void ferCDGeomAddBox(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t lx, fer_real_t ly, fer_real_t lz)
{
    fer_cd_box_t *b;
    fer_cd_obb_t *obb;

    b   = ferCDBoxNew(lx, ly, lz);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)b, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);

    ferCDGeomSetDirty(cd, g);
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

    ferCDGeomSetDirty(cd, g);
}


void ferCDGeomAddCyl(fer_cd_t *cd, fer_cd_geom_t *g,
                     fer_real_t radius, fer_real_t height)
{
    fer_cd_cyl_t *c;
    fer_cd_obb_t *obb;

    c   = ferCDCylNew(radius, height);
    obb = ferCDOBBNewShape((fer_cd_shape_t *)c, cd->build_flags);
    ferListAppend(&g->obbs, &obb->list);

    ferCDGeomSetDirty(cd, g);
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

    ferCDGeomSetDirty(cd, g);
}


void ferCDGeomAddTriMesh(fer_cd_t *cd, fer_cd_geom_t *g,
                         const fer_vec3_t *pts,
                         const unsigned int *ids, size_t len)
{
    ferCDGeomAddTriMesh2(cd, g, pts, ids, len, fer_mat3_identity, fer_vec3_origin);
    ferCDGeomSetDirty(cd, g);
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

    ferCDGeomSetDirty(cd, g);
}

void ferCDGeomAddTrisFromRaw(fer_cd_t *cd, fer_cd_geom_t *g,
                             const char *filename)
{
    ferCDGeomAddTrisFromRawScale(cd, g, filename, FER_ONE);
    ferCDGeomSetDirty(cd, g);
}

void ferCDGeomAddTrisFromRawScale(fer_cd_t *cd, fer_cd_geom_t *g,
                                  const char *filename, fer_real_t scale)
{
    FILE *fin;
    float ax, ay, az, bx, by, bz, cx, cy, cz;
    fer_vec3_t p0, p1, p2;
    fer_cd_tri_t *tri;
    fer_cd_obb_t *obb;
    size_t zero_tris;

    fin = fopen(filename, "r");
    if (!fin){
        fprintf(stderr, "CD Error: Can't open file `%s'\n", filename);
        return;
    }

    zero_tris = 0;
    while (fscanf(fin, "%f %f %f %f %f %f %f %f %f",
                  &ax, &ay, &az, &bx, &by, &bz, &cx, &cy, &cz) == 9){
        ferVec3Set(&p0, ax, ay, az);
        ferVec3Set(&p1, bx, by, bz);
        ferVec3Set(&p2, cx, cy, cz);
        ferVec3Scale(&p0, scale);
        ferVec3Scale(&p1, scale);
        ferVec3Scale(&p2, scale);

        if (ferIsZero(FER_REAL(0.5) * ferVec3TriArea2(&p0, &p1, &p2))){
            zero_tris++;
            continue;
        }

        tri = ferCDTriNew(&p0, &p1, &p2);
        obb = ferCDOBBNewShape((fer_cd_shape_t *)tri, cd->build_flags);
        ferListAppend(&g->obbs, &obb->list);
    }

    if (zero_tris > 0){
        fprintf(stderr, "CD Warning: ferCDGeomAddTrisFromRaw(): %d triangles"
                        " with zero area ignored.\n", zero_tris);
    }

    fclose(fin);

    ferCDGeomSetDirty(cd, g);
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

    c->ret = __ferCDShapeCollide(c->cd,
                                 obb1->shape, &c->g1->rot, &c->g1->tr,
                                 obb2->shape, &c->g2->rot, &c->g2->tr);

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

void ferCDGeomSetDirty(fer_cd_t *cd, fer_cd_geom_t *g)
{
    if (!ferCDGeomDirty(cd, g)){
        ferListAppend(&cd->geoms_dirty, &g->list_dirty);
    }
}

void __ferCDGeomResetDirty(fer_cd_t *cd, fer_cd_geom_t *g)
{
    ferListDel(&g->list_dirty);
}

static void dumpSVT(const fer_cd_geom_t *g,
                    fer_cd_obb_t *obb, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *o;

    if (obb->shape){
        if (obb->shape->cl->dump_svt){
            obb->shape->cl->dump_svt(obb->shape, out, name, &g->rot, &g->tr);
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            dumpSVT(g, o, out, name);
        }
    }
}

void ferCDGeomDumpSVT(const fer_cd_geom_t *g, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        dumpSVT(g, obb, out, name);
    }
}

void ferCDGeomDumpOBBSVT(const fer_cd_geom_t *g, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        ferCDOBBDumpTreeSVT(obb, out, name, &g->rot, &g->tr);
    }
}

static size_t _ferCDGeomDumpTriSVT(const fer_cd_obb_t *obb, FILE *out,
                                   const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    fer_list_t *item;
    fer_cd_obb_t *o;
    fer_cd_tri_t *t;
    fer_vec3_t v;
    size_t s = 0;

    if (ferListEmpty(&obb->obbs)){
        if (obb->shape && (obb->shape->cl->type == FER_CD_SHAPE_TRI
                            || obb->shape->cl->type == FER_CD_SHAPE_TRIMESH_TRI)){
            t = (fer_cd_tri_t *)obb->shape;

            ferMat3MulVec(&v, rot, t->p0);
            ferVec3Add(&v, tr);
            ferVec3Print(&v, out);
            fprintf(out, "\n");

            ferMat3MulVec(&v, rot, t->p1);
            ferVec3Add(&v, tr);
            ferVec3Print(&v, out);
            fprintf(out, "\n");

            ferMat3MulVec(&v, rot, t->p2);
            ferVec3Add(&v, tr);
            ferVec3Print(&v, out);
            fprintf(out, "\n");
            s += 3;
        }
    }else{
        FER_LIST_FOR_EACH(&obb->obbs, item){
            o = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            s += _ferCDGeomDumpTriSVT(o, out, rot, tr);
        }
    }

    return s;
}

void ferCDGeomDumpTriSVT(const fer_cd_geom_t *g, FILE *out, const char *name)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;
    size_t numpts, i;

    fprintf(out, "-----\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    numpts = 0;
    fprintf(out, "Points:\n");
    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        numpts += _ferCDGeomDumpTriSVT(obb, out, &g->rot, &g->tr);
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < numpts; i += 3){
        fprintf(out, "%d %d %d\n", (int)i, (int)i + 1, (int)i + 2);
    }

    fprintf(out, "-----\n");
}


void __ferCDGeomSetMinMax(const fer_cd_geom_t *g,
                          const fer_vec3_t *axis,
                          fer_real_t *min, fer_real_t *max)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    *min = FER_REAL_MAX;
    *max = -FER_REAL_MAX;
    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        __ferCDOBBUpdateMinMax(obb, axis, &g->rot, &g->tr, min, max);
    }
}
