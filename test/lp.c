#include <cu/cu.h>
#include <stdio.h>
#include "boruvka/alloc.h"
#include "boruvka/lp.h"

static bor_lp_t *tspCreate(const double *dist, int num_cities, unsigned flags)
{
    bor_lp_t *lp;
    int ui = num_cities * num_cities;
    int i, j, row;
    char sense;
    double rhs;

    lp = borLPNew(0, num_cities * num_cities, flags | BOR_LP_MIN);
    for (i = 0; i < num_cities * num_cities; ++i){
        borLPSetVarBinary(lp, i);
        borLPSetObj(lp, i, dist[i]);
    }
    assertEquals(borLPNumCols(lp), num_cities * num_cities);
    assertEquals(borLPNumRows(lp), 0);

    borLPAddCols(lp, num_cities);
    assertEquals(borLPNumCols(lp), num_cities * num_cities + num_cities);
    for (i = ui; i < ui + num_cities; ++i){
        borLPSetVarInt(lp, i);
        borLPSetVarRange(lp, i, 0., 1e10);
    }
    borLPAddCols(lp, num_cities);
    assertEquals(borLPNumCols(lp), num_cities * num_cities + 2 * num_cities);
    borLPDelCols(lp, num_cities * num_cities + num_cities,
                     num_cities * num_cities + 2 * num_cities - 1);
    assertEquals(borLPNumCols(lp), num_cities * num_cities + num_cities);

    for (j = 0; j < num_cities; ++j){
        rhs = 1.;
        sense = 'E';
        row = borLPNumRows(lp);
        borLPAddRows(lp, 1, &rhs, &sense);
        borLPAddRows(lp, 1, &rhs, &sense);
        for (i = 0; i < num_cities; ++i){
            if (i == j)
                continue;
            borLPSetCoef(lp, row, i * num_cities + j, 1.);
            borLPSetCoef(lp, row + 1, j * num_cities + i, 1.);
        }
    }

    borLPSetVarRange(lp, ui, 0., 0.);
    for (i = 0; i < num_cities; ++i){
        for (j = 0; j < num_cities; ++j){
            if (i == j || i < 1 || j < 1)
                continue;
            rhs = num_cities - 1.;
            sense = 'L';
            row = borLPNumRows(lp);
            borLPAddRows(lp, 1, &rhs, &sense);
            borLPSetCoef(lp, row, ui + i, 1.);
            borLPSetCoef(lp, row, ui + j, -1.);
            borLPSetCoef(lp, row, i * num_cities + j, num_cities);
        }
    }
    return lp;
}

static void tspTest(const double *dist, int num_cities, unsigned flags)
{
    bor_lp_t *lp;
    double objval[3];
    double *obj[3];
    unsigned solver[3] = { BOR_LP_CPLEX, BOR_LP_GUROBI, BOR_LP_LPSOLVE };
    int avail[3], ret[3];
    int i, j, k;

    for (i = 0; i < 3; ++i)
        obj[i] = BOR_ALLOC_ARR(double, num_cities * num_cities + num_cities);

    for (i = 0; i < 3; ++i){
        avail[i] = borLPSolverAvailable(solver[i]);
        if (!avail[i])
            continue;
        lp = tspCreate(dist, num_cities, flags | solver[i]);
        assertEquals(borLPSolverID(lp), solver[i]);
        if (solver[i] == BOR_LP_CPLEX){
            assertTrue(strcmp(borLPSolverName(lp), "cplex") == 0);
        }else if (solver[i] == BOR_LP_GUROBI){
            assertTrue(strcmp(borLPSolverName(lp), "gurobi") == 0);
        }else if (solver[i] == BOR_LP_LPSOLVE){
            assertTrue(strcmp(borLPSolverName(lp), "lpsolve") == 0);
        }
        //borLPWrite(lp, "test.lp");
        ret[i] = borLPSolve(lp, objval + i, obj[i]);
        borLPDel(lp);
    }

    for (i = 0; i < 3; ++i){
        if (!avail[i])
            continue;
        for (j = i + 1; j < 3; ++j){
            if (!avail[j])
                continue;
            assertEquals(ret[i], ret[j]);
            if (ret[i] == 0){
                assertEquals((int)(objval[i] + 1E-8),
                             (int)(objval[j] + 1E-8));
                for (k = 0; k < num_cities * num_cities + num_cities; ++k){
                    //fprintf(stderr, "obj[%d]: %f %f\n", k, obj[i][k], obj[j][k]);
                    assertEquals((int)(obj[i][k] + 1E-8),
                                 (int)(obj[j][k] + 1E-8));
                }
            }
        }
    }

    for (i = 0; i < 3; ++i)
        BOR_FREE(obj[i]);
}

TEST(testLP)
{
    double tsp1[] = {
        1e10, 1., -3., 12.,
        100., 1e10, 5., 3.,
        -6., 2., 1e10, 2.,
        10., 80., 5., 1e10,
    };
    tspTest(tsp1, 4, 0);
}
