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

/**
 * Simplified interface to HDF5 binary data format.
 */

#ifndef __BOR_HDF5_H__
#define __BOR_HDF5_H__

#include <hdf5.h>

#include <boruvka/core.h>
#include <boruvka/list.h>

struct _bor_h5file_t;

struct _bor_h5dset_t {
    char *path;               /*!< Path to dataset */
    hid_t dset_id;            /*!< Identified of the dataset */
    struct _bor_h5file_t *hf; /*!< Back reference to HDF file */
    bor_list_t list;          /*!< Connector to the list of datasets */

    size_t ndims; /*!< Number of dimensions of the dataset (e.g., 1 for
                       vector, 2 for 2-D matrix, ...) */
    size_t *dims; /*!< Size of the dataset in each dimension (e.g., matrix
                       3x5 would have .ndims = 2 and .dims = {3, 5} */
    size_t num_elements; /*!< Overall number of elements in dataset */

    void *data;
    size_t data_size;
};
typedef struct _bor_h5dset_t bor_h5dset_t;

struct _bor_h5file_t {
    hid_t file_id;   /*!< Identifier of an opened HDF file */
    bor_list_t dset; /*!< List of datasets */
};
typedef struct _bor_h5file_t bor_h5file_t;


/**
 * Enables/Disables error reporting to stderr.
 */
void borH5EnableErrorReports(int enable);

/**
 * Opens a new file and initializes a given bor_h5file_t structure.
 * Valid modes are:
 *  "r"     Readonly, file must exist
 *  "r+"    Read/write, file must exist
 *  "w"     Create file, truncate if exists
 *  "w-"    Create file, fail if exists
 *  "a"     Read/write if exists, create otherwise (default)
 *
 * Returns 0 on success, -1 otherwise.
 */
int borH5FileOpen(bor_h5file_t *hf, const char *fn, const char *mode);

/**
 * Closes a previously opened HDF file and frees all allocated memory.
 * Note that also all datasets are invalidated (in contrast with the
 * original HDF5 API).
 */
int borH5FileClose(bor_h5file_t *hf);

/**
 * Returns an existing dataset from HDF file identified by a path.
 * Note that dims, ndism members of struct are filled with apropriate
 * values and you can use them directly.
 */
bor_h5dset_t *borH5DatasetOpen(bor_h5file_t *hf, const char *path);

/**
 * Close previously obtained dataset.
 */
int borH5DatasetClose(bor_h5dset_t *dset);

/**
 * Loads the *whole* data of the dataset into internal memory considering
 * conversion to the float data type (in this case, for other types see
 * similar functions).
 * The function returns number of elements loaded and if data argument is
 * non-NULL the pointer to data array is also returned.
 * Note that data are stored internaly (see dset->data*) and although you
 * can read/write as you wish you should not free the data block.
 */
size_t borH5DatasetLoadFloat(bor_h5dset_t *dset, float **data);

/**
 * Simliar to borH5DatasetLoadFloat() but double type is used for
 * conversion.
 */
size_t borH5DatasetLoadDouble(bor_h5dset_t *dset, double **data);

/**
 * Simliar to borH5DatasetLoadFloat() but int type is used.
 */
size_t borH5DatasetLoadInt(bor_h5dset_t *dset, int **data);

#endif /* __BOR_HDF5_H__ */
