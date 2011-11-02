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
 * Performs a crossover between {ing1} and {ing2} and put the results into
 * {outg1} and {outg2}.
 * Note that {outg2} can be NULL and thus must be avoided.
 */
typedef void (*fer_ga_crossover)(struct _fer_ga_t *ga,
                                 void *ing1, void *ing2, void *outg1, void *outg2,
                                 void *data);

/**
 * Performs a mutation on genotype.
 */
typedef void (*fer_ga_mutate)(struct _fer_ga_t *ga, void *genotype, void *data);

/**
 * Callback that is peridically called from GNG.
 *
 * It is called every .callback_period'th added node.
 */
typedef void (*fer_ga_callback)(struct _fer_ga_t *ga, void *);

struct _fer_ga_ops_t {
    fer_ga_eval      eval;
    fer_ga_terminate terminate;
    fer_ga_init      init;
    fer_ga_sel       sel;
    fer_ga_crossover crossover;
    fer_ga_mutate    mutate;

    fer_ga_callback callback;
    unsigned long callback_period;

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */

    void *eval_data;
    void *terminate_data;
    void *init_data;
    void *sel_data;
    void *crossover_data;
    void *mutate_data;
    void *callback_data;
};
typedef struct _fer_ga_ops_t fer_ga_ops_t;


/**
 * Initialize ops to default values
 */
void ferGAOpsInit(fer_ga_ops_t *ops);



struct _fer_ga_params_t {
    fer_real_t pc;        /*!< Probability of crossover */
    fer_real_t pm;        /*!< Probability of mutation */
    size_t gene_size;     /*!< Size of one gene (in bytes) */
    size_t genotype_size; /*!< Number of genes in genotype */
    size_t pop_size;      /*!< Size of population */
    size_t fitness_size;  /*!< Number of fitness values */
};
typedef struct _fer_ga_params_t fer_ga_params_t;


/**
 * Initialize params to default values
 */
void ferGAParamsInit(fer_ga_params_t *p);


/**
 * Initialize ops and params to operate on int gene.
 */
void ferGAOpsParamsInt(fer_ga_ops_t *ops, fer_ga_params_t *params,
                       size_t genotype_size, size_t pop_size);

struct _fer_ga_t {
    fer_ga_params_t params;
    fer_ga_ops_t ops;

    void **pop[2];   /*!< Population */
    size_t pop_cur; /*!< Index of current population */

    fer_rand_mt_t *rand;
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


_fer_inline fer_real_t *ferGAIndivFitness(fer_ga_t *ga, void *indiv);
_fer_inline void *ferGAIndivGenotype(fer_ga_t *ga, void *indiv);
_fer_inline void *ferGAIndivGene(fer_ga_t *ga, void *indiv, size_t genepos);


/**** INLINES ****/
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

