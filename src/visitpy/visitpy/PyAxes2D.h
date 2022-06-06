// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXES2D_H
#define PY_AXES2D_H
#include <Python.h>
#include <Py2and3Support.h>
#include <Axes2D.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AXES2D_NMETH 18
void VISITPY_API           PyAxes2D_StartUp(Axes2D *subj, void *data);
void VISITPY_API           PyAxes2D_CloseDown();
VISITPY_API PyMethodDef *  PyAxes2D_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxes2D_Check(PyObject *obj);
VISITPY_API Axes2D *  PyAxes2D_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxes2D_New();
VISITPY_API PyObject *     PyAxes2D_Wrap(const Axes2D *attr);
void VISITPY_API           PyAxes2D_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxes2D_SetDefaults(const Axes2D *atts);
std::string VISITPY_API    PyAxes2D_GetLogString();
std::string VISITPY_API    PyAxes2D_ToString(const Axes2D *, const char *, const bool=false);
VISITPY_API PyObject *     PyAxes2D_getattr(PyObject *self, char *name);
int VISITPY_API            PyAxes2D_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAxes2D_methods[AXES2D_NMETH];

#endif

