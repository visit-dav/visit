// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CONEATTRIBUTES_H
#define PY_CONEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ConeAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CONEATTRIBUTES_NMETH 16
void           PyConeAttributes_StartUp(ConeAttributes *subj, void *data);
void           PyConeAttributes_CloseDown();
PyMethodDef *  PyConeAttributes_GetMethodTable(int *nMethods);
bool           PyConeAttributes_Check(PyObject *obj);
ConeAttributes *  PyConeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyConeAttributes_New();
PyObject *     PyConeAttributes_Wrap(const ConeAttributes *attr);
void           PyConeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyConeAttributes_SetDefaults(const ConeAttributes *atts);
std::string    PyConeAttributes_GetLogString();
std::string    PyConeAttributes_ToString(const ConeAttributes *, const char *, const bool=false);
PyObject *     PyConeAttributes_getattr(PyObject *self, char *name);
int            PyConeAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyConeAttributes_methods[CONEATTRIBUTES_NMETH];

#endif

