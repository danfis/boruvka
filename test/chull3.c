#include <cu/cu.h>
#include <stdarg.h>
#include <boruvka/chull3.h>
#include <boruvka/dbg.h>
#include "data.h"

static void check(bor_chull3_t *h, int num, ...)
{
    bor_list_t *mvl;
    bor_mesh3_vertex_t *mv;
    bor_real_t x, y, z;
    va_list ap;

    va_start(ap, num);

    mvl = &h->mesh->verts;
    for (int i = 0; i < num; ++i){
        x = va_arg(ap, double);
        y = va_arg(ap, double);
        z = va_arg(ap, double);
        mvl = borListNext(mvl);
        mv = bor_container_of(mvl, bor_mesh3_vertex_t, list);
        assertTrue(borVec3Eq2(mv->v, x, y, z));
        if (!borVec3Eq2(mv->v, x, y, z))
            fprintf(stderr, "%.20f\n", x);
    }
    va_end(ap);
}

TEST(testCHull)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();


    borVec3Set(&v, 0., 0., 0.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 0., 0.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 1., 0.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0., 1., 0.);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0., 0., 1.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 0., 1.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 1., 1.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0., 1., 1.);
    borCHull3Add(h, &v);

    check(h, 8,
          0., 0., 0.,
          1., 0., 0.,
          1., 1., 0.,
          0., 1., 0.,
          0., 0., 1.,
          1., 0., 1.,
          1., 1., 1.,
          0., 1., 1.);


    borVec3Set(&v, 0., 0., 0.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 0., 0.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 1., 0.);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0.5, 0.5, 0.1);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0., 1., 2.);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0.5, 0.5, 0.1);
    borCHull3Add(h, &v);

    borVec3Set(&v, 1., 0., 1.);
    borCHull3Add(h, &v);
    borVec3Set(&v, 1., 1., 1.);
    borCHull3Add(h, &v);

    check(h, 9,
          0., 0., 0.,
          1., 0., 0.,
          1., 1., 0.,
          0., 1., 0.,
          0., 0., 1.,
          1., 0., 1.,
          1., 1., 1.,
          0., 1., 1.,
          0., 1., 2.);

    borCHull3Del(h);
}

TEST(testCHull2)
{

    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, 0.154361, 0.056943, 0.857042);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.143624, 0.032206, 0.885913);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.158999, 0.00149, 0.835374);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "3");

    borVec3Set(&v, 0.16607, 0.035644, 0.818261);
    borCHull3Add(h, &v);

    //borCHull3DumpSVT(h, stdout, "3");

    borVec3Set(&v, 0.154361, 0.056943, 0.857042);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.143624, 0.032206, 0.885913);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.158999, 0.00149, 0.835374);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "3");

    borVec3Set(&v, 0.16607, 0.035644, 0.918261);
    borCHull3Add(h, &v);

    check(h, 5,
          0.154361, 0.056943, 0.857042,
          0.143624, 0.032206, 0.885913,
          0.158999, 0.00149, 0.835374,
          0.16607, 0.035644, 0.818261,
          0.16607, 0.035644, 0.918261);

    borCHull3Del(h);


    h = borCHull3New();

    borVec3Set(&v, -0.209063, -0.663393, 0.509344);
    borCHull3Add(h, &v);
    borVec3Set(&v, -0.282599, -0.663393, 0.412411);
    borCHull3Add(h, &v);
    borVec3Set(&v, -0.058384, -0.663401, 0.372891);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "4");

    borVec3Set(&v, -0.127096, -0.663398, 0.479316);
    borCHull3Add(h, &v);

    check(h, 4,
          -0.209063, -0.663393, 0.509344,
          -0.282599, -0.663393, 0.412411,
          -0.058384, -0.663401, 0.372891,
          -0.127096, -0.663398, 0.479316);

    //borVec3Set(&v, -0.209063, -0.663393, 0.509344);
    //borVec3Set(&v, -0.282599, -0.663393, 0.412411);
    //borVec3Set(&v, -0.058384, -0.663401, 0.372891);


    borCHull3Del(h);

    h = borCHull3New();
    borVec3Set(&v, 0.286603, -0.232731, 0.027162);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.274526, -0.261706, 0.00511);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.290866, -0.277304, -0.061905);
    borCHull3Add(h, &v);

    //borCHull3DumpSVT(h, stdout, "5");
    borVec3Set(&v, 0.259842, -0.283292, -0.003185);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "5");
    borVec3Set(&v, 0.222861, -0.340431, -0.03821);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "5");
    borVec3Set(&v, 0.259842, -0.283292, -0.003185);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "5");

    borVec3Set(&v, 0.266526, -0.27365, 0.039597);
    borCHull3Add(h, &v);

    check(h, 4,
          0.286603, -0.232731, 0.027162,
          0.290866, -0.277304, -0.061905,
          0.222861, -0.340431, -0.03821,
          0.266526, -0.27365, 0.039597);

    borCHull3Del(h);
}

TEST(testCHull3)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, 0.104725, 0.108156, -0.94913);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0.118944, 0.012799, -0.880702);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0.061944, 0.014564, -0.882086);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull3");

    borVec3Set(&v, 0.185542, 0.011005, -0.879202);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull3");

    borVec3Set(&v, 0.118944, 0.012799, -0.880702);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull3");

    borVec3Set(&v, 0.104725, 0.108156, -0.94913);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull3");

    borVec3Set(&v, 0.184061, 0.101854, -0.91822);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull3");

    borVec3Set(&v, 0.185542, 0.011005, -0.879202);
    borCHull3Add(h, &v);

    check(h, 5,
          0.104725, 0.108156, -0.94913,
          0.118944, 0.012799, -0.880702,
          0.061944, 0.014564, -0.882086,
          0.185542, 0.011005, -0.879202,
          0.184061, 0.101854, -0.91822);

    borCHull3Del(h);
}

TEST(testCHull4)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, -0.272542, -0.665381, -0.592063);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.257884, -0.665381, -0.658052);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.183019, -0.665378, -0.71763);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.108152, -0.665375, -0.700408);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.108152, -0.665375, -0.700408);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.071683, -0.665372, -0.606385);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.257884, -0.665381, -0.658052);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.186168, -0.605282, -0.72469);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.183019, -0.665378, -0.71763);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.108152, -0.665375, -0.700408);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.186168, -0.605282, -0.72469);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.086439, -0.584866, -0.608343);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.071683, -0.665372, -0.606385);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.257884, -0.665381, -0.658052);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.267633, -0.603419, -0.655209);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.186168, -0.605282, -0.72469);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.168595, -0.529132, -0.654931);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.237395, -0.542544, -0.587188);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.272542, -0.665381, -0.592063);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.28229, -0.61814, -0.58922);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.168595, -0.529132, -0.654931);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull4");

    borVec3Set(&v, -0.141182, -0.437705, -0.798194);
    borCHull3Add(h, &v);

    check(h, 10,
          -0.272542, -0.665381, -0.592063,
          -0.257884, -0.665381, -0.658052,
          -0.183019, -0.665378, -0.71763,
          -0.108152, -0.665375, -0.700408,
          -0.071683, -0.665372, -0.606385,
          -0.086439, -0.584866, -0.608343,
          -0.267633, -0.603419, -0.655209,
          -0.237395, -0.542544, -0.587188,
          -0.28229, -0.61814, -0.58922,
          -0.141182, -0.437705, -0.798194);

    borCHull3Del(h);
}

TEST(testCHull5)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, 0.286603, -0.232731, 0.027162);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0.274526, -0.261706, 0.00511);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0.290866, -0.277304, -0.061905);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull5");

    borVec3Set(&v, 0.259842, -0.283292, -0.003185);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull5");

    borVec3Set(&v, 0.222861, -0.340431, -0.03821);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull5");

    borVec3Set(&v, 0.266526, -0.27365, 0.039597);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull5");

    borVec3Set(&v, 0.290866, -0.277304, -0.061905);
    borCHull3Add(h, &v);

    check(h, 4,
          0.286603, -0.232731, 0.027162,
          0.290866, -0.277304, -0.061905,
          0.222861, -0.340431, -0.03821,
          0.266526, -0.27365, 0.039597);

    borCHull3Del(h);
}

TEST(testCHull6)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, 0, 0, 0);
    borCHull3Add(h, &v);

    borVec3Set(&v, 1, 0, 0);
    borCHull3Add(h, &v);

    borVec3Set(&v, 0, 1, 0);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull6");

    borVec3Set(&v, -1, 0.5, 0);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull6");

    borVec3Set(&v, -2, 1.5, 0);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull6");

    borVec3Set(&v, 0.5, -5, 0);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull6");

    borVec3Set(&v, 1, 1, 2);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull6");

    borVec3Set(&v, -1, 1, 2);
    borCHull3Add(h, &v);
    check(h, 8,
          0., 0., 0.,
          1., 0., 0.,
          0., 1., 0.,
          -1., 0.5, 0.,
          -2., 1.5, 0.,
          0.5, -5., 0.,
          1., 1., 2.,
          -1., 1., 2.);

    borCHull3Del(h);
}

TEST(testCHull7)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, -0.25573, 0.090306, 0.670782);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.195072, 0.129179, 0.731104);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.255594, 0.113833, 0.663389);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.241515, 0.311485, 0.587832);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.22638, 0.212655, 0.61774);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.216677, 0.309274, 0.642272);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.241515, 0.311485, 0.587832);
    borCHull3Add(h, &v);

    borVec3Set(&v, -0.268594, 0.193403, 0.502766);
    borCHull3Add(h, &v);
    //borCHull3DumpSVT(h, stdout, "testCHull7");

    borVec3Set(&v, -0.303961, 0.054199, 0.625921);
    borCHull3Add(h, &v);

    check(h, 6,
          -0.195072, 0.129179, 0.731104,
          -0.241515, 0.311485, 0.587832,
          -0.22638, 0.212655, 0.61774,
          -0.216677, 0.309274, 0.642272,
          -0.268594, 0.193403, 0.502766,
          -0.303961, 0.054199, 0.625921);

    /*
    borVec3Set(&v, -0.277159, 0.126123, 0.443289);
    borCHull3Add(h, &v);
    borCHull3DumpSVT(h, stdout, "testCHull7");

    borVec3Set(&v, -0.287605, -0.005722, 0.531844);
    borCHull3Add(h, &v);
    borCHull3DumpSVT(h, stdout, "testCHull7");

    borVec3Set(&v, -0.268594, 0.193403, 0.502766);
    borCHull3Add(h, &v);
    borCHull3DumpSVT(h, stdout, "testCHull7");


    borVec3Set(&v, -0.22638, 0.212655, 0.61774);
    borCHull3Add(h, &v);
    borCHull3DumpSVT(h, stdout, "testCHull7");
    */


    borCHull3Del(h);
}

TEST(testCHull8)
{
    bor_chull3_t *h;
    bor_vec3_t v;

    h = borCHull3New();

    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.185402, -0.476020, 0.421718);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.053854, -0.663931, 0.328224);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.053854, -0.663931, 0.328224);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.053854, -0.663931, 0.328224);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.204445, -0.664380, 0.513352);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.053854, -0.663931, 0.328224);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.053854, -0.663931, 0.328224);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.053854, -0.663931, 0.328224);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.122854, -0.664138, 0.482322);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.046519, -0.531570, 0.391918);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.055449, -0.663935, 0.375064);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.271436, -0.567707, 0.263965);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.271436, -0.567707, 0.263965);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.271436, -0.567707, 0.263965);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.227520, -0.373261, 0.293101);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.271436, -0.567707, 0.263965);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.227520, -0.373261, 0.293101);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.271436, -0.567707, 0.263965);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.227520, -0.373261, 0.293101);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.183069, -0.509811, 0.329995);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.271436, -0.567707, 0.263965);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.259285, -0.664547, 0.471280);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.277157, -0.528122, 0.400207);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.279163, -0.664604, 0.417328);
    borCHull3Add(h, &v);
    borVec3Set(&v, 0.254815, -0.664541, 0.290686);
    borCHull3Add(h, &v);

    //borCHull3DumpSVT(h, stdout, "1");
    check(h, 12,
          0.279163, -0.664604, 0.417328,
          0.259285, -0.664547, 0.47128,
          0.204445, -0.66438, 0.513352,
          0.122854, -0.664138, 0.482322,
          0.277157, -0.528122, 0.400207,
          0.185402, -0.47602, 0.421718,
          0.046519, -0.53157, 0.391918,
          0.055449, -0.663935, 0.375064,
          0.053854, -0.663931, 0.328224,
          0.254815, -0.664541, 0.290686,
          0.271436, -0.567707, 0.263965,
          0.22752, -0.373261, 0.293101);


    borCHull3Del(h);
}




TEST(testCHullBunny)
{
    bor_chull3_t *h;
    size_t i;

    h = borCHull3New();

    for (i = 0; i < bunny_coords_len; i++){
        borCHull3Add(h, &bunny_coords[i]);
    }

    check(h, 90,
          -0.414624, 0.164083, -0.278254,
          -0.402162, 0.133528, -0.443247,
          -0.339884, -0.04115, -0.668211,
          -0.248382, 0.255825, -0.627493,
          -0.216201, -0.126776, -0.886936,
          -0.168842, 0.102387, -0.920381,
          -0.104072, 0.177278, -0.95253,
          -0.099875, 0.310931, -0.799381,
          0.007237, 0.361687, -0.794439,
          -0.077913, 0.258753, -0.92164,
          0.007957, 0.282241, -0.93168,
          -0.446838, -0.118517, -0.466159,
          -0.459488, -0.093017, -0.311341,
          -0.364126, -0.200299, 0.202388,
          -0.392666, -0.488581, -0.427494,
          -0.381071, -0.629723, -0.350777,
          -0.382112, -0.62406, -0.221577,
          -0.282599, -0.663393, 0.412411,
          -0.263384, -0.663396, 0.466604,
          -0.209063, -0.663393, 0.509344,
          -0.303961, 0.054199, 0.625921,
          -0.253475, -0.081797, 0.756541,
          -0.138444, -0.10425, 0.945975,
          -0.083497, -0.084934, 0.979607,
          -0.004433, -0.146642, 0.985872,
          -0.135892, -0.03552, 0.945455,
          -0.070612, 0.036849, 0.975733,
          -0.004458, -0.042526, 1.01567,
          -0.004249, 0.046042, 1.00324,
          -0.086969, 0.133224, 0.947633,
          -0.003873, 0.161605, 0.970499,
          -0.125544, 0.140012, 0.917678,
          -0.003127, 0.28407, 0.87887,
          -0.159174, 0.125726, 0.888878,
          -0.183807, 0.19697, 0.84448,
          -0.241585, 0.527592, 0.669296,
          -0.265676, 0.513366, 0.634594,
          -0.266772, 0.64233, 0.602061,
          -0.265837, 0.757267, 0.500933,
          -0.309239, 0.776868, 0.304726,
          0.007865, 0.122104, -0.956137,
          -0.257884, -0.665381, -0.658052,
          -0.371322, -0.665382, -0.35362,
          -0.372362, -0.665381, -0.22442,
          -0.335166, -0.66538, -0.078623,
          -0.198638, -0.302135, -0.845816,
          -0.134547, -0.119852, -0.959004,
          -0.124463, -0.293508, -0.899566,
          -0.183019, -0.665378, -0.71763,
          0.193434, -0.665946, -0.715325,
          0.138234, -0.293905, -0.897958,
          0.149571, -0.120281, -0.957264,
          0.21172, -0.302754, -0.843303,
          0.337743, -0.666396, -0.074503,
          0.376722, -0.666513, -0.219833,
          0.377265, -0.666513, -0.349036,
          0.267564, -0.666174, -0.654834,
          0.31148, 0.775931, 0.308527,
          0.265619, 0.756464, 0.504187,
          0.264969, 0.641527, 0.605317,
          0.263086, 0.512567, 0.637832,
          0.238615, 0.526867, 0.672237,
          0.177696, 0.196424, 0.846693,
          0.152305, 0.125256, 0.890786,
          0.118369, 0.139643, 0.919173,
          0.07941, 0.132973, 0.948652,
          0.062419, 0.036648, 0.976547,
          0.127847, -0.035919, 0.94707,
          0.074888, -0.085173, 0.980577,
          0.130184, -0.104656, 0.94762,
          0.247593, -0.082554, 0.75961,
          0.300086, 0.053287, 0.62962,
          0.204445, -0.66438, 0.513353,
          0.259286, -0.664547, 0.471281,
          0.279163, -0.664604, 0.417328,
          0.386561, -0.625221, -0.21687,
          0.387086, -0.630883, -0.346073,
          0.400045, -0.489778, -0.42264,
          0.364663, -0.201399, 0.20685,
          0.466635, -0.094416, -0.305669,
          0.455805, -0.119881, -0.460632,
          0.093627, 0.258494, -0.920589,
          0.114248, 0.310608, -0.79807,
          0.119916, 0.17694, -0.951159,
          0.184061, 0.101854, -0.91822,
          0.230315, -0.12745, -0.884202,
          0.260471, 0.255056, -0.624378,
          0.351567, -0.042194, -0.663976,
          0.411612, 0.132299, -0.438264,
          0.422146, 0.162819, -0.27313);

    borCHull3Del(h);
}
