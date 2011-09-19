#include <stdio.h>
#include <signal.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/gng-t.h>
#include <fermat/pc.h>
#include <fermat/nncells.h>
#include <fermat/alloc.h>
#include <fermat/vec3.h>

struct _node_t {
    fer_gngt_node_t node;

    fer_vec_t *w;
    fer_nncells_el_t cells;

    int _id;
};
typedef struct _node_t node_t;

int dim;
fer_real_t target;
FER_VEC(tmpv, 10);
int dump = 0, dump_num = 0;

fer_gngt_params_t params;
fer_gngt_ops_t ops;
fer_gngt_t *gng;

fer_nncells_params_t cells_params;
fer_nncells_t *cells;

fer_timer_t timer;

fer_pc_t *pc;
fer_pc_it_t pcit;

static int terminate(void *data);
static void callback(void *data);
static const void *input_signal(void *data);
static fer_gngt_node_t *new_node(const void *is, void *);
static fer_gngt_node_t *new_node_between(const fer_gngt_node_t *n1,
                                         const fer_gngt_node_t *n2, void *);
static void del_node(fer_gngt_node_t *n, void *);
static void nearest(const void *input_signal,
                    fer_gngt_node_t **n1,
                    fer_gngt_node_t **n2, void *);
static fer_real_t dist2(const void *input_signal,
                        const fer_gngt_node_t *node, void *);
static void move_towards(fer_gngt_node_t *node,
                         const void *input_signal,
                         fer_real_t fraction, void *);
static void dumpSVT(fer_gngt_t *gng, FILE *out, const char *name);

static void sigDump(int sig);

int main(int argc, char *argv[])
{
    size_t size;
    fer_real_t aabb[30];

    if (argc < 4){
        fprintf(stderr, "Usage: %s dim file.pts target\n", argv[0]);
        return -1;
    }

    dim = atoi(argv[1]);
    target = atof(argv[3]);

    // read input points
    pc = ferPCNew(dim);
    size = ferPCAddFromFile(pc, argv[2]);
    fprintf(stderr, "Added %d points from %s\n", (int)size, argv[2]);
    ferPCPermutate(pc);
    ferPCItInit(&pcit, pc);


    // create NN search structure
    ferNNCellsParamsInit(&cells_params);
    cells_params.dim         = atoi(argv[1]);
    cells_params.num_cells   = 0;
    cells_params.max_dens    = 0.1;
    cells_params.expand_rate = 1.5;
    ferPCAABB(pc, aabb);
    cells_params.aabb = aabb;
    cells = ferNNCellsNew(&cells_params);

    // create GNG-T
    ferGNGTParamsInit(&params);
    params.target = target;
    //params.age_max = 1000;
    //params.lambda = 10000;

    ferGNGTOpsInit(&ops);
    ops.new_node         = new_node;
    ops.new_node_between = new_node_between;
    ops.del_node         = del_node;
    ops.input_signal     = input_signal;
    ops.nearest          = nearest;
    ops.dist2            = dist2;
    ops.move_towards     = move_towards;
    ops.terminate        = terminate;
    ops.callback         = callback;
    ops.callback_period = 300;
    ops.data = NULL;

    gng = ferGNGTNew(&ops, &params);

    signal(SIGINT, sigDump);

    ferTimerStart(&timer);
    ferGNGTRun(gng);
    callback(NULL);
    fprintf(stderr, "\n");

    dumpSVT(gng, stdout, NULL);

    ferGNGTDel(gng);
    ferNNCellsDel(cells);
    ferPCDel(pc);

    return 0;
}


static int terminate(void *data)
{
    FILE *fout;
    static char fn[1000];

    if (dump){
        dump = 0;

        sprintf(fn, "gng-t-%06d.svt", dump_num++);
        fout = fopen(fn, "w");
        dumpSVT(gng, fout, NULL);
        fclose(fout);
    }

    return 0;
    return ferGNGTNodesLen(gng) >= 1000;
}

static void callback(void *data)
{
    size_t nodes_len;

    nodes_len = ferGNGTNodesLen(gng);

    ferTimerStopAndPrintElapsed(&timer, stderr, " n: %d, avg err: %f, target: %f\r",
            nodes_len, ferGNGTAvgErr(gng), target);
}

static const void *input_signal(void *data)
{
    const fer_vec_t *v;

    if (ferPCItEnd(&pcit)){
        ferPCPermutate(pc);
        ferPCItInit(&pcit, pc);
    }
    v = ferPCItGet(&pcit);
    ferPCItNext(&pcit);
    return (const void *)v;
}

static fer_gngt_node_t *new_node(const void *is, void *_)
{
    node_t *n;

    n = FER_ALLOC(node_t);
    n->w = ferVecClone(dim, (const fer_vec_t *)is);
    ferNNCellsElInit(&n->cells, n->w);
    ferNNCellsAdd(cells, &n->cells);

    return &n->node;
}

static fer_gngt_node_t *new_node_between(const fer_gngt_node_t *_n1,
                                         const fer_gngt_node_t *_n2, void *_)
{
    node_t *n1 = fer_container_of(_n1, node_t, node);
    node_t *n2 = fer_container_of(_n2, node_t, node);

    ferVecAdd2(dim, tmpv, n1->w, n2->w);
    ferVecScale(dim, tmpv, FER_REAL(0.5));

    return new_node((const void *)tmpv, NULL);
}

static void del_node(fer_gngt_node_t *_n, void *_)
{
    node_t *n = fer_container_of(_n, node_t, node);

    ferNNCellsRemove(cells, &n->cells);
    ferVecDel(n->w);
    free(n);
}

static void nearest(const void *is,
                    fer_gngt_node_t **n1,
                    fer_gngt_node_t **n2, void *_)
{
    fer_nncells_el_t *els[2];
    node_t *ns[2];

    ferNNCellsNearest(cells, (const fer_vec_t *)is, 2, els);
    ns[0] = fer_container_of(els[0], node_t, cells);
    ns[1] = fer_container_of(els[1], node_t, cells);
    *n1 = &ns[0]->node;
    *n2 = &ns[1]->node;
}

static fer_real_t dist2(const void *is,
                        const fer_gngt_node_t *node, void *_)
{
    node_t *n = fer_container_of(node, node_t, node);

    return ferVecDist2(dim, (const fer_vec_t *)is, n->w);
}

static void move_towards(fer_gngt_node_t *node,
                         const void *is,
                         fer_real_t fraction, void *_)
{
    node_t *n;

    n = fer_container_of(node, node_t, node);

    ferVecSub2(dim, tmpv, (const fer_vec_t *)is, n->w);
    ferVecScale(dim, tmpv, fraction);
    ferVecAdd(dim, n->w, tmpv);

    ferNNCellsUpdate(cells, &n->cells);
}

static void dumpSVT(fer_gngt_t *gng, FILE *out, const char *name)
{
    fer_list_t *list, *item;
    fer_net_node_t *nn;
    fer_gngt_node_t *gn;
    node_t *n;
    fer_net_edge_t *e;
    size_t i, id1, id2;

    if (dim != 2 && dim != 3)
        return;

    fprintf(out, "--------\n");

    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Points:\n");
    list = ferGNGTNodes(gng);
    i = 0;
    FER_LIST_FOR_EACH(list, item){
        gn = ferGNGTNodeFromList(item);
        n  = fer_container_of(gn, node_t, node);

        n->_id = i++;
        if (dim == 2){
            ferVec2Print((const fer_vec2_t *)n->w, out);
        }else{
            ferVec3Print((const fer_vec3_t *)n->w, out);
        }
        fprintf(out, "\n");
    }


    fprintf(out, "Edges:\n");
    list = ferGNGTEdges(gng);
    FER_LIST_FOR_EACH(list, item){
        e = FER_LIST_ENTRY(item, fer_net_edge_t, list);

        nn = ferNetEdgeNode(e, 0);
        gn = ferGNGTNodeFromNet(nn);
        n  = fer_container_of(gn, node_t, node);
        id1 = n->_id;

        nn = ferNetEdgeNode(e, 1);
        gn = ferGNGTNodeFromNet(nn);
        n  = fer_container_of(gn, node_t, node);
        id2 = n->_id;
        fprintf(out, "%d %d\n", (int)id1, (int)id2);
    }

    fprintf(out, "--------\n");
}

static void sigDump(int sig)
{
    dump = 1;
}

