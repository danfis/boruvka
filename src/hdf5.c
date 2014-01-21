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


static size_t load(bor_h5dset_t *dset, hid_t type,
                   size_t elsize, size_t num_els,
                   hid_t memspace, hid_t filespace)
{
    herr_t err;
    size_t size;

    // determine size of the array
    size = elsize * num_els;

    // (re)allocate the internal buffer
    if (dset->data_size < size){
        dset->data      = _BOR_ALLOC_MEMORY(void, dset->data, size);
        dset->data_size = size;
    }

    // read data from dataset
    err = H5Dread(dset->dset_id, type, memspace, filespace, H5P_DEFAULT, dset->data);
    if (err < 0){
        ERR("Could not read data from dataset `%s'", dset->path);
        return -1;
    }

    return num_els;
}

static size_t loadAll(bor_h5dset_t *dset, hid_t type, size_t elsize)
{
    return load(dset, type, elsize, dset->num_elements, H5S_ALL, H5S_ALL);
}

static size_t loadRegion(bor_h5dset_t *dset, hid_t type, size_t elsize,
                         const size_t *start, const size_t *count)
{
    hid_t filespace, memspace;
    herr_t err;
    size_t size, i;
    hsize_t *hstart, *hcount, hsize;

    // copy dataspace from dataset
    filespace = H5Dget_space(dset->dset_id);

    // convert to HDF acceptable types
    size = 1;
    hstart = BOR_ALLOC_ARR(hsize_t, dset->ndims);
    hcount = BOR_ALLOC_ARR(hsize_t, dset->ndims);
    for (i = 0; i < dset->ndims; i++){
        hstart[i] = start[i];
        hcount[i] = count[i];
        size *= count[i];
    }

    // define hyperslab in file dataset
    err = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, hstart, NULL, hcount, NULL);
    if (err < 0){
        BOR_FREE(hstart);
        BOR_FREE(hcount);
        ERR("Could not select hyperslab in dataset `%s'", dset->path);
        return 0;
    }

    // create memspace as 1-D array
    hsize = size;
    memspace = H5Screate_simple(1, &hsize, &hsize);
    if (memspace < 0){
        BOR_FREE(hstart);
        BOR_FREE(hcount);
        ERR("Could not create a memory selection for dataset `%s'", dset->path);
        return 0;
    }

    BOR_FREE(hstart);
    BOR_FREE(hcount);

    size = load(dset, type, elsize, size, memspace, filespace);

    H5Sclose(memspace);
    H5Sclose(filespace);

    return size;
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



size_t borH5DatasetLoadRegionFloat(bor_h5dset_t *dset,
                                   const size_t *start,
                                   const size_t *count,
                                   float **data)
{
    size_t size;

    size = loadRegion(dset, H5T_NATIVE_FLOAT, sizeof(float), start, count);
    if (data)
        *data = (float *)dset->data;
    return size;
}

size_t borH5DatasetLoadRegionDouble(bor_h5dset_t *dset,
                                    const size_t *start,
                                    const size_t *count,
                                    double **data)
{
    size_t size;

    size = loadRegion(dset, H5T_NATIVE_DOUBLE, sizeof(double), start, count);
    if (data)
        *data = (double *)dset->data;
    return size;
}

size_t borH5DatasetLoadRegionInt(bor_h5dset_t *dset,
                                 const size_t *start,
                                 const size_t *count,
                                 int **data)
{
    size_t size;

    size = loadRegion(dset, H5T_NATIVE_INT, sizeof(int), start, count);
    if (data)
        *data = (int *)dset->data;
    return size;
}

size_t borH5DatasetLoadRegionReal(bor_h5dset_t *dset,
                                  const size_t *start,
                                  const size_t *count,
                                  bor_real_t **data)
{
#ifdef BOR_SINGLE
    return borH5DatasetLoadRegionFloat(dset, start, count, data);
#else /* BOR_SINGLE */
    return borH5DatasetLoadRegionDouble(dset, start, count, data);
#endif /* BOR_SINGLE */
}



#ifdef BOR_GSL

bor_gsl_vector *borH5DatasetLoadVec(bor_h5dset_t *dset)
{
    borH5DatasetLoadReal(dset, NULL);
    dset->gsl.vec = bor_gsl_vector_view_array((bor_real_t *)dset->data, dset->num_elements);
    return &dset->gsl.vec.vector;
}

bor_gsl_matrix *borH5DatasetLoadMat(bor_h5dset_t *dset)
{
    if (dset->ndims != 2)
        return NULL;

    borH5DatasetLoadReal(dset, NULL);
    dset->gsl.mat = bor_gsl_matrix_view_array((bor_real_t *)dset->data,
                                              dset->dims[0], dset->dims[1]);
    return &dset->gsl.mat.matrix;
}

bor_gsl_matrix *borH5DatasetLoadMatRowRange(bor_h5dset_t *dset,
                                            size_t start, size_t num)
{
    size_t dstart[2], dcount[2];

    if (dset->ndims != 2)
        return NULL;

    dstart[0] = start;
    dstart[1] = 0;
    dcount[0] = num;
    dcount[1] = dset->dims[1];
    borH5DatasetLoadRegionReal(dset, dstart, dcount, NULL);
    dset->gsl.mat = bor_gsl_matrix_view_array((bor_real_t *)dset->data,
                                              num, dset->dims[1]);
    return &dset->gsl.mat.matrix;
}

#endif /* BOR_GSL */
