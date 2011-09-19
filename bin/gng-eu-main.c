#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/gng-eu.h>

size_t max_nodes;
fer_gng_eu_t *gng;
fer_timer_t timer;
fer_pc_t *pc;
fer_pc_it_t pcit;

static int terminate(void *data);
static void callback(void *data);
static const fer_vec_t *input_signal(void *data);

int main(int argc, char *argv[])
{
    fer_gng_eu_params_t params;
    fer_gng_eu_ops_t ops;
    size_t size;
    fer_real_t aabb[30];

    if (argc < 4){
        fprintf(stderr, "Usage: %s dim file.pts max_nodes\n", argv[0]);
        return -1;
    }

    max_nodes = atoi(argv[3]);


    ferGNGEuParamsInit(&params);
    params.dim = atoi(argv[1]);
    params.use_nn = FER_NN_GUG;
    //params.use_nn = FER_NN_VPTREE;
    params.nn.gug.num_cells = 0;
    params.nn.gug.max_dens = 0.1;
    params.nn.gug.expand_rate = 1.5;

    ferGNGEuOpsInit(&ops);
    ops.terminate = terminate;
    ops.input_signal = input_signal;
    ops.callback  = callback;
    ops.callback_period = 300;
    ops.data = NULL;

    pc = ferPCNew(params.dim);
    size = ferPCAddFromFile(pc, argv[2]);
    ferPCAABB(pc, aabb);
    params.nn.gug.aabb = aabb;
    fprintf(stderr, "Added %d points from %s\n", (int)size, argv[2]);

    ferPCPermutate(pc);
    ferPCItInit(&pcit, pc);

    gng = ferGNGEuNew(&ops, &params);

    ferTimerStart(&timer);
    ferGNGEuRun(gng);
    callback(NULL);
    fprintf(stderr, "\n");

    ferGNGEuDumpSVT(gng, stdout, NULL);

    ferGNGEuDel(gng);

    ferPCDel(pc);

    return 0;
}


static int terminate(void *data)
{
    return ferGNGEuNodesLen(gng) >= max_nodes;
}

static void callback(void *data)
{
    size_t nodes_len;

    nodes_len = ferGNGEuNodesLen(gng);

    ferTimerStopAndPrintElapsed(&timer, stderr, " n: %d / %d\r", nodes_len, max_nodes);
}

static const fer_vec_t *input_signal(void *data)
{
    const fer_vec_t *v;

    if (ferPCItEnd(&pcit)){
        ferPCPermutate(pc);
        ferPCItInit(&pcit, pc);
    }
    v = ferPCItGet(&pcit);
    ferPCItNext(&pcit);
    return v;
}
