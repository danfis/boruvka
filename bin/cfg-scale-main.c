#include <fermat/cfg.h>

int main(int argc, char *argv[])
{
    char format[1000];
    char *name;
    fer_real_t *aabb;
    fer_real_t scale;
    fer_vec2_t m, w;
    const fer_vec2_t *pts;
    size_t pts_len;
    const int *ids;
    size_t ids_len;
    fer_vec2_t init, goal;
    fer_real_t h;
    size_t j, i;
    fer_cfg_t *cfg;
    char **robots;
    size_t robots_len;

    cfg = ferCfgRead(argv[1]);

    ferCfgScan(cfg, "name:s aabb:f[] pts:v2[] pts:v2# ids:i[] ids:i#",
            &name, &aabb, &pts, &pts_len, &ids, &ids_len);
    ferVec2Set(&m, (aabb[0] + aabb[1]) / 2., (aabb[2] + aabb[3]) / 2.);
    scale = FER_MAX(aabb[1] - aabb[0], aabb[3] - aabb[2]);
    scale = scale / 2.;
    if (strcmp(argv[2], "pi") == 0){
        scale = M_PI / scale;
    }else{
        scale = atof(argv[2]) / scale;
    }


    printf("name = '%s'\n", name);
    printf("dim:i = 2\n");
    printf("aabb:f[6] = %f %f %f %f 0 0\n",
            scale * (aabb[0] - ferVec2X(&m)),
            scale * (aabb[1] - ferVec2X(&m)),
            scale * (aabb[2] - ferVec2Y(&m)),
            scale * (aabb[3] - ferVec2Y(&m)));
    printf("\n");
    printf("pts:v3[%d] =\n", (int)pts_len);
    for (i = 0; i < pts_len; i++){
        ferVec2Sub2(&w, &pts[i], &m);
        ferVec2Scale(&w, scale);
        printf("    %f %f 0.\n", ferVec2X(&w), ferVec2Y(&w));
    }
    printf("\n");
    printf("ids:i[%d] =\n", (int)ids_len);
    for (i = 0; i < ids_len; i += 3){
        printf("    %d %d %d\n", ids[i], ids[i + 1], ids[i + 2]);
    }

    ferCfgParamStrArr(cfg, "robots", &robots, &robots_len);
    printf("\nrobots:s[%d] =\n", (int)robots_len);
    for (j = 0; j < robots_len; j++){
        printf("    '%s'\n", robots[j]);
    }
    for (j = 0; j < robots_len; j++){
        snprintf(format, 1000, "%s_pts:v2[] %s_pts:v2# %s_ids:i[] %s_ids:i# %s_init:v2 %s_goal:v2 %s_h:f",
                robots[j], robots[j],
                robots[j], robots[j],
                robots[j], robots[j],
                robots[j]);
        ferCfgScan(cfg, format, &pts, &pts_len, &ids, &ids_len, &init, &goal, &h);

        printf("\n");
        printf("%s_pts:v3[%d] =\n", robots[j], (int)pts_len);
        for (i = 0; i < pts_len; i++){
            ferVec2Copy(&w, &pts[i]);
            ferVec2Scale(&w, scale);
            printf("    %f %f 0.\n", ferVec2X(&w), ferVec2Y(&w));
        }
        printf("%s_ids:i[%d] =\n", robots[j], (int)ids_len);
        for (i = 0; i < ids_len; i += 3){
            printf("    %d %d %d\n", ids[i], ids[i + 1], ids[i + 2]);
        }
        ferVec2Sub2(&w, &init, &m);
        ferVec2Scale(&w, scale);
        printf("%s_init:v3 = %f %f 0.\n", robots[j], ferVec2X(&w), ferVec2Y(&w));
        ferVec2Sub2(&w, &goal, &m);
        ferVec2Scale(&w, scale);
        printf("%s_goal:v3 = %f %f 0.\n", robots[j], ferVec2X(&w), ferVec2Y(&w));
        printf("%s_h = %f\n", robots[j], h * scale);
    }

    return 0;
}
