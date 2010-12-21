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

#ifndef __MG_RAND_H__
#define __MG_RAND_H__

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mg/core.h>

/**
 * Maximal number of returnd random numbers between changing seed.
 */
#ifndef MG_RAND_MAX_RAND_NUMS
# define MG_RAND_MAX_RAND_NUMS 1000
#endif /* MG_RAND_MAX_RAND_NUMS */

/**
 * Struct representing random number generator.
 */
struct _mg_rand_t {
    unsigned short xi[3]; /*! seed for generation random numbers */
    unsigned int rand_nums; /*! number of numbers returned from rand()
                                for one seed */
};
typedef struct _mg_rand_t mg_rand_t;

_mg_inline void mgRandInit(mg_rand_t *r);

/**
 * Returns randomly generated number in range <from, to).
 */
_mg_inline mg_real_t mgRand(mg_rand_t *r, mg_real_t from, mg_real_t to);


/**** INLINES ****/
_mg_inline void mgRandInit(mg_rand_t *r)
{
    r->xi[0] = time(NULL);
    r->xi[1] = getpid();
    r->xi[2] = getpgrp();
    r->rand_nums = 0;
}


_mg_inline mg_real_t mgRand(mg_rand_t *r, mg_real_t from, mg_real_t to)
{
    mg_real_t num;

    if (mg_unlikely(r->rand_nums > MG_RAND_MAX_RAND_NUMS)){
        num = time(NULL);
        r->xi[0] = num * erand48(r->xi);
        r->xi[1] = num * erand48(r->xi);
        r->xi[2] = num * erand48(r->xi);
        r->rand_nums = 0;
    }
    r->rand_nums++;

    num = erand48(r->xi);
    num *= to - from;
    num += from;
    return num;
}

#endif /* __MG_RAND_H__ */
