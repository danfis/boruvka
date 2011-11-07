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

#ifndef __FER_GA_H__
#define __FER_GA_H__

#include <pthread.h>
#include <fermat/core.h>
#include <fermat/rand-mt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Genetic Algorithm
 * ==================
 *
 */

struct _fer_ga_t;


/**
 * Returns true if algorithm should terminate.
 */
typedef int (*fer_ga_terminate)(struct _fer_ga_t *ga, void *);

/**
 * Evaluates genotype and puts a fitness values into {fitness}
 */
typedef void (*fer_ga_eval)(struct _fer_ga_t *ga, void *genotype,
                            fer_real_t *fitness, void *data);

/**
 * Initializes genotype
 */
typedef void (*fer_ga_init)(struct _fer_ga_t *ga, void *genotype, void *data);

/**
 * Returns index of selected individual from population
 */
typedef size_t (*fer_ga_sel)(struct _fer_ga_t *ga, void *data);

/**
 * Performs a crossover.
 * Note that {outg[i]} can be NULL and thus must be avoided.
 */
typedef void (*fer_ga_crossover)(struct _fer_ga_t *ga,
                                 void **ing, void **outg, void *data);

/**
 * Performs a mutation on genotype.
 */
typedef void (*fer_ga_mutate)(struct _fer_ga_t *ga, void *genotype, void *data);

/**
 * Fills {sel} array with IDs of selected individuals and returns number of
 * selected individuals. This number must be maximally
 * params.presel_max.
 */
typedef size_t (*fer_ga_presel)(struct _fer_ga_t *ga, size_t *sel, void *data);

/**
 * Callback that is peridically called from GNG.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_ga_callback)(struct _fer_ga_t *ga, void *);

struct _fer_ga_ops_t {
    fer_ga_eval      eval;      /*!< Default: NULL */
    fer_ga_terminate terminate; /*!< Default: NULL */
    fer_ga_init      init;      /*!< Default: NULL */
    fer_ga_sel       sel;       /*!< Default: ferGASelTournament2 */
    fer_ga_crossover crossover; /*!< Default: ferGACrossover2 */
    fer_ga_mutate    mutate;    /*!< Default: ferGAMutateNone */
    fer_ga_presel    presel;    /*!< Default: NULL */

    fer_ga_callback callback;      /*!< Default: NULL */
    unsigned long callback_period; /*!< Default: 0, i.e. never */

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *eval_data;
    void *terminate_data;
    void *init_data;
    void *sel_data;
    void *crossover_data;
    void *mutate_data;
    void *presel_data;
    void *callback_data;
};
typedef struct _fer_ga_ops_t fer_ga_ops_t;


/**
 * Initialize ops to default values
 */
void ferGAOpsInit(fer_ga_ops_t *ops);



struct _fer_ga_params_t {
    fer_real_t pc;         /*!< Probability of crossover. Default: 0.7 */
    fer_real_t pm;         /*!< Probability of mutation. Default: 0.001 */
    size_t gene_size;      /*!< Size of one gene (in bytes). Default: 1 */
    size_t genotype_size;  /*!< Number of genes in genotype. Default: 1 */
    size_t pop_size;       /*!< Size of population. Default: 1 */
    size_t fitness_size;   /*!< Number of fitness values. Default: 1 */
    size_t crossover_size; /*!< Number of individuals selected for crossover.
                                Default: 2 */
    size_t presel_max;     /*!< Maximal number of preselected individuals.
                                Default: 10 */
    int threads;           /*!< Number of threads. Default: 1 */
};
typedef struct _fer_ga_params_t fer_ga_params_t;


/**
 * Initialize params to default values
 */
void ferGAParamsInit(fer_ga_params_t *p);


struct _fer_ga_t {
    fer_ga_params_t params;
    fer_ga_ops_t ops;

    void **pop[2];   /*!< Population */
    size_t pop_cur; /*!< Index of current population */
    void **gt[2];
    fer_real_t **ft[2];
    size_t *presel;

    fer_rand_mt_t *rand;


    int tid; /*!< Thread ID, in case threads are used */
    int tfrom, tto;
    pthread_mutex_t tlock;
    fer_real_t *trand;
    size_t trand_next, trand_max;
};
typedef struct _fer_ga_t fer_ga_t;


/**
 * Creates new instance of GA
 */
fer_ga_t *ferGANew(const fer_ga_ops_t *ops, const fer_ga_params_t *params);

/**
 * Deletes GA
 */
void ferGADel(fer_ga_t *ga);

/**
 * Runs genetic algorithm
 */
void ferGARun(fer_ga_t *ga);

/**
 * Returns random number from range
 */
_fer_inline fer_real_t ferGARand(fer_ga_t *ga, fer_real_t f, fer_real_t t);

/**
 * Returns random number [0, 1)
 */
_fer_inline fer_real_t ferGARand01(fer_ga_t *ga);

/**
 * Returns random integer number [f, t)
 */
_fer_inline int ferGARandInt(fer_ga_t *ga, int f, int t);

/**
 * Returns crossover probability
 */
_fer_inline fer_real_t ferGAParamPc(const fer_ga_t *ga);

/**
 * Returns mutation probability
 */
_fer_inline fer_real_t ferGAParamPm(const fer_ga_t *ga);

/**
 * Returns gene size (in bytes).
 */
_fer_inline size_t ferGAParamGeneSize(const fer_ga_t *ga);

/**
 * Returns genotype size, i.e., number of genes per genotype.
 */
_fer_inline size_t ferGAParamGenotypeSize(const fer_ga_t *ga);

/**
 * Returns population size
 */
_fer_inline size_t ferGAParamPopSize(const fer_ga_t *ga);

/**
 * Returns fitness size, i.e., number of fitness values.
 */
_fer_inline size_t ferGAParamFitnessSize(const fer_ga_t *ga);

/**
 * Returns crossover size, i.e., number of individuals that undergo a
 * crossover operator.
 */
_fer_inline size_t ferGAParamCrossoverSize(const fer_ga_t *ga);



/**
 * Returns i'th individual
 */
_fer_inline void *ferGAIndiv(fer_ga_t *ga, size_t i);

/**
 * Returns pointer to fitness array of the individual
 */
_fer_inline fer_real_t *ferGAIndivFitness(fer_ga_t *ga, void *indiv);

/**
 * Returns pointer to genotype array of the individual
 */
_fer_inline void *ferGAIndivGenotype(fer_ga_t *ga, void *indiv);

/**
 * Returns pointer to specific gene of the individual
 */
_fer_inline void *ferGAIndivGene(fer_ga_t *ga, void *indiv, size_t genepos);




/**
 * Tournament selection of two individuals
 */
size_t ferGASelTournament2(fer_ga_t *ga, void *data);

/**
 * Tournament selection of three individuals
 */
size_t ferGASelTournament3(fer_ga_t *ga, void *data);

/**
 * Crossover operator between two individuals
 */
void ferGACrossover2(fer_ga_t *ga, void **ing, void **outg, void *data);


/**
 * No mutation
 */
void ferGAMutateNone(fer_ga_t *ga, void *gt, void *data);


/**
 * Preselect operator that selects params.presel_max best individuals
 * (based on first fitness value)
 */
size_t ferGAPreselElite(fer_ga_t *ga, size_t *sel, void *data);



void __ferGATRandRefill(fer_ga_t *ga);

/**** INLINES ****/
_fer_inline fer_real_t ferGARand(fer_ga_t *ga, fer_real_t f, fer_real_t t)
{
    if (ga->tid == -1)
        return ferRandMT(ga->rand, f, t);

    if (ga->trand_next == ga->trand_max)
        __ferGATRandRefill(ga);
    return (ga->trand[ga->trand_next++] * (t - f)) + f;
}

_fer_inline fer_real_t ferGARand01(fer_ga_t *ga)
{
    if (ga->tid == -1)
        return ferRandMT01(ga->rand);
    if (ga->trand_next == ga->trand_max)
        __ferGATRandRefill(ga);
    return ga->trand[ga->trand_next++];
}

_fer_inline int ferGARandInt(fer_ga_t *ga, int f, int t)
{
    int v;
    v = ferGARand(ga, f, t);
    return (v < t ? v : t - 1);
}

_fer_inline fer_real_t ferGAParamPc(const fer_ga_t *ga)
{
    return ga->params.pc;
}

_fer_inline fer_real_t ferGAParamPm(const fer_ga_t *ga)
{
    return ga->params.pm;
}

_fer_inline size_t ferGAParamGeneSize(const fer_ga_t *ga)
{
    return ga->params.gene_size;
}

_fer_inline size_t ferGAParamGenotypeSize(const fer_ga_t *ga)
{
    return ga->params.genotype_size;
}

_fer_inline size_t ferGAParamPopSize(const fer_ga_t *ga)
{
    return ga->params.pop_size;
}

_fer_inline size_t ferGAParamFitnessSize(const fer_ga_t *ga)
{
    return ga->params.fitness_size;
}

_fer_inline size_t ferGAParamCrossoverSize(const fer_ga_t *ga)
{
    return ga->params.crossover_size;
}


_fer_inline void *ferGAIndiv(fer_ga_t *ga, size_t i)
{
    return ga->pop[ga->pop_cur][i];
}

_fer_inline fer_real_t *ferGAIndivFitness(fer_ga_t *ga, void *indiv)
{
    return (fer_real_t *)indiv;
}

_fer_inline void *ferGAIndivGenotype(fer_ga_t *ga, void *indiv)
{
    char *ind;
    size_t fitness_size;

    fitness_size = sizeof(fer_real_t) * ga->params.fitness_size;
    ind = ((char *)indiv) + fitness_size;

    return (void *)ind;
}

_fer_inline void *ferGAIndivGene(fer_ga_t *ga, void *indiv, size_t genepos)
{
    char *ind;

    ind = (char *)ferGAIndivGenotype(ga, indiv);
    ind = ind + ga->params.gene_size * genepos;

    return (void *)ind;
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GA_H__ */

