#ifndef TEST_HDF5_H
#define TEST_HDF5_H

TEST(hdf5SetUp);
TEST(hdf5TearDown);

TEST(hdf5OpenR);
TEST(hdf5Vec);
TEST(hdf5Mat);
TEST(hdf5MatRowRange);
TEST(hdf5Region);

TEST(hdf5WriteMat);

TEST_SUITE(TSHDF5) {
    TEST_ADD(hdf5SetUp),

    TEST_ADD(hdf5OpenR),
    TEST_ADD(hdf5Vec),
    TEST_ADD(hdf5Mat),
    TEST_ADD(hdf5MatRowRange),
    TEST_ADD(hdf5Region),

    TEST_ADD(hdf5WriteMat),

    TEST_ADD(hdf5TearDown),
    TEST_SUITE_CLOSURE
};

#endif

