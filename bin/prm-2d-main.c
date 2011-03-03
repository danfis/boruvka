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

#include <fermat/prm.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>

struct _alg_t {
    fer_prm_t *prm;
    fer_timer_t timer;
    fer_rand_mt_t *rand;
    fer_vec2_t conf;

    fer_vec2_t start, goal;

    size_t max_nodes;
};
typedef struct _alg_t alg_t;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *conf(void *data);
static int eval(const fer_vec_t *conf, void *data);
static int findPath(const fer_vec_t *c1, const fer_vec_t *c2, void *data);
static void printPath(fer_list_t *path, FILE *out);

int main(int argc, char *argv[])
{
    fer_prm_ops_t ops;
    fer_prm_params_t params;
    alg_t alg;

    if (argc != 2){
        fprintf(stderr, "Usage: %s max_nodes\n", argv[0]);
        return -1;
    }

    ferPRMOpsInit(&ops);
    ferPRMParamsInit(&params);

    params.d = 2;
    params.max_dist = 0.001;
    params.max_neighbors = 10;
    params.num_cells = 40000;
    params.aabb[0] = -5;
    params.aabb[1] = 5;
    params.aabb[2] = -5;
    params.aabb[3] = 5;

    ops.data      = &alg;
    ops.conf      = conf;
    ops.terminate = terminate;
    ops.eval      = eval;
    ops.find_path = findPath;
    ops.callback  = callback;
    ops.callback_period = 500;

    alg.max_nodes = atoi(argv[1]);
    ferVec2Set(&alg.start, FER_REAL(-4.), FER_REAL(-4.));
    ferVec2Set(&alg.goal, FER_REAL(1.5), FER_REAL(4.5));

    alg.rand = ferRandMTNewAuto();

    alg.prm = ferPRMNew(&ops, &params);

    ferTimerStart(&alg.timer);
    ferPRMRun(alg.prm);
    callback(&alg);
    fprintf(stderr, "\n");


    ferPRMDumpSVT(alg.prm, stdout, "Result");

    {
        int res;
        fer_list_t path;

        ferListInit(&path);
        res = ferPRMFindPath(alg.prm, (fer_vec_t *)&alg.start,
                                      (fer_vec_t *)&alg.goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferPRMNodesLen(alg.prm));
            printPath(&path, stdout);
        }
    }

    ferPRMDel(alg.prm);
    ferRandMTDel(alg.rand);

    return 0;
}

static int terminate(void *data)
{
    alg_t *alg = (alg_t *)data;
    return ferPRMNodesLen(alg->prm) >= alg->max_nodes;
}

static void callback(void *data)
{
    alg_t *alg = (alg_t *)data;

    ferTimerStopAndPrintElapsed(&alg->timer, stderr, " n: %d / %d\r",
                                (int)ferPRMNodesLen(alg->prm),
                                (int)alg->max_nodes);
    fflush(stderr);
}

static const fer_vec_t *conf(void *data)
{
    alg_t *alg = (alg_t *)data;
    fer_real_t x, y;

    x = ferRandMT(alg->rand, -5, 5);
    y = ferRandMT(alg->rand, -5, 5);

    //fprintf(stderr, "conf: %g %g\n", x, y);

    ferVec2Set(&alg->conf, x, y);

    return (const fer_vec_t *)&alg->conf;
}

static int eval(const fer_vec_t *conf, void *data)
{
    //alg_t *alg = (alg_t *)data;
    fer_real_t x, y;

    x = ferVecGet(conf, 0);
    y = ferVecGet(conf, 1);

    //p->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -0.01 && x < 0.01)
            || (y > 4 && x > -2 && x < 2)){
        //fprintf(stderr, "eval: FREE\n");
        return FER_PRM_FREE;
    }
    //fprintf(stderr, "eval: OBST\n");
    return FER_PRM_OBST;
}

static int findPath(const fer_vec_t *c1, const fer_vec_t *c2, void *data)
{
    //alg_t *alg = (alg_t *)data;

    return 1;
}

static void printPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    fer_prm_node_t *n;
    size_t id;

    fprintf(out, "------\n");
    fprintf(out, "Name: path\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Edge color: 0 0 0\n");

    fprintf(out, "Points:\n");
    ferListForEach(path, item){
        n = ferListEntry(item, fer_prm_node_t, path);
        ferVec2Print((fer_vec2_t *)n->conf, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Edges:\n");
    id = 0;
    ferListForEach(path, item){
        if (ferListNext(item) == path)
            break;

        n = ferListEntry(item, fer_prm_node_t, path);
        fprintf(out, "%d %d\n", id, id + 1);
        id++;
    }

    fprintf(out, "------\n");
}
