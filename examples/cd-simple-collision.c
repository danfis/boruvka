#include <stdio.h>
#include <fermat/cd.h>

int main(int argc, char *argv[])
{
    fer_cd_params_t params;
    fer_cd_t *cd;
    fer_cd_geom_t *g[2];
    fer_vec3_t tr;
    int res;


    // Initalize parameters to default values.
    ferCDParamsInit(&params);

    // Building method can be changed.
    // This will use naive method that uses 5 fixed rotations when fitting
    // OBBs to underlying objects and building will be performed in 8
    // threads.
    params.build_flags = FER_CD_TOP_DOWN
                            | FER_CD_FIT_NAIVE
                            | FER_CD_FIT_NAIVE_NUM_ROT(5)
                            | FER_CD_BUILD_PARALLEL(8);

    // Create FermatCD instance
    cd = ferCDNew(&params);


    // Create first geom
    g[0] = ferCDGeomNew(cd);

    // The first geom consists of box in center and sphere right next to
    // it. First add box with proportions (0.5, 1, 2).
    ferCDGeomAddBox(cd, g[0], 0.5, 1., 2.);

    // Then add sphere with specified offset (no rotation).
    ferVec3Set(&tr, 0.7, 0., 0.);
    ferCDGeomAddSphere2(cd, g[0], 0.5, &tr); 

    // Finally build geom
    ferCDGeomBuild(cd, g[0]);


    // Create second geom
    g[1] = ferCDGeomNew(cd);

    // The second geom consists of a single cylinder.
    // ferCDGeomBuild() doesn't have to be called on this geom because it
    // consists of only one primitive.
    ferCDGeomAddCyl(cd, g[1], 0.3, 1.);


    // Change position and rotation of second object
    ferCDGeomSetTr3(cd, g[1], 1, 1, -1);
    ferCDGeomSetRotEuler(cd, g[1], M_PI_4, 0, 0);

    // Test both geoms on collision.
    // Also callback that will be called for each colliding pair can be
    // provided, but we don't need that.
    res = ferCDCollide(cd, NULL, NULL);
    fprintf(stderr, "res: %d\n", res);


    // Now change position and rotation of first object
    ferCDGeomSetTr3(cd, g[0], 0, 0.5, -0.3);
    ferCDGeomSetRotEuler(cd, g[0], M_PI_4, M_PI_4, 0);

    // And test again.
    res = ferCDCollide(cd, NULL, NULL);
    fprintf(stderr, "res: %d\n", res);


    // Free all allocated memory
    ferCDDel(cd);

    return 0;
}
