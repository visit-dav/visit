// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_METRICTHRESHOLDATTRIBUTES_H
#define PY_METRICTHRESHOLDATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <MetricThresholdAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define METRICTHRESHOLDATTRIBUTES_NMETH 40
void           PyMetricThresholdAttributes_StartUp(MetricThresholdAttributes *subj, void *data);
void           PyMetricThresholdAttributes_CloseDown();
PyMethodDef *  PyMetricThresholdAttributes_GetMethodTable(int *nMethods);
bool           PyMetricThresholdAttributes_Check(PyObject *obj);
MetricThresholdAttributes *  PyMetricThresholdAttributes_FromPyObject(PyObject *obj);
PyObject *     PyMetricThresholdAttributes_New();
PyObject *     PyMetricThresholdAttributes_Wrap(const MetricThresholdAttributes *attr);
void           PyMetricThresholdAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyMetricThresholdAttributes_SetDefaults(const MetricThresholdAttributes *atts);
std::string    PyMetricThresholdAttributes_GetLogString();
std::string    PyMetricThresholdAttributes_ToString(const MetricThresholdAttributes *, const char *, const bool=false);
PyObject *     PyMetricThresholdAttributes_getattr(PyObject *self, char *name);
int            PyMetricThresholdAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyMetricThresholdAttributes_methods[METRICTHRESHOLDATTRIBUTES_NMETH];

#endif

