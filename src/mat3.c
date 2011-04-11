#include <fermat/mat3.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

static FER_MAT3(__fer_mat3_identity, FER_ONE, FER_ZERO, FER_ZERO,
                                     FER_ZERO, FER_ONE, FER_ZERO,
                                     FER_ZERO, FER_ZERO, FER_ONE);
const fer_mat3_t *fer_mat3_identity = &__fer_mat3_identity;

static FER_MAT3(__fer_mat3_zero, FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO,
                                 FER_ZERO, FER_ZERO, FER_ZERO);
const fer_mat3_t *fer_mat3_zero = &__fer_mat3_zero;

fer_mat3_t *ferMat3New(void)
{
    fer_mat3_t *m;

#ifdef FER_SSE
    m = FER_ALLOC_ALIGN(fer_mat3_t, 16);
#else /* FER_SSE */
    m = FER_ALLOC(fer_mat3_t);
#endif /* FER_SSE */

    return m;
}

void ferMat3Del(fer_mat3_t *m)
{
    free(m);
}


#define EIGEN_MAX_STEPS 50
#define EIGEN_TRESH 0.01
static const int eigen_row[3] = { 0, 0, 1 };
static const int eigen_col[3] = { 1, 2, 2 };

int ferMat3Eigen(const fer_mat3_t *m, fer_mat3_t *eigen)
{
    fer_mat3_t rot, rot_t;
    size_t i;
    int row, col;
    fer_real_t angle, y, x;

    // Copy source matrix to matrix holding eigen vectors
    ferMat3Copy(eigen, m);
    // Init rotational matrix
    ferMat3SetIdentity(&rot);

    for (i = 0; i < EIGEN_MAX_STEPS; i++){
        // Choose off-diagonal in upper triangular half, we have prepared
        // its coordinates in eigen_row[] and eigen_col[] arrays
        row = eigen_row[i % 3];
        col = eigen_col[i % 3];

        // Compute rotational angle
        if (ferEq(ferMat3Get(eigen, row, row), ferMat3Get(eigen, col, col))){
            angle = M_PI_2;
        }else{
            y = FER_REAL(2.) * ferMat3Get(eigen, row, col);
            x = ferMat3Get(eigen, row, row) - ferMat3Get(eigen, col, col);
            angle = FER_ATAN(y / x);
            angle *= ferRecp(FER_REAL(2.));
        }

        if (FER_FABS(angle) < EIGEN_TRESH)
            break;

        // set rotation matrix
        if (i % 3 == 0){
            ferMat3Set(&rot, FER_COS(angle), -FER_SIN(angle), FER_ZERO,
                             FER_SIN(angle), FER_COS(angle), FER_ZERO,
                             FER_ZERO, FER_ZERO, FER_ONE);
        }else if (i % 3 == 1){
            ferMat3Set(&rot, FER_COS(angle), FER_ZERO, -FER_SIN(angle),
                             FER_ZERO, FER_ONE, FER_ZERO,
                             FER_SIN(angle), FER_ZERO, FER_COS(angle));
        }else{
            ferMat3Set(&rot, FER_ONE, FER_ZERO, FER_ZERO,
                             FER_ZERO, FER_COS(angle), -FER_SIN(angle),
                             FER_ZERO, FER_SIN(angle), FER_COS(angle));
        }

        // set transposition of rot
        ferMat3Trans2(&rot_t, &rot);

        // rotate matrix
        ferMat3MulLeft(eigen, &rot_t);
        ferMat3Mul(eigen, &rot);
    }

    return 0;
}
