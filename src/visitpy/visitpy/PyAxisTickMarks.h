// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXISTICKMARKS_H
#define PY_AXISTICKMARKS_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AxisTickMarks.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
void VISITPY_API           PyAxisTickMarks_StartUp(AxisTickMarks *subj, void *data);
void VISITPY_API           PyAxisTickMarks_CloseDown();
VISITPY_API PyMethodDef *  PyAxisTickMarks_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxisTickMarks_Check(PyObject *obj);
VISITPY_API AxisTickMarks *  PyAxisTickMarks_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxisTickMarks_New();
VISITPY_API PyObject *     PyAxisTickMarks_Wrap(const AxisTickMarks *attr);
void VISITPY_API           PyAxisTickMarks_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxisTickMarks_SetDefaults(const AxisTickMarks *atts);
std::string VISITPY_API    PyAxisTickMarks_GetLogString();
std::string VISITPY_API    PyAxisTickMarks_ToString(const AxisTickMarks *, const char *, const bool=false);

#endif

