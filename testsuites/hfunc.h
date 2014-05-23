#ifndef TEST_HFUNC_H
#define TEST_HFUNC_H

TEST(hfuncFnv);

TEST_SUITE(TSHFunc) {
    TEST_ADD(hfuncFnv),
    TEST_SUITE_CLOSURE
};

#endif /* TEST_HFUNC_H */
