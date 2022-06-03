// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_COLORATTRIBUTE_H
#define PY_COLORATTRIBUTE_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ColorAttribute.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define COLORATTRIBUTE_NMETH 4
void VISITPY_API           PyColorAttribute_StartUp(ColorAttribute *subj, void *data);
void VISITPY_API           PyColorAttribute_CloseDown();
VISITPY_API PyMethodDef *  PyColorAttribute_GetMethodTable(int *nMethods);
bool VISITPY_API           PyColorAttribute_Check(PyObject *obj);
VISITPY_API ColorAttribute *  PyColorAttribute_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyColorAttribute_New();
VISITPY_API PyObject *     PyColorAttribute_Wrap(const ColorAttribute *attr);
void VISITPY_API           PyColorAttribute_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyColorAttribute_SetDefaults(const ColorAttribute *atts);
std::string VISITPY_API    PyColorAttribute_GetLogString();
std::string VISITPY_API    PyColorAttribute_ToString(const ColorAttribute *, const char *, const bool=false);
VISITPY_API PyObject *     PyColorAttribute_getattr(PyObject *self, char *name);
int VISITPY_API            PyColorAttribute_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyColorAttribute_methods[COLORATTRIBUTE_NMETH];

#endif

