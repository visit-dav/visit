// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PROCESSATTRIBUTES_H
#define PY_PROCESSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ProcessAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define PROCESSATTRIBUTES_NMETH 14
void VISITPY_API           PyProcessAttributes_StartUp(ProcessAttributes *subj, void *data);
void VISITPY_API           PyProcessAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyProcessAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyProcessAttributes_Check(PyObject *obj);
VISITPY_API ProcessAttributes *  PyProcessAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyProcessAttributes_New();
VISITPY_API PyObject *     PyProcessAttributes_Wrap(const ProcessAttributes *attr);
void VISITPY_API           PyProcessAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyProcessAttributes_SetDefaults(const ProcessAttributes *atts);
std::string VISITPY_API    PyProcessAttributes_GetLogString();
std::string VISITPY_API    PyProcessAttributes_ToString(const ProcessAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyProcessAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyProcessAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyProcessAttributes_methods[PROCESSATTRIBUTES_NMETH];

#endif

