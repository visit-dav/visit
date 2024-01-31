// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_REFLECTATTRIBUTES_H
#define PY_REFLECTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ReflectAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define REFLECTATTRIBUTES_NMETH 24
void           PyReflectAttributes_StartUp(ReflectAttributes *subj, void *data);
void           PyReflectAttributes_CloseDown();
PyMethodDef *  PyReflectAttributes_GetMethodTable(int *nMethods);
bool           PyReflectAttributes_Check(PyObject *obj);
ReflectAttributes *  PyReflectAttributes_FromPyObject(PyObject *obj);
PyObject *     PyReflectAttributes_New();
PyObject *     PyReflectAttributes_Wrap(const ReflectAttributes *attr);
void           PyReflectAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyReflectAttributes_SetDefaults(const ReflectAttributes *atts);
std::string    PyReflectAttributes_GetLogString();
std::string    PyReflectAttributes_ToString(const ReflectAttributes *, const char *, const bool=false);
PyObject *     PyReflectAttributes_getattr(PyObject *self, char *name);
int            PyReflectAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyReflectAttributes_methods[REFLECTATTRIBUTES_NMETH];

#endif

