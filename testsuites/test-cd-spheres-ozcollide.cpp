#include <fermat/cd.h>
#include <fermat/timer.h>
#include "ozcollide/ozcollide.h"
#include "data.h"

#include "test-cd-spheres-common.c"

static int collide(ozcollide::AABBTreeSphere *big, ozcollide::Sphere *small,
                   const fer_mat3_t *rot, const fer_vec3_t *tr);

static void test1(void);

int main(int argc, char *argv[])
{
    test1();

    return 0;
}

static int collide(ozcollide::AABBTreeSphere *big, ozcollide::Sphere *small,
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

static void test1(void)
{
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
    fprintf(stdout, "test1 :: Build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    small = new ozcollide::Sphere[protein_small_len];
    for (i = 0; i < protein_small_len; i++){
        small[i].radius   = 0.5 * protein_small_radius[i];
    }
    ferTimerStop(&timer);
    fprintf(stdout, "test1 :: Build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferTimerStart(&timer);
        ret = collide(big, small, &rot2, &tr2, i);
        if (ret != ret2){
            fprintf(stdout, "test1 :: [%04d] FAIL\n", i);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "test1 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    fprintf(stdout, "test1 :: overall_time: %lu\n", overall_time);
}


