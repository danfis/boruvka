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


static void ferGAEvalInt(fer_ga_t *ga, void *gt, fer_real_t *ft, void *data);
static void ferGAInitInt(fer_ga_t *ga, void *gt, void *data);
static void ferGAMutateInt(fer_ga_t *ga, void *gt, void *data);


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
    p->pm = 0.001;
    p->gene_size = 1;
    p->genotype_size = 1;
    p->pop_size = 1;
    p->fitness_size = 1;
    p->crossover_size = 2;
}

void ferGAOpsParamsInt(fer_ga_ops_t *ops, fer_ga_params_t *params,
                       size_t genotype_size, size_t pop_size)
{
    ferGAOpsInit(ops);
    ferGAParamsInit(params);

    ops->eval      = ferGAEvalInt;
    ops->init      = ferGAInitInt;
    ops->mutate    = ferGAMutateInt;

    params->gene_size     = sizeof(int);
    params->genotype_size = genotype_size;
    params->pop_size      = pop_size;
    params->fitness_size  = 1;
    params->crossover_size = 2;
}

fer_ga_t *ferGANew(const fer_ga_ops_t *ops, const fer_ga_params_t *params)
{
    fer_ga_t *ga;
    size_t i;

    ga = FER_ALLOC(fer_ga_t);
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


    ga->gt[0] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
    ga->gt[1] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
    ga->ft[0] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);
    ga->ft[1] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);


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
    FER_FREE(ga->gt[0]);
    FER_FREE(ga->gt[1]);
    FER_FREE(ga->ft[0]);
    FER_FREE(ga->ft[1]);

    ferRandMTDel(ga->rand);

    FER_FREE(ga);
}

void ferGARun(fer_ga_t *ga)
{
    size_t popsize, i, sel;
    unsigned long cb = 0UL;

    for (i = 0; i < ga->params.pop_size; ++i){
        ga->gt[0][0] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]);
        ga->ft[0][0] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][i]);

        ga->ops.init(ga, ga->gt[0][0], ga->ops.init_data);
        ga->ops.eval(ga, ga->gt[0][0], ga->ft[0][0], ga->ops.eval_data);
    }

    do {
        cb += 1UL;

        for (i = 0; i < ga->params.pop_size; i++){
            DBG("[%d]: %f [%d %d %d %d %d]", (int)i,
                    (float)(ferGAIndivFitness(ga, ga->pop[ga->pop_cur][i])[0]),
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[0],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[1],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[2],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[3],
                    ((int *)ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]))[4]);
        }

        if (cb == ga->ops.callback_period && ga->ops.callback){
            ga->ops.callback(ga, ga->ops.callback_data);
            cb = 0UL;
        }

        for (popsize = 0; popsize < ga->params.pop_size;){
            // selection
            for (i = 0; i < ga->params.crossover_size; i++){
                sel = ga->ops.sel(ga, ga->ops.sel_data);
                ga->gt[0][i] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][sel]);
                ga->ft[0][i] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][sel]);

                if (popsize < ga->params.pop_size){
                    ga->gt[1][i] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur ^ 1][popsize]);
                    ga->ft[1][i] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur ^ 1][popsize]);
                    ++popsize;
                }
                DBG("[%d] %d", (int)i, (int)sel);
                //DBG("Sel: %d %d", (int)sel[0], (int)sel[1]);
            }

            // crossover
            if (ferRandMT01(ga->rand) < ga->params.pc){
                ga->ops.crossover(ga, ga->gt[0], ga->gt[1], ga->ops.crossover_data);
            }else{
                for (i = 0; i < ga->params.crossover_size && ga->gt[1][i]; i++){
                    memcpy(ga->gt[1][i], ga->gt[0][i],
                           ga->params.gene_size * ga->params.genotype_size);
                }
            }

            // mutation
            for (i = 0; i < ga->params.crossover_size && ga->gt[1][i]; i++){
                ga->ops.mutate(ga, ga->gt[1][i], ga->ops.mutate_data);
            }

            // eval
            for (i = 0; i < ga->params.crossover_size && ga->gt[1][i]; i++){
                ga->ops.eval(ga, ga->gt[1][i], ga->ft[1][i], ga->ops.eval_data);
            }
        }

        ga->pop_cur ^= 1;
    } while (!ga->ops.terminate(ga, ga->ops.terminate_data));
}


size_t ferGASelTournament2(fer_ga_t *ga, void *data)
{
    size_t t[2];
    fer_real_t f[2];

    t[0] = ferRandMT(ga->rand, 0, ga->params.pop_size);
    t[1] = ferRandMT(ga->rand, 0, ga->params.pop_size);
    f[0] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[0]])[0];
    f[1] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][t[1]])[0];

    if (f[0] > f[1])
        return t[0];
    return t[1];
}

void ferGACrossover2(fer_ga_t *ga, void **ing, void **outg, void *data)
{
    int cross = ferRandMT(ga->rand, 0, ga->params.genotype_size - 1);
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




static void ferGAEvalInt(fer_ga_t *ga, void *_gt, fer_real_t *ft, void *data)
{
    int *gt = (int *)_gt;
    size_t i;

    ft[0] = gt[0];
    for (i = 1; i < ga->params.genotype_size; i++){
        ft[0] += gt[i];
    }
}

static void ferGAInitInt(fer_ga_t *ga, void *_gt, void *data)
{
    size_t i;
    int *gt = (int *)_gt;

    for (i = 0; i < ga->params.genotype_size; i++){
        gt[i] = ferRandMT(ga->rand, -10, 10);
    }
}

static void ferGAMutateInt(fer_ga_t *ga, void *_gt, void *data)
{
    int *gt = (int *)_gt;
    size_t i;

    for (i = 0; i < ga->params.genotype_size; i++){
        if (ferRandMT01(ga->rand) < ga->params.pm){
            gt[i] = ferRandMT(ga->rand, -10, 10);
        }
    }
}
