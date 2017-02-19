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

#ifdef BOR_GLPK
# include <glpk.h>

struct _lp_t {
    bor_lp_t cls;
    int mip;
    glp_prob *lp;
};
typedef struct _lp_t lp_t;

#define LP(l) bor_container_of((l), lp_t, cls)

static int lpSense(char sense)
{
    if (sense == 'L'){
        return GLP_LO;
    }else if (sense == 'G'){
        return GLP_UP;
    }else if (sense == 'E'){
        return GLP_FX;
    }else{
        fprintf(stderr, "GLPK Error: Unkown sense: %c\n", sense);
        return GLP_FX;
    }
}

static bor_lp_t *new(int rows, int cols, unsigned flags)
{
    lp_t *lp;

    lp = BOR_ALLOC(lp_t);
    lp->mip = 0;
    lp->lp = glp_create_prob();
    if ((flags & 0x1u) == BOR_LP_MIN){
        glp_set_obj_dir(lp->lp, GLP_MIN);
    }else{
        glp_set_obj_dir(lp->lp, GLP_MAX);
    }
    if (rows > 0)
        glp_add_rows(lp->lp, rows);
    if (cols > 0)
        glp_add_cols(lp->lp, rows);

    return &lp->cls;
}

static void del(bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    glp_delete_prob(lp->lp);
    BOR_FREE(lp);
}

static void setObj(bor_lp_t *_lp, int i, double coef)
{
    lp_t *lp = LP(_lp);
    glp_set_obj_coef(lp->lp, i + 1, coef);
}

static void setVarRange(bor_lp_t *_lp, int i, double lb, double ub)
{
    lp_t *lp = LP(_lp);
    glp_set_col_bnds(lp->lp, i + 1, GLP_DB, lb, ub);
}

static void setVarFree(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    glp_set_col_bnds(lp->lp, i + 1, GLP_FR, 0., 0.);
}

static void setVarInt(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    lp->mip = 1;
    glp_set_col_kind(lp->lp, i + 1, GLP_IV);
}

static void setVarBinary(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    lp->mip = 1;
    glp_set_col_kind(lp->lp, i + 1, GLP_BV);
}

static void setCoef(bor_lp_t *_lp, int row, int col, double coef)
{
    lp_t *lp = LP(_lp);
    int lcol = col + 1;
    glp_set_mat_row(lp->lp, row + 1, 1, &lcol, &coef);
}

static void setRHS(bor_lp_t *_lp, int row, double rhs, char sense)
{
    lp_t *lp = LP(_lp);
    glp_set_row_bnds(lp->lp, row + 1, lpSense(sense), rhs, rhs);
}

static void addRows(bor_lp_t *_lp, int cnt, const double *rhs, const char *sense)
{
    lp_t *lp = LP(_lp);
    int i, from;

    from = glp_get_num_rows(lp->lp);
    glp_add_rows(lp->lp, cnt);
    for (i = 0; i < cnt; ++i){
        glp_set_row_bnds(lp->lp, i + from + 1,
                         lpSense(sense[i]), rhs[i], rhs[i]);
    }
}

static void delRows(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int i, j, *rows;

    rows = BOR_ALLOC_ARR(int, end - begin + 1);
    for (j = 0, i = begin; i <= end; ++i, ++j)
        rows[j] = i + 1;
    glp_del_rows(lp->lp, end - begin + 1, rows);
    BOR_FREE(rows);
}

static int numRows(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    return glp_get_num_rows(lp->lp);
}

static int lpSolve(bor_lp_t *_lp, double *val, double *obj)
{
    lp_t *lp = LP(_lp);
    int i, cols, ret, st;

    if (lp->mip){
        ret = glp_intopt(lp->lp, NULL);
    }else{
        ret = glp_simplex(lp->lp, NULL);
    }
    if (ret != 0){
        if (ret == GLP_ENOPFS)
            goto lp_unfeas;
        goto lp_error;
    }

    if (lp->mip){
        st = glp_mip_status(lp->lp);
    }else{
        st = glp_get_status(lp->lp);
    }
    if (st == GLP_NOFEAS)
        goto lp_unfeas;
    if (st != GLP_OPT)
        goto lp_error;

    if (val != NULL)
        *val = glp_get_obj_val(lp->lp);
    if (obj != NULL){
        cols = glp_get_num_cols(lp->lp);
        for (i = 0; i < cols; ++i)
            obj[i] = glp_get_col_prim(lp->lp, i + 1);
    }
    return 0;

lp_unfeas:
    if (obj != NULL)
        bzero(obj, sizeof(double) * glp_get_num_cols(lp->lp));
    if (val != NULL)
        *val = 0.;
    return -1;

lp_error:
    if (lp->mip){
        st = glp_mip_status(lp->lp);
    }else{
        st = glp_get_status(lp->lp);
    }
    if (st == GLP_FEAS){
        fprintf(stderr, "GLPK Error: Solution is feasible, but not optimal!\n");
    }else if (st == GLP_INFEAS){
        fprintf(stderr, "GLPK Error: Solution is infeasible!\n");
    }else if (st == GLP_UNBND){
        fprintf(stderr, "GLPK Error: Solution is unbounded!\n");
    }else if (st == GLP_UNDEF){
        fprintf(stderr, "GLPK Error: Solution is undefined!\n");
    }
    exit(-1);
}

static void lpWrite(bor_lp_t *_lp, const char *fn)
{
    lp_t *lp = LP(_lp);
    glp_write_lp(lp->lp, NULL, fn);
}


bor_lp_cls_t bor_lp_glpk = {
    BOR_LP_GLPK,
    "glpk",
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
    lpSolve,
    lpWrite,
};
#else /* BOR_GLPK */
bor_lp_cls_t bor_lp_glpk = bor_lp_not_available;
#endif /* BOR_GLPK */
