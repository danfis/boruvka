/** Adopted from DragonFly BSD's linux_list_sort:
 * Copyright (c) 2013 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Taylor R. Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "boruvka/sort.h"

#define ACCUM_SIZE 64

static bor_list_t *
list_sort_merge(bor_list_t *, bor_list_t *,
                bor_sort_list_cmp cmp, void *carg);
static void
list_sort_merge_into(bor_list_t *,
             bor_list_t *, bor_list_t *,
             bor_sort_list_cmp cmp, void *carg);

void borListSort(bor_list_t *list, bor_sort_list_cmp cmp, void *carg)
{
    /*
     * Array of sorted sublists, counting in binary: accum[i]
     * is sorted, and either is NULL or has length 2^i.
     */
    bor_list_t *accum[ACCUM_SIZE];

    /* Indices into accum.  */
    unsigned int logn, max_logn = 0;

    /* The sorted list we're currently working on.  */
    bor_list_t *sorted;

    /* The remainder of the unsorted list.  */
    bor_list_t *next;

    /* Make sure we can't possibly have more than 2^64-element lists.  */
    //CTASSERT((CHAR_BIT * sizeof(bor_list_t *)) <= 64);

    for (logn = 0; logn < ACCUM_SIZE; logn++)
        accum[logn] = NULL;

    BOR_LIST_FOR_EACH_SAFE(list, sorted, next) {
        /* Pick off a single element, always sorted.  */
        sorted->next = NULL;

        /* Add one and propagate the carry.  */
        for (logn = 0; accum[logn] != NULL; logn++) {
            /*
             * Merge, preferring previously accumulated
             * elements to make the sort stable.
             */
            sorted = list_sort_merge(accum[logn], sorted, cmp, carg);
            accum[logn] = NULL;
            //KKASSERT((logn + 1) < ARRAY_SIZE(accum));
        }

        /* Remember the highest index seen so far.  */
        if (logn > max_logn)
            max_logn = logn;

        /*
         * logn = log_2(length(sorted)), and accum[logn]
         * is now empty, so save the sorted sublist there.
         */
        accum[logn] = sorted;
    }

    /*
     * Merge ~half of everything we have accumulated.
     */
    sorted = NULL;
    for (logn = 0; logn < max_logn; logn++)
        sorted = list_sort_merge(accum[logn], sorted, cmp, carg);

    /*
     * Merge the last ~halves back into the list, and fix the back
     * pointers.
     */
    list_sort_merge_into(list, accum[max_logn], sorted, cmp, carg);
}

/*
 * Merge the NULL-terminated lists starting at nodes `a' and `b',
 * breaking ties by choosing nodes in `a' first, and returning
 * whichever node has the least element.
 */
static bor_list_t *
list_sort_merge(bor_list_t *a, bor_list_t *b,
                bor_sort_list_cmp cmp, void *carg)
{
    bor_list_t head, *tail = &head;

    /*
     * Merge while elements in both remain.
     */
    while ((a != NULL) && (b != NULL)) {
        bor_list_t **const first = ((*cmp)(a, b, carg) <= 0?
            &a : &b);

        tail = tail->next = *first;
        *first = (*first)->next;
    }

    /*
     * Attach whatever remains.
     */
    tail->next = (a != NULL? a : b);
    return head.next;
}

/*
 * Merge the NULL-terminated lists starting at nodes `a' and `b' into
 * the (uninitialized) list head `list', breaking ties by choosing
 * nodes in `a' first, and setting the `prev' pointers as we go.
 */
static void
list_sort_merge_into(bor_list_t *list,
             bor_list_t *a, bor_list_t *b,
             bor_sort_list_cmp cmp, void *carg)
{
    bor_list_t *prev = list;

    /*
     * Merge while elements in both remain.
     */
    while ((a != NULL) && (b != NULL)) {
        bor_list_t **const first = (
            (*cmp)(a, b, carg) <= 0 ? &a : &b);

        (*first)->prev = prev;
        prev = prev->next = *first;
        *first = (*first)->next;
    }

    /*
     * Attach whichever of a and b remains, and fix up the prev
     * pointers all the way down the rest of the list.
     */
    bor_list_t *tail = (a == NULL? b : a);
    while (tail != NULL) {
        prev->next = tail;
        tail->prev = prev;
        prev = prev->next;
        tail = tail->next;
    }

    /*
     * Finally, finish the cycle.
     */
    prev->next = list;
    list->prev = prev;
}
