#ifndef PY_KEYFRAMEATTRIBUTES_H
#define PY_KEYFRAMEATTRIBUTES_H
#include <Python.h>
#include <KeyframeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyKeyframeAttributes_StartUp(KeyframeAttributes *subj, FILE *logFile);
void            PyKeyframeAttributes_CloseDown();
PyMethodDef    *PyKeyframeAttributes_GetMethodTable(int *nMethods);
bool            PyKeyframeAttributes_Check(PyObject *obj);
KeyframeAttributes *PyKeyframeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyKeyframeAttributes_NewPyObject();
PyObject       *PyKeyframeAttributes_WrapPyObject(const KeyframeAttributes *attr);
void            PyKeyframeAttributes_SetLogging(bool val);
void            PyKeyframeAttributes_SetDefaults(const KeyframeAttributes *atts);

#endif

