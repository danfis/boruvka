#include <fermat/cd.h>
#include <fermat/timer.h>
#ifdef HAVE_RAPID
# include <RAPID.H>
#endif
#include "data.h"

void testCD1(void);
void testCD2(void);
void testCD3(void);
void testRapid1(void);
void testRapid2(void);
void testRapid3(void);


int rapidCollide(RAPID_model *g1, RAPID_model *g2,
                 const fer_mat3_t *rot, const fer_vec3_t *tr);
static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret);
static int nextTransBox(fer_mat3_t *rot, fer_vec3_t *tr, int *ret);

int main(int argc, char *argv[])
{
    if (argc == 2){
        if (strcmp(argv[1], "1") == 0){
            testCD1();
            testRapid1();
        }else if (strcmp(argv[1], "2") == 0){
            testCD2();
            testRapid2();
        }else if (strcmp(argv[1], "3") == 0){
            testCD3();
            testRapid3();
        }
    }else{
        testCD1();
        testRapid1();

        testCD2();
        testRapid2();

        testCD3();
        testRapid3();
    }

    return 0;
}

void testCD1(void)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;

    cd = ferCDNew();

    //ferCDSetBuildFlags(cd, FER_CD_FIT_CALIPERS |
    //        FER_CD_FIT_CALIPERS_NUM_ROT(5));
    //ferCDSetBuildFlags(cd, FER_CD_FIT_POLYHEDRAL_MASS);

    ferTimerStart(&timer);
    g1 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g1, bunny_coords, bunny_ids, bunny_tri_len);
    ferCDGeomBuild(cd, g1);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD1 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    g2 = ferCDGeomNew(cd);
    ferCDGeomAddTriMesh(cd, g2, bunny_coords, bunny_ids, bunny_tri_len);
    ferCDGeomBuild(cd, g2);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD1 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));

    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        ret = ferCDGeomCollide(cd, g1, g2);
        if (ret != ret2){
            fprintf(stdout, "# testCD1 :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testCD1 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);

    fprintf(stdout, "# testCD1 :: overall_time: %lu\n", overall_time);
}

void testCD2(void)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;

    cd = ferCDNew();

    //ferCDSetBuildFlags(cd, FER_CD_FIT_COVARIANCE);
    //ferCDSetBuildFlags(cd, FER_CD_FIT_CALIPERS |
    //        FER_CD_FIT_CALIPERS_NUM_ROT(5));
    //ferCDSetBuildFlags(cd, FER_CD_FIT_POLYHEDRAL_MASS);
    ferCDSetBuildFlags(cd, FER_CD_FIT_NAIVE | FER_CD_FIT_NAIVE_NUM_ROT(7));

    ferTimerStart(&timer);
    //g1 = ferCDGeomNew(cd);
    //ferCDGeomAddTrisFromRaw(cd, g1, "dragon.tri");
    //ferCDGeomBuild(cd, g1);
    g1 = ferCDGeomLoad(cd, "dragon.geom");
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD2 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));
    fflush(stdout);

    //ferCDGeomSave(cd, g1, "dragon.geom");
    //ferCDGeomDumpSVT(g1, stdout, "g1");

    ferTimerStart(&timer);
    //g2 = ferCDGeomNew(cd);
    //ferCDGeomAddTrisFromRaw(cd, g2, "dragon.tri");
    //ferCDGeomBuild(cd, g2);
    g2 = ferCDGeomLoad(cd, "dragon.geom");
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD2 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));

    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferVec3Scale(&tr2, 0.01);
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        ret = ferCDGeomCollide(cd, g1, g2);
        if (ret != ret2){
            //fprintf(stdout, "# testCD2 :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testCD2 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);

    fprintf(stdout, "# testCD2 :: overall_time: %lu\n", overall_time);
}

void testCD3(void)
{
    fer_cd_t *cd;
    fer_cd_geom_t *g1, *g2;
    fer_timer_t timer;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    size_t i;
    int ret, ret2;
    unsigned long overall_time = 0L;

    cd = ferCDNew();

    //ferCDSetBuildFlags(cd, FER_CD_FIT_CALIPERS |
    //       FER_CD_FIT_CALIPERS_NUM_ROT(5));
    //ferCDSetBuildFlags(cd, FER_CD_FIT_POLYHEDRAL_MASS);
    ferCDSetBuildFlags(cd, FER_CD_FIT_NAIVE | FER_CD_FIT_NAIVE_NUM_ROT(7));

    ferTimerStart(&timer);
    g1 = ferCDGeomNew(cd);
    ferCDGeomAddTrisFromRaw(cd, g1, "data-box1.tri");
    ferCDGeomBuild(cd, g1);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD3 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    g2 = ferCDGeomNew(cd);
    ferCDGeomAddTrisFromRaw(cd, g2, "data-box2.tri");
    ferCDGeomBuild(cd, g2);
    ferTimerStop(&timer);
    fprintf(stdout, "# testCD3 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));

    for (i = 0; nextTransBox(&rot2, &tr2, &ret2) == 0; i++){
        ferCDGeomSetRot(cd, g2, &rot2);
        ferCDGeomSetTr(cd, g2, &tr2);
        ferTimerStart(&timer);

        ret = ferCDGeomCollide(cd, g1, g2);
        if (ret != ret2){
            //fprintf(stdout, "# testCD3 :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testCD3 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }

    ferCDGeomDel(cd, g1);
    ferCDGeomDel(cd, g2);
    ferCDDel(cd);

    fprintf(stdout, "# testCD3 :: overall_time: %lu\n", overall_time);
}

void testRapid1(void)
{
#ifdef HAVE_RAPID
    RAPID_model *g1, *g2;
    double a[3], b[3], c[3];
    size_t i, j;
    fer_timer_t timer;
    unsigned long overall_time = 0L;
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
    fprintf(stdout, "# testRapid1 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

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
    fprintf(stdout, "# testRapid1 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferTimerStart(&timer);

        ret = rapidCollide(g1, g2, &rot2, &tr2);
        //ferCDGeomDumpSVT(g1, stdout, "g1");
        //ferCDGeomDumpSVT(g2, stdout, "g2");
        if (ret != ret2){
            fprintf(stdout, "# testRapid1 :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testRapid1 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }


    delete g1;
    delete g2;

    fprintf(stdout, "# testRapid1 :: overall_time: %lu\n", overall_time);
#else /* HAVE_RAPID */
    fprintf(stdout, "# testRapid1 :: No RAPID\n");
#endif /* HAVE_RAPID */
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

void testRapid2(void)
{
#ifdef HAVE_RAPID
    RAPID_model *g1, *g2;
    size_t i;
    fer_timer_t timer;
    unsigned long overall_time = 0L;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    int ret, ret2;

    ferTimerStart(&timer);
    g1 = rapidFromRaw("dragon.tri");
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapid2 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    g2 = rapidFromRaw("dragon.tri");
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapid2 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTrans(&rot2, &tr2, &ret2) == 0; i++){
        ferVec3Scale(&tr2, 0.01);
        ferTimerStart(&timer);

        ret = rapidCollide(g1, g2, &rot2, &tr2);
        //ferCDGeomDumpSVT(g1, stdout, "g1");
        //ferCDGeomDumpSVT(g2, stdout, "g2");
        if (ret != ret2){
            //fprintf(stdout, "# testRapid2 :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testRapid2 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }


    delete g1;
    delete g2;

    fprintf(stdout, "# testRapid2 :: overall_time: %lu\n", overall_time);
#else /* HAVE_RAPID */
    fprintf(stdout, "# testRapid2 :: No RAPID\n");
#endif /* HAVE_RAPID */
}

void testRapid3(void)
{
#ifdef HAVE_RAPID
    RAPID_model *g1, *g2;
    size_t i;
    fer_timer_t timer;
    unsigned long overall_time = 0L;
    fer_mat3_t rot2;
    fer_vec3_t tr2;
    int ret, ret2;

    ferTimerStart(&timer);
    g1 = rapidFromRaw("data-box1.tri");
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapid3 :: build g1: %lu\n", ferTimerElapsedInUs(&timer));

    ferTimerStart(&timer);
    g2 = rapidFromRaw("data-box2.tri");
    ferTimerStop(&timer);
    fprintf(stdout, "# testRapid3 :: build g2: %lu\n", ferTimerElapsedInUs(&timer));


    for (i = 0; nextTransBox(&rot2, &tr2, &ret2) == 0; i++){
        ferTimerStart(&timer);

        ret = rapidCollide(g1, g2, &rot2, &tr2);
        //ferCDGeomDumpSVT(g1, stdout, "g1");
        //ferCDGeomDumpSVT(g2, stdout, "g2");
        if (ret != ret2){
            //fprintf(stdout, "# testRapid3 :: [%04d] FAIL (%d %d)\n", i, ret, ret2);
        }
        ferTimerStop(&timer);
        //fprintf(stdout, "# testRapid3 :: Coll[%02d] %04lu - %d (%d)\n", i, ferTimerElapsedInUs(&timer), ret, ret2);
        overall_time += ferTimerElapsedInUs(&timer);
    }


    delete g1;
    delete g2;

    fprintf(stdout, "# testRapid3 :: overall_time: %lu\n", overall_time);
#else /* HAVE_RAPID */
    fprintf(stdout, "# testRapid3 :: No RAPID\n");
#endif /* HAVE_RAPID */
}

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

static FILE *transin = NULL;
static int nextTrans(fer_mat3_t *rot, fer_vec3_t *tr, int *ret)
{
    float x, y, z, w, p, r;
    static int c = 0;

    c++;
    if (!transin)
        transin = fopen("data-test-cd-trimesh.trans.txt", "r");

    if (fscanf(transin, "%f %f %f %f %f %f %d", &x, &y, &z, &w, &p, &r, ret) == 7){
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

static FILE *transin_box = NULL;
static int nextTransBox(fer_mat3_t *rot, fer_vec3_t *tr, int *ret)
{
    float x, y, z, w, p, r;
    static int c = 0;

    c++;
    if (!transin_box)
        transin_box = fopen("data-test-cd-trimesh-box.trans.txt", "r");

    if (fscanf(transin_box, "%f %f %f %f %f %f %d", &x, &y, &z, &w, &p, &r, ret) == 7){
        ferMat3SetRot3D(rot, w, p, r);
        ferVec3Set(tr, x, y, z);
        return 0;
    }else{
        c = 0;
        fclose(transin_box);
        transin_box = NULL;
        return -1;
    }
}
