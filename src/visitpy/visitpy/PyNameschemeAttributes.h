// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_NAMESCHEMEATTRIBUTES_H
#define PY_NAMESCHEMEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <NameschemeAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define NAMESCHEMEATTRIBUTES_NMETH 16
void VISITPY_API           PyNameschemeAttributes_StartUp(NameschemeAttributes *subj, void *data);
void VISITPY_API           PyNameschemeAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyNameschemeAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyNameschemeAttributes_Check(PyObject *obj);
VISITPY_API NameschemeAttributes *  PyNameschemeAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyNameschemeAttributes_New();
VISITPY_API PyObject *     PyNameschemeAttributes_Wrap(const NameschemeAttributes *attr);
void VISITPY_API           PyNameschemeAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyNameschemeAttributes_SetDefaults(const NameschemeAttributes *atts);
std::string VISITPY_API    PyNameschemeAttributes_GetLogString();
std::string VISITPY_API    PyNameschemeAttributes_ToString(const NameschemeAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyNameschemeAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyNameschemeAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyNameschemeAttributes_methods[NAMESCHEMEATTRIBUTES_NMETH];

#endif

