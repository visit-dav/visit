// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWAXISARRAYATTRIBUTES_H
#define PY_VIEWAXISARRAYATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewAxisArrayAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWAXISARRAYATTRIBUTES_NMETH 9
void VISITPY_API           PyViewAxisArrayAttributes_StartUp(ViewAxisArrayAttributes *subj, void *data);
void VISITPY_API           PyViewAxisArrayAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyViewAxisArrayAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewAxisArrayAttributes_Check(PyObject *obj);
VISITPY_API ViewAxisArrayAttributes *  PyViewAxisArrayAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewAxisArrayAttributes_New();
VISITPY_API PyObject *     PyViewAxisArrayAttributes_Wrap(const ViewAxisArrayAttributes *attr);
void VISITPY_API           PyViewAxisArrayAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewAxisArrayAttributes_SetDefaults(const ViewAxisArrayAttributes *atts);
std::string VISITPY_API    PyViewAxisArrayAttributes_GetLogString();
std::string VISITPY_API    PyViewAxisArrayAttributes_ToString(const ViewAxisArrayAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewAxisArrayAttributes_getattro(PyObject *self, PyObject *attr_name);
int VISITPY_API            PyViewAxisArrayAttributes_setattro(PyObject *self, PyObject *attr_name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewAxisArrayAttributes_methods[VIEWAXISARRAYATTRIBUTES_NMETH];

#endif

