#include <fermat/gng.h>
#include <fermat/vec2.h>
#include <fermat/pc2.h>
#include <fermat/cubes2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/timer.h>
#include <fermat/dbg.h>


struct _node_t {
    fer_gng_node_t gng;
    fer_vec2_t *w;

    fer_cubes2_el_t cubes;
    fer_dij_node_t dij;

    int initgoal;
    int _id;
};
typedef struct _node_t node_t;

struct _plan_t {
    fer_gng_t *gng;
    fer_pc2_t *pc;
    fer_pc2_it_t pcit;
    fer_cubes2_t *cubes;
    fer_dij_t *dij;

    fer_timer_t timer;

    size_t max_nodes;

    fer_vec2_t *start, *goal;
    node_t *start_node, *goal_node;
};
typedef struct _plan_t plan_t;

/** Operations for fer_gng_ops_t struct */
static void init(fer_gng_node_t **n1, fer_gng_node_t **n2, void *p);
static fer_gng_node_t *new_node(const void *input_signal, void *);
static fer_gng_node_t *new_node_between(const fer_gng_node_t *n1,
                                         const fer_gng_node_t *n2, void *);
static void del_node(fer_gng_node_t *n, void *);
static const void *input_signal(void *);
static void nearest(const void *input_signal,
                    fer_gng_node_t **n1, fer_gng_node_t **n2, void *);
static fer_real_t dist2(const void *input_signal,
                        const fer_gng_node_t *node, void *);
static void move_towards(fer_gng_node_t *node, const void *input_signal,
                         fer_real_t fraction, void *);
static int terminate(void *data);
static void callback(void *data);

static void netDumpSVT(fer_gng_t *gng, FILE *out, const char *name);

/** Dijkstra callbacks */
static fer_real_t dij_dist(const fer_dij_node_t *n1, const fer_dij_node_t *n2, void *);
static void dij_expand(fer_dij_node_t *n, fer_list_t *list, void *);


static void addStartGoalNodes(plan_t *p);
static void dijDumpPath(plan_t *p);

int main(int argc, char *argv[])
{
    fer_gng_ops_t gng_ops;
    fer_gng_params_t gng_params;
    fer_dij_ops_t dij_ops;
    plan_t plan;
    size_t size;
    const fer_real_t *aabb;
    int res;

    if (argc != 3){
        fprintf(stderr, "Usage: %s file-2d.pts max_nodes\n", argv[0]);
        return -1;
    }

    ferGNGOpsInit(&gng_ops);
    ferGNGParamsInit(&gng_params);
    gng_ops.init             = init;
    gng_ops.new_node         = new_node;
    gng_ops.new_node_between = new_node_between;
    gng_ops.del_node         = del_node;
    gng_ops.input_signal     = input_signal;
    gng_ops.nearest          = nearest;
    gng_ops.dist2            = dist2;
    gng_ops.move_towards     = move_towards;
    gng_ops.terminate        = terminate;
    gng_ops.callback         = callback;
    gng_ops.callback_period  = 100;
    gng_ops.data = &plan;
    gng_ops.terminate_data = &plan;

    plan.gng = ferGNGNew(&gng_ops, &gng_params);

    plan.pc  = ferPC2New();
    size = ferPC2AddFromFile(plan.pc, argv[1]);
    fprintf(stderr, "Added %u points.\n", size);

    plan.max_nodes = atoi(argv[2]);

    aabb = ferPC2AABB(plan.pc);
    plan.cubes = ferCubes2New(aabb, plan.max_nodes);

    plan.start = ferVec2New(1, 1);
    plan.goal  = ferVec2New(14, 9);


    ferTimerStart(&plan.timer);
    ferGNGRun(plan.gng);
    callback(&plan);
    fprintf(stderr, "\n");


    addStartGoalNodes(&plan);

    dij_ops.dist   = dij_dist;
    dij_ops.expand = dij_expand;
    plan.dij = ferDijNew(&dij_ops);

    res = ferDijRun(plan.dij, &plan.start_node->dij,
                              &plan.goal_node->dij);
    if (res != 0){
        fprintf(stderr, "Can't find path!\n");
    }else{
        dijDumpPath(&plan);
    }

    ferDijDel(plan.dij);

    netDumpSVT(plan.gng, stdout, NULL);

    ferGNGDel(plan.gng);
    ferPC2Del(plan.pc);
    ferCubes2Del(plan.cubes);
    ferVec2Del(plan.start);
    ferVec2Del(plan.goal);

    return 0;
}



static void init(fer_gng_node_t **n1, fer_gng_node_t **n2, void *_p)
{
    plan_t *p = (plan_t *)_p;
    node_t *nn;

    ferPC2Permutate(p->pc);
    ferPC2ItInit(&p->pcit, p->pc);

    *n1 = new_node(p->start, p);
    *n2 = new_node(p->goal, p);

    nn = fer_container_of(*n1, node_t, gng);
    nn->initgoal = 1;
    nn = fer_container_of(*n2, node_t, gng);
    nn->initgoal = 1;
}

static fer_gng_node_t *new_node(const void *input_signal, void *_p)
{
    plan_t *p = (plan_t *)_p;
    node_t *n;
    const fer_vec2_t *is;

    is = (const fer_vec2_t *)input_signal;

    n = FER_ALLOC(node_t);
    n->w = ferVec2Clone(is);

    ferCubes2ElInit(&n->cubes, n->w);
    ferCubes2Add(p->cubes, &n->cubes);

    ferDijNodeInit(&n->dij);

    n->initgoal = 0;

    return &n->gng;
}

static fer_gng_node_t *new_node_between(const fer_gng_node_t *_n1,
                                         const fer_gng_node_t *_n2, void *_p)
{
    node_t *n1, *n2;
    fer_vec2_t v;

    n1 = fer_container_of(_n1, node_t, gng);
    n2 = fer_container_of(_n2, node_t, gng);

    ferVec2Add2(&v, n1->w, n2->w);
    ferVec2Scale(&v, FER_REAL(0.5));

    return new_node(&v, _p);
}

static void del_node(fer_gng_node_t *_n, void *_p)
{
    plan_t *p = (plan_t *)_p;
    node_t *n;

    n = fer_container_of(_n, node_t, gng);
    ferVec2Del(n->w);
    ferCubes2Remove(p->cubes, &n->cubes);
    free(n);
}

static const void *input_signal(void *_p)
{
    plan_t *p = (plan_t *)_p;
    const fer_vec2_t *v;

    if (ferPC2ItEnd(&p->pcit)){
        ferPC2Permutate(p->pc);
        ferPC2ItInit(&p->pcit, p->pc);
    }

    v = ferPC2ItGet(&p->pcit);
    ferPC2ItNext(&p->pcit);

    return v;
}

static void nearest(const void *input_signal,
                    fer_gng_node_t **n1, fer_gng_node_t **n2, void *_p)
{
    plan_t *p = (plan_t *)_p;
    fer_cubes2_el_t *els[2];
    node_t *n;

    *n1 = *n2 = NULL;

    ferCubes2Nearest(p->cubes, (const fer_vec2_t *)input_signal, 2, els);

    n = fer_container_of(els[0], node_t, cubes);
    *n1 = &n->gng;
    n = fer_container_of(els[1], node_t, cubes);
    *n2 = &n->gng;
}

static fer_real_t dist2(const void *input_signal,
                        const fer_gng_node_t *node, void *_)
{
    node_t *n;

    n = fer_container_of(node, node_t, gng);
    return ferVec2Dist2((const fer_vec2_t *)input_signal, n->w);
}

static void move_towards(fer_gng_node_t *node, const void *input_signal,
                         fer_real_t fraction, void *_p)
{
    plan_t *p= (plan_t *)_p;
    node_t *n;
    fer_vec2_t move;

    n = fer_container_of(node, node_t, gng);
    ferVec2Sub2(&move, (const fer_vec2_t *)input_signal, n->w);
    ferVec2Scale(&move, fraction);
    ferVec2Add(n->w, &move);

    ferCubes2Update(p->cubes, &n->cubes);
}


static int terminate(void *_p)
{
    plan_t *p = (plan_t *)_p;
    return ferGNGNodesLen(p->gng) >= p->max_nodes;
}

static void callback(void *_p)
{
    plan_t *p = (plan_t *)_p;
    size_t nodes_len;

    nodes_len = ferGNGNodesLen(p->gng);

    ferTimerStopAndPrintElapsed(&p->timer, stderr, " n: %d / %d\r", nodes_len, p->max_nodes);
}

static void netDumpSVT(fer_gng_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gng_node_t *gn;
    fer_net_edge_t *e;
    node_t *n;
    size_t i, id1, id2;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferGNGNodes(gng);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        gn = ferGNGNodeFromList(item);
        n  = fer_container_of(gn, node_t, gng);

        n->_id = i++;
        ferVec2Print(n->w, out);
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGEdges(gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, node_t, gng);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        gn = ferGNGNodeFromNet(nn);
        n  = fer_container_of(gn, node_t, gng);
        id2 = n->_id;
        fprintf(out, "%d %d\n", id1, id2);
    }

    fprintf(out, "--------\n");
}



static fer_real_t dij_dist(const fer_dij_node_t *_n1,
                           const fer_dij_node_t *_n2, void *_)
            
{
    node_t *n1, *n2;
    n1 = fer_container_of(_n1, node_t, dij);
    n2 = fer_container_of(_n2, node_t, dij);

    return ferVec2Dist(n1->w, n2->w);
}

static void dij_expand(fer_dij_node_t *_n, fer_list_t *neighbors, void *_p)
{
    fer_list_t *list, *item;
    node_t *n, *o;
    fer_net_node_t *net_n, *net_o;
    fer_gng_node_t *gng_o;
    fer_net_edge_t *e;

    n = fer_container_of(_n, node_t, dij);

    net_n = ferGNGNodeToNet(&n->gng);
    list = ferNetNodeEdges(net_n);
    FER_LIST_FOR_EACH(list, item){
        e = ferNetEdgeFromNodeList(item);
        net_o = ferNetEdgeOtherNode(e, net_n);
        gng_o = ferGNGNodeFromNet(net_o);
        o     = fer_container_of(gng_o, node_t, gng);

        if (!ferDijNodeClosed(&o->dij)){
            ferDijNodeAdd(&o->dij, neighbors);
        }
    }
}

static void addStartGoalNodes(plan_t *p)
{
    fer_gng_node_t *node;

    node = ferGNGConnectNewNode(p->gng, p->start);
    p->start_node = fer_container_of(node, node_t, gng);
    node = ferGNGConnectNewNode(p->gng, p->goal);
    p->goal_node = fer_container_of(node, node_t, gng);
}

static void dijDumpPath(plan_t *p)
{
    fer_dij_node_t *n;
    node_t *node;
    fer_list_t list, *item;
    size_t i, size;

    fprintf(stdout, "----\n");
    fprintf(stdout, "Name: Path\n");
    fprintf(stdout, "Edge color: 0.7 0 0\n");
    fprintf(stdout, "Edge width: 3\n");

    ferListInit(&list);
    ferDijPath(&p->goal_node->dij, &list);

    fprintf(stdout, "Points:\n");
    size = 0;
    FER_LIST_FOR_EACH(&list, item){
        n = ferDijNodeFromList(item);
        node = fer_container_of(n, node_t, dij);
        fprintf(stdout, "%g %g\n", ferVec2X(node->w), ferVec2Y(node->w));
        size++;
    }
    fprintf(stdout, "Edges:\n");
    for (i = 0; i < size - 1; i++){
        fprintf(stdout, "%d %d\n", i, i + 1);
    }

    fprintf(stdout, "----\n");
}
