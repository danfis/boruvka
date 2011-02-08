/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fermat/pc4.h>
#include <fermat/rand.h>
#include <fermat/parse.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

/**
 * Updates given AABB using given point.
 */
_fer_inline void ferPC4AABBUpdate(fer_real_t *aabb,
                                  fer_real_t x, fer_real_t y,
                                  fer_real_t z, fer_real_t w);

fer_pc4_t *ferPC4New(void)
{
    fer_pc4_t *pc;

    pc = FER_ALLOC(fer_pc4_t);
    ferListInit(&pc->head);
    pc->len = 0;
    pc->aabb[0] = pc->aabb[2] = pc->aabb[4] = pc->aabb[6] = FER_REAL_MAX;
    pc->aabb[1] = pc->aabb[3] = pc->aabb[5] = pc->aabb[7] = FER_REAL_MIN;

    pc->min_chunk_size = FER_PC4_MIN_CHUNK_SIZE;
    

    return pc;
}

#define PC_FUNC(n) ferPC4 ## n
#define PC_T fer_pc4_t
#define VEC_FUNC(n) ferVec4 ## n
#define VEC_T fer_vec4_t
#define PC_UPDATE(pc, v) \
    ferPC4AABBUpdate((pc)->aabb, ferVec4X(v), ferVec4Y(v), ferVec4Z(v), ferVec4W(v))
#define PC_PARSE_VEC ferParseVec4
#include "pc-common.c"

_fer_inline void ferPC4AABBUpdate(fer_real_t *aabb,
                                  fer_real_t x, fer_real_t y,
                                  fer_real_t z, fer_real_t w)
{
    if (aabb[0] > x)
        aabb[0] = x;
    if (aabb[1] < x)
        aabb[1] = x;
    if (aabb[2] > y)
        aabb[2] = y;
    if (aabb[3] < y)
        aabb[3] = y;
    if (aabb[4] > z)
        aabb[4] = z;
    if (aabb[5] < z)
        aabb[5] = z;
    if (aabb[6] > w)
        aabb[6] = w;
    if (aabb[7] < w)
        aabb[7] = w;
}

