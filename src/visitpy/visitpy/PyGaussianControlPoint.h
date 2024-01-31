// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_GAUSSIANCONTROLPOINT_H
#define PY_GAUSSIANCONTROLPOINT_H
#include <Python.h>
#include <Py2and3Support.h>
#include <GaussianControlPoint.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define GAUSSIANCONTROLPOINT_NMETH 12
void VISITPY_API           PyGaussianControlPoint_StartUp(GaussianControlPoint *subj, void *data);
void VISITPY_API           PyGaussianControlPoint_CloseDown();
VISITPY_API PyMethodDef *  PyGaussianControlPoint_GetMethodTable(int *nMethods);
bool VISITPY_API           PyGaussianControlPoint_Check(PyObject *obj);
VISITPY_API GaussianControlPoint *  PyGaussianControlPoint_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyGaussianControlPoint_New();
VISITPY_API PyObject *     PyGaussianControlPoint_Wrap(const GaussianControlPoint *attr);
void VISITPY_API           PyGaussianControlPoint_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyGaussianControlPoint_SetDefaults(const GaussianControlPoint *atts);
std::string VISITPY_API    PyGaussianControlPoint_GetLogString();
std::string VISITPY_API    PyGaussianControlPoint_ToString(const GaussianControlPoint *, const char *, const bool=false);
VISITPY_API PyObject *     PyGaussianControlPoint_getattr(PyObject *self, char *name);
int VISITPY_API            PyGaussianControlPoint_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyGaussianControlPoint_methods[GAUSSIANCONTROLPOINT_NMETH];

#endif

