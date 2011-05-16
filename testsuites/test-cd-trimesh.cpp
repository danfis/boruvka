#include <fermat/cd.h>
#include <fermat/timer.h>
#ifdef HAVE_RAPID
# include <RAPID.H>
#endif
#include "data.h"

const char *task_name[] = {
    "Dragon",
    "Box",
    "Bugtrap",
    "Puzzle",
    "Car-Seat Small"
};

const char *data1_geom[] = {
    "dragon.geom",
    NULL,
    "data-bugtrap-1.geom",
    "data-puzzle-1.geom",
    "data-car-small.geom"
    //"data-car-big.geom"
};

const char *data2_geom[] = {
    "dragon.geom",
    NULL,
    "data-bugtrap-2.geom",
    "data-puzzle-2.geom",
    "data-seat-small.geom"
    //"data-seat-big.geom"
};

const char *data1_tri[] = {
    "dragon.tri",
    "data-box1.tri",
    "data-bugtrap-1.tri",
    "data-puzzle-1.tri",
    "data-car-small.tri"
    //"car.raw"
};

const char *data2_tri[] = {
    "dragon.tri",
    "data-box2.tri",
    "data-bugtrap-2.tri",
    "data-puzzle-2.tri",
    "data-seat-small.tri"
    //"sedadlo.raw"
};

const char *data_trans[] = {
    "data-test-cd-trimesh-dragon.trans.txt",
    "data-test-cd-trimesh-box.trans.txt",
    "data-test-cd-trimesh-bugtrap.trans.txt",
    "data-test-cd-trimesh-puzzle.trans.txt",
    "data-test-cd-trimesh-car-small.trans.txt"
    //"stavy.cooper"
};

const int check_ret[] = {
    1,
    1,
    0,
    0,
    0
};

int loading_enabled = 1;
int progress = 1;
int run = 1;


void testCDBunny(void);
void testCD(int task);

void testRapidBunny(void);
void testRapid(int task);


#ifdef HAVE_RAPID
int rapidCollide(RAPID_model *g1, RAPID_model *g2,
                 const fer_mat3_t *rot, const fer_vec3_t *tr);
static RAPID_model *rapidFromRaw(const char *fn);
#endif /* HAVE_RAPID */
static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret,
                     const char *filename);

int main(int argc, char *argv[])
{
    if (argc == 2){
        if (strcmp(argv[1], "1") == 0 || strcmp(argv[1], "bunny") == 0){
            testCDBunny();
            testRapidBunny();
        }else if (strcmp(argv[1], "2") == 0 || strcmp(argv[1], "dragon") == 0){
            testCD(0);
            testRapid(0);
        }else if (strcmp(argv[1], "3") == 0 || strcmp(argv[1], "box") == 0){
            testCD(1);
            testRapid(1);
        }else if (strcmp(argv[1], "4") == 0 || strcmp(argv[1], "bugtrap") == 0){
            testCD(2);
            testRapid(2);
        }else if (strcmp(argv[1], "5") == 0 || strcmp(argv[1], "puzzle") == 0){
            testCD(3);
            testRapid(3);
        }else if (strcmp(argv[1], "6") == 0 || strcmp(argv[1], "car-small") == 0){
            testCD(4);
            testRapid(4);
        }
    }else{
        fprintf(stdout, "#\n");
        testCDBunny();
        testRapidBunny();

        fprintf(stdout, "#\n");
        testCD(0);
        testRapid(0);

        fprintf(stdout, "#\n");
        testCD(1);
        testRapid(1);

        fprintf(stdout, "#\n");
        testCD(2);
        testRapid(2);

        fprintf(stdout, "#\n");
        testCD(3);
        testRapid(3);

        fprintf(stdout, "#\n");
        testCD(4);
        testRapid(4);
    }

    return 0;
}

void testCDBunny(void)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;
    unsigned long overall_0 = 0L, overall_1 = 0L;
    unsigned int overall_0_num = 0, overall_1_num = 0;

    cd = ferCDNew();

    //ferCDSetBuildFlags(cd, FER_CD_FIT_CALIPERS |
    //        FER_CD_FIT_CALIPERS_NUM_ROT(5));
    //ferCDSetBuildFlags(cd, FER_CD_FIT_POLYHEDRAL_MASS);

    ferTimerStart(&timer);
    g1 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g1, bunny_coords, bunny_ids, bunny_tri_len);
    ferCDGeomBuild(cd, g1);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCDBunny :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    g2 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g2, bunny_coords, bunny_ids, bunny_tri_len);
    ferCDGeomBuild(cd, g2);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCDBunny :: build g2: %lu\n", ferTimerElapsedInUs(&timer));

    for (i = 0; run && nextTrans(&rot2, &tr2, &ret2, "data-test-cd-trimesh.trans.txt") == 0; i++){
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        ret = ferCDGeomCollide(cd, g1, g2);
        if (ret != ret2){
            fprintf(stdout, "# testCDBunny :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testCDBunny :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
        if (ret){
            overall_1 += ferTimerElapsedInUs(&timer);
            overall_1_num += 1;
        }else{
            overall_0 += ferTimerElapsedInUs(&timer);
            overall_0_num += 1;
        }
    }

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);

    fprintf(stdout, "# testCDBunny :: overall_time: %lu\n", overall_time);
    fprintf(stdout, "# testCDBunny :: overall_1: %lu / %u = %f\n",
            overall_1, overall_1_num, (float)overall_1/(float)overall_1_num);
    fprintf(stdout, "# testCDBunny :: overall_0: %lu / %u = %f\n",
            overall_0, overall_0_num, (float)overall_0/(float)overall_0_num);
}

void testRapidBunny(void)
{
#ifdef HAVE_RAPID
    RAPID_model *g1, *g2;
    double a[3], b[3], c[3];
    size_t i, j;
    fer_timer_t timer;
    unsigned long overall_time = 0L;
    unsigned long overall_0 = 0L, overall_1 = 0L;
    unsigned int overall_0_num = 0, overall_1_num = 0;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    int ret, ret2;

    ferTimerStart(&timer);
    g1 = new RAPID_model;
    g1->BeginModel();
    for (i = 0; i < bunny_tri_len; i++){
        for (j = 0; j < 3; j++){
            a[j] = ferVec3Get(&bunny_coords[bunny_ids[3 * i + 0]], j);
            b[j] = ferVec3Get(&bunny_coords[bunny_ids[3 * i + 1]], j);
            c[j] = ferVec3Get(&bunny_coords[bunny_ids[3 * i + 2]], j);
        }
        g1->AddTri(a, b, c, 0);
    }
    g1->EndModel();
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapidBunny :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    g2 = new RAPID_model;
    g2->BeginModel();
    for (i = 0; i < bunny_tri_len; i++){
        for (j = 0; j < 3; j++){
            a[j] = ferVec3Get(&bunny_coords[bunny_ids[3 * i + 0]], j);
            b[j] = ferVec3Get(&bunny_coords[bunny_ids[3 * i + 1]], j);
            c[j] = ferVec3Get(&bunny_coords[bunny_ids[3 * i + 2]], j);
        }
        g2->AddTri(a, b, c, 0);
    }
    g2->EndModel();
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapidBunny :: build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; run && nextTrans(&rot2, &tr2, &ret2, "data-test-cd-trimesh.trans.txt") == 0; i++){
        ferTimerStart(&timer);

        ret = rapidCollide(g1, g2, &rot2, &tr2);
        //ferCDGeomDumpSVT(g1, stdout, "g1");
        //ferCDGeomDumpSVT(g2, stdout, "g2");
        if (ret != ret2){
            fprintf(stdout, "# testRapidBunny :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testRapidBunny :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
        if (ret){
            overall_1 += ferTimerElapsedInUs(&timer);
            overall_1_num += 1;
        }else{
            overall_0 += ferTimerElapsedInUs(&timer);
            overall_0_num += 1;
        }
    }


    delete g1;
    delete g2;

    fprintf(stdout, "# testRapidBunny :: overall_time: %lu\n", overall_time);
    fprintf(stdout, "# testRapidBunny :: overall_1: %lu / %u = %f\n",
            overall_1, overall_1_num, (float)overall_1/(float)overall_1_num);
    fprintf(stdout, "# testRapidBunny :: overall_0: %lu / %u = %f\n",
            overall_0, overall_0_num, (float)overall_0/(float)overall_0_num);
#else /* HAVE_RAPID */
    fprintf(stdout, "# testRapidBunny :: No RAPID\n");
#endif /* HAVE_RAPID */
}




void testCD(int task)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;
    unsigned long overall_0 = 0L, overall_1 = 0L;
    unsigned int overall_0_num = 0, overall_1_num = 0;

    fprintf(stdout, "# Task[%02d]: %s\n", task, task_name[task]);

    cd = ferCDNew();

    //ferCDSetBuildFlags(cd, FER_CD_FIT_COVARIANCE);
    //ferCDSetBuildFlags(cd, FER_CD_FIT_CALIPERS |
    //        FER_CD_FIT_CALIPERS_NUM_ROT(5));
    //ferCDSetBuildFlags(cd, FER_CD_FIT_POLYHEDRAL_MASS);
    ferCDSetBuildFlags(cd, FER_CD_TOP_DOWN
                            | FER_CD_BUILD_PARALLEL(8)
                            | FER_CD_FIT_NAIVE
                            | FER_CD_FIT_NAIVE_NUM_ROT(5));
    //ferCDSetBuildFlags(cd, FER_CD_FIT_COVARIANCE_FAST);
    //ferCDSetBuildFlags(cd, FER_CD_BUILD_PARALLEL(8) | FER_CD_FIT_COVARIANCE_FAST);

    ferTimerStart(&timer);
    if (loading_enabled && data1_geom[task]){
        g1 = ferCDGeomLoad(cd, data1_geom[task]);
        ferTimerStop(&timer);
        fprintf(stdout, "# testCD[%02d] :: load `%s' g1: %lu\n", task,
                data1_geom[task], ferTimerElapsedInUs(&timer));
    }else{
        g1 = ferCDGeomNew(cd);
        ferCDGeomAddTrisFromRaw(cd, g1, data1_tri[task]);
        ferCDGeomBuild(cd, g1);
        ferTimerStop(&timer);
        fprintf(stdout, "# testCD[%02d] :: build `%s' g1: %lu\n", task,
                data1_tri[task], ferTimerElapsedInUs(&timer));
        ferCDGeomSave(cd, g1, "g1");
    }

    //ferCDGeomDumpOBBSVT(g1, stdout, "g1");

    //ferCDGeomSave(cd, g1, "dragon.geom");
    //ferCDGeomDumpSVT(g1, stdout, "g1");

    ferTimerStart(&timer);
    if (loading_enabled && data2_geom[task]){
        g2 = ferCDGeomLoad(cd, data2_geom[task]);
        ferTimerStop(&timer);
        fprintf(stdout, "# testCD[%02d] :: load `%s' g2: %lu\n", task,
                data2_geom[task], ferTimerElapsedInUs(&timer));
    }else{
        g2 = ferCDGeomNew(cd);
        ferCDGeomAddTrisFromRaw(cd, g2, data2_tri[task]);
        ferCDGeomBuild(cd, g2);
        ferTimerStop(&timer);
        fprintf(stdout, "# testCD[%02d] :: build `%s' g2: %lu\n", task,
                data2_tri[task], ferTimerElapsedInUs(&timer));
        ferCDGeomSave(cd, g2, "g2");
    }
    fflush(stdout);

    for (i = 0; run && nextTrans(&rot2, &tr2, &ret2, data_trans[task]) == 0; i++){
        //ferVec3Scale(&tr2, 0.01);
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        /*
        if (i == 510767){
            ferCDGeomDumpTriSVT(g1, stdout, "g1");
            ferCDGeomDumpTriSVT(g2, stdout, "g2");
        }else{
            continue;
        }
        */
        ret = ferCDGeomCollide(cd, g1, g2);
        //exit(-1);
        if (check_ret[task] && ret != ret2){
            fprintf(stdout, "# testCD[%02d] :: [%04d] FAIL (%d %d)\n", task, i, ret, ret2);
        }
        //fprintf(stderr, " %d\n", ret);
        ferTimerStop(&timer);
        //fprintf(stdout, "# testCD2 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
        if (ret){
            overall_1 += ferTimerElapsedInUs(&timer);
            overall_1_num += 1;
        }else{
            overall_0 += ferTimerElapsedInUs(&timer);
            overall_0_num += 1;
        }

        if (progress && (overall_0_num + overall_1_num) % 100 == 0){
            fprintf(stderr, "# overall: %lu (0: %f) (1: %f) [%u]             \r",
                    overall_time, 
                    (float)overall_0/(float)overall_0_num,
                    (float)overall_1/(float)overall_1_num,
                    overall_0_num + overall_1_num);
        }
    }

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);

    fprintf(stdout, "# testCD[%02d] :: overall_time: %lu                        \n", task, overall_time);
    fprintf(stdout, "# testCD[%02d] :: overall_1: %lu / %u = %f\n", task,
            overall_1, overall_1_num, (float)overall_1/(float)overall_1_num);
    fprintf(stdout, "# testCD[%02d] :: overall_0: %lu / %u = %f\n", task,
            overall_0, overall_0_num, (float)overall_0/(float)overall_0_num);
}

void testRapid(int task)
{
#ifdef HAVE_RAPID
    RAPID_model *g1, *g2;
    size_t i;
    fer_timer_t timer;
    unsigned long overall_time = 0L;
    unsigned long overall_0 = 0L, overall_1 = 0L;
    unsigned int overall_0_num = 0, overall_1_num = 0;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    int ret, ret2;

    fprintf(stdout, "# Task[%02d]: %s\n", task, task_name[task]);

    ferTimerStart(&timer);
    g1 = rapidFromRaw(data1_tri[task]);
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapid[%02d] :: build `%s' g1: %lu\n", task,
            data1_tri[task], ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    g2 = rapidFromRaw(data2_tri[task]);
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapid[%02d] :: build `%s' g2: %lu\n", task,
            data2_tri[task], ferTimerElapsedInUs(&timer));


    for (i = 0; run && nextTrans(&rot2, &tr2, &ret2, data_trans[task]) == 0; i++){
        //ferVec3Scale(&tr2, 0.01);
        ferTimerStart(&timer);

        ret = rapidCollide(g1, g2, &rot2, &tr2);
        //ferCDGeomDumpSVT(g1, stdout, "g1");
        //ferCDGeomDumpSVT(g2, stdout, "g2");
        if (check_ret[task] && ret != ret2){
            fprintf(stdout, "# testRapid[%02d] :: [%04d] FAIL (%d %d)\n", task, i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testRapid2 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
        if (ret){
            overall_1 += ferTimerElapsedInUs(&timer);
            overall_1_num += 1;
        }else{
            overall_0 += ferTimerElapsedInUs(&timer);
            overall_0_num += 1;
        }

        if (progress && (overall_0_num + overall_1_num) % 100 == 0){
            fprintf(stderr, "# overall: %lu (0: %f) (1: %f) [%u]             \r",
                    overall_time, 
                    (float)overall_0/(float)overall_0_num,
                    (float)overall_1/(float)overall_1_num,
                    overall_0_num + overall_1_num);
        }
    }


    delete g1;
    delete g2;

    fprintf(stdout, "# testRapid[%02d] :: overall_time: %lu                         \n", task, overall_time);
    fprintf(stdout, "# testRapid[%02d] :: overall_1: %lu / %u = %f\n", task,
            overall_1, overall_1_num, (float)overall_1/(float)overall_1_num);
    fprintf(stdout, "# testRapid[%02d] :: overall_0: %lu / %u = %f\n", task,
            overall_0, overall_0_num, (float)overall_0/(float)overall_0_num);
#else /* HAVE_RAPID */
    fprintf(stdout, "# testRapid[%02d] :: No RAPID\n", task);
#endif /* HAVE_RAPID */
}



#ifdef HAVE_RAPID
int rapidCollide(RAPID_model *g1, RAPID_model *g2,
                 const fer_mat3_t *rot, const fer_vec3_t *tr)
{
    double R1[3][3], R2[3][3], T1[3], T2[3];
    int result;

    T1[0] = T1[1] = T1[2] = 0;
    R1[0][0] = 1;
    R1[1][0] = 0;
    R1[2][0] = 0;
    R1[0][1] = 0;
    R1[1][1] = 1;
    R1[2][1] = 0;
    R1[0][2] = 0;
    R1[1][2] = 0;
    R1[2][2] = 1;


    T2[0] = ferVec3X(tr);
    T2[1] = ferVec3Y(tr);
    T2[2] = ferVec3Z(tr);
    R2[0][0] = ferMat3Get(rot, 0, 0);
    R2[1][0] = ferMat3Get(rot, 1, 0);
    R2[2][0] = ferMat3Get(rot, 2, 0);
    R2[0][1] = ferMat3Get(rot, 0, 1);
    R2[1][1] = ferMat3Get(rot, 1, 1);
    R2[2][1] = ferMat3Get(rot, 2, 1);
    R2[0][2] = ferMat3Get(rot, 0, 2);
    R2[1][2] = ferMat3Get(rot, 1, 2);
    R2[2][2] = ferMat3Get(rot, 2, 2);

    result = RAPID_Collide(R1, T1, g1,
                           R2, T2, g2,
                           RAPID_FIRST_CONTACT);
    if (result == RAPID_OK){
        if (RAPID_num_contacts == 0)
            return 0;
        return 1;
    }
    return -1;
}

static RAPID_model *rapidFromRaw(const char *fn)
{
    FILE *fin;
    double a[3], b[3], c[3];
    RAPID_model *g;

    fin = fopen(fn, "r");
    if (!fin)
        return NULL;

    g = new RAPID_model;
    g->BeginModel();
    while (fscanf(fin, "%lf %lf %lf %lf %lf %lf %lf %lf %lf",
                  a, a + 1, a + 2, b, b + 1, b + 2, c, c + 1, c + 2) == 9){
        g->AddTri(a, b, c, 0);
    }
    g->EndModel();

    fclose(fin);

    return g;
}
#endif /* HAVE_RAPID */


static FILE *transin = NULL;
static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret,
                     const char *filename)
{
    float x, y, z, w, p, r;
    static int c = 0;

    c++;
    if (!transin)
        transin = fopen(filename, "r");

    if (fscanf(transin, "%f %f %f %f %f %f %d", &x, &y, &z, &w, &p, &r, ret) == 7){
        //fprintf(stderr, "%f %f %f %f %f %f", x, y, z, w, p, r);
        ferMat3SetRot3D(rot, w, p, r);
        ferVec3Set(tr, x, y, z);
        return 0;
    }else{
        c = 0;
        fclose(transin);
        transin = NULL;
        return -1;
    }
}
