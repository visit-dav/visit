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
#define MULTICURVEATTRIBUTES_NMETH 34
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
PyObject *     PyMultiCurveAttributes_getattr(PyObject *self, char *name);
int            PyMultiCurveAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyMultiCurveAttributes_methods[MULTICURVEATTRIBUTES_NMETH];

#endif

