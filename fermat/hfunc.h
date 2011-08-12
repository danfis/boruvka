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

#ifndef __FER_HFUNC_H__
#define __FER_HFUNC_H__

#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * HFunc - Hash Functions
 * =======================
 */


/**
 * Robert Jenkins hash function.
 *
 * Taken from http://burtleburtle.net/bob/c/lookup3.c.
 */
uint32_t ferHashJenkins(const uint32_t *k, size_t length, uint32_t initval);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FER_HFUNC_H__ */

