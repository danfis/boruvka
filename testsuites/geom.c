#include "cu.h"
#include <fermat/cd.h>
#include <fermat/dbg.h>
#include "bunny.h"

#if 0
TEST(geomCollideTriMesh)
{
    fer_cd_geom_t *g1, *g2;
    fer_vec3_t tr;
    fer_mat3_t rot;
    int res;

    g1 = ferCDGeomNew();
    g2 = ferCDGeomNew();
    ferCDGeomAddTriMesh(g1, bunny_coords, bunny_ids, bunny_tri_len, NULL, NULL);
    ferCDGeomAddTriMesh(g2, bunny_coords, bunny_ids, bunny_tri_len, NULL, NULL);

    ferVec3Set(&tr, 1, 0, 0);
    ferMat3SetRot3D(&rot, 0, M_PI_4/2, 0);
    ferCDGeomSetTr(g2, &tr);
    ferCDGeomSetRot(g2, &rot);
    res = ferCDGeomCollide(g1, g2);
    assertTrue(res);

    ferCDGeomDumpSVT(g1, stdout, "g1");
    ferCDGeomDumpSVT(g2, stdout, "g2");
    DBG("res: %d", res);

    ferCDGeomDel(g1);
    ferCDGeomDel(g2);
}



TEST(geomSphere)
{
}

#endif
