// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MULTICURVEATTRIBUTES_H
#define PY_MULTICURVEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MultiCurveAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PyMultiCurveAttributes_StartUp(MultiCurveAttributes *subj, void *data);
void           PyMultiCurveAttributes_CloseDown();
PyMethodDef *  PyMultiCurveAttributes_GetMethodTable(int *nMethods);
bool           PyMultiCurveAttributes_Check(PyObject *obj);
MultiCurveAttributes *  PyMultiCurveAttributes_FromPyObject(PyObject *obj);
PyObject *     PyMultiCurveAttributes_New();
PyObject *     PyMultiCurveAttributes_Wrap(const MultiCurveAttributes *attr);
void           PyMultiCurveAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyMultiCurveAttributes_SetDefaults(const MultiCurveAttributes *atts);
std::string    PyMultiCurveAttributes_GetLogString();
std::string    PyMultiCurveAttributes_ToString(const MultiCurveAttributes *, const char *, const bool=false);

#endif

