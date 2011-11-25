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
#include <fermat/rand-mt.h>
#include <fermat/timer.h>
#include <fermat/cd.h>
#include <fermat/cfg.h>
#include <fermat/opts.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>
#include "cfg-2d-map.h"


fer_rand_mt_t *rnd;
fer_vec_t *is;
fer_vec_t *start, *goal;
unsigned long evals = 0UL;
static const fer_real_t *aabb;


int list_robots;
const char *robot_name = NULL;
long callback_period = 10000;
long max_steps = 1000000;


static int terminate(fer_gnnp_t *nn, void *data)
{
    static long count = 0;

    ++count;
    if (count == max_steps)
        return 1;
    return 0;
}

static const fer_vec_t *inputSignal(fer_gnnp_t *nn, void *data)
{
    ferVecSet(is, 0, ferRandMT(rnd, aabb[0], aabb[1]));
    ferVecSet(is, 1, ferRandMT(rnd, aabb[2], aabb[3]));
    return is;
}

static int eval(fer_gnnp_t *nn, const fer_vec_t *conf, void *data)
{
    evals += 1UL;
    return !ferCfg2DMapCollide(conf);
}

static void callback(fer_gnnp_t *nn, void *data)
{
    static int c = 0;
    char fn[100];
    FILE *fout;
    size_t i;

    snprintf(fn, 100, "g-%06d", c);
    fout = fopen(fn, "w");
    if (fout){
        ferGNNPDumpSVT(nn, fout, NULL);
        ferCfg2DMapDumpSVT(fout, NULL);
        fclose(fout);
    }


    fprintf(stderr, "step %d, nodes: %d, evals: %ld, max_depth: %d\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals, (int)nn->max_depth);

    fprintf(stderr, "    depth:");
    for (i = 0; i < nn->depths_alloc; i++){
        if (nn->depths[i] != 0)
            fprintf(stderr, " %02d:%04d", (int)i, (int)nn->depths[i]);
    }
    fprintf(stderr, "\n");

    
    fprintf(stderr, "    free: %d\n", (int)nn->nodes_set[0].len);
    fprintf(stderr, "    obst: %d\n", (int)nn->nodes_set[1].len);

    c++;
}

static void printPath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    printf("----\n");
    printf("Name: PATH\n");

    printf("Points off: 1\n");
    printf("Edge color: 1 0 0\n");
    printf("Edge width: 2\n");
    printf("Points:\n");
    len = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
        ferVecPrint(2, n->w, stdout);
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

int opts(int *argc, char *argv[])
{
    ferOptsAdd("robots", 0x0, FER_OPTS_NONE, (void *)&list_robots, NULL);
    ferOptsAdd("robot", 'r', FER_OPTS_STR, (void *)&robot_name, NULL);
    ferOptsAdd("cb-period", 0x0, FER_OPTS_LONG, (void *)&callback_period, NULL);
    ferOptsAdd("max-steps", 0x0, FER_OPTS_LONG, (void *)&max_steps, NULL);

    if (ferOpts(argc, argv) != 0)
        return -1;

    if (*argc != 2 || robot_name == NULL){
        fprintf(stderr, "Usage: %s [ OPTIONTS ] cfg_file\n", argv[0]);
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    fer_list_t path;
    int ret;
    fer_gnnp_params_t params;
    fer_gnnp_ops_t ops;
    fer_gnnp_t *nn;
    fer_timer_t timer;

    if (opts(&argc, argv) != 0)
        return -1;

    if (ferCfg2DMapInit(argv[1]) != 0)
        return -1;

    if (list_robots){
        ferCfg2DMapListRobots(stderr);
        ferCfg2DMapDestroy();
        return 0;
    }

    aabb = ferCfg2DMapAABB();

    ferGNNPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback        = callback;
    ops.callback_period = callback_period;

    ferGNNPParamsInit(&params);
    params.dim  = 2;
    params.rmax = 6;
    params.h    = 0.25;
    params.prune_delay = 500;
    params.tournament = 3;
    params.nn.type = FER_NN_GUG;
    params.nn.gug.max_dens = 1.;
    params.nn.gug.expand_rate = 1.3;
    params.nn.gug.aabb = (fer_real_t *)aabb;

    is    = ferVecNew(2);
    start = ferVecNew(2);
    goal  = ferVecNew(2);

    // load robot with parameters
    if (ferCfg2DMapRobot(robot_name, &params.h, start, goal) != 0){
        ferVecDel(is);
        ferVecDel(start);
        ferVecDel(goal);
        ferCfg2DMapDestroy();
        return -1;
    }


    //rnd = ferRandMTNewAuto();
    rnd = ferRandMTNew(1111);

    nn = ferGNNPNew(&ops, &params);

    ferTimerStart(&timer);
    ret = ferGNNPFindPath(nn, start, goal, &path);
    ferTimerStop(&timer);
    callback(nn, NULL);
    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "evals: %ld\n", (long)evals);
    ferGNNPDumpSVT(nn, stdout, NULL);
    if (ret == 0){
        printPath(nn, &path);
    }
    ferCfg2DMapDumpSVT(stdout, NULL);
    ferCfg2DMapRobotDumpSVT(start, stdout, "Init");
    ferCfg2DMapRobotDumpSVT(goal, stdout, "Goal");

    fprintf(stderr, "Time: %lu us\n", ferTimerElapsedInUs(&timer));


    ferGNNPDel(nn);


    ferVecDel(is);
    ferVecDel(start);
    ferVecDel(goal);

    ferRandMTDel(rnd);

    ferCfg2DMapDestroy();

    return 0;
}
