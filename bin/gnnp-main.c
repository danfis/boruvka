/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <fermat/gnnp.h>
#include <fermat/rand-mt.h>

fer_rand_mt_t *rnd;
fer_vec_t *is;
fer_vec_t *start, *goal;
unsigned long evals = 0UL;

static int terminate(fer_gnnp_t *nn, void *data)
{
    static int count = 0;

    ++count;
    if (count == 10000000)
        return 1;
    return 0;
}

static const fer_vec_t *inputSignal(fer_gnnp_t *nn, void *data)
{
    ferVecSet(is, 0, ferRandMT(rnd, -5, 5));
    ferVecSet(is, 1, ferRandMT(rnd, -5, 5));
    return is;
}

static int eval(fer_gnnp_t *nn, const fer_vec_t *conf, void *data)
{
    fer_real_t x, y, r = 0.03;
    x = ferVecGet(conf, 0);
    y = ferVecGet(conf, 1);

    evals += 1UL;

    if (y < -2
            || (y < 4 && y > -2 && x > -r && x < r)
            || (y > 4 && x > -2 && x < 2)){
        return 1;
    }
    return 0;
}

static void callback(fer_gnnp_t *nn, void *data)
{
    static int c = 0;

    c++;
    fprintf(stderr, "step %d, nodes: %d, evals: %ld\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals);
    ferGNNPDumpSVT(nn, stdout, NULL);
}

int main(int argc, char *argv[])
{
    fer_list_t path;
    int ret;
    fer_gnnp_params_t params;
    fer_gnnp_ops_t ops;
    fer_gnnp_t *nn;
    fer_real_t aabb[4] = {-5, 5, -5, 5};

    ferGNNPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback        = callback;
    ops.callback_period = 10000;

    ferGNNPParamsInit(&params);
    params.dim  = 2;
    params.rmax = 6;
    params.h    = 0.015;
    params.prune_delay = 500;
    params.nn.type = FER_NN_GUG;
    params.nn.gug.max_dens = 1.;
    params.nn.gug.expand_rate = 1.3;
    params.nn.gug.aabb = aabb;

    //rnd = ferRandMTNewAuto();
    rnd = ferRandMTNew(1111);

    is    = ferVecNew(2);
    start = ferVecNew(2);
    goal  = ferVecNew(2);
    ferVecSetN(2, start, -4., -4.);
    ferVecSetN(2, goal, 1.5, 4.5);


    nn = ferGNNPNew(&ops, &params);

    ferListInit(&path);
    ret = ferGNNPFindPath(nn, start, goal, &path);
    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "evals: %ld\n", (long)evals);

    ferGNNPDumpSVT(nn, stdout, NULL);

    ferGNNPDel(nn);


    ferVecDel(is);
    ferVecDel(start);
    ferVecDel(goal);

    ferRandMTDel(rnd);

    return 0;
}
