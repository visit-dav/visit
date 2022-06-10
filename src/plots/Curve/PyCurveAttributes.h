// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CURVEATTRIBUTES_H
#define PY_CURVEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <CurveAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CURVEATTRIBUTES_NMETH 56
void           PyCurveAttributes_StartUp(CurveAttributes *subj, void *data);
void           PyCurveAttributes_CloseDown();
PyMethodDef *  PyCurveAttributes_GetMethodTable(int *nMethods);
bool           PyCurveAttributes_Check(PyObject *obj);
CurveAttributes *  PyCurveAttributes_FromPyObject(PyObject *obj);
PyObject *     PyCurveAttributes_New();
PyObject *     PyCurveAttributes_Wrap(const CurveAttributes *attr);
void           PyCurveAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyCurveAttributes_SetDefaults(const CurveAttributes *atts);
std::string    PyCurveAttributes_GetLogString();
std::string    PyCurveAttributes_ToString(const CurveAttributes *, const char *, const bool=false);
PyObject *     PyCurveAttributes_getattr(PyObject *self, char *name);
int            PyCurveAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyCurveAttributes_methods[CURVEATTRIBUTES_NMETH];

#endif

