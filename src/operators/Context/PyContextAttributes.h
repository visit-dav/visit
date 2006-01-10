#ifndef PY_CONTEXTATTRIBUTES_H
#define PY_CONTEXTATTRIBUTES_H
#include <Python.h>
#include <ContextAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyContextAttributes_StartUp(ContextAttributes *subj, void *data);
void            PyContextAttributes_CloseDown();
PyMethodDef    *PyContextAttributes_GetMethodTable(int *nMethods);
bool            PyContextAttributes_Check(PyObject *obj);
ContextAttributes *PyContextAttributes_FromPyObject(PyObject *obj);
PyObject       *PyContextAttributes_NewPyObject();
PyObject       *PyContextAttributes_WrapPyObject(const ContextAttributes *attr);
std::string     PyContextAttributes_GetLogString();
void            PyContextAttributes_SetDefaults(const ContextAttributes *atts);

#endif

