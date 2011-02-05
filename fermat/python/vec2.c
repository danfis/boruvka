/***
 * fermat
 * -------
 * Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
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

#include "common.h"
#include "vec2.h"

static int vec2ObjInit(py_vec2 *self, PyObject *args, PyObject *kwds);
static PyObject *vec2AsStr(py_vec2 *self);

/** Returns x, y, z coordinates */
static PyObject *vec2GetX(py_vec2 *self, void *coord);
static PyObject *vec2GetY(py_vec2 *self, void *coord);
/** Sets x, y, z coordinates */
static int vec2SetX(py_vec2 *self, PyObject *val, void *coord);
static int vec2SetY(py_vec2 *self, PyObject *val, void *coord);
/** Compare function */
static PyObject *vec2Cmp(PyObject *a, PyObject *b, int op);

static PyObject *vec2Copy(py_vec2 *self);
static PyObject *vec2Len2(py_vec2 *self);
static PyObject *vec2Len(py_vec2 *self);
static PyObject *vec2Dist2(py_vec2 *self, py_vec2 *o);
static PyObject *vec2Dist(py_vec2 *self, py_vec2 *o);
static PyObject *vec2ScaleToLen(py_vec2 *self, PyObject *o);
static PyObject *vec2ScaledToLen(py_vec2 *self, PyObject *o);
static PyObject *vec2Normalize(py_vec2 *self);
static PyObject *vec2Normalized(py_vec2 *self);
static PyObject *vec2Area2(py_vec2 *self, PyObject *args);
static PyObject *vec2Angle(py_vec2 *self, PyObject *args);
static PyObject *vec2SignedAngle(py_vec2 *self, PyObject *args);
static PyObject *vec2ProjectionOntoSegment(py_vec2 *self, PyObject *args);
static PyObject *vec2InCircle(py_vec2 *self, PyObject *args);
static PyObject *vec2LiesOn(py_vec2 *self, PyObject *args);
static PyObject *vec2Collinear(py_vec2 *self, PyObject *args);
static PyObject *vec2InCone(py_vec2 *self, PyObject *args);
static PyObject *vec2IntersectProp(py_vec2 *self, PyObject *args);
static PyObject *vec2Intersect(py_vec2 *self, PyObject *args);
static PyObject *vec2IntersectPoint(py_vec2 *self, PyObject *args);
static PyObject *vec2OnLeft(py_vec2 *self, PyObject *args);
static PyObject *vec2SegmentInRect(py_vec2 *self, PyObject *args);

/** Number protocol functions */
static PyObject *vec2Add(py_vec2 *self, PyObject *o);
static PyObject *vec2Sub(py_vec2 *self, PyObject *o);
static PyObject *vec2Mul(py_vec2 *self, PyObject *o);
static PyObject *vec2Neg(py_vec2 *self);
static PyObject *vec2Div(py_vec2 *self, PyObject *o);
static PyObject *vec2AddIn(py_vec2 *self, PyObject *o);
static PyObject *vec2SubIn(py_vec2 *self, PyObject *o);
static PyObject *vec2MulIn(py_vec2 *self, PyObject *o);
static PyObject *vec2DivIn(py_vec2 *self, PyObject *o);

/** Sequence protocol functions */
static Py_ssize_t vec2SeqSize(py_vec2 *self);
static PyObject *vec2SeqGet(py_vec2 *self, Py_ssize_t i);
static int vec2SeqSet(py_vec2 *self, Py_ssize_t i, PyObject *val);

static PyGetSetDef py_vec2_getset[] = {
    {"x", (getter)vec2GetX, (setter)vec2SetX, "x coordinate", NULL},
    {"y", (getter)vec2GetY, (setter)vec2SetY, "y coordinate", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef py_vec2_methods[] = {
    { "copy", (PyCFunction)vec2Copy, METH_NOARGS,
      "Returns deep copy of Vec2." },
    { "len2", (PyCFunction)vec2Len2, METH_NOARGS,
      "len2() -> float\n"
      "Returns squared length of vector" },
    { "len", (PyCFunction)vec2Len, METH_NOARGS,
      "len() -> float\n"
      "Returns length of vector" },
    { "dist2", (PyCFunction)vec2Dist2, METH_O,
      "dist2(v : Vec2) -> float\n"
      "Squared distance of two vectors" },
    { "dist", (PyCFunction)vec2Dist, METH_O,
      "dist(v : Vec2) -> float\n"
      "Distance of two vectors" },
    { "scaleToLen", (PyCFunction)vec2ScaleToLen, METH_O,
      "scaleToLen(len : float) -> self\n"
      "Scale vector to given length" },
    { "scaledToLen", (PyCFunction)vec2ScaledToLen, METH_O,
      "scaledToLen(len : float) -> Vec2\n"
      "Returns vector scaled to given length" },
    { "normalize", (PyCFunction)vec2Normalize, METH_NOARGS,
      "normalize() -> self\n"
      "Normalize vector in-place" },
    { "normalized", (PyCFunction)vec2Normalized, METH_NOARGS,
      "normalized() -> Vec2\n"
      "Returns new normalized vector" },
    { "angle", (PyCFunction)vec2Angle, METH_VARARGS | METH_STATIC,
      "angle(a : Vec2, b : Vec2, c : Vec2) -> float\n"
      "Returns angle formed in b between points a and c. "
      "Returned value is between 0 and PI." },
    { "area2", (PyCFunction)vec2Area2, METH_VARARGS | METH_STATIC,
      "area2(a : Vec2, b : Vec2, c : Vec2) -> float\n"
      "Returns twice an area encolsed by triplet of points." },
    { "signedAngle", (PyCFunction)vec2SignedAngle, METH_VARARGS | METH_STATIC,
      "signedAngle(a : Vec2, b : Vec2, c : Vec2) -> float\n"
      "Returns signed angle (-PI, PI) in b. Positive angle is counterclockwise." },
    { "projectionOntoSegment", (PyCFunction)vec2ProjectionOntoSegment, METH_VARARGS,
      "projectionOntoSegment(a : Vec2, b : Vec2) -> Vec2\n"
      "Computes projection of point onto segment a, b.\n"
      "Returns projected point or None." },
    { "inCircle", (PyCFunction)vec2InCircle, METH_VARARGS,
      "inCircle(a : Vec2, b : Vec2, c : Vec2) -> bool\n"
      "Returns true if point lies in circle.\n" },
    { "liesOn", (PyCFunction)vec2LiesOn, METH_VARARGS,
      "liesOn(a : Vec2, b : Vec2) -> bool\n"
      "Returns true if point lies on segment ab.\n" },
    { "collinear", (PyCFunction)vec2Collinear, METH_VARARGS,
      "collinear(a : Vec2, b : Vec2) -> bool\n"
      "Returns true if point is collinear with a and b.\n" },
    { "inCone", (PyCFunction)vec2InCone, METH_VARARGS,
      "inCone(p1 : Vec2, c : Vec2, p2 : Vec2) -> bool\n"
      "Returns true if point lies in cone p1-c-p2.\n" },
    { "intersectProp", (PyCFunction)vec2IntersectProp, METH_VARARGS | METH_STATIC,
      "signedAngle(a : Vec2, b : Vec2, c : Vec2, d : Vec2) -> bool\n"
      "Returns true is segment ab properly intersects segment cd (they\n"
      "share point interior to both segments)." },
    { "intersect", (PyCFunction)vec2Intersect, METH_VARARGS | METH_STATIC,
      "signedAngle(a : Vec2, b : Vec2, c : Vec2, d : Vec2) -> bool\n"
      "Returns true is segment ab intersects segment cd." },
    { "intersectPoint", (PyCFunction)vec2IntersectPoint, METH_VARARGS | METH_STATIC,
      "signedAngle(a : Vec2, b : Vec2, c : Vec2, d : Vec2) -> Vec2\n"
      "Returns intersection point of two segments ab and cd." },
    { "onLeft", (PyCFunction)vec2OnLeft, METH_VARARGS,
      "onLeft(a : Vec2, b : Vec2) -> bool\n"
      "Returns true if point is on left of segment ab." },
    { "segmentInRect", (PyCFunction)vec2SegmentInRect, METH_VARARGS | METH_STATIC,
      "segmentInRect(a : Vec2, b : Vec2, c : Vec2, d : Vec2, x : Vec2, y : Vec2) -> (Vec2, Vec2)\n"
      "Computes intersection of segment xy with rectangle abcd." },
    { NULL }
};

static PyNumberMethods py_vec2_num = {
    (binaryfunc)vec2Add, /* binaryfunc nb_add */
    (binaryfunc)vec2Sub, /* binaryfunc nb_subtract */
    (binaryfunc)vec2Mul, /* binaryfunc nb_multiply */
    0,                   /* binaryfunc nb_remainder */
    0,                   /* binaryfunc nb_divmod */
    0,                   /* ternaryfunc nb_power */
    (unaryfunc)vec2Neg,  /* unaryfunc nb_negative */
    0,                   /* unaryfunc nb_positive */
    0,                   /* unaryfunc nb_absolute */
    0,                   /* inquiry nb_bool */
    0,                   /* unaryfunc nb_invert */
    0,                   /* binaryfunc nb_lshift */
    0,                   /* binaryfunc nb_rshift */
    0,                   /* binaryfunc nb_and */
    0,                   /* binaryfunc nb_xor */
    0,                   /* binaryfunc nb_or */
    0,                   /* unaryfunc nb_int */
    0,                   /* void *nb_reserved */
    0,                   /* unaryfunc nb_float */

    (binaryfunc)vec2AddIn, /* binaryfunc nb_inplace_add */
    (binaryfunc)vec2SubIn, /* binaryfunc nb_inplace_subtract */
    (binaryfunc)vec2MulIn, /* binaryfunc nb_inplace_multiply */
    0,                     /* binaryfunc nb_inplace_remainder */
    0,                     /* ternaryfunc nb_inplace_power */
    0,                     /* binaryfunc nb_inplace_lshift */
    0,                     /* binaryfunc nb_inplace_rshift */
    0,                     /* binaryfunc nb_inplace_and */
    0,                     /* binaryfunc nb_inplace_xor */
    0,                     /* binaryfunc nb_inplace_or */

    (binaryfunc)vec2Div,   /* binaryfunc nb_floor_divide */
    (binaryfunc)vec2Div,   /* binaryfunc nb_true_divide */
    (binaryfunc)vec2DivIn, /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)vec2DivIn, /* binaryfunc nb_inplace_true_divide */

    0                      /* unaryfunc nb_index */
};

static PySequenceMethods py_vec2_seq = {
    (lenfunc)vec2SeqSize,        /* sq_length */
    0,                           /* sq_concat */
    0,                           /* sq_repeat */
    (ssizeargfunc)vec2SeqGet,    /* sq_item */
    0,                           /* was_sq_slice */
    (ssizeobjargproc)vec2SeqSet, /* sq_ass_item */
    0,                           /* was_sq_ass_slice */
    0,                           /* sq_contains */

    0,                           /* sq_inplace_concat */
    0,                           /* sq_inplace_repeat */
};


PyTypeObject py_vec2_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.Vec2",             /* tp_name */
    sizeof(py_vec2),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    __ferDefaultDealloc,       /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)vec2AsStr,       /* tp_repr */
    &py_vec2_num,              /* tp_as_number */
    &py_vec2_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)vec2AsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "3D vector",               /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    vec2Cmp,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py_vec2_methods,           /* tp_methods */
    0,                         /* tp_members */
    py_vec2_getset,            /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)vec2ObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void vec2Init(PyObject *module)
{
    py_vec2_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_vec2_type) < 0)
        return;

    Py_INCREF(&py_vec2_type);
    PyModule_AddObject(module, "Vec2", (PyObject *)&py_vec2_type);
}


static int vec2ObjInit(py_vec2 *self, PyObject *_args, PyObject *kwds)
{
    fer_real_t v[3];
    Py_ssize_t i, len = 0;
    PyObject *args = NULL, *val;

    v[0] = FER_ZERO;
    v[1] = FER_ZERO;

    if (PySequence_Check(_args)){
        len = PySequence_Size(_args);
        args = PySequence_Fast(_args, "error");
    }

    for (i = 0; i < len && i < 2; i++){
        val = PySequence_Fast_GET_ITEM(args, i);
        if (PyFloat_Check(val)){
            v[i] = PyFloat_AsDouble(val);
        }else if (PyNumber_Check(val)){
            v[i] = numberAsReal(val);
        }
    }

    if (args){
        Py_DECREF(args);
    }

    ferVec2Set(&self->v, v[0], v[1]);
    return 0;
}

static PyObject *vec2AsStr(py_vec2 *self)
{
    char str[100];
    snprintf(str, 100, "<Vec2: %f %f>", 
             ferVec2X(&self->v), ferVec2Y(&self->v));
    return PyUnicode_FromString(str);
}

static PyObject *vec2GetX(py_vec2 *self, void *coord)
{
    return PyFloat_FromDouble(ferVec2X(&self->v));
}
static PyObject *vec2GetY(py_vec2 *self, void *coord)
{
    return PyFloat_FromDouble(ferVec2Y(&self->v));
}

static int vec2SetX(py_vec2 *self, PyObject *val, void *coord)
{
    ferVec2SetX(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int vec2SetY(py_vec2 *self, PyObject *val, void *coord)
{
    ferVec2SetY(&self->v, PyFloat_AsDouble(val));
    return 0;
}


static PyObject *vec2Cmp(PyObject *a, PyObject *b, int op)
{
    py_vec2 *v1, *v2;

    if (!PyObject_TypeCheck(a, &py_vec2_type)
            || !PyObject_TypeCheck(b, &py_vec2_type)){
        return Py_NotImplemented;
    }

    v1 = (py_vec2 *)a;
    v2 = (py_vec2 *)b;
    if (op == Py_EQ){
        return ferVec2Eq(&v1->v, &v2->v) ? Py_True : Py_False;
    }else if (op == Py_NE){
        return ferVec2NEq(&v1->v, &v2->v) ? Py_True : Py_False;
    }
    return Py_NotImplemented;
}

static PyObject *vec2Copy(py_vec2 *self)
{
    py_vec2 *v = PyObject_New(py_vec2, &py_vec2_type);
    ferVec2Copy(&v->v, &self->v);
    return (PyObject *)v;
}

static PyObject *vec2Len2(py_vec2 *self)
{
    return PyFloat_FromDouble(ferVec2Len2(&self->v));
}
static PyObject *vec2Len(py_vec2 *self)
{
    return PyFloat_FromDouble(ferVec2Len(&self->v));
}

static PyObject *vec2Dist2(py_vec2 *self, py_vec2 *o)
{
    fer_real_t d;

    CHECK_VEC2(o)

    d = ferVec2Dist2(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}
static PyObject *vec2Dist(py_vec2 *self, py_vec2 *o)
{
    fer_real_t d;

    CHECK_VEC2(o)

    d = ferVec2Dist(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}

static PyObject *vec2ScaleToLen(py_vec2 *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVec2ScaleToLen(&self->v, num);
    Py_INCREF(self);
    return (PyObject *)self;
}
static PyObject *vec2ScaledToLen(py_vec2 *self, PyObject *o)
{
    fer_real_t num;
    py_vec2 *v;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec2, &py_vec2_type);
    ferVec2Copy(&v->v, &self->v);
    ferVec2ScaleToLen(&v->v, num);
    return (PyObject *)v;
}

static PyObject *vec2Normalize(py_vec2 *self)
{
    ferVec2Normalize(&self->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *vec2Normalized(py_vec2 *self)
{
    py_vec2 *v;
    v = PyObject_New(py_vec2, &py_vec2_type);
    ferVec2Copy(&v->v, &self->v);
    ferVec2Normalize(&v->v);
    return (PyObject *)v;
}


#define VEC2_AB \
    if (!PyArg_ParseTuple(args, "OO", &a, &b) \
            || !PyObject_TypeCheck(a, &py_vec2_type) \
            || !PyObject_TypeCheck(b, &py_vec2_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected two Vec2s"); \
        return NULL; \
    }

#define VEC2_ABC \
    if (!PyArg_ParseTuple(args, "OOO", &a, &b, &c) \
            || !PyObject_TypeCheck(a, &py_vec2_type) \
            || !PyObject_TypeCheck(b, &py_vec2_type) \
            || !PyObject_TypeCheck(c, &py_vec2_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected three Vec2s"); \
        return NULL; \
    }

#define VEC2_ABCD \
    if (!PyArg_ParseTuple(args, "OOOO", &a, &b, &c, &d) \
            || !PyObject_TypeCheck(a, &py_vec2_type) \
            || !PyObject_TypeCheck(b, &py_vec2_type) \
            || !PyObject_TypeCheck(c, &py_vec2_type) \
            || !PyObject_TypeCheck(d, &py_vec2_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected four Vec2s"); \
        return NULL; \
    }


#define VEC2_ABC_REAL(func) \
    py_vec2 *a, *b, *c; \
    fer_real_t r; \
    VEC2_ABC \
    r = func(&a->v, &b->v, &c->v); \
    return PyFloat_FromDouble(r)


static PyObject *vec2Angle(py_vec2 *self, PyObject *args)
{
    VEC2_ABC_REAL(ferVec2Angle);
}

static PyObject *vec2Area2(py_vec2 *self, PyObject *args)
{
    VEC2_ABC_REAL(ferVec2Area2);
}

static PyObject *vec2SignedAngle(py_vec2 *self, PyObject *args)
{
    VEC2_ABC_REAL(ferVec2SignedAngle);
}

static PyObject *vec2ProjectionOntoSegment(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *x;
    int res;

    VEC2_AB

    x = PyObject_New(py_vec2, &py_vec2_type);
    res = ferVec2ProjectionPointOntoSegment(&a->v, &b->v, &self->v, &x->v);
    if (res == 0){
        return (PyObject *)x;
    }else{
        PyObject_Del(x);
        Py_RETURN_NONE;
    }
}

static PyObject *vec2InCircle(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *c;

    VEC2_ABC

    if (ferVec2InCircle(&a->v, &b->v, &c->v, &self->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2LiesOn(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b;

    VEC2_AB

    if (ferVec2LiesOn(&self->v, &a->v, &b->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2Collinear(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b;

    VEC2_AB

    if (ferVec2Collinear(&self->v, &a->v, &b->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2InCone(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *c;

    VEC2_ABC

    if (ferVec2InCone(&self->v, &a->v, &b->v, &c->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2IntersectProp(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *c, *d;
    VEC2_ABCD

    if (ferVec2IntersectProp(&a->v, &b->v, &c->v, &d->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2Intersect(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *c, *d;
    VEC2_ABCD

    if (ferVec2Intersect(&a->v, &b->v, &c->v, &d->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2IntersectPoint(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *c, *d, *x;
    VEC2_ABCD

    x = PyObject_New(py_vec2, &py_vec2_type);
    if (ferVec2IntersectPoint(&a->v, &b->v, &c->v, &d->v, &x->v) == 0){
        return (PyObject *)x;
    }else{
        PyObject_Del(x);
        Py_RETURN_NONE;
    }
}

static PyObject *vec2OnLeft(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b;

    VEC2_AB

    if (ferVec2OnLeft(&self->v, &a->v, &b->v)){
        Py_RETURN_TRUE;
    }else{
        Py_RETURN_FALSE;
    }
}

static PyObject *vec2SegmentInRect(py_vec2 *self, PyObject *args)
{
    py_vec2 *a, *b, *c, *d, *x, *y, *s1, *s2;
    PyObject *tuple;

    if (!PyArg_ParseTuple(args, "OOOOOO", &a, &b, &c, &d, &x, &y)
            || !PyObject_TypeCheck(a, &py_vec2_type)
            || !PyObject_TypeCheck(b, &py_vec2_type)
            || !PyObject_TypeCheck(c, &py_vec2_type)
            || !PyObject_TypeCheck(d, &py_vec2_type)
            || !PyObject_TypeCheck(x, &py_vec2_type)
            || !PyObject_TypeCheck(y, &py_vec2_type)){
        PyErr_SetString(PyExc_TypeError, "Expected six Vec2s");
        return NULL;
    }

    s1 = PyObject_New(py_vec2, &py_vec2_type);
    s2 = PyObject_New(py_vec2, &py_vec2_type);

    if (ferVec2SegmentInRect(&a->v, &b->v, &c->v, &d->v, &x->v, &y->v, &s1->v, &s2->v) == 0){
        tuple = PyTuple_New(2);
        PyTuple_SetItem(tuple, 0, (PyObject *)s1);
        PyTuple_SetItem(tuple, 1, (PyObject *)s2);
        return tuple;
    }else{
        PyObject_Del(s1);
        PyObject_Del(s2);
        Py_RETURN_NONE;
    }
}









static PyObject *vec2Add(py_vec2 *self, PyObject *o)
{
    py_vec2 *v;
    fer_real_t f;
    PyObject *tmp;

    if (PyNumber_Check((PyObject *)self)){
        tmp = (PyObject *)self;
        self = (py_vec2 *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_vec2_type)){
        v = PyObject_New(py_vec2, &py_vec2_type);
        ferVec2Add2(&v->v, &self->v, &((py_vec2 *)o)->v);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec2, &py_vec2_type);
        f = numberAsReal(o);

        ferVec2AddConst2(&v->v, &self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec2"); \
        return NULL;
    }

    return (PyObject *)v;
}

static PyObject *vec2Sub(py_vec2 *self, PyObject *o)
{
    py_vec2 *v;
    fer_real_t f;

    if (PyNumber_Check((PyObject *)self)){
        PyErr_SetString(PyExc_TypeError, "Invalid operation");
        return NULL;
    }

    if (PyObject_TypeCheck(o, &py_vec2_type)){
        v = PyObject_New(py_vec2, &py_vec2_type);
        ferVec2Sub2(&v->v, &self->v, &((py_vec2 *)o)->v);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec2, &py_vec2_type);
        f = numberAsReal(o);

        ferVec2SubConst2(&v->v, &self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec2"); \
        return NULL;
    }

    return (PyObject *)v;
}

static PyObject *vec2Mul(py_vec2 *self, PyObject *o)
{
    py_vec2 *v;
    fer_real_t num;
    PyObject *tmp;

    if (PyNumber_Check((PyObject *)self)){
        tmp = (PyObject *)self;
        self = (py_vec2 *)o;
        o = tmp;
    }

    if (PyObject_TypeCheck(o, &py_vec2_type)){
        num = ferVec2Dot(&self->v, &((py_vec2 *)o)->v);
        return PyFloat_FromDouble(num);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec2, &py_vec2_type);
        num = numberAsReal(o);

        ferVec2Copy(&v->v, &self->v);
        ferVec2Scale(&v->v, num);
        return (PyObject *)v;
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected Vec2 or float number");
        return NULL;
    }
}

static PyObject *vec2Neg(py_vec2 *self)
{
    py_vec2 *v;

    v = PyObject_New(py_vec2, &py_vec2_type);
    ferVec2Copy(&v->v, &self->v);
    ferVec2Scale(&v->v, -FER_ONE);
    return (PyObject *)v;
}

static PyObject *vec2Div(py_vec2 *self, PyObject *o)
{
    py_vec2 *v;
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec2, &py_vec2_type);
    ferVec2Copy(&v->v, &self->v);
    ferVec2Scale(&v->v, FER_ONE / num);
    return (PyObject *)v;
}

static PyObject *vec2AddIn(py_vec2 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec2_type)){
        ferVec2Add(&self->v, &((py_vec2 *)o)->v);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferVec2AddConst(&self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec2"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec2SubIn(py_vec2 *self, PyObject *o)
{
    fer_real_t f;

    if (PyObject_TypeCheck(o, &py_vec2_type)){
        ferVec2Sub(&self->v, &((py_vec2 *)o)->v);
    }else if (PyNumber_Check(o)){
        f = numberAsReal(o);
        ferVec2SubConst(&self->v, f);
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected float or Vec2"); \
        return NULL;
    }

    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec2MulIn(py_vec2 *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVec2Scale(&self->v, num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec2DivIn(py_vec2 *self, PyObject *o)
{
    fer_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    ferVec2Scale(&self->v, FER_ONE / num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


static Py_ssize_t vec2SeqSize(py_vec2 *self)
{
    return 2;
}

static PyObject *vec2SeqGet(py_vec2 *self, Py_ssize_t i)
{
    if (i >= 2){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(ferVec2Get(&self->v, i));
}

static int vec2SeqSet(py_vec2 *self, Py_ssize_t i, PyObject *val)
{
    if (i >= 2){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    ferVec2SetCoord(&self->v, i, numberAsReal(val));
    return 0;
}


