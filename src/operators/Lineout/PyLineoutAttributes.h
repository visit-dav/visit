#ifndef PY_LINEOUTATTRIBUTES_H
#define PY_LINEOUTATTRIBUTES_H
#include <Python.h>
#include <LineoutAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyLineoutAttributes_StartUp(LineoutAttributes *subj, void *data);
void            PyLineoutAttributes_CloseDown();
PyMethodDef    *PyLineoutAttributes_GetMethodTable(int *nMethods);
bool            PyLineoutAttributes_Check(PyObject *obj);
LineoutAttributes *PyLineoutAttributes_FromPyObject(PyObject *obj);
PyObject       *PyLineoutAttributes_NewPyObject();
PyObject       *PyLineoutAttributes_WrapPyObject(const LineoutAttributes *attr);
std::string     PyLineoutAttributes_GetLogString();
void            PyLineoutAttributes_SetDefaults(const LineoutAttributes *atts);

#endif

