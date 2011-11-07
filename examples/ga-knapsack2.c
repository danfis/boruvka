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

#include <limits.h>
#include <fermat/ga.h>
#include <fermat/dbg.h>

int input[1000];
int input_size;
int bins = 10;
int popsize = 3000;
fer_real_t elite = 0.05;
int maxiter = 10000;
int cycles = 10;
FILE *flog;

void result(fer_ga_t *ga, int c)
{
    int i, min, mini, *gt;
    void *indiv;
    fer_real_t *ft, f;
    FILE *fres;
    char fn[100];

    sprintf(fn, "ga-res.%02d", c);
    fres = fopen(fn, "w");

    mini = -1;
    min = INT_MAX;
    for (i = 0; i < popsize; i++){
        indiv = ferGAIndiv(ga, i);
        ft    = ferGAIndivFitness(ga, indiv);

        f = -ft[0];
        if (f < min){
            min = f;
            mini = i;
        }
    }

    indiv = ferGAIndiv(ga, mini);
    gt    = (int *)ferGAIndivGenotype(ga, indiv);
    for (i = 0; i < input_size; i++){
        fprintf(fres, "%d %d\n", input[i], gt[i]);
    }

    fclose(fres);
}

int terminate(fer_ga_t *ga, void *data)
{
    int i, max, min, avg;
    void *indiv;
    fer_real_t *ft, f;
    static int counter = 0;

    min = INT_MAX;
    max = -1;
    avg = 0;
    for (i = 0; i < popsize; i++){
        indiv = ferGAIndiv(ga, i);
        ft    = ferGAIndivFitness(ga, indiv);

        f = -ft[0];
        avg += f;
        if (f < min)
            min = f;
        if (f > max)
            max = f;
    }
    avg /= popsize;

    ++counter;
    if (counter == maxiter || min == 0){
        DBG("[%09d]; min: %d, max: %d, avg: %d", counter, min, max, avg);
        counter = 0;
        return 1;
    }

    if (counter % 100 == 0){
        fprintf(flog, "%d %d\n", avg, min);
        DBG("[%09d]; min: %d, max: %d, avg: %d", counter, min, max, avg);
    }

    return 0;
}

void eval(fer_ga_t *ga, void *_gt, fer_real_t *fitness, void *data)
{
    int *gt = (int *)_gt;
    int i, min, max, avg;
    int bin_weights[30];

    for (i = 0; i < bins; i++)
        bin_weights[i] = 0;

    for (i = 0; i < input_size; i++){
        bin_weights[gt[i]] += input[i];
    }

    min = INT_MAX;
    max = -1;
    avg = 0;
    for (i = 0; i < bins; i++){
        if (bin_weights[i] < min)
            min = bin_weights[i];
        if (bin_weights[i] > max)
            max = bin_weights[i];
        avg += bin_weights[i];
    }
    avg /= popsize;

    fitness[0] = -(max - min);
}

void init(fer_ga_t *ga, void *_gt, void *data)
{
    int *gt = (int *)_gt;
    int i;

    for (i = 0; i < input_size; i++){
        gt[i] = ferGARandInt(ga, 0, bins);
    }
}

void mutate(fer_ga_t *ga, void *_gt, void *data)
{
    int *gt = (int *)_gt;
    int i, j, tmp, min;
    int bin_weights[30];

    if (ferGARand01(ga) < 0.5){
        i = ferGARandInt(ga, 0, input_size);
        gt[i] = ferGARandInt(ga, 0, bins);
    }

    i = ferGARandInt(ga, 0, input_size);
    j = ferGARandInt(ga, 0, input_size);
    FER_SWAP(gt[i], gt[j], tmp);
    return;

    for (i = 0; i < bins; i++)
        bin_weights[i] = 0;

    for (i = 0; i < input_size; i++){
        bin_weights[gt[i]] += input[i];
    }

    min = 0;
    for (i = 1; i < bins; i++){
        if (bin_weights[i] < bin_weights[min])
            min = i;
    }

    i = ferGARandInt(ga, 0, input_size);
    gt[i] = min;
}

int main(int argc, char *argv[])
{
    fer_ga_ops_t ops;
    fer_ga_params_t params;
    fer_ga_t *ga;
    char fn[100];
    int i;

    if (argc != 1){
        fprintf(stderr, "Usage: %s <input.txt\n", argv[0]);
        return -1;
    }

    for (input_size = 0; scanf("%d", input + input_size) == 1; input_size++);
    printf("input_size: %d\n", input_size);

    ferGAOpsInit(&ops);
    ferGAParamsInit(&params);

    ops.terminate = terminate;
    ops.eval      = eval;
    ops.init      = init;
    ops.mutate    = mutate;
    ops.sel       = ferGASelTournament2;
    ops.presel    = ferGAPreselElite;

    params.pc             = 0.7;
    params.pm             = 0.3;
    params.gene_size      = sizeof(int);
    params.genotype_size  = input_size;
    params.pop_size       = popsize;
    params.fitness_size   = 1;
    params.crossover_size = 2;
    params.presel_max     = popsize * elite;
    params.threads        = 1;


    for (i = 0; i < cycles; i++){
        sprintf(fn, "ga-log.%02d", i);
        flog = fopen(fn, "w");
        fprintf(flog, "# avg min\n");

        ga = ferGANew(&ops, &params);

        ferGARun(ga);
        result(ga, i);

        ferGADel(ga);

        fclose(flog);
    }

    return 0;
}
