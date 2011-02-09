#include <stdio.h>
#include "cu.h"
#include <fermat/fibo.h>

static void treeDump(fer_list_t *fl, size_t spaces)
{
    fer_list_t *item;
    fer_fibo_node_t *n;
    size_t i;

    ferListForEach(fl, item){
        n = ferListEntry(item, fer_fibo_node_t, list);
        for (i = 0; i < spaces; i++){
            fprintf(stdout, " ");
        }
        fprintf(stdout, "%lx (%d, %d %d)\n", (long)n, n->value, n->degree, n->mark);
        treeDump(&n->children, spaces + 1);
    }
}

static void fiboDump(fer_fibo_t *fibo)
{
    fprintf(stdout, "---\n");
    fprintf(stdout, "min: %lx\n", (long)fibo->min);
    treeDump(&fibo->root, 0);
    fflush(stdout);
}

TEST(fibo1)
{
    fer_fibo_t *fibo;
    fer_fibo_node_t ns[10];
    fer_fibo_node_t *n;
    size_t i;

    fibo = ferFiboNew();

    ns[0].value = 3;
    ferFiboAdd(fibo, &ns[0]);
    fiboDump(fibo);

    ns[1].value = 1;
    ferFiboAdd(fibo, &ns[1]);
    fiboDump(fibo);

    ns[2].value = 2;
    ferFiboAdd(fibo, &ns[2]);
    fiboDump(fibo);

    ns[3].value = 5;
    ferFiboAdd(fibo, &ns[3]);
    fiboDump(fibo);

    ns[4].value = 5;
    ferFiboAdd(fibo, &ns[4]);
    fiboDump(fibo);

    n = ferFiboExtractMin(fibo);
    fiboDump(fibo);
    fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);

    ns[5].value = 2;
    ferFiboAdd(fibo, &ns[5]);
    fiboDump(fibo);

    ns[6].value = -1;
    ferFiboAdd(fibo, &ns[6]);
    fiboDump(fibo);

    ns[7].value = 12;
    ferFiboAdd(fibo, &ns[7]);
    fiboDump(fibo);

    ns[8].value = 0;
    ferFiboAdd(fibo, &ns[8]);
    fiboDump(fibo);

    ns[9].value = 6;
    ferFiboAdd(fibo, &ns[9]);
    fiboDump(fibo);

    n = ferFiboExtractMin(fibo);
    fiboDump(fibo);
    fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);

    n = ferFiboExtractMin(fibo);
    fiboDump(fibo);
    fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);

    n = ferFiboExtractMin(fibo);
    fiboDump(fibo);
    fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);

    n = ferFiboExtractMin(fibo);
    fiboDump(fibo);
    fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);

    ns[9].value = -1;
    ferFiboDecreaseKey(fibo, &ns[9]);
    fiboDump(fibo);

    ns[7].value = 4;
    ferFiboDecreaseKey(fibo, &ns[7]);
    fiboDump(fibo);

    fprintf(stdout, "====\n");
    n = ferFiboExtractMin(fibo);
    while (n){
        fiboDump(fibo);
        fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);
        n = ferFiboExtractMin(fibo);
    }

    fprintf(stdout, "====++++\n");

    for (i = 0; i < 10; i++){
        ferFiboAdd(fibo, &ns[i]);
    }
    fiboDump(fibo);

    ns[7].value = 1;
    ferFiboDecreaseKey(fibo, &ns[7]);
    fiboDump(fibo);

    fprintf(stdout, "\nremove %lx\n", (long)&ns[0]);
    ferFiboRemove(fibo, &ns[0]);
    fiboDump(fibo);

    ns[1].value = -10;
    ferFiboDecreaseKey(fibo, &ns[1]);
    fiboDump(fibo);

    fprintf(stdout, "\nremove %lx\n", (long)&ns[2]);
    ferFiboRemove(fibo, &ns[2]);
    fiboDump(fibo);

    fprintf(stdout, "====\n");
    n = ferFiboExtractMin(fibo);
    while (n){
        fiboDump(fibo);
        fprintf(stdout, "   -- %lx (%d %d %d)\n", (long)n, n->value, n->degree, n->mark);
        n = ferFiboExtractMin(fibo);
    }

    ferFiboDel(fibo);
}

