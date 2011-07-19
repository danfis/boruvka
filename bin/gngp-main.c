#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>

#define FREE 0
#define OBST 1

fer_gng_plan_t *gng;
fer_gng_plan_ops_t ops;
fer_gng_plan_params_t params;
size_t max_nodes;
fer_rand_mt_t *rand_mt;
fer_timer_t timer;
fer_vec_t *is;

int dump_period = 10;
const char *dump_prefix = NULL;


static int terminate(void *data);
static void callback(void *data);
static const void *inputSignal(void *data);
static void setUpScene(const char *scene);
static int eval1_1(const fer_vec_t *w, void *data);
static int eval1_3(const fer_vec_t *w, void *data);
static int eval1_5(const fer_vec_t *w, void *data);
static int eval2_1(const fer_vec_t *w, void *data);
static int eval2_3(const fer_vec_t *w, void *data);
static int eval2_5(const fer_vec_t *w, void *data);
static int eval3_1(const fer_vec_t *w, void *data);
static int eval3_3(const fer_vec_t *w, void *data);
static int eval3_5(const fer_vec_t *w, void *data);
static int eval4_1(const fer_vec_t *w, void *data);
static int eval4_3(const fer_vec_t *w, void *data);
static int eval4_5(const fer_vec_t *w, void *data);

static void dump(void);


int main(int argc, char *argv[])
{
    fer_real_t aabb[4] = { -5, 5, -5, 5 };

    if (argc < 4){
        fprintf(stderr, "Usage: %s max_nodes min_nodes scene [dump_prefix dump_period]\n", argv[0]);
        return -1;
    }

    if (argc >= 5)
        dump_prefix = argv[4];
    if (argc >= 6)
        dump_period = atoi(argv[5]);

    is = ferVecNew(2);

    max_nodes = atoi(argv[1]);

    ferGNGPlanOpsInit(&ops);
    ferGNGPlanParamsInit(&params);

    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = NULL;
    ops.callback     = callback;
    ops.callback_period = 5;

    params.dim = 2;
    params.max_dist  = 0.01;
    params.min_nodes = atoi(argv[2]);
    params.start = ferVecNew(2);
    params.goal  = ferVecNew(2);
    params.cells.d = 2;
    params.cells.aabb = aabb;
    params.cells.max_dens = 2;
    params.cells.expand_rate = 2;

    setUpScene(argv[3]);

    rand_mt = ferRandMTNewAuto();
    gng = ferGNGPlanNew(&ops, &params);

    ferTimerStart(&timer);
    ferGNGPlanRun(gng);
    ferTimerStopAndPrintElapsed(&timer, stderr, "\r\n");

    if (dump_prefix){
        dump();
    }else{
        ferGNGPlanDumpNetSVT(gng, stdout, NULL);
        ferGNGPlanDumpObstSVT(gng, stdout, NULL);
        ferGNGPlanDumpPathSVT(gng, stdout, NULL);
    }

    ferGNGPlanDel(gng);

    ferRandMTDel(rand_mt);
    
    ferVecDel(is);

    return 0;
}


static int terminate(void *data)
{
    size_t nodes_len;
    static int counter = 0;

    if (dump_prefix && ++counter == dump_period){
        dump();
        counter = 0;
    }

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    return nodes_len >= max_nodes;
}

static void callback(void *data)
{
    size_t nodes_len;

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    ferTimerStopAndPrintElapsed(&timer, stderr, " %d\r", (int)nodes_len);
}

static const void *inputSignal(void *data)
{
    fer_real_t x, y;

    x = ferRandMT(rand_mt, -5, 5);
    y = ferRandMT(rand_mt, -5, 5);
    ferVec2Set((fer_vec2_t *)is, x, y);
    return is;
}

static void setUpScene(const char *scene)
{

    if (strcmp(scene, "1_1") == 0){
        ops.eval = eval1_1;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(1.5), FER_REAL(4.5));
        params.max_dist = 0.005;
    }else if (strcmp(scene, "1_3") == 0){
        ops.eval = eval1_3;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(1.5), FER_REAL(4.5));
        params.max_dist = 0.015;
    }else if (strcmp(scene, "1_5") == 0){
        ops.eval = eval1_5;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(1.5), FER_REAL(4.5));
        params.max_dist = 0.025;
    }else if (strcmp(scene, "2_1") == 0){
        ops.eval = eval2_1;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(4.5), FER_REAL(4.5));
        params.max_dist = 0.005;
    }else if (strcmp(scene, "2_3") == 0){
        ops.eval = eval2_3;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(4.5), FER_REAL(4.5));
        params.max_dist = 0.015;
    }else if (strcmp(scene, "2_5") == 0){
        ops.eval = eval2_5;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(4.5), FER_REAL(4.5));
        params.max_dist = 0.025;
    }else if (strcmp(scene, "3_1") == 0){
        ops.eval = eval3_1;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(4.5), FER_REAL(4.5));
        params.max_dist = 0.005;
    }else if (strcmp(scene, "3_3") == 0){
        ops.eval = eval3_3;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(4.5), FER_REAL(4.5));
        params.max_dist = 0.015;
    }else if (strcmp(scene, "3_5") == 0){
        ops.eval = eval3_5;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(4.5), FER_REAL(4.5));
        params.max_dist = 0.025;
    }else if (strcmp(scene, "4_1") == 0){
        ops.eval = eval4_1;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(0.45), FER_REAL(0.));
        params.max_dist = 0.005;
    }else if (strcmp(scene, "4_3") == 0){
        ops.eval = eval4_3;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(0.45), FER_REAL(0.));
        params.max_dist = 0.015;
    }else if (strcmp(scene, "4_5") == 0){
        ops.eval = eval4_5;
        ferVec2Set((fer_vec2_t *)params.start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)params.goal, FER_REAL(0.45), FER_REAL(0.));
        params.max_dist = 0.025;
    }
}

static int eval1(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -r && x < r)
            || (y > 4 && x > -2 && x < 2)){
        return FREE;
    }
    return OBST;
}

static int eval1_1(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.01);
}

static int eval1_3(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.03);
}

static int eval1_5(const fer_vec_t *w, void *data)
{
    //return eval1(w, data, 0.1);
    return eval1(w, data, 0.05);
}

static int eval2(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y > 4 && x > 4)
            || (y > -2 && y < 4 && x > -2 - r && x < 4 + r && x > y - r && x < y + r)){
        return FREE;
    }
    return OBST;
}

static int eval2_1(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.01);
}

static int eval2_3(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.03);
}

static int eval2_5(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.05);
}


static int eval3(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y > 0 && x > 3.5)
            || (x < 3.6 && FER_CUBE(x - 1) + FER_CUBE(y + 1) > 9
                    && FER_CUBE(x - 1) + FER_CUBE(y + 1) < FER_CUBE(3 + r))){
        return FREE;
    }
    return OBST;
}

static int eval3_1(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.01);
}

static int eval3_3(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.03);
}

static int eval3_5(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.05);
}

static int eval4(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
         || (x > 0.3 && x < 0.6 && y > -0.15 && y < 0.15)
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(2.)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(2. + r)
                && !(y < 0 && x > 0))
         || (FER_CUBE(x - 0.5) + FER_CUBE(y) > FER_CUBE(1.5)
                && FER_CUBE(x - 0.5) + FER_CUBE(y) < FER_CUBE(1.5 + r)
                && !(y > 0))
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(1.)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(1. + r)
                && !(y < 0))
         || (FER_CUBE(x - 0.25) + FER_CUBE(y) > FER_CUBE(.75)
                && FER_CUBE(x - 0.25) + FER_CUBE(y) < FER_CUBE(.75 + r)
                && !(y > 0))
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(.5)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(.5 + r)
                && !(y < 0))){
        return FREE;
    }
    return OBST;
}

static int eval4_1(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.01);
}

static int eval4_3(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.03);
}

static int eval4_5(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.05);
}


static void dump(void)
{
    FILE *fout;
    char fn[1000];
    static int counter = 0;

    sprintf(fn, "%s%010d.svt", dump_prefix, counter++);
    fout = fopen(fn, "w");
    if (fout){
        ferGNGPlanDumpNetSVT(gng, fout, NULL);
        ferGNGPlanDumpObstSVT(gng, fout, NULL);
        ferGNGPlanDumpPathSVT(gng, fout, NULL);
        fclose(fout);
    }
}
