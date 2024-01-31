// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_INTEGRALCURVEATTRIBUTES_H
#define PY_INTEGRALCURVEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <IntegralCurveAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define INTEGRALCURVEATTRIBUTES_NMETH 144
void           PyIntegralCurveAttributes_StartUp(IntegralCurveAttributes *subj, void *data);
void           PyIntegralCurveAttributes_CloseDown();
PyMethodDef *  PyIntegralCurveAttributes_GetMethodTable(int *nMethods);
bool           PyIntegralCurveAttributes_Check(PyObject *obj);
IntegralCurveAttributes *  PyIntegralCurveAttributes_FromPyObject(PyObject *obj);
PyObject *     PyIntegralCurveAttributes_New();
PyObject *     PyIntegralCurveAttributes_Wrap(const IntegralCurveAttributes *attr);
void           PyIntegralCurveAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyIntegralCurveAttributes_SetDefaults(const IntegralCurveAttributes *atts);
std::string    PyIntegralCurveAttributes_GetLogString();
std::string    PyIntegralCurveAttributes_ToString(const IntegralCurveAttributes *, const char *, const bool=false);
PyObject *     PyIntegralCurveAttributes_getattr(PyObject *self, char *name);
int            PyIntegralCurveAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyIntegralCurveAttributes_methods[INTEGRALCURVEATTRIBUTES_NMETH];

#endif

