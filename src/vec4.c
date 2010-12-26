/***
 * mg
 * ---
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of mg.
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
#include <mg/alloc.h>
#include <mg/vec4.h>
#include <mg/dbg.h>

static MG_VEC4(__mg_vec4_origin, MG_ZERO, MG_ZERO, MG_ZERO, MG_ZERO);
const mg_vec4_t *mg_vec4_origin = &__mg_vec4_origin;


mg_vec4_t *mgVec4New(mg_real_t x, mg_real_t y, mg_real_t z, mg_real_t w)
{
    mg_vec4_t *v;

#ifdef MG_SSE
    v = MG_ALLOC_ALIGN(mg_vec4_t, sizeof(mg_vec4_t));
#else /* MG_SSE */
    v = MG_ALLOC(mg_vec4_t);
#endif /* MG_SSE */
    mgVec4Set(v, x, y, z, w);
    return v;
}

void mgVec4Del(mg_vec4_t *v)
{
    free(v);
}

mg_vec4_t *mgVec4ArrNew(size_t num_vecs)
{
    mg_vec4_t *vs;

#ifdef MG_SSE
    vs = MG_ALLOC_ALIGN_ARR(mg_vec4_t, num_vecs, sizeof(mg_vec4_t));
#else /* MG_SSE */
    vs = MG_ALLOC_ARR(mg_vec4_t, num_vecs);
#endif /* MG_SSE */

    return vs;
}

void mgVec4ArrDel(mg_vec4_t *v)
{
    free(v);
}

