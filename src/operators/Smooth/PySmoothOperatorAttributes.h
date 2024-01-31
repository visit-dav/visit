// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SMOOTHOPERATORATTRIBUTES_H
#define PY_SMOOTHOPERATORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SmoothOperatorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SMOOTHOPERATORATTRIBUTES_NMETH 16
void           PySmoothOperatorAttributes_StartUp(SmoothOperatorAttributes *subj, void *data);
void           PySmoothOperatorAttributes_CloseDown();
PyMethodDef *  PySmoothOperatorAttributes_GetMethodTable(int *nMethods);
bool           PySmoothOperatorAttributes_Check(PyObject *obj);
SmoothOperatorAttributes *  PySmoothOperatorAttributes_FromPyObject(PyObject *obj);
PyObject *     PySmoothOperatorAttributes_New();
PyObject *     PySmoothOperatorAttributes_Wrap(const SmoothOperatorAttributes *attr);
void           PySmoothOperatorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySmoothOperatorAttributes_SetDefaults(const SmoothOperatorAttributes *atts);
std::string    PySmoothOperatorAttributes_GetLogString();
std::string    PySmoothOperatorAttributes_ToString(const SmoothOperatorAttributes *, const char *, const bool=false);
PyObject *     PySmoothOperatorAttributes_getattr(PyObject *self, char *name);
int            PySmoothOperatorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySmoothOperatorAttributes_methods[SMOOTHOPERATORATTRIBUTES_NMETH];

#endif

