// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MATERIALATTRIBUTES_H
#define PY_MATERIALATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MaterialAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define MATERIALATTRIBUTES_NMETH 26
void VISITPY_API           PyMaterialAttributes_StartUp(MaterialAttributes *subj, void *data);
void VISITPY_API           PyMaterialAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyMaterialAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyMaterialAttributes_Check(PyObject *obj);
VISITPY_API MaterialAttributes *  PyMaterialAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyMaterialAttributes_New();
VISITPY_API PyObject *     PyMaterialAttributes_Wrap(const MaterialAttributes *attr);
void VISITPY_API           PyMaterialAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyMaterialAttributes_SetDefaults(const MaterialAttributes *atts);
std::string VISITPY_API    PyMaterialAttributes_GetLogString();
std::string VISITPY_API    PyMaterialAttributes_ToString(const MaterialAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyMaterialAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyMaterialAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyMaterialAttributes_methods[MATERIALATTRIBUTES_NMETH];

#endif

