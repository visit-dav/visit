#ifndef PY_VECTORATTRIBUTES_H
#define PY_VECTORATTRIBUTES_H
#include <Python.h>
#include <VectorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyVectorAttributes_StartUp(VectorAttributes *subj, FILE *logFile);
void            PyVectorAttributes_CloseDown();
PyMethodDef    *PyVectorAttributes_GetMethodTable(int *nMethods);
bool            PyVectorAttributes_Check(PyObject *obj);
VectorAttributes *PyVectorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyVectorAttributes_NewPyObject();
PyObject       *PyVectorAttributes_WrapPyObject(const VectorAttributes *attr);
void            PyVectorAttributes_SetLogging(bool val);
void            PyVectorAttributes_SetDefaults(const VectorAttributes *atts);

#endif

