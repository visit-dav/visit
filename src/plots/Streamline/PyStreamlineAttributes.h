#ifndef PY_STREAMLINEATTRIBUTES_H
#define PY_STREAMLINEATTRIBUTES_H
#include <Python.h>
#include <StreamlineAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyStreamlineAttributes_StartUp(StreamlineAttributes *subj, FILE *logFile);
void            PyStreamlineAttributes_CloseDown();
PyMethodDef    *PyStreamlineAttributes_GetMethodTable(int *nMethods);
bool            PyStreamlineAttributes_Check(PyObject *obj);
StreamlineAttributes *PyStreamlineAttributes_FromPyObject(PyObject *obj);
PyObject       *PyStreamlineAttributes_NewPyObject();
PyObject       *PyStreamlineAttributes_WrapPyObject(const StreamlineAttributes *attr);
void            PyStreamlineAttributes_SetLogging(bool val);
void            PyStreamlineAttributes_SetDefaults(const StreamlineAttributes *atts);

#endif

