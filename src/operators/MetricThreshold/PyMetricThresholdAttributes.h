#ifndef PY_METRICTHRESHOLDATTRIBUTES_H
#define PY_METRICTHRESHOLDATTRIBUTES_H
#include <Python.h>
#include <MetricThresholdAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyMetricThresholdAttributes_StartUp(MetricThresholdAttributes *subj, void *data);
void            PyMetricThresholdAttributes_CloseDown();
PyMethodDef    *PyMetricThresholdAttributes_GetMethodTable(int *nMethods);
bool            PyMetricThresholdAttributes_Check(PyObject *obj);
MetricThresholdAttributes *PyMetricThresholdAttributes_FromPyObject(PyObject *obj);
PyObject       *PyMetricThresholdAttributes_NewPyObject();
PyObject       *PyMetricThresholdAttributes_WrapPyObject(const MetricThresholdAttributes *attr);
std::string     PyMetricThresholdAttributes_GetLogString();
void            PyMetricThresholdAttributes_SetDefaults(const MetricThresholdAttributes *atts);

#endif

