/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_SORT_H__
#define __BOR_SORT_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Sort Algorithms
 * ================
 */

struct _bor_radix_sort_t {
    bor_real_t key;
    int val;
};
typedef struct _bor_radix_sort_t bor_radix_sort_t;

/**
 * Sorts array {rs} using radix sort by key.
 */
void borRadixSort(bor_radix_sort_t *rs, bor_radix_sort_t *tmp, size_t len);

/**
 * Sorts an array {arr} using radix sort algorithm.
 * Each element of the array is considered as pointer to a struct and an
 * {offset} is offset of bor_real_t key inside that struct.
 * If {descending} is set to true, elements are sorted in descending order.
 */
void borRadixSortPtr(void **arr, void **tmp_arr, size_t arrlen,
                     size_t offset, int descending);

/**
 * Function returning key for counting sort.
 */
typedef int (*bor_sort_key)(const void *, void *arg);

/**
 * Counting sort. Keys has to be between from and to including from and to.
 * The range should be small enough to fit on stack.
 */
void borCountSort(void *base, size_t nmemb, size_t size, int from, int to,
                  bor_sort_key key, void *arg);


/**
 * Compare function for sort functions.
 */
typedef int (*bor_sort_cmp)(const void *, const void *, void *arg);

/**
 * Insertion sort.
 */
void borInsertSort(void *base, size_t nmemb, size_t size,
                   bor_sort_cmp cmp, void *arg);
void borInsertSortInt(int *base, size_t nmemb);

/**
 * BSD heapsort.
 */
void borHeapSort(void *base, size_t nmemb, size_t size,
                 bor_sort_cmp cmp, void *arg);

/**
 * BSD mergesort.
 * Requires size to be at least "sizeof(void *) / 2".
 */
int borMergeSort(void *base, size_t nmemb, size_t size,
                 bor_sort_cmp cmp, void *carg);

/**
 * BSD kqsort.
 * Uses recursion.
 */
void borQSort(void *base, size_t nmemb, size_t size,
              bor_sort_cmp cmp, void *carg);

/**
 * Tim sort.
 * This is just wrapper around https://github.com/patperry/timsort.
 */
int borTimSort(void *base, size_t nmemb, size_t size,
               bor_sort_cmp cmp, void *carg);


/**
 * Default sorting algorithm.
 * Uses timsort if available or qsort if not.
 */
int borSort(void *base, size_t nmemb, size_t size,
            bor_sort_cmp cmp, void *carg);

/**
 * Sorts an array of elements that contain an integer key. {size} is the
 * size of element and {offset} is an offset of the integer key within the
 * element.
 */
int borSortByIntKey(void *base, size_t nmemb, size_t size, size_t offset);
#define BOR_SORT_BY_INT_KEY(base, nmemb, type, member) \
    borSortByIntKey((base), (nmemb), sizeof(type), \
                    bor_offsetof(type, member))

/**
 * Same as borSortByIntKey() but for long keys.
 */
int borSortByLongKey(void *base, size_t nmemb, size_t size, size_t offset);
#define BOR_SORT_BY_LONG_KEY(base, nmemb, type, member) \
    borSortByLongKey((base), (nmemb), sizeof(type), \
                     bor_offsetof(type, member))

/**
 * Compare function for list sort functions.
 */
typedef int (*bor_sort_list_cmp)(const bor_list_t *,
                                 const bor_list_t *, void *arg);

/**
 * List sort based on merge sort (from BSD).
 */
void borListSort(bor_list_t *list, bor_sort_list_cmp cmp, void *carg);

/**
 * Insertion sort for lists.
 * It sorts the list in ascending order
 */
void borListInsertSort(bor_list_t *list, bor_sort_list_cmp cmp, void *data);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_SORT_H__ */
