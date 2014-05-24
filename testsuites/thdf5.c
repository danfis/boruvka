#include <cu/cu.h>
#include <boruvka/hdf5.h>
#include <boruvka/dbg.h>
#include <boruvka/alloc.h>

static bor_h5file_t hf;
static bor_h5dset_t *dset;

TEST(hdf5SetUp)
{
    // open data file
    assertEquals(borH5FileOpen(&hf, "hdf5.h5", "r"), 0);

    // open dataset
    dset = borH5DatasetOpen(&hf, "train/x");
    assertNotEquals(dset, NULL);

    assertEquals(dset->ndims, 2);
    assertEquals(dset->dims[0], 100);
    assertEquals(dset->dims[1], 40000);
}

TEST(hdf5TearDown)
{
    assertEquals(borH5FileClose(&hf), 0);
}

TEST(hdf5OpenR)
{
    size_t size;
    float *dataf;
    int *datai;
    double *datad;
    bor_real_t *datar;

    // get number of elements in dataset
    size = dset->num_elements;
    assertEquals(dset->num_elements, 4000000);

    dataf = borH5DatasetLoadFloat(dset, &size);
    assertEquals(size, dset->num_elements);
    assertTrue(borEq(dataf[0], 0.0078431377f));
    assertTrue(borEq(dataf[size - 1], 0.0117647061f));
    assertTrue(borEq(dataf[10], 0.0392156877));
    assertTrue(borEq(dataf[41000], 0.0156862754));

    datai = borH5DatasetLoadInt(dset, &size);
    assertEquals(dset->num_elements, size);
    assertEquals(datai[0], 0);
    assertEquals(datai[size - 1], 0);

    datad = borH5DatasetLoadDouble(dset, &size);
    assertEquals(size, dset->num_elements);
    assertTrue(borEq(datad[0], 0.0078431377f));
    assertTrue(borEq(datad[size - 1], 0.0117647061f));
    assertTrue(borEq(datad[10], 0.0392156877));
    assertTrue(borEq(datad[41000], 0.0156862754));

    datar = borH5DatasetLoadReal(dset, &size);
    assertEquals(size, dset->num_elements);
    assertTrue(borEq(datar[0], 0.0078431377f));
    assertTrue(borEq(datar[size - 1], 0.0117647061f));
    assertTrue(borEq(datar[10], 0.0392156877));
    assertTrue(borEq(datar[41000], 0.0156862754));
}

TEST(hdf5Vec)
{
    bor_gsl_vector *vec;

    vec = borH5DatasetLoadVec(dset);
    assertEquals(vec->size, 4000000);
    assertTrue(borEq(bor_gsl_vector_get(vec, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_vector_get(vec, 3999999), 0.0117647061f));
    assertTrue(borEq(bor_gsl_vector_get(vec, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_vector_get(vec, 41000), 0.0156862754));
}

TEST(hdf5Mat)
{
    bor_h5dset_t *dset;
    bor_gsl_matrix *mat;

    // try open identical dataset once more
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

    assertEquals(borH5DatasetClose(dset), 0);
}

TEST(hdf5MatRowRange)
{
    bor_gsl_matrix *mat;

    mat = borH5DatasetLoadMatRowRange(dset, 1, 3);
    assertEquals(mat->size1, 3);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 1000), 0.0156862754));

    mat = borH5DatasetLoadMatRowRange(dset, 0, 2);
    assertEquals(mat->size1, 2);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 0), 0.0078431377f));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 0, 10), 0.0392156877));
    assertTrue(borEq(bor_gsl_matrix_get(mat, 1, 1000), 0.0156862754));


    mat = borH5DatasetLoadMatRowRange(dset, 95, 5);
    assertEquals(mat->size1, 5);
    assertEquals(mat->size2, 40000);
    assertTrue(borEq(bor_gsl_matrix_get(mat, 4, 39999), 0.0117647061f));
}

TEST(hdf5Region)
{
    float *dataf;
    double *datad;
    int *datai;
    bor_real_t *datar;
    size_t start[2], count[2], size;

    start[0] = 0;
    start[1] = 10;
    count[0] = 2;
    count[1] = 1000;
    datar = borH5DatasetLoadRegionReal(dset, start, count, &size);
    assertEquals(size, 2000);
    assertTrue(borEq(datar[0], 0.0392156877));
    assertTrue(borEq(datar[1990], 0.0156862754));

    dataf = borH5DatasetLoadRegionFloat(dset, start, count, &size);
    assertEquals(size, 2000);
    assertTrue(borEq(dataf[0], 0.0392156877));
    assertTrue(borEq(dataf[1990], 0.0156862754));

    datad = borH5DatasetLoadRegionDouble(dset, start, count, &size);
    assertEquals(size, 2000);
    assertTrue(borEq(datad[0], 0.0392156877));
    assertTrue(borEq(datad[1990], 0.0156862754));

    datai = borH5DatasetLoadRegionInt(dset, start, count, &size);
    assertEquals(size, 2000);
    assertEquals(datai[0], 0);
    assertEquals(datai[1990], 0);
}

static int eqMat(bor_gsl_matrix *mat1, bor_gsl_matrix *mat2)
{
    size_t i, j;

    assertEquals(mat2->size1, mat1->size1);
    if (mat2->size1 != mat1->size1)
        return 0;

    assertEquals(mat2->size2, mat1->size2);
    if (mat2->size2 != mat1->size2)
        return 0;

    for (i = 0; i < mat1->size1; i++){
        for (j = 0; j < mat1->size2; j++){
            assertTrue(borEq(bor_gsl_matrix_get(mat1, i, j),
                             bor_gsl_matrix_get(mat2, i, j)));
            if (!borEq(bor_gsl_matrix_get(mat1, i, j),
                             bor_gsl_matrix_get(mat2, i, j)))
                return 0;
        }
    }

    return 1;
}

TEST(hdf5WriteMat)
{
    bor_h5file_t f;
    bor_gsl_matrix *mat, *mat2;
    bor_gsl_matrix_view mview;
    bor_h5dset_t *dset, *dset2;

    dset = borH5DatasetOpen(&hf, "/train/x");
    assertNotEquals(dset, NULL);
    mat = borH5DatasetLoadMat(dset);
    assertNotEquals(mat, NULL);

    borH5FileOpen(&f, "tmp.hdf5.h5", "w");

    // write dataset with simple path and check what is written
    assertEquals(borH5WriteMat(&f, "simple", mat), 0);

    dset2 = borH5DatasetOpen(&f, "simple");
    mat2 = borH5DatasetLoadMat(dset2);
    eqMat(mat, mat2);
    borH5DatasetClose(dset2);


    // write submatrix to simple path
    mview = bor_gsl_matrix_submatrix(mat, 0, 10, 2, 1000);
    assertEquals(borH5WriteMat(&f, "submatrix", &mview.matrix), 0);

    dset2 = borH5DatasetOpen(&f, "submatrix");
    mat2 = borH5DatasetLoadMat(dset2);
    eqMat(&mview.matrix, mat2);
    borH5DatasetClose(dset2);


    // write matrix in group(s)
    assertEquals(borH5WriteMat(&f, "g1/g2/g3/mat", mat), 0);

    dset2 = borH5DatasetOpen(&f, "g1/g2/g3/mat");
    mat2 = borH5DatasetLoadMat(dset2);
    eqMat(mat, mat2);
    borH5DatasetClose(dset2);


    mview = bor_gsl_matrix_submatrix(mat, 0, 10, 2, 1000);
    assertEquals(borH5WriteMat(&f, "/g1/g2/g4/submatrix", &mview.matrix), 0);

    dset2 = borH5DatasetOpen(&f, "/g1/g2/g4/submatrix");
    mat2 = borH5DatasetLoadMat(dset2);
    eqMat(&mview.matrix, mat2);
    borH5DatasetClose(dset2);


    mview = bor_gsl_matrix_submatrix(mat, 10, 100, 4, 2000);
    assertEquals(borH5WriteMat(&f, "/g1/g2/g4/submatrix2", &mview.matrix), 0);

    dset2 = borH5DatasetOpen(&f, "/g1/g2/g4/submatrix2");
    mat2 = borH5DatasetLoadMat(dset2);
    eqMat(&mview.matrix, mat2);
    borH5DatasetClose(dset2);

    borH5FileClose(&f);
}
