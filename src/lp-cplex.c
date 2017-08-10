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
#include "boruvka/config.h"
#include "boruvka/lp.h"
#include "boruvka/alloc.h"
#include "_lp.h"

#ifdef BOR_CPLEX
# include <ilcplex/cplex.h>

struct _lp_t {
    bor_lp_t cls;
    CPXENVptr env;
    CPXLPptr lp;
    int mip;
};
typedef struct _lp_t lp_t;

#define LP(l) bor_container_of((l), lp_t, cls)

static void cplexErr(lp_t *lp, int status, const char *s)
{
    char errmsg[1024];
    CPXgeterrorstring(lp->env, status, errmsg);
    fprintf(stderr, "Error: CPLEX: %s: %s\n", s, errmsg);
    exit(-1);
}

static bor_lp_t *new(int rows, int cols, unsigned flags)
{
    lp_t *lp;
    int st, num_threads;

    lp = BOR_ALLOC(lp_t);
    lp->cls.cls = &bor_lp_cplex;
    lp->mip = 0;

    // Initialize CPLEX structures
    lp->env = CPXopenCPLEX(&st);
    if (lp->env == NULL)
        cplexErr(lp, st, "Could not open CPLEX environment");

    // Set number of processing threads
    num_threads = BOR_LP_GET_NUM_THREADS(flags);
    if (num_threads == BOR_LP_NUM_THREADS_AUTO){
        num_threads = 0;
    }else if (num_threads == 0){
        num_threads = 1;
    }
    st = CPXsetintparam(lp->env, CPX_PARAM_THREADS, num_threads);
    if (st != 0)
        cplexErr(lp, st, "Could not set number of threads");

    lp->lp = CPXcreateprob(lp->env, &st, "");
    if (lp->lp == NULL)
        cplexErr(lp, st, "Could not create CPLEX problem");

    // Set up minimaztion
    if ((flags & 0x1u) == 0){
        CPXchgobjsen(lp->env, lp->lp, CPX_MIN);
    }else{
        CPXchgobjsen(lp->env, lp->lp, CPX_MAX);
    }

    st = CPXnewcols(lp->env, lp->lp, cols, NULL, NULL, NULL, NULL, NULL);
    if (st != 0)
        cplexErr(lp, st, "Could not initialize variables");

    st = CPXnewrows(lp->env, lp->lp, rows, NULL, NULL, NULL, NULL);
    if (st != 0)
        cplexErr(lp, st, "Could not initialize constraints");

    return &lp->cls;
}

static void del(bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    if (lp->lp)
        CPXfreeprob(lp->env, &lp->lp);
    if (lp->env)
        CPXcloseCPLEX(&lp->env);
    BOR_FREE(lp);
}

static void setObj(bor_lp_t *_lp, int i, double coef)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXchgcoef(lp->env, lp->lp, -1, i, coef);
    if (st != 0)
        cplexErr(lp, st, "Could not set objective coeficient.");
}

static void setVarRange(bor_lp_t *_lp, int i, double lb, double ub)
{
    lp_t *lp = LP(_lp);
    static const char lu[2] = { 'L', 'U' };
    double bd[2] = { lb, ub };
    int ind[2];
    int st;

    ind[0] = ind[1] = i;
    st = CPXchgbds(lp->env, lp->lp, 2, ind, lu, bd);
    if (st != 0)
        cplexErr(lp, st, "Could not set variable as free.");
}

static void setVarFree(bor_lp_t *_lp, int i)
{
    setVarRange(_lp, i, -CPX_INFBOUND, CPX_INFBOUND);
}

static void setVarInt(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    static char type = CPX_INTEGER;
    int st;

    st = CPXchgctype(lp->env, lp->lp, 1, &i, &type);
    if (st != 0)
        cplexErr(lp, st, "Could not set variable as integer.");
    lp->mip = 1;
}

static void setVarBinary(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    static char type = CPX_BINARY;
    int st;

    st = CPXchgctype(lp->env, lp->lp, 1, &i, &type);
    if (st != 0)
        cplexErr(lp, st, "Could not set variable as binary.");
    lp->mip = 1;
}

static void setCoef(bor_lp_t *_lp, int row, int col, double coef)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXchgcoef(lp->env, lp->lp, row, col, coef);
    if (st != 0)
        cplexErr(lp, st, "Could not set constraint coeficient.");
}

static void setRHS(bor_lp_t *_lp, int row, double rhs, char sense)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXchgcoef(lp->env, lp->lp, row, -1, rhs);
    if (st != 0)
        cplexErr(lp, st, "Could not set right-hand-side.");

    st = CPXchgsense(lp->env, lp->lp, 1, &row, &sense);
    if (st != 0)
        cplexErr(lp, st, "Could not set right-hand-side sense.");
}

static void addRows(bor_lp_t *_lp, int cnt, const double *rhs, const char *sense)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXnewrows(lp->env, lp->lp, cnt, rhs, sense, NULL, NULL);
    if (st != 0)
        cplexErr(lp, st, "Could not add new rows.");
}

static void delRows(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXdelrows(lp->env, lp->lp, begin, end);
    if (st != 0)
        cplexErr(lp, st, "Could not delete rows.");
}

static int numRows(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    return CPXgetnumrows(lp->env, lp->lp);
}

static void addCols(bor_lp_t *_lp, int cnt)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXnewcols(lp->env, lp->lp, cnt, NULL, NULL, NULL, NULL, NULL);
    if (st != 0)
        cplexErr(lp, st, "Could not add new columns.");
}

static void delCols(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXdelcols(lp->env, lp->lp, begin, end);
    if (st != 0)
        cplexErr(lp, st, "Could not delete columns.");
}

static int numCols(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    return CPXgetnumcols(lp->env, lp->lp);
}

static int solve(bor_lp_t *_lp, double *val, double *obj)
{
    lp_t *lp = LP(_lp);
    int st;

    if (lp->mip){
        st = CPXmipopt(lp->env, lp->lp);
    }else{
        st = CPXlpopt(lp->env, lp->lp);
    }
    if (st != 0)
        cplexErr(lp, st, "Failed to optimize LP");

    st = CPXsolution(lp->env, lp->lp, NULL, val, obj, NULL, NULL, NULL);
    if (st == CPXERR_NO_SOLN){
        if (obj != NULL)
            bzero(obj, sizeof(double) * CPXgetnumcols(lp->env, lp->lp));
        if (val != NULL)
            *val = 0.;
        return -1;

    }else if (st != 0){
        cplexErr(lp, st, "Cannot retrieve solution");
        return -1;
    }
    return 0;
}

static void cpxWrite(bor_lp_t *_lp, const char *fn)
{
    lp_t *lp = LP(_lp);
    int st;

    st = CPXwriteprob(lp->env, lp->lp, fn, "LP");
    if (st != 0)
        cplexErr(lp, st, "Failed to optimize ILP");
}


bor_lp_cls_t bor_lp_cplex = {
    BOR_LP_CPLEX,
    "cplex",
    new,
    del,
    setObj,
    setVarRange,
    setVarFree,
    setVarInt,
    setVarBinary,
    setCoef,
    setRHS,
    addRows,
    delRows,
    numRows,
    addCols,
    delCols,
    numCols,
    solve,
    cpxWrite,
};
#endif /* BOR_CPLEX */
