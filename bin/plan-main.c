/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <fermat/gnnp.h>
#include <fermat/rrt.h>
#include <fermat/timer.h>
#include <fermat/cfg.h>
#include <fermat/opts.h>
#include <fermat/dbg.h>
#include "cfg-map.h"


fer_vec_t *is;
fer_vec_t *init, *goal;
unsigned long evals = 0UL;


int list_robots;
const char *robot_name = NULL;
const char *method_name = NULL;
long callback_period = 0;
fer_real_t max_time = 3600.;
fer_real_t elapsed_time = 0.;
int rmax = 0;
fer_real_t h = 0.25;
int use_rot = 0;
int dbg_dump = 0;
int alg_num = 0;
int rrt_goal_conf = 1000;
fer_timer_t timer;


static void setUpNN(fer_nn_params_t *nn);
_fer_inline void updateTimer(void);


static fer_gnnp_t *gnnp = NULL;
static int gnnpTerminate(fer_gnnp_t *nn, void *data);
static const fer_vec_t *gnnpInputSignal(fer_gnnp_t *nn, void *data);
static int gnnpEval(fer_gnnp_t *nn, const fer_vec_t *conf, void *data);
static void gnnpCallback(fer_gnnp_t *nn, void *data);
static void gnnpInit(void);
static void gnnpDestroy(void);
static int gnnpRun(fer_list_t *path);
static void gnnpDump(int ret, fer_list_t *path);
static long gnnpNodesLen(void);


static fer_rrt_t *rrt = NULL;
static int rrtTerminate(const fer_rrt_t *rrt, void *data);
static const fer_vec_t *rrtExpand(const fer_rrt_t *rrt,
                                  const fer_rrt_node_t *n,
                                  const fer_vec_t *rand, void *data);
static const fer_vec_t *rrtConf(const fer_rrt_t *rrt, void *data);
static void rrtCallback(const fer_rrt_t *rrt, void *data);
static void rrtInit(void);
static void rrtDestroy(void);
static int rrtRun(fer_list_t *path);
static void rrtDump(int ret, fer_list_t *path);
static long rrtNodesLen(void);

static int rrtConnectTerminateExpand(const fer_rrt_t *rrt,
                                     const fer_rrt_node_t *start,
                                     const fer_rrt_node_t *last,
                                     const fer_vec_t *rand,
                                     void *data);
static void rrtConnectInit(void);
static int rrtConnectRun(fer_list_t *path);


struct alg_t {
    void (*init)(void);
    void (*destroy)(void);
    int (*run)(fer_list_t *path);
    void (*dump)(int ret, fer_list_t *path);
    long (*nodes_len)(void);
};
#define ALG_GNNP        0
#define ALG_RRT         1
#define ALG_RRT_CONNECT 2
#define ALG_LEN         3

#define ALG_DEF(name) \
    { .init      = name ## Init, \
      .destroy   = name ## Destroy, \
      .run       = name ## Run, \
      .dump      = name ## Dump, \
      .nodes_len = name ## NodesLen }
const char *methods[ALG_LEN] = { "gnnp", "rrt", "rrt-connect" };
struct alg_t algs[ALG_LEN] = {
    ALG_DEF(gnnp),
    ALG_DEF(rrt),
    { .init      = rrtConnectInit, \
      .destroy   = rrtDestroy, \
      .run       = rrtConnectRun, \
      .dump      = rrtDump, \
      .nodes_len = rrtNodesLen }
};

int opts(int *argc, char *argv[])
{
    int i, help;
    int ok = 1;

    ferOptsAddDesc("help", 0x0, FER_OPTS_NONE, (void *)&help, NULL,
                   "Print this help");
    ferOptsAddDesc("robot", 'r', FER_OPTS_STR, (void *)&robot_name, NULL,
                   "The robot with the specified name will be used (default: none)");
    ferOptsAddDesc("method", 'm', FER_OPTS_STR, (void *)&method_name, NULL,
                   "Choose the planning method: gnnp, rrt, rrt-connect (default: gnnp)");
    ferOptsAddDesc("max-time", 0x0, FER_OPTS_REAL, (void *)&max_time, NULL,
                   "Maximal time in seconds (default: 3600)");
    ferOptsAddDesc("rmax", 0x0, FER_OPTS_INT, (void *)&rmax, NULL,
                   "Rmax parameter (default: 2^(dim + 1))");
    ferOptsAddDesc("rot", 0x0, FER_OPTS_NONE, (void *)&use_rot, NULL,
                   "Also rotation is considered");
    ferOptsAddDesc("robots", 0x0, FER_OPTS_NONE, (void *)&list_robots, NULL,
                   "Print list of available robots");
    ferOptsAddDesc("cb-period", 0x0, FER_OPTS_LONG, (void *)&callback_period, NULL,
                   "Callback period");
    ferOptsAddDesc("dbg-dump", 0x0, FER_OPTS_NONE, (void *)&dbg_dump, NULL,
                   "Enables debug dumps into dbg/ directory in each callback");
    ferOptsAddDesc("rrt-goal-conf", 0x0, FER_OPTS_INT, (void *)&rrt_goal_conf, NULL,
                   "How often should be goal configuration presented to the algorithm (default 1000)");


    if (ferOpts(argc, argv) != 0)
        ok = 0;

    if (method_name){
        for (i = 0; i < ALG_LEN; i++){
            if (strcmp(method_name, methods[i]) == 0){
                alg_num = i;
                break;
            }
        }
    }

    if (callback_period == 0){
        if (alg_num == ALG_GNNP){
            callback_period = 100000;
        }else if (alg_num == ALG_RRT || alg_num == ALG_RRT_CONNECT){
            callback_period = 10000;
        }
    }


    if (help || !ok || *argc != 2 || (!list_robots && (robot_name == NULL || method_name == NULL))){
        fprintf(stderr, "Usage: %s [ OPTIONS ] [-r robot | --robots] -m method cfg_file\n", argv[0]);
        fprintf(stderr, "  OPTIONS:\n");
        ferOptsPrint(stderr, "    ");
        return -1;
    }

    if (ok)
        return 0;
    return -1;
}

int main(int argc, char *argv[])
{
    fer_list_t path;
    int ret = 0;

    if (opts(&argc, argv) != 0)
        return -1;

    if (ferCfgMapInit(argv[1]) != 0)
        return -1;

    if (use_rot || ferCfgMapDim() == 3){
        ferCfgMapUseRot();
    }

    if (rmax == 0){
        rmax = powl(2, ferCfgMapConfDim() + 1);
    }

    if (list_robots){
        ferCfgMapListRobots(stdout);
        ferCfgMapDestroy();
        return 0;
    }

    is   = ferVecNew(6);
    init = ferVecNew(6);
    goal = ferVecNew(6);
    ferVecSetZero(6, is);
    ferVecSetZero(6, init);
    ferVecSetZero(6, goal);

    // load robot with parameters
    if (ferCfgMapRobot(robot_name, &h, init, goal) != 0){
        ferVecDel(is);
        ferVecDel(init);
        ferVecDel(goal);
        ferCfgMapDestroy();
        return -1;
    }

    if (ferCfgMapCollide(init)
            || ferCfgMapCollide(goal)){
        if (ferCfgMapCollide(init))
            fprintf(stderr, "Error: init configuration is OBST.\n");
        if (ferCfgMapCollide(goal))
            fprintf(stderr, "Error: goal configuration is OBST.\n");

        if (dbg_dump){
            ferCfgMapDumpSVT(stdout, NULL);
            ferCfgMapRobotDumpSVT(init, stdout, "Init");
            ferCfgMapRobotDumpSVT(goal, stdout, "Goal");
        }

        ferVecDel(is);
        ferVecDel(init);
        ferVecDel(goal);
        ferCfgMapDestroy();
        return -1;
    }

    algs[alg_num].init();

    ferListInit(&path);
    ferTimerStart(&timer);
    ret = algs[alg_num].run(&path);
    ferTimerStop(&timer);
    elapsed_time += ferTimerElapsedInSF(&timer);

    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "nodes: %ld\n", algs[alg_num].nodes_len());
    fprintf(stderr, "evals: %ld\n", (long)evals);
    fprintf(stderr, "Time: %f s\n", (float)elapsed_time);
    fflush(stderr);

    algs[alg_num].dump(ret, &path);
    algs[alg_num].destroy();


    ferVecDel(is);
    ferVecDel(init);
    ferVecDel(goal);

    ferCfgMapDestroy();

    return 0;
}

static void setUpNN(fer_nn_params_t *nn)
{
    nn->type = FER_NN_GUG;

    nn->gug.dim = ferCfgMapConfDim();
    nn->gug.max_dens = 1.;
    nn->gug.expand_rate = 1.3;
    nn->gug.aabb = (fer_real_t *)ferCfgMapAABB();

    if (ferCfgMapConfDim() == 6)
        nn->type = FER_NN_VPTREE;
}

_fer_inline void updateTimer(void)
{
    ferTimerStop(&timer);
    elapsed_time += ferTimerElapsedInSF(&timer);
    ferTimerStart(&timer);
}

/*** GNNP ***/
static int gnnpTerminate(fer_gnnp_t *nn, void *data)
{
    //updateTimer();
    if (elapsed_time > max_time)
        return 1;
    return 0;
}

static const fer_vec_t *gnnpInputSignal(fer_gnnp_t *nn, void *data)
{
    return ferCfgMapConf();
}

static int gnnpEval(fer_gnnp_t *nn, const fer_vec_t *conf, void *data)
{
    evals += 1UL;
    return !ferCfgMapCollide(conf);
}

static void gnnpDumpDBG(fer_gnnp_t *nn, int c)
{
    char fn[100];
    FILE *fout;

    snprintf(fn, 100, "dbg/map-%06d.svt", c);
    fout = fopen(fn, "w");
    if (fout){
        ferGNNPDumpSVT(nn, fout, NULL);
        ferCfgMapDumpSVT(fout, NULL);
        ferCfgMapRobotDumpSVT(init, fout, "Init");
        ferCfgMapRobotDumpSVT(goal, fout, "Goal");
        fclose(fout);
    }
}

static void gnnpCallback(fer_gnnp_t *nn, void *data)
{
    static int c = 0;

    if (dbg_dump)
        gnnpDumpDBG(nn, c);

    updateTimer();
    fprintf(stderr, "step %d, nodes: %d, evals: %ld  [%f s]\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals,
            (float)elapsed_time);

    c++;
}

static void gnnpPrintPath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    if (ferCfgMapConfDim() > 3){
        FER_LIST_FOR_EACH(path, item){
            n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
            printf("#P: ");
            ferVecPrint(ferCfgMapConfDim(), n->w, stdout);
            printf("\n");
        }
        return;
    }

    printf("----\n");
    printf("Name: PATH\n");

    printf("Points off: 1\n");
    printf("Edge color: 1 0 0\n");
    printf("Edge width: 2\n");
    printf("Points:\n");
    len = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
        ferVecPrint(ferCfgMapConfDim(), n->w, stdout);
        printf("\n");
        len++;
    }

    printf("Edges:\n");
    for (i = 0; i < len - 1; i++){
        printf("%d %d\n", i, i + 1);
    }

    printf("----\n");
    printf("----\n");
}

static void gnnpPrintSolutionVideoPath(fer_list_t *path, fer_list_t *end, FILE *out)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    if (ferListNext(path) == end)
        return;

    fprintf(out, "----\n");
    fprintf(out, "Name: PATH\n");
    fprintf(out, "Points off: 1\n");
    fprintf(out, "Edge color: 1 0 0\n");
    fprintf(out, "Edge width: 1\n");

    fprintf(out, "Points:\n");
    len = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);

        ferVecPrint(2, n->w, out);
        fprintf(out, "\n");

        len++;
        if (item == end)
            break;
    }

    fprintf(out, "Edges:\n");
    for (i = 0; i < len - 1; i++){
        fprintf(out, "%d %d\n", i, i + 1);
    }

    fprintf(out, "----\n");
}

static void gnnpPrintSolutionVideo(fer_gnnp_t *nn, fer_list_t *path)
{
    char fn[300];
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i;
    FILE *out;


    i = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
        snprintf(fn, 300, "gen-video/map-%06d.svt", i);
        out = fopen(fn, "w");
        if (out){
            ferCfgMapDumpSVT(out, "Map");
            ferCfgMapRobotDumpSVT(n->w, out, "Robot");
            gnnpPrintSolutionVideoPath(path, item, out);
            fclose(out);
        }
        i++;
    }
}

static void gnnpInit(void)
{
    fer_gnnp_ops_t ops;
    fer_gnnp_params_t params;

    ferGNNPOpsInit(&ops);
    ops.terminate    = gnnpTerminate;
    ops.input_signal = gnnpInputSignal;
    ops.eval         = gnnpEval;
    ops.callback        = gnnpCallback;
    ops.callback_period = callback_period;

    ferGNNPParamsInit(&params);
    params.dim  = ferCfgMapConfDim();
    params.rmax = rmax;
    params.h    = h;
    setUpNN(&params.nn);

    gnnp = ferGNNPNew(&ops, &params);
}

static void gnnpDestroy(void)
{
    ferGNNPDel(gnnp);
}

static int gnnpRun(fer_list_t *path)
{
    return ferGNNPFindPath(gnnp, init, goal, path);
}

static void gnnpDump(int ret, fer_list_t *path)
{
    if (ret == 0){
        gnnpPrintPath(gnnp, path);
        gnnpPrintSolutionVideo(gnnp, path);
    }
    ferGNNPDumpSVT(gnnp, stdout, NULL);
    ferCfgMapDumpSVT(stdout, NULL);
    ferCfgMapRobotDumpSVT(init, stdout, "Init");
    ferCfgMapRobotDumpSVT(goal, stdout, "Goal");
}

static long gnnpNodesLen(void)
{
    return ferGNNPNodesLen(gnnp);
}
/*** GNNP END ***/


/*** RRT ***/
static const fer_rrt_node_t *rrt_last = NULL;
static fer_real_t h2 = 0.;
static FER_VEC(rrt_move, 6);
static FER_VEC(rrt_new_conf, 6);
static fer_real_t near_dist = FER_REAL_MAX;
static int rrt_found = 0;
static int rrtTerminate(const fer_rrt_t *rrt, void *data)
{
    const fer_vec_t *lconf;
    fer_real_t dist;

    //updateTimer();
    if (elapsed_time > max_time)
        return 1;

    rrt_last = ferRRTNodeLast(rrt);
    lconf = ferRRTNodeConf(rrt_last);
    dist = ferVecDist2(ferCfgMapConfDim(), lconf, goal);
    if (dist < near_dist)
        near_dist = dist;
    if (dist < h2){
        rrt_found = 1;
        return 1;
    }

    return 0;
}

static const fer_vec_t *rrtExpand(const fer_rrt_t *rrt,
                                  const fer_rrt_node_t *n,
                                  const fer_vec_t *rand, void *data)
{
    const fer_vec_t *near;
    fer_real_t len;

    near = ferRRTNodeConf(n);

    ferVecSub2(rrt->params.dim, rrt_move, rand, near);
    len = ferVecLen(rrt->params.dim, rrt_move);
    ferVecScale(rrt->params.dim, rrt_move, h * ferRecp(len));

    ferVecAdd2(rrt->params.dim, rrt_new_conf, near, rrt_move);

    evals += 1UL;
    if (ferCfgMapCollide(rrt_new_conf))
        return NULL;
    return rrt_new_conf;
}

static const fer_vec_t *rrtConf(const fer_rrt_t *rrt, void *data)
{
    static int counter = 0;
    if (counter++ == rrt_goal_conf){
        counter = 0;
        return goal;
    }
    return ferCfgMapConf();
}

static void rrtDumpDBG(int c)
{
    char fn[100];
    FILE *fout;

    snprintf(fn, 100, "dbg/map-%06d.svt", c);
    fout = fopen(fn, "w");
    if (fout){
        ferRRTDumpSVT(rrt, fout, NULL);
        ferCfgMapDumpSVT(fout, NULL);
        ferCfgMapRobotDumpSVT(init, fout, "Init");
        ferCfgMapRobotDumpSVT(goal, fout, "Goal");
        fclose(fout);
    }
}

static void rrtCallback(const fer_rrt_t *r, void *data)
{
    static unsigned long c = 0UL;

    if (dbg_dump){
        rrtDumpDBG(c);
    }

    updateTimer();
    fprintf(stderr, "step %ld, nodes: %d, evals: %ld, nearest: %f  [%f s]\n",
            c, (int)ferRRTNodesLen(rrt), (long)evals, FER_SQRT(near_dist),
            (float)elapsed_time);
    c++;
}

static void rrtInit(void)
{
    fer_rrt_ops_t ops;
    fer_rrt_params_t params;

    ferRRTOpsInit(&ops);
    ops.terminate = rrtTerminate;
    ops.expand    = rrtExpand;
    ops.random    = rrtConf;
    ops.callback  = rrtCallback;
    ops.callback_period = callback_period;

    ferRRTParamsInit(&params);
    params.dim = ferCfgMapConfDim();
    setUpNN(&params.nn);

    rrt = ferRRTNew(&ops, &params);

    h2 = h * h;
}

static void rrtDestroy(void)
{
    ferRRTDel(rrt);
}

static int rrtRun(fer_list_t *path)
{
    int ret;
    const fer_rrt_node_t *init_node;

    ferRRTRunBasic(rrt, init);
    init_node = ferRRTNodeInitial(rrt);
    if (rrt_found){
        ferListInit(path);
        ret = ferRRTFindPath(rrt, init_node, rrt_last, path);
        return ret;
    }

    return -1;
}

static void rrtPrintPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    const fer_rrt_node_t *last_node;
    fer_rrt_node_t *n;
    size_t id;

    if (ferCfgMapConfDim() > 3){
        FER_LIST_FOR_EACH(path, item){
            n = FER_LIST_ENTRY(item, fer_rrt_node_t, path);
            fprintf(out, "#P: ");
            ferVecPrint(rrt->params.dim, n->conf, out);
            fprintf(out, "\n");
        }
        return;
    }

    last_node = ferRRTNodeLast(rrt);
    ferRRTNodeNew(rrt, goal, last_node);

    fprintf(out, "------\n");
    fprintf(out, "Name: PATH\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Edge color: 0.8 0 0\n");

    fprintf(out, "Points:\n");
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_rrt_node_t, path);
        ferVecPrint(rrt->params.dim, n->conf, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Edges:\n");
    id = 0;
    FER_LIST_FOR_EACH(path, item){
        if (ferListNext(item) == path)
            break;

        fprintf(out, "%d %d\n", (int)id, (int)id + 1);
        id++;
    }

    fprintf(out, "------\n");
}

static void rrtPrintSolutionVideo(fer_list_t *path)
{
    char fn[300];
    fer_list_t *item;
    fer_rrt_node_t *n;
    int i;
    FILE *out;

    i = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_rrt_node_t, path);
        snprintf(fn, 300, "rrt-gen-video/map-%06d.svt", i);
        out = fopen(fn, "w");
        if (out){
            ferCfgMapDumpSVT(out, "Map");
            ferCfgMapRobotDumpSVT(n->conf, out, "Robot");
            fclose(out);
        }
        i++;
    }
}
static void rrtDump(int ret, fer_list_t *path)
{
    if (ret == 0){
        rrtPrintPath(path, stdout);
        rrtPrintSolutionVideo(path);
    }
    ferRRTDumpSVT(rrt, stdout, NULL);
    ferCfgMapDumpSVT(stdout, NULL);
    ferCfgMapRobotDumpSVT(init, stdout, "Init");
    ferCfgMapRobotDumpSVT(goal, stdout, "Goal");
}

static long rrtNodesLen(void)
{
    return ferRRTNodesLen(rrt);
}

/*** RRT END ***/

/*** RRT CONNECT ***/
static int rrtConnectTerminateExpand(const fer_rrt_t *rrt,
                                     const fer_rrt_node_t *start,
                                     const fer_rrt_node_t *last,
                                     const fer_vec_t *rand,
                                     void *data)
{
    const fer_vec_t *n;
    fer_real_t dist;

    n = ferRRTNodeConf(last);
    dist = ferVecDist2(rrt->params.dim, n, rand);

    return dist <= h2;
}

static void rrtConnectInit(void)
{
    fer_rrt_ops_t ops;
    fer_rrt_params_t params;

    ferRRTOpsInit(&ops);
    ops.terminate = rrtTerminate;
    ops.terminate_expand = rrtConnectTerminateExpand;
    ops.expand    = rrtExpand;
    ops.random    = rrtConf;
    ops.callback  = rrtCallback;
    ops.callback_period = callback_period;

    ferRRTParamsInit(&params);
    params.dim = ferCfgMapConfDim();
    setUpNN(&params.nn);

    rrt = ferRRTNew(&ops, &params);

    h2 = h * h;
}

static int rrtConnectRun(fer_list_t *path)
{
    int ret;
    const fer_rrt_node_t *init_node;

    ferRRTRunConnect(rrt, init);
    init_node = ferRRTNodeInitial(rrt);
    ferListInit(path);
    ret = ferRRTFindPath(rrt, init_node, rrt_last, path);
    return ret;
}
/*** RRT CONNECT END ***/
