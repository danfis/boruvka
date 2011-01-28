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

#include <fermat/surface/gsrm.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>

struct _node_t {
    // TODO: must be changed to pointer (SSE)
    fer_vec3_t v; /*!< Position of node (weight vector) */

    fer_real_t err_counter;  /*!< Error counter */
    size_t err_counter_mark; /*!< Mark used for accumulated error counter */

    int simpl_ban; /*!< true if node is banned from simplification (deletion) */

    fer_mesh3_vertex_t vert; /*!< Vertex in mesh */
    fer_cubes3_el_t cubes;   /*!< Struct for NN search */
};
typedef struct _node_t node_t;

struct _edge_t {
    int age;      /*!< Age of edge */
    int obsolete; /*!< Obsolete flag (TODO) */

    fer_mesh3_edge_t edge; /*!< Edge in mesh */
};
typedef struct _edge_t edge_t;

struct _face_t {
    fer_mesh3_face_t face; /*!< Face in mesh */
};
typedef struct _face_t face_t;

struct _fer_gsrm_cache_t {
    fer_vec3_t *is;     /*!< Input signal */
    node_t *nearest[2]; /*!< Two nearest nodes */
    node_t **common_neighb; /*!< Array of common neighbors */
    size_t common_neighb_size;
    size_t common_neighb_len;

    size_t err_counter_mark; /*!< TODO */
    fer_real_t err_counter_scale; /*!< TODO */
};
typedef struct _fer_gsrm_cache_t fer_gsrm_cache_t;

/** Allocates and deallocates cache */
static fer_gsrm_cache_t *cacheNew(void);
static void cacheDel(fer_gsrm_cache_t *c);

/** Creates new node */
static node_t *nodeNew(fer_gsrm_t *g, const fer_vec3_t *v);
/** Deletes node */
static void nodeDel(fer_gsrm_t *g, node_t *n);

/** TODO */
static void nodeErrCounterApply(fer_gsrm_t *g, node_t *n);
static fer_real_t nodeErrCounter(fer_gsrm_t *g, node_t *n);
static fer_real_t nodeErrCounterScale(fer_gsrm_t *g, node_t *n, fer_real_t s);
static fer_real_t nodeErrCounterReset(fer_gsrm_t *g, node_t *n);
/** Increases error counter (TODO) */
static void nodeErrCounterInc(fer_gsrm_t *g, node_t *n, const fer_vec3_t *v);
/** Reset all error counters */
static void nodeErrCounterResetAll(fer_gsrm_t *g);
/** Scale all error counters */
static void nodeErrCounterScaleAll(fer_gsrm_t *g);


static edge_t *edgeNew(fer_gsrm_t *g, node_t *n1, node_t *n2);
static void edgeDel(fer_gsrm_t *g, edge_t *e);

/** TODO */
static face_t *faceNew(fer_gsrm_t *g, edge_t *e, node_t *n);
static void faceDel(fer_gsrm_t *g, face_t *e);


/** Initializes mesh with three random numbers from input */
static void meshInit(fer_gsrm_t *g);
/** Performes Extended Competitive Hebbian Learning */
static void echl(fer_gsrm_t *g);
static void echlConnectNodes(fer_gsrm_t *g);
static void echlMove(fer_gsrm_t *g);
static void echlUpdate(fer_gsrm_t *g);
/** Creates new node */
static void createNewNode(fer_gsrm_t *g);
/** Decreases error counter */
static void decreaseErrCounter(fer_gsrm_t *g);


fer_gsrm_t *ferGSRMNew(void)
{
    fer_gsrm_t *g;

    g = FER_ALLOC(fer_gsrm_t);

    // init params:
    g->param.lambda = 200;
    g->param.eb = 0.05;
    g->param.en = 0.0006;
    g->param.alpha = 0.95;
    g->param.beta = 0.9995;
    g->param.age_max = 200;
    g->param.max_nodes = 5000;
    g->param.num_cubes = 5000;

    g->param.min_dangle = M_PI_4;
    g->param.max_angle = M_PI_2 * 1.5;
    g->param.angle_merge_edges = M_PI * 0.9;

    g->param.simpl_dist_treshold = 0.;
    g->param.simpl_max_node_dec = -1.;
    g->param.simpl_max_face_dec = -1.;

    // initialize point cloude (input signals)
    g->is = ferPCNew();

    // init 3D mesh
    g->mesh = ferMesh3New();

    // init cubes for NN search to NULL, actual allocation will be made
    // after we know what area do we need to cover
    g->cubes = NULL;

    g->c = NULL;

    return g;
}

void ferGSRMDel(fer_gsrm_t *g)
{
    if (g->c)
        cacheDel(g->c);

    if (g->is)
        ferPCDel(g->is);

    // TODO: This is on 100% wrong because we need to deallocate also all
    //       nodes (vertices), edges and faces
    if (g->mesh)
        ferMesh3Del(g->mesh);

    if (g->cubes)
        ferCubes3Del(g->cubes);

    free(g);
}

size_t ferGSRMAddInputSignals(fer_gsrm_t *g, const char *fn)
{
    return ferPCAddFromFile(g->is, fn);
}

int ferGSRMRun(fer_gsrm_t *g)
{
    const fer_real_t *aabb;
    size_t step;

    // check if there are some input signals
    if (ferPCLen(g->is) <= 3){
        DBG2("No input signals!");
        return -1;
    }

    // initialize cache
    if (!g->c)
        g->c = cacheNew();

    // initialize NN search structure
    if (!g->cubes){
        aabb = ferPCAABB(g->is);
        g->cubes = ferCubes3New(aabb, g->param.num_cubes);
    }

    // first shuffle of all input signals
    ferPCPermutate(g->is);
    // and initialize is's iterator
    ferPCItInit(&g->isit, g->is);

    step = 1;
    while (ferMesh3VerticesLen(g->mesh) < g->param.max_nodes){
        echl(g);

        if (step >= g->param.lambda){
            createNewNode(g);
            step = 0;
        }

        decreaseErrCounter(g);

        step++;
    }

    // TODO
    return -1;
}



static fer_gsrm_cache_t *cacheNew(void)
{
    fer_gsrm_cache_t *c;

    c = FER_ALLOC(fer_gsrm_cache_t);
    c->nearest[0] = c->nearest[1] = NULL;

    c->common_neighb_size = 3;
    c->common_neighb = FER_ALLOC_ARR(node_t *, c->common_neighb_size);
    c->common_neighb_len = 0;

    c->err_counter_mark = 0;
    c->err_counter_scale = FER_ONE;

    return c;
}

static void cacheDel(fer_gsrm_cache_t *c)
{
    free(c->common_neighb);
    free(c);
}



static node_t *nodeNew(fer_gsrm_t *g, const fer_vec3_t *v)
{
    node_t *n;

    n = FER_ALLOC(node_t);
    ferVec3Copy(&n->v, v);

    // initialize mesh's vertex struct with weight vector
    ferMesh3VertexSetCoords(&n->vert, &n->v);

    // initialize cubes struct with its own weight vector
    ferCubes3ElInit(&n->cubes, &n->v);

    // add node into mesh
    ferMesh3AddVertex(g->mesh, &n->vert);
    // and add node into cubes
    ferCubes3Add(g->cubes, &n->cubes);

    return n;
}

static void nodeDel(fer_gsrm_t *g, node_t *n)
{
    fer_list_t *list, *item, *item_tmp;
    fer_list_m_t *mitem;
    fer_mesh3_edge_t *edge;
    edge_t *e;

    // remove node from mesh
    // remove edges first
    list = ferMesh3VertexEdges(&n->vert);
    ferListForEachSafe(list, item, item_tmp){
        mitem = ferListMFromList(item);
        edge = ferListEntry(item, fer_mesh3_edge_t, vlist[mitem->mark]);
        e = fer_container_of(edge, edge_t, edge);

        edgeDel(g, e);
    }
    // then vertex
    ferMesh3RemoveVertex(g->mesh, &n->vert);

    // remove node from cubes
    ferCubes3Remove(g->cubes, &n->cubes);

    // Note: no need of deallocation of .vert and .cubes
    free(n);
}

static void nodeErrCounterApply(fer_gsrm_t *g, node_t *n)
{
    size_t mark, left;
    fer_real_t err;

    mark = n->err_counter_mark;
    left = g->c->err_counter_mark - mark;

    if (fer_likely(left > 0)){
        if (fer_likely(left == g->c->err_counter_mark)){
            // most of nodes in mesh are not touched while ECHL phase - so
            // scale factor can be cumulated and can be used directly
            // without pow() operation
            nodeErrCounterScale(g, n, g->c->err_counter_mark);
        }else{
            err = FER_POW(g->param.beta, (fer_real_t)left);
            nodeErrCounterScale(g, n, err);
        }
    }

    n->err_counter_mark = g->c->err_counter_mark;
}

static fer_real_t nodeErrCounter(fer_gsrm_t *g, node_t *n)
{
    nodeErrCounterApply(g, n);
    return n->err_counter;
}

static fer_real_t nodeErrCounterScale(fer_gsrm_t *g, node_t *n, fer_real_t s)
{
    nodeErrCounterApply(g, n);
    n->err_counter *= s;
    return n->err_counter;
}

static fer_real_t nodeErrCounterReset(fer_gsrm_t *g, node_t *n)
{
    nodeErrCounterApply(g, n);
    n->err_counter_mark = 0;
    return n->err_counter;
}

static void nodeErrCounterInc(fer_gsrm_t *g, node_t *n, const fer_vec3_t *v)
{
    fer_real_t dist;

    dist = ferVec3Dist2(&n->v, v);

    nodeErrCounterApply(g, n);
    n->err_counter += dist;
}

static void nodeErrCounterResetAll(fer_gsrm_t *g)
{
    g->c->err_counter_mark = 0;
    g->c->err_counter_scale = FER_ONE;
}

static void nodeErrCounterScaleAll(fer_gsrm_t *g)
{
    g->c->err_counter_scale *= g->param.beta;
    g->c->err_counter_mark++;
}




static edge_t *edgeNew(fer_gsrm_t *g, node_t *n1, node_t *n2)
{
    edge_t *e;

    e = FER_ALLOC(edge_t);
    e->age = 0;
    e->obsolete = 0;

    ferMesh3AddEdge(g->mesh, &e->edge, &n1->vert, &n2->vert);

    return e;
}

static void edgeDel(fer_gsrm_t *g, edge_t *e)
{
    fer_mesh3_face_t *face;

    // first remove incidenting faces
    face = ferMesh3EdgeFace(&e->edge, 0);
    if (face){
        faceDel(g, fer_container_of(face, face_t, face));
    }

    face = ferMesh3EdgeFace(&e->edge, 1);
    if (face){
        faceDel(g, fer_container_of(face, face_t, face));
    }

    // then remove edge itself
    ferMesh3RemoveEdge(g->mesh, &e->edge);

    free(e);
}




static face_t *faceNew(fer_gsrm_t *g, edge_t *e, node_t *n)
{
    face_t *f;
    fer_mesh3_edge_t *e2, *e3;

    f = FER_ALLOC(face_t);

    e2 = ferMesh3VertexCommonEdge(ferMesh3EdgeVertex(&e->edge, 0), &n->vert);
    e3 = ferMesh3VertexCommonEdge(ferMesh3EdgeVertex(&e->edge, 1), &n->vert);
    if (!e2 || !e3)
        return NULL;

    ferMesh3AddFace(g->mesh, &f->face, &e->edge, e2, e3);

    return f;
}

static void faceDel(fer_gsrm_t *g, face_t *f)
{
    ferMesh3RemoveFace(g->mesh, &f->face);
    free(f);
}


static void meshInit(fer_gsrm_t *g)
{
    fer_vec3_t *v;
    node_t *n;
    size_t i;

    for (i = 0; i < 3; i++){
        // obtain input signal
        v = ferPCItGet(&g->isit);

        // create new node
        n = nodeNew(g, v);

        // move to next point
        ferPCItNext(&g->isit);
    }
}

static void drawInputPoint(fer_gsrm_t *g)
{
    if (ferPCItEnd(&g->isit)){
        // if iterator is at the end permutate point cloud again
        ferPCPermutate(g->is);
        // and re-initialize iterator
        ferPCItInit(&g->isit, g->is);
    }
    g->c->is = ferPCItGet(&g->isit);
}

static void echl(fer_gsrm_t *g)
{
    fer_cubes3_el_t *el[2];

    // 1. Find two nearest nodes
    ferCubes3Nearest(g->cubes, g->c->is, 2, el);
    g->c->nearest[0] = fer_container_of(el[0], node_t, cubes);
    g->c->nearest[1] = fer_container_of(el[1], node_t, cubes);

    // 2. Updates winners error counter
    nodeErrCounterInc(g, g->c->nearest[0], g->c->is);

    // 3. Connect winning nodes
    echlConnectNodes(g);

    // 4. Move winning node and its neighbors towards input signal
    echlMove(g);

    // 5. Update all edges emitating from winning node
    echlUpdate(g);
}

_fer_inline edge_t *echlCommonEdge(node_t *n1, node_t *n2)
{
    fer_list_t *list, *item;
    fer_list_m_t *mlist;
    fer_mesh3_edge_t *edge;
    edge_t *e;

    if (ferMesh3VertexEdgesLen(&n1->vert) <= ferMesh3VertexEdgesLen(&n2->vert)){
        list = ferMesh3VertexEdges(&n1->vert);
    }else{
        list = ferMesh3VertexEdges(&n2->vert);
    }

    ferListForEach(list, item){
        mlist = ferListMFromList(item);
        edge = ferListEntry(item, fer_mesh3_edge_t, vlist[mlist->mark]);

        if ((edge->v[0] == &n1->vert && edge->v[1] == &n2->vert)
                || (edge->v[1] == &n1->vert && edge->v[0] == &n2->vert)){
            e = fer_container_of(edge, edge_t, edge);
            return e;
        }

    }

    return NULL;
}

static void echlCommonNeighbors(fer_gsrm_t *g, node_t *n1, node_t *n2)
{
    fer_list_t *list1, *list2;
    fer_list_t *item1, *item2;
    fer_list_m_t *mitem1, *mitem2;
    fer_mesh3_edge_t *edge1, *edge2;
    fer_mesh3_vertex_t *o1, *o2;
    node_t *n;
    size_t len;

    // allocate enough memory for common neighbors
    if (g->c->common_neighb_size < ferMesh3VertexEdgesLen(&n1->vert)
            && g->c->common_neighb_size < ferMesh3VertexEdgesLen(&n2->vert)){
        len = ferMesh3VertexEdgesLen(&n1->vert);
        len = FER_MIN(len, ferMesh3VertexEdgesLen(&n2->vert));

        g->c->common_neighb = FER_REALLOC_ARR(g->c->common_neighb, node_t *, len);
        g->c->common_neighb_size = len;
    }

    list1 = ferMesh3VertexEdges(&n1->vert);
    list2 = ferMesh3VertexEdges(&n2->vert);
    len = 0;
    ferListForEach(list1, item1){
        mitem1 = ferListMFromList(item1);
        edge1 = ferListEntry(item1, fer_mesh3_edge_t, vlist[mitem1->mark]);
        o1 = ferMesh3EdgeVertex(edge1, 0);
        if (o1 == &n1->vert)
            o1 = ferMesh3EdgeVertex(edge1, 1);

        ferListForEach(list2, item2){
            mitem2 = ferListMFromList(item2);
            edge2 = ferListEntry(item2, fer_mesh3_edge_t, vlist[mitem2->mark]);
            o2 = ferMesh3EdgeVertex(edge2, 0);
            if (o2 == &n2->vert)
                o2 = ferMesh3EdgeVertex(edge2, 1);

            if (o1 == o2){
                n = fer_container_of(o1, node_t, vert);
                g->c->common_neighb[len] = n;
                len++;
            }
        }
    }

    g->c->common_neighb_len = len;
}

static void echlRemoveThales(fer_gsrm_t *g, edge_t *e, node_t *n1, node_t *n2)
{
    node_t *nb;
    size_t i, len;

    len = g->c->common_neighb_len;
    for (i=0; i < len; i++){
        nb = g->c->common_neighb[i];

        if (ferVec3Angle(&n1->v, &nb->v, &n2->v) > M_PI_2){
            // remove edge
            edgeDel(g, e);
            return;
        }
    }
}

static void echlRemoveNeighborsEdges(fer_gsrm_t *g)
{
    size_t i, j, len;
    fer_mesh3_edge_t *edge;
    node_t **ns;

    ns = g->c->common_neighb;
    len = g->c->common_neighb_len;
    if (len == 0)
        return;

    for (i = 0; i < len; i++){
        for (j = i + 1; j < len; j++){
            edge = ferMesh3VertexCommonEdge(&ns[i]->vert, &ns[j]->vert);
            if (edge != NULL){
                edgeDel(g, fer_container_of(edge, edge_t, edge));
                //DBG("Deleting edge %lx", (long)e);
            }
        }
    }
}

static void echlCreateFaces(fer_gsrm_t *g, edge_t *e)
{
    size_t i, len;
    node_t **ns;

    len = FER_MIN(g->c->common_neighb_len, 2);
    ns = g->c->common_neighb;
    for (i = 0; i < len; i++){
        faceNew(g, e, ns[i]);
    }
}

static void echlConnectNodes(fer_gsrm_t *g)
{
    edge_t *e;
    node_t *n1, *n2;

    n1 = g->c->nearest[0];
    n2 = g->c->nearest[1];

    // get edge connecting n1 and n2
    e = echlCommonEdge(n1, n2);

    // get common neighbors
    echlCommonNeighbors(g, n1, n2);

    if (e != NULL){
        //DBG2("Nodes are connected");

        // set age of edge to zero
        e->age = 0;

        // Remove edge if opposite node lies inside thales sphere
        echlRemoveThales(g, e, n1, n2);
    }else{
        //DBG2("Nodes are NOT connected");

        // remove all edges that connect common neighbors
        echlRemoveNeighborsEdges(g);

        // create new edge between n1 and n2
        e = edgeNew(g, n1, n2);

        // create faces with common neighbors
        echlCreateFaces(g, e);
    }
}

_fer_inline void echlMoveNode(fer_gsrm_t *g, node_t *n, fer_real_t k)
{
    fer_vec3_t v;
    ferVec3Sub2(&v, g->c->is, &n->v);
    ferVec3Scale(&v, k);
    ferVec3Add(&n->v, &v);
}

static void echlMove(fer_gsrm_t *g)
{
    fer_list_t *list, *item;
    fer_list_m_t *mitem;
    fer_mesh3_edge_t *edge;
    fer_mesh3_vertex_t *wvert, *vert;
    node_t *wn;

    wn = g->c->nearest[0];
    wvert = &wn->vert;

    // move winning node
    echlMoveNode(g, wn, g->param.eb);

    // move nodes connected with the winner
    list = ferMesh3VertexEdges(wvert);
    ferListForEach(list, item){
        mitem = ferListMFromList(item);
        edge = ferListEntry(item, fer_mesh3_edge_t, vlist[mitem->mark]);
        vert = ferMesh3EdgeOtherVertex(edge, wvert);

        echlMoveNode(g, fer_container_of(vert, node_t, vert), g->param.en);
    }
}

static void echlUpdate(fer_gsrm_t *g)
{
    // TODO: gsrmMeshECHLUpdateEdges(gsrm_mesh_t *m)
}
