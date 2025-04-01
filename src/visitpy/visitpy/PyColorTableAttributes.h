// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_COLORTABLEATTRIBUTES_H
#define PY_COLORTABLEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ColorTableAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
void VISITPY_API           PyColorTableAttributes_StartUp(ColorTableAttributes *subj, void *data);
void VISITPY_API           PyColorTableAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyColorTableAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyColorTableAttributes_Check(PyObject *obj);
VISITPY_API ColorTableAttributes *  PyColorTableAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyColorTableAttributes_New();
VISITPY_API PyObject *     PyColorTableAttributes_Wrap(const ColorTableAttributes *attr);
void VISITPY_API           PyColorTableAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyColorTableAttributes_SetDefaults(const ColorTableAttributes *atts);
std::string VISITPY_API    PyColorTableAttributes_GetLogString();
std::string VISITPY_API    PyColorTableAttributes_ToString(const ColorTableAttributes *, const char *, const bool=false);

#endif

