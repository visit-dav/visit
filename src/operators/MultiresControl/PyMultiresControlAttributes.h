// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MULTIRESCONTROLATTRIBUTES_H
#define PY_MULTIRESCONTROLATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MultiresControlAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define MULTIRESCONTROLATTRIBUTES_NMETH 8
void           PyMultiresControlAttributes_StartUp(MultiresControlAttributes *subj, void *data);
void           PyMultiresControlAttributes_CloseDown();
PyMethodDef *  PyMultiresControlAttributes_GetMethodTable(int *nMethods);
bool           PyMultiresControlAttributes_Check(PyObject *obj);
MultiresControlAttributes *  PyMultiresControlAttributes_FromPyObject(PyObject *obj);
PyObject *     PyMultiresControlAttributes_New();
PyObject *     PyMultiresControlAttributes_Wrap(const MultiresControlAttributes *attr);
void           PyMultiresControlAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyMultiresControlAttributes_SetDefaults(const MultiresControlAttributes *atts);
std::string    PyMultiresControlAttributes_GetLogString();
std::string    PyMultiresControlAttributes_ToString(const MultiresControlAttributes *, const char *, const bool=false);
PyObject *     PyMultiresControlAttributes_getattr(PyObject *self, char *name);
int            PyMultiresControlAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyMultiresControlAttributes_methods[MULTIRESCONTROLATTRIBUTES_NMETH];

#endif

