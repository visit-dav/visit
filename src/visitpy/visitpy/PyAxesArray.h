// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_AXESARRAY_H
#define PY_AXESARRAY_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AxesArray.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define AXESARRAY_NMETH 14
void VISITPY_API           PyAxesArray_StartUp(AxesArray *subj, void *data);
void VISITPY_API           PyAxesArray_CloseDown();
VISITPY_API PyMethodDef *  PyAxesArray_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAxesArray_Check(PyObject *obj);
VISITPY_API AxesArray *  PyAxesArray_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAxesArray_New();
VISITPY_API PyObject *     PyAxesArray_Wrap(const AxesArray *attr);
void VISITPY_API           PyAxesArray_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAxesArray_SetDefaults(const AxesArray *atts);
std::string VISITPY_API    PyAxesArray_GetLogString();
std::string VISITPY_API    PyAxesArray_ToString(const AxesArray *, const char *, const bool=false);
VISITPY_API PyObject *     PyAxesArray_getattr(PyObject *self, char *name);
int VISITPY_API            PyAxesArray_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAxesArray_methods[AXESARRAY_NMETH];

#endif

