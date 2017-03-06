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

    borRandInit(&rnd);

    for (i = 0; i < len; i++){
        arr[i] = BOR_ALLOC(struct rs_t);
        arr[i]->i = i;
        arr[i]->key = borRand(&rnd, -10., 10.);
    }

    borRadixSortPtr((void **)arr, (void **)tmp, len, bor_offsetof(struct rs_t, key), 0);
    for (i = 1; i < len; i++){
        assertTrue(arr[i]->key > arr[i - 1]->key);
    }


    for (i = 0; i < len; i++){
        arr[i]->key = borRand(&rnd, -10., 10.);
    }
    borRadixSortPtr((void **)arr, (void **)tmp, len, bor_offsetof(struct rs_t, key), 1);
    for (i = 1; i < len; i++){
        assertTrue(arr[i]->key < arr[i - 1]->key);
    }

    for (i = 0; i < len; i++){
        BOR_FREE(arr[i]);
    }
}

static int countSortKey(const void *e, void *_)
{
    return *(int *)e;
}

static void testSortCountInt(bor_rand_t *rnd, int size, int from, int to)
{
    int *arr;
    int i;
    double r;

    arr = BOR_ALLOC_ARR(int, size);
    for (i = 0; i < size; ++i){
        r = borRand(rnd, from, to);
        if (r > ((double)to) - .5){
            arr[i] = to;
        }else{
            arr[i] = r;
        }
    }

    borCountSort(arr, size, sizeof(int), from, to, countSortKey, NULL);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1] <= arr[i]);
    }

    BOR_FREE(arr);
}

static void testSortCountRS(bor_rand_t *rnd, int size, int from, int to)
{
    struct rs_t *arr;
    int i;
    double r;

    arr = BOR_ALLOC_ARR(struct rs_t, size);
    for (i = 0; i < size; ++i){
        r = borRand(rnd, from, to);
        if (r > ((double)to) - .5){
            arr[i].i = to;
        }else{
            arr[i].i = r;
        }
        arr[i].key = r;
    }

    borCountSort(arr, size, sizeof(struct rs_t), from, to, countSortKey, NULL);
    if (arr[0].key > (double)to - .5){
        assertEquals(arr[0].i, to);
    }else{
        assertEquals((int)arr[0].key, arr[0].i);
    }
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].i <= arr[i].i);
        if (arr[i].key > (double)to - .5){
            assertEquals(arr[i].i, to);
        }else{
            assertEquals((int)arr[i].key, arr[i].i);
        }
    }

    BOR_FREE(arr);
}

TEST(sortCount)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortCountInt(&rnd, 100, -10, 5);
    testSortCountInt(&rnd, 1000, 0, 100);
    testSortCountInt(&rnd, 10000, -15, 132);
    testSortCountRS(&rnd, 100, -10, 5);
    testSortCountRS(&rnd, 1000, 0, 100);
    testSortCountRS(&rnd, 10000, -15, 132);
}
