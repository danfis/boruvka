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
static const int eigen_row[3] = { 0, 0, 1 };
static const int eigen_col[3] = { 1, 2, 2 };

int ferMat3Eigen(const fer_mat3_t *_m, fer_mat3_t *eigen,
                 fer_vec3_t *eigenvals)
{
    fer_mat3_t rot, m;
    size_t i;
    int row, col;
    fer_real_t angle, y, x;
    fer_real_t upper;

    // Copy source matrix
    ferMat3Copy(&m, _m);
    // Init eigen vector matrix
    ferMat3SetIdentity(eigen);

    for (i = 0; i < EIGEN_MAX_STEPS; i++){
        // Choose off-diagonal largest element in upper triangular half
        // we have prepared its coordinates in eigen_row[] and eigen_col[]
        // arrays
        row = eigen_row[i % 3];
        col = eigen_col[i % 3];

        // Compute rotational angle
        if (ferEq(ferMat3Get(eigen, row, row), ferMat3Get(eigen, col, col))){
            angle = M_PI_2;
        }else{
            y = FER_REAL(2.) * ferMat3Get(&m, row, col);
            x = ferMat3Get(&m, row, row) - ferMat3Get(&m, col, col);
            angle = FER_ATAN(y / x);
            angle *= ferRecp(FER_REAL(2.));
        }

        upper  = ferMat3Get(&m, 0, 1);
        upper += ferMat3Get(&m, 0, 2);
        upper += ferMat3Get(&m, 1, 2);
        if (ferIsZero(upper))
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

        // rotate matrix
        ferMat3MulLeftTrans(&m, &rot);
        ferMat3Mul(&m, &rot);

        // update eigen vector matrix
        ferMat3Mul(eigen, &rot);
    }

    if (eigenvals){
        ferVec3Set(eigenvals, ferMat3Get(&m, 0, 0),
                              ferMat3Get(&m, 1, 1),
                              ferMat3Get(&m, 2, 2));

    }

    return 0;
}
