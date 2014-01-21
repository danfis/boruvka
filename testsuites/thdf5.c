#include <cu/cu.h>
#include <boruvka/hdf5.h>
#include <boruvka/dbg.h>
#include <boruvka/alloc.h>

TEST(hdf5OpenR)
{
    bor_h5file_t hf;
    bor_h5dset_t *dset;
    size_t size;
    float *dataf;
    int *datai;

    // open data file
    assertEquals(borH5FileOpen(&hf, "hdf5.h5", "r"), 0);

    // open dataset
    dset = borH5DatasetOpen(&hf, "train/x");
    assertNotEquals(dset, NULL);

    assertEquals(dset->ndims, 2);
    assertEquals(dset->dims[0], 100);
    assertEquals(dset->dims[1], 40000);

    // get number of elements in dataset
    size = dset->num_elements;
    assertEquals(dset->num_elements, 4000000);

    assertEquals(borH5DatasetLoadFloat(dset, &dataf), size);
    assertTrue(borEq(dataf[0], 0.0078431377f));
    assertTrue(borEq(dataf[size - 1], 0.0117647061f));
    assertTrue(borEq(dataf[10], 0.0392156877));
    assertTrue(borEq(dataf[41000], 0.0156862754));

    assertEquals(borH5DatasetLoadInt(dset, &datai), size);
    assertEquals(datai[0], 0);
    assertEquals(datai[size - 1], 0);

    borH5FileClose(&hf);
}

TEST(hdf5Vec)
{
    bor_h5file_t hf;
    bor_h5dset_t *dset;
    bor_gsl_vector *vec;

    // open data file
    assertEquals(borH5FileOpen(&hf, "hdf5.h5", "r"), 0);

    // open dataset
    dset = borH5DatasetOpen(&hf, "train/x");
    assertNotEquals(dset, NULL);

    assertEquals(dset->ndims, 2);
    assertEquals(dset->dims[0], 100);
    assertEquals(dset->dims[1], 40000);

    vec = borH5DatasetLoadVec(dset);
    assertEquals(vec->size, 4000000);
    assertTrue(borEq(bor_gsl_vector_get(vec, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_vector_get(vec, 3999999), 0.0117647061f));
    assertTrue(borEq(bor_gsl_vector_get(vec, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_vector_get(vec, 41000), 0.0156862754));

    borH5DatasetClose(dset);
    borH5FileClose(&hf);
}

TEST(hdf5Mat)
{
    bor_h5file_t hf;
    bor_h5dset_t *dset;
    bor_gsl_matrix *mat;

    // open data file
    assertEquals(borH5FileOpen(&hf, "hdf5.h5", "r"), 0);

    // open dataset
    dset = borH5DatasetOpen(&hf, "train/x");
    assertNotEquals(dset, NULL);

    assertEquals(dset->ndims, 2);
    assertEquals(dset->dims[0], 100);
    assertEquals(dset->dims[1], 40000);

    mat = borH5DatasetLoadMat(dset);
    assertEquals(mat->size1, 100);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 99, 39999), 0.0117647061f));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 1, 1000), 0.0156862754));

    borH5FileClose(&hf);
}
