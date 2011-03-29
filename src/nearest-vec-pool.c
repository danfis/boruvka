#include <fermat/nearest-vec-pool.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

#ifdef FER_OPENCL
# include "nearest-vec-pool-cl.c"
#endif /* FER_OPENCL */

fer_nn_vec_pool_t *ferNNVecPoolNew(size_t len, size_t dim)
{
    fer_nn_vec_pool_t *vp;

    vp = FER_ALLOC(fer_nn_vec_pool_t);
    vp->len = len;
    vp->dim = dim;

    vp->vecs  = FER_ALLOC_ARR(fer_real_t, len * dim);
    vp->dists = FER_ALLOC_ARR(fer_real_t, len);

#ifdef FER_OPENCL
    vp->cl       = ferCLNewSimple2(opencl_program_len, opencl_program, "");
    ferCLKernelNew(vp->cl, "nearestNeighbor");
    ferCLKernelNew(vp->cl, "nearestNeighbor_2_64");
    vp->vecs_cl  = NULL;
    vp->dists_cl = NULL;
#endif /* FER_OPENCL */

    return vp;
}

void ferNNVecPoolDel(fer_nn_vec_pool_t *vp)
{
    free(vp->vecs);
    free(vp->dists);

#ifdef FER_OPENCL
    if (vp->vecs_cl)
        FER_CL_FREE(vp->cl, vp->vecs_cl);
    if (vp->dists_cl)
        FER_CL_FREE(vp->cl, vp->dists_cl);
    if (vp->cl)
        ferCLDel(vp->cl);
#endif /* FER_OPENCL */

    free(vp);
}

int ferNNVecPoolSetVec(fer_nn_vec_pool_t *vp, size_t i, fer_vec_t *v)
{
    if (vp->len <= i)
        return -1;

    ferVecCopy(vp->dim, vp->vecs + (vp->dim * i), v);
    return 0;
}

static void bubbleUp(fer_vec_t *v, fer_real_t d,
                     size_t len, fer_real_t *dist, fer_vec_t **vecs)
{
    fer_real_t tmpd;
    fer_vec_t *tmpv;

    if (dist[len - 1] > d){
        dist[len - 1] = d;
        vecs[len - 1] = v;
        len--;

        while (len > 0 && dist[len - 1] > d){
            FER_SWAP(dist[len - 1], dist[len], tmpd);
            FER_SWAP(vecs[len - 1], vecs[len], tmpv);
            len--;
        }
    }
}

int ferNNVecPoolNearest(fer_nn_vec_pool_t *vp, size_t len, size_t k,
                        const fer_vec_t *p, fer_vec_t **vecs)
{
    size_t i;
    fer_real_t *dist, d;
    fer_vec_t *v;

    if (len > vp->len)
        return -1;


    dist = FER_ALLOC_ARR(fer_real_t, k);
    for (i = 0; i < k; i++){
        dist[i] = FER_REAL_MAX;
        vecs[i] = NULL;
    }

    for (i = 0; i < len; i++){
        v = vp->vecs + (i * vp->dim);
        d = ferVecDist2(vp->dim, v, p);
        //DBG("d[%d]: %.20f (%lx)", i, d, (long)v);
        bubbleUp(v, d, k, dist, vecs);
    }

    free(dist);

    return 0;
}

int ferNNVecPoolNearestCL(fer_nn_vec_pool_t *vp, size_t len, size_t k,
                          const fer_vec_t *p, fer_vec_t **vecs)
{
    fer_real_t *vecs_dev, *dists_dev, *p_dev;
    int len_dev, k_dev;
    int *ids, *ids_dev;
    size_t glob[1], loc[1], i;

    ids  = FER_ALLOC_ARR(int, k);

    vecs_dev  = FER_CL_CLONE_FROM_HOST(vp->cl, vp->vecs, float, len * vp->dim);
    dists_dev = FER_CL_ALLOC_ARR(vp->cl, float, len);
    ids_dev   = FER_CL_ALLOC_ARR(vp->cl, int, len);
    p_dev     = FER_CL_CLONE_FROM_HOST(vp->cl, p, float, vp->dim);
    len_dev = len;
    k_dev   = k;

    if (vp->dim == 64 && k == 2){
        FER_CL_KERNEL_SET_ARG(vp->cl, 1, 0, len_dev);
        FER_CL_KERNEL_SET_ARG(vp->cl, 1, 1, vecs_dev);
        FER_CL_KERNEL_SET_ARG(vp->cl, 1, 2, p_dev);
        FER_CL_KERNEL_SET_ARG(vp->cl, 1, 3, dists_dev);
        FER_CL_KERNEL_SET_ARG(vp->cl, 1, 4, ids_dev);

        glob[0] = len;
        loc[0] = len;
        ferCLKernelEnqueue(vp->cl, 1, 1, glob, loc);
        ferCLFinish(vp->cl);
    }else{

    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 0, vp->dim);
    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 1, len_dev);
    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 2, k_dev);
    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 3, vecs_dev);
    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 4, p_dev);
    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 5, dists_dev);
    FER_CL_KERNEL_SET_ARG(vp->cl, 0, 6, ids_dev);

    glob[0] = len;
    loc[0] = len;
    ferCLKernelEnqueue(vp->cl, 0, 1, glob, loc);
    ferCLFinish(vp->cl);
    }

    FER_CL_COPY_TO_HOST(vp->cl, ids_dev, ids, int, k);

    for (i = 0; i < k; i++){
        vecs[i] = vp->vecs + (vp->dim * ids[i]);
    }

#if 0
    {
        fer_real_t *dist = FER_ALLOC_ARR(fer_real_t, k);
        FER_CL_COPY_TO_HOST(vp->cl, dists_dev, dist, fer_real_t, k);
        for (i = 0; i < k; i++){
            DBG("dist[%d]: %.20f", i, dist[i]);
        }
        free(dist);
    }
#endif

    FER_CL_FREE(vp->cl, vecs_dev);
    FER_CL_FREE(vp->cl, dists_dev);
    FER_CL_FREE(vp->cl, ids_dev);
    FER_CL_FREE(vp->cl, p_dev);
    free(ids);

    return 0;

#if 0
    fer_real_t dists[15];
    FER_VEC(p, 3);
    fer_real_t *vecs_dev, *dists_dev;
    fer_real_t *p_dev;
    int i, len = 15, dim = 3, k_nearest = 10;
    size_t glob[1], loc[1];

    ferCLPrintPlatforms(stderr);

    program = ferCLProgramFromFile("opencl-nn.cl");
    cl = ferCLNewSimple(program, "");

    ferCLKernelNew(cl, "nearestNeighbor");

    ferTimerStart(&timer);

    ferVecSetZero(3, p);
    vecs_dev  = FER_CL_CLONE_FROM_HOST(cl, vecs, float, len * dim);
    dists_dev = FER_CL_ALLOC_ARR(cl, float, len);
    p_dev     = FER_CL_CLONE_FROM_HOST(cl, p, float, dim);

    FER_CL_KERNEL_SET_ARG(cl, 0, 0, dim);
    FER_CL_KERNEL_SET_ARG(cl, 0, 1, len);
    FER_CL_KERNEL_SET_ARG(cl, 0, 2, k_nearest);
    FER_CL_KERNEL_SET_ARG(cl, 0, 3, vecs_dev);
    FER_CL_KERNEL_SET_ARG(cl, 0, 4, p_dev);
    FER_CL_KERNEL_SET_ARG(cl, 0, 5, dists_dev);

    glob[0] = len;
    loc[0] = len;
    ferCLKernelEnqueue(cl, 0, 1, glob, loc);
    ferCLFinish(cl);

    FER_CL_COPY_TO_HOST(cl, dists_dev, dists, float, len);
    ferTimerStop(&timer);
    fprintf(stderr, "Time: %lu us\n", ferTimerElapsedInUs(&timer));
    for (i = 0; i < len; i++){
        fprintf(stderr, "[%d] %f\n", i, dists[i]);
    }

    ferCLDel(cl);
#endif
}
