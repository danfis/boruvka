/***
 * Boruvka
 * --------
 * Copyright (c)2010-2012 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <boruvka/mat3.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>

static BOR_MAT3(__fer_mat3_identity, BOR_ONE, BOR_ZERO, BOR_ZERO,
                                     BOR_ZERO, BOR_ONE, BOR_ZERO,
                                     BOR_ZERO, BOR_ZERO, BOR_ONE);
const bor_mat3_t *fer_mat3_identity = &__fer_mat3_identity;

static BOR_MAT3(__fer_mat3_zero, BOR_ZERO, BOR_ZERO, BOR_ZERO,
                                 BOR_ZERO, BOR_ZERO, BOR_ZERO,
                                 BOR_ZERO, BOR_ZERO, BOR_ZERO);
const bor_mat3_t *fer_mat3_zero = &__fer_mat3_zero;

bor_mat3_t *ferMat3New(void)
{
    bor_mat3_t *m;

#ifdef BOR_SSE
    m = BOR_ALLOC_ALIGN(bor_mat3_t, 16);
#else /* BOR_SSE */
    m = BOR_ALLOC(bor_mat3_t);
#endif /* BOR_SSE */

    return m;
}

void ferMat3Del(bor_mat3_t *m)
{
    BOR_FREE(m);
}


#define EIGEN_MAX_STEPS 50
static const int eigen_row[3] = { 0, 0, 1 };
static const int eigen_col[3] = { 1, 2, 2 };

int ferMat3Eigen(const bor_mat3_t *_m, bor_mat3_t *eigen,
                 bor_vec3_t *eigenvals)
{
    bor_mat3_t rot, m;
    size_t i;
    int row, col;
    bor_real_t angle, y, x;
    bor_real_t upper;

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
            y = BOR_REAL(2.) * ferMat3Get(&m, row, col);
            x = ferMat3Get(&m, row, row) - ferMat3Get(&m, col, col);
            angle = BOR_ATAN(y / x);
            angle *= ferRecp(BOR_REAL(2.));
        }

        upper  = ferMat3Get(&m, 0, 1);
        upper += ferMat3Get(&m, 0, 2);
        upper += ferMat3Get(&m, 1, 2);
        if (ferIsZero(upper))
            break;

        // set rotation matrix
        if (i % 3 == 0){
            ferMat3Set(&rot, BOR_COS(angle), -BOR_SIN(angle), BOR_ZERO,
                             BOR_SIN(angle), BOR_COS(angle), BOR_ZERO,
                             BOR_ZERO, BOR_ZERO, BOR_ONE);
        }else if (i % 3 == 1){
            ferMat3Set(&rot, BOR_COS(angle), BOR_ZERO, -BOR_SIN(angle),
                             BOR_ZERO, BOR_ONE, BOR_ZERO,
                             BOR_SIN(angle), BOR_ZERO, BOR_COS(angle));
        }else{
            ferMat3Set(&rot, BOR_ONE, BOR_ZERO, BOR_ZERO,
                             BOR_ZERO, BOR_COS(angle), -BOR_SIN(angle),
                             BOR_ZERO, BOR_SIN(angle), BOR_COS(angle));
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
