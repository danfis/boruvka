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


/***
(geom Tr Obbs)

Tr :=
    tr_x tr_y tr_z rot11 rot12 rot13 rot21 rot22 rot23 rot31 rot32 rot33
Obbs :=
    Obb [ Obb [ Obb [ ... ] ] ]
Obb :=
    (obb c_x c_y c_z
         a0_x a0_y a0_z
         a1_x a1_y a1_z
         a2_x a2_y a2_z
         he_0 he_1 he2
         [ Obbs | Shape ])
Shape :=
    (tri p0_x p0_y p0_z p1_x p1_y p1_z p2_x p2_y p2_z)
    (box he_x he_y he_z)
    (cyl radius half_height)
    (sphere radius)
    (off Tr Shape)
*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fermat/cd-geom.h>
#include <fermat/parse.h>
#include <fermat/dbg.h>

static void _ferCDGeomSaveObb(fer_cd_obb_t *obb, FILE *fout);

int ferCDGeomSave(fer_cd_t *cd, const fer_cd_geom_t *g,
                  const char *filename)
{
    FILE *fout;
    fer_list_t *item;
    fer_cd_obb_t *obb;

    fout = fopen(filename, "w");
    if (!fout)
        return -1;

    // 1. header
    fprintf(fout, "(geom ");
    // 2. translation
    fprintf(fout, "%f %f %f\n",
            (float)ferVec3X(&g->tr), (float)ferVec3Y(&g->tr),
            (float)ferVec3Z(&g->tr));
    // 3. rotation
    fprintf(fout, "      %f %f %f %f %f %f %f %f %f\n",
            (float)ferMat3Get(&g->rot, 0, 0),
            (float)ferMat3Get(&g->rot, 0, 1),
            (float)ferMat3Get(&g->rot, 0, 2),
            (float)ferMat3Get(&g->rot, 1, 0),
            (float)ferMat3Get(&g->rot, 1, 1),
            (float)ferMat3Get(&g->rot, 1, 2),
            (float)ferMat3Get(&g->rot, 2, 0),
            (float)ferMat3Get(&g->rot, 2, 1),
            (float)ferMat3Get(&g->rot, 2, 2));

    // 4. Obbs
    FER_LIST_FOR_EACH(&g->obbs, item){
        obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
        _ferCDGeomSaveObb(obb, fout);
    }

    // 5. footer
    fprintf(fout, ")\n");

    fclose(fout);

    return 0;
}

static void _saveShape(fer_cd_shape_t *shape, FILE *fout)
{
    fer_cd_tri_t *tri;
    fer_cd_box_t *box;
    fer_cd_cyl_t *cyl;
    fer_cd_sphere_t *sphere;
    fer_cd_shape_off_t *off;

    if (shape->cl->type == FER_CD_SHAPE_TRI
            || shape->cl->type == FER_CD_SHAPE_TRIMESH_TRI){
        tri = (fer_cd_tri_t *)shape;

        fprintf(fout, "(tri %f %f %f\n", 
                (float)ferVec3X(tri->p0),
                (float)ferVec3Y(tri->p0),
                (float)ferVec3Z(tri->p0));
        fprintf(fout, "     %f %f %f\n", 
                (float)ferVec3X(tri->p1),
                (float)ferVec3Y(tri->p1),
                (float)ferVec3Z(tri->p1));
        fprintf(fout, "     %f %f %f )\n", 
                (float)ferVec3X(tri->p2),
                (float)ferVec3Y(tri->p2),
                (float)ferVec3Z(tri->p2));
    }else if (shape->cl->type == FER_CD_SHAPE_BOX){
        box = (fer_cd_box_t *)shape;
        fprintf(fout, "(box %f %f %f )\n", 
                (float)ferVec3X(box->half_extents),
                (float)ferVec3Y(box->half_extents),
                (float)ferVec3Z(box->half_extents));
    }else if (shape->cl->type == FER_CD_SHAPE_CYL){
        cyl = (fer_cd_cyl_t *)shape;
        fprintf(fout, "(cyl %f %f )\n",
                (float)cyl->radius, (float)cyl->half_height);
    }else if (shape->cl->type == FER_CD_SHAPE_SPHERE){
        sphere = (fer_cd_sphere_t *)shape;
        fprintf(fout, "(sphere %f )\n", (float)sphere->radius);
    }else if (shape->cl->type == FER_CD_SHAPE_OFF){
        off = (fer_cd_shape_off_t *)shape;

        fprintf(fout, "(off %f %f %f\n", 
                (float)ferVec3X(off->tr), (float)ferVec3Y(off->tr),
                (float)ferVec3Z(off->tr));
        fprintf(fout, "      %f %f %f %f %f %f %f %f %f\n",
                (float)ferMat3Get(off->rot, 0, 0),
                (float)ferMat3Get(off->rot, 0, 1),
                (float)ferMat3Get(off->rot, 0, 2),
                (float)ferMat3Get(off->rot, 1, 0),
                (float)ferMat3Get(off->rot, 1, 1),
                (float)ferMat3Get(off->rot, 1, 2),
                (float)ferMat3Get(off->rot, 2, 0),
                (float)ferMat3Get(off->rot, 2, 1),
                (float)ferMat3Get(off->rot, 2, 2));

        _saveShape(off->shape, fout);

        fprintf(fout, ")\n");
    }
}

static void _ferCDGeomSaveObb(fer_cd_obb_t *o, FILE *fout)
{
    fer_list_t *item;
    fer_cd_obb_t *obb;

    // 1. header
    fprintf(fout, "(obb ");

    // 2. center
    fprintf(fout, "%f %f %f\n", 
            (float)ferVec3X(&o->center), (float)ferVec3Y(&o->center),
            (float)ferVec3Z(&o->center));

    // 3. axis[0,1,2]
    fprintf(fout, "     %f %f %f\n", 
            (float)ferVec3X(&o->axis[0]), (float)ferVec3Y(&o->axis[0]),
            (float)ferVec3Z(&o->axis[0]));
    fprintf(fout, "     %f %f %f\n", 
            (float)ferVec3X(&o->axis[1]), (float)ferVec3Y(&o->axis[1]),
            (float)ferVec3Z(&o->axis[1]));
    fprintf(fout, "     %f %f %f\n", 
            (float)ferVec3X(&o->axis[2]), (float)ferVec3Y(&o->axis[2]),
            (float)ferVec3Z(&o->axis[2]));

    // 4. half extents
    fprintf(fout, "     %f %f %f\n", 
            (float)ferVec3X(&o->half_extents),
            (float)ferVec3Y(&o->half_extents),
            (float)ferVec3Z(&o->half_extents));

    // 5. Obbs or shape
    if (!ferListEmpty(&o->obbs)){
        FER_LIST_FOR_EACH(&o->obbs, item){
            obb = FER_LIST_ENTRY(item, fer_cd_obb_t, list);
            _ferCDGeomSaveObb(obb, fout);
        }
    }else if (o->shape){
        _saveShape(o->shape, fout);
    }

    // 6. footer
    fprintf(fout, ")\n");
}



static fer_cd_geom_t *_loadGeom(fer_cd_t *cd, const char *file, size_t size);
static int _loadOBBs(fer_cd_geom_t *g, char **fstr, char *fend);
static fer_cd_obb_t *_parseOBB(char **fstr, char *fend);

fer_cd_geom_t *ferCDGeomLoad(fer_cd_t *cd, const char *filename)
{
    int fd;
    size_t size;
    struct stat st;
    void *file;
    fer_cd_geom_t *geom;

    // open file
    if ((fd = open(filename, O_RDONLY)) == -1){
        fprintf(stderr, "CD Error: Can't open file `%s'\n", filename);
        return NULL;
    }

    // get stats (mainly size of file)
    if (fstat(fd, &st) == -1){
        close(fd);
        fprintf(stderr, "CD Error: Can't get file info of `%s'\n", filename);
        return NULL;
    }

    // pick up size of file
    size = st.st_size;

    // mmap whole file into memory, we need only read from it and don't need
    // to share anything
    file = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file == MAP_FAILED){
        close(fd);
        fprintf(stderr, "CD Error: Can't map file `%s' into memory: %s\n", filename, strerror(errno));
        return NULL;
    }

    // set up char pointers to current char (fstr) and to end of memory (fend)
    geom = _loadGeom(cd, file, size);

    // unmap mapped memory
    munmap(file, size);

    // close file
    close(fd);

    return geom;
}

#define NOT_WS(c) \
    ( c != ' ' && c != '\t' && c != '\n')

static fer_cd_geom_t *_loadGeom(fer_cd_t *cd, const char *file, size_t size)
{
    char *fstr, *fend, *fnext;
    fer_cd_geom_t *geom;
    fer_real_t f[9];
    int i;

    fstr = (char *)file;
    fend = (char *)(file + size);

    while (fstr < fend && *fstr != '(')
        ++fstr;
    if (fstr >= fend)
        return NULL;
    ++fstr;

    // skip whitespace
    while (fstr < fend && !NOT_WS(*fstr))
        ++fstr;
    if (fstr >= fend)
        return NULL;

    // check it begins with 'geom'
    if (fstr + 4 >= fend || strncmp(fstr, "geom", 4) != 0){
        return NULL;
    }
    fstr += 4;

    // skip whitespace
    while (fstr < fend && !NOT_WS(*fstr))
        ++fstr;
    if (fstr >= fend)
        return NULL;

    // we found geom
    geom = ferCDGeomNew(cd);

    // parse translation
    if (ferParseVec3(fstr, fend, &geom->tr, &fnext) != 0){
        ferCDGeomDel(cd, geom);
        return NULL;
    }
    fstr = fnext;

    // parse rotation
    for (i = 0; i < 9; i++){
        if (ferParseReal(fstr, fend, &f[i], &fnext) != 0){
            break;
        }
        fstr = fnext;
    }
    if (i != 9){
        ferCDGeomDel(cd, geom);
        return NULL;
    }
    fstr = fnext;

    ferMat3Set(&geom->rot, f[0], f[1], f[2],
                           f[3], f[4], f[5],
                           f[6], f[7], f[8]);

    // parse OBBs
    if (_loadOBBs(geom, &fstr, fend) != 0){
        ferCDGeomDel(cd, geom);
        return NULL;
    }

    return geom;
}

static int _loadOBBs(fer_cd_geom_t *g, char **fstr, char *fend)
{
    fer_cd_obb_t *obb;

    while ((obb = _parseOBB(fstr, fend)) != NULL){
        ferListAppend(&g->obbs, &obb->list);
    }

    return 0;
}

static int _loadOBBs2(fer_cd_obb_t *obb, char **fstr, char *fend)
{
    fer_cd_obb_t *o;
    int num = 0;

    while ((o = _parseOBB(fstr, fend)) != NULL){
        ferListAppend(&obb->obbs, &o->list);
        num++;
    }

    return num;
}

static fer_cd_shape_t *_parseShape(char **fstr, char *fend)
{
    fer_cd_shape_t *shape, *s;
    fer_vec3_t v[3];
    fer_mat3_t m;
    fer_real_t f[9];
    char *fnext;
    int i;

    // skip whitespace
    while (*fstr < fend && !NOT_WS(**fstr)){
        ++(*fstr);
    }
    if (*fstr >= fend)
        return NULL;

    if (**fstr != '(')
        return NULL;
    ++(*fstr);

    shape = NULL;
    if (*fstr + 3 < fend && strncmp(*fstr, "tri", 3) == 0){
        *fstr += 3;

        for (i = 0; i < 3; i++){
            if (ferParseVec3(*fstr, fend, &v[i], &fnext) != 0)
                return NULL;
            *fstr = fnext;
        }

        shape = (fer_cd_shape_t *)ferCDTriNew(&v[0], &v[1], &v[2]);
    }else if (*fstr + 3 < fend && strncmp(*fstr, "box", 3) == 0){
        *fstr += 3;
        if (ferParseVec3(*fstr, fend, &v[0], &fnext) != 0)
            return NULL;
        *fstr = fnext;

        shape = (fer_cd_shape_t *)ferCDBoxNew(ferVec3X(&v[0]),
                                              ferVec3Y(&v[0]),
                                              ferVec3Z(&v[0]));
    }else if (*fstr + 3 < fend && strncmp(*fstr, "cyl", 3) == 0){
        *fstr += 3;
        if (ferParseReal(*fstr, fend, &f[0], &fnext) != 0)
            return NULL;
        *fstr = fnext;
        if (ferParseReal(*fstr, fend, &f[1], &fnext) != 0)
            return NULL;
        *fstr = fnext;

        shape = (fer_cd_shape_t *)ferCDCylNew(f[0], f[1]);
    }else if (*fstr + 6 < fend && strncmp(*fstr, "sphere", 6) == 0){
        *fstr += 3;
        if (ferParseReal(*fstr, fend, &f[0], &fnext) != 0)
            return NULL;
        *fstr = fnext;

        shape = (fer_cd_shape_t *)ferCDSphereNew(f[0]);
    }else if (*fstr + 3 < fend && strncmp(*fstr, "off", 3) == 0){
        *fstr += 3;

        if (ferParseVec3(*fstr, fend, &v[0], &fnext) != 0)
            return NULL;
        *fstr = fnext;

        for (i = 0; i < 9; i++){
            if (ferParseReal(*fstr, fend, &f[i], &fnext) != 0)
                return NULL;
            *fstr = fnext;
        }
        if (i != 9)
            return NULL;

        s = _parseShape(fstr, fend);
        if (!s)
            return NULL;

        ferMat3Set(&m, f[0], f[1], f[2],
                       f[3], f[4], f[5],
                       f[6], f[7], f[8]);
        shape = (fer_cd_shape_t *)ferCDShapeOffNew(s, &m, &v[0]);
    }else{
        return NULL;
    }


    // skip whitespace
    while (*fstr < fend && !NOT_WS(**fstr))
        ++(*fstr);
    if (*fstr >= fend){
        shape->cl->del(shape);
        return NULL;
    }

    if (**fstr != ')'){
        shape->cl->del(shape);
        return NULL;
    }
    ++(*fstr);

    return shape;
}

static fer_cd_obb_t *_parseOBB(char **fstr, char *fend)
{
    char *fnext;
    fer_cd_obb_t *obb;
    fer_cd_shape_t *shape;
    int i;

    // skip whitespace
    while (*fstr < fend && !NOT_WS(**fstr))
        ++(*fstr);
    if (*fstr >= fend)
        return NULL;

    if (**fstr != '(')
        return NULL;
    ++(*fstr);

    // skip whitespace
    while (*fstr < fend && !NOT_WS(**fstr))
        ++(*fstr);
    if (*fstr >= fend)
        return NULL;

    if (*fstr + 3 >= fend || strncmp(*fstr, "obb", 3) != 0){
        // unwind to open parenthesis
        while (**fstr != '(')
            --(*fstr);
        return NULL;
    }
    *fstr += 3;

    obb = ferCDOBBNew();

    // parse center
    if (ferParseVec3(*fstr, fend, &obb->center, &fnext) != 0){
        ferCDOBBDel(obb);
        return NULL;
    }
    *fstr = fnext;

    // parse axis[0,1,2]
    for (i = 0; i < 3; i++){
        if (ferParseVec3(*fstr, fend, &obb->axis[i], &fnext) != 0){
            ferCDOBBDel(obb);
            return NULL;
        }
        *fstr = fnext;
    }

    // parse half_extents
    if (ferParseVec3(*fstr, fend, &obb->half_extents, &fnext) != 0){
        ferCDOBBDel(obb);
        return NULL;
    }
    *fstr = fnext;

    // parse OBBs
    if (_loadOBBs2(obb, fstr, fend) == 0){
        shape = _parseShape(fstr, fend);
        if (!shape){
            ferCDOBBDel(obb);
            return NULL;
        }

        obb->shape = shape;
    }

    // skip whitespace
    while (*fstr < fend && !NOT_WS(**fstr))
        ++(*fstr);
    if (*fstr >= fend)
        return NULL;

    if (**fstr != ')'){
        ferCDOBBDel(obb);
        return NULL;
    }
    ++(*fstr);

    return obb;
}

