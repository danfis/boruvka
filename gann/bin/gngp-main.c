#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/pc2.h>
#include <gann/gng-plan.h>

struct _params_t {
    size_t max_nodes;
    gann_gngp_t *gng;
    fer_timer_t timer;

    fer_pc2_t *pc;
    fer_pc2_it_t pcit;
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
    size_t size;
    const fer_real_t *aabb;

    if (argc < 2){
        fprintf(stderr, "Usage: %s file.pts [ max_nodes ]\n", argv[0]);
        return -1;
    }

    gannGNGPParamsInit(&params);

    gannGNGPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    //ops.callback  = callback;
    //ops.callback_period = 100;
    ops.data = &p;

    p.pc = ferPC2New();
    size = ferPC2AddFromFile(p.pc, argv[1]);
    fprintf(stderr, "Added %d points from %s\n", size, argv[1]);
    aabb = ferPC2AABB(p.pc);
    params.aabb[0] = aabb[0];
    params.aabb[1] = aabb[1];
    params.aabb[2] = aabb[2];
    params.aabb[3] = aabb[3];

    ferPC2Permutate(p.pc);
    ferPC2ItInit(&p.pcit, p.pc);

    p.max_nodes = 1000;

    gng = gannGNGPNew(&ops, &params);
    p.gng = gng;

    if (argc >= 3){
        p.max_nodes = atoi(argv[2]);
    }

    ferTimerStart(&p.timer);
    gannGNGPRun(gng);
    callback(&p);
    fprintf(stderr, "\n");

    //gannGNGDumpSVT(gng, stdout, NULL);

    gannGNGPDel(gng);

    ferPC2Del(p.pc);

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
    const fer_vec2_t *is;

    if (ferPC2ItEnd(&p->pcit)){
        ferPC2Permutate(p->pc);
        ferPC2ItInit(&p->pcit, p->pc);
    }

    is = ferPC2ItGet(&p->pcit);
    ferPC2ItNext(&p->pcit);

    return is;
}

static int eval(const fer_vec2_t *w, void *data)
{
    if (ferVec2X(w) < FER_ZERO)
        return GANN_GNGP_FREE;
    return GANN_GNGP_OBST;
}
