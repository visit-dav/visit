// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ANARIATTRIBUTES_H
#define PY_ANARIATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AnariAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define ANARIATTRIBUTES_NMETH 17
void VISITPY_API           PyAnariAttributes_StartUp(AnariAttributes *subj, void *data);
void VISITPY_API           PyAnariAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyAnariAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAnariAttributes_Check(PyObject *obj);
VISITPY_API AnariAttributes *  PyAnariAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAnariAttributes_New();
VISITPY_API PyObject *     PyAnariAttributes_Wrap(const AnariAttributes *attr);
void VISITPY_API           PyAnariAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAnariAttributes_SetDefaults(const AnariAttributes *atts);
std::string VISITPY_API    PyAnariAttributes_GetLogString();
std::string VISITPY_API    PyAnariAttributes_ToString(const AnariAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyAnariAttributes_getattro(PyObject *self, PyObject *attr_name);
int VISITPY_API            PyAnariAttributes_setattro(PyObject *self, PyObject *attr_name, PyObject *args);
VISITPY_API extern PyMethodDef PyAnariAttributes_methods[ANARIATTRIBUTES_NMETH];

#endif

