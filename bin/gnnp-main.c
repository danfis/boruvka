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
#include <fermat/kohonen.h>
#include <fermat/dbg.h>

fer_rand_mt_t *rnd;
fer_vec_t *is;
fer_vec_t *start, *goal;
fer_real_t aabb[4] = {-5, 5, -5, 5};
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
    fer_real_t x, y, r = 0.05;
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

static int isc = 0;
static int num[2][100];
static FILE *gng_fout;
const fer_vec_t *kInputSignal(fer_kohonen_t *k, void *data)
{
    fer_gnnp_t *nn = (fer_gnnp_t *)data;
    const fer_gnnp_node_t *wn;
    int i;

    if (isc == 0){
        for (i = 0; i < 100; i++){
            num[0][i] = 0;
            num[1][i] = 0;
        }
    }

    wn = ferGNNPRandNode(nn);

    num[wn->set - 1][wn->depth]++;

    isc++;

    ferVecPrint(2, wn->w, gng_fout);
    fprintf(gng_fout, "\n");
    return wn->w;
}

int kTerminate(fer_kohonen_t *k, void *data)
{
    static int c = 0;
    int i;
    c++;

    if (c == 10000){
        fprintf(stderr, "    -> free:");
        for (i = 0; i < 100; i++){
            if (num[0][i] > 0)
                fprintf(stderr, " %02d:%04d", i, num[0][i]);
        }
        fprintf(stderr, "\n");

        fprintf(stderr, "    -> obst:");
        for (i = 0; i < 100; i++){
            if (num[1][i] > 0)
                fprintf(stderr, " %02d:%04d", i, num[1][i]);
        }
        fprintf(stderr, "\n");

        c = 0;
        return 1;
    }
    return 0;
}

int kNeigh(fer_kohonen_t *k,
           const fer_kohonen_node_t *center,
           const fer_kohonen_node_t *cur,
           int depth,
           fer_real_t *val, void *data)
{
    if (depth > 2)
        return 0;

    *val = FER_EXP(-FER_SQ(depth) / (2. * FER_SQ(0.1)));
    return 1;
}

void kCallback(fer_kohonen_t *k, void *data)
{
}

static void callback(fer_gnnp_t *nn, void *data)
{
    static int c = 0;
    char fn[100];
    FILE *fout;
    size_t i;
    fer_kohonen_ops_t ops;
    fer_kohonen_params_t params;
    fer_kohonen_t *k;

    snprintf(fn, 100, "g-%06d", c);
    fout = fopen(fn, "w");
    if (fout){
        ferGNNPDumpSVT(nn, fout, NULL);
        fclose(fout);
    }


    fprintf(stderr, "step %d, nodes: %d, evals: %ld, max_depth: %d\n",
            c, (int)ferGNNPNodesLen(nn), (long)evals, (int)nn->max_depth);

    fprintf(stderr, "    depth:");
    for (i = 0; i < nn->depths_alloc; i++){
        if (nn->depths[i] != 0)
            fprintf(stderr, " %02d:%04d", (int)i, (int)nn->depths[i]);
    }
    fprintf(stderr, "\n");

    
    fprintf(stderr, "    free: %d\n", (int)nn->nodes_set[0].len);
    fprintf(stderr, "    obst: %d\n", (int)nn->nodes_set[1].len);


    ferKohonenOpsInit(&ops);
    ops.input_signal = kInputSignal;
    ops.terminate    = kTerminate;
    ops.neighborhood = kNeigh;
    ops.data = (void *)nn;

    ferKohonenParamsInit(&params);
    params.learn_rate = 0.01;
    params.nn.gug.aabb = aabb;

    k = ferKohonenNew(&ops, &params);

    {
        fer_kohonen_node_t *n[2];
        int i;
        FER_VEC(inc, 2);
        FER_VEC(pos, 2);

        ferVecSub2(2, inc, goal, start);
        ferVecScale(2, inc, 0.25 / ferVecDist(2, goal, start));

        n[0] = ferKohonenNodeNew(k, start);
        ferKohonenNodeSetFixed(n[0], 1);
        ferVecCopy(2, pos, start);
        ferVecAdd(2, pos, inc);
        for (i = 1; ferVecDist(2, pos, goal) > 0.25; i ^= 1){
            n[i] = ferKohonenNodeNew(k, pos);
            ferKohonenNodeConnect(k, n[i ^ 1], n[i]);
            ferVecAdd(2, pos, inc);
        }
        n[i] = ferKohonenNodeNew(k, goal);
        ferKohonenNodeSetFixed(n[i], 1);
        ferKohonenNodeConnect(k, n[i ^ 1], n[i]);
    }

    isc = 0;
    snprintf(fn, 100, "h-%06d", c);
    gng_fout = fopen(fn, "w");
    fprintf(gng_fout, "Point size: 1\n");
    fprintf(gng_fout, "Points:\n");

    ferKohonenRun(k);
    if (gng_fout){
        ferKohonenDumpSVT(k, gng_fout, NULL);
        fclose(gng_fout);
    }

    ferKohonenDel(k);



    c++;
}

static void printPath(fer_gnnp_t *nn, fer_list_t *path)
{
    fer_list_t *item;
    fer_gnnp_node_t *n;
    int i, len;

    printf("----\n");
    printf("Name: PATH\n");

    printf("Points off: 1\n");
    printf("Edge color: 1 0 0\n");
    printf("Edge width: 2\n");
    printf("Points:\n");
    len = 0;
    FER_LIST_FOR_EACH(path, item){
        n = FER_LIST_ENTRY(item, fer_gnnp_node_t, path);
        ferVecPrint(2, n->w, stdout);
        printf("\n");
        len++;
    }

    printf("Edges:\n");
    for (i = 0; i < len - 1; i++){
        printf("%d %d\n", i, i + 1);
    }

    printf("----\n");
}

int main(int argc, char *argv[])
{
    fer_list_t path;
    int ret;
    fer_gnnp_params_t params;
    fer_gnnp_ops_t ops;
    fer_gnnp_t *nn;

    ferGNNPOpsInit(&ops);
    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback        = callback;
    ops.callback_period = 1000;

    ferGNNPParamsInit(&params);
    params.dim  = 2;
    params.rmax = 6;
    params.h    = 0.025;
    params.prune_delay = 500;
    params.tournament = 3;
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

    ret = ferGNNPFindPath(nn, start, goal, &path);
    callback(nn, NULL);
    fprintf(stderr, "ret: %d\n", ret);
    fprintf(stderr, "evals: %ld\n", (long)evals);
    if (ret == 0)
        printPath(nn, &path);


    ferGNNPDel(nn);


    ferVecDel(is);
    ferVecDel(start);
    ferVecDel(goal);

    ferRandMTDel(rnd);

    return 0;
}
