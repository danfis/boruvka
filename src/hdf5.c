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

#ifdef BOR_SINGLE
# define H5T_NATIVE_REAL H5T_NATIVE_FLOAT
#else /* BOR_SINGLE */
# define H5T_NATIVE_REAL H5T_NATIVE_DOUBLE;
#endif /* BOR_SINGLE */

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
        H5Sclose(dspace_id);
        ERR("Cannot determine number of dimensions of dataset `%s'", path);
        return NULL;
    }

    // get dimensions
    dims = BOR_ALLOC_ARR(hsize_t, ndims);
    if (H5Sget_simple_extent_dims(dspace_id, dims, NULL) < 0){
        H5Dclose(dspace_id);
        ERR("Cannot determine dimensions of dataset `%s'", path);
        BOR_FREE(dims);
        return NULL;
    }

    // close dataspace, we don't need it anymore
    H5Sclose(dspace_id);

    dset = BOR_ALLOC(bor_h5dset_t);
    dset->path    = BOR_STRDUP(path);
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
        H5Sclose(filespace);
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


float *borH5DatasetLoadFloat(bor_h5dset_t *dset, size_t *size)
{
    *size = loadAll(dset, H5T_NATIVE_FLOAT, sizeof(float));
    if (*size == 0)
        return NULL;
    return (float *)dset->data;
}

double *borH5DatasetLoadDouble(bor_h5dset_t *dset, size_t *size)
{
    *size = loadAll(dset, H5T_NATIVE_DOUBLE, sizeof(double));
    if (*size == 0)
        return NULL;
    return (double *)dset->data;
}

int *borH5DatasetLoadInt(bor_h5dset_t *dset, size_t *size)
{
    *size = loadAll(dset, H5T_NATIVE_INT, sizeof(float));
    if (*size == 0)
        return NULL;
    return (int *)dset->data;
}

bor_real_t *borH5DatasetLoadReal(bor_h5dset_t *dset, size_t *size)
{
#ifdef BOR_SINGLE
    return borH5DatasetLoadFloat(dset, size);
#else /* BOR_SINGLE */
    return borH5DatasetLoadDouble(dset, size);
#endif /* BOR_SINGLE */
}



float *borH5DatasetLoadRegionFloat(bor_h5dset_t *dset,
                                   const size_t *start,
                                   const size_t *count,
                                   size_t *size)
{
    *size = loadRegion(dset, H5T_NATIVE_FLOAT, sizeof(float), start, count);
    if (*size == 0)
        return NULL;
    return (float *)dset->data;
}

double *borH5DatasetLoadRegionDouble(bor_h5dset_t *dset,
                                     const size_t *start,
                                     const size_t *count,
                                     size_t *size)
{
    *size = loadRegion(dset, H5T_NATIVE_DOUBLE, sizeof(double), start, count);
    if (*size == 0)
        return NULL;
    return (double *)dset->data;
}

int *borH5DatasetLoadRegionInt(bor_h5dset_t *dset,
                               const size_t *start,
                               const size_t *count,
                               size_t *size)
{
    *size = loadRegion(dset, H5T_NATIVE_INT, sizeof(int), start, count);
    if (*size == 0)
        return NULL;
    return (int *)dset->data;
}

bor_real_t *borH5DatasetLoadRegionReal(bor_h5dset_t *dset,
                                       const size_t *start,
                                       const size_t *count,
                                       size_t *size)
{
#ifdef BOR_SINGLE
    return borH5DatasetLoadRegionFloat(dset, start, count, size);
#else /* BOR_SINGLE */
    return borH5DatasetLoadRegionDouble(dset, start, count, size);
#endif /* BOR_SINGLE */
}



#ifdef BOR_GSL

bor_gsl_vector *borH5DatasetLoadVec(bor_h5dset_t *dset)
{
    size_t size;

    borH5DatasetLoadReal(dset, &size);
    dset->gsl.vec = bor_gsl_vector_view_array((bor_real_t *)dset->data, size);
    return &dset->gsl.vec.vector;
}

bor_gsl_matrix *borH5DatasetLoadMat(bor_h5dset_t *dset)
{
    size_t size;

    if (dset->ndims != 2)
        return NULL;

    borH5DatasetLoadReal(dset, &size);
    dset->gsl.mat = bor_gsl_matrix_view_array((bor_real_t *)dset->data,
                                              dset->dims[0], dset->dims[1]);
    return &dset->gsl.mat.matrix;
}

bor_gsl_matrix *borH5DatasetLoadMatRowRange(bor_h5dset_t *dset,
                                            size_t start, size_t num)
{
    size_t dstart[2], dcount[2], size;

    if (dset->ndims != 2)
        return NULL;

    dstart[0] = start;
    dstart[1] = 0;
    dcount[0] = num;
    dcount[1] = dset->dims[1];
    borH5DatasetLoadRegionReal(dset, dstart, dcount, &size);
    dset->gsl.mat = bor_gsl_matrix_view_array((bor_real_t *)dset->data,
                                              num, dset->dims[1]);
    return &dset->gsl.mat.matrix;
}

/** Creates a simple 2-D dataspace */
static hid_t createSpace2D(hsize_t dim1, hsize_t dim2)
{
    hsize_t dims[2] = {dim1, dim2};
    hid_t id;

    id = H5Screate_simple(2, dims, NULL);
    if (id < 0){
        ERR2("Could not create a 2-D dataspace.");
    }

    return id;
}

/** Selects a simple hyperslab in specified dataspace. No stride or block
 *  features are used. */
static int selectHyperslab2D(hid_t space_id, hsize_t start1, hsize_t start2,
                                             hsize_t cnt1, hsize_t cnt2)
{
    herr_t err;
    hsize_t start[2] = {start1, start2};
    hsize_t cnt[2] = {cnt1, cnt2};

    err = H5Sselect_hyperslab(space_id, H5S_SELECT_SET, start, NULL, cnt, NULL);
    if (err < 0){
        ERR2("Could not select 2-D hyperslab.");
        return -1;
    }
    return 0;
}

static int groupExists(hid_t file_id, const char *path)
{
    H5E_auto2_t func;
    void *data;
    hid_t gid;

    // save current state
    H5Eget_auto2(H5E_DEFAULT, &func, &data);
    // reset auto error reporting
    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);

    gid = H5Gopen2(file_id, path, H5P_DEFAULT);
    if (gid < 0){
        H5Eset_auto2(H5E_DEFAULT, func, data);
        return 0;
    }

    H5Eset_auto2(H5E_DEFAULT, func, data);
    H5Gclose(gid);

    return 1;
}

static int createGroupPathToDataset(hid_t file_id, const char *path)
{
    hid_t gid;
    hid_t prop;
    herr_t err;
    int len;
    char *gpath;

    // first find last '/' in path
    for (len = strlen(path) - 1; len >= 0 && path[len] != '/'; --len);

    // if there is no '/' the path is path to dataset than there are no
    // groups
    if (len < 0)
        return 0;


    // create a group path
    gpath = BOR_ALLOC_ARR(char, len + 1);
    strncpy(gpath, path, len);
    gpath[len] = 0;

    // check whether group exists or not
    if (groupExists(file_id, gpath)){
        BOR_FREE(gpath);
        return 0;
    }

    // force creation of intermediate groups in path
    prop = H5Pcreate(H5P_LINK_CREATE);
    if (prop < 0){
        ERR2("Could not create group-create property.");
        BOR_FREE(gpath);
        return -1;
    }

    err = H5Pset_create_intermediate_group(prop, 1);
    if (err < 0){
        ERR2("Could not set intermediate group creation property.");
        H5Pclose(prop);
        BOR_FREE(gpath);
        return -1;
    }

    // create the group along with intermediate groups
    gid = H5Gcreate2(file_id, gpath, prop, H5P_DEFAULT, H5P_DEFAULT);
    if (gid < 0){
        ERR("Could not create group `%s'", gpath);
        H5Pclose(prop);
        BOR_FREE(gpath);
        return -1;
    }

    BOR_FREE(gpath);
    H5Pclose(prop);
    H5Gclose(gid);

    return 0;
}

int borH5WriteMat(bor_h5file_t *file, const char *path,
                  const bor_gsl_matrix *mat)
{
    hid_t dspace_id, memspace_id;
    hid_t dset_id;
    herr_t err;

    if (createGroupPathToDataset(file->file_id, path) != 0)
        return -1;

    // create dataspace with same dimensions as the matrix
    dspace_id = createSpace2D(mat->size1, mat->size2);
    if (dspace_id < 0)
        return -1;

    // set up memspace
    if (mat->tda == mat->size2){
        memspace_id = H5S_ALL;

    }else{
        // This means that the matrix is in fact submatrix.
        // So, we create a dataspace that covers the whole matrix (in fact
        // even more than that because there is offset within a matrix).
        // Then we set up hyperslab to cover only the "upper right" region
        // of this big matrix. For more info see GSL manual regarding
        // definition of the matrix structure.

        memspace_id = createSpace2D(mat->size1, mat->tda);
        if (memspace_id < 0){
            H5Sclose(dspace_id);
            return -1;
        }

        if (selectHyperslab2D(memspace_id, 0, 0, mat->size1, mat->size2) != 0){
            H5Sclose(dspace_id);
            H5Sclose(memspace_id);
            return -1;
        }
    }

    // create dataset
    dset_id = H5Dcreate2(file->file_id, path, H5T_NATIVE_REAL, dspace_id,
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dset_id < 0){
        H5Sclose(dspace_id);
        if (memspace_id != H5S_ALL)
            H5Sclose(memspace_id);
        ERR("Could not create a dataset `%s'", path);
        return -1;
    }

    // write data to dataset
    err = H5Dwrite(dset_id, H5T_NATIVE_REAL, memspace_id, H5S_ALL, H5P_DEFAULT, mat->data);
    if (err < 0){
        H5Sclose(dspace_id);
        if (memspace_id != H5S_ALL)
            H5Sclose(memspace_id);
        H5Dclose(dset_id);
        ERR("Could not write data to dataset `%s'", path);
        return -1;
    }

    H5Dclose(dset_id);
    H5Sclose(dspace_id);
    if (memspace_id != H5S_ALL)
        H5Sclose(memspace_id);

    return 0;
}

#endif /* BOR_GSL */
