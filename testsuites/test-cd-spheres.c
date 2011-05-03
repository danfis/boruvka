#include <fermat/cd.h>
#include <fermat/timer.h>
#include <fermat/dbg.h>
#include "data.h"

#include "test-cd-spheres-common.c"

static void test1(void);

int main(int argc, char *argv[])
{
    test1();

    return 0;
}

static void test1(void)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;

    cd = ferCDNew();

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
    DBG("Build g1: %lu", ferTimerElapsedInUs(&timer));

    //ferCDGeomDumpOBBSVT(g1, stdout, "g1");

    ferTimerStart(&timer);
    g2 = ferCDGeomNew(cd);
    for (i = 0; i < protein_small_len; i++){
        ferCDGeomAddSphere2(cd, g2, 0.5 * protein_small_radius[i],
                                    &protein_small_center[i]);
    }
    ferCDGeomBuild(cd, g2);
    ferTimerStop(&timer);
    DBG("Build g2: %lu", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        ret = ferCDGeomCollide(cd, g1, g2);
        if (ret != ret2){
            DBG("[%04d] FAIL", i);
        }
        ferTimerStop(&timer);
        //DBG("Coll[%02d] %04lu - %d (%d)", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    ferCDDel(cd);

    DBG("overall_time: %lu", overall_time);
}
