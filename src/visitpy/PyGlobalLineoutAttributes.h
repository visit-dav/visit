#ifndef PY_GLOBALLINEOUTATTRIBUTES_H
#define PY_GLOBALLINEOUTATTRIBUTES_H
#include <Python.h>
#include <GlobalLineoutAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyGlobalLineoutAttributes_StartUp(GlobalLineoutAttributes *subj, FILE *logFile);
void            PyGlobalLineoutAttributes_CloseDown();
PyMethodDef    *PyGlobalLineoutAttributes_GetMethodTable(int *nMethods);
bool            PyGlobalLineoutAttributes_Check(PyObject *obj);
GlobalLineoutAttributes *PyGlobalLineoutAttributes_FromPyObject(PyObject *obj);
PyObject       *PyGlobalLineoutAttributes_NewPyObject();
PyObject       *PyGlobalLineoutAttributes_WrapPyObject(const GlobalLineoutAttributes *attr);
void            PyGlobalLineoutAttributes_SetLogging(bool val);
void            PyGlobalLineoutAttributes_SetDefaults(const GlobalLineoutAttributes *atts);

PyObject       *PyGlobalLineoutAttributes_StringRepresentation(const GlobalLineoutAttributes *atts);

#endif

