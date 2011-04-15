#include "cu.h"
#include <fermat/obb.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include "bunny.h"

static void prv(const char *prefix, const fer_vec3_t *v)
{
    fprintf(stdout, prefix);
    ferVec3Print(v, stdout);
    fprintf(stdout, "\n");
}

static void prOBBTri(fer_real_t x0, fer_real_t y0, fer_real_t z0,
                     fer_real_t x1, fer_real_t y1, fer_real_t z1,
                     fer_real_t x2, fer_real_t y2, fer_real_t z2)
{
    fer_vec3_t v[3];
    fer_obb_t *obb;

    ferVec3Set(&v[0], x0, y0, z0);
    ferVec3Set(&v[1], x1, y1, z1);
    ferVec3Set(&v[2], x2, y2, z2);

    obb = ferOBBNewTri(&v[0], &v[1], &v[2]);

    prv("# tri.p[0]: ", &v[0]);
    prv("# tri.p[1]: ", &v[1]);
    prv("# tri.p[2]: ", &v[2]);

    prv("# obb.center: ", &obb->center);
    prv("# obb.axis[0]: ", &obb->axis[0]);
    prv("# obb.axis[1]: ", &obb->axis[1]);
    prv("# obb.axis[2]: ", &obb->axis[2]);
    prv("# obb.half_extents: ", &obb->half_extents);

    ferOBBTriDumpSVT((fer_obb_tri_t *)obb->pri, stdout, "Tri");
    ferOBBDumpSVT(obb, stdout, "OBB");

    ferOBBDel(obb);
}


TEST(obbNew)
{
    prOBBTri(1, 0, 0,
             2, 0, 0,
             1, 1, 0);
    prOBBTri(1, 3, 0.3,
             2, 2.3, 0.2,
             1, 1, -2.1);
}

TEST(obbCollide)
{
    fer_vec3_t c1, c2, a11, a12, a13, a21, a22, a23, e1, e2;
    fer_vec3_t tr1, tr2;
    fer_mat3_t rot1, rot2;
    fer_obb_t *obb1, *obb2;
    int res;

    ferVec3Set(&c1, 0., 0., 0.);
    ferVec3Set(&a11, 1., 0., 0.);
    ferVec3Set(&a12, 0., 1., 0.);
    ferVec3Set(&a13, 0., 0., 1.);
    ferVec3Set(&e1, 0.5, 0.5, 0.5);
    ferVec3Set(&c2, 2., 0., 0.);
    ferVec3Set(&a21, 1., 0., 0.);
    ferVec3Set(&a22, 0., 1., 0.);
    ferVec3Set(&a23, 0., 0., 1.);
    ferVec3Set(&e2, 0.5, 0.5, 0.5);

    obb1 = ferOBBNew(&c1, &a11, &a12, &a13, &e1, NULL);
    obb2 = ferOBBNew(&c2, &a21, &a22, &a23, &e2, NULL);

    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0., 0.);
    res = ferOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertTrue(res);


    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, -1.2, 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0., 0.);
    res = ferOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertFalse(res);

    ferVec3Set(&obb2->center, 1.01, 0., 0.);
    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0., 0.);
    res = ferOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertTrue(res);

    ferVec3Set(&obb2->center, 1.1, 0., 0.);
    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., 0.);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., M_PI_4, 0.);
    res = ferOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertFalse(res);

    ferVec3Set(&obb2->center, 1.1, 0., 0.);
    ferVec3Set(&tr1, 0., 0., 0.);
    ferMat3SetRot3D(&rot1, 0., 0., M_PI_4);
    ferVec3Set(&tr2, 0., 0., 0.);
    ferMat3SetRot3D(&rot2, 0., 0, 0.);
    res = ferOBBDisjoint(obb1, &rot1, &tr1, obb2, &rot2, &tr2);
    assertFalse(res);

    //fprintf(stderr, "res: %d\n", res);
    //ferOBBDumpSVT(obb1, stdout, "OBB1C");
    //ferOBBDumpSVT(obb2, stdout, "OBB2C");

    ferOBBDel(obb1);
    ferOBBDel(obb2);
}


static void pTree(fer_obb_t *root, int id)
{
    char name[10];
    fer_list_t *item;
    fer_obb_t *obb;

    sprintf(name, "%02d", id);
    ferOBBDumpSVT(root, stdout, name);

    FER_LIST_FOR_EACH(&root->obbs, item){
        obb = fer_container_of(item, fer_obb_t, list);

        if (ferListEmpty(&obb->obbs)){
            sprintf(name, "Tri - %02d", id);
            ferOBBTriDumpSVT((fer_obb_tri_t *)obb->pri, stdout, name);
            sprintf(name, "OBB - %02d", id);
            ferOBBDumpSVT(obb, stdout, name);
        }else{
            pTree(obb, id + 1);
        }
    }
}


TEST(obbTriMesh)
{
    fer_vec3_t pts[5];
    unsigned int ids[3 * 3] = { 0, 1, 2,
                                1, 2, 3,
                                2, 3, 4 };
    size_t len = 3;
    fer_obb_t *obb;

    ferVec3Set(pts + 0, 0., 0., 0.);
    ferVec3Set(pts + 1, 1., 0., 0.);
    ferVec3Set(pts + 2, 0., 0.3, 1.);
    ferVec3Set(pts + 3, 1., 1., 1.);
    ferVec3Set(pts + 4, 0., 1., 1.);

    obb = ferOBBNewTriMesh(pts, ids, len);
    pTree(obb, 0);
    ferOBBDel(obb);

    obb = ferOBBNewTriMesh(bunny_coords, bunny_ids, bunny_tri_len);
    pTree(obb, 0);
    ferOBBDel(obb);
}
