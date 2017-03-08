/***
 * Boruvka
 * --------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
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

#include <boruvka/vptree.h>
#include <boruvka/rand-mt.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/nn.h>
#include <boruvka/sort.h>


/** Finds out radius and variance */
static void radiusVar(bor_vptree_t *vp,
                      const bor_vec_t *p,
                      bor_vptree_el_t **els,
                      bor_real_t *dist,
                      size_t len,
                      bor_real_t *radius, bor_real_t *var);
/** Choose best vantage point */
static void bestVP(bor_vptree_t *vp,
                   bor_vptree_el_t **candidates, size_t clen,
                   bor_vptree_el_t **data, size_t dlen,
                   bor_real_t *dist,
                   bor_vec_t *vp_out, bor_real_t *radius_out);
/** Reorganize els[] to have first all elements nearer to {p} than {radius}.
 *  Number of elements in first part is returned */
static int reorganizeEls(bor_vptree_t *vp,
                         const bor_vec_t *p, bor_real_t radius,
                         bor_vptree_el_t **els, int els_len);

/** Node **/
/** Creates new empty node */
static _bor_vptree_node_t *nodeNew(bor_vptree_t *vp);
/** Deletes node */
static void nodeDel(bor_vptree_t *vp, _bor_vptree_node_t *n);
/** Adds element to node - no check is performed */
static void nodeAdd(bor_vptree_t *vp, _bor_vptree_node_t *n,
                    bor_vptree_el_t *el);
/** Returns best matching leaf node for given element */
static _bor_vptree_node_t *nodeFindLeaf(bor_vptree_t *vp,
                                        _bor_vptree_node_t *n,
                                        const bor_vptree_el_t *el);
/** Split given node and add given element */
static void nodeAddSplit(bor_vptree_t *vp,
                         _bor_vptree_node_t *n,
                         bor_vptree_el_t *el);

/** Build **/
struct _build_t {
    bor_vptree_t *vp;
    bor_rand_mt_t *rand;
    bor_real_t *dist;
    bor_vptree_el_t **els;
    bor_vptree_el_t **ps;
    bor_vptree_el_t **ds;
};
typedef struct _build_t build_t;

/** Adds all elements from els to node */
static void buildAddEls(bor_vptree_t *vp,
                        _bor_vptree_node_t *node,
                        bor_vptree_el_t **els, size_t els_len);
/** Fills {els} with len samples from {els_in} */
static void buildSampleEls(build_t *build, bor_vptree_el_t **els_in, size_t els_len,
                           bor_vptree_el_t **els, size_t len);
/** Builds one level of vp-tree */
static _bor_vptree_node_t *buildNode(build_t *build,
                                     bor_vptree_el_t **els, size_t els_len);


/** Returns distance between v1 and v2 */
_bor_inline bor_real_t borVPTreeDist(const bor_vptree_t *vp,
                                     const bor_vec_t *v1, const bor_vec_t *v2);
/** Euclidean distance callback */
static bor_real_t borVPTreeDistCB(int d, const bor_vec_t *v1,
                                  const bor_vec_t *v2, void *data);

void borVPTreeParamsInit(bor_vptree_params_t *params)
{
    params->dim = 2;
    params->dist = borVPTreeDistCB;
    params->dist_data = NULL;

    params->minsize = 1;
    params->maxsize = 2;

    params->samplesize = 5;
}

void borVPTreeElInit(bor_vptree_el_t *el, const bor_vec_t *p)
{
    borListInit(&el->list);
    el->p = p;
    el->node = NULL;
}

bor_vptree_t *borVPTreeNew(const bor_vptree_params_t *params)
{
    bor_vptree_t *vp;

    vp = BOR_ALLOC(bor_vptree_t);
    vp->type = BOR_NN_VPTREE;

    vp->params = *params;

    vp->root = NULL;

    vp->els_size = vp->params.maxsize + 1;
    vp->els = BOR_ALLOC_ARR(bor_vptree_el_t *, vp->els_size);

    return vp;
}

void borVPTreeDel(bor_vptree_t *vp)
{
    if (vp->root)
        nodeDel(vp, vp->root);
    if (vp->els)
        BOR_FREE(vp->els);
    BOR_FREE(vp);
}


void borVPTreeAdd(bor_vptree_t *vp, bor_vptree_el_t *el)
{
    _bor_vptree_node_t *node;

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

void borVPTreeRemove(bor_vptree_t *vp, bor_vptree_el_t *el)
{
    _bor_vptree_node_t *node, *par, *other;

    // get parent node
    node = el->node;

    // remove element from node
    borListDel(&el->list);
    node->size--;
    el->node = NULL;
    borListInit(&el->list);

    // check if node isn't empty
    if (node->size == 0){
        // if node is empty, exchange node's parent with other parent's
        // node (branch)

        // first check if node isn't root
        if (node == vp->root){
            nodeDel(vp, vp->root);
            vp->root = NULL;
            return;
        }

        // get parent and other (sibling) node
        par = node->parent;
        if (par->left == node){
            other = par->right;
        }else{
            other = par->left;
        }

        // delete node
        nodeDel(vp, node);

        // exchange parent and other
        if (par == vp->root){
            vp->root = other;
            other->parent = NULL;
        }else{
            if (par->parent->left == par){
                par->parent->left = other;
            }else{
                par->parent->right = other;
            }
            other->parent = par->parent;
        }

        // delete par
        borVecDel(par->vp);
        par->left = par->right = NULL;
        nodeDel(vp, par);
    }
}

void borVPTreeUpdate(bor_vptree_t *vp, bor_vptree_el_t *el)
{
    borVPTreeRemove(vp, el);
    borVPTreeAdd(vp, el);
}


/** Nearest */
struct _nearest_t {
    const bor_vptree_t *vp;
    const bor_vec_t *p;
    size_t num;

    bor_real_t radius;
    bor_vptree_el_t **els;
    bor_real_t *dist;
    size_t els_len;
};
typedef struct _nearest_t nearest_t;

static void nearestAdd(nearest_t *n, bor_vptree_el_t *el, bor_real_t dist)
{
    bor_real_t tmpdist;
    bor_vptree_el_t *tmpels;
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
            BOR_SWAP(n->dist[pos], n->dist[pos - 1], tmpdist);
            BOR_SWAP(n->els[pos], n->els[pos - 1], tmpels);
        }else{
            break;
        }
    }
}

static void nearest(nearest_t *n, const _bor_vptree_node_t *node)
{
    bor_real_t d, d2;
    bor_list_t *item;
    bor_vptree_el_t *el;
    bor_real_t dist;

    if (!node->left && !node->right){
        // node is leaf node, try to add all elements
        BOR_LIST_FOR_EACH(&node->els, item){
            el = BOR_LIST_ENTRY(item, bor_vptree_el_t, list);
            dist = borVPTreeDist(n->vp, n->p, el->p);
            if (dist < n->radius){
                nearestAdd(n, el, dist);
                n->radius = n->dist[n->num - 1];
            }
        }
    }else{
        d = borVPTreeDist(n->vp, n->p, node->vp);
        if (d < node->radius){
            if (d < node->radius + n->radius)
                nearest(n, node->left);

            d2 = node->radius - n->radius;
            if (borEq(d, d2) || d > d2)
                nearest(n, node->right);
        }else{
            d2 = node->radius - n->radius;
            if (borEq(d, d2) || d > d2)
                nearest(n, node->right);

            if (d < node->radius + n->radius)
                nearest(n, node->left);
        }
    }
}

size_t borVPTreeNearest(const bor_vptree_t *vp, const bor_vec_t *p, size_t num,
                        bor_vptree_el_t **els)
{
    nearest_t n;
    size_t i;

    n.vp  = vp;
    n.p   = p;
    n.num = num;

    n.radius  = BOR_REAL_MAX;
    n.els     = els;
    n.els_len = 0;

    n.dist    = BOR_ALLOC_ARR(bor_real_t, num);
    for (i = 0; i < num; i++)
        n.dist[i] = BOR_REAL_MAX;

    nearest(&n, vp->root);

    BOR_FREE(n.dist);

    return n.els_len;
}


static void dump(bor_vptree_t *vp, _bor_vptree_node_t *n, _bor_vptree_node_t *par,
                 int level, FILE *out)
{
    bor_list_t *item;
    bor_vptree_el_t *el;
    int i;


    for (i = 0; i < 4 * level; i++)
        fprintf(out, " ");
    fprintf(out, "par[%lx] ", (long)par);

    if (n->left && n->right){
        fprintf(out, "vp: (");
        borVecPrint(vp->params.dim, n->vp, out);
        fprintf(out, "), %f", n->radius);
        fprintf(out, " [%lx]\n", (long)n);

        dump(vp, n->left, n, level + 1, out);
        dump(vp, n->right, n, level + 1, out);
    }else{
        BOR_LIST_FOR_EACH(&n->els, item){
            el = BOR_LIST_ENTRY(item, bor_vptree_el_t, list);
            fprintf(out, "(");
            borVecPrint(vp->params.dim, el->p, out);
            fprintf(out, ") ");
        }
        fprintf(out, " [%lx]\n", (long)n);
    }

}
void borVPTreeDump(bor_vptree_t *vp, FILE *out)
{
    if (vp->root)
        dump(vp, vp->root, NULL, 0, out);
}


_bor_inline bor_real_t borVPTreeDist(const bor_vptree_t *vp,
                                     const bor_vec_t *v1, const bor_vec_t *v2)
{
    return vp->params.dist(vp->params.dim, v1, v2,
                           vp->params.dist_data);
}

static bor_real_t borVPTreeDistCB(int d, const bor_vec_t *v1,
                                  const bor_vec_t *v2, void *data)
{
    return borVecDist(d, v1, v2);
}



static int radiusVarCmp(const void *a, const void *b, void *_)
{
    bor_real_t f1 = *(bor_real_t *)a;
    bor_real_t f2 = *(bor_real_t *)b;

    if (borEq(f1, f2))
        return 0;
    if (f1 < f2)
        return -1;
    return 1;
}
static void radiusVar(bor_vptree_t *vp,
                      const bor_vec_t *p,
                      bor_vptree_el_t **els,
                      bor_real_t *dist,
                      size_t len,
                      bor_real_t *radius, bor_real_t *var)
{
    size_t i, j, distlen;

    distlen = len;
    for (i = 0, j = 0; i < len; i++){
        if (p == els[i]->p){
            distlen--;
        }else{
            dist[j++] = borVPTreeDist(vp, p, els[i]->p);
        }
    }

    if (distlen == 0){
        *radius = 0;
        *var    = 0;
        return;
    }


    borSort(dist, distlen, sizeof(bor_real_t), radiusVarCmp, NULL);

    *radius = dist[distlen / 2];
    if (distlen % 2 == 0 && distlen > 0){
        *radius += dist[distlen / 2 - 1];
        *radius /= BOR_REAL(2.);
    }

    *var = BOR_SQ(dist[0] - *radius);
    for (i = 1; i < distlen; i++){
        *var += BOR_SQ(dist[i] - *radius);
    }
    *var /= (bor_real_t)distlen;
}

static void bestVP(bor_vptree_t *vp,
                   bor_vptree_el_t **candidates, size_t clen,
                   bor_vptree_el_t **data, size_t dlen,
                   bor_real_t *dist,
                   bor_vec_t *vp_out, bor_real_t *radius_out)
{
    bor_real_t var, radius, best_var;
    const bor_vec_t *best_vp;
    size_t i;

    best_var = -BOR_REAL_MAX;
    best_vp  = NULL;
    for (i = 0; i < clen; i++){
        radiusVar(vp, candidates[i]->p, data, dist, dlen, &radius, &var);
        if (var > best_var){
            best_var    = var;
            *radius_out = radius;
            best_vp     = candidates[i]->p;
        }
    }
    borVecCopy(vp->params.dim, vp_out, best_vp);
}

static int reorganizeEls(bor_vptree_t *vp,
                         const bor_vec_t *p, bor_real_t radius,
                         bor_vptree_el_t **els, int els_len)
{
    int i, cur;
    bor_real_t d;
    bor_vptree_el_t *tmpel;

    do {
        for (i = 0, cur = 0; i < els_len; i++){
            d = borVPTreeDist(vp, p, els[i]->p);
            if (d < radius){
                if (cur != i){
                    BOR_SWAP(els[i], els[cur], tmpel);
                }
                ++cur;
            }
        }

        if (cur == els_len)
            radius -= 10 * BOR_EPS;
    } while (cur == els_len);

    return cur;
}
                         

/** Node **/
static _bor_vptree_node_t *nodeNew(bor_vptree_t *vp)
{
    _bor_vptree_node_t *node;

    node = BOR_ALLOC(_bor_vptree_node_t);
    node->vp   = NULL;
    node->radius = BOR_ZERO;
    node->parent = node->left = node->right = NULL;
    borListInit(&node->els);
    node->size = 0;

    return node;
}

static void nodeDel(bor_vptree_t *vp, _bor_vptree_node_t *n)
{
    if (n->left && n->right){
        nodeDel(vp, n->left);
        nodeDel(vp, n->right);
        borVecDel(n->vp);
    }

    BOR_FREE(n);
}

static void nodeAdd(bor_vptree_t *vp, _bor_vptree_node_t *n,
                    bor_vptree_el_t *el)
{
    borListAppend(&n->els, &el->list);
    n->size++;
    el->node = n;
}

static _bor_vptree_node_t *nodeFindLeaf(bor_vptree_t *vp,
                                        _bor_vptree_node_t *n,
                                        const bor_vptree_el_t *el)
{
    bor_real_t dist;

    if (!n->left && !n->right){
        // we are at leaf node
        return n;
    }else{
        dist = borVPTreeDist(vp, n->vp, el->p);
        if (dist < n->radius)
            return nodeFindLeaf(vp, n->left, el);
        return nodeFindLeaf(vp, n->right, el);
    }
}

static void nodeAddSplit(bor_vptree_t *vp,
                         _bor_vptree_node_t *n,
                         bor_vptree_el_t *el)
{
    size_t i;
    int cur;
    bor_vptree_el_t *e;
    bor_list_t *item;
    bor_real_t *dist;

    // allocate array if necessary
    if (vp->els_size < n->size + 1){
        BOR_FREE(vp->els);
        vp->els_size = n->size + 1;
        vp->els = BOR_ALLOC_ARR(bor_vptree_el_t *, vp->els_size);
    }

    // copy all elements to array
    vp->els[0] = el;
    i = 1;
    BOR_LIST_FOR_EACH(&n->els, item){
        e = BOR_LIST_ENTRY(item, bor_vptree_el_t, list);
        vp->els[i++] = e;
    }


    // create vantage point
    if (n->vp)
        borVecDel(n->vp);
    n->vp = borVecNew(vp->params.dim);

    // set the best vantage point
    dist = BOR_ALLOC_ARR(bor_real_t, n->size + 1);
    bestVP(vp, vp->els, n->size + 1, vp->els, n->size + 1, dist, n->vp, &n->radius);
    BOR_FREE(dist);


    // reorganize els[] to left and right side
    cur = reorganizeEls(vp, n->vp, n->radius, vp->els, n->size + 1);
    if (cur == 0 || cur == n->size + 1){
        // partitioning is not possible!
        borVecDel(n->vp);
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

        borListInit(&n->els);
        n->size = 0;
    }
}



bor_vptree_t *borVPTreeBuild(const bor_vptree_params_t *params,
                             bor_vptree_el_t *_els, size_t els_len, size_t stride)
{
    bor_vptree_t *vp;
    build_t build;
    size_t i;

    vp = borVPTreeNew(params);
    vp->type = BOR_NN_VPTREE;

    build.vp   = vp;
    build.els  = BOR_ALLOC_ARR(bor_vptree_el_t *, els_len);
    build.ps   = BOR_ALLOC_ARR(bor_vptree_el_t *, vp->params.samplesize);
    build.ds   = BOR_ALLOC_ARR(bor_vptree_el_t *, vp->params.samplesize);
    build.dist = BOR_ALLOC_ARR(bor_real_t, els_len);
    build.rand = borRandMTNewAuto();
    for (i = 0; i < els_len; i++){
        build.els[i] = _els;
        _els = (bor_vptree_el_t *)((char *)_els + stride);
    }

    vp->root = buildNode(&build, build.els, els_len);

    BOR_FREE(build.els);
    BOR_FREE(build.dist);
    BOR_FREE(build.ps);
    BOR_FREE(build.ds);
    borRandMTDel(build.rand);

    return vp;
}

static void buildAddEls(bor_vptree_t *vp,
                        _bor_vptree_node_t *node,
                        bor_vptree_el_t **els,
                        size_t els_len)
{
    size_t i;

    for (i = 0; i < els_len; i++){
        nodeAdd(vp, node, els[i]);
    }
}

static void buildSampleEls(build_t *build, bor_vptree_el_t **els_in, size_t els_len,
                           bor_vptree_el_t **els, size_t len)
{
    size_t i, p;

    for (i = 0; i < len; i++){
        p = borRandMT(build->rand, 0, els_len);
        els[i] = els_in[p];
    }
}


static _bor_vptree_node_t *buildNode(build_t *build,
                                     bor_vptree_el_t **els, size_t els_len)
{
    _bor_vptree_node_t *node;
    size_t i, cur, len;

    node = nodeNew(build->vp);

    if (els_len <= build->vp->params.maxsize){
        // all elements can fit to current node
        buildAddEls(build->vp, node, els, els_len);
    }else{
        // create vantage point
        node->vp = borVecNew(build->vp->params.dim);

        // generate random sample of VPs and datas
        if (build->vp->params.samplesize < els_len){
            len = build->vp->params.samplesize;
            buildSampleEls(build, els, els_len, build->ps, len);
            buildSampleEls(build, els, els_len, build->ds, len);
        }else{
            len = els_len;
            for (i = 0; i < len; i++){
                build->ps[i] = els[i];
                build->ds[i] = els[i];
            }
        }

        // find out best vantage point
        bestVP(build->vp, build->ps, len, build->ds, len, build->dist,
               node->vp, &node->radius);

        // reorganize elements
        cur = reorganizeEls(build->vp, node->vp, node->radius, els, els_len);

        if (cur == 0 || cur == els_len){
            buildAddEls(build->vp, node, els, els_len);
            borVecDel(node->vp);
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
