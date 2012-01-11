#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fermat/nn.h>
#include <fermat/alloc.h>
#include <fermat/rand.h>
#include <fermat/timer.h>
#include <fermat/dbg.h>

struct _el_t {
    fer_vec2_t v;
    fer_nn_el_t linear;
    fer_nn_el_t gug;
    fer_nn_el_t vp;
};
typedef struct _el_t el_t;


static fer_rand_t r;
static size_t arr_len;
static size_t loops;
static size_t nearest_len;

fer_nn_t *gug, *vp, *linear;


static el_t *elsNew(size_t len)
{
    size_t i;
    fer_real_t x, y;
    el_t *ns;

    ns = FER_ALLOC_ARR(el_t, arr_len);
    for (i = 0; i < len; i++){
        x = ferRand(&r, -15., 15.);
        y = ferRand(&r, -20., 20.);

        ferVec2Set(&ns[i].v, x, y);
        ferNNElInit(linear, &ns[i].linear, (const fer_vec_t *)&ns[i].v);
        ferNNElInit(gug, &ns[i].gug, (const fer_vec_t *)&ns[i].v);
        ferNNElInit(vp, &ns[i].vp, (const fer_vec_t *)&ns[i].v);

        ferNNAdd(linear, &ns[i].linear);
        ferNNAdd(gug, &ns[i].gug);
        ferNNAdd(vp, &ns[i].vp);
    }

    return ns;
}

static void testCorrect(void)
{
    fer_nn_params_t params;
    fer_real_t range[4] = { -15., 15., -18., 17. };
    el_t *ns, *near[3];
    fer_nn_el_t *el_linear[50], *el_gug[50], *el_vp[50];
    int i, j, k;
    int len_linear, len_gug, len_vp;
    int incorrect;
    fer_vec2_t v;


    ferNNParamsInit(&params);
    ferNNParamsSetDim(&params, 2);
    params.gug.num_cells = 0;
    params.gug.max_dens = 1;
    params.gug.expand_rate = 2.;
    params.gug.aabb = range;

    params.type = FER_NN_LINEAR;
    linear = ferNNNew(&params);
    params.type = FER_NN_GUG;
    gug    = ferNNNew(&params);
    params.type = FER_NN_VPTREE;
    vp     = ferNNNew(&params);

    ns = elsNew(arr_len);

    for (k = 0; k < nearest_len; k++){
        incorrect = 0;

        for (i=0; i < loops; i++){
            fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            len_linear = ferNNNearest(linear, (const fer_vec_t *)&v, k + 1, el_linear);
            len_gug    = ferNNNearest(gug, (const fer_vec_t *)&v, k + 1, el_gug);
            len_vp     = ferNNNearest(vp, (const fer_vec_t *)&v, k + 1, el_vp);

            if (len_linear != len_gug
                    || len_linear != len_vp
                    || len_vp != len_gug
                    || len_linear != k + 1){
                incorrect = 1;
            }

            for (j = 0; j < k + 1; j++){
                near[0] = fer_container_of(el_linear[j], el_t, linear);
                near[1] = fer_container_of(el_gug[j], el_t, gug);
                near[2] = fer_container_of(el_vp[j], el_t, vp);
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

    FER_FREE(ns);
    ferNNDel(linear);
    ferNNDel(gug);
    ferNNDel(vp);
}


static void bench(void)
{
    fer_nn_params_t params;
    fer_real_t range[4] = { -15., 15., -18., 17. };
    el_t *ns, *near;
    fer_nn_el_t *el[50];
    int i, j, k;
    fer_vec2_t v;
    int devnull;
    fer_timer_t timer;

    devnull = open("/dev/null", O_WRONLY);
    if (devnull < 0){
        perror("Error: ");
        return;
    }


    ferNNParamsInit(&params);
    ferNNParamsSetDim(&params, 2);
    params.gug.num_cells = 0;
    params.gug.max_dens = 1;
    params.gug.expand_rate = 2.;
    params.gug.aabb = range;

    params.type = FER_NN_LINEAR;
    linear = ferNNNew(&params);
    params.type = FER_NN_GUG;
    gug    = ferNNNew(&params);
    params.type = FER_NN_VPTREE;
    vp     = ferNNNew(&params);

    ns = elsNew(arr_len);

    for (k = 0; k < nearest_len; k++){
        ferTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNNNearest(linear, (const fer_vec_t *)&v, k + 1, el);
            for (j = 0; j < k + 1; j++){
                near = fer_container_of(el[j], el_t, linear);
                write(devnull, &near->v, 1);
            }
        }
        ferTimerStop(&timer);
        ferTimerPrintElapsed(&timer, stderr, " - [%d] - linear -                \n", k);


        ferTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNNNearest(gug, (const fer_vec_t *)&v, k + 1, el);
            for (j = 0; j < k + 1; j++){
                near = fer_container_of(el[j], el_t, gug);
                write(devnull, &near->v, 1);
            }
        }
        ferTimerStop(&timer);
        ferTimerPrintElapsed(&timer, stderr, " - [%d] - gug -                \n", k);


        ferTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "[%d] %08d / %08d\r", (int)k, (int)i, (int)loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNNNearest(vp, (const fer_vec_t *)&v, k + 1, el);
            for (j = 0; j < k + 1; j++){
                near = fer_container_of(el[j], el_t, vp);
                write(devnull, &near->v, 1);
            }
        }
        ferTimerStop(&timer);
        ferTimerPrintElapsed(&timer, stderr, " - [%d] - vptree -                \n", k);
    }

    FER_FREE(ns);
    ferNNDel(linear);
    ferNNDel(gug);
    ferNNDel(vp);

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

    ferRandInit(&r);

    if (strcmp(argv[1], "test") == 0){
        testCorrect();
    }else{
        bench();
    }

    return 0;
}
