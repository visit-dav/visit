// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXISLABELS_H
#define PY_AXISLABELS_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AxisLabels.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AXISLABELS_NMETH 8
void VISITPY_API           PyAxisLabels_StartUp(AxisLabels *subj, void *data);
void VISITPY_API           PyAxisLabels_CloseDown();
VISITPY_API PyMethodDef *  PyAxisLabels_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxisLabels_Check(PyObject *obj);
VISITPY_API AxisLabels *  PyAxisLabels_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxisLabels_New();
VISITPY_API PyObject *     PyAxisLabels_Wrap(const AxisLabels *attr);
void VISITPY_API           PyAxisLabels_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxisLabels_SetDefaults(const AxisLabels *atts);
std::string VISITPY_API    PyAxisLabels_GetLogString();
std::string VISITPY_API    PyAxisLabels_ToString(const AxisLabels *, const char *, const bool=false);
VISITPY_API PyObject *     PyAxisLabels_getattr(PyObject *self, char *name);
int VISITPY_API            PyAxisLabels_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAxisLabels_methods[AXISLABELS_NMETH];

#endif

