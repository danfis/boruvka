#ifndef TEST_HDF5_H
#define TEST_HDF5_H

TEST(hdf5OpenR);

TEST_SUITE(TSHDF5) {
    TEST_ADD(hdf5OpenR),

    TEST_SUITE_CLOSURE
};

#endif

