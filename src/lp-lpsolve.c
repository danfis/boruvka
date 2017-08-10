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

#ifdef BOR_LPSOLVE
# include <lpsolve/lp_lib.h>

struct _lp_t {
    bor_lp_t cls;
    lprec *lp;
};
typedef struct _lp_t lp_t;

#define LP(l) bor_container_of((l), lp_t, cls)

static int lpSense(char sense)
{
    if (sense == 'L'){
        return LE;
    }else if (sense == 'G'){
        return GE;
    }else if (sense == 'E'){
        return EQ;
    }else{
        fprintf(stderr, "LP Error: Unkown sense: %c\n", sense);
        return EQ;
    }
}

static bor_lp_t *new(int rows, int cols, unsigned flags)
{
    lp_t *lp;

    lp = BOR_ALLOC(lp_t);
    lp->cls.cls = &bor_lp_lpsolve;
    lp->lp = make_lp(rows, cols);
    if ((flags & 0x1u) == BOR_LP_MIN){
        set_minim(lp->lp);
    }else{
        set_maxim(lp->lp);
    }

    return &lp->cls;
}

static void del(bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    delete_lp(lp->lp);
    BOR_FREE(lp);
}

static void setObj(bor_lp_t *_lp, int i, double coef)
{
    lp_t *lp = LP(_lp);
    set_obj(lp->lp, i + 1, coef);
}

static void setVarRange(bor_lp_t *_lp, int i, double lb, double ub)
{
    lp_t *lp = LP(_lp);
    set_lowbo(lp->lp, i + 1, lb);
    set_upbo(lp->lp, i + 1, ub);
}

static void setVarFree(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    set_unbounded(lp->lp, i + 1);
}

static void setVarInt(bor_lp_t *_lp, int i)
{
    lp_t *lp = LP(_lp);
    set_int(lp->lp, i + 1, 1);
}

static void setVarBinary(bor_lp_t *_lp, int i)
{
    setVarRange(_lp, i, 0, 1);
    setVarInt(_lp, i);
}

static void setCoef(bor_lp_t *_lp, int row, int col, double coef)
{
    lp_t *lp = LP(_lp);
    set_mat(lp->lp, row + 1, col + 1, coef);
}

static void setRHS(bor_lp_t *_lp, int row, double rhs, char sense)
{
    lp_t *lp = LP(_lp);
    set_rh(lp->lp, row + 1, rhs);
    set_constr_type(lp->lp, row + 1, lpSense(sense));
}

static void addRows(bor_lp_t *_lp, int cnt, const double *rhs, const char *sense)
{
    lp_t *lp = LP(_lp);
    int i, vsen = EQ;
    double vrhs = 0.;

    for (i = 0; i < cnt; ++i){
        if (rhs)
            vrhs = rhs[i];
        if (sense)
            vsen = lpSense(sense[i]);

        add_constraintex(lp->lp, 0, NULL, NULL, vsen, vrhs);
    }
}

static void delRows(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int i;

    for (i = begin; i <= end; ++i)
        del_constraint(lp->lp, begin + 1);
}

static int numRows(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    return get_Nrows(lp->lp);
}

static void addCols(bor_lp_t *_lp, int cnt)
{
    lp_t *lp = LP(_lp);
    int i;

    for (i = 0; i < cnt; ++i)
        add_column(lp->lp, NULL);
}

static void delCols(bor_lp_t *_lp, int begin, int end)
{
    lp_t *lp = LP(_lp);
    int i, size;

    size = end - begin + 1;
    size = BOR_MIN(size, get_Ncolumns(lp->lp));
    for (i = 0; i < size; ++i){
        del_column(lp->lp, begin);
    }
}

static int numCols(const bor_lp_t *_lp)
{
    lp_t *lp = LP(_lp);
    return get_Ncolumns(lp->lp);
}

static int lpSolve(bor_lp_t *_lp, double *val, double *obj)
{
    lp_t *lp = LP(_lp);
    int ret;

    set_verbose(lp->lp, NEUTRAL);
    ret = solve(lp->lp);
    if (ret == OPTIMAL || ret == SUBOPTIMAL){
        if (val != NULL)
            *val = get_objective(lp->lp);
        if (obj != NULL)
            get_variables(lp->lp, obj);

        return 0;

    }else{
        if (obj != NULL)
            bzero(obj, sizeof(double) * get_Ncolumns(lp->lp));
        if (val != NULL)
            *val = 0.;
        return -1;
    }
}

static void lpWrite(bor_lp_t *_lp, const char *fn)
{
    lp_t *lp = LP(_lp);
    write_lp(lp->lp, (char *)fn);
}


bor_lp_cls_t bor_lp_lpsolve = {
    BOR_LP_LPSOLVE,
    "lpsolve",
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
#endif /* BOR_LPSOLVE */
