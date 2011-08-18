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


/** Finds out radius and variance */
static void radiusVar(fer_vptree_t *vp,
                      const fer_vec_t *p,
                      fer_vptree_el_t **els,
                      fer_real_t *dist,
                      size_t len,
                      fer_real_t *radius, fer_real_t *var);
/** Choose best vantage point */
static void bestVP(fer_vptree_t *vp,
                   fer_vptree_el_t **candidates, size_t clen,
                   fer_vptree_el_t **data, size_t dlen,
                   fer_real_t *dist,
                   fer_vec_t *vp_out, fer_real_t *radius_out);
/** Reorganize els[] to have first all elements nearer to {p} than {radius}.
 *  Number of elements in first part is returned */
static int reorganizeEls(fer_vptree_t *vp,
                         const fer_vec_t *p, fer_real_t radius,
                         fer_vptree_el_t **els, int els_len);

/** Node **/
/** Creates new empty node */
static _fer_vptree_node_t *nodeNew(fer_vptree_t *vp);
/** Deletes node */
static void nodeDel(fer_vptree_t *vp, _fer_vptree_node_t *n);
/** Adds element to node - no check is performed */
static void nodeAdd(fer_vptree_t *vp, _fer_vptree_node_t *n,
                    fer_vptree_el_t *el);
/** Returns best matching leaf node for given element */
static _fer_vptree_node_t *nodeFindLeaf(fer_vptree_t *vp,
                                        _fer_vptree_node_t *n,
                                        const fer_vptree_el_t *el);
/** Split given node and add given element */
static void nodeAddSplit(fer_vptree_t *vp,
                         _fer_vptree_node_t *n,
                         fer_vptree_el_t *el);

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

/** Adds all elements from els to node */
static void buildAddEls(_fer_vptree_node_t *node,
                        fer_vptree_el_t **els, size_t els_len);
/** Fills {els} with len samples from {els_in} */
static void buildSampleEls(build_t *build, fer_vptree_el_t **els_in, size_t els_len,
                           fer_vptree_el_t **els, size_t len);
/** Builds one level of vp-tree */
static _fer_vptree_node_t *buildNode(build_t *build,
                                     fer_vptree_el_t **els, size_t els_len);


/** Returns distance between v1 and v2 */
_fer_inline fer_real_t ferVPTreeDist(const fer_vptree_t *vp,
                                     const fer_vec_t *v1, const fer_vec_t *v2);
/** Euclidean distance callback */
static fer_real_t ferVPTreeDistCB(int d, const fer_vec_t *v1,
                                  const fer_vec_t *v2, void *data);

void ferVPTreeParamsInit(fer_vptree_params_t *params)
{
    params->dim = 2;
    params->dist = ferVPTreeDistCB;
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

    vp->els_size = vp->params.maxsize + 1;
    vp->els = FER_ALLOC_ARR(fer_vptree_el_t *, vp->els_size);

    return vp;
}

void ferVPTreeDel(fer_vptree_t *vp)
{
    if (vp->root)
        nodeDel(vp, vp->root);
    if (vp->els)
        free(vp->els);
    free(vp);
}


void ferVPTreeAdd(fer_vptree_t *vp, fer_vptree_el_t *el)
{
    _fer_vptree_node_t *node;

    if (!vp->root){
        vp->root = nodeNew(vp);
        nodeAdd(vp, vp->root, el);
    }else{
        node = nodeFindLeaf(vp, vp->root, el);

        if (node->size < vp->params.maxsize){
            // Node is not full - add it there
            nodeAdd(vp, node, el);
        }else{
            // Node is full - split the node
            nodeAddSplit(vp, node, el);
        }
    }
}

void ferVPTreeRemove(fer_vptree_t *vp, fer_vptree_el_t *el)
{
    // TODO
}

void ferVPTreeUpdate(fer_vptree_t *vp, fer_vptree_el_t *el)
{
    ferVPTreeRemove(vp, el);
    ferVPTreeAdd(vp, el);
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
            dist = ferVPTreeDist(n->vp, n->p, el->p);
            if (dist < n->radius){
                nearestAdd(n, el, dist);
                n->radius = n->dist[n->num - 1];
            }
        }
    }else{
        d = ferVPTreeDist(n->vp, n->p, node->vp);
        if (d < node->radius){
            if (d < node->radius + n->radius)
                nearest(n, node->left);

            d2 = node->radius - n->radius;
            if (ferEq(d, d2) || d > d2)
                nearest(n, node->right);
        }else{
            d2 = node->radius - n->radius;
            if (ferEq(d, d2) || d > d2)
                nearest(n, node->right);

            if (d < node->radius + n->radius)
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
        fprintf(out, "), %f", n->radius);
        fprintf(out, " [%lx]\n", (long)n);

        dump(vp, n->left, n, level + 1, out);
        dump(vp, n->right, n, level + 1, out);
    }else{
        FER_LIST_FOR_EACH(&n->els, item){
            el = FER_LIST_ENTRY(item, fer_vptree_el_t, list);
            fprintf(out, "(");
            ferVecPrint(vp->params.dim, el->p, out);
            fprintf(out, ") ");
        }
        fprintf(out, " [%lx]\n", (long)n);
    }

}
void ferVPTreeDump(fer_vptree_t *vp, FILE *out)
{
    if (vp->root)
        dump(vp, vp->root, NULL, 0, out);
}


_fer_inline fer_real_t ferVPTreeDist(const fer_vptree_t *vp,
                                     const fer_vec_t *v1, const fer_vec_t *v2)
{
    return vp->params.dist(vp->params.dim, v1, v2,
                           vp->params.dist_data);
}

static fer_real_t ferVPTreeDistCB(int d, const fer_vec_t *v1,
                                  const fer_vec_t *v2, void *data)
{
    return ferVecDist(d, v1, v2);
}



static int radiusVarCmp(const void *a, const void *b)
{
    fer_real_t f1 = *(fer_real_t *)a;
    fer_real_t f2 = *(fer_real_t *)b;

    if (ferEq(f1, f2))
        return 0;
    if (f1 < f2)
        return -1;
    return 1;
}
static void radiusVar(fer_vptree_t *vp,
                      const fer_vec_t *p,
                      fer_vptree_el_t **els,
                      fer_real_t *dist,
                      size_t len,
                      fer_real_t *radius, fer_real_t *var)
{
    size_t i, j, distlen;

    distlen = len;
    for (i = 0, j = 0; i < len; i++){
        if (p == els[i]->p){
            distlen--;
        }else{
            dist[j++] = ferVPTreeDist(vp, p, els[i]->p);
        }
    }

    qsort(dist, distlen, sizeof(fer_real_t), radiusVarCmp);

    *radius = dist[distlen / 2];
    if (distlen % 2 == 0){
        *radius += dist[distlen / 2 - 1];
        *radius /= FER_REAL(2.);
    }

    *var = FER_CUBE(dist[0] - *radius);
    for (i = 1; i < distlen; i++){
        *var += FER_CUBE(dist[i] - *radius);
    }
    *var /= (fer_real_t)distlen;
}

static void bestVP(fer_vptree_t *vp,
                   fer_vptree_el_t **candidates, size_t clen,
                   fer_vptree_el_t **data, size_t dlen,
                   fer_real_t *dist,
                   fer_vec_t *vp_out, fer_real_t *radius_out)
{
    fer_real_t var, radius, best_var;
    const fer_vec_t *best_vp;
    size_t i;

    best_var = -FER_REAL_MAX;
    best_vp  = NULL;
    for (i = 0; i < clen; i++){
        radiusVar(vp, candidates[i]->p, data, dist, dlen, &radius, &var);
        if (var > best_var){
            best_var    = var;
            *radius_out = radius;
            best_vp     = candidates[i]->p;
        }
    }
    ferVecCopy(vp->params.dim, vp_out, best_vp);
}

static int reorganizeEls(fer_vptree_t *vp,
                         const fer_vec_t *p, fer_real_t radius,
                         fer_vptree_el_t **els, int els_len)
{
    int i, cur;
    fer_real_t d;
    fer_vptree_el_t *tmpel;

    do {
        for (i = 0, cur = 0; i < els_len; i++){
            d = ferVPTreeDist(vp, p, els[i]->p);
            if (d < radius){
                if (cur != i){
                    FER_SWAP(els[i], els[cur], tmpel);
                }
                ++cur;
            }
        }

        if (cur == els_len)
            radius -= 10 * FER_EPS;
    } while (cur == els_len);

    return cur;
}
                         

/** Node **/
static _fer_vptree_node_t *nodeNew(fer_vptree_t *vp)
{
    _fer_vptree_node_t *node;

    node = FER_ALLOC(_fer_vptree_node_t);
    node->vp   = NULL;
    node->radius = FER_ZERO;
    node->parent = node->left = node->right = NULL;
    ferListInit(&node->els);
    node->size = 0;

    return node;
}

static void nodeDel(fer_vptree_t *vp, _fer_vptree_node_t *n)
{
    if (n->left && n->right){
        nodeDel(vp, n->left);
        nodeDel(vp, n->right);
        ferVecDel(n->vp);
    }

    free(n);
}

static void nodeAdd(fer_vptree_t *vp, _fer_vptree_node_t *n,
                    fer_vptree_el_t *el)
{
    ferListAppend(&n->els, &el->list);
    n->size++;
}

static _fer_vptree_node_t *nodeFindLeaf(fer_vptree_t *vp,
                                        _fer_vptree_node_t *n,
                                        const fer_vptree_el_t *el)
{
    fer_real_t dist;

    if (!n->left && !n->right){
        // we are at leaf node
        return n;
    }else{
        dist = ferVPTreeDist(vp, n->vp, el->p);
        if (dist < n->radius)
            return nodeFindLeaf(vp, n->left, el);
        return nodeFindLeaf(vp, n->right, el);
    }
}

static void nodeAddSplit(fer_vptree_t *vp,
                         _fer_vptree_node_t *n,
                         fer_vptree_el_t *el)
{
    size_t i;
    int cur;
    fer_vptree_el_t *e;
    fer_list_t *item;
    fer_real_t *dist;

    // allocate array if necessary
    if (vp->els_size < n->size + 1){
        free(vp->els);
        vp->els_size = n->size + 1;
        vp->els = FER_ALLOC_ARR(fer_vptree_el_t *, vp->els_size);
    }

    // copy all elements to array
    vp->els[0] = el;
    i = 1;
    FER_LIST_FOR_EACH(&n->els, item){
        e = FER_LIST_ENTRY(item, fer_vptree_el_t, list);
        vp->els[i++] = e;
    }


    // create vantage point
    if (n->vp)
        ferVecDel(n->vp);
    n->vp = ferVecNew(vp->params.dim);

    // set the best vantage point
    dist = FER_ALLOC_ARR(fer_real_t, n->size + 1);
    bestVP(vp, vp->els, n->size + 1, vp->els, n->size + 1, dist, n->vp, &n->radius);
    free(dist);


    // reorganize els[] to left and right side
    cur = reorganizeEls(vp, n->vp, n->radius, vp->els, n->size + 1);
    if (cur == 0 || cur == n->size + 1){
        // partitioning is not possible!
        ferVecDel(n->vp);
        n->vp = NULL;
    }else{
        n->left = nodeNew(vp);
        n->left->parent = n;
        for (i = 0; i < cur; i++){
            nodeAdd(vp, n->left, vp->els[i]);
        }

        n->right = nodeNew(vp);
        n->right->parent = n;
        for (; i < n->size + 1; i++){
            nodeAdd(vp, n->right, vp->els[i]);
        }

        ferListInit(&n->els);
        n->size = 0;
    }
}



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


static _fer_vptree_node_t *buildNode(build_t *build,
                                     fer_vptree_el_t **els, size_t els_len)
{
    _fer_vptree_node_t *node;
    size_t cur, len;

    node = nodeNew(build->vp);

    if (els_len <= build->vp->params.maxsize){
        // all elements can fit to current node
        buildAddEls(node, els, els_len);
    }else{
        len = FER_MIN(build->vp->params.samplesize, els_len);

        // create vantage point
        node->vp = ferVecNew(build->vp->params.dim);

        // generate random sample of VPs
        buildSampleEls(build, els, els_len, build->ps, len);

        // random sample data set
        buildSampleEls(build, els, els_len, build->ds, len);

        // find out best vantage point
        bestVP(build->vp, build->ps, len, build->ds, len, build->dist,
               node->vp, &node->radius);

        // reorganize elements
        cur = reorganizeEls(build->vp, node->vp, node->radius, els, els_len);

        if (cur == 0 || cur == els_len){
            buildAddEls(node, els, els_len);
            ferVecDel(node->vp);
            node->vp = NULL;
        }else{
            // create left and right descendants
            node->left  = buildNode(build, els, cur);
            node->left->parent = node;
            node->right = buildNode(build, els + cur, els_len - cur);
            node->right->parent = node;
        }
    }

    return node;
}
