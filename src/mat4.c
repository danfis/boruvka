/***
 * Boruvka
 * --------
 * Copyright (c)2010-2012 Daniel Fiser <danfis@danfis.cz>
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

#include "boruvka/mat4.h"
#include "boruvka/mat3.h"
#include "boruvka/alloc.h"
#include "boruvka/dbg.h"


/** Returns subdeterminant */
static bor_real_t borMat4Subdet(const bor_mat4_t *m, size_t i, size_t j);


static BOR_MAT4(__bor_mat4_identity, BOR_ONE, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                                     BOR_ZERO, BOR_ONE, BOR_ZERO, BOR_ZERO,
                                     BOR_ZERO, BOR_ZERO, BOR_ONE, BOR_ZERO,
                                     BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ONE);
const bor_mat4_t *bor_mat4_identity = &__bor_mat4_identity;

static BOR_MAT4(__bor_mat4_zero, BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                                 BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                                 BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                                 BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO);
const bor_mat4_t *bor_mat4_zero = &__bor_mat4_zero;

bor_mat4_t *borMat4New(void)
{
    bor_mat4_t *m;

#ifdef BOR_SSE
    m = BOR_ALLOC_ALIGN(bor_mat4_t, 16);
#else /* BOR_SSE */
    m = BOR_ALLOC(bor_mat4_t);
#endif /* BOR_SSE */

    return m;
}

void borMat4Del(bor_mat4_t *m)
{
    BOR_FREE(m);
}

int borMat4Inv2(bor_mat4_t *m, const bor_mat4_t *a)
{
    bor_real_t det, invdet;
    int sign;
    size_t i, j;

    det = borMat4Det(a);
    if (borIsZero(det))
        return -1;

    invdet = borRecp(det);

    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            det  = borMat4Subdet(a, i, j);
            sign = 1 - ((i + j) % 2) * 2;
            borMat4Set1(m, j, i, sign * det * invdet);
        }
    }

    return 0;
}


static bor_real_t borMat4Subdet(const bor_mat4_t *m, size_t i, size_t j)
{
    bor_mat3_t m3;
    size_t k, l, r, c;

    // copy the matrix into m3 leaving out i'th row and j'th column
    r = 0;
    for(k = 0; k < 4; k++){
        if (k == i)
            continue;

        c = 0;
        for (l = 0; l < 4; l++){
            if (l == j)
                continue;
            borMat3Set1(&m3, r, c, borMat4Get(m, k, l));
            c++;
        }

        r++;
    }

    return borMat3Det(&m3);
}
