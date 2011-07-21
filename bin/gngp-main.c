#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>

#define FREE FER_GNG_PLAN_FREE
#define OBST FER_GNG_PLAN_OBST
#include "plan-eval.c"

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
    ops.callback_period = 100;

    params.dim = 2;
    params.max_dist = 0.01;
    params.min_nodes = atoi(argv[2]);
    params.min_nodes_inc = 10;
    params.start = ferVecNew(2);
    params.goal  = ferVecNew(2);
    params.cells.d = 2;
    params.cells.aabb = aabb;
    params.cells.max_dens = 2;
    params.cells.expand_rate = 2;

    setUpScene(argv[3], &ops.eval,
               (fer_vec_t *)params.start, (fer_vec_t *)params.goal,
               &params.max_dist);

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
    ferVecDel((fer_vec_t *)params.start);
    ferVecDel((fer_vec_t *)params.goal);

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
    ferTimerStopAndPrintElapsed(&timer, stderr, " %08d\r", (int)nodes_len);
}

static const void *inputSignal(void *data)
{
    fer_real_t x, y;

    x = ferRandMT(rand_mt, -5, 5);
    y = ferRandMT(rand_mt, -5, 5);
    ferVec2Set((fer_vec2_t *)is, x, y);
    return is;
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
