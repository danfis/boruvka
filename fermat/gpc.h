/***
 * fermat
 * -------
 * Copyright (c)2012 Daniel Fiser <danfis@danfis.cz>
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

#ifndef __FER_GPC_H__
#define __FER_GPC_H__

#include <fermat/core.h>
#include <fermat/rand-mt.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _fer_gpc_t;
struct _fer_gpc_tree_t;
struct _fer_gpc_pred_t;
struct _fer_gpc_class_t;


/**
 * Genetic Programming for Data Classification
 * ============================================
 *
 */


/**
 * Operators
 * ----------
 *
 * See fer_gpc_ops_t.
 */

/** vvvv */

/**
 * Returns fitness of a decision tree.
 * The argument {class} is array of classification results per data row.
 */
typedef fer_real_t (*fer_gpc_fitness)(struct _fer_gpc_t *gpc, int *class, void *);

/**
 * Ought to return {i}'th row from dataset
 */
typedef void *(*fer_gpc_data_row)(struct _fer_gpc_t *gpc, int i, void *);

/**
 * Callback that is called periodically every .callback_period'th cycle.
 */
typedef void (*fer_gpc_callback)(struct _fer_gpc_t *gpc, void *);
/** ^^^^ */

struct _fer_gpc_ops_t {
    fer_gpc_fitness fitness;       /*!< Default: NULL, must be defined */
    fer_gpc_data_row data_row;     /*!< Default: NULL, must be defined */

    fer_gpc_callback callback;     /*!< Default: NULL */
    unsigned long callback_period; /*!< Default: 0, i.e., never */

    void *data; /*!< Data pointer that will be provided to all callbacks if
                     not specified otherwise. */
    void *fitness_data;
    void *data_row_data;
    void *callback_data;
};
typedef struct _fer_gpc_ops_t fer_gpc_ops_t;

/**
 * Initializes operators to default values
 */
void ferGPCOpsInit(fer_gpc_ops_t *ops);



/**
 * Parameters
 * -----------
 */
struct _fer_gpc_params_t {
    size_t pop_size;         /*!< Size of population. Default: 1 */
    size_t max_depth;        /*!< Maximal depth of a tree. Default: 5 */
    size_t keep_best;        /*!< Number of best individuals that ought to
                                  be reproduced to next population
                                  preferentialy. Default: 1 */
    size_t throw_worst;      /*!< Number of best individuals that are
                                  thrown away preferentialy. Default: 1 */
    unsigned long max_steps; /*!< Maximal number of steps of algorithm.
                                  Default: 10 */
    size_t data_rows;        /*!< Number of data rows. Default: 0 */

    size_t tournament_size;  /*!< Number of individuals that enter
                                  tournament selection. Default: 5 */

    /* Probabilities of undergone actions. Any numbers can be used because
     * all will be normized to (pr + pc + pm) = 1 */
    fer_real_t pr; /*!< Probability of reproduction. Default: 14 */
    fer_real_t pc; /*!< Probability of crossover. Default: 85 */
    fer_real_t pm; /*!< Probability of mutation. Default: 1 */

    unsigned long simplify; /*!< A simplification of the trees will be
                                 executed every {simplify} step.
                                 Default: 0 (i.e., never) */
    unsigned long prune_deep; /*!< Prunes all trees that exceeds max_depth
                                   every specified step. Default: 0 */
};
typedef struct _fer_gpc_params_t fer_gpc_params_t;

/**
 * Initialize parameters to default values
 */
void ferGPCParamsInit(fer_gpc_params_t *params);



/**
 * Functions
 * ----------
 */

#define FER_GPC_PRED_INIT_SIZE 10
#define FER_GPC_CLASS_INIT_SIZE 10

struct _fer_gpc_t {
    fer_gpc_params_t params;
    fer_gpc_ops_t ops;

    fer_rand_mt_t *rand;

    struct _fer_gpc_tree_t **pop[3]; /*!< Population (actual and two tmps) */
    size_t pop_size[2];              /*!< Current size of populations */
    int pop_cur;                     /*!< Idx of current population array */

    struct _fer_gpc_pred_t *pred;    /*!< List of predicates */
    size_t pred_size, pred_len;
    struct _fer_gpc_class_t *class;  /*!< List of classes */
    size_t class_size, class_len;

    int *eval_results; /*!< Array of results from evaluation */

    struct {
        unsigned long elapsed; /*!< Number of elapsed steps */
    } stats;
};
typedef struct _fer_gpc_t fer_gpc_t;

/**
 * Creates a new instance of GPC
 */
fer_gpc_t *ferGPCNew(const fer_gpc_ops_t *ops, const fer_gpc_params_t *params);

/**
 * Deletes GPC
 */
void ferGPCDel(fer_gpc_t *gpc);


/**
 * Predicate callback.
 * Should return the number of next descendant node in evaluating a
 * decision tree.
 */
typedef unsigned int (*fer_gpc_pred)(fer_gpc_t *gpc, void *mem, void *data, void *userdata);

/**
 * Initialize callback a predicate data
 */
typedef void (*fer_gpc_pred_init)(fer_gpc_t *gpc, void *mem, void *userdata);

/**
 * Format a callback into given string.
 */
typedef void (*fer_gpc_pred_format)(fer_gpc_t *gpc, void *mem, void *userdata,
                                    char *str, size_t str_maxlen);

/**
 * Adds a new predicate.
 * TODO
 */
int ferGPCAddPred(fer_gpc_t *gpc,
                  fer_gpc_pred pred,
                  fer_gpc_pred_init init,
                  fer_gpc_pred_format format,
                  unsigned int num_descendants, size_t memsize,
                  void *userdata);

/**
 * Adds a new class (terminal).
 */
int ferGPCAddClass(fer_gpc_t *gpc, int class_id);



/**
 * Run GPC algorithm
 */
int ferGPCRun(fer_gpc_t *gpc);

/**
 * Returns fitness of the best individual
 */
fer_real_t ferGPCBestFitness(const fer_gpc_t *gpc);

void ferGPCPrintBest(fer_gpc_t *gpc, FILE *fout);

/**
 * Returns random number from range
 */
_fer_inline fer_real_t ferGPCRand(fer_gpc_t *gpc, fer_real_t f, fer_real_t t);

/**
 * Returns random number [0, 1)
 */
_fer_inline fer_real_t ferGPCRand01(fer_gpc_t *gpc);

/**
 * Returns random integer number [f, t)
 */
_fer_inline int ferGPCRandInt(fer_gpc_t *gpc, int f, int t);


/**
 * Statistics
 * -----------
 */
struct _fer_gpc_stats_t {
    fer_real_t min_fitness;
    fer_real_t max_fitness;
    fer_real_t avg_fitness;
    fer_real_t med_fitness; /*!< median */

    unsigned int min_nodes;
    unsigned int max_nodes;
    fer_real_t avg_nodes;

    unsigned int min_depth;
    unsigned int max_depth;
    fer_real_t avg_depth;

    unsigned long elapsed; /*!< Number of elapsed steps */
};
typedef struct _fer_gpc_stats_t fer_gpc_stats_t;

/**
 * Fills given structure with statistics about from current population
 */
void ferGPCStats(const fer_gpc_t *gpc, fer_gpc_stats_t *stats);



size_t __ferGPCPredMemsize(const fer_gpc_t *gpc, unsigned int idx);

/**** INLINES ****/
_fer_inline fer_real_t ferGPCRand(fer_gpc_t *gpc, fer_real_t f, fer_real_t t)
{
    return ferRandMT(gpc->rand, f, t);
}

_fer_inline fer_real_t ferGPCRand01(fer_gpc_t *gpc)
{
    return ferRandMT01(gpc->rand);
}

_fer_inline int ferGPCRandInt(fer_gpc_t *gpc, int f, int t)
{
    return FER_MIN(ferRandMT(gpc->rand, f, t), t - 1);
}

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __FER_GPC_H__ */

