// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TENSORATTRIBUTES_H
#define PY_TENSORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TensorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TENSORATTRIBUTES_NMETH 40
void           PyTensorAttributes_StartUp(TensorAttributes *subj, void *data);
void           PyTensorAttributes_CloseDown();
PyMethodDef *  PyTensorAttributes_GetMethodTable(int *nMethods);
bool           PyTensorAttributes_Check(PyObject *obj);
TensorAttributes *  PyTensorAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTensorAttributes_New();
PyObject *     PyTensorAttributes_Wrap(const TensorAttributes *attr);
void           PyTensorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTensorAttributes_SetDefaults(const TensorAttributes *atts);
std::string    PyTensorAttributes_GetLogString();
std::string    PyTensorAttributes_ToString(const TensorAttributes *, const char *, const bool=false);
PyObject *     PyTensorAttributes_getattr(PyObject *self, char *name);
int            PyTensorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTensorAttributes_methods[TENSORATTRIBUTES_NMETH];

#endif

