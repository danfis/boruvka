#include "cu.h"
#include <fermat/cd-sphere-grid.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include "bunny.h"
#include "data.h"

TEST(cdspheregridSimple)
{
    fer_cd_sphere_grid_t *g;
    fer_vec3_t c;
    int ret;

    g = ferCDSphereGridNew();

    ferVec3Set(&c, 0, 0, 0);
    ferCDSphereGridAdd(g, 0.1, &c);
    ferVec3Set(&c, 0, 1, 0);
    ferCDSphereGridAdd(g, 0.5, &c);
    ferVec3Set(&c, 1, 1, 0);
    ferCDSphereGridAdd(g, 0.5, &c);
    ferVec3Set(&c, 1, 0, 0);
    ferCDSphereGridAdd(g, 0.5, &c);
    ferVec3Set(&c, 1, 0, 1);
    ferCDSphereGridAdd(g, 0.5, &c);
    ferCDSphereGridBuild(g, -1);

    ferVec3Set(&c, 0.5, 0.5, 0.5);
    ret = ferCDSphereGridCollide(g, 0.3, &c);
    assertFalse(ret);

    ferVec3Set(&c, 0.5, 0.5, 0.5);
    ret = ferCDSphereGridCollide(g, 0.4, &c);
    assertTrue(ret);

    ferVec3Set(&c, 0.1, 0.4, 0.7);
    ret = ferCDSphereGridCollide(g, 0.3, &c);
    assertFalse(ret);

    ferVec3Set(&c, 0.1, 0.4, 0.7);
    ret = ferCDSphereGridCollide(g, 0.7, &c);
    assertTrue(ret);

    ferVec3Set(&c, 3.1, 0.4, 0.7);
    ret = ferCDSphereGridCollide(g, 1.8, &c);
    assertTrue(ret);

    ferCDSphereGridDel(g);
}

TEST(cdspheregridRandom)
{
    fer_cd_sphere_grid_t *g;
    fer_vec3_t c;
    fer_real_t rad;
    int ret, ret2, i;
    fer_rand_mt_t *rand;

    rand = ferRandMTNewAuto();

    g = ferCDSphereGridNew();

    for (i = 0; i < 100; i++){
        ferVec3Set(&c, ferRandMT(rand, -5, 5),
                       ferRandMT(rand, -5, 5),
                       ferRandMT(rand, -5, 5));
        ferCDSphereGridAdd(g, ferRandMT(rand, 0.1, 0.6), &c);
    }
    ferCDSphereGridBuild(g, -1);

    for (i = 0; i < 300; i++){
        ferVec3Set(&c, ferRandMT(rand, -5.5, 5.5),
                       ferRandMT(rand, -5.5, 5.5),
                       ferRandMT(rand, -5.5, 5.5));

        rad = ferRandMT(rand, 0.1, 1);
        ret  = ferCDSphereGridCollide(g, rad, &c);
        ret2 = ferCDSphereGridCollideBruteForce(g, rad, &c);
        assertEquals(ret, ret2);
        if (ret != ret2){
            DBG("sphere grid: %d, brute force %d", ret, ret2);
            ferCDSphereGridCollDumpSVT(g, rad, &c, stdout, NULL);
            exit(-1);
        }
    }

    ferCDSphereGridDel(g);

    ferRandMTDel(rand);
}
