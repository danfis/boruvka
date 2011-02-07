#include <stdio.h>
#include <fermat/vec3.h>
#include <fermat/pc.h>
#include <fermat/alloc.h>
#include <fermat/list.h>
#include <fermat/nearest-linear.h>
#include <fermat/dbg.h>
#include <gann/gng.h>


struct _node_t {
    fer_vec3_t v;
    gann_gng_node_t node;
    fer_list_t list;

    int _id;
};
typedef struct _node_t node_t;

struct _gng_t {
    fer_pc_t *pc;
    fer_pc_it_t pcit;
    size_t max_nodes;

    fer_list_t nodes;

    gann_gng_t *gng;
};
typedef struct _gng_t gng_t;


static void netDumpSVT(gng_t *gng, FILE *out);


static gann_gng_node_t *newNode(const void *input_signal, void *);
static gann_gng_node_t *newNodeBetween(const gann_gng_node_t *n1,
                                       const gann_gng_node_t *n2, void *);
static void delNode(gann_gng_node_t *n, void *);
static const void *inputSignal(void *);
static void nearest(const void *input_signal,
                    gann_gng_node_t **n1, gann_gng_node_t **n2, void *);
static fer_real_t dist2(const void *input_signal, const gann_gng_node_t *node,
                        void *);
static void moveTowards(gann_gng_node_t *node, const void *input_signal,
                        fer_real_t fraction, void *);
static int terminate(void *);

int main(int argc, char *argv[])
{
    gann_gng_params_t params;
    gann_gng_ops_t ops;
    gann_gng_t *gngalg;
    gng_t gng;
    size_t size;

    if (argc != 2){
        fprintf(stderr, "Usage: %s file.pts\n", argv[0]);
        return -1;
    }

    gng.max_nodes = 3000;
    ferListInit(&gng.nodes);
    gng.pc = ferPCNew();
    size = ferPCAddFromFile(gng.pc, argv[1]);
    ferPCPermutate(gng.pc);
    ferPCItInit(&gng.pcit, gng.pc);
    fprintf(stderr, "Added %d points from %s\n", size, argv[1]);

    gannGNGParamsInit(&params);
    ops.new_node         = newNode;
    ops.new_node_between = newNodeBetween;
    ops.del_node         = delNode;
    ops.input_signal     = inputSignal;
    ops.nearest          = nearest;
    ops.dist2            = dist2;
    ops.move_towards     = moveTowards;
    ops.terminate        = terminate;

    gngalg = gannGNGNew(&ops, &params, &gng);
    gng.gng = gngalg;

    gannGNGRun(gngalg);

    netDumpSVT(&gng, stdout);

    gannGNGDel(gngalg);

    ferPCDel(gng.pc);

    return 0;
}

static gann_gng_node_t *newNode(const void *input_signal, void *data)
{
    node_t *n;
    gng_t *gng = (gng_t *)data;

    n = FER_ALLOC(node_t);
    ferVec3Copy(&n->v, (const fer_vec3_t *)input_signal);

    ferListAppend(&gng->nodes, &n->list);
    return &n->node;
}

static gann_gng_node_t *newNodeBetween(const gann_gng_node_t *_n1,
                                       const gann_gng_node_t *_n2,
                                       void *data)
{
    node_t *n1, *n2;
    node_t *newn;
    gng_t *gng = (gng_t *)data;

    n1 = fer_container_of(_n1, node_t, node);
    n2 = fer_container_of(_n2, node_t, node);

    newn = FER_ALLOC(node_t);
    ferVec3Add2(&newn->v, &n1->v, &n2->v);
    ferVec3Scale(&newn->v, FER_REAL(0.5));

    ferListAppend(&gng->nodes, &newn->list);
    return &newn->node;
}

static void delNode(gann_gng_node_t *_n, void *_)
{
    node_t *n;
    n = fer_container_of(_n, node_t, node);
    ferListDel(&n->list);
    free(n);
}

static const void *inputSignal(void *data)
{
    gng_t *gng = (gng_t *)data;
    const fer_vec3_t *v;

    if (ferPCItEnd(&gng->pcit)){
        ferPCPermutate(gng->pc);
        ferPCItInit(&gng->pcit, gng->pc);
    }

    v = ferPCItGet(&gng->pcit);
    ferPCItNext(&gng->pcit);

    return v;
}

static fer_real_t dist22(void *is, fer_list_t *nlist)
{
    node_t *n;
    n = ferListEntry(nlist, node_t, list);
    return ferVec3Dist2((fer_vec3_t *)is, &n->v);
}

static void nearest(const void *input_signal,
                    gann_gng_node_t **n1, gann_gng_node_t **n2, void *data)
{
    gng_t *gng = (gng_t *)data;
    fer_list_t *ns[2];
    node_t *n;

    ns[0] = ns[1] = NULL;
    ferNearestLinear(&gng->nodes, (void *)input_signal, dist22, ns, 2);

    n = ferListEntry(ns[0], node_t, list);
    *n1 = &n->node;
    n = ferListEntry(ns[1], node_t, list);
    *n2 = &n->node;
}

static fer_real_t dist2(const void *input_signal, const gann_gng_node_t *node,
                        void *_)
{
    node_t *n;
    n = fer_container_of(node, node_t, node);
    return ferVec3Dist2((const fer_vec3_t *)input_signal, &n->v);
}

static void moveTowards(gann_gng_node_t *node, const void *input_signal,
                        fer_real_t fraction, void *_)
{
    node_t *n;
    fer_vec3_t move;

    n = fer_container_of(node, node_t, node);
    ferVec3Sub2(&move, (const fer_vec3_t *)input_signal, &n->v);
    ferVec3Scale(&move, fraction);
    ferVec3Add(&n->v, &move);
}

static int terminate(void *data)
{
    gng_t *gng = (gng_t *)data;
    fprintf(stderr, "%d / %d\r", gannGNGNodesLen(gng->gng), gng->max_nodes);
    return gannGNGNodesLen(gng->gng) >= gng->max_nodes;
}


static void netDumpSVT(gng_t *gng, FILE *out)
{
    fer_list_t *list, *item;
    gann_net_node_t *nn;
    gann_gng_node_t *gn;
    gann_net_edge_t *e;
    node_t *n;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    fprintf(out, "Points:\n");
    list = gannGNGNodes(gng->gng);
    i = 0;
    ferListForEach(list, item){
        gn = gannGNGNodeFromList(item);
        n  = fer_container_of(gn, node_t, node);

        n->_id = i++;
        fprintf(out, "%g %g %g\n", ferVec3X(&n->v), ferVec3Y(&n->v), ferVec3Z(&n->v));
    }


    fprintf(out, "Edges:\n");
    list = gannGNGEdges(gng->gng);
    ferListForEach(list, item){
        e = ferListEntry(item, gann_net_edge_t, list);

        nn = gannNetEdgeNode(e, 0);
        gn = gannGNGNodeFromNet(nn);
        n  = fer_container_of(gn, node_t, node);
        id1 = n->_id;

        nn = gannNetEdgeNode(e, 1);
        gn = gannGNGNodeFromNet(nn);
        n  = fer_container_of(gn, node_t, node);
        id2 = n->_id;
        fprintf(out, "%d %d\n", id1, id2);
    }

    fprintf(out, "--------\n");
}
