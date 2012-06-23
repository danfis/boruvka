/***
 * Boruvka
 * --------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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
#include <boruvka/alloc.h>
#include <boruvka/vec4.h>
#include <boruvka/dbg.h>

static BOR_VEC4(__fer_vec4_origin, BOR_ZERO, BOR_ZERO, BOR_ZERO, BOR_ZERO);
const bor_vec4_t *fer_vec4_origin = &__fer_vec4_origin;


bor_vec4_t *ferVec4New(bor_real_t x, bor_real_t y, bor_real_t z, bor_real_t w)
{
    bor_vec4_t *v;

#ifdef BOR_SSE
    v = BOR_ALLOC_ALIGN(bor_vec4_t, 16);
#else /* BOR_SSE */
    v = BOR_ALLOC(bor_vec4_t);
#endif /* BOR_SSE */
    ferVec4Set(v, x, y, z, w);
    return v;
}

void ferVec4Del(bor_vec4_t *v)
{
    BOR_FREE(v);
}

bor_vec4_t *ferVec4ArrNew(size_t num_vecs)
{
    bor_vec4_t *vs;

#ifdef BOR_SSE
    vs = BOR_ALLOC_ALIGN_ARR(bor_vec4_t, num_vecs, sizeof(bor_vec4_t));
#else /* BOR_SSE */
    vs = BOR_ALLOC_ARR(bor_vec4_t, num_vecs);
#endif /* BOR_SSE */

    return vs;
}

void ferVec4ArrDel(bor_vec4_t *v)
{
    BOR_FREE(v);
}

