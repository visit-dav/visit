// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VIEWCURVEATTRIBUTES_H
#define PY_VIEWCURVEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ViewCurveAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define VIEWCURVEATTRIBUTES_NMETH 12
void VISITPY_API           PyViewCurveAttributes_StartUp(ViewCurveAttributes *subj, void *data);
void VISITPY_API           PyViewCurveAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyViewCurveAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyViewCurveAttributes_Check(PyObject *obj);
VISITPY_API ViewCurveAttributes *  PyViewCurveAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyViewCurveAttributes_New();
VISITPY_API PyObject *     PyViewCurveAttributes_Wrap(const ViewCurveAttributes *attr);
void VISITPY_API           PyViewCurveAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyViewCurveAttributes_SetDefaults(const ViewCurveAttributes *atts);
std::string VISITPY_API    PyViewCurveAttributes_GetLogString();
std::string VISITPY_API    PyViewCurveAttributes_ToString(const ViewCurveAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyViewCurveAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyViewCurveAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyViewCurveAttributes_methods[VIEWCURVEATTRIBUTES_NMETH];

#endif

