// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXISTITLES_H
#define PY_AXISTITLES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AxisTitles.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AXISTITLES_NMETH 14
void VISITPY_API           PyAxisTitles_StartUp(AxisTitles *subj, void *data);
void VISITPY_API           PyAxisTitles_CloseDown();
VISITPY_API PyMethodDef *  PyAxisTitles_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxisTitles_Check(PyObject *obj);
VISITPY_API AxisTitles *  PyAxisTitles_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxisTitles_New();
VISITPY_API PyObject *     PyAxisTitles_Wrap(const AxisTitles *attr);
void VISITPY_API           PyAxisTitles_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxisTitles_SetDefaults(const AxisTitles *atts);
std::string VISITPY_API    PyAxisTitles_GetLogString();
std::string VISITPY_API    PyAxisTitles_ToString(const AxisTitles *, const char *, const bool=false);
VISITPY_API PyObject *     PyAxisTitles_getattr(PyObject *self, char *name);
int VISITPY_API            PyAxisTitles_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAxisTitles_methods[AXISTITLES_NMETH];

#endif

