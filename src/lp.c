/***
 * Boruvka
 * --------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <stdlib.h>
#include <stdio.h>
#include "boruvka/lp.h"
#include "_lp.h"

#define SOLVER(flags) ((flags) & 0xf0u)


int borLPSolverAvailable(unsigned solver)
{
    if (SOLVER(solver) == BOR_LP_CPLEX){
#ifdef BOR_CPLEX
        return 1;
#else /* BOR_CPLEX */
        return 0;
#endif /* BOR_CPLEX */
    }else if (SOLVER(solver) == BOR_LP_GUROBI){
#ifdef BOR_GUROBI
        return 1;
#else /* BOR_GUROBI */
        return 0;
#endif /* BOR_GUROBI */
    }else if (SOLVER(solver) == BOR_LP_LPSOLVE){
#ifdef BOR_LPSOLVE
        return 1;
#else /* BOR_LPSOLVE */
        return 0;
#endif /* BOR_LPSOLVE */
    }
#ifdef BOR_CPLEX
        return 1;
#else /* BOR_CPLEX */
#ifdef BOR_GUROBI
        return 1;
#else /* BOR_GUROBI */
#ifdef BOR_LPSOLVE
        return 1;
#else /* BOR_LPSOLVE */
        return 0;
#endif /* BOR_LPSOLVE */
#endif /* BOR_GUROBI */
#endif /* BOR_CPLEX */
}

bor_lp_t *borLPNew(int rows, int cols, unsigned flags)
{
    if (SOLVER(flags) == BOR_LP_CPLEX){
#ifdef BOR_CPLEX
        return bor_lp_cplex.new(rows, cols, flags);
#else /* BOR_CPLEX */
        return bor_lp_not_available.new(rows, cols, flags);
#endif /* BOR_CPLEX */
    }else if (SOLVER(flags) == BOR_LP_GUROBI){
#ifdef BOR_GUROBI
        return bor_lp_gurobi.new(rows, cols, flags);
#else /* BOR_GUROBI */
        return bor_lp_not_available.new(rows, cols, flags);
#endif /* BOR_GUROBI */
    }else if (SOLVER(flags) == BOR_LP_LPSOLVE){
#ifdef BOR_LPSOLVE
        return bor_lp_lpsolve.new(rows, cols, flags);
#else /* BOR_LPSOLVE */
        return bor_lp_not_available.new(rows, cols, flags);
#endif /* BOR_LPSOLVE */
    }

#ifdef BOR_CPLEX
    return bor_lp_cplex.new(rows, cols, flags);
#else /* BOR_CPLEX */
#ifdef BOR_GUROBI
    return bor_lp_gurobi.new(rows, cols, flags);
#else /* BOR_GUROBI */
#ifdef BOR_LPSOLVE
    return bor_lp_lpsolve.new(rows, cols, flags);
#else /* BOR_LPSOLVE */
    return bor_lp_not_available.new(rows, cols, flags);
#endif /* BOR_LPSOLVE */
#endif /* BOR_GUROBI */
#endif /* BOR_CPLEX */
}

void borLPDel(bor_lp_t *lp)
{
    lp->cls->del(lp);
}

const char *borLPSolverName(const bor_lp_t *lp)
{
    return lp->cls->solver_name;
}

int borLPSolverID(const bor_lp_t *lp)
{
    return lp->cls->solver_id;
}

void borLPSetObj(bor_lp_t *lp, int i, double coef)
{
    lp->cls->set_obj(lp, i, coef);
}

void borLPSetVarRange(bor_lp_t *lp, int i, double lb, double ub)
{
    lp->cls->set_var_range(lp, i, lb, ub);
}

void borLPSetVarFree(bor_lp_t *lp, int i)
{
    lp->cls->set_var_free(lp, i);
}

void borLPSetVarInt(bor_lp_t *lp, int i)
{
    lp->cls->set_var_int(lp, i);
}

void borLPSetVarBinary(bor_lp_t *lp, int i)
{
    lp->cls->set_var_binary(lp, i);
}

void borLPSetCoef(bor_lp_t *lp, int row, int col, double coef)
{
    lp->cls->set_coef(lp, row, col, coef);
}

void borLPSetRHS(bor_lp_t *lp, int row, double rhs, char sense)
{
    lp->cls->set_rhs(lp, row, rhs, sense);
}

void borLPAddRows(bor_lp_t *lp, int cnt, const double *rhs, const char *sense)
{
    lp->cls->add_rows(lp, cnt, rhs, sense);
}

void borLPDelRows(bor_lp_t *lp, int begin, int end)
{
    lp->cls->del_rows(lp, begin, end);
}

int borLPNumRows(const bor_lp_t *lp)
{
    return lp->cls->num_rows(lp);
}

void borLPAddCols(bor_lp_t *lp, int cnt)
{
    lp->cls->add_cols(lp, cnt);
}

void borLPDelCols(bor_lp_t *lp, int begin, int end)
{
    lp->cls->del_cols(lp, begin, end);
}

int borLPNumCols(const bor_lp_t *lp)
{
    return lp->cls->num_cols(lp);
}

int borLPSolve(bor_lp_t *lp, double *val, double *obj)
{
    return lp->cls->solve(lp, val, obj);
}

void borLPWrite(bor_lp_t *lp, const char *fn)
{
    lp->cls->write(lp, fn);
}




#define noSolverExit() \
    do { \
    fprintf(stderr, "Error: The requested LP solver is not available!\n"); \
    exit(-1); \
    } while (0)
static bor_lp_t *noNew(int rows, int cols, unsigned flags)
{ noSolverExit(); }
static void noDel(bor_lp_t *lp)
{ noSolverExit(); }
static void noSetObj(bor_lp_t *lp, int i, double coef)
{ noSolverExit(); }
static void noSetVarRange(bor_lp_t *lp, int i, double lb, double ub)
{ noSolverExit(); }
static void noSetVarFree(bor_lp_t *lp, int i)
{ noSolverExit(); }
static void noSetVarInt(bor_lp_t *lp, int i)
{ noSolverExit(); }
static void noSetVarBinary(bor_lp_t *lp, int i)
{ noSolverExit(); }
static void noSetCoef(bor_lp_t *lp, int row, int col, double coef)
{ noSolverExit(); }
static void noSetRHS(bor_lp_t *lp, int row, double rhs, char sense)
{ noSolverExit(); }
static void noAddRows(bor_lp_t *lp, int cnt, const double *rhs, const char *sense)
{ noSolverExit(); }
static void noDelRows(bor_lp_t *lp, int begin, int end)
{ noSolverExit(); }
static int noNumRows(const bor_lp_t *lp)
{ noSolverExit(); }
static void noAddCols(bor_lp_t *lp, int cnt)
{ noSolverExit(); }
static void noDelCols(bor_lp_t *lp, int begin, int end)
{ noSolverExit(); }
static int noNumCols(const bor_lp_t *lp)
{ noSolverExit(); }
static int noSolve(bor_lp_t *lp, double *val, double *obj)
{ noSolverExit(); }
static void noWrite(bor_lp_t *lp, const char *fn)
{ noSolverExit(); }

bor_lp_cls_t bor_lp_not_available = {
    0, "",
    noNew,
    noDel,
    noSetObj,
    noSetVarRange,
    noSetVarFree,
    noSetVarInt,
    noSetVarBinary,
    noSetCoef,
    noSetRHS,
    noAddRows,
    noDelRows,
    noNumRows,
    noAddCols,
    noDelCols,
    noNumCols,
    noSolve,
    noWrite,
};
