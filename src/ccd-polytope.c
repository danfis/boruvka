/***
 * fermat
 * -------
 * Copyright (c)2010,2011 Daniel Fiser <danfis@danfis.cz>
 *
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

#include <fermat/ccd-polytope.h>
#include <fermat/alloc.h>

_fer_inline void _ferCCDPtNearestUpdate(fer_ccd_pt_t *pt, fer_ccd_pt_el_t *el)
{
    if (ferEq(pt->nearest_dist, el->dist)){
        if (el->type < pt->nearest_type){
            pt->nearest = el;
            pt->nearest_dist = el->dist;
            pt->nearest_type = el->type;
        }
    }else if (el->dist < pt->nearest_dist){
        pt->nearest = el;
        pt->nearest_dist = el->dist;
        pt->nearest_type = el->type;
    }
}

static void _ferCCDPtNearestRenew(fer_ccd_pt_t *pt)
{
    fer_ccd_pt_vertex_t *v;
    fer_ccd_pt_edge_t *e;
    fer_ccd_pt_face_t *f;

    pt->nearest_dist = FER_REAL_MAX;
    pt->nearest_type = 3;
    pt->nearest = NULL;

    FER_LIST_FOR_EACH_ENTRY(&pt->vertices, fer_ccd_pt_vertex_t, v, list){
        _ferCCDPtNearestUpdate(pt, (fer_ccd_pt_el_t *)v);
    }

    FER_LIST_FOR_EACH_ENTRY(&pt->edges, fer_ccd_pt_edge_t, e, list){
        _ferCCDPtNearestUpdate(pt, (fer_ccd_pt_el_t *)e);
    }

    FER_LIST_FOR_EACH_ENTRY(&pt->faces, fer_ccd_pt_face_t, f, list){
        _ferCCDPtNearestUpdate(pt, (fer_ccd_pt_el_t *)f);
    }
}



void ferCCDPtInit(fer_ccd_pt_t *pt)
{
    ferListInit(&pt->vertices);
    ferListInit(&pt->edges);
    ferListInit(&pt->faces);

    pt->nearest = NULL;
    pt->nearest_dist = FER_REAL_MAX;
    pt->nearest_type = 3;
}

void ferCCDPtDestroy(fer_ccd_pt_t *pt)
{
    fer_ccd_pt_face_t *f, *f2;
    fer_ccd_pt_edge_t *e, *e2;
    fer_ccd_pt_vertex_t *v, *v2;

    // first delete all faces
    FER_LIST_FOR_EACH_ENTRY_SAFE(&pt->faces, fer_ccd_pt_face_t, f, fer_ccd_pt_face_t, f2, list){
        ferCCDPtDelFace(pt, f);
    }

    // delete all edges
    FER_LIST_FOR_EACH_ENTRY_SAFE(&pt->edges, fer_ccd_pt_edge_t, e, fer_ccd_pt_edge_t, e2, list){
        ferCCDPtDelEdge(pt, e);
    }

    // delete all vertices
    FER_LIST_FOR_EACH_ENTRY_SAFE(&pt->vertices, fer_ccd_pt_vertex_t, v, fer_ccd_pt_vertex_t, v2, list){
        ferCCDPtDelVertex(pt, v);
    }
}


fer_ccd_pt_vertex_t *ferCCDPtAddVertex(fer_ccd_pt_t *pt, const fer_ccd_support_t *v)
{
    fer_ccd_pt_vertex_t *vert;

    vert = FER_ALLOC(fer_ccd_pt_vertex_t);
    vert->type = FER_CCD_PT_VERTEX;
    ferCCDSupportCopy(&vert->v, v);

    vert->dist = ferVec3Len2(&vert->v.v);
    ferVec3Copy(&vert->witness, &vert->v.v);

    ferListInit(&vert->edges);

    // add vertex to list
    ferListAppend(&pt->vertices, &vert->list);

    // update position in .nearest array
    _ferCCDPtNearestUpdate(pt, (fer_ccd_pt_el_t *)vert);

    return vert;
}

fer_ccd_pt_edge_t *ferCCDPtAddEdge(fer_ccd_pt_t *pt, fer_ccd_pt_vertex_t *v1,
                                   fer_ccd_pt_vertex_t *v2)
{
    const fer_vec3_t *a, *b;
    fer_ccd_pt_edge_t *edge;

    edge = FER_ALLOC(fer_ccd_pt_edge_t);
    edge->type = FER_CCD_PT_EDGE;
    edge->vertex[0] = v1;
    edge->vertex[1] = v2;
    edge->faces[0] = edge->faces[1] = NULL;

    a = &edge->vertex[0]->v.v;
    b = &edge->vertex[1]->v.v;
    edge->dist = ferVec3PointSegmentDist2(fer_vec3_origin, a, b, &edge->witness);

    ferListAppend(&edge->vertex[0]->edges, &edge->vertex_list[0]);
    ferListAppend(&edge->vertex[1]->edges, &edge->vertex_list[1]);

    ferListAppend(&pt->edges, &edge->list);

    // update position in .nearest array
    _ferCCDPtNearestUpdate(pt, (fer_ccd_pt_el_t *)edge);

    return edge;
}

fer_ccd_pt_face_t *ferCCDPtAddFace(fer_ccd_pt_t *pt, fer_ccd_pt_edge_t *e1,
                                   fer_ccd_pt_edge_t *e2,
                                   fer_ccd_pt_edge_t *e3)
{
    const fer_vec3_t *a, *b, *c;
    fer_ccd_pt_face_t *face;
    fer_ccd_pt_edge_t *e;
    size_t i;

    face = FER_ALLOC(fer_ccd_pt_face_t);
    face->type = FER_CCD_PT_FACE;
    face->edge[0] = e1;
    face->edge[1] = e2;
    face->edge[2] = e3;

    // obtain triplet of vertices
    a = &face->edge[0]->vertex[0]->v.v;
    b = &face->edge[0]->vertex[1]->v.v;
    e = face->edge[1];
    if (e->vertex[0] != face->edge[0]->vertex[0]
            && e->vertex[0] != face->edge[0]->vertex[1]){
        c = &e->vertex[0]->v.v;
    }else{
        c = &e->vertex[1]->v.v;
    }
    face->dist = ferVec3PointTriDist2(fer_vec3_origin, a, b, c, &face->witness);


    for (i = 0; i < 3; i++){
        if (face->edge[i]->faces[0] == NULL){
            face->edge[i]->faces[0] = face;
        }else{
            face->edge[i]->faces[1] = face;
        }
    }

    ferListAppend(&pt->faces, &face->list);

    // update position in .nearest array
    _ferCCDPtNearestUpdate(pt, (fer_ccd_pt_el_t *)face);

    return face;
}


void ferCCDPtRecomputeDistances(fer_ccd_pt_t *pt)
{
    fer_ccd_pt_vertex_t *v;
    fer_ccd_pt_edge_t *e;
    fer_ccd_pt_face_t *f;
    const fer_vec3_t *a, *b, *c;
    fer_real_t dist;

    FER_LIST_FOR_EACH_ENTRY(&pt->vertices, fer_ccd_pt_vertex_t, v, list){
        dist = ferVec3Len2(&v->v.v);
        v->dist = dist;
        ferVec3Copy(&v->witness, &v->v.v);
    }

    FER_LIST_FOR_EACH_ENTRY(&pt->edges, fer_ccd_pt_edge_t, e, list){
        a = &e->vertex[0]->v.v;
        b = &e->vertex[1]->v.v;
        dist = ferVec3PointSegmentDist2(fer_vec3_origin, a, b, &e->witness);
        e->dist = dist;
    }

    FER_LIST_FOR_EACH_ENTRY(&pt->faces, fer_ccd_pt_face_t, f, list){
        // obtain triplet of vertices
        a = &f->edge[0]->vertex[0]->v.v;
        b = &f->edge[0]->vertex[1]->v.v;
        e = f->edge[1];
        if (e->vertex[0] != f->edge[0]->vertex[0]
                && e->vertex[0] != f->edge[0]->vertex[1]){
            c = &e->vertex[0]->v.v;
        }else{
            c = &e->vertex[1]->v.v;
        }

        dist = ferVec3PointTriDist2(fer_vec3_origin, a, b, c, &f->witness);
        f->dist = dist;
    }
}

fer_ccd_pt_el_t *ferCCDPtNearest(fer_ccd_pt_t *pt)
{
    if (!pt->nearest){
        _ferCCDPtNearestRenew(pt);
    }
    return pt->nearest;
}


void ferCCDPtDumpSVT(fer_ccd_pt_t *pt, const char *fn)
{
    FILE *fout;

    fout = fopen(fn, "a");
    if (fout == NULL)
        return;

    ferCCDPtDumpSVT2(pt, fout);

    fclose(fout);
}

void ferCCDPtDumpSVT2(fer_ccd_pt_t *pt, FILE *fout)
{
    fer_ccd_pt_vertex_t *v, *a, *b, *c;
    fer_ccd_pt_edge_t *e;
    fer_ccd_pt_face_t *f;
    size_t i;

    fprintf(fout, "-----\n");

    fprintf(fout, "Points:\n");
    i = 0;
    FER_LIST_FOR_EACH_ENTRY(&pt->vertices, fer_ccd_pt_vertex_t, v, list){
        v->id = i++;
        fprintf(fout, "%lf %lf %lf\n",
                ferVec3X(&v->v.v), ferVec3Y(&v->v.v), ferVec3Z(&v->v.v));
    }

    fprintf(fout, "Edges:\n");
    FER_LIST_FOR_EACH_ENTRY(&pt->edges, fer_ccd_pt_edge_t, e, list){
        fprintf(fout, "%d %d\n", e->vertex[0]->id, e->vertex[1]->id);
    }

    fprintf(fout, "Faces:\n");
    FER_LIST_FOR_EACH_ENTRY(&pt->faces, fer_ccd_pt_face_t, f, list){
        a = f->edge[0]->vertex[0];
        b = f->edge[0]->vertex[1];
        c = f->edge[1]->vertex[0];
        if (c == a || c == b){
            c = f->edge[1]->vertex[1];
        }
        fprintf(fout, "%d %d %d\n", a->id, b->id, c->id);
    }
}
