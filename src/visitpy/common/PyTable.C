// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             PyTable.C                                     //
// ************************************************************************* //

#include <PyTable.h>

#include <Py2and3Support.h>

#include <iostream>

static const char *visit_PyTable_doc =
"PyTable\n"
"\n"
"\n"
"Description:\n"
"\n"
"Wraps a table of data (2D array) stored in a contiguous buffer of memory.\n"
"Use numpy.asarray() to interact with the data as a numpy ND array.\n"
;

// ****************************************************************************
//  Struct: PyTableData
//
//  Purpose:
//    The underlying data contained in a PyTableObject.
//
//  Programmer: Chris Laganella
//  Creation:   Mon Jan 17 15:11:40 EST 2022
//
//  Modifications:
//
// ****************************************************************************
struct PyTableData
{
    void *buff;
    Py_ssize_t len;       // The size of the buffer
    Py_ssize_t itemsize;  // The size of each element in the buffer
    Py_ssize_t shape[2];
    Py_ssize_t strides[2];
    char format[4];
};

// ****************************************************************************
//  Struct: PyTableObject
//
//  Purpose:
//    Python object used to expose a buffer of memory from C as a
//    2D array in Python.
//
//  Programmer: Chris Laganella
//  Creation:   Mon Jan 17 15:11:40 EST 2022
//
//  Modifications:
//
// ****************************************************************************
struct PyTableObject
{
    PyObject_HEAD
    PyTableData table;
};

// ****************************************************************************
//  Method: PyTableData_dealloc
//
//  Purpose:
//      Free the underlying memory owned by a PyTableData struct.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
PyTableData_dealloc(PyTableData *table)
{
    if(!table)
    {
        return;
    }
    free(table->buff);
}

// ****************************************************************************
//  Method: PyTableObject_dealloc
//
//  Purpose:
//      To be called by Python when the reference count on a PyTableObject
//      reaches 0.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static void
PyTableObject_dealloc(PyTableObject *self)
{
    if(!self)
    {
        return;
    }
    PyTableData_dealloc(&self->table);
    PyObject_Del(self);
}

// ****************************************************************************
//  Method: PyTableObject_getbuffer
//
//  Purpose:
//      Used to return the underlying data in a numpy compatible form.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
static int
PyTableObject_getbuffer(PyObject *obj, Py_buffer *view, int)
{
    if(!view)
    {
        PyErr_SetString(PyExc_BufferError, "NULL view in getbuffer");
        return -1;
    }

    if(!PyTable_Check(obj))
    {
        PyErr_SetString(PyExc_BufferError, "obj is not a table in getbuffer.");
        return -1;
    }

    PyTableObject *self = (PyTableObject*)obj;
    view->buf = (void*)self->table.buff;
    view->obj = obj;
    view->len = self->table.len;
    view->itemsize = self->table.itemsize;
    view->readonly = 0;
    view->ndim = 2;
    view->format = self->table.format;
    view->shape = self->table.shape;
    view->strides = self->table.strides;
    view->suboffsets = NULL;

    // Need to increase the reference count
    Py_INCREF(self);
    return 0;
}

// This definition is only compatible with Python 3.3 and above
static PyBufferProcs PyTableObject_as_buffer = {
  (getbufferproc)PyTableObject_getbuffer,
  (releasebufferproc)0,
};

// NOTE: Need to use the tp_as_buffer field so not using the VISIT_PY_TYPE_OBJ
//  macro.
static PyTypeObject PyTableObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "PyTable",                          /* tp_name */
    sizeof(PyTableObject),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)PyTableObject_dealloc,  /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash  */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    &PyTableObject_as_buffer,           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                 /* tp_flags */
    visit_PyTable_doc,                  /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    0,                                  /* tp_methods */
    0,                                  /* tp_members */
    0,                                  /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    0                                   /* tp_init */
    PyVarObject_TAIL
};

// ****************************************************************************
//  Method: PyTable_Check
//
//  Purpose:
//      Check if the given PyObject is a PyTableObject.
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
bool
PyTable_Check(const PyObject *obj)
{
    return (obj && (obj->ob_type == &PyTableObjectType));
}

// ****************************************************************************
//  Method: PyTable_Create
//
//  Purpose:
//      Construct a PyTableObject using the given data and shape.
//      The data is copied into a new buffer owned by the PyTableObject.
//      Shape is of size 2
//
//  Programmer:   Chris Laganella
//  Creation:     Tue Jan 11 17:26:14 EST 2022
//
//  Modifications:
//
// ****************************************************************************
PyObject* PyTable_Create(const double *data,
                         const long *shape)
{
    PyTableObject *obj = PyObject_New(PyTableObject, &PyTableObjectType);
    if(!obj)
    {
        return NULL;
    }

    PyTableData &table = obj->table;
    table.len = sizeof(double) * shape[0] * shape[1];
    table.buff = malloc(table.len);
    if(!table.buff)
    {
        Py_DecRef((PyObject*)obj);
        return NULL;
    }
    table.itemsize = sizeof(double);
    table.format[0] = 'd'; table.format[1] = '\0';

    memcpy(table.buff, data, table.len);
    table.shape[0] = shape[0];
    table.shape[1] = shape[1];
    table.strides[0] = sizeof(double) * shape[1];
    table.strides[1] = sizeof(double);
    return (PyObject*)obj;
}
