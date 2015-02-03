#ifndef TEST_HFUNC_H
#define TEST_HFUNC_H

TEST(hfuncFnv);
TEST(hfuncMurmur3);
TEST(hfuncCityHash);
TEST(hfuncFastHash);

TEST_SUITE(TSHFunc) {
    TEST_ADD(hfuncFnv),
    TEST_ADD(hfuncMurmur3),
    TEST_ADD(hfuncCityHash),
    TEST_ADD(hfuncFastHash),
    TEST_SUITE_CLOSURE
};

#endif /* TEST_HFUNC_H */
