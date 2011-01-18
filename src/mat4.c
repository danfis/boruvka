#include "fermat/mat4.h"
#include "fermat/mat3.h"
#include "fermat/alloc.h"
#include "fermat/dbg.h"


/** Returns subdeterminant */
static fer_real_t ferMat4Subdet(const fer_mat4_t *m, size_t i, size_t j);


static FER_MAT4(__fer_mat4_identity, FER_ONE, FER_ZERO, FER_ZERO, FER_ZERO,
                                     FER_ZERO, FER_ONE, FER_ZERO, FER_ZERO,
                                     FER_ZERO, FER_ZERO, FER_ONE, FER_ZERO,
                                     FER_ZERO, FER_ZERO, FER_ZERO, FER_ONE);
const fer_mat4_t *fer_mat4_identity = &__fer_mat4_identity;

static FER_MAT4(__fer_mat4_zero, FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO, FER_ZERO);
const fer_mat4_t *fer_mat4_zero = &__fer_mat4_zero;

fer_mat4_t *ferMat4New(void)
{
    fer_mat4_t *m;

    m = FER_ALLOC(fer_mat4_t);

    return m;
}

void ferMat4Del(fer_mat4_t *m)
{
    free(m);
}

int ferMat4Inv2(fer_mat4_t *m, const fer_mat4_t *a)
{
    fer_real_t det, invdet;
    int sign;
    size_t i, j;

    det = ferMat4Det(a);
    if (ferIsZero(det))
        return -1;

    invdet = ferRecp(det);

    for (i = 0; i < 4; i++){
        for (j = 0; j < 4; j++){
            det  = ferMat4Subdet(a, i, j);
            sign = 1 - ((i + j) % 2) * 2;
            ferMat4Set1(m, j, i, sign * det * invdet);
        }
    }

    return 0;
}


static fer_real_t ferMat4Subdet(const fer_mat4_t *m, size_t i, size_t j)
{
    fer_mat3_t m3;
    size_t k, l, r, c;

    // copy the matrix into m3 leaving out i'th row and j'th column
    r = 0;
    for(k = 0; k < 4; k++){
        if (k == i)
            continue;

        c = 0;
        for (l = 0; l < 4; l++){
            if (l == j)
                continue;
            ferMat3Set1(&m3, r, c, ferMat4Get(m, k, l));
            c++;
        }

        r++;
    }

    return ferMat3Det(&m3);
}
