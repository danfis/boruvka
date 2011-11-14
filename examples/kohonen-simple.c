/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <fermat/kohonen.h>
#include <fermat/alloc.h>
#include <fermat/rand-mt.h>

fer_rand_mt_t *rnd;
fer_real_t aabb[6] = { -FER_REAL(5.), FER_REAL(5.),
                       -FER_REAL(5.), FER_REAL(5.),
                       -FER_REAL(5.), FER_REAL(5.) };

FER_VEC(is, 2);

static const fer_vec_t *inputSignal(fer_kohonen_t *k, void *_)
{
    ferVecSet(is, 0, ferRandMT(rnd, -5, 5));
    ferVecSet(is, 1, ferRandMT(rnd, -5, 5));
    return is;
}

static int neighborhood(fer_kohonen_t *k,
                        const fer_kohonen_node_t *center,
                        const fer_kohonen_node_t *cur,
                        int depth,
                        fer_real_t *val, void *_)
{
    if (depth > 1)
        return 0;

    *val = FER_EXP(-FER_SQ(depth) / (2. * FER_SQ(0.1)));
    return 1;
}

static int terminate(fer_kohonen_t *k, void *_)
{
    static int c = 0;

    if (c++ == 1000000)
        return 1;
    return 0;
}

static void callback(fer_kohonen_t *k, void *_)
{
    ferKohonenDumpSVT(k, stdout, NULL);
}

static void createGrid(fer_kohonen_t *k, int x, int y)
{
    fer_kohonen_node_t **n;
    int i, j;

    n = FER_ALLOC_ARR(fer_kohonen_node_t *, x * y);

    for (i = 0; i < x * y; i++){
        n[i] = ferKohonenNodeNew(k, inputSignal(k, NULL));
    }

    for (i = 0; i < y; i++){
        for (j = 0; j < x; j++){
            if (j < x - 1)
                ferKohonenNodeConnect(k, n[i * x + j], n[i * x + j + 1]);
            if (i < y - 1)
                ferKohonenNodeConnect(k, n[i * x + j], n[(i + 1) * x + j]);
        }
    }

    FER_FREE(n);
}

int main(int argc, char *argv[])
{
    fer_kohonen_ops_t ops;
    fer_kohonen_params_t params;
    fer_kohonen_t *k;

    //rnd = ferRandMTNewAuto();
    rnd = ferRandMTNew(1111);

    ferKohonenOpsInit(&ops);
    ops.input_signal = inputSignal;
    ops.neighborhood = neighborhood;
    ops.terminate    = terminate;
    ops.callback     = callback;
    ops.callback_period = 100;

    ferKohonenParamsInit(&params);
    params.dim = 2;
    params.learn_rate = 0.01;
    params.nn.type     = FER_NN_GUG;
    params.nn.gug.aabb = aabb;

    k = ferKohonenNew(&ops, &params);
    createGrid(k, 5, 5);
    ferKohonenRun(k);
    ferKohonenDel(k);

    ferRandMTDel(rnd);

    return 0;
}
