#include <cu/cu.h>
#include <fermat/dij.h>
#include <fermat/vec2.h>

#define NUM_NODES 20


static fer_real_t dist(const fer_dij_node_t *n1,
                       const fer_dij_node_t *n2);
static void expand(fer_dij_node_t *n, fer_list_t *list);

struct _node_t {
    fer_vec2_t v;
    fer_dij_node_t dij;

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
        fprintf(out, "%g %g\n", ferVec2X(&nodes[i].v), ferVec2Y(&nodes[i].v));
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
    fer_dij_node_t *n;
    node_t *node;
    fer_list_t list, *item;
    size_t i, size;

    fprintf(out, "----\n");
    fprintf(out, "Name: Path\n");
    fprintf(out, "Edge color: 0.7 0 0\n");

    ferListInit(&list);
    ferDijPath(&endnode->dij, &list);

    fprintf(out, "Points:\n");
    size = 0;
    ferListForEach(&list, item){
        n = ferDijNodeFromList(item);
        node = fer_container_of(n, node_t, dij);
        fprintf(out, "%g %g\n", ferVec2X(&node->v), ferVec2Y(&node->v));
        size++;
    }
    fprintf(out, "Edges:\n");
    for (i = 0; i < size - 1; i++){
        fprintf(out, "%d %d\n", i, i + 1);
    }

    fprintf(out, "----\n");
}

static void initNodes(node_t *nodes)
{
    size_t i;

    for (i = 0; i < NUM_NODES; i++){
        ferDijNodeInit(&nodes[i].dij);
        nodes[i].nodes[0] = NULL;
        nodes[i].nodes[1] = NULL;
        nodes[i].nodes[2] = NULL;
        nodes[i].nodes_len = 0;
        nodes[i].id = i;
        ferVec2Set(&nodes[i].v, -1, -1);

        //fprintf(stderr, "[%d] %lx %lx\n", i, (long)&nodes[i], (long)&nodes[i].dij);
    }
}

static void initNodes1(node_t *nodes)
{
    size_t i, j;

    initNodes(nodes);

    // start and end node
    ferVec2Set(&nodes[0].v, 0., 0.);
    ferVec2Set(&nodes[1].v, 4., 4.);

    for (i = 0; i < 3; i++){
        for (j = 0; j < 3; j++){
            ferVec2Set(&nodes[2 + i * 3 + j].v, j + 1., i + 1.);
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
    fer_dij_t *dij;
    fer_dij_ops_t ops;
    node_t nodes[NUM_NODES];
    int res;

    initNodes1(nodes);

    ferDijOpsInit(&ops);
    ops.dist = dist;
    ops.expand = expand;

    dij = ferDijNew(&ops);

    dumpNodes(nodes, stdout);
    res = ferDijRun(dij, &nodes[0].dij, &nodes[1].dij);
    dumpPath(&nodes[1], stdout);

    assertEquals(res, 0);

    ferDijDel(dij);
}

static fer_real_t dist(const fer_dij_node_t *_n1,
                       const fer_dij_node_t *_n2)
{
    node_t *n1, *n2;
    n1 = fer_container_of(_n1, node_t, dij);
    n2 = fer_container_of(_n2, node_t, dij);

    return ferVec2Dist(&n1->v, &n2->v);
}

static void expand(fer_dij_node_t *_n, fer_list_t *list)
{
    size_t i;
    node_t *n;

    n = fer_container_of(_n, node_t, dij);

    for (i = 0; i < n->nodes_len; i++){
        if (!ferDijNodeClosed(&n->nodes[i]->dij))
            ferDijNodeAdd(&n->nodes[i]->dij, list);
    }
}
