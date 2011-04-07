#include "cu.h"
#include <fermat/obb.h>
#include <fermat/dbg.h>

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
    fer_obb_tri_t *tri;
    fer_obb_t *obb;

    ferVec3Set(&v[0], x0, y0, z0);
    ferVec3Set(&v[1], x1, y1, z1);
    ferVec3Set(&v[2], x2, y2, z2);

    tri = ferOBBTriNew(&v[0], &v[1], &v[2]);
    obb = ferOBBNewTri(tri);

    prv("# tri.p[0]: ", &v[0]);
    prv("# tri.p[1]: ", &v[1]);
    prv("# tri.p[2]: ", &v[2]);

    prv("# obb.center: ", &obb->center);
    prv("# obb.axis[0]: ", &obb->axis[0]);
    prv("# obb.axis[1]: ", &obb->axis[1]);
    prv("# obb.axis[2]: ", &obb->axis[2]);
    prv("# obb.half_extents: ", &obb->half_extents);

    ferOBBTriDumpSVT(tri, stdout, "Tri");
    ferOBBDumpSVT(obb, stdout, "OBB");

    ferOBBDel(obb);
    ferOBBTriDel(tri);
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
