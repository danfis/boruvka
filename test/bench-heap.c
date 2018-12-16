#include <boruvka/rand.h>
#include <boruvka/timer.h>
#include <boruvka/alloc.h>

struct _el_t {
    bor_real_t val;
    HNODE node;
    int onheap;
};
typedef struct _el_t el_t;

static FILE *fout;

static el_t *randomEls(size_t num)
{
    bor_rand_t r;
    bor_real_t val;
    el_t *els;
    size_t i;

    borRandInit(&r);

    els = BOR_ALLOC_ARR(el_t, num);
    for (i = 0; i < num; i++){
        val = borRand(&r, -50., 50.);
        els[i].val = val;
        els[i].onheap = 0;
    }

    return els;
}

static int ltEl(const HNODE *n1, const HNODE *n2, void *_)
{
    el_t *el1, *el2;

    el1 = bor_container_of(n1, el_t, node);
    el2 = bor_container_of(n2, el_t, node);

    return el1->val < el2->val;
}

static void IP(HHEAP *heap, el_t *els, size_t num)
{
    HNODE *node;
    size_t i;

    for (i = 0; i < num; i++){
        HFUNC(Add)(heap, &els[i].node);
    }

    while (!HFUNC(Empty)(heap)){
        node = HFUNC(ExtractMin)(heap);
        fprintf(fout, "%lx\n", (long)node);
    }
}

static void IPR(HHEAP *heap, el_t *els, size_t num)
{
    HNODE *node;
    el_t *el;
    size_t i, insert, num_inserts;
    bor_rand_t r;

    borRandInit(&r);

    i = 0;
    insert = 0;
    num_inserts = 0;
    while (num_inserts != num){
        if (insert == 0){
            while (!HFUNC(Empty)(heap)){
                node = HFUNC(ExtractMin)(heap);
                el = bor_container_of(node, el_t, node);
                el->onheap = 0;
                fprintf(fout, "%lx\n", (long)node);
            }

            insert = borRand(&r, 1, 500);
        }

        HFUNC(Add)(heap, &els[i].node);
        els[i].onheap = 1;
        i++;
        num_inserts++;
        insert--;
    }


    while (!HFUNC(Empty)(heap)){
        node = HFUNC(ExtractMin)(heap);
        el = bor_container_of(node, el_t, node);
        el->onheap = 0;
        fprintf(fout, "%lx\n", (long)node);
    }
}

static void IPD(HHEAP *heap, el_t *els, size_t num)
{
    HNODE *node;
    el_t *el;
    size_t i, dec, num_inserts;
    size_t loop;
    bor_rand_t r;

    borRandInit(&r);

    i = 0;
    num_inserts = 0;
    while (num_inserts != num){
        // insert some
        loop = borRand(&r, 1, 1000);
        while (loop > 0 && num_inserts != num){
            HFUNC(Add)(heap, &els[i].node);
            els[i].onheap = 1;
            i++;
            loop--;
            num_inserts++;
        }

        // pop some
        loop = borRand(&r, 1, 1000);
        while (!HFUNC(Empty)(heap) && loop > 0 && num_inserts != num){
            node = HFUNC(ExtractMin)(heap);
            el = bor_container_of(node, el_t, node);
            el->onheap = 0;
            fprintf(fout, "%lx\n", (long)node);
            loop--;
        }

        // decrease some
        loop = borRand(&r, 1, 1000);
        while (loop > 0){
            dec = borRand(&r, 0, num);
            if (els[dec].onheap){
                els[dec].val -= borRand(&r, 1, 10);
                HFUNC(DecreaseKey)(heap, &els[dec].node);
            }

            loop--;
        }

        // pop some
        loop = borRand(&r, 1, 1000);
        while (!HFUNC(Empty)(heap) && loop > 0 && num_inserts != num){
            node = HFUNC(ExtractMin)(heap);
            el = bor_container_of(node, el_t, node);
            el->onheap = 0;
            fprintf(fout, "%lx\n", (long)node);
            loop--;
        }
    }


    while (!HFUNC(Empty)(heap)){
        node = HFUNC(ExtractMin)(heap);
        el = bor_container_of(node, el_t, node);
        el->onheap = 0;
        fprintf(fout, "%lx\n", (long)node);
    }
}

static void IPC(HHEAP *heap, el_t *els, size_t num)
{
    HNODE *node;
    el_t *el;
    size_t i, dec, num_inserts;
    size_t loop;
    bor_rand_t r;

    borRandInit(&r);

    i = 0;
    num_inserts = 0;
    while (num_inserts != num){
        // insert some
        loop = borRand(&r, 1, 1000);
        while (loop > 0 && num_inserts != num){
            HFUNC(Add)(heap, &els[i].node);
            els[i].onheap = 1;
            i++;
            loop--;
            num_inserts++;
        }

        // pop some
        loop = borRand(&r, 1, 1000);
        while (!HFUNC(Empty)(heap) && loop > 0 && num_inserts != num){
            node = HFUNC(ExtractMin)(heap);
            el = bor_container_of(node, el_t, node);
            el->onheap = 0;
            fprintf(fout, "%lx\n", (long)node);
            loop--;
        }

        // change some
        loop = borRand(&r, 1, 1000);
        while (loop > 0){
            dec = borRand(&r, 0, num);
            if (els[dec].onheap){
                els[dec].val += borRand(&r, -10, 10);
                HFUNC(Update)(heap, &els[dec].node);
            }

            loop--;
        }

        // pop some
        loop = borRand(&r, 1, 1000);
        while (!HFUNC(Empty)(heap) && loop > 0 && num_inserts != num){
            node = HFUNC(ExtractMin)(heap);
            el = bor_container_of(node, el_t, node);
            el->onheap = 0;
            fprintf(fout, "%lx\n", (long)node);
            loop--;
        }
    }


    while (!HFUNC(Empty)(heap)){
        node = HFUNC(ExtractMin)(heap);
        el = bor_container_of(node, el_t, node);
        el->onheap = 0;
        fprintf(fout, "%lx\n", (long)node);
    }
}

int main(int argc, char *argv[])
{
    size_t num;
    HHEAP *heap;
    el_t *els;
    bor_timer_t timer;

    if (argc != 3){
        fprintf(stderr, "Usage: %s ip|ipr|ipd|ipc num_els\n", argv[0]);
        return -1;
    }

    fout = fopen("/dev/null", "w");

    num = atoi(argv[2]);

    els = randomEls(num);
    heap = HFUNC(New)(ltEl, NULL);

    borTimerStart(&timer);

    if (strcmp(argv[1], "ip") == 0){
        IP(heap, els, num);
    }else if (strcmp(argv[1], "ipr") == 0){
        IPR(heap, els, num);
    }else if (strcmp(argv[1], "ipd") == 0){
        IPD(heap, els, num);
    }else if (strcmp(argv[1], "ipc") == 0){
        IPC(heap, els, num);
    }else{
        fprintf(stderr, "Unkown method!\n");
    }

    borTimerStopAndPrintElapsed(&timer, stdout, "\n");

    HFUNC(Del)(heap);
    free(els);

    fclose(fout);

    return 0;
}
