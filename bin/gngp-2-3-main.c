#include <stdio.h>
#include <fermat/dbg.h>
#include <fermat/timer.h>
#include <fermat/rand-mt.h>
#include <fermat/gng-plan2.h>
#include <fermat/dij.h>
#include <fermat/alloc.h>
#include <fermat/vec2.h>
#include <fermat/vec3.h>
#include <fermat/mat3.h>
#include <fermat/alloc.h>

#define FREE FER_GNG_PLAN_FREE
#define OBST FER_GNG_PLAN_OBST

struct _plan_2_3_tri_t {
    fer_vec2_t p[3];
    fer_list_t list;
};
typedef struct _plan_2_3_tri_t plan_2_3_tri_t;

struct _plan_2_3_obj_t {
    fer_list_t tris;
    fer_list_t list;
};
typedef struct _plan_2_3_obj_t plan_2_3_obj_t;

struct _plan_2_3_t {
    fer_real_t aabb[6];
    fer_vec3_t start, goal;

    fer_real_t max_dist;
    fer_real_t min_dist;

    plan_2_3_obj_t robot;
    fer_list_t obsts;
};
typedef struct _plan_2_3_t plan_2_3_t;

plan_2_3_t *plan;

plan_2_3_t *planNew(const char *fn);
void planDel(plan_2_3_t *plan);

fer_gng_plan_t *gng;
fer_gng_plan_ops_t ops;
fer_gng_plan_params_t params;
size_t max_nodes = 100000000;
fer_rand_mt_t *rand_mt;
fer_timer_t timer;
fer_vec_t *is;

int dump_period = 10;
const char *dump_prefix = NULL;

const char *scene;

static int terminate(void *data);
static void callback(void *data);
static const void *inputSignal(void *data);
static int eval(const fer_vec_t *w, void *data);

static void dump(void);
static void dumpScene(FILE *out);
static void dumpObj(plan_2_3_obj_t *obj, const fer_vec_t *w, FILE *out);
static void dumpRobotPath(FILE *out);


int main(int argc, char *argv[])
{
    if (argc < 2){
        fprintf(stderr, "Usage: %s scene [dump_prefix dump_period]\n", argv[0]);
        return -1;
    }
    if (argc >= 3)
        dump_prefix = argv[2];
    if (argc >= 4)
        dump_period = atoi(argv[3]);

    is = ferVecNew(3);

    plan = planNew(argv[1]);

    ferGNGPlanOpsInit(&ops);
    ferGNGPlanParamsInit(&params);

    ops.terminate    = terminate;
    ops.input_signal = inputSignal;
    ops.eval         = eval;
    ops.callback     = callback;
    ops.callback_period = 100;

    params.dim = 3;
    params.max_dist = plan->max_dist;
    params.min_dist = plan->min_dist;
    params.min_nodes = 100;
    params.start = (const fer_vec_t *)&plan->start;
    params.goal  = (const fer_vec_t *)&plan->goal;
    params.cells.d = 3;
    params.cells.aabb = plan->aabb;
    params.cells.max_dens = 1;
    params.cells.expand_rate = 1.4;
    params.gng.lambda = 1000;
    params.gng.age_max = 20;


    rand_mt = ferRandMTNewAuto();
    gng = ferGNGPlanNew(&ops, &params);

    ferTimerStart(&timer);
    ferGNGPlanRun(gng);
    ferTimerStopAndPrintElapsed(&timer, stderr, "\r\n");

    if (dump_prefix){
        dump();
    }
    ferGNGPlanDumpNetSVT(gng, stdout, NULL);
    ferGNGPlanDumpObstSVT(gng, stdout, NULL);
    ferGNGPlanDumpPathSVT(gng, stdout, NULL);
    dumpScene(stdout);
    dumpRobotPath(stdout);


    ferGNGPlanDel(gng);

    ferRandMTDel(rand_mt);
    
    ferVecDel(is);

    planDel(plan);

    return 0;
}


static int terminate(void *data)
{
    size_t nodes_len;
    static int counter = 0;

    if (dump_prefix && ++counter == dump_period){
        dump();
        counter = 0;
    }

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    return nodes_len >= max_nodes;
}

static void callback(void *data)
{
    size_t nodes_len;

    nodes_len = ferGNGNodesLen(ferGNGPlanGNG(gng));
    ferTimerStopAndPrintElapsed(&timer, stderr, " nodes: %08d, evals: %010lu\r",
                                (int)nodes_len, ferGNGPlanEvals(gng));
}

static const void *inputSignal(void *data)
{
    fer_real_t x, y, z;

    x = ferRandMT(rand_mt, plan->aabb[0], plan->aabb[1]);
    y = ferRandMT(rand_mt, plan->aabb[2], plan->aabb[3]);
    z = ferRandMT(rand_mt, plan->aabb[4], plan->aabb[5]);
    ferVec3Set((fer_vec3_t *)is, x, y, z);
    return is;
}

static int eval(const fer_vec_t *w, void *data)
{
    fer_mat3_t tr;
    fer_vec2_t p[3];
    fer_list_t *item_obst, *item_tri, *item_obst_tri;
    plan_2_3_obj_t *obst;
    plan_2_3_tri_t *tri;
    int ev;

    ferMat3SetRot(&tr, ferVecGet(w, 2));
    ferMat3Set1(&tr, 0, 2, ferVecGet(w, 0));
    ferMat3Set1(&tr, 1, 2, ferVecGet(w, 1));

    FER_LIST_FOR_EACH(&plan->robot.tris, item_tri){
        tri = FER_LIST_ENTRY(item_tri, plan_2_3_tri_t, list);
        ferMat3MulVec2(&p[0], &tr, &tri->p[0]);
        ferMat3MulVec2(&p[1], &tr, &tri->p[1]);
        ferMat3MulVec2(&p[2], &tr, &tri->p[2]);

        FER_LIST_FOR_EACH(&plan->obsts, item_obst){
            obst = FER_LIST_ENTRY(item_obst, plan_2_3_obj_t, list);
            FER_LIST_FOR_EACH(&obst->tris, item_obst_tri){
                tri = FER_LIST_ENTRY(item_obst_tri, plan_2_3_tri_t, list);

                ev = ferVec2TriTriOverlap(&p[0], &p[1], &p[2],
                                          &tri->p[0], &tri->p[1], &tri->p[2]);
                if (ev)
                    return OBST;
            }
        }
    }

    return FREE;
}



static void dump(void)
{
    FILE *fout;
    char fn[1000];
    static int counter = 0;

    sprintf(fn, "%s%010d.svt", dump_prefix, counter++);
    fout = fopen(fn, "w");
    if (fout){
        ferGNGPlanDumpNetSVT(gng, fout, NULL);
        ferGNGPlanDumpObstSVT(gng, fout, NULL);
        ferGNGPlanDumpPathSVT(gng, fout, NULL);
        dumpScene(fout);
        fclose(fout);
    }
}

static void dumpScene(FILE *out)
{
    fer_list_t *item_obst, *item_tri;
    plan_2_3_obj_t *obst;
    plan_2_3_tri_t *tri;
    int i, size;

    fprintf(out, "----\n");
    fprintf(out, "Face color: 0.8 0.1 0.8\n");
    fprintf(out, "Point color: 0.8 0.1 0.8\n");
    fprintf(out, "Name: Scene\n");
    fprintf(out, "Points:\n");

    size = 0;
    FER_LIST_FOR_EACH(&plan->obsts, item_obst){
        obst = FER_LIST_ENTRY(item_obst, plan_2_3_obj_t, list);

        FER_LIST_FOR_EACH(&obst->tris, item_tri){
            tri = FER_LIST_ENTRY(item_tri, plan_2_3_tri_t, list);

            ferVec2Print(&tri->p[0], out);
            fprintf(out, " 0 \n");
            ferVec2Print(&tri->p[1], out);
            fprintf(out, " 0 \n");
            ferVec2Print(&tri->p[2], out);
            fprintf(out, " 0 \n");

            ++size;
        }
    }

    fprintf(out, "Faces:\n");
    for (i = 0; i < size; i++){
        fprintf(out, "%d %d %d\n", 3 * i, 3 * i + 1, 3 * i + 2);
    }

    fprintf(out, "----\n");

}

static void dumpObj(plan_2_3_obj_t *obj, const fer_vec_t *w, FILE *out)
{
    fer_mat3_t tr;
    fer_vec2_t p[3];
    fer_list_t *item;
    plan_2_3_tri_t *tri;
    int i, size;

    ferMat3SetRot(&tr, ferVecGet(w, 2));
    ferMat3Set1(&tr, 0, 2, ferVecGet(w, 0));
    ferMat3Set1(&tr, 1, 2, ferVecGet(w, 1));

    fprintf(out, "----\n");
    fprintf(out, "Edge color: 0.1 0.8 0.1\n");
    fprintf(out, "Point color: 0.1 0.8 0.1\n");
    fprintf(out, "Edge width: 1\n");
    fprintf(out, "Point size: 1\n");
    fprintf(out, "Name: Robot 1\n");
    fprintf(out, "Points:\n");

    size = 0;
    FER_LIST_FOR_EACH(&plan->robot.tris, item){
        tri = FER_LIST_ENTRY(item, plan_2_3_tri_t, list);
        ferMat3MulVec2(&p[0], &tr, &tri->p[0]);
        ferMat3MulVec2(&p[1], &tr, &tri->p[1]);
        ferMat3MulVec2(&p[2], &tr, &tri->p[2]);

        ferVec2Print(&p[0], out);
        fprintf(out, " 0 \n");
        ferVec2Print(&p[1], out);
        fprintf(out, " 0 \n");
        ferVec2Print(&p[2], out);
        fprintf(out, " 0 \n");
        size++;
    }

    fprintf(out, "Edges:\n");
    for (i = 0; i < size; i++){
        fprintf(out, "%d %d\n%d %d\n%d %d\n",
                     3 * i    , 3 * i + 1,
                     3 * i + 1, 3 * i + 2,
                     3 * i + 2, 3 * i);
    }
    fprintf(out, "----\n");
}

static void dumpRobotPath(FILE *out)
{
    fer_gng_plan_node_t *n;
    fer_list_t *item;

    if (ferListEmpty(&gng->path))
        return;

    fprintf(out, "# PATH: ");
    ferVec3Print(&plan->start, out);
    fprintf(out, "\n");
    FER_LIST_FOR_EACH(&gng->path, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);
        fprintf(out, "# PATH: %f %f %f\n",
                ferVecGet(n->w, 0), ferVecGet(n->w, 1), ferVecGet(n->w, 2));
    }
    fprintf(out, "# PATH: ");
    ferVec3Print(&plan->goal, out);
    fprintf(out, "\n");

    dumpObj(&plan->robot, (const fer_vec_t *)&plan->start, out);
    FER_LIST_FOR_EACH(&gng->path, item){
        n = FER_LIST_ENTRY(item, fer_gng_plan_node_t, path);

        dumpObj(&plan->robot, n->w, out);
    }
    dumpObj(&plan->robot, (const fer_vec_t *)&plan->goal, out);
}



static char *planNextLine(FILE *fin, char *line, size_t len)
{
    char *ret;

    do {
        ret = fgets(line, len, fin);
    } while (ret != NULL && line[0] == '#');

    return ret;
}

static void planNextLineEmpty(FILE *fin, char *line, size_t len, const char *sec)
{
    char *l = planNextLine(fin, line, len);
    if (l[0] != '\n'){
        fprintf(stderr, "Invalid file format (%s).\n", sec);
        exit(-1);
    }
}

void planDump(const plan_2_3_t *plan)
{
    fer_list_t *item_obst, *item_tri, *item_obst_tri;
    plan_2_3_obj_t *obst;
    plan_2_3_tri_t *tri;
    int i;

    fprintf(stderr, "AABB:");
    for (i = 0; i < 6; i++){
        fprintf(stderr, " %f", (float)plan->aabb[i]);
    }
    fprintf(stderr, "\n\n");

    fprintf(stderr, "start: %f %f %f\n", (float)ferVec3X(&plan->start),
            (float)ferVec3Y(&plan->start), (float)ferVec3Z(&plan->start));
    fprintf(stderr, "goal:  %f %f %f\n", (float)ferVec3X(&plan->goal),
            (float)ferVec3Y(&plan->goal), (float)ferVec3Z(&plan->goal));
    fprintf(stderr, "\n");

    fprintf(stderr, "max dist: %f\n", plan->max_dist);
    fprintf(stderr, "min dist: %f\n", plan->min_dist);
    fprintf(stderr, "\n");

    FER_LIST_FOR_EACH(&plan->robot.tris, item_tri){
        tri = FER_LIST_ENTRY(item_tri, plan_2_3_tri_t, list);
        fprintf(stderr, "robot: <%f %f, %f %f, %f %f>\n",
                ferVec2X(&tri->p[0]), ferVec2Y(&tri->p[0]),
                ferVec2X(&tri->p[1]), ferVec2Y(&tri->p[1]),
                ferVec2X(&tri->p[2]), ferVec2Y(&tri->p[2]));
    }
    fprintf(stderr, "\n");

    i = 0;
    FER_LIST_FOR_EACH(&plan->obsts, item_obst){
        obst = FER_LIST_ENTRY(item_obst, plan_2_3_obj_t, list);

        FER_LIST_FOR_EACH(&obst->tris, item_obst_tri){
            tri = FER_LIST_ENTRY(item_obst_tri, plan_2_3_tri_t, list);

            fprintf(stderr, "obst[%02d]: <%f %f, %f %f, %f %f>\n",
                    i, ferVec2X(&tri->p[0]), ferVec2Y(&tri->p[0]),
                       ferVec2X(&tri->p[1]), ferVec2Y(&tri->p[1]),
                       ferVec2X(&tri->p[2]), ferVec2Y(&tri->p[2]));
        }

        i++;
    }
}

plan_2_3_t *planNew(const char *fn)
{
    plan_2_3_t *plan;
    plan_2_3_obj_t *obj;
    plan_2_3_tri_t *tri;
    FILE *fin;
    size_t __len = 1024;
    char __line[1024];
    char *line;
    float f[6];
    int i;

    if ((fin = fopen(fn, "r")) == NULL){
        perror("Can't open file");
        return NULL;
    }

    plan = FER_ALLOC(plan_2_3_t);

    // AABB
    line = planNextLine(fin, __line, __len);
    if (!line || sscanf(line, "%f %f %f %f %f %f", f, f + 1, f + 2, f + 3, f + 4, f + 5) != 6){
        fprintf(stderr, "Invalid file format (AABB).\n");
        exit(-1);
    }
    for (i = 0; i < 6; i++){
        plan->aabb[i] = f[i];
    }
    planNextLineEmpty(fin, __line, __len, "AABB");

    // Start position
    line = planNextLine(fin, __line, __len);
    if (!line || sscanf(line, "%f %f %f", f, f + 1, f + 2) != 3){
        fprintf(stderr, "Invalid file format (start position).\n");
        exit(-1);
    }
    ferVec3Set(&plan->start, f[0], f[1], f[2]);
    planNextLineEmpty(fin, __line, __len, "start position");

    // Goal position
    line = planNextLine(fin, __line, __len);
    if (!line || sscanf(line, "%f %f %f", f, f + 1, f + 2) != 3){
        fprintf(stderr, "Invalid file format (goal position).\n");
        exit(-1);
    }
    ferVec3Set(&plan->goal, f[0], f[1], f[2]);
    planNextLineEmpty(fin, __line, __len, "goal position");

    // params
    line = planNextLine(fin, __line, __len);
    if (!line || sscanf(line, "%f %f", f, f + 1) != 2){
        fprintf(stderr, "Invalid file format (params).\n");
        exit(-1);
    }
    plan->max_dist = f[0];
    plan->min_dist = f[1];
    planNextLineEmpty(fin, __line, __len, "params");

    // robot
    ferListInit(&plan->robot.tris);
    line = planNextLine(fin, __line, __len);
    while (line && line[0] != '\n'){
        if (sscanf(line, "%f %f %f %f %f %f", f, f + 1, f + 2, f + 3, f + 4, f + 5) != 6){
            fprintf(stderr, "Invalid file format (robot).\n");
            exit(-1);
        }

        tri = FER_ALLOC(plan_2_3_tri_t);
        ferVec2Set(&tri->p[0], f[0], f[1]);
        ferVec2Set(&tri->p[1], f[2], f[3]);
        ferVec2Set(&tri->p[2], f[4], f[5]);
        ferListAppend(&plan->robot.tris, &tri->list);

        line = planNextLine(fin, __line, __len);
    }

    // obstacles
    ferListInit(&plan->obsts);
    line = planNextLine(fin, __line, __len);
    while (line){
        obj = FER_ALLOC(plan_2_3_obj_t);
        ferListInit(&obj->tris);
        ferListAppend(&plan->obsts, &obj->list);

        while (line && line[0] != '\n'){
            if (sscanf(line, "%f %f %f %f %f %f", f, f + 1, f + 2, f + 3, f + 4, f + 5) != 6){
                fprintf(stderr, "Invalid file format (robot).\n");
                exit(-1);
            }

            tri = FER_ALLOC(plan_2_3_tri_t);
            ferVec2Set(&tri->p[0], f[0], f[1]);
            ferVec2Set(&tri->p[1], f[2], f[3]);
            ferVec2Set(&tri->p[2], f[4], f[5]);
            ferListAppend(&obj->tris, &tri->list);

            line = planNextLine(fin, __line, __len);
        }

        if (line)
            line = planNextLine(fin, __line, __len);
    }

    planDump(plan);
    return plan;
}

void planDel(plan_2_3_t *plan)
{
    fer_list_t *item;
    plan_2_3_obj_t *obj;
    plan_2_3_tri_t *tri;

    while (!ferListEmpty(&plan->robot.tris)){
        item = ferListNext(&plan->robot.tris);
        tri  = FER_LIST_ENTRY(item, plan_2_3_tri_t, list);
        ferListDel(item);
        free(tri);
    }

    while (!ferListEmpty(&plan->obsts)){
        item = ferListNext(&plan->obsts);
        obj  = FER_LIST_ENTRY(item, plan_2_3_obj_t, list);
        ferListDel(item);

        while (!ferListEmpty(&obj->tris)){
            item = ferListNext(&obj->tris);
            tri  = FER_LIST_ENTRY(item, plan_2_3_tri_t, list);
            ferListDel(item);
            free(tri);
        }

        free(obj);
    }

    free(plan);
}
