// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXISATTRIBUTES_H
#define PY_AXISATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AxisAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AXISATTRIBUTES_NMETH 10
void VISITPY_API           PyAxisAttributes_StartUp(AxisAttributes *subj, void *data);
void VISITPY_API           PyAxisAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyAxisAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxisAttributes_Check(PyObject *obj);
VISITPY_API AxisAttributes *  PyAxisAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxisAttributes_New();
VISITPY_API PyObject *     PyAxisAttributes_Wrap(const AxisAttributes *attr);
void VISITPY_API           PyAxisAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxisAttributes_SetDefaults(const AxisAttributes *atts);
std::string VISITPY_API    PyAxisAttributes_GetLogString();
std::string VISITPY_API    PyAxisAttributes_ToString(const AxisAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyAxisAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyAxisAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAxisAttributes_methods[AXISATTRIBUTES_NMETH];

#endif

