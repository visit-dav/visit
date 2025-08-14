// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_FXAAOPTIONS_H
#define PY_FXAAOPTIONS_H
#include <Python.h>
#include <Py2and3Support.h>
#include <FXAAOptions.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define FXAAOPTIONS_NMETH 23
void VISITPY_API           PyFXAAOptions_StartUp(FXAAOptions *subj, void *data);
void VISITPY_API           PyFXAAOptions_CloseDown();
VISITPY_API PyMethodDef *  PyFXAAOptions_GetMethodTable(int *nMethods);
bool VISITPY_API           PyFXAAOptions_Check(PyObject *obj);
VISITPY_API FXAAOptions *  PyFXAAOptions_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyFXAAOptions_New();
VISITPY_API PyObject *     PyFXAAOptions_Wrap(const FXAAOptions *attr);
void VISITPY_API           PyFXAAOptions_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyFXAAOptions_SetDefaults(const FXAAOptions *atts);
std::string VISITPY_API    PyFXAAOptions_GetLogString();
std::string VISITPY_API    PyFXAAOptions_ToString(const FXAAOptions *, const char *, const bool=false);
VISITPY_API PyObject *     PyFXAAOptions_getattro(PyObject *self, PyObject *attr_name);
int VISITPY_API            PyFXAAOptions_setattro(PyObject *self, PyObject *attr_name, PyObject *args);
VISITPY_API extern PyMethodDef PyFXAAOptions_methods[FXAAOPTIONS_NMETH];

#endif

