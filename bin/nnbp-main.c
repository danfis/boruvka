#include <fermat/nnbp.h>

int main(int argc, char *argv[])
{
    fer_nnbp_params_t params;
    fer_nnbp_t *nn;
    size_t layer_size[] = { 3, 4, 5 };
    FER_VEC(t1, 3) = { 1, 2, 3 };
    fer_real_t err;

    params.layers_num = 3;
    params.layer_size = layer_size;

    params.eta    = 0.3;
    params.alpha  = 0.7;
    params.lambda = 1;

    nn = ferNNBPNew(&params);

    err = ferNNBPLearn(nn, t1);
    printf("%f\n", err);

    ferNNBPDel(nn);

    return 0;
}
