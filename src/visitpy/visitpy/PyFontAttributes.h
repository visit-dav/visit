// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FONTATTRIBUTES_H
#define PY_FONTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FontAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define FONTATTRIBUTES_NMETH 14
void VISITPY_API           PyFontAttributes_StartUp(FontAttributes *subj, void *data);
void VISITPY_API           PyFontAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyFontAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyFontAttributes_Check(PyObject *obj);
VISITPY_API FontAttributes *  PyFontAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyFontAttributes_New();
VISITPY_API PyObject *     PyFontAttributes_Wrap(const FontAttributes *attr);
void VISITPY_API           PyFontAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyFontAttributes_SetDefaults(const FontAttributes *atts);
std::string VISITPY_API    PyFontAttributes_GetLogString();
std::string VISITPY_API    PyFontAttributes_ToString(const FontAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyFontAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyFontAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyFontAttributes_methods[FONTATTRIBUTES_NMETH];

#endif

