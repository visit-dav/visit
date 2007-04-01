#ifndef PY_GLOBALATTRIBUTES_H
#define PY_GLOBALATTRIBUTES_H
#include <Python.h>
#include <GlobalAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyGlobalAttributes_StartUp(GlobalAttributes *subj, FILE *logFile);
void            PyGlobalAttributes_CloseDown();
PyMethodDef    *PyGlobalAttributes_GetMethodTable(int *nMethods);
bool            PyGlobalAttributes_Check(PyObject *obj);
GlobalAttributes *PyGlobalAttributes_FromPyObject(PyObject *obj);
PyObject       *PyGlobalAttributes_NewPyObject();
PyObject       *PyGlobalAttributes_WrapPyObject(const GlobalAttributes *attr);
void            PyGlobalAttributes_SetLogging(bool val);
void            PyGlobalAttributes_SetDefaults(const GlobalAttributes *atts);

PyObject       *PyGlobalAttributes_StringRepresentation(const GlobalAttributes *atts);

#endif

