#ifndef PY_LINEOUTATTRIBUTES_H
#define PY_LINEOUTATTRIBUTES_H
#include <Python.h>
#include <LineoutAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyLineoutAttributes_StartUp(LineoutAttributes *subj, FILE *logFile);
void            PyLineoutAttributes_CloseDown();
PyMethodDef    *PyLineoutAttributes_GetMethodTable(int *nMethods);
bool            PyLineoutAttributes_Check(PyObject *obj);
LineoutAttributes *PyLineoutAttributes_FromPyObject(PyObject *obj);
PyObject       *PyLineoutAttributes_NewPyObject();
PyObject       *PyLineoutAttributes_WrapPyObject(const LineoutAttributes *attr);
void            PyLineoutAttributes_SetLogging(bool val);
void            PyLineoutAttributes_SetDefaults(const LineoutAttributes *atts);

PyObject       *PyLineoutAttributes_StringRepresentation(const LineoutAttributes *atts);

#endif

