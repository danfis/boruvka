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

#ifndef __FER_OPENCL_H__
#define __FER_OPENCL_H__

#include <stdio.h>
#include <CL/opencl.h>
#include <fermat/core.h>

#ifndef FER_OPENCL
# error "Fermat is not compiled with OpenCL support!"
#endif /* FER_OPENCL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Allocates array on device.
 */
#define FER_CL_ALLOC_ARR(cl, type, count) \
    (type *)ferCLAlloc((cl), sizeof(type) * (count), NULL, CL_MEM_READ_WRITE)
#define FER_CL_ALLOC_ARR_RO(cl, type, count) \
    (type *)ferCLAlloc((cl), sizeof(type) * (count), NULL, CL_MEM_READ_ONLY)
#define FER_CL_ALLOC_ARR_WO(cl, type, count) \
    (type *)ferCLAlloc((cl), sizeof(type) * (count), NULL, CL_MEM_WRITE_ONLY)

/**
 * Allocates array on device and copy there data from provided host memory.
 */
#define FER_CL_CLONE_FROM_HOST(cl, src, type, count) \
    (type *)ferCLAlloc((cl), sizeof(type) * (count), (void *)(src), CL_MEM_READ_WRITE)
#define FER_CL_CLONE_FROM_HOST_RO(cl, src, type, count) \
    (type *)ferCLAlloc((cl), sizeof(type) * (count), (void *)(src), CL_MEM_READ_ONLY)
#define FER_CL_CLONE_FROM_HOST_WO(cl, src, type, count) \
    (type *)ferCLAlloc((cl), sizeof(type) * (count), (void *)(src), CL_MEM_WRITE_ONLY)

#define FER_CL_MEM_MAP(cl, src, type, count) \
    (type *)ferCLMemMap((cl), (src), sizeof(type) * (count), CL_MEM_READ_WRITE)
#define FER_CL_MEM_MAP_RO(cl, src, type, count) \
    (type *)ferCLMemMap((cl), (src), sizeof(type) * (count), CL_MEM_READ_ONLY)
#define FER_CL_MEM_MAP_WO(cl, src, type, count) \
    (type *)ferCLMemMap((cl), (src), sizeof(type) * (count), CL_MEM_WRITE_ONLY)

#define FER_CL_COPY_FROM_HOST(cl, src, dst, type, count) \
    ferCLCopyFromHost((cl), (src), (dst), sizeof(type) * count)
#define FER_CL_COPY_TO_HOST(cl, src, dst, type, count) \
    ferCLCopyToHost((cl), (src), (dst), sizeof(type) * count)

/**
 * Allocates memory on host and copy there data from device memory.
 */
#define FER_CL_CLONE_TO_HOST(cl, src, type, count) \
    (type *)ferCLCloneToHost(cl, src, sizeof(type) * (count))

/**
 * Free memory allocated on device.
 */
#define FER_CL_FREE(cl, ptr) \
    clReleaseMemObject((void *)(ptr))


struct _fer_cl_t {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;

    cl_kernel *kernels; /*!< List of kernels from program */
    size_t kernels_len;
};
typedef struct _fer_cl_t fer_cl_t;

/**
 * Creates new OpenCL program from given NULL-terminated string.
 * A "Simple" means that first GPU capable device will be chosen.
 */
fer_cl_t *ferCLNewSimple(const char *program, const char *buildopts);

/**
 * Deletes previoulsy created OpenCL program.
 */
void ferCLDel(fer_cl_t *cl);

/**
 * Prints to given output info about platforms and devices.
 */
void ferCLPrintPlatforms(FILE *out);

/**
 * Reads program from given file. Malloc'ed array is returned.
 */
char *ferCLProgramFromFile(const char *filename);

/**
 * Creates new kernel of given file from OpenCL program.
 * Kernel's ID (that can be used in ferCLKernel() function) is returned
 * or -1 is returned on error.
 */
int ferCLKernelNew(fer_cl_t *cl, const char *kernel);

/**
 * Returns number of kernels stored in OpenCL program.
 */
_fer_inline size_t ferCLKernelsLen(const fer_cl_t *cl);

/**
 * Returns i'th kernel.
 */
_fer_inline cl_kernel ferCLKernel(const fer_cl_t *cl, size_t i);

/**
 * Setting kernel's arg_idx's argument - on size of value must be spcified.
 */
#define FER_CL_KERNEL_SET_ARG(cl, kernel, arg_idx, value) \
    ferCLKernelSetArg((cl), (kernel), (arg_idx), sizeof(value), (const void *)&(value))

/**
 * Sets arg_idx'th arguemnt of kernel.
 */
int ferCLKernelSetArg(fer_cl_t *cl, size_t kernel, size_t arg_idx,
                      size_t size, const void *arg);

/**
 * Enqueue kernel in command queue.
 * Parameter {dim} is dimension of {glob} and {loc}, {glob} is array of
 * number of work groups (CUDA's grid size), {loc} is array of
 * number of work items per group (CUDA's thread size).
 */
int ferCLKernelEnqueue(fer_cl_t *cl, size_t kernel,
                       size_t dim, const size_t *glob, const size_t *loc);


/**
 * Flush device queue and block until finished.
 */
int ferCLFinish(fer_cl_t *cl);

/**
 * Allocate size bytes of memory on device. If src in non-NULL it must
 * point to host memory and that is copyied to newly allocated device
 * memory.
 */
void *ferCLAlloc(fer_cl_t *cl, size_t size, void *src, cl_mem_flags flags);

/**
 * Clone device memory to host memory.
 */
void *ferCLCloneToHost(fer_cl_t *cl, const void *src, size_t size);

void ferCLCopyToHost(fer_cl_t *cl, const void *src, void *dst, size_t size);
void ferCLCopyFromHost(fer_cl_t *cl, const void *src, void *dst, size_t size);

/**
 * Map given host memory to device memory.
 */
void *ferCLMemMap(fer_cl_t *cl, void *src, size_t size, cl_mem_flags flags);

/**** INLINES ****/
_fer_inline size_t ferCLKernelsLen(const fer_cl_t *cl)
{
    return cl->kernels_len;
}

_fer_inline cl_kernel ferCLKernel(const fer_cl_t *cl, size_t i)
{
    return cl->kernels[i];
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_OPENCL_H__ */

