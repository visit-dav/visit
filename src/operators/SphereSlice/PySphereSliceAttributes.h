// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SPHERESLICEATTRIBUTES_H
#define PY_SPHERESLICEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SphereSliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SPHERESLICEATTRIBUTES_NMETH 6
void           PySphereSliceAttributes_StartUp(SphereSliceAttributes *subj, void *data);
void           PySphereSliceAttributes_CloseDown();
PyMethodDef *  PySphereSliceAttributes_GetMethodTable(int *nMethods);
bool           PySphereSliceAttributes_Check(PyObject *obj);
SphereSliceAttributes *  PySphereSliceAttributes_FromPyObject(PyObject *obj);
PyObject *     PySphereSliceAttributes_New();
PyObject *     PySphereSliceAttributes_Wrap(const SphereSliceAttributes *attr);
void           PySphereSliceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySphereSliceAttributes_SetDefaults(const SphereSliceAttributes *atts);
std::string    PySphereSliceAttributes_GetLogString();
std::string    PySphereSliceAttributes_ToString(const SphereSliceAttributes *, const char *, const bool=false);
PyObject *     PySphereSliceAttributes_getattr(PyObject *self, char *name);
int            PySphereSliceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySphereSliceAttributes_methods[SPHERESLICEATTRIBUTES_NMETH];

#endif

