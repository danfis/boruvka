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
#include <fermat/tasks.h>
#include <fermat/sort.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <strings.h>
#include <string.h>
#include <limits.h>

/** Performs one breeding step on population in specified range */
static void _ferGAStep(fer_ga_t *ga, size_t from, size_t to);
/** Initialize population in specified range */
static void _ferGAInit(fer_ga_t *ga, size_t from, size_t to);

/** Runs GA in threads */
static void __ferGAStepTask(int id, void *data, const fer_tasks_thinfo_t *thinfo);
static void _ferGARunThreads(fer_ga_t *ga);
/** Runs single-threaded GA */
static void _ferGARun1(fer_ga_t *ga);


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
    p->pc             = 0.7;
    p->pm             = 0.3;
    p->gene_size      = 1;
    p->genotype_size  = 1;
    p->pop_size       = 1;
    p->fitness_size   = 1;
    p->crossover_size = 2;
    p->presel_max     = 10;
    p->threads        = 1;
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

    ga->presel = FER_ALLOC_ARR(size_t, ga->params.presel_max);

    ga->rand = ferRandMTNewAuto();

    pthread_mutex_init(&ga->tlock, NULL);

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

    FER_FREE(ga->presel);

    ferRandMTDel(ga->rand);

    FER_FREE(ga);
}

void ferGARun(fer_ga_t *ga)
{
    if (ga->params.threads > 1){
        _ferGARunThreads(ga);
    }else{
        _ferGARun1(ga);
    }
}


size_t ferGASelTournament2(fer_ga_t *ga, void *data)
{
    size_t t[2];
    fer_real_t f[2];

    t[0] = ferGARandInt(ga, 0, ga->params.pop_size);
    t[1] = ferGARandInt(ga, 0, ga->params.pop_size);
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

    t[0] = ferGARandInt(ga, 0, ga->params.pop_size);
    t[1] = ferGARandInt(ga, 0, ga->params.pop_size);
    t[2] = ferGARandInt(ga, 0, ga->params.pop_size);
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
    int cross = ferGARandInt(ga, 0, ga->params.genotype_size - 1);
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

size_t ferGAPreselElite(fer_ga_t *ga, size_t *sel, void *data)
{
    fer_radix_sort_t *rs, *rstmp;
    void *indiv;
    size_t i;

    if (ga->params.presel_max == 0)
        return 0;

    rs = FER_ALLOC_ARR(fer_radix_sort_t, ga->params.pop_size);
    rstmp = FER_ALLOC_ARR(fer_radix_sort_t, ga->params.pop_size);
    for (i = 0; i < ga->params.pop_size; i++){
        indiv = ferGAIndiv(ga, i);
        rs[i].key = ferGAIndivFitness(ga, indiv)[0];
        rs[i].val = i;
    }
    ferRadixSort(rs, rstmp, ga->params.pop_size);

    for (i = 0; i < ga->params.presel_max; i++){
        sel[i] = rs[ga->params.pop_size - 1 - i].val;
    }

    FER_FREE(rs);
    FER_FREE(rstmp);
    return ga->params.presel_max;
}



void __ferGATRandRefill(fer_ga_t *ga)
{
    size_t i;

    pthread_mutex_lock(&ga->tlock);
    for (i = 0; i < ga->trand_max; i++){
        ga->trand[i] = ferRandMT01(ga->rand);
    }
    pthread_mutex_unlock(&ga->tlock);

    ga->trand_next = 0;
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

    for (i = from; i < to; ++i){
        ga->gt[0][0] = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][i]);
        ga->ft[0][0] = ferGAIndivFitness(ga, ga->pop[ga->pop_cur][i]);

        ga->ops.init(ga, ga->gt[0][0], ga->ops.init_data);
        ga->ops.eval(ga, ga->gt[0][0], ga->ft[0][0], ga->ops.eval_data);
    }
}

static size_t _ferGAPreselect(fer_ga_t *ga)
{
    size_t i, len;
    void *f, *t;
    fer_real_t *ft;

    if (!ga->ops.presel)
        return 0;

    len = ga->ops.presel(ga, ga->presel, ga->ops.presel_data);
    for (i = 0; i < len; i++){
        t  = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur ^ 1][i]);
        ft = ferGAIndivFitness(ga, ga->pop[ga->pop_cur ^ 1][i]);
        f = ferGAIndivGenotype(ga, ga->pop[ga->pop_cur][ga->presel[i]]);

        memcpy(t, f, ga->params.gene_size * ga->params.genotype_size);
        ga->ops.eval(ga, t, ft, ga->ops.eval_data);
    }

    return len;
}

static void __ferGAStepTask(int id, void *data, const fer_tasks_thinfo_t *thinfo)
{
    fer_ga_t *ga = (fer_ga_t *)data;
    _ferGAStep(ga, ga->tfrom, ga->tto);
}

static void _ferGARunThreads(fer_ga_t *ga)
{
    unsigned long cb = 0UL;

    fer_tasks_t *tasks;
    fer_ga_t *gas;
    int i, poplen, popfrom;

    gas = FER_ALLOC_ARR(fer_ga_t, ga->params.threads);
    for (i = 0; i < ga->params.threads; i++){
        gas[i] = *ga;

        // set up thread specific data
        gas[i].tid   = i;

        // alloc temporary memory
        gas[i].gt[0] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
        gas[i].gt[1] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
        gas[i].ft[0] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);
        gas[i].ft[1] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);

        // prepare array for random numbers
        gas[i].trand_max = ga->params.pop_size * 5;
        gas[i].trand = FER_ALLOC_ARR(fer_real_t, gas[i].trand_max);
        gas[i].trand_next = 0;
        __ferGATRandRefill(&gas[i]);
    }

    // create thread pool
    tasks = ferTasksNew(ga->params.threads);
    ferTasksRun(tasks);


    // initialize individuals
    _ferGAInit(&gas[0], 0, ga->params.pop_size);

    while (!ga->ops.terminate(ga, ga->ops.terminate_data)){
        cb += 1UL;
        if (cb == ga->ops.callback_period && ga->ops.callback){
            ga->ops.callback(ga, ga->ops.callback_data);
            cb = 0UL;
        }

        // preselect
        popfrom = _ferGAPreselect(ga);

        // one step of breeding
        poplen  = (ga->params.pop_size - popfrom) / ga->params.crossover_size;
        poplen /= ga->params.threads;
        poplen *= ga->params.crossover_size;

        for (i = 0; i < ga->params.threads; i++){
            // set up range for specific thread
            gas[i].tfrom = (i * poplen) + popfrom;
            gas[i].tto   = gas[i].tfrom + poplen;
            if (i == ga->params.threads - 1)
                gas[i].tto = ga->params.pop_size;

            ferTasksAdd(tasks, __ferGAStepTask, i, (void *)&gas[i]);
        }
        ferTasksBarrier(tasks);

        ga->pop_cur ^= 1;
        for (i = 0; i < ga->params.threads; i++){
            gas[i].pop_cur ^= 1;
        }
    }


    // delete thread pool
    ferTasksDel(tasks);

    for (i = 0; i < ga->params.threads; i++){
        // free tmp memory
        FER_FREE(gas[i].gt[0]);
        FER_FREE(gas[i].gt[1]);
        FER_FREE(gas[i].ft[0]);
        FER_FREE(gas[i].ft[1]);

        gas[i].gt[0] = gas[i].gt[1] = NULL;
        gas[i].ft[0] = gas[i].ft[1] = NULL;

        FER_FREE(gas[i].trand);
    }

    FER_FREE(gas);
}

static void _ferGARun1(fer_ga_t *ga)
{
    unsigned long cb = 0UL;
    size_t popfrom;

    // alloc temporary memory
    ga->gt[0] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
    ga->gt[1] = FER_ALLOC_ARR(void *, ga->params.crossover_size);
    ga->ft[0] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);
    ga->ft[1] = FER_ALLOC_ARR(fer_real_t *, ga->params.crossover_size);

    // initialize individuals
    _ferGAInit(ga, 0, ga->params.pop_size);

    do {
        cb += 1UL;
        if (cb == ga->ops.callback_period && ga->ops.callback){
            ga->ops.callback(ga, ga->ops.callback_data);
            cb = 0UL;
        }

        // preselect
        popfrom = _ferGAPreselect(ga);

        // one step of breeding
        _ferGAStep(ga, popfrom, ga->params.pop_size);

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
