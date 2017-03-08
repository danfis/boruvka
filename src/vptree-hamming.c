/***
 * Boruvka
 * --------
 * Copyright (c)2013 Daniel Fiser <danfis@danfis.cz>
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


#include <boruvka/vptree-hamming.h>
#include <boruvka/rand-mt.h>
#include <boruvka/alloc.h>
#include <boruvka/dbg.h>
#include <boruvka/nn.h>
#include <boruvka/sort.h>

/* DEBUG:
static char bin_buf[33];
static const char *binUInt32(const unsigned char *_v)
{
    uint32_t v = *(uint32_t *)_v;
    int i;
    for (i = 0; i < 32; i++){
        bin_buf[31 - i] = '0' + (v & 0x1u);
        v >>= 1;
    }
    bin_buf[32] = 0x0;
    return bin_buf;
}
*/

/** Hamming distance between two bytes */
static int hammingDistByte(unsigned char a, unsigned char b);
/** Returns Hamming distance between two points */
static int hammingDist(const unsigned char *p1,
                       const unsigned char *p2,
                       size_t size);

/** Finds out radius and variance */
static void radiusVar(bor_vptree_hamming_t *vp,
                      const unsigned char *p,
                      bor_vptree_hamming_el_t **els,
                      int *dist,
                      size_t len,
                      bor_real_t *radius,
                      bor_real_t *var);
/** Choose best vantage point */
static void bestVP(bor_vptree_hamming_t *vp,
                   bor_vptree_hamming_el_t **candidates, size_t clen,
                   bor_vptree_hamming_el_t **data, size_t dlen,
                   int *dist,
                   unsigned char *vp_out,
                   int *radius_out);
/** Reorganize els[] to have first all elements nearer to {p} than {radius}.
 *  Number of elements in first part is returned */
static int reorganizeEls(bor_vptree_hamming_t *vp,
                         const unsigned char *p, int radius,
                         bor_vptree_hamming_el_t **els, int els_len);

/** Nodes **/
/** Deletes node */
/** Creates new empty node */
static _bor_vptree_hamming_node_t *nodeNew(bor_vptree_hamming_t *vp);
/** Deletes node */
static void nodeDel(bor_vptree_hamming_t *vp, _bor_vptree_hamming_node_t *n);
/** Adds element to node - no check is performed */
static void nodeAdd(bor_vptree_hamming_t *vp, _bor_vptree_hamming_node_t *n,
                    bor_vptree_hamming_el_t *el);
/** Returns best matching leaf node for given element */
static _bor_vptree_hamming_node_t *nodeFindLeaf(bor_vptree_hamming_t *vp,
                                                _bor_vptree_hamming_node_t *n,
                                                const bor_vptree_hamming_el_t *el);
/** Split given node and add given element */
static void nodeAddSplit(bor_vptree_hamming_t *vp,
                         _bor_vptree_hamming_node_t *n,
                         bor_vptree_hamming_el_t *el);

void borVPTreeHammingParamsInit(bor_vptree_hamming_params_t *params)
{
    params->size = 1;

    params->minsize = 1;
    params->maxsize = 2;

    params->samplesize = 5;
}

void borVPTreeHammingElInit(bor_vptree_hamming_el_t *el,
                            const unsigned char *p)
{
    borListInit(&el->list);
    el->p = p;
    el->node = NULL;
}

bor_vptree_hamming_t *borVPTreeHammingNew(const bor_vptree_hamming_params_t *params)
{
    bor_vptree_hamming_t *vp;

    vp = BOR_ALLOC(bor_vptree_hamming_t);

    vp->params = *params;

    vp->root = NULL;

    vp->els_size = vp->params.maxsize + 1;
    vp->els = BOR_ALLOC_ARR(bor_vptree_hamming_el_t *, vp->els_size);

    return vp;
}

void borVPTreeHammingDel(bor_vptree_hamming_t *vp)
{
    if (vp->root)
        nodeDel(vp, vp->root);
    if (vp->els)
        BOR_FREE(vp->els);
    BOR_FREE(vp);
}

void borVPTreeHammingAdd(bor_vptree_hamming_t *vp, bor_vptree_hamming_el_t *el)
{
    _bor_vptree_hamming_node_t *node;

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

void borVPTreeHammingRemove(bor_vptree_hamming_t *vp,
                            bor_vptree_hamming_el_t *el)
{
    _bor_vptree_hamming_node_t *node, *par, *other;

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
        BOR_FREE(par->vp);
        par->left = par->right = NULL;
        nodeDel(vp, par);
    }
}

void borVPTreeHammingUpdate(bor_vptree_hamming_t *vp,
                            bor_vptree_hamming_el_t *el)
{
    borVPTreeHammingRemove(vp, el);
    borVPTreeHammingAdd(vp, el);
}



/** Nearest */
struct _nearest_t {
    const bor_vptree_hamming_t *vp;
    const unsigned char *p;
    size_t num;

    int radius;
    bor_vptree_hamming_el_t **els;
    int *dist;
    size_t els_len;
};
typedef struct _nearest_t nearest_t;

static void nearestAdd(nearest_t *n, bor_vptree_hamming_el_t *el, int dist)
{
    int tmpdist;
    bor_vptree_hamming_el_t *tmpels;
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

static void nearest(nearest_t *n, const _bor_vptree_hamming_node_t *node)
{
    bor_list_t *item;
    bor_vptree_hamming_el_t *el;
    int d, d2, dist;

    if (!node->left && !node->right){
        // node is leaf node, try to add all elements
        BOR_LIST_FOR_EACH(&node->els, item){
            el = BOR_LIST_ENTRY(item, bor_vptree_hamming_el_t, list);
            dist = hammingDist(n->p, el->p, n->vp->params.size);
            if (dist < n->radius){
                nearestAdd(n, el, dist);
                n->radius = n->dist[n->num - 1];
            }
        }
    }else{
        d = hammingDist(n->p, node->vp, n->vp->params.size);
        if (d < node->radius){
            if (d < node->radius + n->radius){
                nearest(n, node->left);
            }

            d2 = node->radius - n->radius;
            if (d >= d2){
                nearest(n, node->right);
            }
        }else{ // d >= node->radius
            d2 = node->radius - n->radius;
            if (d >= d2){
                nearest(n, node->right);
            }

            if (d < node->radius + n->radius){
                nearest(n, node->left);
            }
        }
    }
}
size_t borVPTreeHammingNearest(const bor_vptree_hamming_t *vp,
                               const unsigned char *p, size_t num,
                               bor_vptree_hamming_el_t **els)
{
    nearest_t n;
    size_t i;
    int max_dist;
   
    max_dist = vp->params.size * 8 + 1;

    n.vp  = vp;
    n.p   = p;
    n.num = num;

    n.radius  = max_dist;
    n.els     = els;
    n.els_len = 0;

    n.dist    = BOR_ALLOC_ARR(int, num);
    for (i = 0; i < num; i++)
        n.dist[i] = max_dist;

    nearest(&n, vp->root);

    BOR_FREE(n.dist);

    return n.els_len;
}



static int hammingDistByte(unsigned char a, unsigned char b)
{
    int dist = 0;
    unsigned char val = a ^ b;

    while(val){
        ++dist; 
        val &= val - 1;
    }

    return dist;
}

static int hammingDist(const unsigned char *p1,
                       const unsigned char *p2,
                       size_t size)
{
    int i;
    int dist = 0;

    for (i = 0; i < size; i++){
        dist += hammingDistByte(p1[i], p2[i]);
    }

    return dist;
}


static void radiusVar(bor_vptree_hamming_t *vp,
                      const unsigned char *p,
                      bor_vptree_hamming_el_t **els,
                      int *dist,
                      size_t len,
                      bor_real_t *radius,
                      bor_real_t *var)
{
    size_t i, j, distlen;

    distlen = len;
    for (i = 0, j = 0; i < len; i++){
        if (p == els[i]->p){
            distlen--;
        }else{
            dist[j++] = hammingDist(p, els[i]->p, vp->params.size);
        }
    }

    if (distlen == 0){
        *radius = 0;
        *var    = 0;
        return;
    }


    borSortByIntKey(dist, distlen, sizeof(int), 0);

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

static void bestVP(bor_vptree_hamming_t *vp,
                   bor_vptree_hamming_el_t **candidates, size_t clen,
                   bor_vptree_hamming_el_t **data, size_t dlen,
                   int *dist,
                   unsigned char *vp_out,
                   int *radius_out)
{
    bor_real_t var, radius, best_var;
    const unsigned char *best_vp;
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
    memcpy(vp_out, best_vp, vp->params.size);
}

static int reorganizeEls(bor_vptree_hamming_t *vp,
                         const unsigned char *p, int radius,
                         bor_vptree_hamming_el_t **els, int els_len)
{
    int i, cur;
    int d;
    bor_vptree_hamming_el_t *tmpel;

    do {
        for (i = 0, cur = 0; i < els_len; i++){
            d = hammingDist(p, els[i]->p, vp->params.size);
            if (d < radius){
                if (cur != i){
                    BOR_SWAP(els[i], els[cur], tmpel);
                }
                ++cur;
            }
        }

        if (cur == els_len){
            // TODO
            fprintf(stderr, "cur == els_len\n");
            exit(-1);
            radius -= 10 * BOR_EPS;
        }
    } while (cur == els_len);

    return cur;
}

/** Node **/
static _bor_vptree_hamming_node_t *nodeNew(bor_vptree_hamming_t *vp)
{
    _bor_vptree_hamming_node_t *node;

    node = BOR_ALLOC(_bor_vptree_hamming_node_t);
    node->vp     = NULL;
    node->radius = BOR_ZERO;
    node->parent = node->left = node->right = NULL;
    borListInit(&node->els);
    node->size = 0;

    return node;
}

static void nodeDel(bor_vptree_hamming_t *vp, _bor_vptree_hamming_node_t *n)
{
    if (n->left && n->right){
        nodeDel(vp, n->left);
        nodeDel(vp, n->right);
        BOR_FREE(n->vp);
    }

    BOR_FREE(n);
}

static void nodeAdd(bor_vptree_hamming_t *vp, _bor_vptree_hamming_node_t *n,
                    bor_vptree_hamming_el_t *el)
{
    borListAppend(&n->els, &el->list);
    n->size++;
    el->node = n;
}

static _bor_vptree_hamming_node_t *nodeFindLeaf(bor_vptree_hamming_t *vp,
                                                _bor_vptree_hamming_node_t *n,
                                                const bor_vptree_hamming_el_t *el)
{
    int dist;

    if (!n->left && !n->right){
        // we are at leaf node
        return n;
    }else{
        dist = hammingDist(n->vp, el->p, vp->params.size);
        if (dist < n->radius)
            return nodeFindLeaf(vp, n->left, el);
        return nodeFindLeaf(vp, n->right, el);
    }
}

static void nodeAddSplit(bor_vptree_hamming_t *vp,
                         _bor_vptree_hamming_node_t *n,
                         bor_vptree_hamming_el_t *el)
{
    size_t i;
    int cur;
    bor_vptree_hamming_el_t *e;
    bor_list_t *item;
    int *dist;

    // allocate array if necessary
    if (vp->els_size < n->size + 1){
        BOR_FREE(vp->els);
        vp->els_size = n->size + 1;
        vp->els = BOR_ALLOC_ARR(bor_vptree_hamming_el_t *, vp->els_size);
    }

    // copy all elements to array
    vp->els[0] = el;
    i = 1;
    BOR_LIST_FOR_EACH(&n->els, item){
        e = BOR_LIST_ENTRY(item, bor_vptree_hamming_el_t, list);
        vp->els[i++] = e;
    }


    // create vantage point is needed
    if (!n->vp)
        n->vp = BOR_ALLOC_ARR(unsigned char, vp->params.size);

    // set the best vantage point
    dist = BOR_ALLOC_ARR(int, n->size + 1);
    bestVP(vp, vp->els, n->size + 1, vp->els, n->size + 1, dist, n->vp, &n->radius);
    BOR_FREE(dist);


    // reorganize els[] to left and right side
    cur = reorganizeEls(vp, n->vp, n->radius, vp->els, n->size + 1);
    if (cur == 0 || cur == n->size + 1){
        // partitioning is not possible!
        BOR_FREE(n->vp);
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
