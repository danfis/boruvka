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


static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *conf(void *data);
static const fer_vec_t *newConf(const fer_vec_t *near,
                                const fer_vec_t *rand, void *data);
static int eval(const fer_vec2_t *conf, alg_t *alg);
static void printPath(fer_list_t *path, FILE *out);

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
    ops.conf      = conf;
    ops.terminate = terminate;
    ops.new_conf  = newConf;
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
    ferRRTRun(alg.rrt, (fer_vec_t *)&alg.start);
    callback(&alg);
    fprintf(stderr, "\n");


    ferRRTDumpSVT(alg.rrt, stdout, "Result");

    ferRRTDel(alg.rrt);
    ferRandMTDel(alg.rand);
    
    fprintf(stderr, "Evals: %ld\n", alg.evals);
    fprintf(stdout, "# Evals: %ld\n", alg.evals);

    return 0;
}

static int terminate(void *data)
{
    alg_t *alg = (alg_t *)data;
    size_t nodes;
    const fer_rrt_node_t *last, *init, *goal;
    int res;
    fer_list_t path;

    nodes = ferRRTNodesLen(alg->rrt);

    last = ferRRTNodeLast(alg->rrt);
    if (ferVec2Dist((const fer_vec2_t *)ferRRTNodeConf(last), &alg->goal) < alg->step){
        ferListInit(&path);

        init = ferRRTNodeInitial(alg->rrt);
        goal = ferRRTNodeNew(alg->rrt, (const fer_vec_t *)&alg->goal, last);

        res = ferRRTFindPath(alg->rrt, init, goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferRRTNodesLen(alg->rrt));
            printPath(&path, stdout);

            return 1;
        }
    }

    return nodes >= alg->max_nodes;
}

static void callback(void *data)
{
    alg_t *alg = (alg_t *)data;

    ferTimerStopAndPrintElapsed(&alg->timer, stderr, " n: %d / %d\r",
                                (int)ferRRTNodesLen(alg->rrt),
                                (int)alg->max_nodes);
    fflush(stderr);
}

static const fer_vec_t *conf(void *data)
{
    alg_t *alg = (alg_t *)data;
    fer_real_t x, y;

    do {
        x = ferRandMT(alg->rand, -5, 5);
        y = ferRandMT(alg->rand, -5, 5);

    //fprintf(stderr, "conf: %g %g\n", x, y);

        ferVec2Set(&alg->conf, x, y);
    } while (eval(&alg->conf, alg) == FER_RRT_OBST);

    return (const fer_vec_t *)&alg->conf;
}

static const fer_vec_t *newConf(const fer_vec_t *near,
                                const fer_vec_t *rand, void *data)
{
    alg_t *alg = (alg_t *)data;
    fer_vec2_t move;

    ferVec2Sub2(&move, (const fer_vec2_t *)rand, (const fer_vec2_t *)near);
    ferVec2Scale(&move, alg->step);

    ferVec2Add2(&alg->new_conf, (const fer_vec2_t *)near, &move);

    if (eval(&alg->new_conf, alg) == FER_RRT_OBST)
        return NULL;
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

static void printPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    fer_rrt_node_t *n;
    size_t id;

    fprintf(out, "------\n");
    fprintf(out, "Name: path\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Edge color: 0 0 0\n");

    fprintf(out, "Points:\n");
    ferListForEach(path, item){
        n = ferListEntry(item, fer_rrt_node_t, path);
        ferVec2Print((fer_vec2_t *)n->conf, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Edges:\n");
    id = 0;
    ferListForEach(path, item){
        if (ferListNext(item) == path)
            break;

        n = ferListEntry(item, fer_rrt_node_t, path);
        fprintf(out, "%d %d\n", id, id + 1);
        id++;
    }

    fprintf(out, "------\n");
}

