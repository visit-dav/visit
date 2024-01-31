// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_STAGGERATTRIBUTES_H
#define PY_STAGGERATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <StaggerAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define STAGGERATTRIBUTES_NMETH 8
void           PyStaggerAttributes_StartUp(StaggerAttributes *subj, void *data);
void           PyStaggerAttributes_CloseDown();
PyMethodDef *  PyStaggerAttributes_GetMethodTable(int *nMethods);
bool           PyStaggerAttributes_Check(PyObject *obj);
StaggerAttributes *  PyStaggerAttributes_FromPyObject(PyObject *obj);
PyObject *     PyStaggerAttributes_New();
PyObject *     PyStaggerAttributes_Wrap(const StaggerAttributes *attr);
void           PyStaggerAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyStaggerAttributes_SetDefaults(const StaggerAttributes *atts);
std::string    PyStaggerAttributes_GetLogString();
std::string    PyStaggerAttributes_ToString(const StaggerAttributes *, const char *, const bool=false);
PyObject *     PyStaggerAttributes_getattr(PyObject *self, char *name);
int            PyStaggerAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyStaggerAttributes_methods[STAGGERATTRIBUTES_NMETH];

#endif

