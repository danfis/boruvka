
static int eval1(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y < 4 && y > -2 && x > -r && x < r)
            || (y > 4 && x > -2 && x < 2)){
        return FREE;
    }
    return OBST;
}

static int eval1_1(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.01);
}

static int eval1_3(const fer_vec_t *w, void *data)
{
    return eval1(w, data, 0.03);
}

static int eval1_5(const fer_vec_t *w, void *data)
{
    //return eval1(w, data, 0.1);
    return eval1(w, data, 0.05);
}

static int eval2(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y > 4 && x > 4)
            || (y > -2 && y < 4 && x > -2 - r && x < 4 + r && x > y - r && x < y + r)){
        return FREE;
    }
    return OBST;
}

static int eval2_1(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.01);
}

static int eval2_3(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.03);
}

static int eval2_5(const fer_vec_t *w, void *data)
{
    return eval2(w, data, 0.05);
}


static int eval3(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
            || (y > 0 && x > 3.5)
            || (x < 3.6 && FER_CUBE(x - 1) + FER_CUBE(y + 1) > 9
                    && FER_CUBE(x - 1) + FER_CUBE(y + 1) < FER_CUBE(3 + r))){
        return FREE;
    }
    return OBST;
}

static int eval3_1(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.01);
}

static int eval3_3(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.03);
}

static int eval3_5(const fer_vec_t *w, void *data)
{
    return eval3(w, data, 0.05);
}

static int eval4(const fer_vec_t *w, void *data, fer_real_t r)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if (y < -2
         || (x > 0.3 && x < 0.6 && y > -0.15 && y < 0.15)
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(2.)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(2. + r)
                && !(y < 0 && x > 0))
         || (FER_CUBE(x - 0.5) + FER_CUBE(y) > FER_CUBE(1.5)
                && FER_CUBE(x - 0.5) + FER_CUBE(y) < FER_CUBE(1.5 + r)
                && !(y > 0))
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(1.)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(1. + r)
                && !(y < 0))
         || (FER_CUBE(x - 0.25) + FER_CUBE(y) > FER_CUBE(.75)
                && FER_CUBE(x - 0.25) + FER_CUBE(y) < FER_CUBE(.75 + r)
                && !(y > 0))
         || (FER_CUBE(x) + FER_CUBE(y) > FER_CUBE(.5)
                && FER_CUBE(x) + FER_CUBE(y) < FER_CUBE(.5 + r)
                && !(y < 0))){
        return FREE;
    }
    return OBST;
}

static int eval4_1(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.01);
}

static int eval4_3(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.03);
}

static int eval4_5(const fer_vec_t *w, void *data)
{
    return eval4(w, data, 0.05);
}

static int eval5_5(const fer_vec_t *w, void *data)
{
    fer_real_t x, y;
    x = ferVecGet(w, 0);
    y = ferVecGet(w, 1);

    //p->evals += 1L;

    if ((x > -2 && x < 2 && y > -2 && y < 2)
            || (x > -4.5 && x < -4 && y > 4)
            || (x > -4.5 && x < -4 && y > 3.5 && y < 3.95)
            || (x < -4.45 && y > 3.5 && y < 3.95)

            || (x > 2 && x < 4.5 && y < -3.)
            || (x > 4.55 && y > -4.5 && y < -3.)
        ){
        return OBST;
    }
    return FREE;
}


static void setUpScene(const char *scene,
                       int (**eval)(const fer_vec_t *, void *),
                       fer_vec_t *_start, fer_vec_t *_goal,
                       fer_real_t *max_dist)
{
    fer_vec2_t *start = (fer_vec2_t *)_start;
    fer_vec2_t *goal = (fer_vec2_t *)_goal;

    if (strcmp(scene, "1_1") == 0){
        *eval = eval1_1;
        ferVec2Set(start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set(goal, FER_REAL(1.5), FER_REAL(4.5));
        *max_dist = 0.005;
    }else if (strcmp(scene, "1_3") == 0){
        *eval = eval1_3;
        ferVec2Set(start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set(goal, FER_REAL(1.5), FER_REAL(4.5));
        *max_dist = 0.015;
    }else if (strcmp(scene, "1_5") == 0){
        *eval = eval1_5;
        ferVec2Set(start, FER_REAL(-4.), FER_REAL(-4.));
        ferVec2Set(goal, FER_REAL(1.5), FER_REAL(4.5));
        *max_dist = 0.025;
    }else if (strcmp(scene, "2_1") == 0){
        *eval = eval2_1;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(4.5), FER_REAL(4.5));
        *max_dist = 0.005;
    }else if (strcmp(scene, "2_3") == 0){
        *eval = eval2_3;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(4.5), FER_REAL(4.5));
        *max_dist = 0.015;
    }else if (strcmp(scene, "2_5") == 0){
        *eval = eval2_5;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(4.5), FER_REAL(4.5));
        *max_dist = 0.025;
    }else if (strcmp(scene, "3_1") == 0){
        *eval = eval3_1;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(4.5), FER_REAL(4.5));
        *max_dist = 0.005;
    }else if (strcmp(scene, "3_3") == 0){
        *eval = eval3_3;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(4.5), FER_REAL(4.5));
        *max_dist = 0.015;
    }else if (strcmp(scene, "3_5") == 0){
        *eval = eval3_5;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(4.5), FER_REAL(4.5));
        *max_dist = 0.025;
    }else if (strcmp(scene, "4_1") == 0){
        *eval = eval4_1;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(0.45), FER_REAL(0.));
        *max_dist = 0.005;
    }else if (strcmp(scene, "4_3") == 0){
        *eval = eval4_3;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(0.45), FER_REAL(0.));
        *max_dist = 0.015;
    }else if (strcmp(scene, "4_5") == 0){
        *eval = eval4_5;
        ferVec2Set(start, FER_REAL(4.5), FER_REAL(-4.5));
        ferVec2Set(goal, FER_REAL(0.45), FER_REAL(0.));
        *max_dist = 0.025;
    }else if (strcmp(scene, "5_5") == 0){
        *eval = eval5_5;
        ferVec2Set(start, FER_REAL(-4.7), FER_REAL(4.7));
        ferVec2Set(goal, FER_REAL(4.7), FER_REAL(-4.7));
        *max_dist = 0.025;
    }
}
