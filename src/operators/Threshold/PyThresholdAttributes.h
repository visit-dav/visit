#ifndef PY_THRESHOLDATTRIBUTES_H
#define PY_THRESHOLDATTRIBUTES_H
#include <Python.h>
#include <ThresholdAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyThresholdAttributes_StartUp(ThresholdAttributes *subj, FILE *logFile);
void            PyThresholdAttributes_CloseDown();
PyMethodDef    *PyThresholdAttributes_GetMethodTable(int *nMethods);
bool            PyThresholdAttributes_Check(PyObject *obj);
ThresholdAttributes *PyThresholdAttributes_FromPyObject(PyObject *obj);
PyObject       *PyThresholdAttributes_NewPyObject();
PyObject       *PyThresholdAttributes_WrapPyObject(const ThresholdAttributes *attr);
void            PyThresholdAttributes_SetLogging(bool val);
void            PyThresholdAttributes_SetDefaults(const ThresholdAttributes *atts);

PyObject       *PyThresholdAttributes_StringRepresentation(const ThresholdAttributes *atts);

#endif

