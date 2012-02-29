#include <stdio.h>
#include <fermat/gpc.h>
#include <fermat/dbg.h>

struct _data_row_t {
    int a, b, c;
    const char *str;
    int class;
};
typedef struct _data_row_t data_row_t;

data_row_t data[10] = {
    { 1, 2, 3, "aaa", 1 },
    { 1, 4, 3, "aca", 1 },
    { 1, 12, 3, "abg", 1 },
    { 1, 20, 5, "qqq", 1 },
    { 11, 2, 3, "asd", 1 },
    { 1, 2, 3, "alk", 0 },
    { 1, 40, 3, "laa", 0 },
    { 1, 2, 3, "aala", 0 },
    { 1, 10, 3, "12.1", 0 },
    { 1, 2, 13, "120",  0 },
};


void *dataRow(fer_gpc_t *gpc, int i, void *_)
{
    return &data[i];
}

fer_real_t fitness(fer_gpc_t *gpc, int *class, void *_)
{
    int i, ft;

    ft = 0;
    for (i = 0; i < 10; i++){
        ft += (class[i] == data[i].class);
    }

    return ft;
}

void initIntLT(fer_gpc_t *gpc, void *mem, void *ud)
{
    *(unsigned int *)mem = ferGPCRandInt(gpc, 0, 3);
    *((int *)mem + 1) = ferGPCRandInt(gpc, 0, 14);
}

unsigned int predIntLT(fer_gpc_t *gpc, void *mem, void *data, void *ud)
{
    data_row_t *row;
    unsigned int i;
    int val;

    i = *(unsigned int *)mem;
    val = *((int *)mem + 1);
    row = (data_row_t *)data;

    if (i == 0){
        return row->a < val;
    }else if (i == 1){
        return row->b < val;
    }else{
        return row->c < val;
    }
}

unsigned int predStrIsInt(fer_gpc_t *gpc, void *mem, void *data, void *ud)
{
    data_row_t *row;
    const char *str;

    row = (data_row_t *)data;
    str = row->str;
    while (*str){
        if (*str < '0' || *str > '9')
            return 1;
        ++str;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int res;

    fer_gpc_ops_t ops;
    fer_gpc_params_t params;
    fer_gpc_t *gpc;

    ferGPCOpsInit(&ops);
    ops.fitness  = fitness;
    ops.data_row = dataRow;

    ferGPCParamsInit(&params);
    params.pop_size  = 10;
    params.max_depth = 3;
    params.data_rows = 10;
    params.pm = 20;

    gpc = ferGPCNew(&ops, &params);

    ferGPCAddClass(gpc, 1);
    ferGPCAddClass(gpc, 0);
    ferGPCAddPred(gpc, predIntLT, initIntLT, 2, sizeof(int) * 2, NULL);
    ferGPCAddPred(gpc, predStrIsInt, NULL, 2, 0, NULL);

    res = ferGPCRun(gpc);
    printf("res: %d\n", res);

    ferGPCDel(gpc);

    return 0;
}
