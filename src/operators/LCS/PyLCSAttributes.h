// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LCSATTRIBUTES_H
#define PY_LCSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LCSAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LCSATTRIBUTES_NMETH 110
void           PyLCSAttributes_StartUp(LCSAttributes *subj, void *data);
void           PyLCSAttributes_CloseDown();
PyMethodDef *  PyLCSAttributes_GetMethodTable(int *nMethods);
bool           PyLCSAttributes_Check(PyObject *obj);
LCSAttributes *  PyLCSAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLCSAttributes_New();
PyObject *     PyLCSAttributes_Wrap(const LCSAttributes *attr);
void           PyLCSAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLCSAttributes_SetDefaults(const LCSAttributes *atts);
std::string    PyLCSAttributes_GetLogString();
std::string    PyLCSAttributes_ToString(const LCSAttributes *, const char *, const bool=false);
PyObject *     PyLCSAttributes_getattr(PyObject *self, char *name);
int            PyLCSAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLCSAttributes_methods[LCSATTRIBUTES_NMETH];

#endif

