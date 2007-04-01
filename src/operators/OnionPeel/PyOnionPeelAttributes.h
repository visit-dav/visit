#ifndef PY_ONIONPEELATTRIBUTES_H
#define PY_ONIONPEELATTRIBUTES_H
#include <Python.h>
#include <OnionPeelAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyOnionPeelAttributes_StartUp(OnionPeelAttributes *subj, FILE *logFile);
void            PyOnionPeelAttributes_CloseDown();
PyMethodDef    *PyOnionPeelAttributes_GetMethodTable(int *nMethods);
bool            PyOnionPeelAttributes_Check(PyObject *obj);
OnionPeelAttributes *PyOnionPeelAttributes_FromPyObject(PyObject *obj);
PyObject       *PyOnionPeelAttributes_NewPyObject();
PyObject       *PyOnionPeelAttributes_WrapPyObject(const OnionPeelAttributes *attr);
void            PyOnionPeelAttributes_SetLogging(bool val);
void            PyOnionPeelAttributes_SetDefaults(const OnionPeelAttributes *atts);

PyObject       *PyOnionPeelAttributes_StringRepresentation(const OnionPeelAttributes *atts);

#endif

