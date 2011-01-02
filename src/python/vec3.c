#include "common.h"
#include "vec3.h"

static int vec3ObjInit(py_vec3 *self, PyObject *args, PyObject *kwds);
static PyObject *vec3AsStr(py_vec3 *self);

/** Returns x, y, z coordinates */
static PyObject *vec3GetX(py_vec3 *self, void *coord);
static PyObject *vec3GetY(py_vec3 *self, void *coord);
static PyObject *vec3GetZ(py_vec3 *self, void *coord);
/** Sets x, y, z coordinates */
static int vec3SetX(py_vec3 *self, PyObject *val, void *coord);
static int vec3SetY(py_vec3 *self, PyObject *val, void *coord);
static int vec3SetZ(py_vec3 *self, PyObject *val, void *coord);
/** Compare function */
static PyObject *vec3Cmp(PyObject *a, PyObject *b, int op);

static PyObject *vec3Copy(py_vec3 *self);
static PyObject *vec3Len2(py_vec3 *self);
static PyObject *vec3Len(py_vec3 *self);
static PyObject *vec3Dist2(py_vec3 *self, py_vec3 *o);
static PyObject *vec3Dist(py_vec3 *self, py_vec3 *o);
static PyObject *vec3ScaleToLen(py_vec3 *self, PyObject *o);
static PyObject *vec3ScaledToLen(py_vec3 *self, PyObject *o);
static PyObject *vec3Normalize(py_vec3 *self);
static PyObject *vec3Normalized(py_vec3 *self);
static PyObject *vec3Cross(py_vec3 *self, py_vec3 *o);
static PyObject *vec3SegmentDist2(py_vec3 *self, PyObject *args);
static PyObject *vec3TriDist2(py_vec3 *self, PyObject *args);
static PyObject *vec3InTri(py_vec3 *self, PyObject *args);
static PyObject *vec3Angle(py_vec3 *self, PyObject *args);
static PyObject *vec3DihedralAngle(py_vec3 *self, PyObject *args);
static PyObject *vec3ProjToPlane(py_vec3 *self, PyObject *args);
static PyObject *vec3ProjToPlane2(py_vec3 *self, PyObject *args);
static PyObject *vec3TriArea2(py_vec3 *self, PyObject *args);

/** Number protocol functions */
static PyObject *vec3Add(py_vec3 *self, PyObject *o);
static PyObject *vec3Sub(py_vec3 *self, PyObject *o);
static PyObject *vec3Mul(py_vec3 *self, PyObject *o);
static PyObject *vec3Neg(py_vec3 *self);
static PyObject *vec3Div(py_vec3 *self, PyObject *o);
static PyObject *vec3AddIn(py_vec3 *self, py_vec3 *o);
static PyObject *vec3SubIn(py_vec3 *self, py_vec3 *o);
static PyObject *vec3MulIn(py_vec3 *self, PyObject *o);
static PyObject *vec3DivIn(py_vec3 *self, PyObject *o);

/** Sequence protocol functions */
static Py_ssize_t vec3SeqSize(py_vec3 *self);
static PyObject *vec3SeqGet(py_vec3 *self, Py_ssize_t i);
static int vec3SeqSet(py_vec3 *self, Py_ssize_t i, PyObject *val);

static PyGetSetDef py_vec3_getset[] = {
    {"x", (getter)vec3GetX, (setter)vec3SetX, "x coordinate", NULL},
    {"y", (getter)vec3GetY, (setter)vec3SetY, "y coordinate", NULL},
    {"z", (getter)vec3GetZ, (setter)vec3SetZ, "z coordinate", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef py_vec3_methods[] = {
    { "copy", (PyCFunction)vec3Copy, METH_NOARGS,
      "Returns deep copy of Vec3." },
    { "len2", (PyCFunction)vec3Len2, METH_NOARGS,
      "len2() -> float\n"
      "Returns squared length of vector" },
    { "len", (PyCFunction)vec3Len, METH_NOARGS,
      "len() -> float\n"
      "Returns length of vector" },
    { "dist2", (PyCFunction)vec3Dist2, METH_O,
      "dist2(v : Vec3) -> float\n"
      "Squared distance of two vectors" },
    { "dist", (PyCFunction)vec3Dist, METH_O,
      "dist(v : Vec3) -> float\n"
      "Distance of two vectors" },
    { "scaleToLen", (PyCFunction)vec3ScaleToLen, METH_O,
      "scaleToLen(len : float) -> self\n"
      "Scale vector to given length" },
    { "scaledToLen", (PyCFunction)vec3ScaledToLen, METH_O,
      "scaledToLen(len : float) -> Vec3\n"
      "Returns vector scaled to given length" },
    { "normalize", (PyCFunction)vec3Normalize, METH_NOARGS,
      "normalize() -> self\n"
      "Normalize vector in-place" },
    { "normalized", (PyCFunction)vec3Normalized, METH_NOARGS,
      "normalized() -> Vec3\n"
      "Returns new normalized vector" },
    { "cross", (PyCFunction)vec3Cross, METH_O,
      "cross(v : Vec3) -> Vec3\n"
      "Returns cross product of vectors" },
    { "segmentDist2", (PyCFunction)vec3SegmentDist2, METH_VARARGS,
      "segmentDist2(a : Vec3, b : Vec3) -> (float, Vec3)\n"
      "Computes squared distance of point to specified segment ab.\n"
      "Returns tuple (squared distance, witness point)" },
    { "triDist2", (PyCFunction)vec3TriDist2, METH_VARARGS,
      "triDist2(a : Vec3, b : Vec3, c : Vec3) -> (float, Vec3)\n"
      "Computes distance of point to specified triangle abc.\n"
      "Returns tuple (squared distance, witness point)" },
    { "inTri", (PyCFunction)vec3InTri, METH_VARARGS,
      "inTri(a : Vec3, b : Vec3, c : Vec3) -> bool\n"
      "Returns true if point is in triangle abc." },
    { "angle", (PyCFunction)vec3Angle, METH_VARARGS | METH_STATIC,
      "angle(a : Vec3, b : Vec3, c : Vec3) -> float\n"
      "Returns angle formed in b between points a and c. "
      "Returned value is between 0 and PI." },
    { "dihedralAngle", (PyCFunction)vec3DihedralAngle, METH_VARARGS | METH_STATIC,
      "dihedralAngle(a : Vec3, b : Vec3, c : Vec3, d : Vec3) -> float\n"
      "Returns dihedral angle between planes abc and bcd." },
    { "projToPlane", (PyCFunction)vec3ProjToPlane, METH_VARARGS,
      "projToPlane(a : Vec3, b : Vec3, c : Vec3) -> (float, Vec3)\n"
      "Computes projection of point onto specified plane.\n"
      "Returns tuple (squared distance, witness point)." },
    { "projToPlane2", (PyCFunction)vec3ProjToPlane2, METH_VARARGS,
      "projToPlane2(x : Vec3, n : Vec3) -> (float, Vec3)\n"
      "Computes projection of point onto specified plane (by point and normal).\n"
      "Returns tuple (squared distance, witness point)." },
    { "triArea2", (PyCFunction)vec3TriArea2, METH_VARARGS | METH_STATIC,
      "triArea2(a : Vec3, b : Vec3, c : Vec3) -> float\n"
      "Returns twice of area of triangle." },
    { NULL }
};

static PyNumberMethods py_vec3_num = {
    (binaryfunc)vec3Add, /* binaryfunc nb_add */
    (binaryfunc)vec3Sub, /* binaryfunc nb_subtract */
    (binaryfunc)vec3Mul, /* binaryfunc nb_multiply */
    0, /* binaryfunc nb_remainder */
    0, /* binaryfunc nb_divmod */
    0, /* ternaryfunc nb_power */
    (unaryfunc)vec3Neg, /* unaryfunc nb_negative */
    0, /* unaryfunc nb_positive */
    0, /* unaryfunc nb_absolute */
    0, /* inquiry nb_bool */
    0, /* unaryfunc nb_invert */
    0, /* binaryfunc nb_lshift */
    0, /* binaryfunc nb_rshift */
    0, /* binaryfunc nb_and */
    0, /* binaryfunc nb_xor */
    0, /* binaryfunc nb_or */
    0, /* unaryfunc nb_int */
    0, /* void *nb_reserved */
    0, /* unaryfunc nb_float */

    (binaryfunc)vec3AddIn, /* binaryfunc nb_inplace_add */
    (binaryfunc)vec3SubIn, /* binaryfunc nb_inplace_subtract */
    (binaryfunc)vec3MulIn, /* binaryfunc nb_inplace_multiply */
    0, /* binaryfunc nb_inplace_remainder */
    0, /* ternaryfunc nb_inplace_power */
    0, /* binaryfunc nb_inplace_lshift */
    0, /* binaryfunc nb_inplace_rshift */
    0, /* binaryfunc nb_inplace_and */
    0, /* binaryfunc nb_inplace_xor */
    0, /* binaryfunc nb_inplace_or */

    (binaryfunc)vec3Div, /* binaryfunc nb_floor_divide */
    (binaryfunc)vec3Div, /* binaryfunc nb_true_divide */
    (binaryfunc)vec3DivIn, /* binaryfunc nb_inplace_floor_divide */
    (binaryfunc)vec3DivIn, /* binaryfunc nb_inplace_true_divide */

    0  /* unaryfunc nb_index */
};

static PySequenceMethods py_vec3_seq = {
    (lenfunc)vec3SeqSize, /* sq_length */
    0, /* sq_concat */
    0, /* sq_repeat */
    (ssizeargfunc)vec3SeqGet, /* sq_item */
    0, /* was_sq_slice */
    (ssizeobjargproc)vec3SeqSet, /* sq_ass_item */
    0, /* was_sq_ass_slice */
    0, /* sq_contains */

    0, /* sq_inplace_concat */
    0, /* sq_inplace_repeat */
};


PyTypeObject py_vec3_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "mg.Vec3",                 /* tp_name */
    sizeof(py_vec3),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    (reprfunc)vec3AsStr,       /* tp_repr */
    &py_vec3_num,              /* tp_as_number */
    &py_vec3_seq,              /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)vec3AsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "3D vector",               /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    vec3Cmp,                   /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    py_vec3_methods,           /* tp_methods */
    0,                         /* tp_members */
    py_vec3_getset,            /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)vec3ObjInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_free - Low-level free-memory routine */
    0,                         /* tp_is_gc - For PyObject_IS_GC */
    0,                         /* tp_bases */
    0,                         /* tp_mro - method resolution order */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0                          /* tp_weaklist */
};

void vec3Init(PyObject *module)
{
    py_vec3_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_vec3_type) < 0)
        return;

    Py_INCREF(&py_vec3_type);
    PyModule_AddObject(module, "Vec3", (PyObject *)&py_vec3_type);
}


static int vec3ObjInit(py_vec3 *self, PyObject *_args, PyObject *kwds)
{
    mg_real_t v[3];
    Py_ssize_t i, len = 0;
    PyObject *args = NULL, *val;

    v[0] = MG_ZERO;
    v[1] = MG_ZERO;
    v[2] = MG_ZERO;

    if (PySequence_Check(_args)){
        len = PySequence_Size(_args);
        args = PySequence_Fast(_args, "error");
    }

    for (i = 0; i < len && i < 3; i++){
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

    mgVec3Set(&self->v, v[0], v[1], v[2]);
    return 0;
}

static PyObject *vec3AsStr(py_vec3 *self)
{
    char str[100];
    snprintf(str, 100, "<Vec3: %f %f %f>", 
             mgVec3X(&self->v), mgVec3Y(&self->v), mgVec3Z(&self->v));
    return PyUnicode_FromString(str);
}

static PyObject *vec3GetX(py_vec3 *self, void *coord)
{
    return PyFloat_FromDouble(mgVec3X(&self->v));
}
static PyObject *vec3GetY(py_vec3 *self, void *coord)
{
    return PyFloat_FromDouble(mgVec3Y(&self->v));
}
static PyObject *vec3GetZ(py_vec3 *self, void *coord)
{
    return PyFloat_FromDouble(mgVec3Z(&self->v));
}

static int vec3SetX(py_vec3 *self, PyObject *val, void *coord)
{
    mgVec3SetX(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int vec3SetY(py_vec3 *self, PyObject *val, void *coord)
{
    mgVec3SetY(&self->v, PyFloat_AsDouble(val));
    return 0;
}
static int vec3SetZ(py_vec3 *self, PyObject *val, void *coord)
{
    mgVec3SetZ(&self->v, PyFloat_AsDouble(val));
    return 0;
}


static PyObject *vec3Cmp(PyObject *a, PyObject *b, int op)
{
    py_vec3 *v1, *v2;

    if (!PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)){
        return Py_NotImplemented;
    }

    v1 = (py_vec3 *)a;
    v2 = (py_vec3 *)b;
    if (op == Py_EQ){
        return mgVec3Eq(&v1->v, &v2->v) ? Py_True : Py_False;
    }else if (op == Py_NE){
        return mgVec3NEq(&v1->v, &v2->v) ? Py_True : Py_False;
    }
    return Py_NotImplemented;
}

static PyObject *vec3Copy(py_vec3 *self)
{
    py_vec3 *v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Copy(&v->v, &self->v);
    return (PyObject *)v;
}

static PyObject *vec3Len2(py_vec3 *self)
{
    return PyFloat_FromDouble(mgVec3Len2(&self->v));
}
static PyObject *vec3Len(py_vec3 *self)
{
    return PyFloat_FromDouble(mgVec3Len(&self->v));
}

static PyObject *vec3Dist2(py_vec3 *self, py_vec3 *o)
{
    mg_real_t d;

    CHECK_VEC3(o)

    d = mgVec3Dist2(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}
static PyObject *vec3Dist(py_vec3 *self, py_vec3 *o)
{
    mg_real_t d;

    CHECK_VEC3(o)

    d = mgVec3Dist(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}

static PyObject *vec3ScaleToLen(py_vec3 *self, PyObject *o)
{
    mg_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    mgVec3ScaleToLen(&self->v, num);
    Py_INCREF(self);
    return (PyObject *)self;
}
static PyObject *vec3ScaledToLen(py_vec3 *self, PyObject *o)
{
    mg_real_t num;
    py_vec3 *v;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Copy(&v->v, &self->v);
    mgVec3ScaleToLen(&v->v, num);
    return (PyObject *)v;
}

static PyObject *vec3Normalize(py_vec3 *self)
{
    mgVec3Normalize(&self->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *vec3Normalized(py_vec3 *self)
{
    py_vec3 *v;
    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Copy(&v->v, &self->v);
    mgVec3Normalize(&v->v);
    return (PyObject *)v;
}

static PyObject *vec3Cross(py_vec3 *self, py_vec3 *o)
{
    py_vec3 *v;

    CHECK_VEC3(o)

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Cross(&v->v, &self->v, &o->v);
    return (PyObject *)v;
}

static PyObject *vec3SegmentDist2(py_vec3 *self, PyObject *args)
{
    PyObject *ret;
    mg_real_t dist;
    py_vec3 *a, *b, *witness;

    if (!PyArg_ParseTuple(args, "OO", &a, &b)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected two Vec3s");
        return NULL;
    }

    ret = PyTuple_New(2);
    witness = PyObject_New(py_vec3, &py_vec3_type);
    dist = mgVec3PointSegmentDist2(&self->v, &a->v, &b->v, &witness->v);
    PyTuple_SET_ITEM(ret, 0, PyFloat_FromDouble(dist));
    PyTuple_SET_ITEM(ret, 1, (PyObject *)witness);
    return ret;
}

static PyObject *vec3TriDist2(py_vec3 *self, PyObject *args)
{
    PyObject *ret;
    mg_real_t dist;
    py_vec3 *a, *b, *c, *witness;

    if (!PyArg_ParseTuple(args, "OOO", &a, &b, &c)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)
            || !PyObject_TypeCheck(c, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected three Vec3s");
        return NULL;
    }

    ret = PyTuple_New(2);
    witness = PyObject_New(py_vec3, &py_vec3_type);
    dist = mgVec3PointTriDist2(&self->v, &a->v, &b->v, &c->v, &witness->v);
    PyTuple_SET_ITEM(ret, 0, PyFloat_FromDouble(dist));
    PyTuple_SET_ITEM(ret, 1, (PyObject *)witness);
    return ret;
}

static PyObject *vec3InTri(py_vec3 *self, PyObject *args)
{
    py_vec3 *a, *b, *c;

    if (!PyArg_ParseTuple(args, "OOO", &a, &b, &c)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)
            || !PyObject_TypeCheck(c, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected three Vec3s");
        return NULL;
    }

    return mgVec3PointInTri(&self->v, &a->v, &b->v, &c->v) ? Py_True : Py_False;
}

static PyObject *vec3Angle(py_vec3 *self, PyObject *args)
{
    py_vec3 *a, *b, *c;
    mg_real_t angle;

    if (!PyArg_ParseTuple(args, "OOO", &a, &b, &c)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)
            || !PyObject_TypeCheck(c, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected three Vec3s");
        return NULL;
    }

    angle = mgVec3Angle(&a->v, &b->v, &c->v);
    return PyFloat_FromDouble(angle);
}

static PyObject *vec3DihedralAngle(py_vec3 *self, PyObject *args)
{
    py_vec3 *a, *b, *c, *d;
    mg_real_t angle;

    if (!PyArg_ParseTuple(args, "OOOO", &a, &b, &c, &d)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)
            || !PyObject_TypeCheck(c, &py_vec3_type)
            || !PyObject_TypeCheck(d, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected three Vec3s");
        return NULL;
    }

    angle = mgVec3DihedralAngle(&a->v, &b->v, &c->v, &d->v);
    return PyFloat_FromDouble(angle);
}

static PyObject *vec3ProjToPlane(py_vec3 *self, PyObject *args)
{
    PyObject *ret;
    mg_real_t dist;
    py_vec3 *a, *b, *c, *witness;

    if (!PyArg_ParseTuple(args, "OOO", &a, &b, &c)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)
            || !PyObject_TypeCheck(c, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected three Vec3s");
        return NULL;
    }

    ret = PyTuple_New(2);
    witness = PyObject_New(py_vec3, &py_vec3_type);
    dist = mgVec3ProjToPlane(&self->v, &a->v, &b->v, &c->v, &witness->v);
    PyTuple_SET_ITEM(ret, 0, PyFloat_FromDouble(dist));
    PyTuple_SET_ITEM(ret, 1, (PyObject *)witness);
    return ret;
}

static PyObject *vec3ProjToPlane2(py_vec3 *self, PyObject *args)
{
    PyObject *ret;
    mg_real_t dist;
    py_vec3 *x, *n, *witness;

    if (!PyArg_ParseTuple(args, "OO", &x, &n)
            || !PyObject_TypeCheck(x, &py_vec3_type)
            || !PyObject_TypeCheck(n, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected two Vec3s");
        return NULL;
    }

    ret = PyTuple_New(2);
    witness = PyObject_New(py_vec3, &py_vec3_type);
    dist = mgVec3ProjToPlane2(&self->v, &x->v, &n->v, &witness->v);
    PyTuple_SET_ITEM(ret, 0, PyFloat_FromDouble(dist));
    PyTuple_SET_ITEM(ret, 1, (PyObject *)witness);
    return ret;
}

static PyObject *vec3TriArea2(py_vec3 *self, PyObject *args)
{
    py_vec3 *a, *b, *c;
    mg_real_t angle;

    if (!PyArg_ParseTuple(args, "OOO", &a, &b, &c)
            || !PyObject_TypeCheck(a, &py_vec3_type)
            || !PyObject_TypeCheck(b, &py_vec3_type)
            || !PyObject_TypeCheck(c, &py_vec3_type)){
        PyErr_SetString(PyExc_TypeError, "Expected three Vec3s");
        return NULL;
    }

    angle = mgVec3TriArea2(&a->v, &b->v, &c->v);
    return PyFloat_FromDouble(angle);
}



static PyObject *vec3Add(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;

    CHECK_VEC3(o)

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Add2(&v->v, &self->v, &((py_vec3 *)o)->v);
    return (PyObject *)v;
}

static PyObject *vec3Sub(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;

    CHECK_VEC3(o)

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Sub2(&v->v, &self->v, &((py_vec3 *)o)->v);
    return (PyObject *)v;
}

static PyObject *vec3Mul(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;
    mg_real_t num;

    if (PyObject_TypeCheck(o, &py_vec3_type)){
        num = mgVec3Dot(&self->v, &((py_vec3 *)o)->v);
        return PyFloat_FromDouble(num);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec3, &py_vec3_type);
        num = numberAsReal(o);

        mgVec3Copy(&v->v, &self->v);
        mgVec3Scale(&v->v, num);
        return (PyObject *)v;
    }else{
        PyErr_SetString(PyExc_TypeError, "Expected Vec3 or float number");
        return NULL;
    }
}

static PyObject *vec3Neg(py_vec3 *self)
{
    py_vec3 *v;

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Copy(&v->v, &self->v);
    mgVec3Scale(&v->v, -MG_ONE);
    return (PyObject *)v;
}

static PyObject *vec3Div(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;
    mg_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Copy(&v->v, &self->v);
    mgVec3Scale(&v->v, MG_ONE / num);
    return (PyObject *)v;
}

static PyObject *vec3AddIn(py_vec3 *self, py_vec3 *o)
{
    CHECK_VEC3(o)

    mgVec3Add(&self->v, &o->v);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec3SubIn(py_vec3 *self, py_vec3 *o)
{
    CHECK_VEC3(o)

    mgVec3Sub(&self->v, &o->v);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec3MulIn(py_vec3 *self, PyObject *o)
{
    mg_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    mgVec3Scale(&self->v, num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec3DivIn(py_vec3 *self, PyObject *o)
{
    mg_real_t num;

    CHECK_FLOAT(o);
    num = numberAsReal(o);

    mgVec3Scale(&self->v, MG_ONE / num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}


static Py_ssize_t vec3SeqSize(py_vec3 *self)
{
    return 3;
}

static PyObject *vec3SeqGet(py_vec3 *self, Py_ssize_t i)
{
    if (i >= 3){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    return PyFloat_FromDouble(mgVec3Get(&self->v, i));
}

static int vec3SeqSet(py_vec3 *self, Py_ssize_t i, PyObject *val)
{
    if (i >= 3){
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return -1;
    }

    CHECK_FLOAT2(val, -1);

    mgVec3SetCoord(&self->v, i, numberAsReal(val));
    return 0;
}
