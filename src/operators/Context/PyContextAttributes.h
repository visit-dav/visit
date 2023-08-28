// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CONTEXTATTRIBUTES_H
#define PY_CONTEXTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ContextAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CONTEXTATTRIBUTES_NMETH 16
void           PyContextAttributes_StartUp(ContextAttributes *subj, void *data);
void           PyContextAttributes_CloseDown();
PyMethodDef *  PyContextAttributes_GetMethodTable(int *nMethods);
bool           PyContextAttributes_Check(PyObject *obj);
ContextAttributes *  PyContextAttributes_FromPyObject(PyObject *obj);
PyObject *     PyContextAttributes_New();
PyObject *     PyContextAttributes_Wrap(const ContextAttributes *attr);
void           PyContextAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyContextAttributes_SetDefaults(const ContextAttributes *atts);
std::string    PyContextAttributes_GetLogString();
std::string    PyContextAttributes_ToString(const ContextAttributes *, const char *, const bool=false);
PyObject *     PyContextAttributes_getattr(PyObject *self, char *name);
int            PyContextAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyContextAttributes_methods[CONTEXTATTRIBUTES_NMETH];

#endif

