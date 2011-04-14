#include <cu/cu.h>
#include <fermat/chull3.h>
#include <fermat/dbg.h>
#include "bunny.h"

TEST(testCHull)
{
    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();


    ferVec3Set(&v, 0., 0., 0.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 0., 0.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 1., 0.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0., 1., 0.);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0., 0., 1.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 0., 1.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 1., 1.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0., 1., 1.);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "1");


    ferVec3Set(&v, 0., 0., 0.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 0., 0.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 1., 0.);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0.5, 0.5, 0.1);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0., 1., 2.);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0.5, 0.5, 0.1);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 1., 0., 1.);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 1., 1., 1.);
    ferCHull3Add(h, &v);

    ferCHull3DumpSVT(h, stdout, "2");

    ferCHull3Del(h);
}

TEST(testCHullBunny)
{
    fer_chull3_t *h;
    size_t i;

    h = ferCHull3New();

    for (i = 0; i < bunny_coords_len; i++){
        ferCHull3Add(h, &bunny_coords[i]);
    }

    testBunnyDumpSVT(stdout, "bunny object");
    ferCHull3DumpSVT(h, stdout, "bunny");
    ferCHull3Del(h);
}
