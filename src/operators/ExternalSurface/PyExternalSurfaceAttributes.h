// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXTERNALSURFACEATTRIBUTES_H
#define PY_EXTERNALSURFACEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExternalSurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define EXTERNALSURFACEATTRIBUTES_NMETH 6
void           PyExternalSurfaceAttributes_StartUp(ExternalSurfaceAttributes *subj, void *data);
void           PyExternalSurfaceAttributes_CloseDown();
PyMethodDef *  PyExternalSurfaceAttributes_GetMethodTable(int *nMethods);
bool           PyExternalSurfaceAttributes_Check(PyObject *obj);
ExternalSurfaceAttributes *  PyExternalSurfaceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyExternalSurfaceAttributes_New();
PyObject *     PyExternalSurfaceAttributes_Wrap(const ExternalSurfaceAttributes *attr);
void           PyExternalSurfaceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyExternalSurfaceAttributes_SetDefaults(const ExternalSurfaceAttributes *atts);
std::string    PyExternalSurfaceAttributes_GetLogString();
std::string    PyExternalSurfaceAttributes_ToString(const ExternalSurfaceAttributes *, const char *, const bool=false);
PyObject *     PyExternalSurfaceAttributes_getattr(PyObject *self, char *name);
int            PyExternalSurfaceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyExternalSurfaceAttributes_methods[EXTERNALSURFACEATTRIBUTES_NMETH];

#endif

