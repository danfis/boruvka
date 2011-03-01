#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <gann/gng-plan.h>

struct _params_t {
    size_t max_nodes;
    size_t find_path;
    gann_gngp_t *gng;
    fer_timer_t timer;

    fer_vec2_t start, goal;

    fer_rand_mt_t *rand;
    fer_vec2_t is;

    unsigned long evals;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec2_t *inputSignal(void *data);
static int eval(const fer_vec2_t *w, void *data);
static void printPath(fer_list_t *path, FILE *out);

int main(int argc, char *argv[])
{
    gann_gngp_params_t params;
    gann_gngp_ops_t ops;
    gann_gngp_t *gng;
    params_t p;

    if (argc < 3){
        fprintf(stderr, "Usage: %s max_nodes find_path_period\n", argv[0]);
        return -1;
    }

    gannGNGPParamsInit(&params);

    gannGNGPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback  = callback;
    ops.callback_period = 500;
    ops.data = &p;

    p.rand = ferRandMTNewAuto();
    params.aabb[0] = -5;
    params.aabb[1] = 5;
    params.aabb[2] = -5;
    params.aabb[3] = 5;

    p.evals = 0L;
    p.max_nodes = atoi(argv[1]);
    p.find_path = atoi(argv[2]);
    params.num_cubes = p.max_nodes;
    //params.num_cubes = 30000;
    params.warm_start = p.find_path;
    ferVec2Set(&p.start, FER_REAL(-4.), FER_REAL(-4.));
    ferVec2Set(&p.goal, FER_REAL(1.5), FER_REAL(4.5));

    gng = gannGNGPNew(&ops, &params);
    p.gng = gng;

    ferTimerStart(&p.timer);
    gannGNGPRun(gng);
    callback(&p);
    fprintf(stderr, "\n");
    fprintf(stderr, "Evals: %ld\n", p.evals);

    gannGNGPDumpSVT(gng, stdout, NULL);
    fprintf(stdout, "# ");
    ferTimerStopAndPrintElapsed(&p.timer, stdout, " n: %d / %d\n",
                                gannGNGPNodesLen(p.gng), p.max_nodes);
    fprintf(stdout, "# Evals: %ld\n", p.evals);

    gannGNGPDel(gng);

    ferRandMTDel(p.rand);

    

    return 0;
}


static int terminate(void *data)
{
    params_t *p = (params_t *)data;
    int res;
    fer_list_t path;

    if (gannGNGPNodesLen(p->gng) > p->find_path
            && gannGNGPNodesLen(p->gng) % p->find_path == 0){
        ferListInit(&path);
        res = gannGNGPFindPath(p->gng, &p->start, &p->goal, &path);
        if (res == 0){
            fprintf(stderr, "\n");
            fprintf(stderr, "Path found. Nodes: %d\n",
                    (int)gannGNGPNodesLen(p->gng));
            printPath(&path, stdout);
            return 1;
        }
    }

    return gannGNGPNodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *data)
{
    params_t *p = data;
    size_t nodes_len;
    FILE *fout;
    char fn[1000];

    nodes_len = gannGNGPNodesLen(p->gng);

    sprintf(fn, "out/%010d.svt", gannGNGPNodesLen(p->gng));
    fout = fopen(fn, "w");
    gannGNGPDumpSVT(p->gng, fout, NULL);
    fclose(fout);

    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d\r", nodes_len, p->max_nodes);
}

static const fer_vec2_t *inputSignal(void *data)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;

    x = ferRandMT(p->rand, -5, 5);
    y = ferRandMT(p->rand, -5, 5);
    ferVec2Set(&p->is, x, y);
    return &p->is;
}

static int eval(const fer_vec2_t *w, void *data)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;
    x = ferVec2X(w);
    y = ferVec2Y(w);

    p->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -0.01 && x < 0.01)
            || (y > 4 && x > -2 && x < 2)){
        return GANN_GNGP_FREE;
    }
    return GANN_GNGP_OBST;
}

static void printPath(fer_list_t *path, FILE *out)
{
    fer_list_t *item;
    gann_gngp_node_t *n;
    size_t id;

    fprintf(out, "------\n");
    fprintf(out, "Name: path\n");
    fprintf(out, "Edge width: 3\n");
    fprintf(out, "Edge color: 0 0 0\n");

    fprintf(out, "Points:\n");
    ferListForEach(path, item){
        n = ferListEntry(item, gann_gngp_node_t, path);
        ferVec2Print(&n->w, out);
        fprintf(out, "\n");
    }

    fprintf(out, "Edges:\n");
    id = 0;
    ferListForEach(path, item){
        if (ferListNext(item) == path)
            break;

        n = ferListEntry(item, gann_gngp_node_t, path);
        fprintf(out, "%d %d\n", id, id + 1);
        id++;
    }

    fprintf(out, "------\n");
}
