#include <cu/cu.h>
#include <boruvka/dij.h>
#include <boruvka/vec2.h>

#define NUM_NODES 20


static void expand(bor_dij_node_t *n, bor_list_t *list, void *);

struct _node_t {
    bor_vec2_t v;
    bor_dij_node_t dij;

    struct _node_t *nodes[3];
    size_t nodes_len;

    int id;
};
typedef struct _node_t node_t;

static void dumpNodes(node_t *nodes, FILE *out)
{
    size_t i, j;

    fprintf(out, "----\nPoints:\n");
    for (i = 0; i < NUM_NODES; i++){
        fprintf(out, "%g %g\n", borVec2X(&nodes[i].v), borVec2Y(&nodes[i].v));
    }

    fprintf(out, "Edges:\n");
    for (i = 0; i < NUM_NODES; i++){
        for (j = 0; j < nodes[i].nodes_len; j++){
            fprintf(out, "%d %d\n", nodes[i].id, nodes[i].nodes[j]->id);
        }
    }
    fprintf(out, "----\n");
}

static void dumpPath(node_t *endnode, FILE *out)
{
    bor_dij_node_t *n;
    node_t *node;
    bor_list_t list, *item;
    size_t i, size;

    fprintf(out, "----\n");
    fprintf(out, "Name: Path\n");
    fprintf(out, "Edge color: 0.7 0 0\n");

    borListInit(&list);
    borDijPath(&endnode->dij, &list);

    fprintf(out, "Points:\n");
    size = 0;
    BOR_LIST_FOR_EACH(&list, item){
        n = borDijNodeFromList(item);
        node = bor_container_of(n, node_t, dij);
        fprintf(out, "%g %g\n", borVec2X(&node->v), borVec2Y(&node->v));
        size++;
    }
    fprintf(out, "Edges:\n");
    for (i = 0; i < size - 1; i++){
        fprintf(out, "%d %d\n", (int)i, (int)i + 1);
    }

    fprintf(out, "----\n");
}

static void initNodes(node_t *nodes)
{
    size_t i;

    for (i = 0; i < NUM_NODES; i++){
        borDijNodeInit(&nodes[i].dij);
        nodes[i].nodes[0] = NULL;
        nodes[i].nodes[1] = NULL;
        nodes[i].nodes[2] = NULL;
        nodes[i].nodes_len = 0;
        nodes[i].id = i;
        borVec2Set(&nodes[i].v, -1, -1);

        //fprintf(stderr, "[%d] %lx %lx\n", i, (long)&nodes[i], (long)&nodes[i].dij);
    }
}

static void initNodes1(node_t *nodes)
{
    size_t i, j;

    initNodes(nodes);

    // start and end node
    borVec2Set(&nodes[0].v, 0., 0.);
    borVec2Set(&nodes[1].v, 4., 4.);

    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            borVec2Set(&nodes[2 + i * 3 + j].v, j + 1., i + 1.);
        }
    }

    nodes[0].nodes[0] = &nodes[2];
    nodes[0].nodes[1] = &nodes[3];
    nodes[0].nodes[2] = &nodes[5];
    nodes[0].nodes_len = 3;

    nodes[2].nodes[0] = &nodes[3];
    nodes[2].nodes_len = 1;
    nodes[3].nodes[0] = &nodes[6];
    nodes[3].nodes_len = 1;
    nodes[4].nodes[0] = &nodes[3];
    nodes[4].nodes_len = 1;
    nodes[6].nodes[0] = &nodes[8];
    nodes[6].nodes[1] = &nodes[5];
    nodes[6].nodes[2] = &nodes[4];
    nodes[6].nodes_len = 3;
    nodes[7].nodes[0] = &nodes[4];
    nodes[7].nodes[1] = &nodes[10];
    nodes[7].nodes_len = 2;
    nodes[8].nodes[0] = &nodes[9];
    nodes[8].nodes[1] = &nodes[5];
    nodes[8].nodes_len = 2;
    nodes[9].nodes[0] = &nodes[7];
    nodes[9].nodes_len = 1;
    nodes[10].nodes[0] = &nodes[1];
    nodes[10].nodes_len = 1;
}

TEST(dij1)
{
    bor_dij_t *dij;
    bor_dij_ops_t ops;
    node_t nodes[NUM_NODES];
    int res;

    initNodes1(nodes);

    borDijOpsInit(&ops);
    ops.expand = expand;

    dij = borDijNew(&ops);

    dumpNodes(nodes, stdout);
    res = borDijRun(dij, &nodes[0].dij, &nodes[1].dij);
    dumpPath(&nodes[1], stdout);

    assertEquals(res, 0);

    borDijDel(dij);
}

static void expand(bor_dij_node_t *_n, bor_list_t *list, void *_)
{
    size_t i;
    node_t *n;
    bor_real_t dist;

    n = bor_container_of(_n, node_t, dij);

    for (i = 0; i < n->nodes_len; i++){
        if (!borDijNodeClosed(&n->nodes[i]->dij)){
            dist = borVec2Dist(&n->v, &n->nodes[i]->v);
            borDijNodeAdd(&n->nodes[i]->dij, list, dist);
        }
    }
}
