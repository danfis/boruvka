#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>

#define FREE 0
#define OBST 1

fer_gng_plan_t *gng;
fer_gng_plan_ops_t ops;
fer_gng_plan_params_t params;
size_t max_nodes;
fer_rand_mt_t *rand_mt;
fer_timer_t timer;
fer_vec_t *is;


static void dumpSVT(fer_gng_plan_t *gng, FILE *out, const char *name);
static int (*eval)(const fer_vec_t *w, void *data);

static int terminate(void *data);
static void callback(void *data);
static const void *inputSignal(void *data);
static void setUpScene(const char *scene,
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
    fer_real_t aabb[4] = { -5, 5, -5, 5 };

    if (argc < 2){
        fprintf(stderr, "Usage: %s max_nodes find_path_period scene\n", argv[0]);
        return -1;
    }

    is = ferVecNew(2);

    max_nodes = atoi(argv[1]);

    ferGNGPlanOpsInit(&ops);
    ferGNGPlanParamsInit(&params);

    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.callback     = callback;
    ops.callback_period = 500;

    params.dim = 2;
    params.cells.d = 2;
    params.cells.aabb = aabb;
    params.cells.max_dens = 2;
    params.cells.expand_rate = 2;

    rand_mt = ferRandMTNewAuto();
    gng = ferGNGPlanNew(&ops, &params);

    ferTimerStart(&timer);
    ferGNGPlanRun(gng);
    ferTimerStopAndPrintElapsed(&timer, stderr, "\r\n");

    dumpSVT(gng, stdout, "Result");
    ferGNGPlanDel(gng);

    ferRandMTDel(rand_mt);
    
    ferVecDel(is);

    return 0;
}


static int terminate(void *data)
{
    size_t nodes_len;

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));

#if 0
    int res;
    fer_list_t path;

    if (ferGNGEuNodesLen(p->gng) > p->find_path
            && ferGNGEuNodesLen(p->gng) % p->find_path == 0){
        ferListInit(&path);
        res = ferGNGEuFindPath(p->gng, p->start, p->goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferGNGPNodesLen(p->gng));
            printPath(&path, stdout);
            return 1;
        }
    }
#endif

    return nodes_len >= max_nodes;
}

static void callback(void *data)
{
    size_t nodes_len;
    /*
    FILE *fout;
    char fn[1000];

    nodes_len = ferGNGPNodesLen(p->gng);

    sprintf(fn, "out/%010d.svt", ferGNGPNodesLen(p->gng));
    fout = fopen(fn, "w");
    ferGNGPDumpSVT(p->gng, fout, NULL);
    fclose(fout);
    */

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    ferTimerStopAndPrintElapsed(&timer, stderr, " %d\r", (int)nodes_len);
}

static const void *inputSignal(void *data)
{
    fer_real_t x, y;

    x = ferRandMT(rand_mt, -5, 5);
    y = ferRandMT(rand_mt, -5, 5);
    ferVec2Set((fer_vec2_t *)is, x, y);
    return is;
}

static void setUpScene(const char *scene,
                       fer_vec_t *start, fer_vec_t *goal)
{

    if (strcmp(scene, "1_1") == 0){
        eval = eval1_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(1.5), FER_REAL(4.5));
    }else if (strcmp(scene, "1_3") == 0){
        eval = eval1_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(1.5), FER_REAL(4.5));
    }else if (strcmp(scene, "1_5") == 0){
        eval = eval1_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(1.5), FER_REAL(4.5));
    }else if (strcmp(scene, "2_1") == 0){
        eval = eval2_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "2_3") == 0){
        eval = eval2_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "2_5") == 0){
        eval = eval2_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "3_1") == 0){
        eval = eval3_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "3_3") == 0){
        eval = eval3_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "3_5") == 0){
        eval = eval3_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(4.5), FER_REAL(4.5));
    }else if (strcmp(scene, "4_1") == 0){
        eval = eval4_1;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(0.45), FER_REAL(0.));
    }else if (strcmp(scene, "4_3") == 0){
        eval = eval4_3;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(0.45), FER_REAL(0.));
    }else if (strcmp(scene, "4_5") == 0){
        eval = eval4_5;
        ferVec2Set((fer_vec2_t *)start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set((fer_vec2_t *)goal, FER_REAL(0.45), FER_REAL(0.));
    }
}

static int eval1(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -r && x < r)
            || (y > 4 && x > -2 && x < 2)){
        return FREE;
    }
    return OBST;
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
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y > 4 && x > 4)
            || (y > -2 && y < 4 && x > -2 - r && x < 4 + r && x > y - r && x < y + r)){
        return FREE;
    }
    return OBST;
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
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y > 0 && x > 3.5)
            || (x < 3.6 && FER_CUBE(x - 1) + FER_CUBE(y + 1) > 9
                    && FER_CUBE(x - 1) + FER_CUBE(y + 1) < FER_CUBE(3 + r))){
        return FREE;
    }
    return OBST;
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
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

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
        return FREE;
    }
    return OBST;
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


static void dumpSVT(fer_gng_plan_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_t *net;
    fer_net_node_t *nn;
    fer_gng_node_t *gn;
    fer_net_edge_t *e;
    fer_gng_plan_node_t *n;
    size_t i, id1, id2;

    if (gng->dim != 2 && gng->dim != 3)
        return;

    net = ferGNGPlanNet(gng);

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferNetNodes(net);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        nn = FER_LIST_ENTRY(item, fer_net_node_t, list);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);

        n->_id = i++;
        if (gng->dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEdges(gng->gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, fer_gng_plan_node_t, node);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}
