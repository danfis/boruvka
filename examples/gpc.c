#include <stdio.h>
#include <fermat/gpc.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>
#include <fermat/cfg.h>

fer_cfg_t *cfg = NULL;

int classes, cols;

int train_rows;
const fer_real_t *train_x;
const int *train_y;
size_t train_len;

int test_rows;
const fer_real_t *test_x;
const int *test_y;
size_t test_len;

static int readCfg(const char *fn);

void *dataRow(fer_gpc_t *gpc, int i, void *_)
{
    return (void *)(train_x + (i * cols));
}

fer_real_t fitness(fer_gpc_t *gpc, int *class, void *_)
{
    int i, ft;

    ft = 0;
    for (i = 0; i < train_rows; i++){
        ft += (class[i] == train_y[i]);
    }

    return ((fer_real_t)ft) / train_rows;
}

void callback(fer_gpc_t *gpc, void *_)
{
    fer_gpc_stats_t stats;

    ferGPCStats(gpc, &stats);
    fprintf(stderr, "[%06ld] min: %f, max: %f, avg: %f, med: %f "
                    "| nodes - min: % 2d, max: % 5d, avg: % 7.2f "
                    "| depth - min: % 2d, max: % 5d, avg: % 7.2f\r",
            stats.elapsed, stats.min_fitness, stats.max_fitness,
            stats.avg_fitness, stats.med_fitness,
            (int)stats.min_nodes, (int)stats.max_nodes, stats.avg_nodes,
            (int)stats.min_depth, (int)stats.max_depth, stats.avg_depth);
    fflush(stderr);
}


struct cmp_t {
    int idx;
    fer_real_t val;
};

void cmpInit(fer_gpc_t *gpc, void *mem, void *ud)
{
    struct cmp_t *m = (struct cmp_t *)mem;
    m->idx = ferGPCRandInt(gpc, 0, cols);
    m->val = ferGPCRand(gpc, -1., 1.);
}

unsigned int ltPred(fer_gpc_t *gpc, void *mem, void *data, void *ud)
{
    struct cmp_t *m = (struct cmp_t *)mem;
    fer_real_t *d = (fer_real_t *)data;
    return d[m->idx] < m->val;
}

void ltFormat(fer_gpc_t *gpc, void *mem, void *ud, char *str, size_t max)
{
    struct cmp_t *m = (struct cmp_t *)mem;
    snprintf(str, max, "lt[%d] < %f", (int)m->idx, m->val);
}

unsigned int gtPred(fer_gpc_t *gpc, void *mem, void *data, void *ud)
{
    struct cmp_t *m = (struct cmp_t *)mem;
    fer_real_t *d = (fer_real_t *)data;
    return d[m->idx] > m->val;
}

void gtFormat(fer_gpc_t *gpc, void *mem, void *ud, char *str, size_t max)
{
    struct cmp_t *m = (struct cmp_t *)mem;
    snprintf(str, max, "gt[%d] > %f", (int)m->idx, m->val);
}

struct cmp2_t {
    int idx1, idx2;
};

void cmp2Init(fer_gpc_t *gpc, void *mem, void *ud)
{
    struct cmp2_t *m = (struct cmp2_t *)mem;
    m->idx1 = ferGPCRandInt(gpc, 0, cols);
    do {
        m->idx2 = ferGPCRandInt(gpc, 0, cols);
    } while (m->idx2 == m->idx1);
}

unsigned int lt2Pred(fer_gpc_t *gpc, void *mem, void *data, void *ud)
{
    struct cmp2_t *m = (struct cmp2_t *)mem;
    fer_real_t *d = (fer_real_t *)data;
    return d[m->idx1] < d[m->idx2];
}

void lt2Format(fer_gpc_t *gpc, void *mem, void *ud, char *str, size_t max)
{
    struct cmp2_t *m = (struct cmp2_t *)mem;
    snprintf(str, max, "[%d] < [%d]", m->idx1, m->idx2);
}

unsigned int gt2Pred(fer_gpc_t *gpc, void *mem, void *data, void *ud)
{
    struct cmp2_t *m = (struct cmp2_t *)mem;
    fer_real_t *d = (fer_real_t *)data;
    return d[m->idx1] > d[m->idx2];
}

void gt2Format(fer_gpc_t *gpc, void *mem, void *ud, char *str, size_t max)
{
    struct cmp2_t *m = (struct cmp2_t *)mem;
    snprintf(str, max, "[%d] > [%d]", m->idx1, m->idx2);
}


int main(int argc, char *argv[])
{
    int res;
    size_t i;
    fer_gpc_ops_t ops;
    fer_gpc_params_t params;
    fer_gpc_t *gpc;


    if (argc != 2){
        fprintf(stderr, "Usage: %s data.train\n", argv[0]);
        return -1;
    }

    if (readCfg(argv[1]) != 0){
        fprintf(stderr, "Invalid input file `%s'\n", argv[1]);
        return -1;
    }

    ferGPCOpsInit(&ops);
    ops.fitness  = fitness;
    ops.data_row = dataRow;
    ops.callback = callback;
    ops.callback_period = 20;

    ferGPCParamsInit(&params);
    params.pop_size    = 2000;
    params.max_depth   = 10;
    params.data_rows   = train_rows;
    params.keep_best   = params.pop_size * 0.05;
    params.throw_worst = params.pop_size * 0.05;
    params.max_steps   = 10040;
    params.tournament_size = 5;
    params.pr = 10;
    params.pc = 10;
    params.pm = 10;
    params.simplify = 100UL;
    params.prune_deep = 50UL;

    gpc = ferGPCNew(&ops, &params);

    for (i = 0; i < classes; i++){
        ferGPCAddClass(gpc, i);
    }

    ferGPCAddPred(gpc, ltPred, cmpInit, ltFormat, 2, sizeof(struct cmp_t), NULL);
    ferGPCAddPred(gpc, gtPred, cmpInit, gtFormat, 2, sizeof(struct cmp_t), NULL);
    ferGPCAddPred(gpc, lt2Pred, cmp2Init, lt2Format, 2, sizeof(struct cmp_t), NULL);
    ferGPCAddPred(gpc, gt2Pred, cmp2Init, gt2Format, 2, sizeof(struct cmp_t), NULL);

    res = ferGPCRun(gpc);
    callback(gpc, NULL);
    fprintf(stderr, "\n");
    printf("res: %d\n", res);

    ferGPCPrintBest(gpc, stdout);
    fprintf(stdout, "Best fitness: %f\n", ferGPCBestFitness(gpc));

    ferGPCDel(gpc);

    if (cfg)
        ferCfgDel(cfg);

    return 0;
}


static int readCfg(const char *fn)
{
    size_t len;

    if ((cfg = ferCfgRead(fn)) == NULL)
        return -1;

    if (!ferCfgParamIsInt(cfg, "classes")
            || !ferCfgParamIsInt(cfg, "cols")
            || !ferCfgParamIsInt(cfg, "train_rows")
            || !ferCfgParamIsInt(cfg, "test_rows")
            || !ferCfgParamIsFlt(cfg, "train_x")
            || !ferCfgParamIsArr(cfg, "train_x")
            || !ferCfgParamIsInt(cfg, "train_y")
            || !ferCfgParamIsArr(cfg, "train_y")
            || !ferCfgParamIsFlt(cfg, "test_x")
            || !ferCfgParamIsArr(cfg, "test_x")
            || !ferCfgParamIsInt(cfg, "test_y")
            || !ferCfgParamIsArr(cfg, "test_y")
       ){
        ferCfgDel(cfg);
        return -1;
    }

    ferCfgParamInt(cfg, "classes", &classes);
    ferCfgParamInt(cfg, "cols", &cols);

    ferCfgParamInt(cfg, "train_rows", &train_rows);
    ferCfgParamFltArr(cfg, "train_x", &train_x, &len);
    ferCfgParamIntArr(cfg, "train_y", &train_y, &train_len);
    if (len / cols != train_len){
        fprintf(stderr, "Error: len(train_x) != len(train_y) [%d != %d]\n",
                (int)train_len, (int)(len / cols));
        ferCfgDel(cfg);
        return -1;
    }

    ferCfgParamInt(cfg, "test_rows", &test_rows);
    ferCfgParamFltArr(cfg, "test_x", &test_x, &len);
    ferCfgParamIntArr(cfg, "test_y", &test_y, &test_len);
    if (len / cols != test_len){
        fprintf(stderr, "Error: len(test_x) != len(test_y) [%d != %d]\n",
                (int)test_len, (int)(len / cols));
        ferCfgDel(cfg);
        return -1;
    }

    return 0;
}
