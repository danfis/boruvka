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
static bor_real_t ferMat4Subdet(const bor_mat4_t *m, size_t i, size_t j);


static FER_MAT4(__fer_mat4_identity, FER_ONE, FER_ZERO, FER_ZERO, FER_ZERO,
                                     FER_ZERO, FER_ONE, FER_ZERO, FER_ZERO,
                                     FER_ZERO, FER_ZERO, FER_ONE, FER_ZERO,
                                     FER_ZERO, FER_ZERO, FER_ZERO, FER_ONE);
const bor_mat4_t *fer_mat4_identity = &__fer_mat4_identity;

static FER_MAT4(__fer_mat4_zero, FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO);
const bor_mat4_t *fer_mat4_zero = &__fer_mat4_zero;

bor_mat4_t *ferMat4New(void)
{
    bor_mat4_t *m;

#ifdef FER_SSE
    m = FER_ALLOC_ALIGN(bor_mat4_t, 16);
#else /* FER_SSE */
    m = FER_ALLOC(bor_mat4_t);
#endif /* FER_SSE */

    return m;
}

void ferMat4Del(bor_mat4_t *m)
{
    FER_FREE(m);
}

int ferMat4Inv2(bor_mat4_t *m, const bor_mat4_t *a)
{
    bor_real_t det, invdet;
    int sign;
    size_t i, j;

    det = ferMat4Det(a);
    if (ferIsZero(det))
        return -1;

    invdet = ferRecp(det);

    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            det  = ferMat4Subdet(a, i, j);
            sign = 1 - ((i + j) % 2) * 2;
            ferMat4Set1(m, j, i, sign * det * invdet);
        }
    }

    return 0;
}


static bor_real_t ferMat4Subdet(const bor_mat4_t *m, size_t i, size_t j)
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
            ferMat3Set1(&m3, r, c, ferMat4Get(m, k, l));
            c++;
        }

        r++;
    }

    return ferMat3Det(&m3);
}
