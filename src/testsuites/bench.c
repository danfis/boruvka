#define CU_ENABLE_TIMER
#include <stdio.h>
#include <cu/cu.h>
#include <mg/vec3.h>
#include "data.h"



#define prTime() printf("%15s: %ld %09ld\n", __func__, t->tv_sec, t->tv_nsec)

static void add(void)
{
    size_t i;
    const struct timespec *t;
    mg_vec3_t v;

    mgVec3Set(&v, MG_ZERO, MG_ZERO, MG_ZERO);
    cuTimerStart();
    for (i = 0; i < vecs_len; i++){
        mgVec3Add(&v, &vecs[i]);
    }
    t = cuTimerStop();
    prTime();
}

static void sub(void)
{
    size_t i;
    const struct timespec *t;
    mg_vec3_t v;

    mgVec3Set(&v, MG_ZERO, MG_ZERO, MG_ZERO);
    cuTimerStart();
    for (i = 0; i < vecs_len; i++){
        mgVec3Sub(&v, &vecs[i]);
    }
    t = cuTimerStop();
    prTime();
}

static void scale(void)
{
    size_t i;
    const struct timespec *t;
    mg_vec3_t v;

    mgVec3Copy(&v, &vecs[0]);
    cuTimerStart();
    for (i = 0; i < vecs_len; i++){
        mgVec3Scale(&v, mgVec3X(&vecs[i]));
    }
    t = cuTimerStop();
    prTime();
}

static void normalize(void)
{
    size_t i;
    const struct timespec *t;
    mg_vec3_t v;

    cuTimerStart();
    for (i = 0; i < vecs_len; i++){
        mgVec3Copy(&v, &vecs[i]);
        mgVec3Normalize(&v);
    }
    t = cuTimerStop();
    prTime();
}

static void dot(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t dot;

    cuTimerStart();
    for (i = 0; i < vecs_len - 1; i++){
        dot = mgVec3Dot(&vecs[i], &vecs[i + 1]);
    }
    t = cuTimerStop();
    prTime();
}

static void cross(void)
{
    size_t i;
    const struct timespec *t;
    mg_vec3_t v;

    cuTimerStart();
    for (i = 0; i < vecs_len - 1; i++){
        mgVec3Cross(&v, &vecs[i], &vecs[i + 1]);
    }
    t = cuTimerStop();
    prTime();
}

static void len2(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t len;

    cuTimerStart();
    for (i = 0; i < vecs_len; i++){
        len = mgVec3Len2(&vecs[i]);
    }
    t = cuTimerStop();
    prTime();
}

static void len(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t len;

    cuTimerStart();
    for (i = 0; i < vecs_len; i++){
        len = mgVec3Len(&vecs[i]);
    }
    t = cuTimerStop();
    prTime();
}

static void dist2(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t d;

    cuTimerStart();
    for (i = 0; i < vecs_len - 1; i++){
        d = mgVec3Dist2(&vecs[i], &vecs[i + 1]);
    }
    t = cuTimerStop();
    prTime();
}

static void dist(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t d;

    cuTimerStart();
    for (i = 0; i < vecs_len - 1; i++){
        d = mgVec3Dist(&vecs[i], &vecs[i + 1]);
    }
    t = cuTimerStop();
    prTime();
}

static void segment_dist(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t d;
    mg_vec3_t w;

    cuTimerStart();
    for (i = 0; i < vecs_len - 2; i++){
        d = mgVec3PointSegmentDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &w);
    }
    t = cuTimerStop();
    prTime();
}

static void tri_dist(void)
{
    size_t i;
    const struct timespec *t;
    mg_real_t d;
    mg_vec3_t w;

    cuTimerStart();
    for (i = 0; i < vecs_len - 3; i++){
        d = mgVec3PointTriDist2(&vecs[i], &vecs[i + 1], &vecs[i + 2], &vecs[i + 3], &w);
    }
    t = cuTimerStop();
    prTime();
}

static void eq(void)
{
    size_t i;
    const struct timespec *t;
    int e;

    cuTimerStart();
    for (i = 0; i < vecs_len - 1; i++){
        e = mgVec3Eq(&vecs[i], &vecs[i]);
        e = mgVec3Eq(&vecs[i], &vecs[i + 1]);
    }
    t = cuTimerStop();
    prTime();
}

static void neq(void)
{
    size_t i;
    const struct timespec *t;
    int e;

    cuTimerStart();
    for (i = 0; i < vecs_len - 1; i++){
        e = mgVec3NEq(&vecs[i], &vecs[i]);
        e = mgVec3NEq(&vecs[i], &vecs[i + 1]);
    }
    t = cuTimerStop();
    prTime();
}

TEST_SUITES {
    TEST_SUITES_CLOSURE
};

int main(int argc, char *argv[])
{
    add();
    sub();
    scale();
    normalize();
    dot();
    cross();
    len2();
    len();
    dist2();
    dist();
    segment_dist();
    tri_dist();
    eq();
    neq();

    return 0;
}
