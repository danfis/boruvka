/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <fermat/rrt.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/dbg.h>

struct _alg_t {
    fer_rrt_t *rrt;
    fer_timer_t timer;
    fer_rand_mt_t *rand;
    fer_vec2_t conf;
    fer_vec2_t new_conf;

    fer_vec2_t start, goal;

    size_t max_nodes;
    size_t find_path;
    fer_real_t step;

    unsigned long evals;
};
typedef struct _alg_t alg_t;


static int terminate(const fer_rrt_t *rrt, void *data);
static void callback(const fer_rrt_t *rrt, void *data);
static const fer_vec_t *rand_conf(const fer_rrt_t *rrt, void *data);
static const fer_vec_t *expand(const fer_rrt_t *rrt,
                               const fer_rrt_node_t *n,
                               const fer_vec_t *rand, void *data);
static int eval(const fer_vec2_t *conf, alg_t *alg);
static void printPath(alg_t *alg, FILE *out);

int main(int argc, char *argv[])
{
    fer_rrt_ops_t ops;
    fer_rrt_params_t params;
    fer_real_t aabb[4] = { -5, 5, -5, 5 };
    alg_t alg;

    if (argc != 2){
        fprintf(stderr, "Usage: %s max_nodes\n", argv[0]);
        return -1;
    }

    ferRRTOpsInit(&ops);
    ferRRTParamsInit(&params);

    params.d = 2;
    //params.cells.num_cells = 40000;
    params.cells.num_cells = 0;
    params.cells.max_dens  = 1;
    params.cells.expand_rate = 2.;
    params.cells.aabb = aabb;

    ops.data      = &alg;
    ops.random    = rand_conf;
    ops.expand    = expand;
    ops.terminate = terminate;
    ops.callback  = callback;
    ops.callback_period = 500;

    alg.max_nodes = atoi(argv[1]);
    alg.find_path = 5000;
    ferVec2Set(&alg.start, FER_REAL(-4.), FER_REAL(-4.));
    ferVec2Set(&alg.goal, FER_REAL(1.5), FER_REAL(4.5));
    alg.evals = 0;
    alg.step = 0.01;

    alg.rand = ferRandMTNewAuto();

    alg.rrt = ferRRTNew(&ops, &params);

    ferTimerStart(&alg.timer);
    ferRRTRunBasic(alg.rrt, (fer_vec_t *)&alg.start);
    callback(alg.rrt, &alg);
    fprintf(stderr, "\n");

    printPath(&alg, stdout);
    ferRRTDumpSVT(alg.rrt, stdout, "Result");

    ferRRTDel(alg.rrt);
    ferRandMTDel(alg.rand);
    
    fprintf(stderr, "Evals: %ld\n", alg.evals);
    fprintf(stdout, "# Evals: %ld\n", alg.evals);

    return 0;
}

static int terminate(const fer_rrt_t *rrt, void *data)
{
    alg_t *alg = (alg_t *)data;
    size_t nodes;
    const fer_rrt_node_t *last;
    const fer_vec2_t *lastv;
    fer_real_t dist;

    nodes = ferRRTNodesLen(rrt);
    if (nodes >= alg->max_nodes)
        return 1;

    last = ferRRTNodeLast(rrt);
    lastv = (const fer_vec2_t *)ferRRTNodeConf(last);
    dist = ferVec2Dist(lastv, &alg->goal);
    //DBG("%f %lx", dist, (long)last);
    if (dist < alg->step)
        return 1;

    return 0;
}

static void callback(const fer_rrt_t *rrt, void *data)
{
    alg_t *alg = (alg_t *)data;

    ferTimerStopAndPrintElapsed(&alg->timer, stderr, " n: %d / %d\r",
                                (int)ferRRTNodesLen(alg->rrt),
                                (int)alg->max_nodes);
    fflush(stderr);
}

static const fer_vec_t *rand_conf(const fer_rrt_t *rrt, void *data)
{
    alg_t *alg = (alg_t *)data;
    fer_real_t x, y;

    x = ferRandMT(alg->rand, -5, 5);
    y = ferRandMT(alg->rand, -5, 5);

    //fprintf(stderr, "conf: %g %g\n", x, y);
    ferVec2Set(&alg->conf, x, y);

    return (const fer_vec_t *)&alg->conf;
}

static const fer_vec_t *expand(const fer_rrt_t *rrt,
                               const fer_rrt_node_t *node_near,
                               const fer_vec_t *rand, void *data)
{
    alg_t *alg = (alg_t *)data;
    fer_vec2_t move;
    const fer_vec2_t *near;

    near = (const fer_vec2_t *)ferRRTNodeConf(node_near);

    ferVec2Sub2(&move, (const fer_vec2_t *)rand, (const fer_vec2_t *)near);
    ferVec2Normalize(&move);
    ferVec2Scale(&move, alg->step);

    ferVec2Add2(&alg->new_conf, (const fer_vec2_t *)near, &move);

    if (eval(&alg->new_conf, alg) == FER_RRT_OBST)
        return NULL;
    //DBG("expand: %f %f", ferVec2X(&alg->new_conf), ferVec2Y(&alg->new_conf));
    return (const fer_vec_t *)&alg->new_conf;
}

static int eval(const fer_vec2_t *conf, alg_t *alg)
{
    fer_real_t x, y;

    x = ferVec2X(conf);
    y = ferVec2Y(conf);

    alg->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -0.15 && x < 0.15)
            || (y > 4 && x > -2 && x < 2)){
        //fprintf(stderr, "eval: FREE\n");
        return FER_RRT_FREE;
    }
    //fprintf(stderr, "eval: OBST\n");
    return FER_RRT_OBST;
}

static void printPath(alg_t *alg, FILE *out)
{
    fer_list_t *item;
    const fer_rrt_node_t *last_node, *init_node, *goal_node;
    const fer_vec2_t *last;
    fer_rrt_node_t *n;
    size_t id;
    fer_list_t path;

    last_node = ferRRTNodeLast(alg->rrt);
    last = (const fer_vec2_t *)ferRRTNodeConf(last_node);
    if (ferVec2Dist(last, &alg->goal) > alg->step)
        return;

    init_node = ferRRTNodeInitial(alg->rrt);
    goal_node = ferRRTNodeNew(alg->rrt, (const fer_vec_t *)&alg->goal,
                              last_node);

    ferListInit(&path);
    ferRRTFindPath(alg->rrt, init_node, goal_node, &path);

    fprintf(out, "------\n");
    fprintf(out, "Name: path\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Edge color: 0.8 0 0\n");

    fprintf(out, "Points:\n");
    FER_LIST_FOR_EACH(&path, item){
        n = FER_LIST_ENTRY(item, fer_rrt_node_t, path);
        ferVec2Print((fer_vec2_t *)n->conf, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Edges:\n");
    id = 0;
    FER_LIST_FOR_EACH(&path, item){
        if (ferListNext(item) == &path)
            break;

        n = FER_LIST_ENTRY(item, fer_rrt_node_t, path);
        fprintf(out, "%d %d\n", (int)id, (int)id + 1);
        id++;
    }

    fprintf(out, "------\n");
}
