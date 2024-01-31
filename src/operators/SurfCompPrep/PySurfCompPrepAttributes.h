// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SURFCOMPPREPATTRIBUTES_H
#define PY_SURFCOMPPREPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SurfCompPrepAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SURFCOMPPREPATTRIBUTES_NMETH 42
void           PySurfCompPrepAttributes_StartUp(SurfCompPrepAttributes *subj, void *data);
void           PySurfCompPrepAttributes_CloseDown();
PyMethodDef *  PySurfCompPrepAttributes_GetMethodTable(int *nMethods);
bool           PySurfCompPrepAttributes_Check(PyObject *obj);
SurfCompPrepAttributes *  PySurfCompPrepAttributes_FromPyObject(PyObject *obj);
PyObject *     PySurfCompPrepAttributes_New();
PyObject *     PySurfCompPrepAttributes_Wrap(const SurfCompPrepAttributes *attr);
void           PySurfCompPrepAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySurfCompPrepAttributes_SetDefaults(const SurfCompPrepAttributes *atts);
std::string    PySurfCompPrepAttributes_GetLogString();
std::string    PySurfCompPrepAttributes_ToString(const SurfCompPrepAttributes *, const char *, const bool=false);
PyObject *     PySurfCompPrepAttributes_getattr(PyObject *self, char *name);
int            PySurfCompPrepAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySurfCompPrepAttributes_methods[SURFCOMPPREPATTRIBUTES_NMETH];

#endif

