2D vector
**********

.. c:type:: fer_vec2_t

.. c:function:: fer_vec2_t *ferVec2New(fer_real_t x, fer_real_t y)

    Allocate and initialize new vector.

.. c:function:: void ferVec2Del(fer_vec2_t *)

    Delete vector.

.. c:function:: _fer_inline fer_vec2_t *ferVec2Clone(const fer_vec2_t *v)

    Clone given fer_vec2_t. This does deep copy.

.. c:function:: _fer_inline void ferVec2Copy(fer_vec2_t *v, const fer_vec2_t *w)

    Copies w into v.

.. c:function:: _fer_inline fer_real_t ferVec2X(const fer_vec2_t *v)


.. c:function:: _fer_inline fer_real_t ferVec2Y(const fer_vec2_t *v)


.. c:function:: _fer_inline fer_real_t ferVec2Get(const fer_vec2_t *v, int d)


.. c:function:: _fer_inline void ferVec2SetX(fer_vec2_t *v, fer_real_t val)


.. c:function:: _fer_inline void ferVec2SetY(fer_vec2_t *v, fer_real_t val)


.. c:function:: _fer_inline void ferVec2Set(fer_vec2_t *v, fer_real_t x, fer_real_t y)


.. c:function:: _fer_inline void ferVec2SetCoord(fer_vec2_t *v, size_t i, fer_real_t val)


.. c:function:: _fer_inline int ferVec2Eq(const fer_vec2_t *x, const fer_vec2_t *y)


.. c:function:: _fer_inline int ferVec2NEq(const fer_vec2_t *x, const fer_vec2_t *y)


.. c:function:: _fer_inline int ferVec2Eq2(const fer_vec2_t *v, fer_real_t x, fer_real_t y)


.. c:function:: _fer_inline int ferVec2NEq2(const fer_vec2_t *v, fer_real_t x, fer_real_t y)


.. c:function:: _fer_inline fer_real_t ferVec2Dist2(const fer_vec2_t *v, const fer_vec2_t *w)

    Compute squared distance between two points represented as vectors.

.. c:function:: _fer_inline fer_real_t ferVec2Dist(const fer_vec2_t *v, const fer_vec2_t *w)

    Distance of two vectors.

.. c:function:: _fer_inline fer_real_t ferVec2Len2(const fer_vec2_t *v)

    Squared length of vector.

.. c:function:: _fer_inline fer_real_t ferVec2Len(const fer_vec2_t *v)

    Length of vector.

.. c:function:: _fer_inline void ferVec2Add(fer_vec2_t *v, const fer_vec2_t *w)

    Adds vector W to vector V (and result is stored in V):
    v = v + w

.. c:function:: _fer_inline void ferVec2Add2(fer_vec2_t *d, const fer_vec2_t *v, const fer_vec2_t *w)

    d = v + w

.. c:function:: _fer_inline void ferVec2Sub(fer_vec2_t *v, const fer_vec2_t *w)

    Substracts coordinates of vector W from vector V:
    v = v - w

.. c:function:: _fer_inline void ferVec2Sub2(fer_vec2_t *v, const fer_vec2_t *w, const fer_vec2_t *ww)

     v = w - ww

.. c:function:: _fer_inline void ferVec2AddConst(fer_vec2_t *v, fer_real_t f)


.. c:function:: _fer_inline void ferVec2AddConst2(fer_vec2_t *d, const fer_vec2_t *v, fer_real_t f)


.. c:function:: _fer_inline void ferVec2SubConst(fer_vec2_t *v, fer_real_t f)


.. c:function:: _fer_inline void ferVec2SubConst2(fer_vec2_t *d, const fer_vec2_t *v, fer_real_t f)


.. c:function:: _fer_inline void ferVec2ScaleToLen(fer_vec2_t *v, fer_real_t len)

    Scales vector to given length.

.. c:function:: _fer_inline void ferVec2Normalize(fer_vec2_t *v)

    Normalizes vector to unit vector.

.. c:function:: _fer_inline fer_real_t ferVec2Dot(const fer_vec2_t *v, const fer_vec2_t *w)

    Dot product of two vectors.

.. c:function:: _fer_inline void ferVec2MulComp(fer_vec2_t *a, const fer_vec2_t *b)

    Multiplies vectors by components:
    a.x = a.x * b.x
    a.y = a.y * b.y

.. c:function:: _fer_inline void ferVec2MulComp2(fer_vec2_t *d, const fer_vec2_t *a, const fer_vec2_t *b)

    Multiplies vectors by components:
    d.x = a.x * b.x
    d.y = a.y * b.y

.. c:function:: _fer_inline void ferVec2Scale(fer_vec2_t *v, fer_real_t k)

    Scales vector V using constant k:
     v = k * v

.. c:function:: _fer_inline fer_real_t ferVec2Area2(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)

    Returns twice area enclosed by given vectors.
    a, b, c should be named in counterclockwise order to get positive
    area and clockwise to get negative.

.. c:function:: fer_real_t ferVec2Angle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)

    Returns angle in b formed by vectors a, b, c.
    Returned value is between 0 and PI

.. c:function:: _fer_inline fer_real_t ferVec2SignedAngle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)

    Returns angle formed by points a, b, c in this order, it means, that
    is returned angle in point b and in direction from segment ab to cb.
    Returned angle is from -PI to PI. Positive angle is in
    counterclockwise direction.

.. c:function:: int ferVec2ProjectionPointOntoSegment(const fer_vec2_t *A, const fer_vec2_t *B, const fer_vec2_t *C, fer_vec2_t *X)

    This function computes projection of point C onto segment AB. Point of
    projection is returned in X.

    Returns 0 if there exists any projection, otherwise -1.

.. c:function:: int ferVec2InCircle(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c, const fer_vec2_t *d)

    Returns true if point d is in circle formed by points a, b, c.
    Vectors a, b, c must be in counterclockwise order.

.. c:function:: int ferVec2LiesOn(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c)

    Returns true if point a lies on segment formed by b c.

.. c:function:: int ferVec2InCone(const fer_vec2_t *v, const fer_vec2_t *p1, const fer_vec2_t *c, const fer_vec2_t *p2)

    Returns true, if vector v is in cone formed by p1, c, p2 (in
    counterclockwise order).

.. c:function:: _fer_inline int ferVec2IntersectProp(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c, const fer_vec2_t *d)

    Returns true if segment ab properly intersects segement cd (they share
    point interior to both segments).
    Properness of intersection means that two segmensts intersect at a point
    interior to both segments. Improper intersection (which is not covered
    by this function) means that one of end point lies somewhere on other
    segment.

.. c:function:: _fer_inline int ferVec2Intersect(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c, const fer_vec2_t *d)

    Returns true if segment ab intersects segment cd properly or improperly.

.. c:function:: int ferVec2IntersectPoint(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c, const fer_vec2_t *d, fer_vec2_t *p)

    Compute intersection point of two segments - (a, b) and (c, d).
    Returns 0 if there exists intersection, -1 otherwise.
    Intersetion point is returned in p, where p must point to already
    allocated fer_vec2_t.

.. c:function:: _fer_inline int ferVec2OnLeft(const fer_vec2_t *v, const fer_vec2_t *p1, const fer_vec2_t *p2)

    Returns true iff vector v is on left side from segment formed by p1 and
    p2 in this ordering.

.. c:function:: int ferVec2SegmentInRect(const fer_vec2_t *a, const fer_vec2_t *b, const fer_vec2_t *c, const fer_vec2_t *d, const fer_vec2_t *x, const fer_vec2_t *y, fer_vec2_t *s1, fer_vec2_t *s2)

    This function takes as arguments rectangle (a, b, c, d) and segment
    (x, y) and tries to find which part of segment (x, y) is enclosed by
    rectangle (is within rectangle). This segment is returned via (s1, s2).
    In fact, (a, b, c, d) does not have to be rectangle, but it can be any
    convex polygon formed by four sides.

    If any part of segment (x, y) does not lies within given rectangle,
    -1 is returned, 0 if segment is found.

.. c:function:: fer_real_t ferVec2AngleSameDir(const fer_vec2_t *a, const fer_vec2_t *b)

    Returns angle by which must be vector b rotated about origin to have
    same direction as vector a.
    Returned angle is in range -PI, PI.

.. c:function:: fer_real_t ferVec2AngleSegsSameDir(const fer_vec2_t *A, const fer_vec2_t *B, const fer_vec2_t *C, const fer_vec2_t *D)

    Retuns angle by which must be rotated oriented segment CD to have same
    direction as oriented segment AB. Directional vector of segment CD is
    vector (D - C) and directional vector of segment AB is vector (B - A).
    Returned angle will be between -PI and PI.

