#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand.h>
#include <gann/gng-plan.h>

struct _params_t {
    size_t max_nodes;
    gann_gngp_t *gng;
    fer_timer_t timer;

    fer_rand_t rand;
    fer_vec2_t is;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);
static const fer_vec2_t *inputSignal(void *data);
static int eval(const fer_vec2_t *w, void *data);

int main(int argc, char *argv[])
{
    gann_gngp_params_t params;
    gann_gngp_ops_t ops;
    gann_gngp_t *gng;
    params_t p;

    if (argc < 2){
        fprintf(stderr, "Usage: %s max_nodes\n", argv[0]);
        return -1;
    }

    gannGNGPParamsInit(&params);

    gannGNGPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback  = callback;
    ops.callback_period = 100;
    ops.data = &p;

    ferRandInit(&p.rand);
    params.aabb[0] = -5;
    params.aabb[1] = 5;
    params.aabb[2] = -5;
    params.aabb[3] = 5;

    p.max_nodes = atoi(argv[1]);
    params.num_cubes = p.max_nodes;
    params.cut_subnet_nodes = p.max_nodes / 2;
    //params.cut_subnet_nodes = 500;

    gng = gannGNGPNew(&ops, &params);
    p.gng = gng;

    ferTimerStart(&p.timer);
    gannGNGPRun(gng);
    callback(&p);
    fprintf(stderr, "\n");

    gannGNGPDumpSVT(gng, stdout, NULL);

    gannGNGPDel(gng);

    return 0;
}


static int terminate(void *data)
{
    params_t *p = (params_t *)data;
    return gannGNGPNodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *data)
{
    params_t *p = data;
    size_t nodes_len;

    nodes_len = gannGNGPNodesLen(p->gng);

    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d\r", nodes_len, p->max_nodes);
}

static const fer_vec2_t *inputSignal(void *data)
{
    params_t *p = (params_t *)data;
    fer_real_t x, y;

    x = ferRand(&p->rand, -5, 5);
    y = ferRand(&p->rand, -5, 5);
    ferVec2Set(&p->is, x, y);
    return &p->is;
}

static int eval(const fer_vec2_t *w, void *data)
{
    fer_real_t x, y;
    x = ferVec2X(w);
    y = ferVec2Y(w);

    if (y < -2
            || (y < 4 && y > -2 && x > -0.01 && x < 0.01)
            || (y > 4 && x > -2 && x < 2)){
        return GANN_GNGP_FREE;
    }
    return GANN_GNGP_OBST;
}
