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

#include <boruvka/nearest-linear.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

static void bubbleUp(fer_real_t *dists, fer_list_t **nearest, size_t len);

size_t ferNearestLinear(fer_list_t *list, void *p,
                        fer_nearest_linear_dist_t dist_cb,
                        fer_list_t **nearest, size_t num,
                        void *data)
{
    fer_list_t *item;
    fer_real_t *dists, dist;
    size_t len;

    if (num == 0)
        return 0;

    dists = FER_ALLOC_ARR(fer_real_t, num);
    len = 0;

    FER_LIST_FOR_EACH(list, item){
        dist = dist_cb(p, item, data);

        if (len < num){
            dists[len]   = dist;
            nearest[len] = item;
            len++;

            bubbleUp(dists, nearest, len);
        }else if (dist < dists[len - 1]){
            dists[len - 1]   = dist;
            nearest[len - 1] = item;

            bubbleUp(dists, nearest, len);
        }
    }

    FER_FREE(dists);
    return len;
}

static void bubbleUp(fer_real_t *dists, fer_list_t **nearest, size_t len)
{
    size_t i;
    fer_real_t tmpd;
    fer_list_t *tmpn;

    // don't worry, len can never be zero
    for (i = len - 1; i > 0; i--){
        if (dists[i] < dists[i - 1]){
            FER_SWAP(dists[i], dists[i - 1], tmpd);
            FER_SWAP(nearest[i], nearest[i - 1], tmpn);
        }else{
            break;
        }
    }
}
