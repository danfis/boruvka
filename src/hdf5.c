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

#include "boruvka/alloc.h"
#include "boruvka/hdf5.h"

/** Returns true if file exists */
static int fileExist(const char *fn)
{
    struct stat st;
    if (stat(fn, &st) == 0 && S_ISREG(st.st_mode))
        return 1;
    return 0;
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
    }

    if (hf->file_id < 0){
        // TODO: Error report
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
            // TODO: Error report
            return -1;
        }
    }

    if (H5Fclose(hf->file_id) < 0){
        // TODO: Error report
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
        // TODO: Error report
        return NULL;
    }

    // get dataspace of dataset
    dspace_id = H5Dget_space(dset_id);
    if (dspace_id < 0){
        // TODO: Error report
        return NULL;
    }

    // get number of dimensions
    ndims = H5Sget_simple_extent_ndims(dspace_id);
    if (ndims < 0){
        // TODO: Error report
        return NULL;
    }

    // get dimensions
    dims = BOR_ALLOC_ARR(hsize_t, ndims);
    if (H5Sget_simple_extent_dims(dspace_id, dims, NULL) < 0){
        // TODO: Error report
        BOR_FREE(dims);
        return NULL;
    }

    dset = BOR_ALLOC(bor_h5dset_t);
    dset->dset_id = dset_id;
    dset->hf      = hf;
    borListAppend(&hf->dset, &dset->list);

    // write dataspace info
    dset->ndims = ndims;
    dset->dims = BOR_ALLOC_ARR(size_t, ndims);
    for (i = 0; i < ndims; i++){
        dset->dims[i] = dims[i];
    }

    BOR_FREE(dims);

    return dset;
}

int borH5DatasetClose(bor_h5dset_t *dset)
{
    if (H5Dclose(dset->dset_id) < 0){
        // TODO: Error report
        return -1;

    }else{
        BOR_FREE(dset->dims);
        borListDel(&dset->list);
        BOR_FREE(dset);
        return 0;
    }
}

size_t borH5DatasetNumElements(bor_h5dset_t *dset)
{
    hid_t dspace;
    hssize_t num_els;

    // open dataspace
    dspace = H5Dget_space(dset->dset_id);
    if (dspace < 0){
        // TODO: Error report
        return 0;
    }

    // determine number of elements in dataspace
    num_els = H5Sget_simple_extent_npoints(dspace);

    // close dataspace
    H5Sclose(dspace);

    return num_els;
}

static int datasetRead(bor_h5dset_t *dset, hid_t type, void *data)
{
    herr_t err;

    err = H5Dread(dset->dset_id, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    if (err < 0){
        // TODO: Error report
        return -1;
    }

    return 0;
}

int borH5DatasetReadFloat(bor_h5dset_t *dset, float *data)
{
    return datasetRead(dset, H5T_NATIVE_FLOAT, data);
}

int borH5DatasetReadDouble(bor_h5dset_t *dset, float *data)
{
    return datasetRead(dset, H5T_NATIVE_DOUBLE, data);
}

int borH5DatasetReadInt(bor_h5dset_t *dset, int *data)
{
    return datasetRead(dset, H5T_NATIVE_INT, data);
}
