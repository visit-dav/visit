#ifndef PY_REFLECTATTRIBUTES_H
#define PY_REFLECTATTRIBUTES_H
#include <Python.h>
#include <ReflectAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyReflectAttributes_StartUp(ReflectAttributes *subj, FILE *logFile);
void            PyReflectAttributes_CloseDown();
PyMethodDef    *PyReflectAttributes_GetMethodTable(int *nMethods);
bool            PyReflectAttributes_Check(PyObject *obj);
ReflectAttributes *PyReflectAttributes_FromPyObject(PyObject *obj);
PyObject       *PyReflectAttributes_NewPyObject();
PyObject       *PyReflectAttributes_WrapPyObject(const ReflectAttributes *attr);
void            PyReflectAttributes_SetLogging(bool val);
void            PyReflectAttributes_SetDefaults(const ReflectAttributes *atts);

#endif

