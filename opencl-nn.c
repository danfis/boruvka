#include <fermat/opencl.h>
#include <fermat/vec.h>
#include <fermat/timer.h>
#include <fermat/alloc.h>
#include <fermat/surf-matching.h>

#include "surf1.c"


int main(int argc, char *argv[])
{
    size_t i;
    fer_surf_match_t *sm, *sml;
    fer_timer_t timer;
    int n1, n2, nl1, nl2, len1, len2;
    fer_real_t d1, d2, dl1, dl2;

    sm = ferSurfMatchNew(3000, 3000);
    sml = ferSurfMatchNew(3000, 3000);

    for (i = 0; i < sizeof(surf1) / sizeof(float) / 64; i++){
        ferSurfMatchSet1(sm, i, surf1 + (i * 64));
        ferSurfMatchSet1(sml, i, surf1 + (i * 64));
    }
    for (i = 0; i < sizeof(surf3) / sizeof(float) / 64; i++){
        ferSurfMatchSet2(sm, i, surf3 + (i * 64));
        ferSurfMatchSet2(sml, i, surf3 + (i * 64));
    }

    len1 = sizeof(surf1) / sizeof(float) / 64;
    len2 = sizeof(surf2) / sizeof(float) / 64;
    //len1 = 20;
    //len2 = 20;

    ferTimerStart(&timer);
    ferSurfMatchLinear(sml, len1, len2);
    ferTimerStop(&timer);
    printf("Linear: %lu us\n", ferTimerElapsedInUs(&timer));

    /*
    for (i = 0; i < len1; i++){
        ferSurfMatchNearest(sml, i, &n1, &n2, &d1, &d2);
        printf("[%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, n1, d1, n2, d2);
    }
    */

    ferTimerStart(&timer);
    ferSurfMatch(sm, len1, len2);
    ferTimerStop(&timer);
    printf("OpenCL: %lu us\n", ferTimerElapsedInUs(&timer));

    /*
    for (i = 0; i < len1; i++){
        ferSurfMatchNearest(sm, i, &n1, &n2, &d1, &d2);
        printf("[%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, n1, d1, n2, d2);
    }
    */

    // check
    for (i = 0; i < len1; i++){
        ferSurfMatchNearest(sm, i, &n1, &n2, &d1, &d2);
        ferSurfMatchNearest(sml, i, &nl1, &nl2, &dl1, &dl2);

        if (n1 != nl1 || n2 != nl2){
            printf("diff opencl [%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, n1, d1, n2, d2);
            printf("diff linear [%04d] - % 4d (%2.3f), % 4d (%2.3f)\n", i, nl1, dl1, nl2, dl2);
        }
    }

    ferSurfMatchDel(sm);
    ferSurfMatchDel(sml);

#if 0
    fer_nn_vec_pool_t *vp;
    fer_vec_t *vecs[10];
    FER_VEC(p, 64);
    int i, k, res;
    fer_timer_t timer;

    vp = ferNNVecPoolNew(3000, 64);
    for (i = 0; i < sizeof(surf1) / sizeof(float) / 64; i++){
        ferNNVecPoolSetVec(vp, i, surf1 + (i * 64));
    }

    k = 3;
    ferVecSetZero(64, p);

    ferTimerStart(&timer);
    res = ferNNVecPoolNearest(vp, 500, k, p, vecs);
    ferTimerStop(&timer);

    printf("Linear: %d, %lu us\n", res, ferTimerElapsedInUs(&timer));
    for (i = 0; i < k; i++){
        printf("[%u] %.20f (%lx)\n", i, ferVecDist2(64, p, vecs[i]), (long)vecs[i]);
    }

    ferTimerStart(&timer);
    res = ferNNVecPoolNearestCL(vp, 500, k, p, vecs);
    ferTimerStop(&timer);

    printf("OpenCL: %d, %lu us\n", res, ferTimerElapsedInUs(&timer));
    for (i = 0; i < k; i++){
        printf("[%u] %.20f (%lx)\n", i, ferVecDist2(64, p, vecs[i]), (long)vecs[i]);
    }

    k = 2;
    ferVecSetZero(64, p);

    ferTimerStart(&timer);
    res = ferNNVecPoolNearest(vp, 500, k, p, vecs);
    ferTimerStop(&timer);

    printf("Linear: %d, %lu us\n", res, ferTimerElapsedInUs(&timer));
    for (i = 0; i < k; i++){
        printf("[%u] %.20f (%lx)\n", i, ferVecDist2(64, p, vecs[i]), (long)vecs[i]);
    }

    ferTimerStart(&timer);
    res = ferNNVecPoolNearestCL(vp, 500, k, p, vecs);
    ferTimerStop(&timer);

    printf("OpenCL: %d, %lu us\n", res, ferTimerElapsedInUs(&timer));
    for (i = 0; i < k; i++){
        printf("[%u] %.20f (%lx)\n", i, ferVecDist2(64, p, vecs[i]), (long)vecs[i]);
    }


    ferNNVecPoolDel(vp);
#endif

#if 0
    fer_cl_t *cl;
    char *program;
    //fer_vec_t *vecs;
    fer_vec_t vecs[] = { 1, 2, 3,
                         1, 3, 4,
                         1, 5, 6,
                         1, 1, 1,
                         3, 3, 3,
                         1, 2, 3,
                         1, 3, 4,
                         1, 5, 6,
                         1, 1, 1,
                         3, 3, 3,
                         1, 2, 3,
                         1, 3, 4,
                         1, 5, 6,
                         1, 1, 1,
                         3, 3, 3,
                         1, 1, 1,
                         2, 2, 2
    };

    fer_timer_t timer;
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
    free(program);
#endif

    return 0;
}
