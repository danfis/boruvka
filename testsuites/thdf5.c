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
    fprintf(stderr, "%.10f %.10f\n", dataf[0], dataf[size - 1]);

    assertEquals(borH5DatasetLoadInt(dset, &datai), size);
    assertEquals(datai[0], 0);
    assertEquals(datai[size - 1], 0);
    fprintf(stderr, "%d %d\n", datai[0], datai[size - 1]);

    borH5FileClose(&hf);
}

