/***
 * fermat
 * -------
 * Copyright (c)2011 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of fermat.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#include <fermat/cd-sphere-grid.h>
#include <fermat/alloc.h>
#include <fermat/dbg.h>


struct _sphere_t {
    fer_vec3_t center;
    fer_real_t radius;
    fer_list_t list;
} fer_packed fer_aligned(16);
typedef struct _sphere_t sphere_t;

struct _cell_sphere_t {
    sphere_t *sphere;
    fer_list_t list;
};
typedef struct _cell_sphere_t cell_sphere_t;



static void assignSpheresToGrid(fer_cd_sphere_grid_t *g);

typedef int (*incident_cell_fn)(fer_cd_sphere_grid_cell_t *cell,
                                void *data);
static void incidentCells(fer_cd_sphere_grid_t *g,
                          fer_real_t radius, const fer_vec3_t *center,
                          incident_cell_fn fn, void *data);


fer_cd_sphere_grid_t *ferCDSphereGridNew(void)
{
    fer_cd_sphere_grid_t *g;

    g = FER_ALLOC_ALIGN(fer_cd_sphere_grid_t, 16);
    g->grid = NULL;
    ferListInit(&g->spheres);

    return g;
}

void ferCDSphereGridDel(fer_cd_sphere_grid_t *g)
{
    fer_list_t *item;
    sphere_t *s;
    cell_sphere_t *cs;
    size_t i, len;

    if (g->grid){
        len = g->dim[0] * g->dim[1] * g->dim[2];
        for (i = 0; i < len; i++){
            while (!ferListEmpty(&g->grid[i].spheres)){
                item = ferListNext(&g->grid[i].spheres);
                ferListDel(item);
                cs = FER_LIST_ENTRY(item, cell_sphere_t, list);
                free(cs);
            }
        }

        free(g->grid);
    }

    while (!ferListEmpty(&g->spheres)){
        item = ferListNext(&g->spheres);
        ferListDel(item);
        s = FER_LIST_ENTRY(item, sphere_t, list);
        free(s);
    }

    free(g);
}


void ferCDSphereGridAdd(fer_cd_sphere_grid_t *g,
                        fer_real_t radius, const fer_vec3_t *center)
{
    sphere_t *s;

    s = FER_ALLOC_ALIGN(sphere_t, 16);
    s->radius = radius;
    ferVec3Copy(&s->center, center);
    ferListAppend(&g->spheres, &s->list);
}

void ferCDSphereGridBuild(fer_cd_sphere_grid_t *g, fer_real_t edge)
{
    fer_real_t min[3], max[3];
    fer_list_t *item;
    sphere_t *s;
    int i, num;

    if (ferListEmpty(&g->spheres))
        return;

    if (g->grid)
        free(g->grid);
    g->grid = NULL;

    min[0] = min[1] = min[2] = FER_REAL_MAX;
    max[0] = max[1] = max[2] = -FER_REAL_MAX;

    // find out aabb for all spheres (fill min[] and max[] arrays)
    // also set edge as average radius
    g->edge = FER_ZERO;
    num = 0;
    FER_LIST_FOR_EACH(&g->spheres, item){
        s = FER_LIST_ENTRY(item, sphere_t, list);

        for (i = 0; i < 3; i++){
            if (ferVec3Get(&s->center, i) - s->radius < min[i])
                min[i] = ferVec3Get(&s->center, i) - s->radius;
            if (ferVec3Get(&s->center, i) + s->radius > max[i])
                max[i] = ferVec3Get(&s->center, i) + s->radius;
        }

        g->edge += s->radius;
        num++;
    }
    g->edge *= FER_REAL(2.) * ferRecp((fer_real_t)num);

    if (edge > FER_ZERO)
        g->edge = edge;

    // set up origin
    ferVec3Set(&g->origin, min[0], min[1], min[2]);

    // set up dimensions
    for (i = 0; i < 3; i++){
        g->dim[i] = (size_t)((max[i] - min[i]) / g->edge) + 1;
    }

    // create new grid
    num = g->dim[0] * g->dim[1] * g->dim[2];
    g->grid = FER_ALLOC_ARR(fer_cd_sphere_grid_cell_t, num);

    for (i = 0; i < num; i++){
        ferListInit(&g->grid[i].spheres);
    }


    assignSpheresToGrid(g);
}


struct __collide_t {
    fer_real_t radius;
    const fer_vec3_t *center;
    int ret;
};

static int __collide(fer_cd_sphere_grid_cell_t *cell, void *data)
{
    struct __collide_t *c = (struct __collide_t *)data;
    fer_list_t *item;
    cell_sphere_t *cs;
    fer_real_t dist;

    FER_LIST_FOR_EACH(&cell->spheres, item){
        cs = FER_LIST_ENTRY(item, cell_sphere_t, list);

        dist = ferVec3Dist(&cs->sphere->center, c->center);
        if (dist < (cs->sphere->radius + c->radius)){
            c->ret = 1;
            return -1;
        }
    }

    return 0;
}

int ferCDSphereGridCollide(const fer_cd_sphere_grid_t *g,
                           fer_real_t radius, const fer_vec3_t *center)
{
    struct __collide_t c;

    if (!g->grid)
        return 0;

    c.radius = radius;
    c.center = center;
    c.ret = 0;

    incidentCells((fer_cd_sphere_grid_t *)g, radius, center, __collide, (void *)&c);

    return c.ret;
}

int ferCDSphereGridCollideBruteForce(const fer_cd_sphere_grid_t *g,
                                     fer_real_t radius,
                                     const fer_vec3_t *center)
{
    fer_list_t *item;
    sphere_t *s;
    fer_real_t dist;

    FER_LIST_FOR_EACH(&g->spheres, item){
        s = FER_LIST_ENTRY(item, sphere_t, list);

        dist = ferVec3Dist(&s->center, center);
        if (dist < (s->radius + radius))
            return 1;
    }

    return 0;
}


static int __assignSphere(fer_cd_sphere_grid_cell_t *c,
                          void *data)
{
    sphere_t *s = (sphere_t *)data;
    cell_sphere_t *cs;

    cs = FER_ALLOC(cell_sphere_t);
    cs->sphere = s;
    ferListAppend(&c->spheres, &cs->list);

    return 0;
}

static void assignSpheresToGrid(fer_cd_sphere_grid_t *g)
{
    fer_list_t *item;
    sphere_t *s;

    FER_LIST_FOR_EACH(&g->spheres, item){
        s = FER_LIST_ENTRY(item, sphere_t, list);
        incidentCells(g, s->radius, &s->center, __assignSphere, (void *)s);
    }
}


static void incidentCells(fer_cd_sphere_grid_t *g,
                          fer_real_t radius, const fer_vec3_t *center,
                          incident_cell_fn fn, void *data)
{
    int x[2], y[2], z[2], id;
    int i, j, k;
    fer_vec3_t c;

    ferVec3Sub2(&c, center, &g->origin);

    x[0] = (ferVec3X(&c) - radius) / g->edge;
    x[1] = (ferVec3X(&c) + radius) / g->edge;
    y[0] = (ferVec3Y(&c) - radius) / g->edge;
    y[1] = (ferVec3Y(&c) + radius) / g->edge;
    z[0] = (ferVec3Z(&c) - radius) / g->edge;
    z[1] = (ferVec3Z(&c) + radius) / g->edge;

    for (i = x[0]; i <= x[1]; i++){
        if (i < 0 || i >= g->dim[0])
            continue;

        for (j = y[0]; j <= y[1]; j++){
            if (j < 0 || j >= g->dim[1])
                continue;

            for (k = z[0]; k <= z[1]; k++){
                if (k < 0 || k >= g->dim[2])
                    continue;

                id = (k * g->dim[0] * g->dim[1])
                        + (j * g->dim[0])
                        + i;

                if (fn(&g->grid[id], data) == -1)
                    return;
            }
        }
    }
}



void ferCDSphereGridCollDumpSVT(const fer_cd_sphere_grid_t *g,
                                fer_real_t radius, const fer_vec3_t *center,
                                FILE *out, const char *name)
{
    fer_list_t *item;
    sphere_t *s;

    fprintf(out, "-----\n");
    if (name)
        fprintf(out, "Name: %s\n", name);

    fprintf(out, "Spheres:\n");
    FER_LIST_FOR_EACH(&g->spheres, item){
        s = FER_LIST_ENTRY(item, sphere_t, list);
        fprintf(out, "%f ", (float)s->radius);
        ferVec3Print(&s->center, out);
        fprintf(out, "\n");
    }

    fprintf(out, "-----\n");

    fprintf(out, "Spheres:\n");
    fprintf(out, "%f ", (float)radius);
    ferVec3Print(center, out);
    fprintf(out, "\n");
    fprintf(out, "-----\n");
}
