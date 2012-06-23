#include <stdio.h>
#include <cu/cu.h>
#include <boruvka/sort.h>
#include <boruvka/rand.h>
#include <boruvka/alloc.h>

struct rs_t {
    int i;
    bor_real_t key;
};

#define RS_PTR_LEN 1000

TEST(sortRadixPtr)
{
    bor_rand_t rnd;
    struct rs_t *arr[RS_PTR_LEN], *tmp[RS_PTR_LEN];
    size_t i, len = RS_PTR_LEN;

    ferRandInit(&rnd);

    for (i = 0; i < len; i++){
        arr[i] = FER_ALLOC(struct rs_t);
        arr[i]->i = i;
        arr[i]->key = ferRand(&rnd, -10., 10.);
    }

    ferRadixSortPtr((void **)arr, (void **)tmp, len, fer_offsetof(struct rs_t, key), 0);
    for (i = 1; i < len; i++){
        assertTrue(arr[i]->key > arr[i - 1]->key);
    }


    for (i = 0; i < len; i++){
        arr[i]->key = ferRand(&rnd, -10., 10.);
    }
    ferRadixSortPtr((void **)arr, (void **)tmp, len, fer_offsetof(struct rs_t, key), 1);
    for (i = 1; i < len; i++){
        assertTrue(arr[i]->key < arr[i - 1]->key);
    }

    for (i = 0; i < len; i++){
        FER_FREE(arr[i]);
    }
}
