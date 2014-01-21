/***
 * Boruvka
 * --------
 * Copyright (c)2014 Daniel Fiser <danfis@danfis.cz>
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

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "boruvka/alloc.h"
#include "boruvka/hdf5.h"


/** Set to true if reporting on stderr should be enabled */
static int __bor_h5_enable_error_reports = 1;


#define ERR(format, ...) \
    error(__func__, format, __VA_ARGS__)
#define ERR2(msg) \
    error2(__func__, msg)

/** Prints error to stderr */
static void error(const char *func, const char *format, ...)
{
    va_list ap;

    if (__bor_h5_enable_error_reports){
        va_start(ap, format);
        fprintf(stderr, "HDF5 Error[%s] :: ", func);
        vfprintf(stderr, format, ap);
        va_end(ap);
        fprintf(stderr, "\n");
        fflush(stderr);
    }
}
static void error2(const char *func, const char *msg)
{
    if (__bor_h5_enable_error_reports){
        fprintf(stderr, "HDF5 Error[%s] :: %s\n", func, msg);
        fflush(stderr);
    }
}

/** Returns true if file exists */
static int fileExist(const char *fn)
{
    struct stat st;
    if (stat(fn, &st) == 0 && S_ISREG(st.st_mode))
        return 1;
    return 0;
}


void borH5EnableErrorReports(int enable)
{
    __bor_h5_enable_error_reports = enable;
}

int borH5FileOpen(bor_h5file_t *hf, const char *fn, const char *mode)
{
    if (strcmp(mode, "r") == 0){
        hf->file_id = H5Fopen(fn, H5F_ACC_RDONLY, H5P_DEFAULT);
    }else if (strcmp(mode, "r+") == 0){
        hf->file_id = H5Fopen(fn, H5F_ACC_RDWR, H5P_DEFAULT);
    }else if (strcmp(mode, "w") == 0){
        hf->file_id = H5Fcreate(fn, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }else if (strcmp(mode, "w-") == 0){
        hf->file_id = H5Fcreate(fn, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
    }else if (strcmp(mode, "a") == 0){
        if (fileExist(fn)){
            hf->file_id = H5Fopen(fn, H5F_ACC_RDWR, H5P_DEFAULT);
        }else{
            hf->file_id = H5Fcreate(fn, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
        }
    }else{
        ERR("Unknown mode `%s'", mode);
        return -1;
    }

    if (hf->file_id < 0){
        ERR("Could not open file `%s' in mode `%s'", fn, mode);
        return -1;
    }

    // initialize list of datasets
    borListInit(&hf->dset);

    return 0;
}

int borH5FileClose(bor_h5file_t *hf)
{
    bor_h5dset_t *dset;

    // first free all datasets
    while (!borListEmpty(&hf->dset)){
        dset = BOR_LIST_ENTRY(borListNext(&hf->dset), bor_h5dset_t, list);
        if (borH5DatasetClose(dset) != 0){
            return -1;
        }
    }

    if (H5Fclose(hf->file_id) < 0){
        ERR2("Could not close HDF file.");
        return -1;
    }

    hf->file_id = -1;
    return 0;
}

bor_h5dset_t *borH5DatasetOpen(bor_h5file_t *hf, const char *path)
{
    hid_t dset_id;
    hid_t dspace_id;
    bor_h5dset_t *dset;
    int ndims;
    hsize_t *dims;
    int i;

    dset_id = H5Dopen2(hf->file_id, path, H5P_DEFAULT);
    if (dset_id < 0){
        ERR("Could not open dataset `%s'", path);
        return NULL;
    }

    // get dataspace of dataset
    dspace_id = H5Dget_space(dset_id);
    if (dspace_id < 0){
        ERR("Cannot obtain dataspace from dataset `%s'", path);
        return NULL;
    }

    // get number of dimensions
    ndims = H5Sget_simple_extent_ndims(dspace_id);
    if (ndims < 0){
        ERR("Cannot determine number of dimensions of dataset `%s'", path);
        return NULL;
    }

    // get dimensions
    dims = BOR_ALLOC_ARR(hsize_t, ndims);
    if (H5Sget_simple_extent_dims(dspace_id, dims, NULL) < 0){
        ERR("Cannot determine dimensions of dataset `%s'", path);
        BOR_FREE(dims);
        return NULL;
    }

    dset = BOR_ALLOC(bor_h5dset_t);
    dset->path    = strdup(path);
    dset->dset_id = dset_id;
    dset->hf      = hf;
    borListAppend(&hf->dset, &dset->list);

    // write dataspace info
    dset->ndims = ndims;
    dset->dims = BOR_ALLOC_ARR(size_t, ndims);
    for (i = 0; i < ndims; i++){
        dset->dims[i] = dims[i];
    }

    dset->num_elements = 1;
    for (i = 0; i < ndims; i++){
        dset->num_elements *= dims[i];
    }

    dset->data = NULL;
    dset->data_size = 0;

    BOR_FREE(dims);

    return dset;
}

int borH5DatasetClose(bor_h5dset_t *dset)
{
    if (H5Dclose(dset->dset_id) < 0){
        ERR("Could not close dataset `%s'", dset->path);
        return -1;

    }else{
        BOR_FREE(dset->path);
        BOR_FREE(dset->dims);

        if (dset->data)
            BOR_FREE(dset->data);

        borListDel(&dset->list);
        BOR_FREE(dset);
        return 0;
    }
}


static size_t loadAll(bor_h5dset_t *dset, hid_t type, size_t elsize)
{
    herr_t err;
    size_t size;

    // determine size of the array
    size = elsize * dset->num_elements;

    // (re)allocate the internal buffer
    if (dset->data_size < size)
        dset->data = _BOR_ALLOC_MEMORY(void, dset->data, size);

    // read data from dataset
    err = H5Dread(dset->dset_id, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset->data);
    if (err < 0){
        ERR("Could not read data from dataset `%s'", dset->path);
        return -1;
    }

    return dset->num_elements;
}

size_t borH5DatasetLoadFloat(bor_h5dset_t *dset, float **data)
{
    size_t size;

    size = loadAll(dset, H5T_NATIVE_FLOAT, sizeof(float));
    if (data)
        *data = (float *)dset->data;
    return size;
}

size_t borH5DatasetLoadDouble(bor_h5dset_t *dset, double **data)
{
    size_t size;

    size = loadAll(dset, H5T_NATIVE_DOUBLE, sizeof(double));
    if (data)
        *data = (double *)dset->data;
    return size;
}

size_t borH5DatasetLoadInt(bor_h5dset_t *dset, int **data)
{
    size_t size;

    size = loadAll(dset, H5T_NATIVE_INT, sizeof(int));
    if (data)
        *data = (int *)dset->data;
    return size;
}

size_t borH5DatasetLoadReal(bor_h5dset_t *dset, bor_real_t **data)
{
#ifdef BOR_SINGLE
    return borH5DatasetLoadFloat(dset, data);
#else /* BOR_SINGLE */
    return borH5DatasetLoadDouble(dset, data);
#endif /* BOR_SINGLE */
}
