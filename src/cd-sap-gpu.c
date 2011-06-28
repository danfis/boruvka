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
    cl_uint *block_offsets;
    cl_uint *counter;
    cl_uint *counter_sum;
    uint32_t num_groups;

    struct {
        int reduce;
        int fix_counter1;
        int fix_counter2;
        int copy;

        int blocks;
        int offsets;
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
    ferCLPrintPlatforms(stdout);

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
            || ferCLKernelNew(sap->cl, "radixSortCopy") != 0
            || ferCLKernelNew(sap->cl, "radixSortBlocks") != 0
            || ferCLKernelNew(sap->cl, "radixSortOffsets") != 0){
        fprintf(stderr, "Error: Can't load kernels for GPU Radix Sort! Exiting...\n");
        exit(-1);
    }
    sap->kernel.reduce       = 0;
    sap->kernel.fix_counter1 = 1;
    sap->kernel.fix_counter2 = 2;
    sap->kernel.copy         = 3;
    sap->kernel.blocks       = 4;
    sap->kernel.offsets      = 5;

    sap->num_groups  = 1;
    sap->minmax      = NULL;
    sap->minmax_tmp  = NULL;
    sap->minmax_len  = 0;
    // TODO
    //sap->counter     = FER_CL_ALLOC_ARR(sap->cl, cl_uint, 16 * 16 * sap->num_groups);
    //sap->counter_sum = FER_CL_ALLOC_ARR(sap->cl, cl_uint, 16 * sap->num_groups);

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
    size_t glob[1], loc[1];
    uint32_t i, shift, nbits, startbit, num_groups;
    fer_cd_sap_minmax_t *src, *dst, *tmp;

    num_groups = sap->minmax_len / (256 * 4);
    if (sap->minmax_len % (256 * 4) > 0)
        num_groups += 1;
    nbits = 32;
    startbit = 0;
    glob[0] = 256 * 1;
    loc[0]  = 256;

    for (i = 0; i < 256 * 4 * 2; i++){
        if (i % 4 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " [% 11f (%x)]", sap->sap.minmax[0][i].val,
                (int)ferRealAsUInt(sap->sap.minmax[0][i].val));
        if ((i + 1) % 4 == 0)
            fprintf(stderr, "\n");
        if ((i + 1) % (256 * 4) == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.blocks, 0, sap->minmax);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.blocks, 1, sap->minmax_tmp);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.blocks, 2, sap->minmax_len);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.blocks, 3, startbit);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.blocks, 4, num_groups);
    ferCLKernelSetArg(sap->cl, sap->kernel.blocks, 5, 4 * 256 * sizeof(uint32_t), NULL);
    ferCLKernelEnqueue(sap->cl, sap->kernel.blocks, 1, glob, loc);

    ferCLFinish(sap->cl);

    FER_CL_COPY_TO_HOST(sap->cl, sap->minmax_tmp, sap->sap.minmax[0],
                        fer_cd_sap_minmax_t, sap->minmax_len);
    for (i = 0; i < 256 * 4 * 2; i++){
        if (i % 4 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " [% 11f (%x)]", sap->sap.minmax[0][i].val,
                (int)ferRealAsUInt(sap->sap.minmax[0][i].val));
        if ((i + 1) % 4 == 0)
            fprintf(stderr, "\n");
        if ((i + 1) % (256 * 4) == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    // ---------

    fprintf(stderr, "len, num_groups: %d %d\n", (int)sap->minmax_len, (int)num_groups);
    num_groups *= 2;
    glob[0] *= 1;
    sap->block_offsets = FER_CL_ALLOC_ARR(sap->cl, cl_uint, 16 * num_groups);
    sap->counter       = FER_CL_ALLOC_ARR(sap->cl, cl_uint, 16 * num_groups);

    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.offsets, 0, sap->minmax_tmp);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.offsets, 1, sap->counter);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.offsets, 2, sap->block_offsets);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.offsets, 3, sap->minmax_len);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.offsets, 4, startbit);
    FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.offsets, 5, num_groups);
    ferCLKernelSetArg(sap->cl, sap->kernel.offsets, 6, 3 * 256 * sizeof(uint32_t), NULL);
    ferCLKernelEnqueue(sap->cl, sap->kernel.offsets, 1, glob, loc);

    ferCLFinish(sap->cl);

    uint32_t *counter       = FER_ALLOC_ARR(uint32_t, 16 * num_groups);
    uint32_t *block_offsets = FER_ALLOC_ARR(uint32_t, 16 * num_groups);

    FER_CL_COPY_TO_HOST(sap->cl, sap->counter, counter, uint32_t, 16 * num_groups);
    FER_CL_COPY_TO_HOST(sap->cl, sap->block_offsets, block_offsets, uint32_t, 16 * num_groups);

    for (i = 0; i < 16 * num_groups; i++){
        if (i % 16 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " %03d", counter[i]);
        if ((i + 1) % 16 == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    for (i = 0; i < 16 * num_groups; i++){
        if (i % 16 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " %03d", block_offsets[i]);
        if ((i + 1) % 16 == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    FER_CL_FREE(sap->cl, sap->counter);
    FER_CL_FREE(sap->cl, sap->block_offsets);

    exit(-1);

    glob[0] = 16 * sap->num_groups;
    loc[0] = 16;

    shift = 0;
    src = sap->minmax;
    dst = sap->minmax_tmp;
    for (i = 0; i < 8; i++){
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 0, src);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 1, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 2, sap->counter);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 3, sap->counter_sum);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.reduce, 4, shift);
        ferCLKernelEnqueue(sap->cl, sap->kernel.reduce, 1, glob, loc);

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter1, 0, sap->counter_sum);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter1, 1, sap->num_groups);
        ferCLKernelEnqueue(sap->cl, sap->kernel.fix_counter1, 1, loc, loc);

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter2, 0, sap->counter);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.fix_counter2, 1, sap->counter_sum);
        ferCLKernelEnqueue(sap->cl, sap->kernel.fix_counter2, 1, glob, loc);

        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 0, src);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 1, dst);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 2, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 3, sap->counter);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->kernel.copy, 4, shift);
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
