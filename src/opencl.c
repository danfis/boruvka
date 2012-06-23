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

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <boruvka/opencl.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

const char *__borCLErrorStr(cl_int err);
_bor_inline int __borCLErrorCheck(cl_int err, const char *errstr);

bor_cl_t *borCLNewSimple(const char *program, const char *buildopts)
{
    return borCLNewSimple2(1, &program, buildopts);
}

bor_cl_t *borCLNewSimple2(size_t program_count, const char **program,
                          const char *buildopts)
{
    cl_uint num_platforms, num_devices, i;
    cl_int err;
    cl_platform_id *platforms, platform;
    cl_device_id device;
    size_t bufsize;
    char buf[1024], *buf2;
    bor_cl_t *cl;

    // find platform and device
    platform = (cl_platform_id)-1;

    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if (__borCLErrorCheck(err, "Can't get any platform") != 0)
        return NULL;
    if (num_platforms == 0)
        return NULL;

    platforms = BOR_ALLOC_ARR(cl_platform_id, num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    if (__borCLErrorCheck(err, "Can't get any platform") != 0)
        return NULL;

    for (i = 0; i < num_platforms; i++){
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
        if (__borCLErrorCheck(err, "Cant'get any device") != 0)
            break;

        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &device, &num_devices);
        if (__borCLErrorCheck(err, "Cant'get any device") != 0)
            break;

        if (num_devices > 0){
            platform = platforms[i];
            break;
        }
    }

    BOR_FREE(platforms);
    if (platform == (cl_platform_id)-1)
        return NULL;


    cl = BOR_ALLOC(bor_cl_t);
    cl->platform = platform;
    cl->device   = device;

    // create context
    cl->context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (__borCLErrorCheck(err, "Can't create context") != 0){
        BOR_FREE(cl);
        return NULL;
    }

    // create queue
    cl->queue = clCreateCommandQueue(cl->context, cl->device, 0, &err);
    if (__borCLErrorCheck(err, "Can't create command queue") != 0){
        clReleaseContext(cl->context);
        BOR_FREE(cl);
        return NULL;
    }

    // create program
    cl->program = clCreateProgramWithSource(cl->context, program_count, program, NULL, &err);
    if (__borCLErrorCheck(err, "Can't create program") != 0){
        clReleaseCommandQueue(cl->queue);
        clReleaseContext(cl->context);
        BOR_FREE(cl);
        return NULL;
    }

    // build program
    err = clBuildProgram(cl->program, 1, &cl->device, buildopts, NULL, NULL);
    if (__borCLErrorCheck(err, "Can't build program") != 0){
        err = clGetProgramBuildInfo(cl->program, cl->device, CL_PROGRAM_BUILD_LOG,
                                    1024, buf, &bufsize);
        if (err == CL_INVALID_VALUE && bufsize > 1024){
            buf2 = BOR_ALLOC_ARR(char, bufsize);
            err = clGetProgramBuildInfo(cl->program, cl->device, CL_PROGRAM_BUILD_LOG,
                                        bufsize, buf2, NULL);
            if (__borCLErrorCheck(err, "Can't obtain build log") == 0){
                fprintf(stderr, " >> Build log:\n%s\n", buf2);
            }
            BOR_FREE(buf2);
        }else{
            if (__borCLErrorCheck(err, "Can't obtain build log") == 0){
                fprintf(stderr, " >> Build log:\n%s\n", buf);
            }
        }

        clReleaseProgram(cl->program);
        clReleaseCommandQueue(cl->queue);
        clReleaseContext(cl->context);
        BOR_FREE(cl);
        return NULL;
    }

    cl->kernels = NULL;
    cl->kernels_len = 0;

    return cl;
}

void borCLDel(bor_cl_t *cl)
{
    size_t i;

    clFinish(cl->queue);

    for (i = 0; i < cl->kernels_len; i++){
        clReleaseKernel(cl->kernels[i]);
    }

    clReleaseProgram(cl->program);
    clReleaseCommandQueue(cl->queue);
    clReleaseContext(cl->context);

    if (cl->kernels)
        BOR_FREE(cl->kernels);
    BOR_FREE(cl);
}

int borCLKernelNew(bor_cl_t *cl, const char *kernel)
{
    cl_kernel k;
    cl_int err;

    k = clCreateKernel(cl->program, kernel, &err);
    if (__borCLErrorCheck(err, "Can't create kernel") != 0)
        return -1;

    cl->kernels_len += 1;
    cl->kernels = BOR_REALLOC_ARR(cl->kernels, cl_kernel, cl->kernels_len);
    cl->kernels[cl->kernels_len - 1] = k;

    return 0;
}


void borCLPrintPlatforms(FILE *out)
{
    cl_uint i, j, num_platforms, num_devices;
    cl_int err;
    cl_platform_id *platforms;
    cl_device_id *devices;
    char buf[1024];
    cl_bool buf_bool;
    cl_ulong buf_ulong;
    cl_uint buf_uint;
    size_t buf_size, *buf_size2, bi;

    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if (__borCLErrorCheck(err, "Can't get platforms IDs") != 0)
        return;

    if (num_platforms == 0){
        fprintf(out, "No platforms found.\n");
        return;
    }

    platforms = BOR_ALLOC_ARR(cl_platform_id, num_platforms);
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    if (__borCLErrorCheck(err, "Can't get platforms IDs") != 0)
        return;

    fprintf(out, "Available Platforms (%d):\n", (int)num_platforms);
    for (i = 0; i < num_platforms; i++){
        fprintf(out, "  platform [%d]:\n", i);
        fprintf(out, "      id:         %ld\n", (long)platforms[i]);
        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 1024, buf, NULL);
        if (err == CL_SUCCESS){
            fprintf(out, "      name:       %s\n", buf);
        }

        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, 1024, buf, NULL);
        if (err == CL_SUCCESS){
            fprintf(out, "      version:    %s\n", buf);
        }

        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 1024, buf, NULL);
        if (err == CL_SUCCESS){
            fprintf(out, "      vendor:     %s\n", buf);
        }

        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_EXTENSIONS, 1024, buf, NULL);
        if (err == CL_SUCCESS){
            fprintf(out, "      extensions: %s\n", buf);
        }

        err = clGetPlatformInfo(platforms[i], CL_PLATFORM_PROFILE, 1024, buf, NULL);
        if (err == CL_SUCCESS){
            fprintf(out, "      profile:    %s\n", buf);
        }

        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
        if (__borCLErrorCheck(err, "Can't get devices' IDs") == 0){
            devices = BOR_ALLOC_ARR(cl_device_id, num_devices);
            clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU,
                           num_devices, devices, NULL);

            for (j = 0; j < num_devices; j++){
                fprintf(out, "      Device [%d]:\n", (int)j);

                err = clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 1024, buf, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        name:       %s\n", buf);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_AVAILABLE,
                                      sizeof(buf_bool), &buf_bool, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        available:  %d\n", (int)buf_bool);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_COMPILER_AVAILABLE,
                                      sizeof(buf_bool), &buf_bool, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        compiler:   %d\n", (int)buf_bool);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE,
                                      sizeof(buf_ulong), &buf_ulong, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        global mem: %ld\n", (long)buf_ulong);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE,
                                      sizeof(buf_ulong), &buf_ulong, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        local mem:  %ld\n", (long)buf_ulong);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                                      sizeof(buf_size), &buf_size, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        max compute units: %d\n", (int)buf_size);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE,
                                      sizeof(buf_size), &buf_size, NULL);
                if (err == CL_SUCCESS){
                    fprintf(out, "        max group:  %d\n", (int)buf_size);
                }

                err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                                      sizeof(buf_uint), &buf_uint, NULL);
                if (err == CL_SUCCESS){
                    buf_size2 = BOR_ALLOC_ARR(size_t, buf_uint);
                    err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_SIZES,
                                           sizeof(size_t) * buf_uint, buf_size2, NULL);
                    fprintf(out, "        max item:   (%d", (int)buf_size2[0]);
                    for (bi = 1; bi < buf_uint; bi++){
                        fprintf(out, ", %d", (int)buf_size2[bi]);
                    }
                    fprintf(out, ")\n");
                    BOR_FREE(buf_size2);
                }
            }

            BOR_FREE(devices);
        }
    }

    BOR_FREE(platforms);
}

char *borCLProgramFromFile(const char *filename)
{
    char *out, *buf;
    size_t len, written;
    struct stat st;
    int fd;

    if (stat(filename, &st) != 0)
        return NULL;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
        return NULL;

    len = st.st_size;
    out = BOR_ALLOC_ARR(char, len + 1);
    buf = out;
    while ((written = read(fd, buf, len)) > 0){
        len -= written;
        buf += written;
    }

    out[st.st_size] = 0x0;

    return out;
}

int borCLKernelSetArg(bor_cl_t *cl, size_t kernel, size_t arg_idx,
                      size_t size, const void *arg)
{
    cl_int err;

    err = clSetKernelArg(cl->kernels[kernel], arg_idx, size, arg);
    if (__borCLErrorCheck(err, "Can't set kernel's arg") != 0)
        return -1;
    return 0;
}

int borCLKernelEnqueue(bor_cl_t *cl, size_t kernel,
                       size_t dim, const size_t *glob, const size_t *loc)
{
    cl_int err;

    //DBG("%u %u", glob[0], loc[0]);
    err = clEnqueueNDRangeKernel(cl->queue, cl->kernels[kernel],
                                 dim, NULL, glob, loc, 0, NULL, NULL);
    if (__borCLErrorCheck(err, "Can't enqueue kernel") != 0)
        return -1;
    return 0;
}

int borCLFinish(bor_cl_t *cl)
{
    cl_int err;
    
    err = clFinish(cl->queue);
    if (__borCLErrorCheck(err, "Can't finish queue") != 0)
        return -1;
    return 0;
}


void *borCLAlloc(bor_cl_t *cl, size_t size, void *src, cl_mem_flags _flags)
{
    void *dst;
    cl_int err;
    cl_mem_flags flags = _flags;

    if (src)
        flags |= CL_MEM_COPY_HOST_PTR;

    dst = clCreateBuffer(cl->context, flags, size, src, &err);
    if (err != CL_SUCCESS)
        return NULL;
    return dst;
}

void *borCLCloneToHost(bor_cl_t *cl, const void *src, size_t size)
{
    void *dst;
    cl_int err;

    dst = borRealloc(NULL, size);
    err = clEnqueueReadBuffer(cl->queue, (cl_mem)src, CL_TRUE, 0, size, dst, 0, NULL, NULL);
    if (__borCLErrorCheck(err, "Can't read device memory") != 0){
        BOR_FREE(dst);
        return NULL;
    }
    return dst;
}

void borCLCopyToHost(bor_cl_t *cl, const void *src, void *dst, size_t size)
{
    cl_int err;

    err = clEnqueueReadBuffer(cl->queue, (cl_mem)src, CL_TRUE, 0, size, dst, 0, NULL, NULL);
    __borCLErrorCheck(err, "Can't copy device memory to host memory");
}

void borCLCopyFromHost(bor_cl_t *cl, const void *src, void *dst, size_t size)
{
    cl_int err;

    err = clEnqueueWriteBuffer(cl->queue, dst, CL_TRUE, 0, size, src, 0, NULL, NULL);
    __borCLErrorCheck(err, "Can't copy device memory to host memory");
}

void *borCLMemMap(bor_cl_t *cl, void *src, size_t size, cl_mem_flags _flags)
{
    void *dst;
    cl_int err;
    cl_mem_flags flags = _flags;

    if (src)
        flags |= CL_MEM_USE_HOST_PTR;

    dst = clCreateBuffer(cl->context, flags, size, src, &err);
    if (__borCLErrorCheck(err, "Can't map host memory to device memory") != 0)
        return NULL;
    return dst;
}


const char *__borCLErrorStr(cl_int err)
{
    static const char* error_str[] = {
        "CL_SUCCESS",
        "CL_DEVICE_NOT_FOUND",
        "CL_DEVICE_NOT_AVAILABLE",
        "CL_COMPILER_NOT_AVAILABLE",
        "CL_MEM_OBJECT_ALLOCATION_FAILURE",
        "CL_OUT_OF_RESOURCES",
        "CL_OUT_OF_HOST_MEMORY",
        "CL_PROFILING_INFO_NOT_AVAILABLE",
        "CL_MEM_COPY_OVERLAP",
        "CL_IMAGE_FORMAT_MISMATCH",
        "CL_IMAGE_FORMAT_NOT_SUPPORTED",
        "CL_BUILD_PROGRAM_FAILURE",
        "CL_MAP_FAILURE",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "CL_INVALID_VALUE",
        "CL_INVALID_DEVICE_TYPE",
        "CL_INVALID_PLATFORM",
        "CL_INVALID_DEVICE",
        "CL_INVALID_CONTEXT",
        "CL_INVALID_QUEUE_PROPERTIES",
        "CL_INVALID_COMMAND_QUEUE",
        "CL_INVALID_HOST_PTR",
        "CL_INVALID_MEM_OBJECT",
        "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
        "CL_INVALID_IMAGE_SIZE",
        "CL_INVALID_SAMPLER",
        "CL_INVALID_BINARY",
        "CL_INVALID_BUILD_OPTIONS",
        "CL_INVALID_PROGRAM",
        "CL_INVALID_PROGRAM_EXECUTABLE",
        "CL_INVALID_KERNEL_NAME",
        "CL_INVALID_KERNEL_DEFINITION",
        "CL_INVALID_KERNEL",
        "CL_INVALID_ARG_INDEX",
        "CL_INVALID_ARG_VALUE",
        "CL_INVALID_ARG_SIZE",
        "CL_INVALID_KERNEL_ARGS",
        "CL_INVALID_WORK_DIMENSION",
        "CL_INVALID_WORK_GROUP_SIZE",
        "CL_INVALID_WORK_ITEM_SIZE",
        "CL_INVALID_GLOBAL_OFFSET",
        "CL_INVALID_EVENT_WAIT_LIST",
        "CL_INVALID_EVENT",
        "CL_INVALID_OPERATION",
        "CL_INVALID_GL_OBJECT",
        "CL_INVALID_BUFBOR_SIZE",
        "CL_INVALID_MIP_LEVEL",
        "CL_INVALID_GLOBAL_WORK_SIZE",
    };
    static const size_t count = sizeof(error_str) / sizeof(const char *);

    if (-err <= count){
        return error_str[-err];
    }
    return "UNKOWN ERROR";
}

_bor_inline int __borCLErrorCheck(cl_int err, const char *s)
{
    if (err != CL_SUCCESS){
        fprintf(stderr, "OpenCL Error (%s): %s\n", __borCLErrorStr(err), s);
        return -1;
    }
    return 0;
}
