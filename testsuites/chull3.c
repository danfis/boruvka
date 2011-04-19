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

TEST(testCHull2)
{

    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();

    ferVec3Set(&v, 0.154361, 0.056943, 0.857042);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0.143624, 0.032206, 0.885913);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0.158999, 0.00149, 0.835374);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "3");

    ferVec3Set(&v, 0.16607, 0.035644, 0.818261);
    ferCHull3Add(h, &v);

    ferCHull3DumpSVT(h, stdout, "3");

    ferVec3Set(&v, 0.154361, 0.056943, 0.857042);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0.143624, 0.032206, 0.885913);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0.158999, 0.00149, 0.835374);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "3");

    ferVec3Set(&v, 0.16607, 0.035644, 0.918261);
    ferCHull3Add(h, &v);

    ferCHull3DumpSVT(h, stdout, "3");
    ferCHull3Del(h);


    h = ferCHull3New();

    ferVec3Set(&v, -0.209063, -0.663393, 0.509344);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, -0.282599, -0.663393, 0.412411);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, -0.058384, -0.663401, 0.372891);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "4");

    ferVec3Set(&v, -0.127096, -0.663398, 0.479316);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "4");

    //ferVec3Set(&v, -0.209063, -0.663393, 0.509344);
    //ferVec3Set(&v, -0.282599, -0.663393, 0.412411);
    //ferVec3Set(&v, -0.058384, -0.663401, 0.372891);


    ferCHull3Del(h);

    h = ferCHull3New();
    ferVec3Set(&v, 0.286603, -0.232731, 0.027162);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0.274526, -0.261706, 0.00511);
    ferCHull3Add(h, &v);
    ferVec3Set(&v, 0.290866, -0.277304, -0.061905);
    ferCHull3Add(h, &v);

    ferCHull3DumpSVT(h, stdout, "5");
    ferVec3Set(&v, 0.259842, -0.283292, -0.003185);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "5");
    ferVec3Set(&v, 0.222861, -0.340431, -0.03821);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "5");
    ferVec3Set(&v, 0.259842, -0.283292, -0.003185);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "5");

    ferVec3Set(&v, 0.266526, -0.27365, 0.039597);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "5");

    ferCHull3Del(h);
}

TEST(testCHull3)
{
    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();

    ferVec3Set(&v, 0.104725, 0.108156, -0.94913);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0.118944, 0.012799, -0.880702);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0.061944, 0.014564, -0.882086);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull3");

    ferVec3Set(&v, 0.185542, 0.011005, -0.879202);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull3");

    ferVec3Set(&v, 0.118944, 0.012799, -0.880702);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull3");

    ferVec3Set(&v, 0.104725, 0.108156, -0.94913);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull3");

    ferVec3Set(&v, 0.184061, 0.101854, -0.91822);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull3");

    ferVec3Set(&v, 0.185542, 0.011005, -0.879202);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull3");

    ferCHull3Del(h);
}

TEST(testCHull4)
{
    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();

    ferVec3Set(&v, -0.272542, -0.665381, -0.592063);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.257884, -0.665381, -0.658052);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.183019, -0.665378, -0.71763);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.108152, -0.665375, -0.700408);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.108152, -0.665375, -0.700408);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.071683, -0.665372, -0.606385);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.257884, -0.665381, -0.658052);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.186168, -0.605282, -0.72469);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.183019, -0.665378, -0.71763);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.108152, -0.665375, -0.700408);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.186168, -0.605282, -0.72469);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.086439, -0.584866, -0.608343);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.071683, -0.665372, -0.606385);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.257884, -0.665381, -0.658052);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.267633, -0.603419, -0.655209);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.186168, -0.605282, -0.72469);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.168595, -0.529132, -0.654931);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.237395, -0.542544, -0.587188);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.272542, -0.665381, -0.592063);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.28229, -0.61814, -0.58922);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.168595, -0.529132, -0.654931);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferVec3Set(&v, -0.141182, -0.437705, -0.798194);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull4");

    ferCHull3Del(h);
}

TEST(testCHull5)
{
    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();

    ferVec3Set(&v, 0.286603, -0.232731, 0.027162);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0.274526, -0.261706, 0.00511);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0.290866, -0.277304, -0.061905);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull5");

    ferVec3Set(&v, 0.259842, -0.283292, -0.003185);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull5");

    ferVec3Set(&v, 0.222861, -0.340431, -0.03821);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull5");

    ferVec3Set(&v, 0.266526, -0.27365, 0.039597);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull5");

    ferVec3Set(&v, 0.290866, -0.277304, -0.061905);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull5");

    ferCHull3Del(h);
}

TEST(testCHull6)
{
    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();

    ferVec3Set(&v, 0, 0, 0);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 1, 0, 0);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, 0, 1, 0);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull6");

    ferVec3Set(&v, -1, 0.5, 0);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull6");

    ferVec3Set(&v, -2, 1.5, 0);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull6");

    ferVec3Set(&v, 0.5, -5, 0);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull6");

    ferVec3Set(&v, 1, 1, 2);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull6");

    ferVec3Set(&v, -1, 1, 2);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull6");

    ferCHull3Del(h);
}

TEST(testCHull7)
{
    fer_chull3_t *h;
    fer_vec3_t v;

    h = ferCHull3New();

    ferVec3Set(&v, -0.25573, 0.090306, 0.670782);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.195072, 0.129179, 0.731104);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.255594, 0.113833, 0.663389);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.241515, 0.311485, 0.587832);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.22638, 0.212655, 0.61774);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.216677, 0.309274, 0.642272);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.241515, 0.311485, 0.587832);
    ferCHull3Add(h, &v);

    ferVec3Set(&v, -0.268594, 0.193403, 0.502766);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull7");

    ferVec3Set(&v, -0.303961, 0.054199, 0.625921);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull7");

    /*
    ferVec3Set(&v, -0.277159, 0.126123, 0.443289);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull7");

    ferVec3Set(&v, -0.287605, -0.005722, 0.531844);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull7");

    ferVec3Set(&v, -0.268594, 0.193403, 0.502766);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull7");


    ferVec3Set(&v, -0.22638, 0.212655, 0.61774);
    ferCHull3Add(h, &v);
    ferCHull3DumpSVT(h, stdout, "testCHull7");
    */


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
