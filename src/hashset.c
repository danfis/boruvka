/***
 * Boruvka
 * --------
 * Copyright (c)2019 Daniel Fiser <danfis@danfis.cz>
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

#include <stdio.h>
#include <unistd.h>
#include "boruvka/alloc.h"
#include "boruvka/hfunc.h"
#include "boruvka/hashset.h"
#include "boruvka/iset.h"
#include "boruvka/lset.h"
#include "boruvka/cset.h"

struct bor_hashset_el {
    int id;
    void *el;
    bor_htable_key_t hash;
    bor_list_t htable;
};
typedef struct bor_hashset_el bor_hashset_el_t;

static bor_htable_key_t hash(const bor_list_t *key, void *_)
{
    const bor_hashset_el_t *el;
    el = BOR_LIST_ENTRY(key, bor_hashset_el_t, htable);
    return el->hash;
}

static int eq(const bor_list_t *e1, const bor_list_t *e2, void *_hs)
{
    const bor_hashset_t *hs = _hs;
    const bor_hashset_el_t *s1, *s2;
    s1 = BOR_LIST_ENTRY(e1, bor_hashset_el_t, htable);
    s2 = BOR_LIST_ENTRY(e2, bor_hashset_el_t, htable);
    return hs->eq_fn(s1->el, s2->el, hs->userdata);
}

void borHashSetInit(bor_hashset_t *s,
                    bor_hashset_hash_fn hash_fn,
                    bor_hashset_eq_fn eq_fn,
                    bor_hashset_clone_fn clone_fn,
                    bor_hashset_del_fn del_fn,
                    void *userdata)
{
    bzero(s, sizeof(*s));
    s->hash_fn = hash_fn;
    s->eq_fn = eq_fn;
    s->clone_fn = clone_fn;
    s->del_fn = del_fn;
    s->userdata = userdata;

    // compute best segment size
    size_t segment_size = sysconf(_SC_PAGESIZE);
    segment_size *= 8;
    while (segment_size < 32 * sizeof(bor_hashset_el_t))
        segment_size *= 2;
    s->el = borSegmArrNew(sizeof(bor_hashset_el_t), segment_size);
    s->htable = borHTableNew(hash, eq, s);
}

static bor_htable_key_t isetHash(const void *el, void *_)
{
    const bor_iset_t *s = el;
    if (borISetSize(s) == 0)
        return 0;
    return borCityHash_64(s->s, sizeof(*s->s) * s->size);
}

static int isetEq(const void *el1, const void *el2, void *_)
{
    const bor_iset_t *s1 = el1;
    const bor_iset_t *s2 = el2;
    return borISetEq(s1, s2);
}

static void *isetClone(const void *el, void *_)
{
    bor_iset_t *s = BOR_ALLOC(bor_iset_t);
    borISetInit(s);
    borISetUnion(s, (const bor_iset_t *)el);
    return s;
}

static void isetDel(void *el, void *_)
{
    bor_iset_t *s = el;
    borISetFree(s);
    BOR_FREE(s);
}

void borHashSetInitISet(bor_hashset_t *s)
{
    borHashSetInit(s, isetHash, isetEq, isetClone, isetDel, NULL);
}

static bor_htable_key_t lsetHash(const void *el, void *_)
{
    const bor_lset_t *s = el;
    if (borLSetSize(s) == 0)
        return 0;
    return borCityHash_64(s->s, sizeof(*s->s) * s->size);
}

static int lsetEq(const void *el1, const void *el2, void *_)
{
    const bor_lset_t *s1 = el1;
    const bor_lset_t *s2 = el2;
    return borLSetEq(s1, s2);
}

static void *lsetClone(const void *el, void *_)
{
    bor_lset_t *s = BOR_ALLOC(bor_lset_t);
    borLSetInit(s);
    borLSetUnion(s, (const bor_lset_t *)el);
    return s;
}

static void lsetDel(void *el, void *_)
{
    bor_lset_t *s = el;
    borLSetFree(s);
    BOR_FREE(s);
}

void borHashSetInitLSet(bor_hashset_t *s)
{
    borHashSetInit(s, lsetHash, lsetEq, lsetClone, lsetDel, NULL);
}

static bor_htable_key_t csetHash(const void *el, void *_)
{
    const bor_cset_t *s = el;
    if (borCSetSize(s) == 0)
        return 0;
    return borCityHash_64(s->s, sizeof(*s->s) * s->size);
}

static int csetEq(const void *el1, const void *el2, void *_)
{
    const bor_cset_t *s1 = el1;
    const bor_cset_t *s2 = el2;
    return borCSetEq(s1, s2);
}

static void *csetClone(const void *el, void *_)
{
    bor_cset_t *s = BOR_ALLOC(bor_cset_t);
    borCSetInit(s);
    borCSetUnion(s, (const bor_cset_t *)el);
    return s;
}

static void csetDel(void *el, void *_)
{
    bor_cset_t *s = el;
    borCSetFree(s);
    BOR_FREE(s);
}

void borHashSetInitCSet(bor_hashset_t *s)
{
    borHashSetInit(s, csetHash, csetEq, csetClone, csetDel, NULL);
}



void borHashSetFree(bor_hashset_t *s)
{
    if (s->htable != NULL)
        borHTableDel(s->htable);

    for (int i = 0; i < s->size; ++i){
        bor_hashset_el_t *el = borSegmArrGet(s->el, i);
        s->del_fn(el->el, s->userdata);
    }
    if (s->el != NULL)
        borSegmArrDel(s->el);
}

int borHashSetAdd(bor_hashset_t *s, const void *ins)
{
    bor_hashset_el_t *el;
    bor_list_t *find;

    el = borSegmArrGet(s->el, s->size);
    el->id = s->size;
    el->el = (void *)ins;
    el->hash = s->hash_fn(ins, s->userdata);
    borListInit(&el->htable);

    if ((find = borHTableInsertUnique(s->htable, &el->htable)) == NULL){
        el->el = s->clone_fn(ins, s->userdata);
        ++s->size;
        return el->id;
    }else{
        el = BOR_LIST_ENTRY(find, bor_hashset_el_t, htable);
        return el->id;
    }
}

int borHashSetFind(const bor_hashset_t *s, const void *find_el)
{
    bor_hashset_el_t el;
    bor_list_t *find;

    el.id = s->size;
    el.el = (void *)find_el;
    el.hash = s->hash_fn(find_el, s->userdata);
    borListInit(&el.htable);

    if ((find = borHTableFind(s->htable, &el.htable)) != NULL){
        const bor_hashset_el_t *f;
        f = BOR_LIST_ENTRY(find, bor_hashset_el_t, htable);
        return f->id;
    }else{
        return -1;
    }
}

const void *borHashSetGet(const bor_hashset_t *s, int id)
{
    if (id < 0 || id >= s->size)
        return NULL;
    const bor_hashset_el_t *el = borSegmArrConstGet(s->el, id);
    return el->el;
}
