#include <stdio.h>
#include <cu/cu.h>
#include <fermat/mat3.h>
#include <fermat/dbg.h>
#include "data.h"

TEST(mat3SetUp)
{
}

TEST(mat3TearDown)
{
}

TEST(mat3Alloc)
{
    /*
    fer_mat3_t *v, w;

    v = ferMat3New(0., 1., 2., 3.);
    ferMat3Set(&w, 0., 1., 2., 3.);
    assertTrue(ferMat3Eq(v, &w));
    ferMat3Del(v);
    */
}

static void pr(const fer_mat3_t *m, const char *p)
{
    printf("# %s %g %g %g %g %g %g %g %g %g\n",
           p,
           ferMat3Get(m, 0, 0), ferMat3Get(m, 0, 1), ferMat3Get(m, 0, 2), 
           ferMat3Get(m, 1, 0), ferMat3Get(m, 1, 1), ferMat3Get(m, 1, 2), 
           ferMat3Get(m, 2, 0), ferMat3Get(m, 2, 1), ferMat3Get(m, 2, 2));
}

TEST(mat3AddSub)
{
    size_t i;
    fer_mat3_t m;

    printf("# ---- add sub ----\n");
    for (i = 0; i < mat3s_len - 1; i++){
        ferMat3Add2(&m, &mat3s[i], &mat3s[i + 1]);
        pr(&m, "add");
        ferMat3Copy(&m, &mat3s[i]);
        ferMat3Add(&m, &mat3s[i + 1]);
        pr(&m, "add");

        ferMat3Sub2(&m, &mat3s[i], &mat3s[i + 1]);
        pr(&m, "sub");
        ferMat3Copy(&m, &mat3s[i]);
        ferMat3Sub(&m, &mat3s[i + 1]);
        pr(&m, "sub");
    }
    printf("# ---- add sub end ----\n\n");
}

TEST(mat3Const)
{
    size_t i;
    fer_mat3_t m;
    fer_real_t c;

    printf("# ---- const ----\n");
    for (i = 0; i < mat3s_len; i++){
        c = ferMat3Get(&mat3s[i], 0, 0);
        ferMat3Scale2(&m, &mat3s[i], c);
        pr(&m, "scale");

        ferMat3Copy(&m, &mat3s[i]);
        ferMat3Scale(&m, c);
        pr(&m, "scale");

        ferMat3AddConst2(&m, &mat3s[i], c);
        pr(&m, "add");
        ferMat3Copy(&m, &mat3s[i]);
        ferMat3AddConst(&m, c);
        pr(&m, "add");
    }
    printf("# ---- const end ----\n\n");
}

TEST(mat3Mul)
{
    size_t i;
    fer_mat3_t m;

    printf("# ---- mul ----\n");
    for (i = 0; i < mat3s_len - 1; i++){
        ferMat3Mul2(&m, &mat3s[i], &mat3s[i + 1]);
        pr(&m, "right");
        ferMat3Copy(&m, &mat3s[i]);
        ferMat3MulLeft(&m, &mat3s[i + 1]);
        pr(&m, "left");
    }
    printf("# ---- mul end ----\n\n");
}

TEST(mat3Trans)
{
    size_t i;
    fer_mat3_t m;

    printf("# ---- trans ----\n");
    for (i = 0; i < mat3s_len; i++){
        ferMat3Trans2(&m, &mat3s[i]);
        pr(&m, "");
        ferMat3Copy(&m, &mat3s[i]);
        ferMat3Trans(&m);
        pr(&m, "");
    }
    printf("# ---- trans end ----\n\n");
}

TEST(mat3Det)
{
    size_t i;
    fer_real_t d;

    printf("# ---- det ----\n");
    for (i = 0; i < mat3s_len; i++){
        d = ferMat3Det(&mat3s[i]);
        printf("# %g\n", d);
    }
    printf("# ---- det end ----\n\n");
}

TEST(mat3Inv)
{
    size_t i;
    fer_mat3_t m;

    printf("# ---- inv ----\n");
    for (i = 0; i < mat3s_len; i++){
        if (ferMat3Inv2(&m, &mat3s[i]) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }

        ferMat3Copy(&m, &mat3s[i]);
        if (ferMat3Inv(&m) == 0){
            pr(&m, "");
        }else{
            printf("# 0\n");
        }
    }
    printf("# ---- inv end ----\n\n");
}

TEST(mat3MulVec)
{
    size_t i;
    fer_vec3_t u, v;
    fer_vec2_t a, b;

    printf("# ---- mul vec ----\n");
    for (i = 0; i < mat3s_len; i++){
        ferVec3Set(&v, ferMat3Get(&mat3s[i], 0, 1),
                       ferMat3Get(&mat3s[i], 0, 2),
                       ferMat3Get(&mat3s[i], 1, 0));
        ferVec2Set(&b, ferMat3Get(&mat3s[i], 1, 0),
                       ferMat3Get(&mat3s[i], 1, 1));

        ferMat3MulVec(&u, &mat3s[i], &v);
        ferMat3MulVec2(&a, &mat3s[i], &b);

        printf("# vec3 %g %g %g\n",
               ferVec3X(&u), ferVec3Y(&u), ferVec3Z(&u));
        printf("# vec2 %g %g\n",
               ferVec2X(&a), ferVec2Y(&a));
    }
    printf("# ---- mul vec end ----\n\n");
}

