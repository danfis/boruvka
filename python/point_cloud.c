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
#include "vec3.h"
#include "point_cloud.h"

struct _py_pc_iter {
    PyObject_HEAD
    py_pc *pc;
    fer_pc3_it_t it;
};
typedef struct _py_pc_iter py_pc_iter;
static PyTypeObject py_pc_iter_type;


static int pcObjInit(py_pc *self, PyObject *args, PyObject *kwds);
static void pcDealloc(py_pc *self);
static PyObject *pcAsStr(py_pc *self);

//static PyObject *pcCopy(py_pc *self);
static PyObject *pcAdd(py_pc *self, py_vec3 *p);
static PyObject *pcPermutate(py_pc *self);
static PyObject *pcAddFromFile(py_pc *self, PyObject *args);

/** Sequence protocol functions */
static Py_ssize_t pcSeqSize(py_pc *self);
static PyObject *pcSeqGet(py_pc *self, Py_ssize_t i);


/** Iterator functions */
static PyObject *pcIter(py_pc *pc);
static int pcIterTraverse(py_pc_iter *it, visitproc visit, void *arg);
static void pcIterDealloc(py_pc_iter *self);
static PyObject *pcIterNext(py_pc_iter *self);


static PyGetSetDef py_pc_getset[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef py_pc_methods[] = {
    { "add", (PyCFunction)pcAdd, METH_O,
      "Adds point to point cloud." },
    { "addFromFile", (PyCFunction)pcAddFromFile, METH_VARARGS,
      "Adds points from file to point cloud." },
    { "permutate", (PyCFunction)pcPermutate, METH_NOARGS,
      "Permutates point cloud." },
    { NULL }
};


static PySequenceMethods py_pc_seq = {
    (lenfunc)pcSeqSize,     /* sq_length */
    0,                      /* sq_concat */
    0,                      /* sq_repeat */
    (ssizeargfunc)pcSeqGet, /* sq_item */
    0,                      /* was_sq_slice */
    0,                      /* sq_ass_item */
    0,                      /* was_sq_ass_slice */
    0,                      /* sq_contains */

    0,                      /* sq_inplace_concat */
    0,                      /* sq_inplace_repeat */
};


PyTypeObject py_pc_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.PC3",             /* tp_name */
    sizeof(py_pc),           /* tp_basicsize */
    0,                       /* tp_itemsize */
    (destructor)pcDealloc,   /* tp_dealloc */
    0,                       /* tp_print */
    0,                       /* tp_getattr */
    0,                       /* tp_setattr */
    0,                       /* tp_reserved */
    (reprfunc)pcAsStr,       /* tp_repr */
    0,                       /* tp_as_number */
    &py_pc_seq,              /* tp_as_sequence */
    0,                       /* tp_as_mapping */
    0,                       /* tp_hash  */
    0,                       /* tp_call */
    (reprfunc)pcAsStr,       /* tp_str */
    0,                       /* tp_getattro */
    0,                       /* tp_setattro */
    0,                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,      /* tp_flags */
    "Point cloud",           /* tp_doc */
    0,                       /* tp_traverse */
    0,                       /* tp_clear */
    0,                       /* tp_richcompare */
    0,                       /* tp_weaklistoffset */
    (getiterfunc)pcIter,     /* tp_iter */
    0,                       /* tp_iternext */
    py_pc_methods,           /* tp_methods */
    0,                       /* tp_members */
    py_pc_getset,            /* tp_getset */
    0,                       /* tp_base */
    0,                       /* tp_dict */
    0,                       /* tp_descr_get */
    0,                       /* tp_descr_set */
    0,                       /* tp_dictoffset */
    (initproc)pcObjInit,     /* tp_init */
    0,                       /* tp_alloc */
    0,                       /* tp_free - Low-level free-memory routine */
    0,                       /* tp_is_gc - For PyObject_IS_GC */
    0,                       /* tp_bases */
    0,                       /* tp_mro - method resolution order */
    0,                       /* tp_cache */
    0,                       /* tp_subclasses */
    0                        /* tp_weaklist */
};

void pcInit(PyObject *module)
{
    py_pc_type.tp_new = PyType_GenericNew;
    py_pc_iter_type.tp_new = PyType_GenericNew;

    if (PyType_Ready(&py_pc_type) < 0)
        return;
    Py_INCREF(&py_pc_type);

    if (PyType_Ready(&py_pc_iter_type) < 0)
        return;
    Py_INCREF(&py_pc_iter_type);

    PyModule_AddObject(module, "PC3", (PyObject *)&py_pc_type);
}


static int pcObjInit(py_pc *self, PyObject *_args, PyObject *kwds)
{
    self->pc = ferPC3New();
    return 0;
}
static void pcDealloc(py_pc *self)
{
    if (self->pc){
        ferPC3Del(self->pc);
        self->pc = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyObject *pcAsStr(py_pc *self)
{
    char str[100];
    snprintf(str, 100, "<PC3 (%d)>", (int)ferPC3Len(self->pc));
    return PyUnicode_FromString(str);
}




static PyObject *pcAdd(py_pc *self, py_vec3 *v)
{
    CHECK_VEC3(v);
    ferPC3Add(self->pc, &v->v);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *pcPermutate(py_pc *self)
{
    ferPC3Permutate(self->pc);
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject *pcAddFromFile(py_pc *self, PyObject *args)
{
    const char *fn;
    size_t len;

    if (!PyArg_ParseTuple(args, "s", &fn)){
        PyErr_SetString(PyExc_TypeError, "The only argument should be filename.");
        return NULL;
    }

    len = ferPC3AddFromFile(self->pc, fn);
    return PyLong_FromLong(len);
}


static Py_ssize_t pcSeqSize(py_pc *self)
{
    return ferPC3Len(self->pc);
}

static PyObject *pcSeqGet(py_pc *self, Py_ssize_t i)
{
    py_vec3 *v;
    fer_vec3_t *w;

    if (i >= ferPC3Len(self->pc)){
        PyErr_SetString(PyExc_IndexError, "Index out of range");
        return NULL;
    }

    w = ferPC3Get(self->pc, i);
    v = PyObject_New(py_vec3, &py_vec3_type);
    ferVec3Copy(&v->v, w);
    return (PyObject *)v;
}


/**** Iterator ****/
static PyTypeObject py_pc_iter_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "fermat.PC3Iter",              /* tp_name */
    sizeof(py_pc_iter),           /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)pcIterDealloc,    /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_reserved */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash  */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT
        | Py_TPFLAGS_HAVE_GC,     /* tp_flags */
    "Point cloud iterator",       /* tp_doc */
    (traverseproc)pcIterTraverse, /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    PyObject_SelfIter,            /* tp_iter */
    (iternextfunc)pcIterNext,     /* tp_iternext */
    0,                            /* tp_methods */
};

static PyObject *pcIter(py_pc *self)
{
    py_pc_iter *it;

    it = PyObject_GC_New(py_pc_iter, &py_pc_iter_type);
    it->pc = self;
    Py_INCREF(self);

    ferPC3ItInit(&it->it, self->pc);

    PyObject_GC_Track(it);
    return (PyObject *)it;
}

static int pcIterTraverse(py_pc_iter *it, visitproc visit, void *arg)
{
    Py_VISIT(it->pc);
    return 0;
}

static void pcIterDealloc(py_pc_iter *self)
{
    if (self->pc){
        Py_DECREF(self->pc);
        self->pc = NULL;
    }
    PyObject_GC_Del(self);
}

static PyObject *pcIterNext(py_pc_iter *self)
{
    py_vec3 *v;
    fer_vec3_t *w;

    if (ferPC3ItEnd(&self->it))
        return NULL;

    v = PyObject_New(py_vec3, &py_vec3_type);
    w = ferPC3ItGet(&self->it);
    ferVec3Copy(&v->v, w);

    ferPC3ItNext(&self->it);

    return (PyObject *)v;
}
