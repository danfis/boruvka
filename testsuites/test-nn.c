#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boruvka/nn.h>
#include <boruvka/alloc.h>
#include <boruvka/rand.h>
#include <boruvka/timer.h>
#include <boruvka/dbg.h>

struct _el_t {
    bor_vec2_t v;
    bor_nn_el_t linear;
    bor_nn_el_t gug;
    bor_nn_el_t vp;
};
typedef struct _el_t el_t;


static bor_rand_t r;
static size_t arr_len;
static size_t loops;
static size_t nearest_len;

bor_nn_t *gug, *vp, *linear;


static el_t *elsNew(size_t len)
{
    size_t i;
    bor_real_t x, y;
    el_t *ns;

    ns = BOR_ALLOC_ARR(el_t, arr_len);
    for (i = 0; i < len; i++){
        x = borRand(&r, -15., 15.);
        y = borRand(&r, -20., 20.);

        borVec2Set(&ns[i].v, x, y);
        borNNElInit(linear, &ns[i].linear, (const bor_vec_t *)&ns[i].v);
        borNNElInit(gug, &ns[i].gug, (const bor_vec_t *)&ns[i].v);
        borNNElInit(vp, &ns[i].vp, (const bor_vec_t *)&ns[i].v);

        borNNAdd(linear, &ns[i].linear);
        borNNAdd(gug, &ns[i].gug);
        borNNAdd(vp, &ns[i].vp);
    }

    return ns;
}

static void testCorrect(void)
{
    bor_nn_params_t params;
    bor_real_t range[4] = { -15., 15., -18., 17. };
    el_t *ns, *near[3];
    bor_nn_el_t *el_linear[50], *el_gug[50], *el_vp[50];
    int i, j, k;
    int len_linear, len_gug, len_vp;
    int incorrect;
    bor_vec2_t v;


    borNNParamsInit(&params);
    borNNParamsSetDim(&params, 2);
    params.gug.num_cells = 0;
    params.gug.max_dens = 1;
    params.gug.expand_rate = 2.;
    params.gug.aabb = range;

    params.type = BOR_NN_LINEAR;
    linear = borNNNew(&params);
    params.type = BOR_NN_GUG;
    gug    = borNNNew(&params);
    params.type = BOR_NN_VPTREE;
    vp     = borNNNew(&params);

    ns = elsNew(arr_len);

    for (k = 0; k < nearest_len; k++){
        incorrect = 0;

        for (i=0; i < loops; i++){
            fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            borVec2Set(&v, borRand(&r, -10., 10.), borRand(&r, -10, 10));

            len_linear = borNNNearest(linear, (const bor_vec_t *)&v, k + 1, el_linear);
            len_gug    = borNNNearest(gug, (const bor_vec_t *)&v, k + 1, el_gug);
            len_vp     = borNNNearest(vp, (const bor_vec_t *)&v, k + 1, el_vp);

            if (len_linear != len_gug
                    || len_linear != len_vp
                    || len_vp != len_gug
                    || len_linear != k + 1){
                incorrect = 1;
            }

            for (j = 0; j < k + 1; j++){
                near[0] = bor_container_of(el_linear[j], el_t, linear);
                near[1] = bor_container_of(el_gug[j], el_t, gug);
                near[2] = bor_container_of(el_vp[j], el_t, vp);
                if (near[0] != near[1]
                        || near[0] != near[2]
                        || near[1] != near[2]){
                    incorrect = 1;
                }
            }
        }

        if (incorrect){
            fprintf(stderr, "[%d] %08d / %08d FAIL\n", (int)k, (int)i, (int)loops);
        }else{
            fprintf(stderr, "[%d] %08d / %08d OK\n", (int)k, (int)i, (int)loops);
        }
    }

    BOR_FREE(ns);
    borNNDel(linear);
    borNNDel(gug);
    borNNDel(vp);
}


static void bench(void)
{
    bor_nn_params_t params;
    bor_real_t range[4] = { -15., 15., -18., 17. };
    el_t *ns, *near;
    bor_nn_el_t *el[50];
    int i, j, k;
    bor_vec2_t v;
    int devnull;
    bor_timer_t timer;

    devnull = open("/dev/null", O_WRONLY);
    if (devnull < 0){
        perror("Error: ");
        return;
    }


    borNNParamsInit(&params);
    borNNParamsSetDim(&params, 2);
    params.gug.num_cells = 0;
    params.gug.max_dens = 1;
    params.gug.expand_rate = 2.;
    params.gug.aabb = range;

    params.type = BOR_NN_LINEAR;
    linear = borNNNew(&params);
    params.type = BOR_NN_GUG;
    gug    = borNNNew(&params);
    params.type = BOR_NN_VPTREE;
    vp     = borNNNew(&params);

    ns = elsNew(arr_len);

    for (k = 0; k < nearest_len; k++){
        borTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            borVec2Set(&v, borRand(&r, -10., 10.), borRand(&r, -10, 10));

            borNNNearest(linear, (const bor_vec_t *)&v, k + 1, el);
            for (j = 0; j < k + 1; j++){
                near = bor_container_of(el[j], el_t, linear);
                write(devnull, &near->v, 1);
            }
        }
        borTimerStop(&timer);
        borTimerPrintElapsed(&timer, stderr, " - [%d] - linear -                \n", k);


        borTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            borVec2Set(&v, borRand(&r, -10., 10.), borRand(&r, -10, 10));

            borNNNearest(gug, (const bor_vec_t *)&v, k + 1, el);
            for (j = 0; j < k + 1; j++){
                near = bor_container_of(el[j], el_t, gug);
                write(devnull, &near->v, 1);
            }
        }
        borTimerStop(&timer);
        borTimerPrintElapsed(&timer, stderr, " - [%d] - gug -                \n", k);


        borTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            borVec2Set(&v, borRand(&r, -10., 10.), borRand(&r, -10, 10));

            borNNNearest(vp, (const bor_vec_t *)&v, k + 1, el);
            for (j = 0; j < k + 1; j++){
                near = bor_container_of(el[j], el_t, vp);
                write(devnull, &near->v, 1);
            }
        }
        borTimerStop(&timer);
        borTimerPrintElapsed(&timer, stderr, " - [%d] - vptree -                \n", k);
    }

    BOR_FREE(ns);
    borNNDel(linear);
    borNNDel(gug);
    borNNDel(vp);

    close(devnull);
}

int main(int argc, char *argv[])
{
    if (argc != 5){
        fprintf(stderr, "Usage: %s test|bench arr_len loops nearest_len<50\n", argv[0]);
        return -1;
    }
    arr_len     = atoi(argv[2]);
    loops       = atoi(argv[3]);
    nearest_len = atoi(argv[4]);

    borRandInit(&r);

    if (strcmp(argv[1], "test") == 0){
        testCorrect();
    }else{
        bench();
    }

    return 0;
}
