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

#include <fermat/vptree.h>
#include <fermat/rand-mt.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

static fer_real_t ferVPTreeDist(int d, const fer_vec_t *v1,
                                const fer_vec_t *v2, void *data);

void ferVPTreeParamsInit(fer_vptree_params_t *params)
{
    params->dim = 2;
    params->dist = ferVPTreeDist;
    params->dist_data = NULL;

    params->minsize = 1;
    params->maxsize = 2;

    params->samplesize = 5;
}

void ferVPTreeElInit(fer_vptree_el_t *el, const fer_vec_t *p)
{
    ferListInit(&el->list);
    el->p = p;
}

fer_vptree_t *ferVPTreeNew(const fer_vptree_params_t *params)
{
    fer_vptree_t *vp;

    vp = FER_ALLOC(fer_vptree_t);
    vp->params = *params;

    vp->root = NULL;

    return vp;
}


static void ferVPTreeDelNode(fer_vptree_t *vp,
                             _fer_vptree_node_t *n)
{
    if (n->left && n->right){
        ferVPTreeDelNode(vp, n->left);
        ferVPTreeDelNode(vp, n->right);
        ferVecDel(n->vp);
    }

    free(n);
}

void ferVPTreeDel(fer_vptree_t *vp)
{
    if (vp->root)
        ferVPTreeDelNode(vp, vp->root);
    free(vp);
}

void ferVPTreeAdd(fer_vptree_t *vp, fer_vptree_el_t *el)
{
    // TODO
}

void ferVPTreeRemove(fer_vptree_t *vp, fer_vptree_el_t *el)
{
    // TODO
}

void ferVPTreeUpdate(fer_vptree_t *vp, fer_vptree_el_t *el)
{
    // TODO
}


/** Nearest */
struct _nearest_t {
    const fer_vptree_t *vp;
    const fer_vec_t *p;
    size_t num;

    fer_real_t radius;
    fer_vptree_el_t **els;
    fer_real_t *dist;
    size_t els_len;
};
typedef struct _nearest_t nearest_t;

static void nearestAdd(nearest_t *n, fer_vptree_el_t *el, fer_real_t dist)
{
    fer_real_t tmpdist;
    fer_vptree_el_t *tmpels;
    int pos;

    if (n->els_len < n->num){
        n->els[n->els_len]  = el;
        n->dist[n->els_len] = dist;

        pos = n->els_len;
        n->els_len++;
    }else{
        n->els[n->els_len - 1]  = el;
        n->dist[n->els_len - 1] = dist;

        pos = n->els_len - 1;
    }

    for (; pos > 0; pos--){
        if (n->dist[pos] < n->dist[pos - 1]){
            FER_SWAP(n->dist[pos], n->dist[pos - 1], tmpdist);
            FER_SWAP(n->els[pos], n->els[pos - 1], tmpels);
        }else{
            break;
        }
    }
}

static void nearest(nearest_t *n, const _fer_vptree_node_t *node)
{
    fer_real_t d, d2;
    fer_list_t *item;
    fer_vptree_el_t *el;
    fer_real_t dist;

    if (!node->left && !node->right){
        // node is leaf node, try to add all elements
        FER_LIST_FOR_EACH(&node->els, item){
            el = FER_LIST_ENTRY(item, fer_vptree_el_t, list);
            dist = n->vp->params.dist(n->vp->params.dim, n->p, el->p,
                                      n->vp->params.dist_data);
            if (dist < n->radius){
                nearestAdd(n, el, dist);
                n->radius = n->dist[n->num - 1];
            }
        }
    }else{
        d = n->vp->params.dist(n->vp->params.dim, n->p, node->vp,
                               n->vp->params.dist_data);
        if (d < node->mean){
            if (d < node->mean + n->radius)
                nearest(n, node->left);

            d2 = node->mean - n->radius;
            if (ferEq(d, d2) || d > d2)
                nearest(n, node->right);
        }else{
            d2 = node->mean - n->radius;
            if (ferEq(d, d2) || d > d2)
                nearest(n, node->right);

            if (d < node->mean + n->radius)
                nearest(n, node->left);
        }
    }
}

size_t ferVPTreeNearest(const fer_vptree_t *vp, const fer_vec_t *p, size_t num,
                        fer_vptree_el_t **els)
{
    nearest_t n;
    size_t i;

    n.vp  = vp;
    n.p   = p;
    n.num = num;

    n.radius  = FER_REAL_MAX;
    n.els     = els;
    n.els_len = 0;

    n.dist    = FER_ALLOC_ARR(fer_real_t, num);
    for (i = 0; i < num; i++)
        n.dist[i] = FER_REAL_MAX;

    nearest(&n, vp->root);

    free(n.dist);

    return n.els_len;
}


static void dump(fer_vptree_t *vp, _fer_vptree_node_t *n, _fer_vptree_node_t *par,
                 int level, FILE *out)
{
    fer_list_t *item;
    fer_vptree_el_t *el;
    int i;


    for (i = 0; i < 4 * level; i++)
        fprintf(out, " ");
    fprintf(out, "par[%lx] ", (long)par);

    if (n->left && n->right){
        fprintf(out, "vp: (");
        ferVecPrint(vp->params.dim, n->vp, out);
        fprintf(out, ") [%lx]\n", (long)n);

        dump(vp, n->left, n, level + 1, out);
        dump(vp, n->right, n, level + 1, out);
    }else{
        FER_LIST_FOR_EACH(&n->els, item){
            el = FER_LIST_ENTRY(item, fer_vptree_el_t, list);
            fprintf(out, "(");
            ferVecPrint(vp->params.dim, el->p, out);
            fprintf(out, ") ");
        }

        fprintf(out, "\n");
    }
}
void ferVPTreeDump(fer_vptree_t *vp, FILE *out)
{
    if (vp->root)
        dump(vp, vp->root, NULL, 0, out);
}


static fer_real_t ferVPTreeDist(int d, const fer_vec_t *v1,
                                const fer_vec_t *v2, void *data)
{
    return ferVecDist(d, v1, v2);
}



/** Build **/
struct _build_t {
    fer_vptree_t *vp;
    fer_rand_mt_t *rand;
    fer_real_t *dist;
    fer_vptree_el_t **els;
    fer_vptree_el_t **ps;
    fer_vptree_el_t **ds;
};
typedef struct _build_t build_t;

/** Callback from qsort() */
static int buildCmp(const void *a, const void *b);
/** Adds all elements from els to node */
static void buildAddEls(_fer_vptree_node_t *node,
                        fer_vptree_el_t **els, size_t els_len);
/** Fills {els} with len samples from {els_in} */
static void buildSampleEls(build_t *build, fer_vptree_el_t **els_in, size_t els_len,
                           fer_vptree_el_t **els, size_t len);
/** Compute mean and variance around vp */
static void buildMeanVar(build_t *build, const fer_vec_t *vp,
                         fer_vptree_el_t **els, size_t els_len,
                         fer_real_t *mean, fer_real_t *var);
/** Builds one level of vp-tree */
static _fer_vptree_node_t *buildNode(build_t *build,
                                     fer_vptree_el_t **els, size_t els_len);

fer_vptree_t *ferVPTreeBuild(const fer_vptree_params_t *params,
                             fer_vptree_el_t *_els, size_t els_len, size_t stride)
{
    fer_vptree_t *vp;
    build_t build;
    size_t i;

    vp = ferVPTreeNew(params);

    build.vp   = vp;
    build.els  = FER_ALLOC_ARR(fer_vptree_el_t *, els_len);
    build.ps   = FER_ALLOC_ARR(fer_vptree_el_t *, vp->params.samplesize);
    build.ds   = FER_ALLOC_ARR(fer_vptree_el_t *, vp->params.samplesize);
    build.dist = FER_ALLOC_ARR(fer_real_t, els_len);
    build.rand = ferRandMTNewAuto();
    for (i = 0; i < els_len; i++){
        build.els[i] = _els;
        _els = (fer_vptree_el_t *)((char *)_els + stride);
    }

    vp->root = buildNode(&build, build.els, els_len);

    free(build.els);
    free(build.dist);
    free(build.ps);
    free(build.ds);
    ferRandMTDel(build.rand);

    return vp;
}

static int buildCmp(const void *a, const void *b)
{
    fer_real_t f1 = *(fer_real_t *)a;
    fer_real_t f2 = *(fer_real_t *)b;

    if (ferEq(f1, f2))
        return 0;
    if (f1 < f2)
        return -1;
    return 1;
}

static void buildAddEls(_fer_vptree_node_t *node,
                        fer_vptree_el_t **els,
                        size_t els_len)
{
    size_t i;

    for (i = 0; i < els_len; i++){
        ferListAppend(&node->els, &els[i]->list);
    }
    node->size = els_len;
}

static void buildSampleEls(build_t *build, fer_vptree_el_t **els_in, size_t els_len,
                           fer_vptree_el_t **els, size_t len)
{
    size_t i, p;

    for (i = 0; i < len; i++){
        p = ferRandMT(build->rand, 0, els_len);
        els[i] = els_in[p];
    }
}

static void buildMeanVar(build_t *build, const fer_vec_t *vp,
                         fer_vptree_el_t **els, size_t els_len,
                         fer_real_t *mean, fer_real_t *var)
{
    size_t i;

    for (i = 0; i < els_len; i++){
        build->dist[i] = build->vp->params.dist(build->vp->params.dim,
                                                vp, els[i]->p,
                                                build->vp->params.dist_data);
    }

    qsort(build->dist, els_len, sizeof(fer_real_t), buildCmp);

    *mean = build->dist[els_len / 2];
    if (els_len % 2 == 0){
        *mean += build->dist[els_len / 2 - 1];
        *mean /= FER_REAL(2.);
    }

    *var = FER_CUBE(build->dist[0] - *mean);
    for (i = 1; i < els_len; i++){
        *var += FER_CUBE(build->dist[i] - *mean);
    }
    *var /= (fer_real_t)els_len;
}

static _fer_vptree_node_t *buildNode(build_t *build,
                                     fer_vptree_el_t **els, size_t els_len)
{
    _fer_vptree_node_t *node;
    fer_vptree_el_t *tmpel;
    size_t i, cur, len;
    const fer_vec_t *best_vp;
    fer_real_t best_var, var, mean;
    fer_real_t d;

    node = FER_ALLOC(_fer_vptree_node_t);
    node->vp = NULL;
    node->left = node->right = NULL;
    ferListInit(&node->els);
    node->size = 0;

    if (els_len <= build->vp->params.maxsize){
        // all elements can fit to current node
        buildAddEls(node, els, els_len);
    }else{
        len = FER_MIN(build->vp->params.samplesize, els_len);

        // create vantage point
        node->vp = ferVecNew(build->vp->params.dim);

        // generate random sample of VPs
        buildSampleEls(build, els, els_len, build->ps, len);

        // find best vantage point
        best_var = -FER_REAL_MAX;
        best_vp  = NULL;
        for (i = 0; i < len; i++){
            // random sample data set
            buildSampleEls(build, els, els_len, build->ds, len);

            // compute mean and var
            buildMeanVar(build, build->ps[i]->p, build->ds, len, &mean, &var);

            if (var > best_var){
                best_var   = var;
                node->mean = mean;
                best_vp    = build->ps[i]->p;
            }
        }
        ferVecCopy(build->vp->params.dim, node->vp, best_vp);


        do {
            // reorganize els[]
            for (i = 0, cur = 0; i < els_len; i++){
                d = build->vp->params.dist(build->vp->params.dim,
                                           node->vp, els[i]->p,
                                           build->vp->params.dist_data);
                if (ferEq(d, node->mean) || d < node->mean){
                    if (cur != i){
                        FER_SWAP(els[i], els[cur], tmpel);
                    }
                    ++cur;
                }
            }

            if (cur == els_len)
                node->mean -= 10 * FER_EPS;
        } while (cur == els_len);

        if (cur == 0 || cur == els_len){
            buildAddEls(node, els, els_len);
            ferVecDel(node->vp);
            node->vp = NULL;
        }else{
            // create left and right descendants
            node->left  = buildNode(build, els, cur);
            node->right = buildNode(build, els + cur, els_len - cur);
        }
    }

    return node;
}
