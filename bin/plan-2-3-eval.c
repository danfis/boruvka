#include <fermat/mat3.h>

_fer_inline int __eval1(const fer_vec2_t *v)
{
    fer_real_t x, y;

    x = ferVec2X(v);
    y = ferVec2Y(v);

    //p->evals += 1L;

    if (x < -5 || x > 5 || y < -5 || y > 5
            || (x > -3 && x < -2 && y < 4)
            || (x > 2 && x < 3 && y > -4)
       ){
        return OBST;
    }
    return FREE;
}

static int eval1(const fer_vec_t *w, void *data)
{
    static FER_VEC2(b1, 0.25, 0.25);
    static FER_VEC2(b2, -0.25, 0.25);
    static FER_VEC2(b3, -0.25, -0.25);
    static FER_VEC2(b4, 0.25, -0.25);
    fer_mat3_t T;
    fer_vec2_t v;

    ferMat3SetRot(&T, ferVecGet(w, 2));
    ferMat3Set1(&T, 0, 2,  ferVecGet(w, 0));
    ferMat3Set1(&T, 1, 2,  ferVecGet(w, 1));

    ferMat3MulVec2(&v, &T, &b1);
    if (__eval1(&v) == OBST)
        return OBST;
    ferMat3MulVec2(&v, &T, &b2);
    if (__eval1(&v) == OBST)
        return OBST;
    ferMat3MulVec2(&v, &T, &b3);
    if (__eval1(&v) == OBST)
        return OBST;
    ferMat3MulVec2(&v, &T, &b4);
    if (__eval1(&v) == OBST)
        return OBST;
    return FREE;
}



static void setUpScene(const char *scene,
                       int (**eval)(const fer_vec_t *, void *),
                       fer_vec_t *_start, fer_vec_t *_goal,
                       fer_real_t *max_dist)
{
    fer_vec3_t *start = (fer_vec3_t *)_start;
    fer_vec3_t *goal = (fer_vec3_t *)_goal;

    if (strcmp(scene, "1") == 0){
        *eval = eval1;
        ferVec3Set(start, FER_REAL(-4.), FER_REAL(-4.), FER_ZERO);
        ferVec3Set(goal, FER_REAL(4.), FER_REAL(4.), FER_ZERO);
        *max_dist = 0.025;
    }
}

static void dumpScene(const char *scene, FILE *out)
{
    if (strcmp(scene, "1") == 0){
        fprintf(out, "----\n");
        fprintf(out, "Face color: 0.8 0.1 0.8\n");
        fprintf(out, "Point color: 0.8 0.1 0.8\n");
        fprintf(out, "Name: Scene 1\n");
        fprintf(out, "Points:\n");
        fprintf(out, "-3 -5\n-3 4\n-2 4\n-2 -5\n");
        fprintf(out, "2 -4\n2 5\n3 5\n3 -4\n");
        fprintf(out, "Faces:\n");
        fprintf(out, "0 1 2\n0 2 3\n");
        fprintf(out, "4 5 6\n4 6 7\n");
        fprintf(out, "----\n");
    }
}

static void dumpRobot(const char *scene, FILE *out,
                      const fer_vec_t *w)
{
    static FER_VEC2(r11, 0.25, 0.25);
    static FER_VEC2(r12, -0.25, 0.25);
    static FER_VEC2(r13, -0.25, -0.25);
    static FER_VEC2(r14, 0.25, -0.25);
    fer_mat3_t T;
    fer_vec2_t v;

    ferMat3SetRot(&T, ferVecGet(w, 2));
    ferMat3Set1(&T, 0, 2,  ferVecGet(w, 0));
    ferMat3Set1(&T, 1, 2,  ferVecGet(w, 1));

    if (strcmp(scene, "1") == 0){
        fprintf(out, "----\n");
        fprintf(out, "Edge color: 0.1 0.8 0.1\n");
        fprintf(out, "Point color: 0.1 0.8 0.1\n");
        fprintf(out, "Edge width: 1\n");
        fprintf(out, "Point size: 1\n");
        fprintf(out, "Name: Robot 1\n");
        fprintf(out, "Points:\n");
        ferMat3MulVec2(&v, &T, &r11);
        fprintf(out, "%f %f\n", (float)ferVec2X(&v), (float)ferVec2Y(&v));
        ferMat3MulVec2(&v, &T, &r12);
        fprintf(out, "%f %f\n", (float)ferVec2X(&v), (float)ferVec2Y(&v));
        ferMat3MulVec2(&v, &T, &r13);
        fprintf(out, "%f %f\n", (float)ferVec2X(&v), (float)ferVec2Y(&v));
        ferMat3MulVec2(&v, &T, &r14);
        fprintf(out, "%f %f\n", (float)ferVec2X(&v), (float)ferVec2Y(&v));
        fprintf(out, "Edges:\n");
        fprintf(out, "0 1\n1 2\n2 3\n3 0\n");
        fprintf(out, "----\n");
    }
}

