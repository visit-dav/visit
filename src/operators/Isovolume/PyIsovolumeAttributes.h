// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ISOVOLUMEATTRIBUTES_H
#define PY_ISOVOLUMEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <IsovolumeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define ISOVOLUMEATTRIBUTES_NMETH 8
void           PyIsovolumeAttributes_StartUp(IsovolumeAttributes *subj, void *data);
void           PyIsovolumeAttributes_CloseDown();
PyMethodDef *  PyIsovolumeAttributes_GetMethodTable(int *nMethods);
bool           PyIsovolumeAttributes_Check(PyObject *obj);
IsovolumeAttributes *  PyIsovolumeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyIsovolumeAttributes_New();
PyObject *     PyIsovolumeAttributes_Wrap(const IsovolumeAttributes *attr);
void           PyIsovolumeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyIsovolumeAttributes_SetDefaults(const IsovolumeAttributes *atts);
std::string    PyIsovolumeAttributes_GetLogString();
std::string    PyIsovolumeAttributes_ToString(const IsovolumeAttributes *, const char *, const bool=false);
PyObject *     PyIsovolumeAttributes_getattr(PyObject *self, char *name);
int            PyIsovolumeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyIsovolumeAttributes_methods[ISOVOLUMEATTRIBUTES_NMETH];

#endif

