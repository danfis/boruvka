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


static void ferGAInitInt(fer_ga_t *ga, void *indiv);
static size_t ferGASelTournament(fer_ga_t *ga, void **pop);
static void ferGAEvalInt(fer_ga_t *ga, void *indiv);
static void ferGACrossoverInt(fer_ga_t *ga, void *ing1, void *ing2,
                                            void *outg1, void *outg2);
static void ferGAMutateInt(fer_ga_t *ga, void *genotype);


void ferGAOpsInit(fer_ga_ops_t *ops)
{
    bzero(ops, sizeof(fer_ga_ops_t));
}

void ferGAParamsInit(fer_ga_params_t *p)
{
    p->pc = 0.7;
    p->pm = 0.001;
    p->gene_size = 1;
    p->genotype_size = 1;
    p->pop_size = 1;
    p->fitness_size = 1;
}

void ferGAOpsParamsInt(fer_ga_ops_t *ops, fer_ga_params_t *params,
                       size_t genotype_size, size_t pop_size)
{
    ferGAOpsInit(ops);
    ferGAParamsInit(params);

    params->gene_size     = sizeof(int);
    params->genotype_size = genotype_size;
    params->pop_size      = pop_size;
    params->fitness_size  = 1;
}

fer_ga_t *ferGANew(const fer_ga_ops_t *ops, const fer_ga_params_t *params)
{
    fer_ga_t *ga;
    size_t i;

    ga = FER_ALLOC(fer_ga_t);
    ga->params = *params;

    ga->ops = *ops;
    if (!ga->ops.eval_data)
        ga->ops.eval_data = ga->ops.data;
    if (!ga->ops.terminate_data)
        ga->ops.terminate_data = ga->ops.data;
    if (!ga->ops.init_data)
        ga->ops.init_data = ga->ops.data;
    if (!ga->ops.sel_data)
        ga->ops.sel_data = ga->ops.data;
    if (!ga->ops.crossover_data)
        ga->ops.crossover_data = ga->ops.data;
    if (!ga->ops.mutate_data)
        ga->ops.mutate_data = ga->ops.data;
    if (!ga->ops.callback_data)
        ga->ops.callback_data = ga->ops.data;

    if (!ga->ops.terminate){
        fprintf(stderr, "Fermat :: GA :: No terminate callback set.\n");
        exit(-1);
    }
    if (!ga->ops.eval){
        fprintf(stderr, "Fermat :: GA :: No eval callback set.\n");
        exit(-1);
    }


    // allocate populations
    ga->pop[0] = FER_ALLOC_ARR(void *, ga->params.pop_size);
    ga->pop[1] = FER_ALLOC_ARR(void *, ga->params.pop_size);
    for (i = 0; i < ga->params.pop_size; i++){
        ga->pop[0][i] = ferGAIndivNew(ga);
        ga->pop[1][i] = ferGAIndivNew(ga);
    }
    ga->pop_cur = 0;

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

void ferGARun(fer_ga_t *ga)
{
    size_t popsize, i;
    size_t sel[2];
    void *indsel[2];
    void *indselgp[2];
    fer_real_t *indself[2];
    void *ind[2];
    void *indgp[2];
    fer_real_t *indf[2];
    unsigned long cb = 0UL;

    for (i = 0; i < ga->params.pop_size; ++i){
        indsel[0]   = ga->pop[ga->pop_cur][i];
        indselgp[0] = ferGAIndivGenotype(ga, indsel[0]);
        indself[0]  = ferGAIndivFitness(ga, indsel[0]);

        ga->ops.init(ga, indselgp[0], ga->ops.init_data);
        ga->ops.eval(ga, indselgp[0], indself[0], ga->ops.eval_data);
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
            sel[0] = ga->ops.sel(ga, ga->ops.sel_data);
            sel[1] = ga->ops.sel(ga, ga->ops.sel_data);
            indsel[0] = ga->pop[ga->pop_cur][sel[0]];
            indsel[1] = ga->pop[ga->pop_cur][sel[1]];
            indselgp[0] = ferGAIndivGenotype(ga, indsel[0]);
            indselgp[1] = ferGAIndivGenotype(ga, indsel[1]);
            indself[0] = ferGAIndivFitness(ga, indsel[0]);
            indself[1] = ferGAIndivFitness(ga, indsel[1]);
            DBG("Sel: %d %d", (int)sel[0], (int)sel[1]);

            ind[0] = ind[1] = NULL;
            indgp[0] = indgp[1] = NULL;
            if (popsize < ga->params.pop_size){
                ind[0]   = ga->pop[ga->pop_cur ^ 1][popsize++];
                indgp[0] = ferGAIndivGenotype(ga, ind[0]);
                indf[0]  = ferGAIndivFitness(ga, ind[0]);
            }
            if (popsize < ga->params.pop_size){
                ind[1]   = ga->pop[ga->pop_cur ^ 1][popsize++];
                indgp[1] = ferGAIndivGenotype(ga, ind[1]);
                indf[1]  = ferGAIndivFitness(ga, ind[1]);
            }

            // crossover
            if (ferRandMT01(ga->rand) < ga->params.pc){
                ga->ops.crossover(ga, indselgp[0], indselgp[1],
                                      indgp[0], indgp[1],
                                      ga->ops.crossover_data);
            }else{
                if (ind[0])
                    memcpy(indgp[0], indselgp[0],
                           ga->params.gene_size * ga->params.genotype_size);
                if (ind[1])
                    memcpy(indgp[1], indselgp[1],
                           ga->params.gene_size * ga->params.genotype_size);
            }

            // mutation
            if (ind[0])
                ga->ops.mutate(ga, indgp[0], ga->ops.mutate_data);
            if (ind[1])
                ga->ops.mutate(ga, indgp[1], ga->ops.mutate_data);

            // eval
            if (ind[0])
                ga->ops.eval(ga, indgp[0], indf[0], ga->ops.eval_data);
            if (ind[1])
                ga->ops.eval(ga, indgp[1], indf[1], ga->ops.eval_data);
        }

        ga->pop_cur ^= 1;
    } while (!ga->ops.terminate(ga, ga->ops.terminate_data));
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



static void ferGAInitInt(fer_ga_t *ga, void *indiv)
{
    size_t i;
    int *genotype;

    genotype = (int *)ferGAIndivGenotype(ga, indiv);
    for (i = 0; i < ga->params.genotype_size; i++){
        genotype[i] = ferRandMT(ga->rand, -10, 10);
    }
}

static size_t ferGASelTournament(fer_ga_t *ga, void **pop)
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

static void ferGAEvalInt(fer_ga_t *ga, void *indiv)
{
    fer_real_t *f;
    int *g;
    size_t i;

    f = ferGAIndivFitness(ga, indiv);
    g = ferGAIndivGenotype(ga, indiv);

    f[0] = g[0];
    for (i = 1; i < ga->params.genotype_size; i++){
        f[0] += g[i];
    }
}

static void ferGACrossoverInt(fer_ga_t *ga, void *ing1, void *ing2,
                                            void *outg1, void *outg2)
{
    int cross = ferRandMT(ga->rand, 0, ga->params.genotype_size - 1);
    size_t size1, size2;

    DBG("crossover = %d", cross);

    // size of first and second half
    size1 = ga->params.gene_size * (cross + 1);
    size2 = ga->params.gene_size * (ga->params.genotype_size - cross - 1);

    memcpy(outg1, ing1, size1);
    memcpy((char *)outg1 + size1, (char *)ing2 + size1, size2);

    if (outg2){
        memcpy(outg2, ing2, size1);
        memcpy((char *)outg2 + size1, (char *)ing1 + size1, size2);
    }
}

static void ferGAMutateInt(fer_ga_t *ga, void *genotype)
{
    int *gp = (int *)genotype;
    size_t i;

    for (i = 0; i < ga->params.genotype_size; i++){
        if (ferRandMT01(ga->rand) < ga->params.pm){
            gp[i] = ferRandMT(ga->rand, -10, 10);
        }
    }
}
