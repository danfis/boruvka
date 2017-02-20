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

#ifdef BOR_GUROBI
# include <gurobi_c.h>

struct _lp_t {
    bor_lp_t cls;
    GRBenv *env;
    GRBmodel *model;
};
typedef struct _lp_t lp_t;

#define LP(l) bor_container_of((l), lp_t, cls)

static char lpSense(char sense)
{
    if (sense == 'L'){
        return GRB_LESS_EQUAL;
    }else if (sense == 'G'){
        return GRB_GREATER_EQUAL;
    }else if (sense == 'E'){
        return GRB_EQUAL;
    }else{
        fprintf(stderr, "Gurobi Error: Unkown sense: %c\n", sense);
        return GRB_EQUAL;
    }
}

static void grbError(lp_t *lp)
{
    fprintf(stderr, "LP Gurobi Error: %s\n", GRBgeterrormsg(lp->env));
    exit(-1);
}

static bor_lp_t *new(int rows, int cols, unsigned flags)
{
    lp_t *lp;
    int ret, sense, num_threads;

    lp = BOR_ALLOC(lp_t);
    lp->cls.cls = &bor_lp_gurobi;
    if ((ret = GRBloadenv(&lp->env, NULL)) != 0){
        fprintf(stderr, "LP Gurobi Error: Could not create environment"
                        " (error-code: %d)!\n", ret);
        exit(-1);
    }
    if (GRBnewmodel(lp->env, &lp->model, NULL, cols,
                    NULL, NULL, NULL, NULL, NULL) != 0){
        grbError(lp);
    }

    if (GRBsetintparam(lp->env, "OutputFlag", 0) != 0)
        grbError(lp);

    num_threads = BOR_LP_GET_NUM_THREADS(flags);
    if (num_threads == BOR_LP_NUM_THREADS_AUTO){
        if (GRBsetintparam(lp->env, "Threads", 0) != 0)
            grbError(lp);
    }else if (num_threads == 0){
        if (GRBsetintparam(lp->env, "Threads", 1) != 0)
            grbError(lp);
    }else{
        if (GRBsetintparam(lp->env, "Threads", num_threads) != 0)
            grbError(lp);
    }

    if (rows > 0){
        if (GRBaddconstrs(lp->model, rows, 0,
                          NULL, NULL, NULL, NULL, NULL, NULL) != 0){
            grbError(lp);
        }
    }

    if ((flags & 0x1) == BOR_LP_MIN){
        sense = GRB_MINIMIZE;
    }else{
        sense = GRB_MAXIMIZE;
    }
    if (GRBsetintattr(lp->model, GRB_INT_ATTR_MODELSENSE, sense) != 0)
        grbError(lp);

    GRBupdatemodel(lp->model);
    return &lp->cls;
}

static void del(bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    GRBfreeenv(lp->env);
    BOR_FREE(lp);
}

static void setObj(bor_lp_t *_lp, int i, double coef)
{
    lp_t *lp = LP(_lp);
    if (GRBsetdblattrelement(lp->model, "Obj", i, coef) != 0)
        grbError(lp);
}

static void setVarRange(bor_lp_t *_lp, int i, double lb, double ub)
{
    lp_t *lp = LP(_lp);
    if (GRBsetdblattrelement(lp->model, "LB", i, lb) != 0)
        grbError(lp);
    if (GRBsetdblattrelement(lp->model, "UB", i, ub) != 0)
        grbError(lp);
}

static void setVarFree(bor_lp_t *_lp, int i)
{
    setVarRange(_lp, i, -1e21, 1e21);
}

static void setVarInt(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    if (GRBsetcharattrelement(lp->model, "VType", i, 'I') != 0)
        grbError(lp);
}

static void setVarBinary(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    if (GRBsetcharattrelement(lp->model, "VType", i, 'B') != 0)
        grbError(lp);
}

static void setCoef(bor_lp_t *_lp, int row, int col, double coef)
{
    lp_t *lp = LP(_lp);
    if (GRBchgcoeffs(lp->model, 1, &row, &col, &coef) != 0)
        grbError(lp);
    GRBupdatemodel(lp->model);
}

static void setRHS(bor_lp_t *_lp, int row, double rhs, char sense)
{
    lp_t *lp = LP(_lp);
    if (GRBsetcharattrelement(lp->model, "Sense", row, lpSense(sense)) != 0)
        grbError(lp);
    if (GRBsetdblattrelement(lp->model, "RHS", row, rhs) != 0)
        grbError(lp);
}

static void addRows(bor_lp_t *_lp, int cnt, const double *rhs, const char *sense)
{
    lp_t *lp = LP(_lp);
    int i;
    char *gsense;

    gsense = BOR_ALLOC_ARR(char, cnt);
    for (i = 0; i < cnt; ++i)
        gsense[i] = lpSense(sense[i]);

    if (GRBaddconstrs(lp->model, cnt, 0, NULL, NULL, NULL,
                      gsense, (double *)rhs, NULL) != 0){
        BOR_FREE(gsense);
        grbError(lp);
    }
    GRBupdatemodel(lp->model);
    BOR_FREE(gsense);
}

static void delRows(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int i, j, *ind;

    ind = BOR_ALLOC_ARR(int, end - begin + 1);
    for (j = 0, i = begin; i <= end; ++i, ++j)
        ind[j] = i;

    if (GRBdelconstrs(lp->model, end - begin + 1, ind) != 0){
        BOR_FREE(ind);
        grbError(lp);
    }
    BOR_FREE(ind);
}

static int numRows(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    int rows;
    if (GRBgetintattr(lp->model, "NumConstrs", &rows) != 0)
        grbError(lp);
    return rows;
}

static void addCols(bor_lp_t *_lp, int cnt)
{
    lp_t *lp = LP(_lp);

    if (GRBaddvars(lp->model, cnt, 0, NULL, NULL, NULL,
                   NULL, NULL, NULL, NULL, NULL) != 0){
        grbError(lp);
    }
    GRBupdatemodel(lp->model);
}

static void delCols(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int i, j, *ind;

    ind = BOR_ALLOC_ARR(int, end - begin + 1);
    for (j = 0, i = begin; i <= end; ++i, ++j)
        ind[j] = i;

    if (GRBdelvars(lp->model, end - begin + 1, ind) != 0){
        BOR_FREE(ind);
        grbError(lp);
    }
    GRBupdatemodel(lp->model);
    BOR_FREE(ind);
}

static int numCols(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    int cols;
    if (GRBgetintattr(lp->model, "NumVars", &cols) != 0)
        grbError(lp);
    return cols;
}

static int lpSolve(bor_lp_t *_lp, double *val, double *obj)
{
    lp_t *lp = LP(_lp);
    int st, i, cols;

    if (GRBoptimize(lp->model) != 0)
        grbError(lp);
    if (GRBgetintattr(lp->model, "Status", &st) != 0)
        grbError(lp);

    if (st == GRB_OPTIMAL){
        if (val != NULL){
            if (GRBgetdblattr(lp->model, "ObjVal", val) != 0)
                grbError(lp);
        }
        if (obj != NULL){
            if (GRBgetintattr(lp->model, "NumVars", &cols) != 0)
                grbError(lp);
            for (i = 0; i < cols; ++i){
                if (GRBgetdblattrelement(lp->model, "X", i, obj + i) != 0)
                    grbError(lp);
            }
        }
        return 0;
    }else{
        if (obj != NULL){
            if (GRBgetintattr(lp->model, "NumVars", &cols) != 0)
                grbError(lp);
            bzero(obj, sizeof(double) * cols);
        }
        if (val != NULL)
            *val = 0.;
        return -1;
    }
}

static void lpWrite(bor_lp_t *_lp, const char *fn)
{
    lp_t *lp = LP(_lp);
    if (GRBwrite(lp->model, fn) != 0)
        grbError(lp);
}


bor_lp_cls_t bor_lp_gurobi = {
    BOR_LP_GUROBI,
    "gurobi",
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
    lpSolve,
    lpWrite,
};
#endif /* BOR_GUROBI */
