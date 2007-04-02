#ifndef PY_DEFEREXPRESSIONATTRIBUTES_H
#define PY_DEFEREXPRESSIONATTRIBUTES_H
#include <Python.h>
#include <DeferExpressionAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyDeferExpressionAttributes_StartUp(DeferExpressionAttributes *subj, FILE *logFile);
void            PyDeferExpressionAttributes_CloseDown();
PyMethodDef    *PyDeferExpressionAttributes_GetMethodTable(int *nMethods);
bool            PyDeferExpressionAttributes_Check(PyObject *obj);
DeferExpressionAttributes *PyDeferExpressionAttributes_FromPyObject(PyObject *obj);
PyObject       *PyDeferExpressionAttributes_NewPyObject();
PyObject       *PyDeferExpressionAttributes_WrapPyObject(const DeferExpressionAttributes *attr);
void            PyDeferExpressionAttributes_SetLogging(bool val);
void            PyDeferExpressionAttributes_SetDefaults(const DeferExpressionAttributes *atts);

PyObject       *PyDeferExpressionAttributes_StringRepresentation(const DeferExpressionAttributes *atts);

#endif

