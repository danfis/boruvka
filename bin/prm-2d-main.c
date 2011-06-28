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

struct _params_t {
    fer_prm_t *prm;
    fer_timer_t timer;
    fer_rand_mt_t *rand;
    fer_vec2_t conf;

    fer_vec2_t start, goal;

    size_t max_nodes;
    size_t find_path;
    fer_real_t step;

    unsigned long evals;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *conf(void *data);
static int eval(const fer_vec_t *conf, void *data);
static int findPath(const fer_vec_t *c1, const fer_vec_t *c2, void *data);
static void printPath(fer_list_t *path, FILE *out);

static void setUpScene(const char *scene, params_t *p, fer_prm_ops_t *ops,
                       fer_vec_t *start, fer_vec_t *goal);
static int eval1_1(const fer_vec_t *w, void *data);
static int eval1_3(const fer_vec_t *w, void *data);
static int eval1_5(const fer_vec_t *w, void *data);
static int eval2_1(const fer_vec_t *w, void *data);
static int eval2_3(const fer_vec_t *w, void *data);
static int eval2_5(const fer_vec_t *w, void *data);
static int eval3_1(const fer_vec_t *w, void *data);
static int eval3_3(const fer_vec_t *w, void *data);
static int eval3_5(const fer_vec_t *w, void *data);
static int eval4_1(const fer_vec_t *w, void *data);
static int eval4_3(const fer_vec_t *w, void *data);
static int eval4_5(const fer_vec_t *w, void *data);

int main(int argc, char *argv[])
{
    fer_prm_ops_t ops;
    fer_prm_params_t params;
    fer_real_t aabb[4] = { -5, 5, -5, 5 };
    params_t alg;

    if (argc != 3){
        fprintf(stderr, "Usage: %s max_nodes scene\n", argv[0]);
        return -1;
    }

    ferPRMOpsInit(&ops);
    ferPRMParamsInit(&params);

    params.d = 2;
    params.max_dist = 0.1;
    params.max_neighbors = 10;
    //params.cells.num_cells = 40000;
    params.cells.num_cells = 0;
    params.cells.max_dens  = 1;
    params.cells.expand_rate = 2.;
    params.cells.aabb = aabb;

    ops.data      = &alg;
    ops.conf      = conf;
    ops.terminate = terminate;
    ops.find_path = findPath;
    ops.callback  = callback;
    ops.callback_period = 500;

    alg.max_nodes = atoi(argv[1]);
    alg.find_path = 5000;
    ferVec2Set(&alg.start, FER_REAL(-4.), FER_REAL(-4.));
    ferVec2Set(&alg.goal, FER_REAL(1.5), FER_REAL(4.5));
    alg.evals = 0;
    alg.step = 0.01;

    alg.rand = ferRandMTNewAuto();

    setUpScene(argv[2], &alg, &ops, (fer_vec_t *)&alg.start, (fer_vec_t *)&alg.goal);

    alg.prm = ferPRMNew(&ops, &params);

    ferTimerStart(&alg.timer);
    ferPRMRun(alg.prm);
    callback(&alg);
    fprintf(stderr, "\n");


    ferPRMDumpSVT(alg.prm, stdout, "Result");

    ferPRMDel(alg.prm);
    ferRandMTDel(alg.rand);
    
    fprintf(stderr, "Evals: %ld\n", alg.evals);
    fprintf(stdout, "# Evals: %ld\n", alg.evals);

    return 0;
}

static int terminate(void *data)
{
    params_t *alg = (params_t *)data;
    size_t nodes;
    int res;
    fer_list_t path;

    nodes = ferPRMNodesLen(alg->prm);

    if (nodes > alg->find_path && nodes % alg->find_path == 0){
        ferListInit(&path);
        res = ferPRMFindPath(alg->prm, (fer_vec_t *)&alg->start,
                                       (fer_vec_t *)&alg->goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferPRMNodesLen(alg->prm));
            printPath(&path, stdout);

            return 1;
        }
    }

    return nodes >= alg->max_nodes;
}

static void callback(void *data)
{
    params_t *alg = (params_t *)data;

    ferTimerStopAndPrintElapsed(&alg->timer, stderr, " n: %d / %d\r",
                                (int)ferPRMNodesLen(alg->prm),
                                (int)alg->max_nodes);
    fflush(stderr);
}

static const fer_vec_t *conf(void *data)
{
    params_t *alg = (params_t *)data;
    fer_real_t x, y;

    x = ferRandMT(alg->rand, -5, 5);
    y = ferRandMT(alg->rand, -5, 5);

    //fprintf(stderr, "conf: %g %g\n", x, y);

    ferVec2Set(&alg->conf, x, y);

    return (const fer_vec_t *)&alg->conf;
}

static int eval(const fer_vec_t *conf, void *data)
{
    params_t *alg = (params_t *)data;
    fer_real_t x, y;

    x = ferVecGet(conf, 0);
    y = ferVecGet(conf, 1);

    alg->evals += 1L;

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
    params_t *alg = (params_t *)data;
    FER_VEC(move, 2);
    FER_VEC(c, 2);
    fer_real_t step;
    int ev;

    ferVecCopy(2, c, c1);
    ferVecSub2(2, move, c2, c1);
    ferVecScale(2, move, alg->step);

    step = alg->step;
    while (step < FER_ONE){
        ferVecAdd(2, c, move);

        ev = eval(c, data);
        if (ev != FER_PRM_FREE)
            return 0;

        step += alg->step;
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


static void setUpScene(const char *scene, params_t *p, fer_prm_ops_t *ops,
                       fer_vec_t *start, fer_vec_t *goal)
{

    if (strcmp(scene, "1_1") == 0){
        ops->eval = eval1_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(1.5), FER_REAL(4.5));
    }else if (strcmp(scene, "1_3") == 0){
        ops->eval = eval1_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(1.5), FER_REAL(4.5));
    }else if (strcmp(scene, "1_5") == 0){
        ops->eval = eval1_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(1.5), FER_REAL(4.5));
    }else if (strcmp(scene, "2_1") == 0){
        ops->eval = eval2_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "2_3") == 0){
        ops->eval = eval2_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "2_5") == 0){
        ops->eval = eval2_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "3_1") == 0){
        ops->eval = eval3_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "3_3") == 0){
        ops->eval = eval3_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "3_5") == 0){
        ops->eval = eval3_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "4_1") == 0){
        ops->eval = eval4_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(0.45), FER_REAL(0.));
    }else if (strcmp(scene, "4_3") == 0){
        ops->eval = eval4_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(0.45), FER_REAL(0.));
    }else if (strcmp(scene, "4_5") == 0){
        ops->eval = eval4_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(0.45), FER_REAL(0.));
    }
}

static int eval1(const fer_vec_t *w, void *data, fer_real_t r)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    p->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -r && x < r)
            || (y > 4 && x > -2 && x < 2)){
        return FER_PRM_FREE;
    }
    return FER_PRM_OBST;
}

static int eval1_1(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.01);
}

static int eval1_3(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.03);
}

static int eval1_5(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.05);
}

static int eval2(const fer_vec_t *w, void *data, fer_real_t r)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    p->evals += 1L;

    if (y < -2
            || (y > 4 && x > 4)
            || (y > -2 && y < 4 && x > -2 - r && x < 4 + r && x > y - r && x < y + r)){
        return FER_PRM_FREE;
    }
    return FER_PRM_OBST;
}

static int eval2_1(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.01);
}

static int eval2_3(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.03);
}

static int eval2_5(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.05);
}


static int eval3(const fer_vec_t *w, void *data, fer_real_t r)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    p->evals += 1L;

    if (y < -2
            || (y > 0 && x > 3.5)
            || (x < 3.6 && FER_CUBE(x - 1) + FER_CUBE(y + 1) > 9
                    && FER_CUBE(x - 1) + FER_CUBE(y + 1) < FER_CUBE(3 + r))){
        return FER_PRM_FREE;
    }
    return FER_PRM_OBST;
}

static int eval3_1(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.01);
}

static int eval3_3(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.03);
}

static int eval3_5(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.05);
}

static int eval4(const fer_vec_t *w, void *data, fer_real_t r)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    p->evals += 1L;

    if (y < -2
         || (x > 0.3 && x < 0.6 && y > -0.15 && y < 0.15)
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(2.)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(2. + r)
                && !(y < 0 && x > 0))
         || (FER_CUBE(x - 0.5) + FER_CUBE(y) > FER_CUBE(1.5)
                && FER_CUBE(x - 0.5) + FER_CUBE(y) < FER_CUBE(1.5 + r)
                && !(y > 0))
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(1.)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(1. + r)
                && !(y < 0))
         || (FER_CUBE(x - 0.25) + FER_CUBE(y) > FER_CUBE(.75)
                && FER_CUBE(x - 0.25) + FER_CUBE(y) < FER_CUBE(.75 + r)
                && !(y > 0))
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(.5)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(.5 + r)
                && !(y < 0))){
        return FER_PRM_FREE;
    }
    return FER_PRM_OBST;
}

static int eval4_1(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.01);
}

static int eval4_3(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.03);
}

static int eval4_5(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.05);
}
