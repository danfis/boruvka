/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __BOR_NEAREST_LINEAR_H__
#define __BOR_NEAREST_LINEAR_H__

#include <boruvka/core.h>
#include <boruvka/list.h>

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
typedef bor_real_t (*bor_nearest_linear_dist_t)(void *item1, bor_list_t *item2,
                                                void *data);

/** ^^^^ */

/**
 * Searches list for *num* items nearest to *p*.
 * Returns number of found items.
 */
size_t borNearestLinear(bor_list_t *list, void *p,
                        bor_nearest_linear_dist_t dist_cb,
                        bor_list_t **nearest, size_t num,
                        void *data);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_NEAREST_LINEAR_H__ */

