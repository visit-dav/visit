// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_INTERACTORATTRIBUTES_H
#define PY_INTERACTORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <InteractorAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define INTERACTORATTRIBUTES_NMETH 14
void VISITPY_API           PyInteractorAttributes_StartUp(InteractorAttributes *subj, void *data);
void VISITPY_API           PyInteractorAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyInteractorAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyInteractorAttributes_Check(PyObject *obj);
VISITPY_API InteractorAttributes *  PyInteractorAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyInteractorAttributes_New();
VISITPY_API PyObject *     PyInteractorAttributes_Wrap(const InteractorAttributes *attr);
void VISITPY_API           PyInteractorAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyInteractorAttributes_SetDefaults(const InteractorAttributes *atts);
std::string VISITPY_API    PyInteractorAttributes_GetLogString();
std::string VISITPY_API    PyInteractorAttributes_ToString(const InteractorAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyInteractorAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyInteractorAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyInteractorAttributes_methods[INTERACTORATTRIBUTES_NMETH];

#endif

