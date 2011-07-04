#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/gng-eu.h>

struct _params_t {
    size_t max_nodes;
    fer_gng_eu_t *gng;
    fer_timer_t timer;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);

int main(int argc, char *argv[])
{
    fer_gng_eu_params_t params;
    fer_gng_ops_t ops;
    fer_gng_eu_t *gng;
    params_t p;
    size_t size;

    if (argc < 4){
        fprintf(stderr, "Usage: %s dim file.pts max_nodes\n", argv[0]);
        return -1;
    }

    p.max_nodes = atoi(argv[3]);


    ferGNGEuParamsInit(&params);
    params.dim = atoi(argv[1]);
    params.use_cells = 1;
    params.cells.num_cells = p.max_nodes;

    ferGNGEuOpsInit(&ops);
    ops.terminate = terminate;
    ops.callback  = callback;
    ops.callback_period = 300;
    ops.data = &p;

    gng = ferGNGEuNew(&ops, &params);
    p.gng = gng;
    size = ferGNGEuAddInputSignalsFromFile(gng, argv[2]);
    fprintf(stderr, "Added %d points from %s\n", (int)size, argv[2]);


    ferTimerStart(&p.timer);
    ferGNGEuRun(gng);
    callback(&p);
    fprintf(stderr, "\n");

    ferGNGEuDumpSVT(gng, stdout, NULL);

    ferGNGEuDel(gng);

    return 0;
}


static int terminate(void *data)
{
    params_t *p = (params_t *)data;
    return ferGNGEuNodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *data)
{
    params_t *p = data;
    size_t nodes_len;

    nodes_len = ferGNGEuNodesLen(p->gng);

    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d\r", nodes_len, p->max_nodes);
}
