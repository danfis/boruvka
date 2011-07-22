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
#include <fermat/cd.h>
#include <fermat/dbg.h>

#define FREE FER_RRT_FREE
#define OBST FER_RRT_OBST

fer_cd_t *cd;
fer_cd_geom_t *map, *robot;
fer_vec3_t cdtr;
fer_mat3_t cdrot;

fer_real_t aabb[12] = { -54, 85, -45, 93, -48, 90,
                        -M_PI, M_PI, -M_PI, M_PI, -M_PI, M_PI };
fer_rrt_ops_t ops;
fer_rrt_params_t params;
fer_rrt_t *rrt;

fer_rand_mt_t *rand_mt;

fer_timer_t timer;

fer_vec_t *conf, *new_conf, *start, *goal;
fer_real_t step = 0.12;
size_t max_nodes;
size_t find_path = 5000;
unsigned long evals = 0;


static int terminate(const fer_rrt_t *rrt, void *data);
static int terminate_expand(const fer_rrt_t *rrt,
                            const fer_rrt_node_t *start,
                            const fer_rrt_node_t *last,
                            const fer_vec_t *rand, void *data);
static void callback(const fer_rrt_t *rrt, void *data);
static const fer_vec_t *rand_conf(const fer_rrt_t *rrt, void *data);
static const fer_vec_t *expand(const fer_rrt_t *rrt,
                               const fer_rrt_node_t *n,
                               const fer_vec_t *rand, void *data);
static void expand_all(const fer_rrt_t *rrt,
                       const fer_rrt_node_t *n,
                       const fer_vec_t *rand, void *data,
                       fer_list_t *list);
static int filter_blossom(const fer_rrt_t *rrt,
                          const fer_vec_t *c,
                          const fer_rrt_node_t *src,
                          const fer_rrt_node_t *near,
                          void *data);
static int eval(const fer_vec_t *conf);
static void printPath(FILE *out);


int main(int argc, char *argv[])
{
    if (argc != 3){
        fprintf(stderr, "Usage: %s max_nodes scene\n", argv[0]);
        return -1;
    }

    ferRRTOpsInit(&ops);
    ferRRTParamsInit(&params);

    params.d = 6;
    //params.cells.num_cells = 40000;
    params.cells.num_cells = 0;
    params.cells.max_dens  = 1;
    params.cells.expand_rate = 1.1;
    params.cells.aabb = aabb;

    ops.data      = NULL;
    ops.random    = rand_conf;
    ops.expand    = expand;
    ops.expand_all = expand_all;
    ops.filter_blossom = filter_blossom;
    ops.terminate = terminate;
    ops.terminate_expand = terminate_expand;
    ops.callback  = callback;
    ops.callback_period = 500;


    max_nodes = atoi(argv[1]);

    conf  = ferVecNew(6);
    new_conf = ferVecNew(6);
    start = ferVecNew(6);
    goal  = ferVecNew(6);


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

        step = 0.12;

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

        step = 0.15;

        ferCDGeomAddTrisFromRaw(cd, map, "/home/danfis/dev/imr-data/trimesh/room/map.raw");
        ferCDGeomAddTrisFromRaw(cd, robot, "/home/danfis/dev/imr-data/trimesh/room/robot.raw");
    }



    ferCDGeomBuild(cd, map);
    ferCDGeomBuild(cd, robot);


    rand_mt = ferRandMTNewAuto();

    rrt = ferRRTNew(&ops, &params);

    ferTimerStart(&timer);
    ferRRTRunBasic(rrt, start);
    //ferRRTRunConnect(rrt, start);
    //ferRRTRunBlossom(rrt, start);
    callback(rrt, NULL);
    fprintf(stderr, "\n");


    //ferPRMDumpSVT(rrt, stdout, "Result");

    ferRRTDel(rrt);
    ferRandMTDel(rand_mt);
    
    fprintf(stderr, "Evals: %ld\n", evals);
    fprintf(stdout, "# Evals: %ld\n", evals);

    ferCDDel(cd);

    return 0;
}

static int terminate(const fer_rrt_t *rrt, void *data)
{
    size_t nodes;
    const fer_rrt_node_t *last;
    const fer_vec_t *lastv;
    fer_real_t dist;

    nodes = ferRRTNodesLen(rrt);
    if (nodes >= max_nodes){
        fprintf(stderr, "\nPath not found\n");
        printPath(stdout);
        return 1;
    }

    last = ferRRTNodeLast(rrt);
    lastv = ferRRTNodeConf(last);
    dist = ferVecDist(6, lastv, goal);
    //DBG("%f %lx", dist, (long)last);
    if (dist < step){
        printPath(stdout);
        return 1;
    }

    return 0;
}

static int terminate_expand(const fer_rrt_t *rrt,
                            const fer_rrt_node_t *start,
                            const fer_rrt_node_t *last,
                            const fer_vec_t *rand, void *data)
{
    const fer_vec_t *n;
    fer_real_t dist;

    n = ferRRTNodeConf(last);
    dist = ferVecDist(6, n, rand);

    return dist <= step;
}

static void callback(const fer_rrt_t *rrt, void *data)
{
    ferTimerStopAndPrintElapsed(&timer, stderr, " n: %d / %d, evals: %lu\r",
                                (int)ferRRTNodesLen(rrt),
                                (int)max_nodes, evals);
    fflush(stderr);
}

static const fer_vec_t *rand_conf(const fer_rrt_t *rrt, void *data)
{
    int i;
    static int c = 0;

    if (c++ == 5000){
        c = 0;
        return goal;
    }

    for (i = 0; i < 6; i++){
        ferVecSet(conf, i, ferRandMT(rand_mt, aabb[2 * i], aabb[2 * i + 1]));
    }

    return conf;
}

static const fer_vec_t *expand(const fer_rrt_t *rrt,
                               const fer_rrt_node_t *node_near,
                               const fer_vec_t *rand, void *data)
{
    FER_VEC(vec, 6);
    fer_real_t dist, best_dist, move;
    const fer_vec_t *near;
    int i;

    near = ferRRTNodeConf(node_near);

    best_dist = FER_REAL_MAX;
    for (i = 0; i < 3; i++){
        move = ferRandMT(rand_mt, -step, step);

        ferVecCopy(6, vec, near);
        ferVecSet(vec, i, ferVecGet(vec, i) + move);
        if (eval(vec) == FER_RRT_FREE){
            dist = ferVecDist(6, rand, vec);
            if (dist < best_dist){
                ferVecCopy(6, new_conf, vec);
                best_dist = dist;
            }
        }
    }

    for (i = 3; i < 6; i++){
        move = ferRandMT(rand_mt, -0.08, 0.08);

        ferVecCopy(6, vec, near);
        ferVecSet(vec, i, ferVecGet(vec, i) + move);
        if (eval(vec) == FER_RRT_FREE){
            dist = ferVecDist(6, rand, vec);
            if (dist < best_dist){
                ferVecCopy(6, new_conf, vec);
                best_dist = dist;
            }
        }
    }

    if (best_dist < FER_REAL_MAX)
        return new_conf;

    return NULL;
    /*
    ferVecSub2(6, move, rand, near);
    ferVecScale(6, move, step * ferRecp(ferVecLen(6, move)));

    ferVecAdd2(6, new_conf, near, move);

    if (eval(new_conf) == FER_RRT_OBST)
        return NULL;
    //DBG("expand: %f %f", ferVec2X(&alg->new_conf), ferVec2Y(&alg->new_conf));
    return new_conf;
    */
}

static void expand_all(const fer_rrt_t *rrt,
                       const fer_rrt_node_t *node_near,
                       const fer_vec_t *rand, void *data,
                       fer_list_t *list)
{
    const fer_vec_t *conf;
    FER_VEC(c, 6);
    FER_VEC(add, 6);

    ferVecCopy(6, c, rand);

    conf = expand(rrt, node_near, c, data);
    if (conf)
        ferRRTExpandAdd(6, conf, list);

    ferVecSetAll(6, add, step);
    ferVecAdd(6, c, add);
    conf = expand(rrt, node_near, c, data);
    if (conf)
        ferRRTExpandAdd(6, conf, list);

    ferVecSub(6, c, add);
    ferVecSub(6, c, add);
    conf = expand(rrt, node_near, c, data);
    if (conf)
        ferRRTExpandAdd(6, conf, list);
}

static int filter_blossom(const fer_rrt_t *rrt,
                          const fer_vec_t *c,
                          const fer_rrt_node_t *src,
                          const fer_rrt_node_t *near,
                          void *data)
{
    const fer_vec_t *s, *n;
    s = ferRRTNodeConf(src);
    n = ferRRTNodeConf(near);

    return s == n || ferVecDist(6, c, n) > ferVecDist(6, c, s);
}

static int eval(const fer_vec_t *conf)
{
    evals += 1L;

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

static void printPath(FILE *out)
{
    fer_list_t *item;
    const fer_rrt_node_t *last_node, *init_node, *goal_node;
    const fer_vec_t *last;
    fer_rrt_node_t *n;
    size_t i;
    fer_list_t path;

    last_node = ferRRTNodeLast(rrt);
    //last = ferRRTNodeConf(last_node);
    //if (ferVecDist(6, last, goal) > step)
    //    return;

    init_node = ferRRTNodeInitial(rrt);
    goal_node = ferRRTNodeNew(rrt, goal, last_node);

    ferListInit(&path);
    ferRRTFindPath(rrt, init_node, goal_node, &path);

    FER_LIST_FOR_EACH(&path, item){
        n = FER_LIST_ENTRY(item, fer_rrt_node_t, path);
        for (i = 0; i < 6; i++){
            fprintf(out, "%f ", (float)ferVecGet(n->conf, i));
        }
        fprintf(out, "\n");
    }
}
