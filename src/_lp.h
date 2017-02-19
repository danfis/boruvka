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

#ifndef __BOR__LP_H__
#define __BOR__LP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _bor_lp_cls_t {
    int solver_id;
    const char *solver_name;
    bor_lp_t *(*new)(int rows, int cols, unsigned flags);
    void (*del)(bor_lp_t *);
    void (*set_obj)(bor_lp_t *lp, int i, double coef);
    void (*set_var_range)(bor_lp_t *lp, int i, double lb, double ub);
    void (*set_var_free)(bor_lp_t *lp, int i);
    void (*set_var_int)(bor_lp_t *lp, int i);
    void (*set_var_binary)(bor_lp_t *lp, int i);
    void (*set_coef)(bor_lp_t *lp, int row, int col, double coef);
    void (*set_rhs)(bor_lp_t *lp, int row, double rhs, char sense);
    void (*add_rows)(bor_lp_t *lp, int cnt, const double *rhs, const char *sense);
    void (*del_rows)(bor_lp_t *lp, int begin, int end);
    int (*num_rows)(const bor_lp_t *lp);
    int (*solve)(bor_lp_t *lp, double *val, double *obj);
    void (*write)(bor_lp_t *lp, const char *fn);
};
typedef struct _bor_lp_cls_t bor_lp_cls_t;

struct _bor_lp_t {
    bor_lp_cls_t *cls;
};

extern bor_lp_cls_t bor_lp_not_available;
extern bor_lp_cls_t bor_lp_cplex;
extern bor_lp_cls_t bor_lp_gurobi;
extern bor_lp_cls_t bor_lp_lpsolve;
extern bor_lp_cls_t bor_lp_glpk;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR__LP_H__ */
