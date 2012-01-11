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
#include <fermat/dbg.h>

#define FREE FER_PRM_FREE
#define OBST FER_PRM_OBST

fer_real_t aabb[12] = { -54, 85, -45, 93, -48, 90,
                        -M_PI, M_PI, -M_PI, M_PI, -M_PI, M_PI };
fer_prm_ops_t ops;
fer_prm_params_t params;
fer_prm_t *prm;

fer_rand_mt_t *rand_mt;

fer_timer_t timer;

fer_vec_t *conf, *start, *goal;
fer_real_t step = 0.12;
size_t max_nodes;
size_t find_path = 5000;
unsigned long evals = 0;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *confcb(void *data);
static int eval(const fer_vec_t *conf, void *data);
static int findPath(const fer_vec_t *c1, const fer_vec_t *c2, void *data);
static void printPath(fer_list_t *path, FILE *out);


int main(int argc, char *argv[])
{
    if (argc != 3){
        fprintf(stderr, "Usage: %s max_nodes scene\n", argv[0]);
        return -1;
    }

    ferPRMOpsInit(&ops);
    ferPRMParamsInit(&params);

    params.d = 6;
    params.max_neighbors = 10;
    params.max_dist      = 5.;
    params.gug.num_cells = 0;
    params.gug.max_dens  = 1;
    params.gug.expand_rate = 1.2;
    params.gug.aabb = aabb;

    ops.data      = NULL;
    ops.conf      = confcb;
    ops.eval      = eval;
    ops.terminate = terminate;
    ops.find_path = findPath;
    ops.callback  = callback;
    ops.callback_period = 500;


    rand_mt = ferRandMTNewAuto();

    prm = ferPRMNew(&ops, &params);

    max_nodes = atoi(argv[1]);

    conf  = ferVecNew(6);
    start = ferVecNew(6);
    goal  = ferVecNew(6);

    ferVecSetZero(6, start);
    ferVecSet(start, 0, -2.6);
    ferVecSet(start, 1, 2.34);
    ferVecSet(start, 2, 0.71);
    ferVecSetZero(6, goal);
    ferVecSet(goal, 0, -8);
    ferVecSet(goal, 1, 57);
    ferVecSet(goal, 2, -1.4);

    ferTimerStart(&timer);
    ferPRMRun(prm);
    callback(NULL);
    fprintf(stderr, "\n");


    //ferPRMDumpSVT(prm, stdout, "Result");

    ferPRMDel(prm);
    ferRandMTDel(rand_mt);
    
    fprintf(stderr, "Evals: %ld\n", evals);
    fprintf(stdout, "# Evals: %ld\n", evals);

    return 0;
}

static int terminate(void *data)
{
    size_t nodes;
    int res;
    fer_list_t path;

    nodes = ferPRMNodesLen(prm);

    if (nodes > find_path && nodes % find_path == 0){
        ferListInit(&path);
        res = ferPRMFindPath(prm, start, goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferPRMNodesLen(prm));
            printPath(&path, stdout);

            return 1;
        }
    }

    return nodes >= max_nodes;
}

static void callback(void *data)
{
    ferTimerStopAndPrintElapsed(&timer, stderr, " n: %d / %d, evals: %lu\r",
                                (int)ferPRMNodesLen(prm),
                                (int)max_nodes,
                                evals);
    fflush(stderr);
}

static const fer_vec_t *confcb(void *data)
{
    int i;

    for (i = 0; i < 6; i++){
        ferVecSet(conf, i, ferRandMT(rand_mt, aabb[2 * i], aabb[2 * i + 1]));
    }

    return conf;
}

static int eval(const fer_vec_t *conf, void *data)
{
    evals += 1L;
    // TODO
    //return __eval(conf, NULL);
    return FREE;
}

static int findPath(const fer_vec_t *c1, const fer_vec_t *c2, void *data)
{
    FER_VEC(move, 6);
    FER_VEC(c, 6);
    fer_real_t dist;
    int ev;

    ferVecCopy(6, c, c1);
    ferVecSub2(6, move, c2, c1);
    dist = ferVecLen(6, move);
    ferVecScale(6, move, step * ferRecp(dist));

    while (dist > step){
        ferVecAdd(6, c, move);

        ev = eval(c, data);
        if (ev != FER_PRM_FREE)
            return 0;

        dist -= step;
    }

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
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_prm_node_t, path);
        ferVec2Print((fer_vec2_t *)n->conf, out);
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



