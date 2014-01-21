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
    hid_t dset_id;            /*!< Identified of the dataset */
    struct _bor_h5file_t *hf; /*!< Back reference to HDF file */
    bor_list_t list;          /*!< Connector to the list of datasets */
};
typedef struct _bor_h5dset_t bor_h5dset_t;

struct _bor_h5file_t {
    hid_t file_id;   /*!< Identifier of an opened HDF file */
    bor_list_t dset; /*!< List of datasets */
};
typedef struct _bor_h5file_t bor_h5file_t;

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
 */
bor_h5dset_t *borH5DatasetOpen(bor_h5file_t *hf, const char *path);

/**
 * Close previously obtained dataset.
 */
int borH5DatasetClose(bor_h5dset_t *dset);

/**
 * Returns number of elements stored in dataset.
 */
size_t borH5DatasetNumElements(bor_h5dset_t *dset);

/**
 * The entire dataset is stored in provided buffer.
 * All data elements are converted to float.
 * The provided data array must be large enough -- it is callers
 * responsibility to know how much memory must be allocated
 * (see borH5DatasetNumElements()).
 *
 * Returns 0 on success.
 */
int borH5DatasetReadFloat(bor_h5dset_t *dset, float *data);

/**
 * Similar to borH5DatasetReadFloat() but data are converted to double.
 */
int borH5DatasetReadDouble(bor_h5dset_t *dset, float *data);

/**
 * Simliar to borH5DatasetReadFloat() but data are converted to int.
 */
int borH5DatasetReadInt(bor_h5dset_t *dset, int *data);

#endif /* __BOR_HDF5_H__ */
