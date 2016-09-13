/***
 * Boruvka
 * --------
 * Copyright (c)2016 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/alloc.h>
#include <boruvka/poly2.h>

static void poly2Init(bor_poly2_t *p, int size)
{
    p->size = size;
    p->px = BOR_ALLOC_ARR(bor_real_t, size);
    p->py = BOR_ALLOC_ARR(bor_real_t, size);
    p->constant = BOR_ALLOC_ARR(bor_real_t, p->size);
    p->multiple = BOR_ALLOC_ARR(bor_real_t, p->size);
}

static void poly2InitConstantMultiple(bor_poly2_t *p)
{
    int i, j;
    bor_real_t vy, val;

    for (i = 0, j = p->size - 1; i < p->size; j = i++){
        if (p->py[j] == p->py[i]){
            p->constant[i] = p->px[i];
            p->multiple[i] = 0;

        }else{
            vy = p->py[j] - p->py[i];
            val  = p->px[i];
            val -= (p->py[i] * p->px[j]) / vy;
            val += (p->py[i] * p->px[i]) / vy;
            p->constant[i] = val;
            p->multiple[i] = (p->px[j] - p->px[i]) / vy;
        }
    }
}

bor_poly2_t *borPoly2New(const bor_vec2_t *corners, int size)
{
    bor_poly2_t *p;

    p = BOR_ALLOC(bor_poly2_t);
    borPoly2Init(p, corners, size);
    return p;
}

bor_poly2_t *borPoly2New2(const bor_vec2_t *corners, int *idx, int size)
{
    bor_poly2_t *p;

    p = BOR_ALLOC(bor_poly2_t);
    borPoly2Init2(p, corners, idx, size);
    return p;
}

void borPoly2Del(bor_poly2_t *p)
{
    borPoly2Free(p);
    BOR_FREE(p);
}

void borPoly2Init(bor_poly2_t *p, const bor_vec2_t *corners, int size)
{
    int i;

    poly2Init(p, size);
    for (i = 0; i < size; ++i){
        p->px[i] = borVec2X(corners + i);
        p->py[i] = borVec2Y(corners + i);
    }
    poly2InitConstantMultiple(p);
}

void borPoly2Init2(bor_poly2_t *p, const bor_vec2_t *corners,
                   int *idx, int size)
{
    int i;

    poly2Init(p, size);
    for (i = 0; i < size; ++i){
        p->px[i] = borVec2X(corners + idx[i]);
        p->py[i] = borVec2Y(corners + idx[i]);
    }
    poly2InitConstantMultiple(p);
}

void borPoly2Free(bor_poly2_t *p)
{
    if (p->px)
        BOR_FREE(p->px);
    if (p->py)
        BOR_FREE(p->py);
    if (p->constant)
        BOR_FREE(p->constant);
    if (p->multiple)
        BOR_FREE(p->multiple);
}

int borPoly2PointIn(const bor_poly2_t *p, const bor_vec2_t *v)
{
    int i, j, odd = 0;
    bor_real_t x, y;

    x = borVec2X(v);
    y = borVec2Y(v);

    for (i = 0, j = p->size - 1; i < p->size; j = i++){
        if ((p->py[i] < y && p->py[j] >= y)
                || (p->py[j] < y && p->py[i] >= y)){
            odd ^= (y * p->multiple[i] + p->constant[i] < x);
        }
    }
    return odd;
}
