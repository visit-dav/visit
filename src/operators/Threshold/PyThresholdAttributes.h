// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_THRESHOLDATTRIBUTES_H
#define PY_THRESHOLDATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ThresholdAttributes.h>
#include <PyThresholdOpAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PyThresholdAttributes_StartUp(ThresholdAttributes *subj, void *data);
void           PyThresholdAttributes_CloseDown();
PyMethodDef *  PyThresholdAttributes_GetMethodTable(int *nMethods);
bool           PyThresholdAttributes_Check(PyObject *obj);
ThresholdAttributes *  PyThresholdAttributes_FromPyObject(PyObject *obj);
PyObject *     PyThresholdAttributes_New();
PyObject *     PyThresholdAttributes_Wrap(const ThresholdAttributes *attr);
void           PyThresholdAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyThresholdAttributes_SetDefaults(const ThresholdAttributes *atts);
std::string    PyThresholdAttributes_GetLogString();
std::string    PyThresholdAttributes_ToString(const ThresholdAttributes *, const char *, const bool=false);

#endif

