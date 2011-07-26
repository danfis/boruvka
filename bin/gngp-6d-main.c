#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>
#include <fermat/cd.h>

#define FREE FER_GNG_PLAN_FREE
#define OBST FER_GNG_PLAN_OBST

fer_gng_plan_t *gng;
fer_gng_plan_ops_t ops;
fer_gng_plan_params_t params;
size_t max_nodes;
fer_rand_mt_t *rand_mt;
fer_timer_t timer;
fer_vec_t *is, *start, *goal;

int dump_period = 10;
const char *dump_prefix = NULL;

fer_real_t aabb[12] = { -54, 85, -45, 93, -48, 90,
                        -M_PI, M_PI, -M_PI, M_PI, -M_PI, M_PI };

fer_cd_t *cd;
fer_cd_geom_t *map, *robot;
fer_vec3_t cdtr;
fer_mat3_t cdrot;


static int terminate(void *data);
static void callback(void *data);
static const void *inputSignal(void *data);
static int eval(const fer_vec_t *w, void *);

static void dump(void);


int main(int argc, char *argv[])
{
    if (argc < 3){
        fprintf(stderr, "Usage: %s max_nodes scene [dump_prefix dump_period]\n", argv[0]);
        return -1;
    }

    if (argc >= 4)
        dump_prefix = argv[3];
    if (argc >= 5)
        dump_period = atoi(argv[4]);

    is    = ferVecNew(6);
    start = ferVecNew(6);
    goal  = ferVecNew(6);

    max_nodes = atoi(argv[1]);

    ferGNGPlanOpsInit(&ops);
    ferGNGPlanParamsInit(&params);

    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback     = callback;
    ops.callback_period = 100;

    params.dim = 6;
    params.max_dist = 0.12;
    params.min_nodes = 100;
    params.start = start;
    params.goal  = goal;
    params.cells.d = 6;
    params.cells.aabb = aabb;
    params.cells.max_dens = 1.;
    params.cells.expand_rate = 1.1;
    params.cells.approx = 1;


    cd = ferCDNew();
    ferCDSetBuildFlags(cd, FER_CD_FIT_NAIVE
                            | FER_CD_FIT_NAIVE_NUM_ROT(5)
                            | FER_CD_BUILD_PARALLEL(8));
    map = ferCDGeomNew(cd);
    robot = ferCDGeomNew(cd);


    if (strcmp(argv[2], "alpha") == 0){
        ferVecSetZero(6, start);
        ferVecSet(start, 0, -2.6);
        ferVecSet(start, 1, 2.34);
        ferVecSet(start, 2, 0.71);
        ferVecSetZero(6, goal);
        ferVecSet(goal, 0, -8);
        ferVecSet(goal, 1, 57);
        ferVecSet(goal, 2, -1.4);

        aabb[0] = -54;
        aabb[1] = 85;
        aabb[2] = -45;
        aabb[3] = 93;
        aabb[4] = -48;
        aabb[5] = 90;

        params.max_dist = 0.12;

        ferCDGeomAddTrisFromRaw(cd, map, "/home/danfis/dev/imr-data/trimesh/alpha-puzzle/map.raw");
        ferCDGeomAddTrisFromRaw(cd, robot, "/home/danfis/dev/imr-data/trimesh/alpha-puzzle/robot.raw");

    }else if (strcmp(argv[2], "room") == 0){
        ferVecSetZero(6, start);
        ferVecSet(start, 0, -7.8);
        ferVecSet(start, 1, 6.9);
        ferVecSet(start, 2, 2.36);
        ferVecSetZero(6, goal);
        ferVecSet(goal, 0, -7.8);
        ferVecSet(goal, 1, -10);
        ferVecSet(goal, 2, 2.36);

        aabb[0] = -10;
        aabb[1] = 15;
        aabb[2] = -15;
        aabb[3] = 11;
        aabb[4] = 0;
        aabb[5] = 6;

        params.max_dist = 0.15;

        ferCDGeomAddTrisFromRaw(cd, map, "/home/danfis/dev/imr-data/trimesh/room/map.raw");
        ferCDGeomAddTrisFromRaw(cd, robot, "/home/danfis/dev/imr-data/trimesh/room/robot.raw");
    }


    ferCDGeomBuild(cd, map);
    ferCDGeomBuild(cd, robot);


    rand_mt = ferRandMTNewAuto();
    gng = ferGNGPlanNew(&ops, &params);

    ferTimerStart(&timer);
    ferGNGPlanRun(gng);
    ferTimerStopAndPrintElapsed(&timer, stderr, "\r\n");

    ferGNGPlanDumpPath(gng, stdout);

    ferGNGPlanDel(gng);

    ferRandMTDel(rand_mt);
    
    ferVecDel(is);
    ferVecDel(start);
    ferVecDel(goal);

    ferCDDel(cd);

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
    ferTimerStopAndPrintElapsed(&timer, stderr, " nodes: %08d, evals: %010lu\r",
                                (int)nodes_len, ferGNGPlanEvals(gng));
}

static const void *inputSignal(void *data)
{
    int i;

    for (i = 0; i < 6; i++){
        ferVecSet(is, i, ferRandMT(rand_mt, aabb[2 * i], aabb[2 * i + 1]));
    }

    return is;
}

static int eval(const fer_vec_t *conf, void *_)
{
    ferVec3Set(&cdtr, ferVecGet(conf, 0),
                      ferVecGet(conf, 1),
                      ferVecGet(conf, 2));
    ferMat3SetRot3D(&cdrot, ferVecGet(conf, 3),
                            ferVecGet(conf, 4),
                            ferVecGet(conf, 5));
    ferCDGeomSetTr(cd, robot, &cdtr);
    ferCDGeomSetRot(cd, robot, &cdrot);

    if (ferCDGeomCollide(cd, map, robot))
        return OBST;
    return FREE;
}



static void dump(void)
{
    FILE *fout;
    char fn[1000];
    static int counter = 0;

    sprintf(fn, "%s%010d.svt", dump_prefix, counter++);
    fout = fopen(fn, "w");
    if (fout){
        ferGNGPlanDumpPath(gng, fout);
        fclose(fout);
    }
}
