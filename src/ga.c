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

#include <fermat/ga.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <strings.h>
#include <string.h>

/** Invididual is array organized as follows:
 *  (fer_real_t * fitness_size) | (gene_size * genotype_size)
 */
static void *ferGAIndivNew(fer_ga_t *ga);
static void ferGAIndivDel(fer_ga_t *ga, void *indiv);

#define OPS_DATA(name) \
    if (!ga->ops.name ## _data) \
        ga->ops.name ## _data = ga->ops.data;

#define OPS_CHECK(name) \
    if (!ga->ops.name){ \
        fprintf(stderr, "Fermat :: GA :: No " #name "callback set.\n"); \
        exit(-1); \
    }

#define OPS_CHECK_DATA(name) \
    OPS_DATA(name) \
    OPS_CHECK(name)



void ferGAOpsInit(fer_ga_ops_t *ops)
{
    bzero(ops, sizeof(fer_ga_ops_t));
    ops->sel       = ferGASelTournament2;
    ops->crossover = ferGACrossover2;
    ops->mutate    = ferGAMutateNone;
}

void ferGAParamsInit(fer_ga_params_t *p)
{
    p->pc = 0.7;
    p->pm = 0.3;
    p->gene_size = 1;
    p->genotype_size = 1;
    p->pop_size = 1;
    p->fitness_size = 1;
    p->crossover_size = 2;
    p->threads = 1;
}

fer_ga_t *ferGANew(const fer_ga_ops_t *ops, const fer_ga_params_t *params)
{
    fer_ga_t *ga;
    size_t i;

    ga = FER_ALLOC(fer_ga_t);
    ga->tid = -1;
    ga->params = *params;

    ga->ops = *ops;
    OPS_CHECK_DATA(eval)
    OPS_CHECK_DATA(terminate)
    OPS_CHECK_DATA(init)
    OPS_CHECK_DATA(sel)
    OPS_CHECK_DATA(crossover)
    OPS_CHECK_DATA(mutate)
    OPS_DATA(callback)


    // allocate populations
    ga->pop[0] = FER_ALLOC_ARR(void *, ga->params.pop_size);
    ga->pop[1] = FER_ALLOC_ARR(void *, ga->params.pop_size);
    for (i = 0; i < ga->params.pop_size; i++){
        ga->pop[0][i] = ferGAIndivNew(ga);
        ga->pop[1][i] = ferGAIndivNew(ga);
    }
    ga->pop_cur = 0;

    ga->gt[0] = ga->gt[1] = NULL;
    ga->ft[0] = ga->ft[1] = NULL;

    ga->rand = ferRandMTNewAuto();

    return ga;
}

void ferGADel(fer_ga_t *ga)
{
    size_t i;

    for (i = 0; i < ga->params.pop_size; i++){
        ferGAIndivDel(ga, ga->pop[0][i]);
        ferGAIndivDel(ga, ga->pop[1][i]);
    }
    FER_FREE(ga->pop[0]);
    FER_FREE(ga->pop[1]);

    ferRandMTDel(ga->rand);

    FER_FREE(ga);
}

static void _ferGAStep(fer_ga_t *ga, size_t from, size_t to)
{
    size_t ind, i, sel;


    ind = from;
    while (ind < to){
        // selection
        for (i = 0; i < ga->params.crossover_size; i++){
            sel = ga->ops.sel(ga, ga->ops.sel_data);
            ga->gt[0][i] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][sel]);
            ga->ft[0][i] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][sel]);

            if (ind < to){
                ga->gt[1][i] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur ^ 1][ind]);
                ga->ft[1][i] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur ^ 1][ind]);
                ++ind;
            }
            //DBG("[%d] %d", (int)i, (int)sel);
            //DBG("Sel: %d %d", (int)sel[0], (int)sel[1]);
        }

        // crossover
        if (ferGARand01(ga) < ga->params.pc){
            ga->ops.crossover(ga, ga->gt[0], ga->gt[1], ga->ops.crossover_data);
        }else{
            for (i = 0; i < ga->params.crossover_size && ga->gt[1][i]; i++){
                memcpy(ga->gt[1][i], ga->gt[0][i],
                       ga->params.gene_size * ga->params.genotype_size);
            }
        }

        // mutation
        for (i = 0; i < ga->params.crossover_size && ga->gt[1][i]; i++){
            if (ferGARand01(ga) < ga->params.pm){
                ga->ops.mutate(ga, ga->gt[1][i], ga->ops.mutate_data);
            }
        }

        // eval
        for (i = 0; i < ga->params.crossover_size && ga->gt[1][i]; i++){
            ga->ops.eval(ga, ga->gt[1][i], ga->ft[1][i], ga->ops.eval_data);
        }
    }
}

static void _ferGAInit(fer_ga_t *ga, size_t from, size_t to)
{
    size_t i;

    for (i = 0; i < ga->params.pop_size; ++i){
        ga->gt[0][0] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]);
        ga->ft[0][0] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][i]);

        ga->ops.init(ga, ga->gt[0][0], ga->ops.init_data);
        ga->ops.eval(ga, ga->gt[0][0], ga->ft[0][0], ga->ops.eval_data);
    }
}

void ferGARun(fer_ga_t *ga)
{
    unsigned long cb = 0UL;

    // alloc temporary memory
    ga->gt[0] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
    ga->gt[1] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
    ga->ft[0] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);
    ga->ft[1] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);

    // initialize individuals
    _ferGAInit(ga, 0, ga->params.pop_size);

    do {
        cb += 1UL;

        /*
        for (i = 0; i < ga->params.pop_size; i++){
            DBG("[%d]: %f [%d %d %d %d %d]", (int)i,
                    (float)(ferGAIndivFitness(ga, ga->pop[ga->pop_cur][i])[0]),
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[0],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[1],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[2],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[3],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[4]);
        }
        */

        if (cb == ga->ops.callback_period && ga->ops.callback){
            ga->ops.callback(ga, ga->ops.callback_data);
            cb = 0UL;
        }

        // one step of breeding
        _ferGAStep(ga, 0, ga->params.pop_size);

        ga->pop_cur ^= 1;
    } while (!ga->ops.terminate(ga, ga->ops.terminate_data));

    // free tmp memory
    FER_FREE(ga->gt[0]);
    FER_FREE(ga->gt[1]);
    FER_FREE(ga->ft[0]);
    FER_FREE(ga->ft[1]);

    ga->gt[0] = ga->gt[1] = NULL;
    ga->ft[0] = ga->ft[1] = NULL;
}


size_t ferGASelTournament2(fer_ga_t *ga, void *data)
{
    size_t t[2];
    fer_real_t f[2];

    t[0] = ferGARand(ga, 0, ga->params.pop_size);
    t[1] = ferGARand(ga, 0, ga->params.pop_size);
    f[0] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[0]])[0];
    f[1] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[1]])[0];

    if (f[0] > f[1])
        return t[0];
    return t[1];
}

size_t ferGASelTournament3(fer_ga_t *ga, void *data)
{
    size_t t[3];
    fer_real_t f[3];

    t[0] = ferGARand(ga, 0, ga->params.pop_size);
    t[1] = ferGARand(ga, 0, ga->params.pop_size);
    t[2] = ferGARand(ga, 0, ga->params.pop_size);
    f[0] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[0]])[0];
    f[1] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[1]])[0];
    f[2] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[2]])[0];

    if (f[0] > f[1]){
        if (f[0] > f[2])
            return t[0];
        return t[2];
    }else{
        if (f[1] > f[2])
            return t[1];
        return t[2];
    }
}

void ferGACrossover2(fer_ga_t *ga, void **ing, void **outg, void *data)
{
    int cross = ferGARand(ga, 0, ga->params.genotype_size - 1);
    size_t size1, size2;

    // size of first and second half
    size1 = ga->params.gene_size * (cross + 1);
    size2 = ga->params.gene_size * (ga->params.genotype_size - cross - 1);

    memcpy(outg[0], ing[0], size1);
    memcpy((char *)outg[0] + size1, (char *)ing[1] + size1, size2);

    if (outg[1]){
        memcpy(outg[1], ing[1], size1);
        memcpy((char *)outg[1] + size1, (char *)ing[0] + size1, size2);
    }
}

void ferGAMutateNone(fer_ga_t *ga, void *gt, void *data)
{
}




static void *ferGAIndivNew(fer_ga_t *ga)
{
    size_t size;
    void *indiv;

    size = sizeof(fer_real_t) * ga->params.fitness_size;
    size += ga->params.gene_size * ga->params.genotype_size;

    indiv = ferRealloc(NULL, size);

    return indiv;
}

static void ferGAIndivDel(fer_ga_t *ga, void *indiv)
{
    FER_FREE(indiv);
}
