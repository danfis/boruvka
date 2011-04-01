from core cimport *

cdef extern from "../fermat/vec2.h":
    ctypedef int fer_vec2_t

    void ferVec2Set(fer_vec2_t *v, fer_real_t x, fer_real_t y)
    fer_real_t ferVec2X(fer_vec2_t *v)
    fer_real_t ferVec2Y(fer_vec2_t *v)
    void ferVec2SetX(fer_vec2_t *v, fer_real_t val)
    void ferVec2SetY(fer_vec2_t *v, fer_real_t val)

    fer_real_t ferVec2Dist2(fer_vec2_t *v, fer_vec2_t *w)
    fer_real_t ferVec2Dist(fer_vec2_t *v, fer_vec2_t *w)
    fer_real_t ferVec2Len2(fer_vec2_t *v)
    fer_real_t ferVec2Len(fer_vec2_t *v)

    void ferVec2Add(fer_vec2_t *v, fer_vec2_t *w)
    void ferVec2Add2(fer_vec2_t *d, fer_vec2_t *v, fer_vec2_t *w)
    void ferVec2Sub(fer_vec2_t *v, fer_vec2_t *w)
    void ferVec2Sub2(fer_vec2_t *d, fer_vec2_t *v, fer_vec2_t *w)

    void ferVec2AddConst(fer_vec2_t *v, fer_real_t f)
    void ferVec2AddConst2(fer_vec2_t *d, fer_vec2_t *v, fer_real_t f)
    void ferVec2SubConst(fer_vec2_t *v, fer_real_t f)
    void ferVec2SubConst2(fer_vec2_t *d, fer_vec2_t *v, fer_real_t f)

    void ferVec2ScaleToLen(fer_vec2_t *v, fer_real_t len)
    void ferVec2Normalize(fer_vec2_t *v)

    fer_real_t ferVec2Dot(fer_vec2_t *v, fer_vec2_t *w)
    void ferVec2MulComp(fer_vec2_t *a, fer_vec2_t *b)
    void ferVec2MulComp2(fer_vec2_t *d, fer_vec2_t *a, fer_vec2_t *b)

    void ferVec2Scale(fer_vec2_t *v, fer_real_t k)

    fer_real_t ferVec2Area2(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c)
    fer_real_t ferVec2Angle(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c)
    fer_real_t ferVec2SignedAngle(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c)

    int ferVec2ProjectionPointOntoSegment(fer_vec2_t *A, fer_vec2_t *B, fer_vec2_t *C, fer_vec2_t *X)

    # TODO
    int ferVec2InCircle(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c, fer_vec2_t *d)
    int ferVec2LiesOn(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c)
    int ferVec2Collinear(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c)
    int ferVec2InCone(fer_vec2_t *v, fer_vec2_t *p1, fer_vec2_t *c, fer_vec2_t *p2)
    int ferVec2IntersectProp(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c, fer_vec2_t *d)
    int ferVec2Intersect(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c, fer_vec2_t *d)
    int ferVec2IntersectPoint(fer_vec2_t *a, fer_vec2_t *b, fer_vec2_t *c, fer_vec2_t *d, fer_vec2_t *p)
    int ferVec2OnLeft(fer_vec2_t *v, fer_vec2_t *p1, fer_vec2_t *p2)
    int ferVec2SegmentInRect(fer_vec2_t *a, fer_vec2_t *b, \
                             fer_vec2_t *c, fer_vec2_t *d, \
                             fer_vec2_t *x, fer_vec2_t *y, \
                             fer_vec2_t *s1, fer_vec2_t *s2)
    fer_real_t ferVec2AngleSameDir(fer_vec2_t *a, fer_vec2_t *b)
    fer_real_t ferVec2AngleSegsSameDir(fer_vec2_t *A, fer_vec2_t *B, fer_vec2_t *C, fer_vec2_t *D)
