#include <string.h>
#include <fermat/nnbp.h>

void xor(void)
{
    fer_nnbp_params_t params;
    fer_nnbp_t *nn;
    size_t layer_size[] = { 3, 4, 1 };
    fer_real_t err;
    const fer_vec_t *out;
    fer_vec_t *in;
    int i, j, max_iter;
    FER_VEC_ARR(data, 4, 8) = {
        {FER_REAL(0.), FER_REAL(0.), FER_REAL(0.), FER_REAL(0.)},
        {FER_REAL(0.), FER_REAL(0.), FER_REAL(1.), FER_REAL(1.)},
        {FER_REAL(0.), FER_REAL(1.), FER_REAL(0.), FER_REAL(1.)},
        {FER_REAL(0.), FER_REAL(1.), FER_REAL(1.), FER_REAL(0.)},
        {FER_REAL(1.), FER_REAL(0.), FER_REAL(0.), FER_REAL(1.)},
        {FER_REAL(1.), FER_REAL(0.), FER_REAL(1.), FER_REAL(0.)},
        {FER_REAL(1.), FER_REAL(1.), FER_REAL(0.), FER_REAL(0.)},
        {FER_REAL(1.), FER_REAL(1.), FER_REAL(1.), FER_REAL(1.)}
    };
    FER_VEC_ARR(testdata, 3, 8) = {
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 1},
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1}
    };

    ferNNBPParamsInit(&params);
    params.layers_num = sizeof(layer_size) / sizeof(size_t);
    params.layer_size = layer_size;

    params.eta    = 0.2;
    params.alpha  = 0.1;
    params.lambda = 1.;
    max_iter = 500000;

    nn = ferNNBPNew(&params);

    for (i = 0; i < max_iter; i++){
        for (j = 0; j < 5; j++){
            in = data[i % 8];
            ferNNBPLearn(nn, in, ferVecOff(in, 3));
        }

        err = ferNNBPErr(nn, ferVecOff(in, 3));
        printf("[%08d]: %f\n", (int)i, (float)err);
        if (err < 0.00001)
            break;
    }

    printf("train data:\n");
    for (i = 0; i < 8; i++){
        in = data[i];
        out = ferNNBPFeed(nn, in);
        err = ferNNBPErr(nn, ferVecOff(in, 3));
        printf("[%d] train: %f, out: %f, err: %f\n", i, ferVecGet(in, 3), ferVecGet(out, 0), err);
    }

    printf("test data:\n");
    for (i = 0; i < 8; i++){
        in = testdata[i];
        out = ferNNBPFeed(nn, in);
        err = ferNNBPErr(nn, ferVecOff(in, 3));
        printf("[%d] %f %f %f: %f\n", i,
               ferVecGet(in, 0), ferVecGet(in, 1), ferVecGet(in, 2),
               ferVecGet(out, 0));
    }

    ferNNBPDel(nn);
}

int usage(int argc, char *argv[])
{
    fprintf(stderr, "Usage: %s xor\n", argv[0]);
    return -1;
}

int main(int argc, char *argv[])
{
    if (argc != 2){
        return usage(argc, argv);
    }

    if (strcmp(argv[1], "xor") == 0){
        xor();
    }else{
        return usage(argc, argv);
    }

    return 0;
}
