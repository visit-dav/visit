#ifndef PY_CONTEXTATTRIBUTES_H
#define PY_CONTEXTATTRIBUTES_H
#include <Python.h>
#include <ContextAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyContextAttributes_StartUp(ContextAttributes *subj, FILE *logFile);
void            PyContextAttributes_CloseDown();
PyMethodDef    *PyContextAttributes_GetMethodTable(int *nMethods);
bool            PyContextAttributes_Check(PyObject *obj);
ContextAttributes *PyContextAttributes_FromPyObject(PyObject *obj);
PyObject       *PyContextAttributes_NewPyObject();
PyObject       *PyContextAttributes_WrapPyObject(const ContextAttributes *attr);
void            PyContextAttributes_SetLogging(bool val);
void            PyContextAttributes_SetDefaults(const ContextAttributes *atts);

#endif

