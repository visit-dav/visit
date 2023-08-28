// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_THRESHOLDOPATTRIBUTES_H
#define PY_THRESHOLDOPATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ThresholdOpAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define THRESHOLDOPATTRIBUTES_NMETH 20
void VISITPY_API           PyThresholdOpAttributes_StartUp(ThresholdOpAttributes *subj, void *data);
void VISITPY_API           PyThresholdOpAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyThresholdOpAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyThresholdOpAttributes_Check(PyObject *obj);
VISITPY_API ThresholdOpAttributes *  PyThresholdOpAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyThresholdOpAttributes_New();
VISITPY_API PyObject *     PyThresholdOpAttributes_Wrap(const ThresholdOpAttributes *attr);
void VISITPY_API           PyThresholdOpAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyThresholdOpAttributes_SetDefaults(const ThresholdOpAttributes *atts);
std::string VISITPY_API    PyThresholdOpAttributes_GetLogString();
std::string VISITPY_API    PyThresholdOpAttributes_ToString(const ThresholdOpAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyThresholdOpAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyThresholdOpAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyThresholdOpAttributes_methods[THRESHOLDOPATTRIBUTES_NMETH];

#endif

