#include <stdio.h>
#include <fermat/gpc.h>
#include <fermat/dbg.h>
#include <fermat/alloc.h>

fer_real_t **data;
size_t num_rows;
size_t num_cols;
int *data_cl;
size_t num_cl;

static int readData(const char *fn, fer_real_t ***data, int **cl,
                    size_t *num_rows, size_t *num_cols, size_t *num_cl);

void *dataRow(fer_gpc_t *gpc, int i, void *_)
{
    return (void *)data[i];
}

fer_real_t fitness(fer_gpc_t *gpc, int *class, void *_)
{
    int i, ft;

    ft = 0;
    for (i = 0; i < num_rows; i++){
        ft += (class[i] == data_cl[i]);
    }

    return ((fer_real_t)ft) / num_rows;
}

void callback(fer_gpc_t *gpc, void *_)
{
    fer_gpc_stats_t stats;

    ferGPCStats(gpc, &stats);
    fprintf(stderr, "[%06ld] min: %f, max: %f, avg: %f, med: %f "
                    "| nodes - min: % 2d, max: % 5d, avg: % 7.2f"
                    "| depth - min: % 2d, max: % 5d, avg: % 7.2f\n",
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
    m->idx = ferGPCRandInt(gpc, 0, num_cols);
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
    m->idx1 = ferGPCRandInt(gpc, 0, num_cols);
    do {
        m->idx2 = ferGPCRandInt(gpc, 0, num_cols);
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

    if (argc != 2){
        fprintf(stderr, "Usage: %s data.train\n", argv[0]);
        return -1;
    }

    if (readData(argv[1], &data, &data_cl, &num_rows, &num_cols, &num_cl) != 0){
        fprintf(stderr, "Invalid input file `%s'\n", argv[1]);
        return -1;
    }

    fer_gpc_ops_t ops;
    fer_gpc_params_t params;
    fer_gpc_t *gpc;

    ferGPCOpsInit(&ops);
    ops.fitness  = fitness;
    ops.data_row = dataRow;
    ops.callback = callback;
    ops.callback_period = 20;

    ferGPCParamsInit(&params);
    params.pop_size    = 1000;
    params.max_depth   = 5;
    params.data_rows   = num_rows;
    params.keep_best   = params.pop_size * 0.1;
    params.throw_worst = params.pop_size * 0.1;
    params.max_steps   = 5001;
    params.tournament_size = 5;
    params.pr = 10;
    params.pc = 10;
    params.pm = 10;
    params.simplify = 100UL;
    params.prune_deep = 100UL;

    gpc = ferGPCNew(&ops, &params);

    for (i = 0; i < num_cl; i++){
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

    return 0;
}


static int readData(const char *fn, fer_real_t ***data, int **cl,
                    size_t *num_rows, size_t *num_cols, size_t *num_cl)
{
    FILE *fin;
    int len, num_preds, i, j, ncl;

    fin = fopen(fn, "r");
    if (!fin)
        return -1;

    fscanf(fin, "%d %d %d", &len, &num_preds, &ncl);
    *num_rows = len;
    *num_cols = num_preds;
    *num_cl   = ncl;

    *data = FER_ALLOC_ARR(fer_real_t *, len);
    *cl   = FER_ALLOC_ARR(int, len);

    for (i = 0; i < len; i++){
        // class
        fscanf(fin, "%d", &(*cl)[i]);

        (*data)[i] = FER_ALLOC_ARR(fer_real_t, num_preds);
        for (j = 0; j < num_preds; j++){
            fscanf(fin, "%f", &(*data)[i][j]);
        }
    }


    fclose(fin);

    return 0;
}
