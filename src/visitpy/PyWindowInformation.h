#ifndef PY_WINDOWINFORMATION_H
#define PY_WINDOWINFORMATION_H
#include <Python.h>
#include <WindowInformation.h>

//
// Functions exposed to the VisIt module.
//
void            PyWindowInformation_StartUp(WindowInformation *subj, FILE *logFile);
void            PyWindowInformation_CloseDown();
PyMethodDef    *PyWindowInformation_GetMethodTable(int *nMethods);
bool            PyWindowInformation_Check(PyObject *obj);
WindowInformation *PyWindowInformation_FromPyObject(PyObject *obj);
PyObject       *PyWindowInformation_NewPyObject();
PyObject       *PyWindowInformation_WrapPyObject(const WindowInformation *attr);
void            PyWindowInformation_SetLogging(bool val);
void            PyWindowInformation_SetDefaults(const WindowInformation *atts);

PyObject       *PyWindowInformation_StringRepresentation(const WindowInformation *atts);

#endif

