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
/** Vector length */
static PyObject *vec3Len2(py_vec3 *self);
static PyObject *vec3Len(py_vec3 *self);
/** Distance of two vectors */
static PyObject *vec3Dist2(py_vec3 *self, py_vec3 *o);
static PyObject *vec3Dist(py_vec3 *self, py_vec3 *o);
/** Normalize */
static PyObject *vec3Normalize(py_vec3 *self);
static PyObject *vec3Normalized(py_vec3 *self);
/** Cross product */
static PyObject *vec3Cross(py_vec3 *self, py_vec3 *o);
/** Number protocol functions */
static PyObject *vec3Add(py_vec3 *self, PyObject *o);
static PyObject *vec3Sub(py_vec3 *self, PyObject *o);
static PyObject *vec3Mul(py_vec3 *self, PyObject *o);
static PyObject *vec3Neg(py_vec3 *self);
static PyObject *vec3AddIn(py_vec3 *self, py_vec3 *o);
static PyObject *vec3SubIn(py_vec3 *self, py_vec3 *o);
static PyObject *vec3MulIn(py_vec3 *self, PyObject *o);


static PyGetSetDef py_vec3_getset[] = {
    {"x", (getter)vec3GetX, (setter)vec3SetX, "x coordinate", NULL},
    {"y", (getter)vec3GetY, (setter)vec3SetY, "y coordinate", NULL},
    {"z", (getter)vec3GetZ, (setter)vec3SetZ, "z coordinate", NULL},
    {NULL}  /* Sentinel */
};

static PyMethodDef py_vec3_methods[] = {
    { "len2", (PyCFunction)vec3Len2, METH_NOARGS,
      "Returns squared length of vector" },
    { "len", (PyCFunction)vec3Len, METH_NOARGS,
      "Returns length of vector" },
    { "dist2", (PyCFunction)vec3Dist2, METH_O,
      "Squared distance of two vectors" },
    { "dist", (PyCFunction)vec3Dist, METH_O,
      "Distance of two vectors" },
    { "normalize", (PyCFunction)vec3Normalize, METH_NOARGS,
      "Normalize vector in-place" },
    { "normalized", (PyCFunction)vec3Normalized, METH_NOARGS,
      "Returns new normalized vector" },
    { "cross", (PyCFunction)vec3Cross, METH_O,
      "Returns cross product of vectors" },
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

    0, /* binaryfunc nb_floor_divide */
    0, /* binaryfunc nb_true_divide */
    0, /* binaryfunc nb_inplace_floor_divide */
    0, /* binaryfunc nb_inplace_true_divide */

    0  /* unaryfunc nb_index */
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
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    (reprfunc)vec3AsStr,       /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "3D vector",               /* tp_doc */
    0,		                   /* tp_traverse */
    0,		               /* tp_clear */
    vec3Cmp,		           /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    py_vec3_methods,    /* tp_methods */
    0,    /* tp_members */
    py_vec3_getset,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)vec3ObjInit,      /* tp_init */
    0,                         /* tp_alloc */
};

void vec3Init(PyObject *module)
{
    py_vec3_type.tp_new = PyType_GenericNew;
    if (PyType_Ready(&py_vec3_type) < 0)
        return;

    Py_INCREF(&py_vec3_type);
    PyModule_AddObject(module, "Vec3", (PyObject *)&py_vec3_type);
}


#define CHECK_VEC3_O(o) \
    if (!PyObject_TypeCheck((o), &py_vec3_type)){ \
        PyErr_SetString(PyExc_TypeError, "Expected Vec3"); \
        return NULL; \
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

    for (i = 0; i < len; i++){
        val = PySequence_Fast_GET_ITEM(args, i);
        if (PyFloat_Check(val)){
            v[i] = PyFloat_AsDouble(val);
        }else if (PyNumber_Check(val)){
            val = PyNumber_Float(val);
            v[i] = PyFloat_AsDouble(val);
            Py_DECREF(val);
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
    snprintf(str, 100, "<%f %f %f>", 
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

    CHECK_VEC3_O(o)

    d = mgVec3Dist2(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}
static PyObject *vec3Dist(py_vec3 *self, py_vec3 *o)
{
    mg_real_t d;

    CHECK_VEC3_O(o)

    d = mgVec3Dist(&self->v, &o->v);
    return PyFloat_FromDouble(d);
}

static PyObject *vec3Normalize(py_vec3 *self)
{
    mgVec3Normalize(&self->v);
    return Py_None;
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

    CHECK_VEC3_O(o)

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Cross(&v->v, &self->v, &o->v);
    return (PyObject *)v;
}

static PyObject *vec3Add(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;

    CHECK_VEC3_O(o)

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Add2(&v->v, &self->v, &((py_vec3 *)o)->v);
    return (PyObject *)v;
}

static PyObject *vec3Sub(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;

    CHECK_VEC3_O(o)

    v = PyObject_New(py_vec3, &py_vec3_type);
    mgVec3Sub2(&v->v, &self->v, &((py_vec3 *)o)->v);
    return (PyObject *)v;
}

static PyObject *vec3Mul(py_vec3 *self, PyObject *o)
{
    py_vec3 *v;
    mg_real_t num;
    PyObject *onum;

    if (PyObject_TypeCheck(o, &py_vec3_type)){
        num = mgVec3Dot(&self->v, &((py_vec3 *)o)->v);
        return PyFloat_FromDouble(num);
    }else if (PyNumber_Check(o)){
        v = PyObject_New(py_vec3, &py_vec3_type);
        onum = PyNumber_Float(o);
        num = PyFloat_AS_DOUBLE(onum);
        Py_DECREF(onum);

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

static PyObject *vec3AddIn(py_vec3 *self, py_vec3 *o)
{
    CHECK_VEC3_O(o)

    mgVec3Add(&self->v, &o->v);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec3SubIn(py_vec3 *self, py_vec3 *o)
{
    CHECK_VEC3_O(o)

    mgVec3Sub(&self->v, &o->v);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}

static PyObject *vec3MulIn(py_vec3 *self, PyObject *o)
{
    mg_real_t num;
    PyObject *onum;

    if (!PyNumber_Check(o)){
        PyErr_SetString(PyExc_TypeError, "Expected float number");
        return NULL;
    }

    onum = PyNumber_Float(o);
    num = PyFloat_AS_DOUBLE(onum);
    Py_DECREF(onum);

    mgVec3Scale(&self->v, num);
    Py_INCREF((PyObject *)self);
    return (PyObject *)self;
}
