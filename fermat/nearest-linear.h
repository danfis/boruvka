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

#ifndef __FER_NEAREST_LINEAR_H__
#define __FER_NEAREST_LINEAR_H__

#include <fermat/core.h>
#include <fermat/list.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Nearest Linear
 * ===============
 */


/** vvvv */

/**
 * Callback that takes two items and returns distance between them.
 */
typedef fer_real_t (*fer_nearest_linear_dist_t)(void *item1, fer_list_t *item2);

/** ^^^^ */

/**
 * Searches list for *num* items nearest to *p*.
 * Returns number of found items.
 */
size_t ferNearestLinear(fer_list_t *list, void *p,
                        fer_nearest_linear_dist_t dist_cb,
                        fer_list_t **nearest, size_t num);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_NEAREST_LINEAR_H__ */

