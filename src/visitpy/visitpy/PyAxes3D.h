// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXES3D_H
#define PY_AXES3D_H
#include <Python.h>
#include <Py2and3Support.h>
#include <Axes3D.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AXES3D_NMETH 40
void VISITPY_API           PyAxes3D_StartUp(Axes3D *subj, void *data);
void VISITPY_API           PyAxes3D_CloseDown();
VISITPY_API PyMethodDef *  PyAxes3D_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxes3D_Check(PyObject *obj);
VISITPY_API Axes3D *  PyAxes3D_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxes3D_New();
VISITPY_API PyObject *     PyAxes3D_Wrap(const Axes3D *attr);
void VISITPY_API           PyAxes3D_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxes3D_SetDefaults(const Axes3D *atts);
std::string VISITPY_API    PyAxes3D_GetLogString();
std::string VISITPY_API    PyAxes3D_ToString(const Axes3D *, const char *, const bool=false);
VISITPY_API PyObject *     PyAxes3D_getattr(PyObject *self, char *name);
int VISITPY_API            PyAxes3D_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAxes3D_methods[AXES3D_NMETH];

#endif

