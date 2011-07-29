#include <fermat/cd.h>
#include <fermat/cd-sphere-grid.h>
#include <fermat/timer.h>
#ifdef HAVE_OZCOLLIDE
#include "ozcollide/ozcollide.h"
#endif /* HAVE_OZCOLLIDE */
#include "data.h"

static void testCD1(void);
static void testCDSphereGrid1(void);
static void testOZCollide1(void);

#ifdef HAVE_OZCOLLIDE
static int OZCollide(ozcollide::AABBTreeSphere *big, ozcollide::Sphere *small,
                     const fer_mat3_t *rot, const fer_vec3_t *tr, int num);
#endif /* HAVE_OZCOLLIDE */

static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret);

int main(int argc, char *argv[])
{
    testCD1();
    testCDSphereGrid1();
    testOZCollide1();

    return 0;
}

static void testCD1(void)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;

    cd = ferCDNew(NULL);

    ferCDSetBuildFlags(cd, FER_CD_FIT_CALIPERS |
            FER_CD_FIT_CALIPERS_NUM_ROT(10));

    ferTimerStart(&timer);
    g1 = ferCDGeomNew(cd);
    for (i = 0; i < protein_big_len; i++){
        ferCDGeomAddSphere2(cd, g1, protein_big_radius[i],
                                    &protein_big_center[i]);
    }
    ferCDGeomBuild(cd, g1);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD1 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    //ferCDGeomDumpOBBSVT(g1, stdout, "g1");

    ferTimerStart(&timer);
    g2 = ferCDGeomNew(cd);
    for (i = 0; i < protein_small_len; i++){
        ferCDGeomAddSphere2(cd, g2, 0.5 * protein_small_radius[i],
                                    &protein_small_center[i]);
    }
    ferCDGeomBuild(cd, g2);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD1 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        ret = ferCDGeomCollide(cd, g1, g2);
        if (ret != ret2){
            fprintf(stdout, "# testCD1 :: [%04d] FAIL\n", (int)i);
        }
        ferTimerStop(&timer);
        //DBG("Coll[%02d] %04lu - %d (%d)", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    ferCDDel(cd);

    fprintf(stdout, "# testCD1 :: overall_time: %lu\n", overall_time);
}

static void testCDSphereGrid1(void)
{
    fer_cd_sphere_grid_t *g;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2, c;
    size_t i, j;
    int ret, ret2;
    unsigned long overall_time = 0L;

    g = ferCDSphereGridNew();

    ferTimerStart(&timer);
    for (i = 0; i < protein_big_len; i++){
        ferCDSphereGridAdd(g, protein_big_radius[i],
                              &protein_big_center[i]);
    }
    ferCDSphereGridBuild(g, -1.);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCDSphereGrid1 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferTimerStart(&timer);

        ret = 0;
        for (j = 0; j < protein_small_len; j++){
            ferMat3MulVec(&c, &rot2, &protein_small_center[j]);
            ferVec3Add(&c, &tr2);
            ret = ferCDSphereGridCollide(g, 0.5 * protein_small_radius[j], &c);
            //ferCDSphereGridCollDumpSVT(g, protein_small_radius[j], &c, stdout, "1");
            if (ret)
                break;
        }

        if (ret != ret2){
            fprintf(stdout, "# testCDSphereGrid1 :: [%04d] FAIL (%d %d)\n", (int)i, (int)ret, (int)ret2);
        }
        ferTimerStop(&timer);
        //DBG("Coll[%02d] %04lu - %d (%d)", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    ferCDSphereGridDel(g);

    fprintf(stdout, "# testCDSphereGrid1 :: overall_time: %lu\n", overall_time);
}

static void testOZCollide1(void)
{
#ifdef HAVE_OZCOLLIDE
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    ozcollide::AABBTreeSphere *big;
    ozcollide::Sphere *small;
    unsigned long overall_time = 0L;

    ferTimerStart(&timer);
    ozcollide::Sphere *s = new ozcollide::Sphere[protein_big_len];
    for (i = 0; i < protein_big_len; i++){
        s[i].center.x = ferVec3X(&protein_big_center[i]);
        s[i].center.y = ferVec3Y(&protein_big_center[i]);
        s[i].center.z = ferVec3Z(&protein_big_center[i]);
        s[i].radius   = protein_big_radius[i];
    }
    ozcollide::AABBTreeSphere_Builder builder;
    big = builder.build(protein_big_len, s);
    ferTimerStop(&timer);
    fprintf(stdout, "# testOZCollide1 :: Build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    small = new ozcollide::Sphere[protein_small_len];
    for (i = 0; i < protein_small_len; i++){
        small[i].radius   = 0.5 * protein_small_radius[i];
    }
    ferTimerStop(&timer);
    fprintf(stdout, "# testOZCollide1 :: Build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferTimerStart(&timer);
        ret = OZCollide(big, small, &rot2, &tr2, i);
        if (ret != ret2){
            fprintf(stdout, "# testOZCollide1 :: [%04d] FAIL\n", i);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testOZCollide1 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    fprintf(stdout, "# testOZCollide1 :: overall_time: %lu\n", overall_time);
#else /* HAVE_OZCOLLIDE */
    fprintf(stdout, "# testOZCollide1 :: No OZCollide\n");
#endif /* HAVE_OZCOLLIDE */
}



#ifdef HAVE_OZCOLLIDE
static int OZCollide(ozcollide::AABBTreeSphere *big, ozcollide::Sphere *small,
                     const fer_mat3_t *rot, const fer_vec3_t *tr, int num)
{
    size_t i;
    int ret;
    fer_vec3_t v;

    ret = 0;
    for (i = 0; i < protein_small_len; i++){
        ferMat3MulVec(&v, rot, &protein_small_center[i]);
        ferVec3Add(&v, tr);

        small[i].center.x = ferVec3X(&v);
        small[i].center.y = ferVec3Y(&v);
        small[i].center.z = ferVec3Z(&v);

        ret = big->isCollideWithSphere(small[i]);
        if (ret)
            break;
    }

    return ret;
}
#endif /* HAVE_OZCOLLIDE */


static FILE *transin = NULL;
static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret)
{
    float x, y, z, w, p, r;
    static int c = 0;
    fer_vec3_t v;

    c++;
    if (!transin)
        transin = fopen("data-test-cd-spheres.trans.txt", "r");

    if (fscanf(transin, "%f %f %f %f %f %f %d", &x, &y, &z, &w, &p, &r, ret) == 7){
        ferMat3SetRot3D(rot, w, p, r);
        ferVec3Set(&v, 8.72557, 2.69257, 21.6929);
        ferMat3MulVec(tr, rot, &v);
        ferVec3Set(&v, x, y, z);
        ferVec3Add(tr, &v);
        return 0;
    }else{
        fclose(transin);
        transin = NULL;
        return -1;
    }
}
