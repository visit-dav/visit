#ifndef PY_DEFEREXPRESSIONATTRIBUTES_H
#define PY_DEFEREXPRESSIONATTRIBUTES_H
#include <Python.h>
#include <DeferExpressionAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyDeferExpressionAttributes_StartUp(DeferExpressionAttributes *subj, void *data);
void            PyDeferExpressionAttributes_CloseDown();
PyMethodDef    *PyDeferExpressionAttributes_GetMethodTable(int *nMethods);
bool            PyDeferExpressionAttributes_Check(PyObject *obj);
DeferExpressionAttributes *PyDeferExpressionAttributes_FromPyObject(PyObject *obj);
PyObject       *PyDeferExpressionAttributes_NewPyObject();
PyObject       *PyDeferExpressionAttributes_WrapPyObject(const DeferExpressionAttributes *attr);
std::string     PyDeferExpressionAttributes_GetLogString();
void            PyDeferExpressionAttributes_SetDefaults(const DeferExpressionAttributes *atts);

#endif

