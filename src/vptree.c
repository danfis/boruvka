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
/** Callback from qsort() */
static int __ferVPTreeMedianCMP(const void *a, const void *b);
/** Computes median from distances of els to p */
static void __ferVPTreeMeanMedian(fer_vptree_t *vp,
                                 fer_vptree_el_t **els,
                                 fer_real_t *dist,
                                 size_t els_len,
                                 const fer_vec_t *p,
                                 fer_real_t *median);
/** Finds vantage point from array of elements */
static void __ferVPTreeFindVP(fer_vptree_t *vp,
                              fer_vptree_el_t **els, fer_real_t *dist,
                              size_t els_len,
                              fer_vec_t *p, fer_real_t *mean);
/** Adds all elements from els to node */
static void __ferVPTreeBuildAddEls(_fer_vptree_node_t *node,
                                   fer_vptree_el_t **els,
                                   size_t els_len);
/** Builds one level of vp-tree */
static _fer_vptree_node_t *__ferVPTreeBuild(fer_vptree_t *vp,
                                            fer_vptree_el_t **els,
                                            fer_real_t *dist,
                                            size_t els_len);

fer_vptree_t *ferVPTreeBuild(const fer_vptree_params_t *params,
                             fer_vptree_el_t *_els, size_t els_len, size_t stride)
{
    fer_vptree_t *vp;
    fer_vptree_el_t **els;
    fer_real_t *dist;
    size_t i;

    vp = ferVPTreeNew(params);

    els  = FER_ALLOC_ARR(fer_vptree_el_t *, els_len);
    dist = FER_ALLOC_ARR(fer_real_t, els_len);
    for (i = 0; i < els_len; i++){
        els[i] = _els;
        _els = (fer_vptree_el_t *)((char *)_els + stride);
    }

    vp->root = __ferVPTreeBuild(vp, els, dist, els_len);

    free(els);
    free(dist);

    return vp;
}

static int __ferVPTreeMedianCMP(const void *a, const void *b)
{
    fer_real_t f1 = *(fer_real_t *)a;
    fer_real_t f2 = *(fer_real_t *)b;

    if (ferEq(f1, f2))
        return 0;
    if (f1 < f2)
        return -1;
    return 1;
}

static void __ferVPTreeMeanMedian(fer_vptree_t *vp,
                                 fer_vptree_el_t **els,
                                 fer_real_t *dist,
                                 size_t els_len,
                                 const fer_vec_t *p,
                                 fer_real_t *median)
{
    size_t i;

    for (i = 0; i < els_len; i++){
        dist[i] = vp->params.dist(vp->params.dim, p, els[i]->p,
                                  vp->params.dist_data);
    }

    qsort(dist, els_len, sizeof(fer_real_t), __ferVPTreeMedianCMP);

    *median = dist[els_len / 2];
    if (els_len % 2 == 0){
        *median += dist[els_len / 2 - 1];
        *median /= FER_REAL(2.);
    }
}

/** Finds vantage point from array of elements */
static void __ferVPTreeFindVP(fer_vptree_t *vp,
                              fer_vptree_el_t **els, fer_real_t *dist,
                              size_t els_len,
                              fer_vec_t *p, fer_real_t *mean)
{
    size_t i;

    ferVecCopy(vp->params.dim, p, els[0]->p);
    for (i = 1; i < els_len; i++){
        ferVecAdd(vp->params.dim, p, els[i]->p);
    }
    ferVecScale(vp->params.dim, p, ferRecp(els_len));

    __ferVPTreeMeanMedian(vp, els, dist, els_len, p, mean);
}

static void __ferVPTreeBuildAddEls(_fer_vptree_node_t *node,
                                   fer_vptree_el_t **els,
                                   size_t els_len)
{
    size_t i;

    for (i = 0; i < els_len; i++){
        ferListAppend(&node->els, &els[i]->list);
    }
    node->size = els_len;
}

/** Builds one level of vp-tree */
static _fer_vptree_node_t *__ferVPTreeBuild(fer_vptree_t *vp,
                                            fer_vptree_el_t **els,
                                            fer_real_t *dist,
                                            size_t els_len)
{
    _fer_vptree_node_t *node;
    fer_vptree_el_t *tmpel;
    size_t i, cur;
    fer_real_t d;

    node = FER_ALLOC(_fer_vptree_node_t);
    node->vp = NULL;
    node->left = node->right = NULL;
    ferListInit(&node->els);
    node->size = 0;

    if (els_len <= vp->params.maxsize){
        // all elements can fit to current node
        __ferVPTreeBuildAddEls(node, els, els_len);
    }else{
        // create vantage point
        node->vp = ferVecNew(vp->params.dim);

        // find best vantage point
        __ferVPTreeFindVP(vp, els, dist, els_len, node->vp, &node->mean);

        do {
            // reorganize els[]
            for (i = 0, cur = 0; i < els_len; i++){
                d = vp->params.dist(vp->params.dim, node->vp, els[i]->p,
                        vp->params.dist_data);
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
            __ferVPTreeBuildAddEls(node, els, els_len);
            ferVecDel(node->vp);
            node->vp = NULL;
        }else{
            // create left and right descendants
            node->left  = __ferVPTreeBuild(vp, els, dist, cur);
            node->right = __ferVPTreeBuild(vp, els + cur, dist, els_len - cur);
        }
    }

    return node;
}
