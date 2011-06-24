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

#include <fermat/opencl.h>

struct _fer_cd_sap_gpu_t {
    fer_cd_sap_t sap;

    fer_hmap_t *pairs_reg; /*!< Register (hash map) of collide pairs */

    fer_cl_t *cl;
    fer_cd_sap_minmax_t *minmax, *minmax_tmp;
    uint32_t minmax_len;
    cl_uint *counter;
    cl_uint *counter_sum;
    cl_uint *negative;
    uint32_t num_groups;

    struct {
        int reduce;
        int fix_counter1;
        int fix_counter2;
        int copy;
    } kernel;
};
typedef struct _fer_cd_sap_gpu_t fer_cd_sap_gpu_t;

static void sapgpuRadixSort(fer_cd_sap_t *sap, int axis);
static void sapgpuFindPairs(fer_cd_sap_t *sap);

static fer_cd_sap_t *ferCDSAPGPUNew(fer_cd_t *cd, uint64_t flags)
{
    fer_cd_sap_gpu_t *sap;
    char *program;
    size_t regsize;

    sap = FER_ALLOC(fer_cd_sap_gpu_t);

    ferCDSAPInit(cd, &sap->sap, 1);
    sap->sap.type = FER_CD_SAP_TYPE_GPU;
    sap->sap.radix_sort = sapgpuRadixSort;
    sap->sap.find_pairs = sapgpuFindPairs;

    regsize = __FER_CD_SAP_HASH_TABLE_SIZE(flags);
    sap->pairs_reg = ferHMapNew(regsize, pairHash, pairEq, (void *)sap);


    // TODO
    // create OpenCL instance
    program = ferCLProgramFromFile("sap.cl");
    sap->cl = ferCLNewSimple(program, "");
    free(program);

    // create kernels
    if (ferCLKernelNew(sap->cl, "radixSortReduce") != 0
            || ferCLKernelNew(sap->cl, "radixSortFixCounter1") != 0
            || ferCLKernelNew(sap->cl, "radixSortFixCounter2") != 0
            || ferCLKernelNew(sap->cl, "radixSortCopy") != 0){
        fprintf(stderr, "Error: Can't load kernels for GPU Radix Sort! Exiting...\n");
        exit(-1);
    }
    sap->kernel.reduce       = 0;
    sap->kernel.fix_counter1 = 1;
    sap->kernel.fix_counter2 = 2;
    sap->kernel.copy         = 3;

    sap->num_groups  = 40;
    sap->minmax      = NULL;
    sap->minmax_tmp  = NULL;
    sap->minmax_len  = 0;
    sap->counter     = FER_CL_ALLOC_ARR(sap->cl, cl_uint, 16 * 16 * sap->num_groups);
    sap->counter_sum = FER_CL_ALLOC_ARR(sap->cl, cl_uint, 16 * sap->num_groups);
    sap->negative    = FER_CL_ALLOC_ARR(sap->cl, cl_uint, sap->num_groups);

    return (fer_cd_sap_t *)sap;
}

static void ferCDSAPGPUDel(fer_cd_sap_t *_sap)
{
    fer_cd_sap_gpu_t *sap = (fer_cd_sap_gpu_t *)_sap;

    ferHMapDel(sap->pairs_reg);

    if (sap->minmax){
        FER_CL_FREE(sap->cl, sap->minmax);
        FER_CL_FREE(sap->cl, sap->minmax_tmp);
    }
    if (sap->counter){
        FER_CL_FREE(sap->cl, sap->counter);
    }
    if (sap->counter_sum){
        FER_CL_FREE(sap->cl, sap->counter_sum);
    }
    if (sap->negative){
        FER_CL_FREE(sap->cl, sap->negative);
    }

    ferCDSAPDestroy(&sap->sap);

    free(sap);
}

/* alloc enough memory on device and copy there data from host */
static void gpuRadixSortLoad(fer_cd_sap_gpu_t *sap, int axis);
static void gpuRadixSortSave(fer_cd_sap_gpu_t *sap, int axis);
static void gpuRadixSortRun(fer_cd_sap_gpu_t *sap);

static void sapgpuRadixSort(fer_cd_sap_t *_sap, int axis)
{
    fer_cd_sap_gpu_t *sap = (fer_cd_sap_gpu_t *)_sap;

    gpuRadixSortLoad(sap, axis);
    gpuRadixSortRun(sap);
    gpuRadixSortSave(sap, axis);

}

static void gpuRadixSortLoad(fer_cd_sap_gpu_t *sap, int axis)
{
    if (sap->minmax_len != 2 * sap->sap.geoms_len){
        if (sap->minmax){
            FER_CL_FREE(sap->cl, sap->minmax);
            FER_CL_FREE(sap->cl, sap->minmax_tmp);
        }

        sap->minmax_len = 2 * sap->sap.geoms_len;
        sap->minmax     = FER_CL_CLONE_FROM_HOST(sap->cl, sap->sap.minmax[axis],
                                                 fer_cd_sap_minmax_t, sap->minmax_len);
        sap->minmax_tmp = FER_CL_ALLOC_ARR(sap->cl, fer_cd_sap_minmax_t, sap->minmax_len);
    }else{
        FER_CL_COPY_FROM_HOST(sap->cl, sap->sap.minmax[axis], sap->minmax,
                              fer_cd_sap_minmax_t, sap->minmax_len);
    }
}

static void gpuRadixSortSave(fer_cd_sap_gpu_t *sap, int axis)
{
    size_t i;
    fer_cd_sap_minmax_t *minmax;
    fer_cd_sap_geom_t *geoms;

    minmax = sap->sap.minmax[axis];
    geoms  = sap->sap.geoms;

    FER_CL_COPY_TO_HOST(sap->cl, sap->minmax, minmax,
                        fer_cd_sap_minmax_t, sap->minmax_len);

    for (i = 0; i < sap->minmax_len; i++){
        if (MINMAX_ISMAX(&minmax[i])){
            geoms[MINMAX_GEOM(&minmax[i])].max[axis] = i;
        }else{
            geoms[MINMAX_GEOM(&minmax[i])].min[axis] = i;
        }
    }
}

static void gpuRadixSortRun(fer_cd_sap_gpu_t *sap)
{
    size_t glob[1], loc[1], glob_fix[1];
    uint32_t i, shift, use_negative;
    fer_cd_sap_minmax_t *src, *dst, *tmp;

    glob[0] = 16 * sap->num_groups;
    loc[0] = 16;
    glob_fix[0] = 2 * loc[0];

    shift = 0;
    src = sap->minmax;
    dst = sap->minmax_tmp;
    for (i = 0; i < 8; i++){
        use_negative = (i == 7);

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 0, src);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 1, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 2, sap->counter);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 3, sap->counter_sum);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 4, sap->negative);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 5, shift);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 6, use_negative);
        ferCLKernelEnqueue(sap->cl, sap->kernel.reduce, 1, glob, loc);

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter1, 0, sap->counter_sum);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter1, 1, sap->num_groups);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter1, 2, sap->negative);
        if (use_negative){
            ferCLKernelEnqueue(sap->cl, sap->kernel.fix_counter1, 1, glob_fix, loc);
        }else{
            ferCLKernelEnqueue(sap->cl, sap->kernel.fix_counter1, 1, loc, loc);
        }

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter2, 0, sap->counter);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter2, 1, sap->counter_sum);
        ferCLKernelEnqueue(sap->cl, sap->kernel.fix_counter2, 1, glob, loc);

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 0, src);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 1, dst);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 2, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 3, sap->counter);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 4, sap->negative);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 5, shift);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 6, use_negative);
        ferCLKernelEnqueue(sap->cl, sap->kernel.copy, 1, glob, loc);

        FER_SWAP(src, dst, tmp);
        shift += 4;
    }

    ferCLFinish(sap->cl);
}


static void sapgpuFindPairs(fer_cd_sap_t *_sap)
{
    fer_cd_sap_gpu_t *sap = (fer_cd_sap_gpu_t *)_sap;
    sapFindPairs1(_sap, sap->pairs_reg);
}
