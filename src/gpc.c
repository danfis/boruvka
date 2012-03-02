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

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>
#include <fermat/sort.h>
#include <fermat/gpc.h>
#include <fermat/gpc-tree.h>


/**
 * Definition of predicate
 */
struct _fer_gpc_pred_t {
    fer_gpc_pred pred;      /*!< Predicate callback */
    fer_gpc_pred_init init; /*!< Initialization callback */
    fer_gpc_pred_format format; /*!< Format callback */
    size_t memsize;         /*!< Size of allocated memory per predicate instance */
    int ndesc;              /*!< Number of descendants */
    void *data;             /*!< User data */
};
typedef struct _fer_gpc_pred_t fer_gpc_pred_t;

/**
 * Definition of class
 */
struct _fer_gpc_class_t {
    int class; /*!< Class identifier */
};
typedef struct _fer_gpc_class_t fer_gpc_class_t;


/** Randomly generate a node with its subtree */
static fer_gpc_node_t *ferGPCGenTree(fer_gpc_t *gpc, int depth, int max_depth);
static fer_gpc_node_t *ferGPCGenClass(fer_gpc_t *gpc);
/** Create randomly generated initial population in gpc->pop[pop] */
static void ferGPCCreateInitPop(fer_gpc_t *gpc, int pop);
/** Evals a tree with one data row. Returns resulting class as defined by
 *  ferGPCAddClass() function. */
static int ferGPCEvalTreeClass(fer_gpc_t *gpc, fer_gpc_tree_t *tree, void *data);
/** Evaluate a tree on all data rows and records a new fitness that is also
 *  returned */
static fer_real_t ferGPCEvalTree(fer_gpc_t *gpc, fer_gpc_tree_t *tree);
/** Evaluate whole population of decision trees */
static void ferGPCEvalPop(fer_gpc_t *gpc, int pop);
/** Performs elitism */
static void ferGPCKeepBest(fer_gpc_t *gpc, int from_pop, int to_pop);
/** Throw away the worst individuals from given population */
static void ferGPCThrowWorst(fer_gpc_t *gpc, int pop);
/** Create a new population */
static void ferGPCCreateNewPop(fer_gpc_t *gpc, int from_pop, int to_pop);
/** Reset speficied population, i.e., delete remaining individuals and
 *  truncate it to zero length. */
static void ferGPCResetPop(fer_gpc_t *gpc, int pop);
/** Tournament selection */
static int ferGPCSelectionTournament(fer_gpc_t *gpc, int tour_size,
                                     fer_gpc_tree_t **pop, int pop_size);
/** Genetic operators: */
static void ferGPCReproduction(fer_gpc_t *gpc, int from_pop, int to_pop);
static void ferGPCCrossover(fer_gpc_t *gpc, int from_pop, int to_pop);
static void ferGPCMutation(fer_gpc_t *gpc, int from_pop, int to_pop);
/** Reproduce a particular tree */
static void ferGPCReproduction2(fer_gpc_t *gpc, int to_pop, fer_gpc_tree_t *tree);

/** Simplifies all trees in speficied population */
static void ferGPCSimplify(fer_gpc_t *gpc, int pop);
static void ferGPCPruneDeep(fer_gpc_t *gpc, int pop);


#define OPS_DATA(name) \
    if (!gpc->ops.name ## _data) \
        gpc->ops.name ## _data = gpc->ops.data;

#define OPS_CHECK(name) \
    if (!gpc->ops.name){ \
        fprintf(stderr, "Fermat :: GPC :: No ." #name " callback set.\n"); \
        exit(-1); \
    }

#define OPS_CHECK_DATA(name) \
    OPS_DATA(name) \
    OPS_CHECK(name)


void ferGPCOpsInit(fer_gpc_ops_t *ops)
{
    memset(ops, 0, sizeof(*ops));
}

void ferGPCParamsInit(fer_gpc_params_t *params)
{
    params->pop_size    = 1;
    params->max_depth   = 5;
    params->keep_best   = 1;
    params->throw_worst = 1;
    params->max_steps   = 10UL;
    params->data_rows   = 0;

    params->tournament_size = 5;

    params->pr        = 14;
    params->pc        = 85;
    params->pm        = 1;

    params->simplify = 0UL;
    params->prune_deep = 0UL;
}



fer_gpc_t *ferGPCNew(const fer_gpc_ops_t *ops, const fer_gpc_params_t *params)
{
    fer_gpc_t *gpc;
    fer_real_t prob;

    gpc = FER_ALLOC(fer_gpc_t);

    gpc->params = *params;
    prob = gpc->params.pr + gpc->params.pc + gpc->params.pm;
    gpc->params.pr = gpc->params.pr / prob;
    gpc->params.pc = gpc->params.pc / prob;
    gpc->params.pm = gpc->params.pm / prob;

    gpc->ops = *ops;
    OPS_CHECK_DATA(fitness);
    OPS_CHECK_DATA(data_row);
    OPS_DATA(callback);

    //gpc->rand = ferRandMTNewAuto();
    gpc->rand = ferRandMTNew(9999);

    gpc->pop[0] = FER_ALLOC_ARR(fer_gpc_tree_t *, gpc->params.pop_size);
    gpc->pop[1] = FER_ALLOC_ARR(fer_gpc_tree_t *, gpc->params.pop_size);
    gpc->pop[2] = FER_ALLOC_ARR(fer_gpc_tree_t *, gpc->params.pop_size);
    memset(gpc->pop[0], 0, sizeof(fer_gpc_tree_t *) * gpc->params.pop_size);
    memset(gpc->pop[1], 0, sizeof(fer_gpc_tree_t *) * gpc->params.pop_size);
    gpc->pop_size[0] = gpc->pop_size[2] = 0;

    gpc->pred_size = FER_GPC_PRED_INIT_SIZE;
    gpc->pred_len  = 0;
    gpc->pred      = FER_ALLOC_ARR(fer_gpc_pred_t, gpc->pred_size);

    gpc->class_size = FER_GPC_CLASS_INIT_SIZE;
    gpc->class_len  = 0;
    gpc->class      = FER_ALLOC_ARR(fer_gpc_class_t, gpc->class_size);

    gpc->eval_results = FER_ALLOC_ARR(int, gpc->params.data_rows);

    return gpc;
}

void ferGPCDel(fer_gpc_t *gpc)
{
    int i;

    ferRandMTDel(gpc->rand);

    for (i = 0; i < gpc->params.pop_size; i++){
        if (gpc->pop[0][i] != NULL)
            ferGPCTreeDel(gpc->pop[0][i]);
        if (gpc->pop[1][i] != NULL)
            ferGPCTreeDel(gpc->pop[1][i]);
    }
    FER_FREE(gpc->pop[0]);
    FER_FREE(gpc->pop[1]);
    FER_FREE(gpc->pop[2]);

    FER_FREE(gpc->pred);
    FER_FREE(gpc->class);
    FER_FREE(gpc->eval_results);
    FER_FREE(gpc);
}

int ferGPCAddPred(fer_gpc_t *gpc,
                  fer_gpc_pred pred,
                  fer_gpc_pred_init init,
                  fer_gpc_pred_format format,
                  int num_descendants, size_t memsize,
                  void *data)
{
    if (gpc->pred_len >= gpc->pred_size){
        gpc->pred_size *= 2;
        gpc->pred = FER_REALLOC_ARR(gpc->pred, fer_gpc_pred_t, gpc->pred_size);
    }

    gpc->pred[gpc->pred_len].pred    = pred;
    gpc->pred[gpc->pred_len].init    = init;
    gpc->pred[gpc->pred_len].format  = format;
    gpc->pred[gpc->pred_len].memsize = memsize;
    gpc->pred[gpc->pred_len].ndesc   = num_descendants;
    gpc->pred[gpc->pred_len].data    = data;
    gpc->pred_len++;

    return 0;
}

int ferGPCAddClass(fer_gpc_t *gpc, int class_id)
{
    if (gpc->class_len >= gpc->class_size){
        gpc->class_size *= 2;
        gpc->class = FER_REALLOC_ARR(gpc->class, fer_gpc_class_t, gpc->class_size);
    }

    gpc->class[gpc->class_len].class = class_id;
    gpc->class_len++;

    return 0;
}

int __ferGPCPredMemsize(const fer_gpc_t *gpc, int idx)
{
    return gpc->pred[idx].memsize;
}



int ferGPCRun(fer_gpc_t *gpc)
{
    unsigned long step, cb, simplify, prune_deep;
    int pop_cur, pop_other, pop_tmp;

    // early exit if we don't have any classes
    if (gpc->class_len == 0)
        return -1;

    // initialize stats
    memset(&gpc->stats, 0, sizeof(gpc->stats));


    gpc->pop_cur = pop_cur = 0;
    pop_other = 1;


    // create initial population
    ferGPCCreateInitPop(gpc, pop_cur);

    // evaluate initial population
    ferGPCEvalPop(gpc, pop_cur);

    if (gpc->ops.callback){
        gpc->ops.callback(gpc, gpc->ops.callback_data);
    }

    cb = 0UL;
    simplify = 0UL;
    prune_deep = 0UL;
    for (step = 0UL; step < gpc->params.max_steps; step += 1UL){
        // perform elitism and the opposite
        ferGPCKeepBest(gpc, pop_cur, pop_other);
        ferGPCThrowWorst(gpc, pop_cur);

        // create a new population
        ferGPCCreateNewPop(gpc, pop_cur, pop_other);

        // reset the old population
        ferGPCResetPop(gpc, pop_cur);

        // prune deep trees
        prune_deep += 1UL;
        if (prune_deep == gpc->params.prune_deep){
            ferGPCPruneDeep(gpc, pop_other);
            prune_deep = 0UL;
        }

        // simplify a new population
        simplify += 1UL;
        if (simplify == gpc->params.simplify){
            ferGPCSimplify(gpc, pop_other);
            simplify = 0UL;
        }

        // evaluate a new population
        ferGPCEvalPop(gpc, pop_other);

        // switch old and new population
        FER_SWAP(pop_cur, pop_other, pop_tmp);
        gpc->pop_cur = pop_cur;


        // update stats
        gpc->stats.elapsed = step + 1;

        cb += 1UL;
        if (cb == gpc->ops.callback_period && gpc->ops.callback){
            gpc->ops.callback(gpc, gpc->ops.callback_data);
            cb = 0UL;
        }
    }

    // simplify resulting population
    ferGPCSimplify(gpc, gpc->pop_cur);
    ferGPCEvalTree(gpc, gpc->pop[gpc->pop_cur][0]);

    return 0;
}

fer_real_t ferGPCBestFitness(const fer_gpc_t *gpc)
{
    if (gpc->pop[gpc->pop_cur][0])
        return gpc->pop[gpc->pop_cur][0]->fitness;
    return -FER_REAL_MAX;
}

void *ferGPCTree(const fer_gpc_t *gpc, int i)
{
    i = FER_MIN(i, gpc->pop_size[gpc->pop_cur] - 1);
    return (void *)gpc->pop[gpc->pop_cur][i];
}

int ferGPCTreeEval(fer_gpc_t *gpc, void *tree, void *data)
{
    return ferGPCEvalTreeClass(gpc, (fer_gpc_tree_t *)tree, data);
}



static void printBest(fer_gpc_t *gpc, fer_gpc_node_t *node, FILE *fout,
                      char *str, size_t str_maxlen, int depth)
{
    fer_gpc_node_t **desc;
    int i;

    str[0] = 0x0;

    for (i = 0; i < depth; i++){
        fprintf(fout, "  ");
    }

    fprintf(fout, "(");

    if (node->ndesc == 0){
        // TODO
        fprintf(fout, "%d", gpc->class[node->idx].class);
    }else{
        if (gpc->pred[node->idx].format){
            gpc->pred[node->idx].format(gpc, FER_GPC_NODE_MEM(node),
                                        gpc->pred[node->idx].data,
                                        str, str_maxlen);
            fprintf(fout, "%s", str);
        }else{
            fprintf(fout, "PRED");
        }

        desc = FER_GPC_NODE_DESC(node);
        for (i = 0; i < node->ndesc; i++){
            fprintf(fout, "\n");
            printBest(gpc, desc[i], fout, str, str_maxlen, depth + 1);
        }

    }

    fprintf(fout, ")");
}
void ferGPCPrintBest(fer_gpc_t *gpc, FILE *fout)
{
    char str[1024];

    if (!gpc->pop[gpc->pop_cur][0])
        return;

    printBest(gpc, gpc->pop[gpc->pop_cur][0]->root, fout, str, 1024, 0);
    fprintf(fout, "\n");
}

void ferGPCStats(const fer_gpc_t *gpc, fer_gpc_stats_t *stats)
{
    int i, pop;
    fer_gpc_tree_t *tree;

    pop = gpc->pop_cur;

    stats->min_fitness = FER_REAL_MAX;
    stats->max_fitness = -FER_REAL_MAX;
    stats->avg_fitness = FER_ZERO;
    stats->min_nodes = INT_MAX;
    stats->max_nodes = 0;
    stats->avg_nodes = 0;
    stats->min_depth = INT_MAX;
    stats->max_depth = 0;
    stats->avg_depth = 0;
    for (i = 0; i < gpc->pop_size[pop]; i++){
        tree = gpc->pop[pop][i];

        if (tree->fitness > stats->max_fitness)
            stats->max_fitness = tree->fitness;
        if (tree->fitness < stats->min_fitness)
            stats->min_fitness = tree->fitness;
        stats->avg_fitness += tree->fitness;

        if (tree->num_nodes < stats->min_nodes)
            stats->min_nodes = tree->num_nodes;
        if (tree->num_nodes > stats->max_nodes)
            stats->max_nodes = tree->num_nodes;
        stats->avg_nodes += tree->num_nodes;

        if (tree->depth < stats->min_depth)
            stats->min_depth = tree->depth;
        if (tree->depth > stats->max_depth)
            stats->max_depth = tree->depth;
        stats->avg_depth += tree->depth;

    }
    stats->avg_fitness /= gpc->pop_size[pop];
    stats->avg_nodes /= gpc->pop_size[pop];
    stats->avg_depth /= gpc->pop_size[pop];

    stats->med_fitness = gpc->pop[pop][gpc->pop_size[pop] / 2 + 1]->fitness;
    if (gpc->pop_size[pop] % 2 == 0){
        stats->med_fitness += gpc->pop[pop][gpc->pop_size[pop] / 2]->fitness;
        stats->med_fitness /= FER_REAL(2.);
    }

    stats->elapsed = gpc->stats.elapsed;
}


static fer_gpc_node_t *ferGPCGenTree(fer_gpc_t *gpc, int depth, int max_depth)
{
    fer_gpc_node_t *node;
    fer_gpc_node_t **desc;
    int idx, i;

    // Randomly choose a predicate or a class.
    // If max_depth is reached, generate only a class.
    if (depth == max_depth){
        idx = ferGPCRandInt(gpc, 0, gpc->class_len);
        idx += gpc->pred_len;
    }else{
        idx = ferGPCRandInt(gpc, 0, gpc->pred_len + gpc->class_len);
    }

    if (idx >= gpc->pred_len){
        // create a new class node
        idx = idx - gpc->pred_len;
        node = ferGPCNodeNew(idx, 0, 0);

    }else{
        // create a new predicate node
        node = ferGPCNodeNew(idx, gpc->pred[idx].ndesc,
                             gpc->pred[idx].memsize);
        // initialize it
        if (gpc->pred[idx].init){
            gpc->pred[idx].init(gpc, FER_GPC_NODE_MEM(node),
                                gpc->pred[idx].data);
        }

        // and fill descendants
        desc = FER_GPC_NODE_DESC(node);
        for (i = 0; i < node->ndesc; i++){
            desc[i] = ferGPCGenTree(gpc, depth + 1, max_depth);
        }
    }

    return node;
}
static fer_gpc_node_t *ferGPCGenClass(fer_gpc_t *gpc)
{
    int idx;
    idx = ferGPCRandInt(gpc, 0, gpc->class_len);
    return ferGPCNodeNew(idx, 0, 0);
}


static void ferGPCCreateInitPop(fer_gpc_t *gpc, int pop)
{
    int i, len;
    int pop_other;

    pop_other = (pop + 1) % 2;

    gpc->pop_size[pop] = 0;
    gpc->pop_size[pop_other] = 0;

    len = gpc->params.pop_size;
    for (i = 0; i < len; i++){
        gpc->pop[pop][i] = ferGPCTreeNew();
        gpc->pop[pop][i]->root = ferGPCGenTree(gpc, 0, gpc->params.max_depth);
        ferGPCTreeFix(gpc->pop[pop][i]);
        gpc->pop_size[pop]++;

        gpc->pop[pop_other][i] = NULL;

        //ferGPCTreePrint(gpc->pop[0][i], stdout);
    }
}

static int ferGPCEvalTreeClass(fer_gpc_t *gpc, fer_gpc_tree_t *tree, void *data)
{
    fer_gpc_node_t *node;
    fer_gpc_node_t **desc;
    fer_gpc_pred pred;
    void *mem;
    int dispatch;

    // traverse the decision tree
    node = tree->root;
    while (node->ndesc != 0){
        pred = gpc->pred[node->idx].pred;
        mem  = FER_GPC_NODE_MEM(node);

        // find which descendant is used
        dispatch = pred(gpc, mem, data, gpc->pred[node->idx].data);
        dispatch = FER_MIN(dispatch, node->ndesc - 1);

        // descent to next node
        desc = FER_GPC_NODE_DESC(node);
        node = desc[dispatch];
    }

    return gpc->class[node->idx].class;
}

static fer_real_t ferGPCEvalTree(fer_gpc_t *gpc, fer_gpc_tree_t *tree)
{
    int i, class;
    void *data;

    for (i = 0; i < gpc->params.data_rows; i++){
        // obtain a data row using a callback
        data = gpc->ops.data_row(gpc, i, gpc->ops.data_row_data);

        // eval the tree on the data row
        class = ferGPCEvalTreeClass(gpc, tree, data);
        gpc->eval_results[i] = class;
    }

    // compute a fitness value
    tree->fitness = gpc->ops.fitness(gpc, gpc->eval_results,
                                     gpc->ops.fitness_data);

    //ferGPCTreePrint(tree, stdout);

    return tree->fitness;
}

static void ferGPCEvalPop(fer_gpc_t *gpc, int pop)
{
    int i;

    for (i = 0; i < gpc->pop_size[pop]; i++){
        ferGPCEvalTree(gpc, gpc->pop[pop][i]);
    }

    // sort based on fitness
    ferRadixSortPtr((void **)gpc->pop[pop], (void **)gpc->pop[2],
                    gpc->pop_size[pop],
                    fer_offsetof(fer_gpc_tree_t, fitness), 1);
}

static void ferGPCKeepBest(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    int i;

    // copy the best individuals
    for (i = 0; i < gpc->params.keep_best; i++){
        if (gpc->pop[from_pop][i] != NULL)
            ferGPCReproduction2(gpc, to_pop, gpc->pop[from_pop][i]);
    }
}

static void ferGPCThrowWorst(fer_gpc_t *gpc, int pop)
{
    fer_gpc_tree_t *tree;
    int i;

    // throw the worst
    for (i = 0; i < gpc->params.throw_worst; i++){
        tree = gpc->pop[pop][gpc->pop_size[pop] - i - 1];
        if (tree != NULL)
            ferGPCTreeDel(tree);
        gpc->pop[pop][gpc->pop_size[pop] - i - 1] = NULL;
    }
    gpc->pop_size[pop] -= gpc->params.throw_worst;
}


static void ferGPCCreateNewPop(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    fer_real_t action;

    while (gpc->pop_size[to_pop] < gpc->params.pop_size
            && gpc->pop_size[from_pop] > 0){
        // choose action
        action = ferGPCRand01(gpc);

        if (action < gpc->params.pr){
            // reproduction
            ferGPCReproduction(gpc, from_pop, to_pop);
        }else if (action < gpc->params.pr + gpc->params.pc){
            // crossover
            ferGPCCrossover(gpc, from_pop, to_pop);
        }else{
            // mutation
            ferGPCMutation(gpc, from_pop, to_pop);
        }
    }
}

static void ferGPCResetPop(fer_gpc_t *gpc, int pop)
{
    int i;

    for (i = 0; i < gpc->pop_size[pop]; i++){
        if (gpc->pop[pop][i])
            ferGPCTreeDel(gpc->pop[pop][i]);
        gpc->pop[pop][i] = NULL;
    }
    gpc->pop_size[pop] = 0;
}

static int ferGPCSelectionTournament(fer_gpc_t *gpc, int tour_size,
                                     fer_gpc_tree_t **pop, int pop_size)
{
    fer_real_t best_fitness;
    int i, sel, best = 0;

    best_fitness = -FER_REAL_MAX;
    for (i = 0; i < tour_size; i++){
        sel = ferGPCRandInt(gpc, 0, pop_size);
        if (pop[sel]->fitness > best_fitness){
            best = sel;
        }
    }

    return best;
}

static void ferGPCReproduction(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    int idx;

    // select an individual from population
    idx = ferGPCSelectionTournament(gpc, gpc->params.tournament_size,
                                    gpc->pop[from_pop], gpc->pop_size[from_pop]);

    // and copy the individual to other population
    gpc->pop[to_pop][gpc->pop_size[to_pop]] = ferGPCTreeClone(gpc, gpc->pop[from_pop][idx]);
    gpc->pop_size[to_pop]++;
}

static void ferGPCReproduction2(fer_gpc_t *gpc, int to_pop, fer_gpc_tree_t *tree)
{
    gpc->pop[to_pop][gpc->pop_size[to_pop]] = ferGPCTreeClone(gpc, tree);
    gpc->pop_size[to_pop]++;
}

static void ferGPCCrossover(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    int idx[2], node_idx[2];
    int depth[2];
    int i;
    fer_gpc_node_t *node[2];
    fer_gpc_node_t **desc[2];
    fer_gpc_tree_t *tree[2];

    if (gpc->pop_size[from_pop] < 2)
        return;

    // select two individuals from population
    idx[0] = ferGPCSelectionTournament(gpc, gpc->params.tournament_size,
                                       gpc->pop[from_pop], gpc->pop_size[from_pop]);
    do {
        idx[1] = ferGPCSelectionTournament(gpc, gpc->params.tournament_size,
                                           gpc->pop[from_pop], gpc->pop_size[from_pop]);
    } while (idx[0] == idx[1]);

    for (i = 0; i < 2; i++){
        tree[i] = gpc->pop[from_pop][idx[i]];

        // choose crossover nodes
        node_idx[i] = ferGPCRandInt(gpc, 0, tree[i]->num_nodes);

        // obtain those nodes
        node[i] = ferGPCTreeNodeById(tree[i], node_idx[i], &desc[i], &depth[i]);
    }

    // switch the subtrees
    *desc[0] = node[1];
    *desc[1] = node[0];
    ferGPCTreeFix(tree[0]);
    ferGPCTreeFix(tree[1]);


    if (idx[0] < idx[1]){
        FER_SWAP(idx[0], idx[1], i);
    }


    // move the trees to destination population
    for (i = 0; i < 2; i++){
        if (gpc->pop_size[to_pop] >= gpc->params.pop_size){
            // delete a tree if the population is full
            ferGPCTreeDel(tree[i]);
        }else{
            //ferGPCTreePrint(tree[i], stderr);
            gpc->pop[to_pop][gpc->pop_size[to_pop]] = tree[i];
            gpc->pop_size[to_pop]++;
        }

        gpc->pop[from_pop][idx[i]] = gpc->pop[from_pop][gpc->pop_size[from_pop] - 1];
        gpc->pop[from_pop][gpc->pop_size[from_pop] - 1] = NULL;
        gpc->pop_size[from_pop]--;
    }
        
}


static void ferGPCMutation(fer_gpc_t *gpc, int from_pop, int to_pop)
{
    int idx, node_idx;
    int depth;
    fer_gpc_tree_t *tree;
    fer_gpc_node_t *node, **desc;

    // select an individual from population
    idx = ferGPCSelectionTournament(gpc, gpc->params.tournament_size,
                                    gpc->pop[from_pop], gpc->pop_size[from_pop]);
    tree = gpc->pop[from_pop][idx];

    do {
        // choose a node that will undergo a mutation
        node_idx = ferGPCRandInt(gpc, 0, tree->num_nodes);

        // get a node from the tree
        node = ferGPCTreeNodeById(tree, node_idx, &desc, &depth);

    } while (gpc->params.max_depth < depth);

    // delete an old subtree and generate a new one
    ferGPCNodeDel(node);
    *desc = ferGPCGenTree(gpc, 0, gpc->params.max_depth - depth);
    ferGPCTreeFix(tree);

    // copy the tree to destination array
    gpc->pop[to_pop][gpc->pop_size[to_pop]] = tree;
    gpc->pop_size[to_pop]++;

    // remove the tree from source array
    gpc->pop[from_pop][idx] = gpc->pop[from_pop][gpc->pop_size[from_pop] - 1];
    gpc->pop[from_pop][gpc->pop_size[from_pop] - 1] = NULL;
    gpc->pop_size[from_pop]--;
}


static fer_gpc_node_t *ferGPCSimplifySubtree(fer_gpc_t *gpc, fer_gpc_node_t *node)
{
    int i, idx;
    fer_gpc_node_t **desc, *rnode;

    if (node->ndesc == 0)
        return node;

    desc = FER_GPC_NODE_DESC(node);


    // first dive down in tree
    for (i = 0; i < node->ndesc; i++){
        if (desc[i]->ndesc > 0)
            desc[i] = ferGPCSimplifySubtree(gpc, desc[i]);
    }

    // check if all descendants are terminals and compare all their idx
    idx = desc[0]->idx;
    for (i = 0; i < node->ndesc; i++){
        if (desc[i]->ndesc != 0 || idx != desc[i]->idx)
            break;
    }

    if (i == node->ndesc){
        rnode = desc[0];
        desc[0] = NULL;
        ferGPCNodeDel(node);
        return rnode;
    }

    return node;
}

static void ferGPCSimplify(fer_gpc_t *gpc, int pop)
{
    int i;

    for (i = 0; i < gpc->pop_size[pop]; i++){
        gpc->pop[pop][i]->root = ferGPCSimplifySubtree(gpc, gpc->pop[pop][i]->root);
        ferGPCTreeFix(gpc->pop[pop][i]);
    }
}


static void ferGPCPruneDeepSubtree(fer_gpc_t *gpc, fer_gpc_node_t *node,
                                   int depth)
{
    fer_gpc_node_t **desc;
    int i;

    if (node->ndesc == 0)
        return;

    desc = FER_GPC_NODE_DESC(node);

    if (depth == gpc->params.max_depth - 1){
        for (i = 0; i < node->ndesc; i++){
            if (desc[i]->ndesc != 0){
                ferGPCNodeDel(desc[i]);
                desc[i] = ferGPCGenClass(gpc);
            }
        }
    }else{
        for (i = 0; i < node->ndesc; i++){
            if (desc[i]->ndesc != 0){
                ferGPCPruneDeepSubtree(gpc, desc[i], depth + 1);
            }
        }
    }
}

static void ferGPCPruneDeep(fer_gpc_t *gpc, int pop)
{
    int i;

    for (i = 0; i < gpc->pop_size[pop]; i++){
        ferGPCPruneDeepSubtree(gpc, gpc->pop[pop][i]->root, 0);
        ferGPCTreeFix(gpc->pop[pop][i]);
    }
}
