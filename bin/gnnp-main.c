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
#include "cfg-map.h"


fer_vec_t *is;
fer_vec_t *start, *goal;
unsigned long evals = 0UL;
static const fer_real_t *aabb;


int list_robots;
const char *robot_name = NULL;
long callback_period = 10000;
long max_steps = 1000000;
int rmax = 0;
int use_rot = 0;
int dbg_dump = 0;


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
    return ferCfgConf();
}

static int eval(fer_gnnp_t *nn, const fer_vec_t *conf, void *data)
{
    evals += 1UL;
    return !ferCfgMapCollide(conf);
}

static void dumpDBG(fer_gnnp_t *nn, int c)
{
    char fn[100];
    FILE *fout;

    snprintf(fn, 100, "dbg/map-%06d", c);
    fout = fopen(fn, "w");
    if (fout){
        ferGNNPDumpSVT(nn, fout, NULL);
        ferCfgMapDumpSVT(fout, NULL);
        ferCfgMapRobotDumpSVT(start, fout, "Init");
        ferCfgMapRobotDumpSVT(goal, fout, "Goal");
        fclose(fout);
    }
}

static void callback(fer_gnnp_t *nn, void *data)
{
    static int c = 0;

    if (dbg_dump)
        dumpDBG(nn, c);

    fprintf(stderr, "step %d, nodes: %d, evals: %ld\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals);

    c++;
}

static void printPath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    if (ferCfgConfDim() > 3)
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
        ferVecPrint(ferCfgConfDim(), n->w, stdout);
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

static void printSolutionVideoPath(fer_list_t *path, fer_list_t *end, FILE *out)
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

static void printSolutionVideo(fer_gnnp_t *nn, fer_list_t *path)
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
            printSolutionVideoPath(path, item, out);
            fclose(out);
        }
        i++;
    }
}

int opts(int *argc, char *argv[])
{
    int ok = 1;

    ferOptsAdd("robots", 0x0, FER_OPTS_NONE, (void *)&list_robots, NULL);
    ferOptsAdd("robot", 'r', FER_OPTS_STR, (void *)&robot_name, NULL);
    ferOptsAdd("cb-period", 0x0, FER_OPTS_LONG, (void *)&callback_period, NULL);
    ferOptsAdd("dbg-dump", 0x0, FER_OPTS_NONE, (void *)&dbg_dump, NULL);
    ferOptsAdd("max-steps", 0x0, FER_OPTS_LONG, (void *)&max_steps, NULL);
    ferOptsAdd("rmax", 0x0, FER_OPTS_INT, (void *)&rmax, NULL);
    ferOptsAdd("rot", 0x0, FER_OPTS_NONE, (void *)&use_rot, NULL);


    if (ferOpts(argc, argv) != 0)
        ok = 0;

    if (!ok || *argc != 2 || (!list_robots && robot_name == NULL)){
        fprintf(stderr, "Usage: %s [ OPTIONS ] [-r robot | --robots] cfg_file\n", argv[0]);
        fprintf(stderr, "  OPTIONS:\n");
        fprintf(stderr, "    -r           str  The robot with specified name will be used\n");
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
    int ret;
    fer_gnnp_params_t params;
    fer_gnnp_ops_t ops;
    fer_gnnp_t *nn;
    fer_timer_t timer;

    if (opts(&argc, argv) != 0)
        return -1;

    if (ferCfgMapInit(argv[1]) != 0)
        return -1;

    if (use_rot){
        ferCfgUseRot();
    }

    if (rmax == 0){
        rmax = powl(2, ferCfgConfDim() + 1);
        fprintf(stderr, "rmax set to %d\n", rmax);
    }

    if (list_robots){
        ferCfgMapListRobots(stderr);
        ferCfgMapDestroy();
        return 0;
    }

    aabb = ferCfgMapAABB();

    ferGNNPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback        = callback;
    ops.callback_period = callback_period;

    ferGNNPParamsInit(&params);
    params.dim  = ferCfgConfDim();
    params.rmax = rmax;
    params.h    = 0.25;
    params.nn.type = FER_NN_GUG;
    params.nn.gug.dim = ferCfgConfDim();
    params.nn.gug.max_dens = 1.;
    params.nn.gug.expand_rate = 1.3;
    params.nn.gug.aabb = (fer_real_t *)aabb;

    is    = ferVecNew(6);
    start = ferVecNew(6);
    goal  = ferVecNew(6);
    ferVecSetZero(6, is);
    ferVecSetZero(6, start);
    ferVecSetZero(6, goal);

    // load robot with parameters
    if (ferCfgMapRobot(robot_name, &params.h, start, goal) != 0){
        ferVecDel(is);
        ferVecDel(start);
        ferVecDel(goal);
        ferCfgMapDestroy();
        return -1;
    }


    nn = ferGNNPNew(&ops, &params);

    ferTimerStart(&timer);
    ret = ferGNNPFindPath(nn, start, goal, &path);
    ferTimerStop(&timer);

    callback(nn, NULL);
    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "evals: %ld\n", (long)evals);
    fprintf(stderr, "Time: %lu us\n", ferTimerElapsedInUs(&timer));
    fflush(stderr);

    if (ret == 0){
        printPath(nn, &path);
        printSolutionVideo(nn, &path);
    }
    ferGNNPDumpSVT(nn, stdout, NULL);
    ferCfgMapDumpSVT(stdout, NULL);
    ferCfgMapRobotDumpSVT(start, stdout, "Init");
    ferCfgMapRobotDumpSVT(goal, stdout, "Goal");


    ferGNNPDel(nn);


    ferVecDel(is);
    ferVecDel(start);
    ferVecDel(goal);

    ferCfgMapDestroy();

    return 0;
}
