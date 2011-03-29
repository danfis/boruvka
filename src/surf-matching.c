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

#include <fermat/surf-matching.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

#include "surf-matching-cl.c"

static const int dim = 64; /*!< Dimension of surf vectors */


fer_surf_match_t *ferSurfMatchNew(size_t maxlen1, size_t maxlen2)
{
    fer_surf_match_t *sm;

    sm = FER_ALLOC(fer_surf_match_t);
    sm->vecs1 = FER_ALLOC_ARR(fer_real_t, dim * maxlen1);
    sm->vecs2 = FER_ALLOC_ARR(fer_real_t, dim * maxlen2);

    sm->cl = ferCLNewSimple2(opencl_program_len, opencl_program, "");
    ferCLKernelNew(sm->cl, "nearestNeighbor");

    sm->dist2   = FER_ALLOC_ARR(fer_real_t, maxlen1 * 2);
    sm->nearest = FER_ALLOC_ARR(int, maxlen1 * 2);

    return sm;
}

void ferSurfMatchDel(fer_surf_match_t *sm)
{
    ferCLDel(sm->cl);
    free(sm->vecs1);
    free(sm->vecs2);
    free(sm);
}

void ferSurfMatchSet1(fer_surf_match_t *sm, size_t i, const fer_vec_t *v)
{
    ferVecCopy(dim, sm->vecs1 + (dim * i), v);
}

void ferSurfMatchSet2(fer_surf_match_t *sm, size_t i, const fer_vec_t *v)
{
    ferVecCopy(dim, sm->vecs2 + (dim * i), v);
}

const fer_vec_t *ferSurfMatchGet1(fer_surf_match_t *sm, size_t i)
{
    return sm->vecs1 + (dim * i);
}

const fer_vec_t *ferSurfMatchGet2(fer_surf_match_t *sm, size_t i)
{
    return sm->vecs2 + (dim * i);
}



void ferSurfMatch(fer_surf_match_t *sm, size_t _len1, size_t _len2,
                  size_t max_threads)
{
    fer_real_t *vecs1, *vecs2;
    fer_real_t *dist, *dist_host, tmp_dist;
    int *ids, *ids_host, tmp_ids;
    int len1 = _len1, len2 = _len2;
    int row, i, col, pos;
    size_t glob[1], loc[1];

    vecs1 = FER_CL_CLONE_FROM_HOST(sm->cl, sm->vecs1, fer_real_t, len1 * dim);
    vecs2 = FER_CL_CLONE_FROM_HOST(sm->cl, sm->vecs2, fer_real_t, len2 * dim);
    dist  = FER_CL_ALLOC_ARR(sm->cl, fer_real_t, len1 * len2);
    ids   = FER_CL_ALLOC_ARR(sm->cl, int, len1 * len2);

    FER_CL_KERNEL_SET_ARG(sm->cl, 0, 0, len1);
    FER_CL_KERNEL_SET_ARG(sm->cl, 0, 1, vecs1);
    FER_CL_KERNEL_SET_ARG(sm->cl, 0, 2, len2);
    FER_CL_KERNEL_SET_ARG(sm->cl, 0, 3, vecs2);
    FER_CL_KERNEL_SET_ARG(sm->cl, 0, 4, dist);
    FER_CL_KERNEL_SET_ARG(sm->cl, 0, 5, ids);

    glob[0] = len2;
    if (len2 < max_threads){
        loc[0] = len2;
    }else if (len2 % max_threads == 0){
        loc[0] = max_threads;
    }else{
        for (i = max_threads; i > 0; i--){
            if (len2 % i == 0){
                loc[0] = i;
                break;
            }
        }
    }

    ferCLKernelEnqueue(sm->cl, 0, 1, glob, loc);
    ferCLFinish(sm->cl);

    dist_host = FER_ALLOC_ARR(fer_real_t, len1 * len2);
    ids_host  = FER_ALLOC_ARR(int, len1 * len2);
    FER_CL_COPY_TO_HOST(sm->cl, dist, dist_host, fer_real_t, len1 * len2);
    FER_CL_COPY_TO_HOST(sm->cl, ids, ids_host, int, len1 * len2);

    /*
    {
        int i, j;
        for (i = 0; i < len1; i++){
            fprintf(stderr, "[%03d]", i);
            for (j = 0; j < len2; j++){
                fprintf(stderr, " %1.3f", dist_host[len2 * i + j]);
            }
            fprintf(stderr, "\n");
            fprintf(stderr, "[%03d]", i);
            for (j = 0; j < len2; j++){
                fprintf(stderr, " % 5d", ids_host[len2 * i + j]);
            }
            fprintf(stderr, "\n");
        }
    }
    */

    // sort what couldn't be sorted on device
    for (row = 0; row < len1; row++){
        sm->dist2[2 * row] = dist_host[len2 * row];
        sm->dist2[2 * row + 1] = dist_host[len2 * row + 1];
        sm->nearest[2 * row] = ids_host[len2 * row];
        sm->nearest[2 * row + 1] = ids_host[len2 * row + 1];

        for (col = loc[0]; col < glob[0]; col += loc[0]){
            for (i = 0; i < 2; i++){
                pos = len2 * row + col + i;

                if (dist_host[pos] < sm->dist2[2 * row + 1]){
                    sm->dist2[2 * row + 1] = dist_host[pos];
                    sm->nearest[2 * row + 1] = ids_host[pos];

                    if (sm->dist2[2 * row + 1] < sm->dist2[2 * row]){
                        FER_SWAP(sm->dist2[2 * row + 1], sm->dist2[2 * row], tmp_dist);
                        FER_SWAP(sm->nearest[2 * row + 1], sm->nearest[2 * row], tmp_ids);
                    }
                }
            }
        }

        // test correctness
        /*
        for (i = 0; i < len2; i++){
            pos = len2 * row + i;

            if (ids_host[pos] != sm->nearest[2 * row]
                    && ids_host[pos] != sm->nearest[2 * row + 1]
                    && dist_host[pos] < sm->dist2[2 * row + 1]){
                DBG("Error: [%d] dist: %f, idx: %d", row, dist_host[pos], ids_host[pos]);
                DBG("Error:      dist1: %f, idx1: %d, dist2: %f, idx2: %d",
                    sm->dist2[2 * row], sm->nearest[2 * row],
                    sm->dist2[2 * row + 1], sm->nearest[2 * row + 1]);
            }
        }
        */
    }

    free(dist_host);
    free(ids_host);

    FER_CL_FREE(sm->cl, vecs1);
    FER_CL_FREE(sm->cl, vecs2);
    FER_CL_FREE(sm->cl, dist);
    FER_CL_FREE(sm->cl, ids);
}



static void bubbleUp(int vi, fer_real_t d, fer_real_t *dist, int *nearest)
{
    fer_real_t tmp_dist;
    int tmp_nearest;

    if (dist[1] > d){
        dist[1] = d;
        nearest[1] = vi;

        if (dist[0] > dist[1]){
            FER_SWAP(dist[0], dist[1], tmp_dist);
            FER_SWAP(nearest[0], nearest[1], tmp_nearest);
        }
    }
}

static void nearestLinear(fer_surf_match_t *sm, size_t pi, size_t len2)
{
    fer_real_t dist[2], d;
    int nearest[2];
    const fer_vec_t *p = sm->vecs1 + (pi * dim);
    const fer_vec_t *v;
    size_t i;

    dist[0] = dist[1] = FER_REAL_MAX;
    nearest[0] = nearest[1] = -1;

    for (i = 0; i < len2; i++){
        v = sm->vecs2 + (i * dim);
        d = ferVecDist2(dim, v, p);
        //DBG("d[%d]: %.20f (%lx)", i, d, (long)v);
        bubbleUp(i, d, dist, nearest);
    }

    sm->dist2[2 * pi]       = dist[0];
    sm->dist2[2 * pi + 1]   = dist[1];
    sm->nearest[2 * pi]     = nearest[0];
    sm->nearest[2 * pi + 1] = nearest[1];
}

void ferSurfMatchLinear(fer_surf_match_t *sm, size_t len1, size_t len2)
{
    size_t i;

    for (i = 0; i < len1; i++){
        nearestLinear(sm, i, len2);
    }
}


void ferSurfMatchNearest(const fer_surf_match_t *sm, size_t i,
                         int *nearest1, int *nearest2,
                         fer_real_t *dist1, fer_real_t *dist2)
{
    if (nearest1)
        *nearest1 = sm->nearest[2 * i];
    if (nearest2)
        *nearest2 = sm->nearest[2 * i + 1];

    if (dist1)
        *dist1 = sm->dist2[2 * i];
    if (dist2)
        *dist2 = sm->dist2[2 * i + 1];
}
