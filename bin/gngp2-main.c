#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan.h>

struct _params_t {
    size_t max_nodes;
    size_t find_path;
    fer_gngp_t *gng;
    fer_timer_t timer;

    fer_vec_t *start, *goal;

    fer_rand_mt_t *rand;
    fer_vec_t *is;

    unsigned long evals;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *inputSignal(void *data);
static void setUpScene(const char *scene, params_t *p, fer_gngp_ops_t *ops,
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
static void printPath(fer_list_t *path, FILE *out);

int main(int argc, char *argv[])
{
    fer_gngp_params_t params;
    fer_gngp_ops_t ops;
    fer_gngp_t *gng;
    params_t p;
    fer_real_t aabb[4] = { -5, 5, -5, 5 };

    if (argc < 4){
        fprintf(stderr, "Usage: %s max_nodes find_path_period scene\n", argv[0]);
        return -1;
    }

    ferGNGPParamsInit(&params);

    ferGNGPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.callback  = callback;
    ops.callback_period = 500;
    ops.data = &p;

    p.start = ferVecNew(2);
    p.goal  = ferVecNew(2);
    p.is    = ferVecNew(2);
    p.rand = ferRandMTNewAuto();

    p.evals = 0L;
    p.max_nodes = atoi(argv[1]);
    p.find_path = atoi(argv[2]);
    params.d = 2;
    //params.num_cells = p.max_nodes;
    //params.cells.num_cells = 10000;
    params.cells.num_cells = 0;
    params.cells.max_dens  = 1;
    params.cells.expand_rate = 1.5;
    params.cells.aabb = aabb;
    params.warm_start = p.find_path;
    params.alpha = 0.;
    params.beta  = 1.;
    params.lambda = 200;

    setUpScene(argv[3], &p, &ops, p.start, p.goal);

    gng = ferGNGPNew(&ops, &params);
    p.gng = gng;

    ferTimerStart(&p.timer);
    callback(&p);
    ferGNGPRun(gng);
    callback(&p);
    fprintf(stderr, "\n");
    fprintf(stderr, "Evals: %lu\n", p.evals);

    ferGNGPDumpSVT(gng, stdout, NULL);
    fprintf(stdout, "# ");
    ferTimerStopAndPrintElapsed(&p.timer, stdout, " n: %d / %d\n",
                                ferGNGPNodesLen(p.gng), p.max_nodes);
    fprintf(stdout, "# Evals: %ld\n", p.evals);

    ferGNGPDel(gng);

    ferRandMTDel(p.rand);

    ferVecDel(p.start);
    ferVecDel(p.goal);
    ferVecDel(p.is);
    

    return 0;
}


static int terminate(void *data)
{
    params_t *p = (params_t *)data;
    int res;
    fer_list_t path;

    if (ferGNGPNodesLen(p->gng) > p->find_path
            && ferGNGPNodesLen(p->gng) % p->find_path == 0){
        ferListInit(&path);
        res = ferGNGPFindPath(p->gng, p->start, p->goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)ferGNGPNodesLen(p->gng));
            printPath(&path, stdout);
            return 1;
        }
    }

    return ferGNGPNodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *data)
{
    params_t *p = data;
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

    nodes_len = ferGNGPNodesLen(p->gng);
    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d\r", nodes_len, p->max_nodes);
}

static const fer_vec_t *inputSignal(void *data)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;

    x = ferRandMT(p->rand, -5, 5);
    y = ferRandMT(p->rand, -5, 5);
    ferVec2Set((fer_vec2_t *)p->is, x, y);
    return p->is;
}

static void setUpScene(const char *scene, params_t *p, fer_gngp_ops_t *ops,
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
        return FER_GNGP_FREE;
    }
    return FER_GNGP_OBST;
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
        return FER_GNGP_FREE;
    }
    return FER_GNGP_OBST;
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
        return FER_GNGP_FREE;
    }
    return FER_GNGP_OBST;
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
        return FER_GNGP_FREE;
    }
    return FER_GNGP_OBST;
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



static void printPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    fer_gngp_node_t *n;
    size_t id;

    fprintf(out, "------\n");
    fprintf(out, "Name: path\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Edge color: 0 0 0\n");

    fprintf(out, "Points:\n");
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gngp_node_t, path);
        ferVecPrint(2, n->w, out);
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
