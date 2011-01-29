/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_RAND_H__
#define __FER_RAND_H__

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fermat/core.h>

/**
 * Maximal number of returnd random numbers between changing seed.
 */
#ifndef FER_RAND_MAX_RAND_NUMS
# define FER_RAND_MAX_RAND_NUMS 1000
#endif /* FER_RAND_MAX_RAND_NUMS */

/**
 * Rand
 * =====
 *
 * Struct representing random number generator.
 */
struct _fer_rand_t {
    unsigned short xi[3]; /*! seed for generation random numbers */
    unsigned int rand_nums; /*! number of numbers returned from rand()
                                for one seed */
};
typedef struct _fer_rand_t fer_rand_t;

_fer_inline void ferRandInit(fer_rand_t *r);

/**
 * Returns randomly generated number in range <from, to).
 */
_fer_inline fer_real_t ferRand(fer_rand_t *r, fer_real_t from, fer_real_t to);


/**** INLINES ****/
_fer_inline void ferRandInit(fer_rand_t *r)
{
    r->xi[0] = time(NULL);
    r->xi[1] = getpid();
    r->xi[2] = getpgrp();
    r->rand_nums = 0;
}


_fer_inline fer_real_t ferRand(fer_rand_t *r, fer_real_t from, fer_real_t to)
{
    fer_real_t num;

    if (fer_unlikely(r->rand_nums > FER_RAND_MAX_RAND_NUMS)){
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

#endif /* __FER_RAND_H__ */
