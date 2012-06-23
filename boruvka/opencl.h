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

#ifndef __BOR_OPENCL_H__
#define __BOR_OPENCL_H__

#include <stdio.h>
#include <CL/opencl.h>
#include <boruvka/core.h>

#ifndef BOR_OPENCL
# error "Boruvka is not compiled with OpenCL support!"
#endif /* BOR_OPENCL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * OpenCL - Simplified API To OpenCL
 * ==================================
 */
struct _bor_cl_t {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;

    cl_kernel *kernels; /*!< List of kernels from program */
    size_t kernels_len;
};
typedef struct _bor_cl_t bor_cl_t;

/**
 * Functions and Macros
 * ---------------------
 */


/**
 * Allocates array on device.
 */
#define BOR_CL_ALLOC_ARR(cl, type, count) \
    (type *)borCLAlloc((cl), sizeof(type) * (count), NULL, CL_MEM_READ_WRITE)
#define BOR_CL_ALLOC_ARR_RO(cl, type, count) \
    (type *)borCLAlloc((cl), sizeof(type) * (count), NULL, CL_MEM_READ_ONLY)
#define BOR_CL_ALLOC_ARR_WO(cl, type, count) \
    (type *)borCLAlloc((cl), sizeof(type) * (count), NULL, CL_MEM_WRITE_ONLY)

/**
 * Allocates array on device and copy there data from provided host memory.
 */
#define BOR_CL_CLONE_FROM_HOST(cl, src, type, count) \
    (type *)borCLAlloc((cl), sizeof(type) * (count), (void *)(src), CL_MEM_READ_WRITE)
#define BOR_CL_CLONE_FROM_HOST_RO(cl, src, type, count) \
    (type *)borCLAlloc((cl), sizeof(type) * (count), (void *)(src), CL_MEM_READ_ONLY)
#define BOR_CL_CLONE_FROM_HOST_WO(cl, src, type, count) \
    (type *)borCLAlloc((cl), sizeof(type) * (count), (void *)(src), CL_MEM_WRITE_ONLY)

#define BOR_CL_MEM_MAP(cl, src, type, count) \
    (type *)borCLMemMap((cl), (src), sizeof(type) * (count), CL_MEM_READ_WRITE)
#define BOR_CL_MEM_MAP_RO(cl, src, type, count) \
    (type *)borCLMemMap((cl), (src), sizeof(type) * (count), CL_MEM_READ_ONLY)
#define BOR_CL_MEM_MAP_WO(cl, src, type, count) \
    (type *)borCLMemMap((cl), (src), sizeof(type) * (count), CL_MEM_WRITE_ONLY)

#define BOR_CL_COPY_FROM_HOST(cl, src, dst, type, count) \
    borCLCopyFromHost((cl), (src), (dst), sizeof(type) * count)
#define BOR_CL_COPY_TO_HOST(cl, src, dst, type, count) \
    borCLCopyToHost((cl), (src), (dst), sizeof(type) * count)

/**
 * Allocates memory on host and copy there data from device memory.
 */
#define BOR_CL_CLONE_TO_HOST(cl, src, type, count) \
    (type *)borCLCloneToHost(cl, src, sizeof(type) * (count))

/**
 * Free memory allocated on device.
 */
#define BOR_CL_FREE(cl, ptr) \
    clReleaseMemObject((void *)(ptr))


/**
 * Creates new OpenCL program from given NULL-terminated string.
 * A "Simple" means that first GPU capable device will be chosen.
 */
bor_cl_t *borCLNewSimple(const char *program, const char *buildopts);

/**
 * Similar to borCLNewSimple() but program is created from array of
 * NULL-terminated strings.
 */
bor_cl_t *borCLNewSimple2(size_t program_count, const char **program,
                          const char *buildopts);

/**
 * Deletes previoulsy created OpenCL program.
 */
void borCLDel(bor_cl_t *cl);

/**
 * Prints to given output info about platforms and devices.
 */
void borCLPrintPlatforms(FILE *out);

/**
 * Reads program from given file. Malloc'ed array is returned.
 */
char *borCLProgramFromFile(const char *filename);

/**
 * Creates new kernel of given file from OpenCL program.
 * Kernel's ID (that can be used in borCLKernel() function) is returned
 * or -1 is returned on error.
 */
int borCLKernelNew(bor_cl_t *cl, const char *kernel);

/**
 * Returns number of kernels stored in OpenCL program.
 */
_bor_inline size_t borCLKernelsLen(const bor_cl_t *cl);

/**
 * Returns i'th kernel.
 */
_bor_inline cl_kernel borCLKernel(const bor_cl_t *cl, size_t i);

/**
 * Setting kernel's arg_idx's argument - on size of value must be spcified.
 */
#define BOR_CL_KERNEL_SET_ARG(cl, kernel, arg_idx, value) \
    borCLKernelSetArg((cl), (kernel), (arg_idx), sizeof(value), (const void *)&(value))

/**
 * Sets arg_idx'th arguemnt of kernel.
 */
int borCLKernelSetArg(bor_cl_t *cl, size_t kernel, size_t arg_idx,
                      size_t size, const void *arg);

/**
 * Enqueue kernel in command queue.
 * Parameter {dim} is dimension of {glob} and {loc}, {glob} is array of
 * number of work groups (CUDA's grid size), {loc} is array of
 * number of work items per group (CUDA's thread size).
 */
int borCLKernelEnqueue(bor_cl_t *cl, size_t kernel,
                       size_t dim, const size_t *glob, const size_t *loc);


/**
 * Flush device queue and block until finished.
 */
int borCLFinish(bor_cl_t *cl);

/**
 * Allocate size bytes of memory on device. If src in non-NULL it must
 * point to host memory and that is copyied to newly allocated device
 * memory.
 */
void *borCLAlloc(bor_cl_t *cl, size_t size, void *src, cl_mem_flags flags);

/**
 * Clone device memory to host memory.
 */
void *borCLCloneToHost(bor_cl_t *cl, const void *src, size_t size);

void borCLCopyToHost(bor_cl_t *cl, const void *src, void *dst, size_t size);
void borCLCopyFromHost(bor_cl_t *cl, const void *src, void *dst, size_t size);

/**
 * Map given host memory to device memory.
 */
void *borCLMemMap(bor_cl_t *cl, void *src, size_t size, cl_mem_flags flags);

/**** INLINES ****/
_bor_inline size_t borCLKernelsLen(const bor_cl_t *cl)
{
    return cl->kernels_len;
}

_bor_inline cl_kernel borCLKernel(const bor_cl_t *cl, size_t i)
{
    return cl->kernels[i];
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __BOR_OPENCL_H__ */

