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
long callback_period = 100000;
long max_steps = 1000000;
int rmax = 0;
fer_real_t h = 0.25;
int use_rot = 0;
int dbg_dump = 0;


static void setUpNN(fer_nn_params_t *nn);


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


struct alg_t {
    void (*init)(void);
    void (*destroy)(void);
    int (*run)(fer_list_t *path);
    void (*dump)(int ret, fer_list_t *path);
    long (*nodes_len)(void);
};
#define ALG_GNNP 0
#define ALG_RRT  1
#define ALG_LEN  2

#define ALG_DEF(name) \
    { .init      = name ## Init, \
      .destroy   = name ## Destroy, \
      .run       = name ## Run, \
      .dump      = name ## Dump, \
      .nodes_len = name ## NodesLen }
struct alg_t algs[ALG_LEN] = {
    ALG_DEF(gnnp),
    ALG_DEF(rrt)
};

int opts(int *argc, char *argv[])
{
    int ok = 1;

    ferOptsAdd("robots", 0x0, FER_OPTS_NONE, (void *)&list_robots, NULL);
    ferOptsAdd("robot", 'r', FER_OPTS_STR, (void *)&robot_name, NULL);
    ferOptsAdd("mehotd", 'm', FER_OPTS_STR, (void *)&method_name, NULL);
    ferOptsAdd("cb-period", 0x0, FER_OPTS_LONG, (void *)&callback_period, NULL);
    ferOptsAdd("dbg-dump", 0x0, FER_OPTS_NONE, (void *)&dbg_dump, NULL);
    ferOptsAdd("max-steps", 0x0, FER_OPTS_LONG, (void *)&max_steps, NULL);
    ferOptsAdd("rmax", 0x0, FER_OPTS_INT, (void *)&rmax, NULL);
    ferOptsAdd("rot", 0x0, FER_OPTS_NONE, (void *)&use_rot, NULL);


    if (ferOpts(argc, argv) != 0)
        ok = 0;

    if (!ok || *argc != 2 || (!list_robots && (robot_name == NULL || method_name == NULL))){
        fprintf(stderr, "Usage: %s [ OPTIONS ] [-r robot | --robots] -m method cfg_file\n", argv[0]);
        fprintf(stderr, "  OPTIONS:\n");
        fprintf(stderr, "    -m/--method  str  Choose the planning method (gnnp,rrt)\n");
        fprintf(stderr, "    -r/--robot   str  The robot with specified name will be used\n");
        fprintf(stderr, "    --robots          Print list of available robots\n");
        fprintf(stderr, "    --cb-period  int  Callback period\n");
        fprintf(stderr, "    --dbg-dump        Enables debug dumps in each callback (dbg/*)\n");
        fprintf(stderr, "    --max-steps  int  Maximal number of steps\n");
        fprintf(stderr, "    --rmax       flt  Rmax parameter\n");
        fprintf(stderr, "    --rot             Also rotation is considered\n");
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
    fer_timer_t timer;
    int alg_num = 0;

    if (opts(&argc, argv) != 0)
        return -1;

    if (ferCfgMapInit(argv[1]) != 0)
        return -1;

    if (use_rot){
        ferCfgMapUseRot();
    }

    if (rmax == 0){
        rmax = powl(2, ferCfgMapConfDim() + 1);
    }

    if (list_robots){
        ferCfgMapListRobots(stderr);
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

    if (strcmp(method_name, "gnnp") == 0){
        alg_num = ALG_GNNP;
    }else if (strcmp(method_name, "rrt") == 0){
        alg_num = ALG_RRT;
    }

    algs[alg_num].init();

    ferListInit(&path);
    ferTimerStart(&timer);
    ret = algs[alg_num].run(&path);
    ferTimerStop(&timer);

    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "nodes: %ld\n", algs[alg_num].nodes_len());
    fprintf(stderr, "evals: %ld\n", (long)evals);
    fprintf(stderr, "Time: %lu us\n", ferTimerElapsedInUs(&timer));
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
}


/*** GNNP ***/
static int gnnpTerminate(fer_gnnp_t *nn, void *data)
{
    static long count = 0;

    ++count;
    if (count == max_steps)
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

    snprintf(fn, 100, "dbg/map-%06d", c);
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

    fprintf(stderr, "step %d, nodes: %d, evals: %ld\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals);

    c++;
}

static void gnnpPrintPath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    if (ferCfgMapConfDim() > 3)
        return;

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
        snprintf(fn, 300, "gen-video/map-%06d", i);
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
static int rrtTerminate(const fer_rrt_t *rrt, void *data)
{
    const fer_vec_t *lconf;

    if (ferRRTNodesLen(rrt) >= max_steps)
        return 1;

    rrt_last = ferRRTNodeLast(rrt);
    lconf = ferRRTNodeConf(rrt_last);
    if (ferVecDist2(ferCfgMapConfDim(), lconf, goal) < h2)
        return 1;

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
    return ferCfgMapConf();
}

static void rrtDumpDBG(int c)
{
    char fn[100];
    FILE *fout;

    snprintf(fn, 100, "dbg/map-%06d", c);
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

    fprintf(stderr, "step %lu, nodes: %d, evals: %ld\n",
            c, (int)ferRRTNodesLen(rrt), (long)evals);
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
    ferListInit(path);
    ret = ferRRTFindPath(rrt, init_node, rrt_last, path);
    return ret;
}

static void rrtPrintPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    const fer_rrt_node_t *last_node;
    fer_rrt_node_t *n;
    size_t id;

    if (ferCfgMapConfDim() > 3)
        return;

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

static void rrtDump(int ret, fer_list_t *path)
{
    if (ret == 0){
        rrtPrintPath(path, stdout);
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
