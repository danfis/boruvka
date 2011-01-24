#include <fermat/mat3.h>
#include <fermat/alloc.h>

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
