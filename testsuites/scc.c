#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/scc.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

struct _graph_t {
    int *graph;
    int size;
};
typedef struct _graph_t graph_t;

static int graph1_table[] = {
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 1, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 1, 0,
};
static graph_t graph1 = {
    graph1_table, 8
};

static long iter(int node_id, void *ud)
{
    return 0;
}

static int next(int node_id, long *iter, void *ud)
{
    const graph_t *g = ud;
    const int *row = g->graph + (node_id * g->size);
    int i;

    for (i = *iter; i < g->size; ++i){
        if (row[i]){
            *iter = i + 1;
            return i;
        }
    }
    return -1;
}

static void print(const bor_scc_t *scc, FILE *fout, const char *header)
{
    int i, j;

    fprintf(fout, "---- %s ----\n", header);
    for (i = 0; i < scc->comp_size; ++i){
        fprintf(fout, "Component %d:", i);
        for (j = 0; j < scc->comp[i].node_size; ++j){
            fprintf(fout, " %d", scc->comp[i].node[j]);
        }
        fprintf(fout, "\n");
    }
    fprintf(fout, "---- %s END ----\n", header);
}

TEST(testSCC)
{
    bor_scc_t scc;

    borSCCInit(&scc, graph1.size, iter, next, &graph1);
    borSCC(&scc);
    print(&scc, stdout, "graph1");
    borSCCFree(&scc);

    borSCCInit(&scc, graph1.size, iter, next, &graph1);
    borSCC1(&scc, 5);
    print(&scc, stdout, "graph1 - 5");
    borSCCFree(&scc);
}
