// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SEEDMEATTRIBUTES_H
#define PY_SEEDMEATTRIBUTES_H
#include <Python.h>
#include <SeedMeAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define SEEDMEATTRIBUTES_NMETH 62
void VISITPY_API           PySeedMeAttributes_StartUp(SeedMeAttributes *subj, void *data);
void VISITPY_API           PySeedMeAttributes_CloseDown();
VISITPY_API PyMethodDef *  PySeedMeAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PySeedMeAttributes_Check(PyObject *obj);
VISITPY_API SeedMeAttributes *  PySeedMeAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PySeedMeAttributes_New();
VISITPY_API PyObject *     PySeedMeAttributes_Wrap(const SeedMeAttributes *attr);
void VISITPY_API           PySeedMeAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PySeedMeAttributes_SetDefaults(const SeedMeAttributes *atts);
std::string VISITPY_API    PySeedMeAttributes_GetLogString();
std::string VISITPY_API    PySeedMeAttributes_ToString(const SeedMeAttributes *, const char *);
VISITPY_API PyObject *     PySeedMeAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PySeedMeAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PySeedMeAttributes_methods[SEEDMEATTRIBUTES_NMETH];

#endif

