#include <stdio.h>
#include <fermat/cd.h>

static int colCB(const fer_cd_t *cd,
                 const fer_cd_geom_t *g1,
                 const fer_cd_geom_t *g2,
                 void *data)
{
    const char *name[2];

    // get names from data pointers
    name[0] = (const char *)ferCDGeomData(g1);
    name[1] = (const char *)ferCDGeomData(g2);

    // print colliding pair
    fprintf(stderr, "Detected collision between `%s' and `%s'.\n",
            name[0], name[1]);

    // return 0 to continue
    return 0;
}


int main(int argc, char *argv[])
{
    fer_cd_params_t params;
    fer_cd_t *cd;
    fer_cd_geom_t *g[3];
    int i;
    const char *names[] = { "g[0]", "g[1]", "g[2]" };

    if (argc != 3){
        fprintf(stderr, "Usage: %s load.g1 trimesh.tris\n", argv[0]);
        return -1;
    }

    // Initalize parameters
    ferCDParamsInit(&params);

    // Create FermatCD instance
    cd = ferCDNew(&params);


    // Create first geom  as cube
    g[0] = ferCDGeomNew(cd);
    ferCDGeomAddBox(cd, g[0], 10, 10, 10);

    // Load second geom from file
    // As example file can be used: examples/bugtrap-1.geom
    g[1] = ferCDGeomNew(cd);
    ferCDGeomLoad(cd, g[1], argv[1]);
    ferCDGeomBuild(cd, g[1]);

    // Read triangles from file (one triangle per line) and create
    // triangular mesh from it
    // As example file can be used: examples/bugtrap-2.raw
    g[2] = ferCDGeomNew(cd);
    ferCDGeomAddTrisFromRaw(cd, g[2], argv[2]);
    ferCDGeomBuild(cd, g[2]);


    // Set names to objects via data pointer
    ferCDGeomSetData(g[0], (void *)names[0]);
    ferCDGeomSetData(g[1], (void *)names[1]);
    ferCDGeomSetData(g[2], (void *)names[2]);


    // Change position and rotation of second objects
    ferCDGeomSetTr3(cd, g[0], -7, 0, 0);
    ferCDGeomSetRotEuler(cd, g[0], M_PI_4, -M_PI_4, 0);

    ferCDGeomSetTr3(cd, g[2], 40, 0, 0);
    ferCDGeomSetRotEuler(cd, g[2], 0, M_PI_2, 0);

    // Move g[2] along x axis and test for collision
    for (i = 0; i < 10; i++){
        ferCDGeomSetTr3(cd, g[2], 40 - i * 5., 0, 0);
        fprintf(stderr, "[%d]:\n", i);
        ferCDCollide(cd, colCB, NULL);
    }

    // Free all allocated memory
    ferCDDel(cd);

    return 0;
}

