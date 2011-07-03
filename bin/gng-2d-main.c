#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/gng2.h>

struct _params_t {
    size_t max_nodes;
    fer_gng2_t *gng;
    fer_timer_t timer;
};
typedef struct _params_t params_t;


static int terminate(void *data);
static void callback(void *data);

int main(int argc, char *argv[])
{
    fer_gng2_params_t params;
    fer_gng_ops_t ops;
    fer_gng2_t *gng;
    params_t p;
    size_t size;

    if (argc < 3){
        fprintf(stderr, "Usage: %s file.pts max_nodes\n", argv[0]);
        return -1;
    }

    p.max_nodes = atoi(argv[2]);


    ferGNG2ParamsInit(&params);
    params.use_cubes = 1;
    params.num_cubes = p.max_nodes;

    ferGNG2OpsInit(&ops);
    ops.terminate = terminate;
    ops.callback  = callback;
    ops.callback_period = 300;
    ops.data = &p;

    gng = ferGNG2New(&ops, &params);
    p.gng = gng;
    size = ferGNG2AddInputSignalsFromFile(gng, argv[1]);
    fprintf(stderr, "Added %d points from %s\n", (int)size, argv[1]);


    ferTimerStart(&p.timer);
    ferGNG2Run(gng);
    callback(&p);
    fprintf(stderr, "\n");

    ferGNG2DumpSVT(gng, stdout, NULL);

    ferGNG2Del(gng);

    return 0;
}


static int terminate(void *data)
{
    params_t *p = (params_t *)data;
    return ferGNG2NodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *data)
{
    params_t *p = data;
    size_t nodes_len;

    nodes_len = ferGNG2NodesLen(p->gng);

    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d\r", nodes_len, p->max_nodes);
}
