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
 * Rand
 * =====
 *
 * Struct representing random number generator.
 */
struct _fer_rand_t {
    unsigned short xi[3]; /*! seed for generation random numbers */
};
typedef struct _fer_rand_t fer_rand_t;

/**
 * Functions
 * ----------
 */

/**
 * Initializes rand struct.
 */
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
}


_fer_inline fer_real_t ferRand(fer_rand_t *r, fer_real_t from, fer_real_t to)
{
    fer_real_t num;

    num = erand48(r->xi);
    num *= to - from;
    num += from;

    return num;
}

#endif /* __FER_RAND_H__ */
