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

#ifndef __BOR_LP_H__
#define __BOR_LP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Forward declaration */
typedef struct _bor_lp_t bor_lp_t;

/**
 * Solvers.
 * Not all may be available (TODO)
 */
#define BOR_LP_DEFAULT 0x0000u
#define BOR_LP_CPLEX   0x0010u
#define BOR_LP_GUROBI  0x0020u
#define BOR_LP_LPSOLVE 0x0030u

/**
 * Sets the number of parallel threads that will be invoked by a
 * parallel optimizer.
 * By default one thread is used.
 * Set num threads to -1 to switch to auto mode.
 */
#define BOR_LP_NUM_THREADS(num) \
    ((((unsigned)(num)) & 0x3fu) << 8u)

/**
 * Sets minimization (default).
 */
#define BOR_LP_MIN 0x0

/**
 * Sets maximization.
 */
#define BOR_LP_MAX 0x1


/**
 * Returns true if the specified solver is available.
 * For BOR_LP_DEFAULT returns false if there is no LP solver available.
 */
int borLPSolverAvailable(unsigned solver);

/**
 * Creates a new LP problem with specified number of rows and columns.
 */
bor_lp_t *borLPNew(int rows, int cols, unsigned flags);

/**
 * Deletes the LP object.
 */
void borLPDel(bor_lp_t *lp);

/**
 * Returns name of the current LP solver.
 */
const char *borLPSolverName(const bor_lp_t *lp);

/**
 * Returns one of BOR_LP_{CPLEX,GUROBI,LPSOLVE} constants according to the
 * current solver.
 */
int borLPSolverID(const bor_lp_t *lp);

/**
 * Sets objective coeficient for i'th variable.
 */
void borLPSetObj(bor_lp_t *lp, int i, double coef);

/**
 * Sets i'th variable's range.
 */
void borLPSetVarRange(bor_lp_t *lp, int i, double lb, double ub);

/**
 * Sets i'th variable as free.
 */
void borLPSetVarFree(bor_lp_t *lp, int i);

/**
 * Sets i'th variable as integer.
 */
void borLPSetVarInt(bor_lp_t *lp, int i);

/**
 * Sets i'th variable as binary.
 */
void borLPSetVarBinary(bor_lp_t *lp, int i);

/**
 * Sets coefficient for row's constraint and col's variable.
 */
void borLPSetCoef(bor_lp_t *lp, int row, int col, double coef);

/**
 * Sets right hand side of the row'th constraint.
 * Also sense of the constraint must be defined:
 *      - 'L' <=
 *      - 'G' >=
 *      - 'E' =
 */
void borLPSetRHS(bor_lp_t *lp, int row, double rhs, char sense);

/**
 * Adds cnt rows to the model.
 */
void borLPAddRows(bor_lp_t *lp, int cnt, const double *rhs, const char *sense);

/**
 * Deletes rows with indexes between begin and end including both limits,
 * i.e., first deleted row has index {begin} the last deleted row has index
 * {end}.
 */
void borLPDelRows(bor_lp_t *lp, int begin, int end);

/**
 * Returns number of rows in model.
 */
int borLPNumRows(const bor_lp_t *lp);

/**
 * Adds cnt columns to the model.
 */
void borLPAddCols(bor_lp_t *lp, int cnt);

/**
 * Deletes columns with indexes between begin and end including both
 * limits, i.e., first deleted column has index {begin} the last deleted
 * column has index {end}.
 */
void borLPDelCols(bor_lp_t *lp, int begin, int end);

/**
 * Returns number of columns in model.
 */
int borLPNumCols(const bor_lp_t *lp);

/**
 * Solves (I)LP problem.
 * Return 0 if problem was solved, -1 if the problem has no solution.
 * Objective value is returned via argument val and values of each variable
 * via argument obj if non-NULL.
 */
int borLPSolve(bor_lp_t *lp, double *val, double *obj);


void borLPWrite(bor_lp_t *lp, const char *fn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_LP_H__ */
