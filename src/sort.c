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

#include <boruvka/sort.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>


/**** RADIX SORT ****/
#define RADIX_SORT_MASK 0xffu

_bor_inline uint32_t radixSortVal(const bor_radix_sort_t *m,
                                  uint32_t shift)
{
    bor_uint_t val;

    val = borRealAsUInt(m->key);
    val = (val >> shift) & RADIX_SORT_MASK;

    return val;
}

_bor_inline void radixSortZeroizeCounter(uint32_t *counter)
{
    int i;
    for (i = 0; i < 256; i++)
        counter[i] = 0;
}

static void radixSortCount(const bor_radix_sort_t *m,
                           uint32_t len,
                           uint32_t *counter, uint32_t shift)
{
    uint32_t i;
    bor_uint_t val;

    radixSortZeroizeCounter(counter);
    for (i = 0; i < len; i++){
        val = radixSortVal(&m[i], shift);
        ++counter[val];
    }
}

static void radixSortCountFinal(const bor_radix_sort_t *m,
                                uint32_t len,
                                uint32_t *counter,
                                uint32_t *negative,
                                uint32_t shift)
{
    uint32_t i;
    bor_uint_t val;

    radixSortZeroizeCounter(counter);
    *negative = 0;
    for (i = 0; i < len; i++){
        val = radixSortVal(&m[i], shift);
        ++counter[val];

        if (m[i].key < BOR_ZERO)
            ++(*negative);
    }
}

static void radixSortFixCounter(uint32_t *counter)
{
    uint32_t i, val, t;

    val = 0;
    for (i = 0; i < 256; i++){
        t = counter[i];
        counter[i] = val;
        val += t;
    }
}

static void radixSortSort(bor_radix_sort_t *src,
                          bor_radix_sort_t *dst,
                          uint32_t len,
                          uint32_t *counter,
                          uint32_t shift)
{
    uint32_t i;
    bor_uint_t val;

    for (i = 0; i < len; i++){
        val = radixSortVal(&src[i], shift);
        dst[counter[val]] = src[i];
        ++counter[val];
    }
}

static void radixSortSortFinal(bor_radix_sort_t *src,
                               uint32_t srclen,
                               bor_radix_sort_t *dst,
                               uint32_t dstlen,
                               uint32_t *counter,
                               uint32_t negative,
                               uint32_t shift)
{
    uint32_t i;
    bor_uint_t val, pos;

    for (i = 0; i < srclen; i++){
        val = radixSortVal(&src[i], shift);
        pos = counter[val];
        if (pos >= dstlen - negative){
            pos = dstlen - pos - 1;
        }else{
            pos = negative + pos;
        }

        dst[pos] = src[i];

        ++counter[val];
    }
}

void borRadixSort(bor_radix_sort_t *rs, bor_radix_sort_t *rs_tmp, size_t rslen)
{
    bor_radix_sort_t *src, *dst, *tmp;
    uint32_t shift, i, len;
    uint32_t counter[RADIX_SORT_MASK], negative;


    len = (bor_uint_t)sizeof(bor_real_t) - 1;
    shift = 0;
    src = rs;
    dst = rs_tmp;
    for (i = 0; i < len; i++){
        radixSortCount(src, rslen, counter, shift);
        radixSortFixCounter(counter);
        radixSortSort(src, dst, rslen, counter, shift);

        shift += 8;
        BOR_SWAP(src, dst, tmp);
    }

    radixSortCountFinal(src, rslen, counter, &negative, shift);
    radixSortFixCounter(counter);
    radixSortSortFinal(src, rslen, dst, rslen, counter, negative, shift);
}

/**** RADIX SORT END ****/


/**** RADIX SORT PTR ****/
_bor_inline bor_real_t radixSortPtrKey(const void *el, size_t offset)
{
    return *(bor_real_t *)((char *)el + offset);
}

_bor_inline uint32_t radixSortPtrVal(const void *el, size_t offset,
                                     uint32_t shift)
{
    bor_uint_t val;

    val = borRealAsUInt(radixSortPtrKey(el, offset));
    val = (val >> shift) & RADIX_SORT_MASK;

    return val;
}

_bor_inline void radixSortPtrZeroizeCounter(uint32_t *counter)
{
    int i;
    for (i = 0; i < 256; i++)
        counter[i] = 0;
}

static void radixSortPtrCount(void * const*m,
                              uint32_t len, size_t offset,
                              uint32_t *counter, uint32_t shift)
{
    uint32_t i;
    bor_uint_t val;

    radixSortPtrZeroizeCounter(counter);
    for (i = 0; i < len; i++){
        val = radixSortPtrVal(m[i], offset, shift);
        ++counter[val];
    }
}

static void radixSortPtrCountFinal(void * const*m,
                                   uint32_t len,
                                   size_t offset,
                                   uint32_t *counter,
                                   uint32_t *negative,
                                   uint32_t shift)
{
    uint32_t i;
    bor_uint_t val;

    radixSortPtrZeroizeCounter(counter);
    *negative = 0;
    for (i = 0; i < len; i++){
        val = radixSortPtrVal(m[i], offset, shift);
        ++counter[val];

        if (radixSortPtrKey(m[i], offset) < BOR_ZERO)
            ++(*negative);
    }
}

static void radixSortPtrFixCounter(uint32_t *counter)
{
    uint32_t i, val, t;

    val = 0;
    for (i = 0; i < 256; i++){
        t = counter[i];
        counter[i] = val;
        val += t;
    }
}

static void radixSortPtrSort(void **src, void **dst,
                             uint32_t len,
                             size_t offset,
                             uint32_t *counter,
                             uint32_t shift)
{
    uint32_t i;
    bor_uint_t val;

    for (i = 0; i < len; i++){
        val = radixSortPtrVal(src[i], offset, shift);
        dst[counter[val]] = src[i];
        ++counter[val];
    }
}

static void radixSortPtrSortFinal(void **src, uint32_t srclen,
                                  void **dst, uint32_t dstlen,
                                  size_t offset,
                                  uint32_t *counter,
                                  uint32_t negative,
                                  uint32_t shift,
                                  int reverse)
{
    uint32_t i;
    bor_uint_t val, pos;

    for (i = 0; i < srclen; i++){
        val = radixSortPtrVal(src[i], offset, shift);
        pos = counter[val];
        if (pos >= dstlen - negative){
            pos = dstlen - pos - 1;
        }else{
            pos = negative + pos;
        }

        if (reverse){
            pos = dstlen - 1 - pos;
        }

        dst[pos] = src[i];

        ++counter[val];
    }
}

void borRadixSortPtr(void **arr, void **tmp_arr, size_t arrlen,
                     size_t offset, int desc)
{
    void **src, **dst, **tmp;
    uint32_t shift, i, len;
    uint32_t counter[RADIX_SORT_MASK], negative;


    len = (bor_uint_t)sizeof(bor_real_t) - 1;
    shift = 0;
    src = arr;
    dst = tmp_arr;
    for (i = 0; i < len; i++){
        radixSortPtrCount(src, arrlen, offset, counter, shift);
        radixSortPtrCount(src, arrlen, offset, counter, shift);
        radixSortPtrFixCounter(counter);
        radixSortPtrSort(src, dst, arrlen, offset, counter, shift);

        shift += 8;
        BOR_SWAP(src, dst, tmp);
    }

    radixSortPtrCountFinal(src, arrlen, offset, counter, &negative, shift);
    radixSortPtrFixCounter(counter);
    radixSortPtrSortFinal(src, arrlen, dst, arrlen, offset, counter, negative, shift, desc);
}
/**** RADIX SORT PTR END ****/

/**** INSERT SORT LIST ****/
void borListInsertSort(bor_list_t *list, bor_sort_list_cmp cmp, void *data)
{
    bor_list_t out;
    bor_list_t *cur, *item;

    // empty list - no need to sort
    if (borListEmpty(list))
        return;

    // list with one item - no need to sort
    if (borListNext(list) == borListPrev(list))
        return;

    borListInit(&out);
    while (!borListEmpty(list)){
        // pick up next item from list
        cur = borListNext(list);
        borListDel(cur);

        // find the place where to put it
        item = borListPrev(&out);
        while (item != &out && cmp(cur, item, data) < 0){
            item = borListPrev(item);
        }

        // put it after the item
        borListPrepend(item, cur);
    }

    // and finally store sorted
    *list = out;
    list->next->prev = list;
    list->prev->next = list;
}

/**** INSERT SORT LIST END ****/

void borCountSort(void *base, size_t nmemb, size_t size, int from, int to,
                  bor_sort_key get_key, void *arg)
{
    int range = to - from + 1;
    int cnt[range];
    unsigned char *cur, *end, *tmp;
    int i, key;

    bzero(cnt, sizeof(int) * range);
    for (cur = base, end = cur + (nmemb * size); cur != end; cur += size){
        key = get_key(cur, arg) - from;
        ++cnt[key];
    }

    for (i = 1; i < range; ++i)
        cnt[i] += cnt[i - 1];

    tmp = BOR_ALLOC_ARR(unsigned char, nmemb * size);
    for (cur = base, end = cur + (nmemb * size); cur != end; cur += size){
        key = get_key(cur, arg) - from;
        --cnt[key];
        memcpy(tmp + (cnt[key] * size), cur, size);
    }
    memcpy(base, tmp, nmemb * size);
    BOR_FREE(tmp);
}

_bor_inline void sort2(void *e1, void *e2, size_t size,
                       bor_sort_cmp cmp, void *arg)
{
    if (cmp(e2, e1, arg) < 0){
        unsigned char tmp[size];
        memcpy(tmp, e1, size);
        memcpy(e1, e2, size);
        memcpy(e2, tmp, size);
    }
}

_bor_inline void sort3(void *e1, void *e2, void *e3, size_t size,
                       bor_sort_cmp cmp, void *arg)
{
    sort2(e1, e2, size, cmp, arg);
    sort2(e2, e3, size, cmp, arg);
    sort2(e1, e2, size, cmp, arg);
}

void borInsertSort(void *base, size_t nmemb, size_t size,
                   bor_sort_cmp cmp, void *arg)
{
    unsigned char *begin = base, *cur, *end;
    unsigned char *prev, *ins;
    unsigned char tmp[size];

    if (nmemb <= 1)
        return;
    if (nmemb == 2){
        sort2(begin, begin + size, size, cmp, arg);
        return;
    }
    if (nmemb == 3){
        sort3(begin, begin + size, begin + size + size, size, cmp, arg);
        return;
    }

    end = begin + (nmemb * size);
    for (cur = begin + size; cur != end; cur += size){
        prev = cur - size;

        // Test whether current value is misplaced
        if (cmp(cur, prev, arg) < 0){
            // Remember the current value
            memcpy(tmp, cur, size);

            // Find its position backwards
            for (ins = prev, prev -= size;
                 prev >= begin && cmp(tmp, prev, arg) < 0;
                 prev -= size, ins -= size);

            // Move all values from ins to the right
            memmove(ins + size, ins, cur - ins);

            // Finally insert remembered value
            memcpy(ins, tmp, size);
        }
    }
}

void borInsertSortInt(int *arr, size_t nmemb)
{
    int tmp, i, j;

    if (nmemb <= 1)
        return;

    for (i = 1; i < nmemb; ++i){
        if (arr[i] < arr[i - 1]){
            tmp = arr[i];

            for (j = i - 1; j >= 0 && arr[j] > tmp; --j)
                arr[j + 1] = arr[j];
            arr[j + 1] = tmp;
        }
    }
}

#ifndef BOR_TIMSORT
int borTimSort(void *base, size_t nmemb, size_t size,
               bor_sort_cmp cmp, void *carg)
{
    fprintf(stderr, "Fatal Error: TimSort is not compiled in!\n");
    exit(-1);
}
#endif /* BOR_TIMSORT */

int borSort(void *base, size_t nmemb, size_t size,
            bor_sort_cmp cmp, void *carg)
{
    if (nmemb <= 1)
        return 0;
    if (nmemb == 2){
        char *begin = base;
        sort2(begin, begin + size, size, cmp, carg);
        return 0;
    }
    if (nmemb == 3){
        char *begin = base;
        sort3(begin, begin + size, begin + size + size, size, cmp, carg);
        return 0;
    }

#ifndef BOR_TIMSORT
    return borTimSort(base, nmemb, size, cmp, carg);
#else /* BOR_TIMSORT */
    borQSort(base, nmemb, size, cmp, carg);
    return 0;
#endif /* BOR_TIMSORT */
}

static int cmpIntKey(const void *a, const void *b, void *d)
{
    long offset = (long)d;
    int i1 = *(int *)(((char *)a) + offset);
    int i2 = *(int *)(((char *)b) + offset);

    if (i1 < i2)
        return -1;
    if (i1 > i2)
        return 1;
    return 0;
}

static int cmpLongKey(const void *a, const void *b, void *d)
{
    long offset = (long)d;
    long i1 = *(long *)(((char *)a) + offset);
    long i2 = *(long *)(((char *)b) + offset);

    if (i1 < i2)
        return -1;
    if (i1 > i2)
        return 1;
    return 0;
}

static int cmpRealKey(const void *a, const void *b, void *d)
{
    long offset = (long)d;
    bor_real_t i1 = *(bor_real_t *)(((char *)a) + offset);
    bor_real_t i2 = *(bor_real_t *)(((char *)b) + offset);

    if (borEq(i1, i2))
        return 0;
    if (i1 < i2)
        return -1;
    return 1;
}

int borSortByIntKey(void *base, size_t nmemb, size_t size, size_t offset)
{
    return borSort(base, nmemb, size, cmpIntKey, (void *)offset);
}

int borSortByLongKey(void *base, size_t nmemb, size_t size, size_t offset)
{
    return borSort(base, nmemb, size, cmpLongKey, (void *)offset);
}

int borSortByRealKey(void *base, size_t nmemb, size_t size, size_t offset)
{
    return borSort(base, nmemb, size, cmpRealKey, (void *)offset);
}
