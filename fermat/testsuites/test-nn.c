#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fermat/nncells.h>
#include <fermat/cubes2.h>
#include <fermat/nearest-linear.h>
#include <fermat/rand.h>
#include <fermat/timer.h>
#include <fermat/dbg.h>

struct _el_t {
    fer_vec2_t v;
    fer_nncells_el_t c;
    fer_cubes2_el_t c2;
    fer_list_t list;
};
typedef struct _el_t el_t;


static fer_rand_t r;
static size_t arr_len;
static size_t loops;
static size_t nearest_len;

static void elNew(el_t *ns, size_t len, fer_list_t *head)
{
    size_t i;
    fer_real_t x, y;

    ferListInit(head);

    for (i = 0; i < len; i++){
        x = ferRand(&r, -15., 15.);
        y = ferRand(&r, -20., 20.);

        ferVec2Set(&ns[i].v, x, y);
        ferNNCellsElInit(&ns[i].c, (const fer_vec_t *)&ns[i].v);
        ferCubes2ElInit(&ns[i].c2, &ns[i].v);

        ferListAppend(head, &ns[i].list);
    }
}

static void elAdd(fer_nncells_t *cs, fer_cubes2_t *cs2, el_t *ns, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++){
        ferNNCellsAdd(cs, &ns[i].c);
        ferCubes2Add(cs2, &ns[i].c2);
    }
}

static fer_real_t dist2(void *item1, fer_list_t *item2)
{
    el_t *el2;
    fer_vec2_t *v;

    v   = (fer_vec2_t *)item1;
    el2 = fer_container_of(item2, el_t, list);
    return ferVec2Dist2(v, &el2->v);
}

static void testCorrect(void)
{
    fer_vec2_t v;
    fer_list_t head;
    el_t *ns;
    fer_nncells_el_t *nsc[50];
    fer_cubes2_el_t *nsc2[50];
    fer_list_t *nsl[50];
    el_t *near[10];
    fer_nncells_t *cs;
    fer_cubes2_t *cs2;
    fer_real_t range[4] = { -15., 15., -18., 17. };
    size_t num = 500, i, j, k;
    int incorrect = 0;

    printf("nncells2Nearest:\n");

    ns = (el_t *)malloc(sizeof(el_t) * arr_len);

    cs = ferNNCellsNew(2, range, num);
    cs2 = ferCubes2New(range, num);
    ferVec2Set(&v, 0., 0.1);
    elNew(ns, arr_len, &head);
    elAdd(cs, cs2, ns, arr_len);

    for (k = 0; k < nearest_len; k++){
        for (i=0; i < loops; i++){
            fprintf(stderr, "[%d] %08d / %08d\r", k, i, loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNNCellsNearest(cs, (const fer_vec_t *)&v, k + 1, nsc);
            ferCubes2Nearest(cs2, &v, k + 1, nsc2);
            ferNearestLinear(&head, &v, dist2, nsl, k + 1);

            for (j = 0; j < k + 1; j++){
                near[0] = fer_container_of(nsc[j], el_t, c);
                near[1] = fer_container_of(nsc2[j], el_t, c2);
                near[2] = ferListEntry(nsl[j], el_t, list);
                if (near[0] != near[1]
                        || near[0] != near[2]
                        || near[1] != near[2]){
                    incorrect = 1;
                }
            }
        }

        if (incorrect){
            fprintf(stderr, "[%d] %08d / %08d FAIL\n", k, i, loops);
        }else{
            fprintf(stderr, "[%d] %08d / %08d OK\n", k, i, loops);
        }
    }

    ferNNCellsDel(cs);
    ferCubes2Del(cs2);

    free(ns);
}


static void bench(void)
{
    fer_vec2_t v;
    fer_list_t head;
    el_t *ns;
    fer_nncells_el_t *nsc[50];
    fer_cubes2_el_t *nsc2[50];
    fer_list_t *nsl[50];
    el_t *near[10];
    fer_nncells_t *cs;
    fer_cubes2_t *cs2;
    fer_real_t range[4] = { -9., 9., -11., 7. };
    size_t num = 40, i, j, k;
    fer_timer_t timer;
    int devnull;

    devnull = open("/dev/null", O_WRONLY);

    ns = (el_t *)malloc(sizeof(el_t) * arr_len);

    cs = ferNNCellsNew(2, range, num);
    cs2 = ferCubes2New(range, num);
    ferVec2Set(&v, 0., 0.1);
    elNew(ns, arr_len, &head);
    elAdd(cs, cs2, ns, arr_len);

    for (k = 0; k < nearest_len; k++){
        ferTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "nncells [%d] %08d / %08d\r", k, i, loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNNCellsNearest(cs, (const fer_vec_t *)&v, k + 1, nsc);
            for (j = 0; j < k + 1; j++){
                near[0] = fer_container_of(nsc[j], el_t, c);
                write(devnull, &near[0]->v, 1);
            }
        }
        ferTimerStop(&timer);
        ferTimerPrintElapsed(&timer, stderr, " - [%d] - nncells -                \n", k);


        ferTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "nncells [%d] %08d / %08d\r", k, i, loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferCubes2Nearest(cs2, &v, k + 1, nsc2);
            for (j = 0; j < k + 1; j++){
                near[0] = fer_container_of(nsc2[j], el_t, c2);
                write(devnull, &near[0]->v, 1);
            }
        }
        ferTimerStop(&timer);
        ferTimerPrintElapsed(&timer, stderr, " - [%d] - cubes2 -                \n", k);


        ferTimerStart(&timer);
        for (i=0; i < loops; i++){
            //fprintf(stderr, "nncells [%d] %08d / %08d\r", k, i, loops);
            ferVec2Set(&v, ferRand(&r, -10., 10.), ferRand(&r, -10, 10));

            ferNearestLinear(&head, &v, dist2, nsl, k + 1);
            for (j = 0; j < k + 1; j++){
                near[0] = ferListEntry(nsl[j], el_t, list);
                write(devnull, &near[0]->v, 1);
            }
        }
        ferTimerStop(&timer);
        ferTimerPrintElapsed(&timer, stderr, " - [%d] - linear -                \n", k);

    }

    ferNNCellsDel(cs);

    free(ns);

    close(devnull);
}

int main(int argc, char *argv[])
{
    if (argc != 5){
        fprintf(stderr, "Usage: %s test|bench arr_len loops nearest_len\n", argv[0]);
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
