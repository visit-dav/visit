// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SPHRESAMPLEATTRIBUTES_H
#define PY_SPHRESAMPLEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <SPHResampleAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define SPHRESAMPLEATTRIBUTES_NMETH 26
void           PySPHResampleAttributes_StartUp(SPHResampleAttributes *subj, void *data);
void           PySPHResampleAttributes_CloseDown();
PyMethodDef *  PySPHResampleAttributes_GetMethodTable(int *nMethods);
bool           PySPHResampleAttributes_Check(PyObject *obj);
SPHResampleAttributes *  PySPHResampleAttributes_FromPyObject(PyObject *obj);
PyObject *     PySPHResampleAttributes_New();
PyObject *     PySPHResampleAttributes_Wrap(const SPHResampleAttributes *attr);
void           PySPHResampleAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PySPHResampleAttributes_SetDefaults(const SPHResampleAttributes *atts);
std::string    PySPHResampleAttributes_GetLogString();
std::string    PySPHResampleAttributes_ToString(const SPHResampleAttributes *, const char *, const bool=false);
PyObject *     PySPHResampleAttributes_getattr(PyObject *self, char *name);
int            PySPHResampleAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PySPHResampleAttributes_methods[SPHRESAMPLEATTRIBUTES_NMETH];

#endif

