#include <stdio.h>
#include <limits.h>
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
        assertTrue(arr[i]->key >= arr[i - 1]->key);
    }


    for (i = 0; i < len; i++){
        arr[i]->key = borRand(&rnd, -10., 10.);
    }
    borRadixSortPtr((void **)arr, (void **)tmp, len, bor_offsetof(struct rs_t, key), 1);
    for (i = 1; i < len; i++){
        assertTrue(arr[i]->key <= arr[i - 1]->key);
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

static int cmpInt(const void *a, const void *b, void *_)
{
    return *(int *)a - *(int *)b;
}

static int cmpRS(const void *a, const void *b, void *_)
{
    return ((struct rs_t *)a)->i - ((struct rs_t *)b)->i;
}

static int cmpRSX(const void *a, const void *b, void *d)
{
    assertEquals((long)d, 12345L);
    return ((struct rs_t *)a)->i - ((struct rs_t *)b)->i;
}


static void testSortInt(bor_rand_t *rnd, int size, int from, int to,
                void (*sort)(void *, size_t, size_t, bor_sort_cmp, void *))
{
    int *arr;
    int i;

    arr = BOR_ALLOC_ARR(int, size);
    for (i = 0; i < size; ++i){
        arr[i] = borRand(rnd, from, to);
    }

    sort(arr, size, sizeof(int), cmpInt, NULL);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1] <= arr[i]);
    }

    BOR_FREE(arr);
}

static void testSortInt2(bor_rand_t *rnd, int size, int from, int to,
                int (*sort)(void *, size_t, size_t, bor_sort_cmp, void *))
{
    int *arr;
    int i, ret;

    arr = BOR_ALLOC_ARR(int, size);
    for (i = 0; i < size; ++i){
        arr[i] = borRand(rnd, from, to);
    }

    ret = sort(arr, size, sizeof(int), cmpInt, NULL);
    assertEquals(ret, 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1] <= arr[i]);
    }

    BOR_FREE(arr);
}

static void testSortRS(bor_rand_t *rnd, int size, int from, int to,
                void (*sort)(void *, size_t, size_t, bor_sort_cmp, void *),
                int cmpx)
{
    struct rs_t *arr;
    int i;

    arr = BOR_ALLOC_ARR(struct rs_t, size);
    for (i = 0; i < size; ++i){
        arr[i].key = borRand(rnd, from, to);
        arr[i].i = arr[i].key;
    }

    sort(arr, size, sizeof(struct rs_t),
         (cmpx ? cmpRSX : cmpRS), (void *)12345);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].i <= arr[i].i);
    }

    BOR_FREE(arr);
}

static void testSortRS2(bor_rand_t *rnd, int size, int from, int to,
                int (*sort)(void *, size_t, size_t, bor_sort_cmp, void *),
                int cmpx)
{
    struct rs_t *arr;
    int i, ret;

    arr = BOR_ALLOC_ARR(struct rs_t, size);
    for (i = 0; i < size; ++i){
        arr[i].key = borRand(rnd, from, to);
        arr[i].i = arr[i].key;
    }

    ret = sort(arr, size, sizeof(struct rs_t),
               (cmpx ? cmpRSX : cmpRS), (void *)12345);
    assertEquals(ret, 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].i <= arr[i].i);
    }

    BOR_FREE(arr);
}

static void testSortIntSpec(bor_rand_t *rnd, int size, int from, int to,
                            void (*sort)(int *, size_t))
{
    int *arr;
    int i;

    arr = BOR_ALLOC_ARR(int, size);
    for (i = 0; i < size; ++i){
        arr[i] = borRand(rnd, from, to);
    }

    sort(arr, size);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1] <= arr[i]);
    }

    BOR_FREE(arr);
}

TEST(sortInsert)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortInt(&rnd, 1, -10, 5, borInsertSort);
    testSortInt(&rnd, 2, -10, 5, borInsertSort);
    testSortInt(&rnd, 3, INT_MIN / 100, INT_MAX / 100, borInsertSort);
    testSortInt(&rnd, 100, -10, 5, borInsertSort);
    testSortInt(&rnd, 1000, 0, INT_MAX / 100, borInsertSort);
    testSortInt(&rnd, 1000, -15, 132, borInsertSort);

    testSortRS(&rnd, 1, -10, 5, borInsertSort, 0);
    testSortRS(&rnd, 2, -10, 5, borInsertSort, 0);
    testSortRS(&rnd, 3, -10, 5, borInsertSort, 0);
    testSortRS(&rnd, 100, -10, 5, borInsertSort, 1);
    testSortRS(&rnd, 1000, 0, 100, borInsertSort, 0);
    testSortRS(&rnd, 1000, INT_MIN / 100, INT_MAX / 100, borInsertSort, 0);

    testSortIntSpec(&rnd, 1, 10, 5, borInsertSortInt);
    testSortIntSpec(&rnd, 2, -10, 5, borInsertSortInt);
    testSortIntSpec(&rnd, 3, -10, 5, borInsertSortInt);
    testSortIntSpec(&rnd, 100, -10, 5, borInsertSortInt);
    testSortIntSpec(&rnd, 1000, 0, 100, borInsertSortInt);
    testSortIntSpec(&rnd, 1000, INT_MIN, INT_MAX, borInsertSortInt);
}

TEST(sortHeap)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortInt(&rnd, 1, -10, 5, borHeapSort);
    testSortInt(&rnd, 2, -10, 5, borHeapSort);
    testSortInt(&rnd, 3, -10, 5, borHeapSort);
    testSortInt(&rnd, 100, -10, 5, borHeapSort);
    testSortInt(&rnd, 1000, 0, 100, borHeapSort);
    testSortInt(&rnd, 1000, -15, 132, borHeapSort);

    testSortRS(&rnd, 1, -10, 5, borHeapSort, 0);
    testSortRS(&rnd, 2, -10, 5, borHeapSort, 0);
    testSortRS(&rnd, 3, -10, 5, borHeapSort, 0);
    testSortRS(&rnd, 100, -10, 5, borHeapSort, 1);
    testSortRS(&rnd, 1000, 0, 100, borHeapSort, 0);
    testSortRS(&rnd, 1000, INT_MIN / 100, INT_MAX / 100, borHeapSort, 0);
}

TEST(sortMerge)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortInt2(&rnd, 1, -10, 5, borMergeSort);
    testSortInt2(&rnd, 2, -10, 5, borMergeSort);
    testSortInt2(&rnd, 3, -10, 5, borMergeSort);
    testSortInt2(&rnd, 100, -10, 5, borMergeSort);
    testSortInt2(&rnd, 1000, 0, 100, borMergeSort);
    testSortInt2(&rnd, 1000, -15, 132, borMergeSort);

    testSortRS2(&rnd, 1, -10, 5, borMergeSort, 0);
    testSortRS2(&rnd, 2, -10, 5, borMergeSort, 0);
    testSortRS2(&rnd, 3, -10, 5, borMergeSort, 0);
    testSortRS2(&rnd, 100, -10, 5, borMergeSort, 1);
    testSortRS2(&rnd, 1000, 0, 100, borMergeSort, 0);
    testSortRS2(&rnd, 1000, INT_MIN / 100, INT_MAX / 100, borMergeSort, 0);
}

TEST(sortQuick)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortInt(&rnd, 1, -10, 5, borQSort);
    testSortInt(&rnd, 2, -10, 5, borQSort);
    testSortInt(&rnd, 3, -10, 5, borQSort);
    testSortInt(&rnd, 100, -10, 5, borQSort);
    testSortInt(&rnd, 1000, 0, 100, borQSort);
    testSortInt(&rnd, 1000, -15, 132, borQSort);

    testSortRS(&rnd, 1, -10, 5, borQSort, 0);
    testSortRS(&rnd, 2, -10, 5, borQSort, 0);
    testSortRS(&rnd, 3, -10, 5, borQSort, 0);
    testSortRS(&rnd, 100, -10, 5, borQSort, 1);
    testSortRS(&rnd, 1000, 0, 100, borQSort, 0);
    testSortRS(&rnd, 1000, INT_MIN / 100, INT_MAX / 100, borQSort, 0);
}

TEST(sortTim)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortInt2(&rnd, 1, -10, 5, borTimSort);
    testSortInt2(&rnd, 2, -10, 5, borTimSort);
    testSortInt2(&rnd, 3, -10, 5, borTimSort);
    testSortInt2(&rnd, 100, -10, 5, borTimSort);
    testSortInt2(&rnd, 1000, 0, 100, borTimSort);
    testSortInt2(&rnd, 1000, -15, 132, borTimSort);

    testSortRS2(&rnd, 1, -10, 5, borTimSort, 0);
    testSortRS2(&rnd, 2, -10, 5, borTimSort, 0);
    testSortRS2(&rnd, 3, -10, 5, borTimSort, 0);
    testSortRS2(&rnd, 100, -10, 5, borTimSort, 1);
    testSortRS2(&rnd, 1000, 0, 100, borTimSort, 0);
    testSortRS2(&rnd, 1000, INT_MIN / 100, INT_MAX / 100, borTimSort, 0);
}

TEST(sort)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortInt2(&rnd, 1, -10, 5, borSort);
    testSortInt2(&rnd, 2, -10, 5, borSort);
    testSortInt2(&rnd, 3, -10, 5, borSort);
    testSortInt2(&rnd, 100, -10, 5, borSort);
    testSortInt2(&rnd, 1000, 0, 100, borSort);
    testSortInt2(&rnd, 1000, -15, 132, borSort);

    testSortRS2(&rnd, 1, -10, 5, borSort, 0);
    testSortRS2(&rnd, 2, -10, 5, borSort, 0);
    testSortRS2(&rnd, 3, -10, 5, borSort, 0);
    testSortRS2(&rnd, 100, -10, 5, borSort, 1);
    testSortRS2(&rnd, 1000, 0, 100, borSort, 0);
    testSortRS2(&rnd, 1000, INT_MIN / 100, INT_MAX / 100, borSort, 0);
}

static void testSortByIntKeyInt(bor_rand_t *rnd, int size, int from, int to)
{
    int *arr;
    int i;

    arr = BOR_ALLOC_ARR(int, size);
    for (i = 0; i < size; ++i){
        arr[i] = borRand(rnd, from, to);
    }

    borSortByIntKey(arr, size, sizeof(int), 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1] <= arr[i]);
    }

    BOR_FREE(arr);
}

static void testSortByLongKeyLong(bor_rand_t *rnd, int size, int from, int to)
{
    long *arr;
    int i;

    arr = BOR_ALLOC_ARR(long, size);
    for (i = 0; i < size; ++i){
        arr[i] = borRand(rnd, from, to);
    }

    borSortByLongKey(arr, size, sizeof(long), 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1] <= arr[i]);
    }

    BOR_FREE(arr);
}

struct s_t {
    double x;
    int i;
    long l;
};

static void testSortByIntKeyS(bor_rand_t *rnd, int size, int from, int to)
{
    struct s_t *arr;
    int i, ret;

    arr = BOR_ALLOC_ARR(struct s_t, size);
    for (i = 0; i < size; ++i){
        arr[i].x = borRand(rnd, from, to);
        arr[i].i = arr[i].x;
    }

    ret = borSortByIntKey(arr, size, sizeof(struct s_t),
                          bor_offsetof(struct s_t, i));
    assertEquals(ret, 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].i <= arr[i].i);
    }

    BOR_FREE(arr);

    arr = BOR_ALLOC_ARR(struct s_t, size);
    for (i = 0; i < size; ++i){
        arr[i].x = borRand(rnd, from, to);
        arr[i].i = arr[i].x;
    }

    ret = BOR_SORT_BY_INT_KEY(arr, size, struct s_t, i);
    assertEquals(ret, 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].i <= arr[i].i);
    }

    BOR_FREE(arr);
}

static void testSortByLongKeyS(bor_rand_t *rnd, int size, long from, long to)
{
    struct s_t *arr;
    int i, ret;

    arr = BOR_ALLOC_ARR(struct s_t, size);
    for (i = 0; i < size; ++i){
        arr[i].x = borRand(rnd, from, to);
        arr[i].l = arr[i].x;
    }

    ret = borSortByLongKey(arr, size, sizeof(struct s_t),
                           bor_offsetof(struct s_t, l));
    assertEquals(ret, 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].l <= arr[i].l);
    }

    BOR_FREE(arr);

    arr = BOR_ALLOC_ARR(struct s_t, size);
    for (i = 0; i < size; ++i){
        arr[i].x = borRand(rnd, from, to);
        arr[i].l = arr[i].x;
    }

    ret = BOR_SORT_BY_LONG_KEY(arr, size, struct s_t, l);
    assertEquals(ret, 0);
    for (i = 1; i < size; ++i){
        assertTrue(arr[i - 1].l <= arr[i].l);
    }

    BOR_FREE(arr);
}

TEST(sortByIntKey)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortByIntKeyInt(&rnd, 1, -10, 5);
    testSortByIntKeyInt(&rnd, 2, -10, 5);
    testSortByIntKeyInt(&rnd, 3, -10, 5);
    testSortByIntKeyInt(&rnd, 100, -10, 5);
    testSortByIntKeyInt(&rnd, 1000, 0, 126);
    testSortByIntKeyInt(&rnd, 1000, -128, 30);

    testSortByIntKeyS(&rnd, 1, -10, 5);
    testSortByIntKeyS(&rnd, 2, -10, 5);
    testSortByIntKeyS(&rnd, 3, -10, 5);
    testSortByIntKeyS(&rnd, 100, -10, 5);
    testSortByIntKeyS(&rnd, 1000, 0, 126);
    testSortByIntKeyS(&rnd, 1000, INT_MIN, INT_MAX);
}

TEST(sortByLongKey)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testSortByLongKeyLong(&rnd, 1, -10, 5);
    testSortByLongKeyLong(&rnd, 2, -10, 5);
    testSortByLongKeyLong(&rnd, 3, -10, 5);
    testSortByLongKeyLong(&rnd, 100, -10, 5);
    testSortByLongKeyLong(&rnd, 1000, 0, 126);
    testSortByLongKeyLong(&rnd, 1000, -128, 30);

    testSortByLongKeyS(&rnd, 1, -10, 5);
    testSortByLongKeyS(&rnd, 2, -10, 5);
    testSortByLongKeyS(&rnd, 3, -10, 5);
    testSortByLongKeyS(&rnd, 100, -10, 5);
    testSortByLongKeyS(&rnd, 1000, 0, 1000);
    testSortByLongKeyS(&rnd, 1000, LONG_MIN, LONG_MAX);
}


struct l_t {
    int val;
    bor_list_t list;
};

static int cmpListInt(const bor_list_t *a, const bor_list_t *b, void *_)
{
    struct l_t *l1 = BOR_LIST_ENTRY(a, struct l_t, list);
    struct l_t *l2 = BOR_LIST_ENTRY(b, struct l_t, list);
    return l1->val - l2->val;
}

static int cmpListIntX(const bor_list_t *a, const bor_list_t *b, void *d)
{
    struct l_t *l1 = BOR_LIST_ENTRY(a, struct l_t, list);
    struct l_t *l2 = BOR_LIST_ENTRY(b, struct l_t, list);
    assertEquals((long)d, 12345L);
    return l1->val - l2->val;
}


static void testListSortInt(bor_rand_t *rnd, int size, int from, int to,
                void (*sort)(bor_list_t *list, bor_sort_list_cmp, void *),
                int cmpx)
{
    bor_list_t list, *item;
    struct l_t *arr, *li;
    int i, last;

    borListInit(&list);
    arr = BOR_ALLOC_ARR(struct l_t, size);
    for (i = 0; i < size; ++i){
        arr[i].val = borRand(rnd, from, to);
        borListAppend(&list, &arr[i].list);
    }

    sort(&list, (cmpx ? cmpListIntX : cmpListInt), (void *)12345);
    last = -9999999;
    BOR_LIST_FOR_EACH(&list, item){
        li = BOR_LIST_ENTRY(item, struct l_t, list);
        assertTrue(last <= li->val);
        last = li->val;
    }

    BOR_FREE(arr);
}

TEST(sortListInsert)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testListSortInt(&rnd, 1, -10, 5, borListInsertSort, 0);
    testListSortInt(&rnd, 2, -10, 5, borListInsertSort, 0);
    testListSortInt(&rnd, 3, -10, 5, borListInsertSort, 0);
    testListSortInt(&rnd, 100, -10, 5, borListInsertSort, 1);
    testListSortInt(&rnd, 1000, 0, 100, borListInsertSort, 0);
    testListSortInt(&rnd, 1000, -15, 132, borListInsertSort, 0);
}

TEST(sortList)
{
    bor_rand_t rnd;
    borRandInit(&rnd);
    testListSortInt(&rnd, 1, -10, 5, borListSort, 0);
    testListSortInt(&rnd, 2, -10, 5, borListSort, 0);
    testListSortInt(&rnd, 3, -10, 5, borListSort, 0);
    testListSortInt(&rnd, 100, -10, 5, borListSort, 1);
    testListSortInt(&rnd, 1000, 0, 100, borListSort, 0);
    testListSortInt(&rnd, 1000, -15, 132, borListSort, 0);
}
