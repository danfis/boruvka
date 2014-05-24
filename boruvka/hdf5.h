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

#include <boruvka/core.h>

#ifndef BOR_HDF5
# error "Boruvka is not compiled with HDF5 support!"
#endif /* BOR_HDF5 */

#include <hdf5.h>

#include <boruvka/list.h>

#ifdef BOR_GSL
# include <boruvka/gsl.h>
#endif /* BOR_GSL */

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

    void *data;       /*!< Internal storage for loaded data */
    size_t data_size; /*!< Size of .data array */


#ifdef BOR_GSL
    union {
        bor_gsl_vector_view vec; /*!< Vector view to .data */
        bor_gsl_matrix_view mat; /*!< Matrix view to .data */
    } gsl;
#endif /* BOR_GSL */
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
 * The function returns pointer to the data array and the size of the array
 * (via {size} arguments).
 * Note that data are stored internaly (see dset->data*) and although you
 * can read/write as you wish you should not free the data block.
 */
float *borH5DatasetLoadFloat(bor_h5dset_t *dset, size_t *size);

/**
 * Simliar to borH5DatasetLoadFloat() but double type is used for
 * conversion.
 */
double *borH5DatasetLoadDouble(bor_h5dset_t *dset, size_t *size);

/**
 * Simliar to borH5DatasetLoadFloat() but int type is used.
 */
int *borH5DatasetLoadInt(bor_h5dset_t *dset, size_t *size);

/**
 * Similar to borH5DatasetLoadFloat() but configured data type (float or
 * double) is used.
 */
bor_real_t *borH5DatasetLoadReal(bor_h5dset_t *dset, size_t *size);

/**
 * Loads (hyper-)rectangular region from the dataset into internal data
 * array. The region is defined by a pair {start} and {count} both must
 * have dset->ndims elements. {start} defines start of the region and
 * {count} defines number of desired elements in each dimension.
 * Data are converted to the float data type and for the returned data
 * block holds same rules as in case of borH5DatasetLoadFloat() function
 * (in fact the very same data array is used).
 */
float *borH5DatasetLoadRegionFloat(bor_h5dset_t *dset,
                                   const size_t *start,
                                   const size_t *count,
                                   size_t *size);

/**
 * Same as borH5DatasetLoadRegionFloat() but in doubles.
 */
double *borH5DatasetLoadRegionDouble(bor_h5dset_t *dset,
                                     const size_t *start,
                                     const size_t *count,
                                     size_t *size);

/**
 * Same as borH5DatasetLoadRegionFloat() but int type is used.
 */
int *borH5DatasetLoadRegionInt(bor_h5dset_t *dset,
                               const size_t *start,
                               const size_t *count,
                               size_t *size);

/**
 * Same as borH5DatasetLoadRegionFloat() but pre-configured floating point
 * type is used (bor_real_t).
 */
bor_real_t *borH5DatasetLoadRegionReal(bor_h5dset_t *dset,
                                       const size_t *start,
                                       const size_t *count,
                                       size_t *size);

#ifdef BOR_GSL

/**
 * Loads *whole* dataset into gsl vector.
 * Note: The returned vector is managed internally. *Do not free it*.
 */
bor_gsl_vector *borH5DatasetLoadVec(bor_h5dset_t *dset);

/**
 * Loads *whole* dataset into gsl matrix. The dataset's dimensions are use
 * for matrix which means that dataset must have .ndims == 2.
 * Note: The returned matrix in managed internally.
 */
bor_gsl_matrix *borH5DatasetLoadMat(bor_h5dset_t *dset);

/**
 * Loads and returns in form of GSL matrix a specified range of rows.
 * It is assumed that the corresponding HDF data are 2-D (dset->ndims == 2).
 * The row range is defined by starting row and number of rows that
 * follows.
 */
bor_gsl_matrix *borH5DatasetLoadMatRowRange(bor_h5dset_t *dset,
                                            size_t start, size_t num);

/**
 * Writes a given matrix to the dataset specified by path.
 * Return 0 on success.
 */
int borH5WriteMat(bor_h5file_t *file, const char *path,
                  const bor_gsl_matrix *mat);

#endif /* BOR_GSL */

#endif /* __BOR_HDF5_H__ */
