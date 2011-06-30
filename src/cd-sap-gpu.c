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
#include "cd-sap-gpu-cl.c"

struct _fer_cd_sap_gpu_t {
    fer_cd_sap_t sap;

    fer_hmap_t *pairs_reg; /*!< Register (hash map) of collide pairs */

    fer_cl_t *cl;
    fer_cd_sap_minmax_t *minmax, *minmax_tmp;
    uint32_t minmax_len;
    uint32_t *counters, *block_offsets, *sums;

    struct {
        int kernel;
        size_t glob[1], loc[1];
        uint32_t num_groups, sharedmem;
    } blocks;

    struct {
        int kernel;
        size_t glob[1], loc[1];
        uint32_t num_groups, sharedmem;
    } offsets;

    struct {
        int kernel1, kernel2;
        size_t glob1[1], loc1[1];
        size_t glob2[1], loc2[1];
        size_t glob3[1], loc3[1];
        uint32_t len;
        uint32_t num_groups;
        uint32_t sharedmem;
    } prescan;

    struct {
        int kernel;
        size_t glob[1], loc[1];
        uint32_t num_groups;
    } reorder;
};
typedef struct _fer_cd_sap_gpu_t fer_cd_sap_gpu_t;

static void sapgpuRadixSort(fer_cd_sap_t *sap, int axis);
static void sapgpuFindPairs(fer_cd_sap_t *sap);

static fer_cd_sap_t *ferCDSAPGPUNew(fer_cd_t *cd, uint64_t flags)
{
    fer_cd_sap_gpu_t *sap;
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
    sap->cl = ferCLNewSimple2(opencl_program_len, opencl_program, "");

    // create kernels
    if (ferCLKernelNew(sap->cl, "radixSortBlocks") != 0
            || ferCLKernelNew(sap->cl, "radixSortOffsets") != 0
            || ferCLKernelNew(sap->cl, "radixSortPrescan") != 0
            || ferCLKernelNew(sap->cl, "radixSortPrescanFinish") != 0
            || ferCLKernelNew(sap->cl, "radixSortReorder") != 0){
        fprintf(stderr, "Error: Can't load kernels for GPU Radix Sort! Exiting...\n");
        exit(-1);
    }
    sap->blocks.kernel   = 0;
    sap->offsets.kernel  = 1;
    sap->prescan.kernel1 = 2;
    sap->prescan.kernel2 = 3;
    sap->reorder.kernel  = 4;

    return (fer_cd_sap_t *)sap;
}

static void ferCDSAPGPUDel(fer_cd_sap_t *_sap)
{
    fer_cd_sap_gpu_t *sap = (fer_cd_sap_gpu_t *)_sap;

    ferHMapDel(sap->pairs_reg);

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
    size_t len;

    // "Blocks":
    // number of groups needed for "blocks"
    sap->blocks.num_groups = (2 * sap->sap.geoms_len) / (256 * 4);
    if (sap->blocks.num_groups % (256 * 4) > 0)
        sap->blocks.num_groups += 1;
    if (sap->blocks.num_groups == 0)
        sap->blocks.num_groups = 1;
    sap->blocks.loc[0] = 256;
    sap->blocks.glob[0] = 256 * sap->blocks.num_groups;
    sap->blocks.sharedmem = 4 * 256 * sizeof(uint32_t);

    // "Offsets":
    sap->offsets.num_groups = (2 * sap->sap.geoms_len) / (256 * 2);
    if (sap->offsets.num_groups % (256 * 2) > 0)
        sap->offsets.num_groups += 1;
    if (sap->offsets.num_groups == 0)
        sap->offsets.num_groups = 1;
    sap->offsets.loc[0] = 256;
    sap->offsets.glob[0] = 256 * sap->offsets.num_groups;
    sap->offsets.sharedmem = 2 * 256 * sizeof(uint32_t);

    // "Prescan"
    sap->prescan.len = 16 * sap->offsets.num_groups;
    sap->prescan.num_groups = sap->prescan.len / (4 * 256);
    if (sap->prescan.num_groups % (4 * 256) > 0)
        sap->prescan.num_groups += 1;
    if (sap->prescan.num_groups == 0)
        sap->prescan.num_groups = 1;
    sap->prescan.loc1[0] = 256;
    sap->prescan.glob1[0] = 256 * sap->prescan.num_groups;

    if (sap->prescan.num_groups > 4 * 256){
        fprintf(stderr, "Error: Can't sort %d groups using one group!\n",
                sap->prescan.num_groups);
        exit(-1);
    }
    sap->prescan.loc2[0] = 256;
    sap->prescan.glob2[0] = 256;
    sap->prescan.loc3[0] = 256;
    sap->prescan.glob3[0] = sap->prescan.glob1[0] - 256;
    sap->prescan.sharedmem = 4 * 256 * sizeof(uint32_t);

    // "Reorder"
    sap->reorder.num_groups = sap->offsets.num_groups;
    sap->reorder.loc[0] = 256;
    sap->reorder.glob[0] = sap->offsets.glob[0];


    // alloc memory
    sap->minmax_len = 2 * sap->sap.geoms_len;
    sap->minmax     = FER_CL_CLONE_FROM_HOST(sap->cl, sap->sap.minmax[axis],
                                             fer_cd_sap_minmax_t, sap->minmax_len);
    sap->minmax_tmp = FER_CL_ALLOC_ARR(sap->cl, fer_cd_sap_minmax_t, sap->minmax_len);

    len = sap->offsets.num_groups * 16;
    sap->counters = FER_CL_ALLOC_ARR(sap->cl, uint32_t, len);
    sap->block_offsets = FER_CL_ALLOC_ARR(sap->cl, uint32_t, len);

    len = sap->prescan.num_groups;
    sap->sums = FER_CL_ALLOC_ARR(sap->cl, uint32_t, len);
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

    // free allocated memory
    FER_CL_FREE(sap->cl, sap->minmax);
    FER_CL_FREE(sap->cl, sap->minmax_tmp);
    FER_CL_FREE(sap->cl, sap->counters);
    FER_CL_FREE(sap->cl, sap->block_offsets);
    FER_CL_FREE(sap->cl, sap->sums);
}

static void gpuRadixSortRun(fer_cd_sap_gpu_t *sap)
{
    uint32_t startbit;
    fer_cd_sap_minmax_t *src, *dst;
    uint32_t *zero = NULL;
    uint32_t one = 1;

    src = sap->minmax;
    dst = sap->minmax_tmp;

    for (startbit = 0; startbit < 32; startbit += 4){
        // sort blocks of 4 * 256 elements
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->blocks.kernel, 0, src);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->blocks.kernel, 1, dst);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->blocks.kernel, 2, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->blocks.kernel, 3, startbit);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->blocks.kernel, 4, sap->blocks.num_groups);
        ferCLKernelSetArg(sap->cl, sap->blocks.kernel, 5, sap->blocks.sharedmem, NULL);
        ferCLKernelEnqueue(sap->cl, sap->blocks.kernel, 1, sap->blocks.glob, sap->blocks.loc);


        // compute counters in each block
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->offsets.kernel, 0, dst);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->offsets.kernel, 1, sap->counters);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->offsets.kernel, 2, sap->block_offsets);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->offsets.kernel, 3, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->offsets.kernel, 4, startbit);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->offsets.kernel, 5, sap->offsets.num_groups);
        ferCLKernelSetArg(sap->cl, sap->offsets.kernel, 6, sap->offsets.sharedmem, NULL);
        ferCLKernelEnqueue(sap->cl, sap->offsets.kernel, 1, sap->offsets.glob, sap->offsets.loc);


        // prescan all counters
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 0, sap->counters);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 1, sap->counters);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 2, sap->sums);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 3, sap->prescan.len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 4, sap->prescan.num_groups);
        ferCLKernelSetArg(sap->cl, sap->prescan.kernel1, 5, sap->prescan.sharedmem, NULL);
        ferCLKernelEnqueue(sap->cl, sap->prescan.kernel1, 1, sap->prescan.glob1, sap->prescan.loc1);

        if (sap->prescan.num_groups > 1){
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 0, sap->sums);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 1, sap->sums);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 2, zero);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 3, sap->prescan.num_groups);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel1, 4, one);
            ferCLKernelSetArg(sap->cl, sap->prescan.kernel1, 5, sap->prescan.sharedmem, NULL);
            ferCLKernelEnqueue(sap->cl, sap->prescan.kernel1, 1, sap->prescan.glob2, sap->prescan.loc2);

            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel2, 0, sap->counters);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel2, 1, sap->counters);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel2, 2, sap->sums);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel2, 3, sap->prescan.len);
            FER_CL_KERNEL_SET_ARG(sap->cl, sap->prescan.kernel2, 4, sap->prescan.num_groups);
            ferCLKernelEnqueue(sap->cl, sap->prescan.kernel2, 1, sap->prescan.glob3, sap->prescan.loc3);
        }


        // reorder data according to counters
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 0, dst);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 1, src);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 2, sap->counters);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 3, sap->block_offsets);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 4, sap->minmax_len);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 5, startbit);
        FER_CL_KERNEL_SET_ARG(sap->cl, sap->reorder.kernel, 6, sap->reorder.num_groups);
        ferCLKernelEnqueue(sap->cl, sap->reorder.kernel, 1, sap->reorder.glob, sap->reorder.loc);
    }

    ferCLFinish(sap->cl);

    /*
    {
        int i;

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

    FER_CL_COPY_TO_HOST(sap->cl, src, sap->sap.minmax[0],
                        fer_cd_sap_minmax_t, sap->minmax_len);
    for (i = 0; i < 256 * 4 * 2; i++){
        if (i % 4 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " [% 11f (%x) | %04d]", sap->sap.minmax[0][i].val,
                (int)ferRealAsUInt(sap->sap.minmax[0][i].val),
                (int)sap->sap.minmax[0][i].geom_ismax);
        if ((i + 1) % 4 == 0)
            fprintf(stderr, "\n");
        if ((i + 1) % (256 * 4) == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    uint32_t *__counter       = FER_ALLOC_ARR(uint32_t, 16 * sap->offsets.num_groups);
    uint32_t *__block_offsets = FER_ALLOC_ARR(uint32_t, 16 * sap->offsets.num_groups);

    FER_CL_COPY_TO_HOST(sap->cl, sap->counters, __counter, uint32_t, 16 * sap->offsets.num_groups);
    FER_CL_COPY_TO_HOST(sap->cl, sap->block_offsets, __block_offsets, uint32_t, 16 * sap->offsets.num_groups);

    for (i = 0; i < 16 * sap->offsets.num_groups; i++){
        if (i % 16 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " %03d", __counter[i]);
        if ((i + 1) % 16 == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    for (i = 0; i < 16 * sap->offsets.num_groups; i++){
        if (i % 16 == 0)
            fprintf(stderr, "[%04d]: ", i);
        fprintf(stderr, " %03d", __block_offsets[i]);
        if ((i + 1) % 16 == 0)
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");

    free(__counter);
    free(__block_offsets);
    }
    //exit(-1);
    */
}


static void sapgpuFindPairs(fer_cd_sap_t *_sap)
{
    fer_cd_sap_gpu_t *sap = (fer_cd_sap_gpu_t *)_sap;
    sapFindPairs1(_sap, sap->pairs_reg);
}
