// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_GAUSSIANCONTROLPOINTLIST_H
#define PY_GAUSSIANCONTROLPOINTLIST_H
#include <Python.h>
#include <Py2and3Support.h>
#include <GaussianControlPointList.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define GAUSSIANCONTROLPOINTLIST_NMETH 8
void VISITPY_API           PyGaussianControlPointList_StartUp(GaussianControlPointList *subj, void *data);
void VISITPY_API           PyGaussianControlPointList_CloseDown();
VISITPY_API PyMethodDef *  PyGaussianControlPointList_GetMethodTable(int *nMethods);
bool VISITPY_API           PyGaussianControlPointList_Check(PyObject *obj);
VISITPY_API GaussianControlPointList *  PyGaussianControlPointList_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyGaussianControlPointList_New();
VISITPY_API PyObject *     PyGaussianControlPointList_Wrap(const GaussianControlPointList *attr);
void VISITPY_API           PyGaussianControlPointList_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyGaussianControlPointList_SetDefaults(const GaussianControlPointList *atts);
std::string VISITPY_API    PyGaussianControlPointList_GetLogString();
std::string VISITPY_API    PyGaussianControlPointList_ToString(const GaussianControlPointList *, const char *, const bool=false);
VISITPY_API PyObject *     PyGaussianControlPointList_getattr(PyObject *self, char *name);
int VISITPY_API            PyGaussianControlPointList_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyGaussianControlPointList_methods[GAUSSIANCONTROLPOINTLIST_NMETH];

#endif

