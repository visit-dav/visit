#ifndef PY_LIGHTATTRIBUTES_H
#define PY_LIGHTATTRIBUTES_H
#include <Python.h>
#include <LightAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyLightAttributes_StartUp(LightAttributes *subj, FILE *logFile);
void            PyLightAttributes_CloseDown();
PyMethodDef    *PyLightAttributes_GetMethodTable(int *nMethods);
bool            PyLightAttributes_Check(PyObject *obj);
LightAttributes *PyLightAttributes_FromPyObject(PyObject *obj);
PyObject       *PyLightAttributes_NewPyObject();
PyObject       *PyLightAttributes_WrapPyObject(const LightAttributes *attr);
void            PyLightAttributes_SetLogging(bool val);
void            PyLightAttributes_SetDefaults(const LightAttributes *atts);

PyObject       *PyLightAttributes_StringRepresentation(const LightAttributes *atts);

#endif

