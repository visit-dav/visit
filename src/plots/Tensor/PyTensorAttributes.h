#ifndef PY_TENSORATTRIBUTES_H
#define PY_TENSORATTRIBUTES_H
#include <Python.h>
#include <TensorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyTensorAttributes_StartUp(TensorAttributes *subj, void *data);
void            PyTensorAttributes_CloseDown();
PyMethodDef    *PyTensorAttributes_GetMethodTable(int *nMethods);
bool            PyTensorAttributes_Check(PyObject *obj);
TensorAttributes *PyTensorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyTensorAttributes_NewPyObject();
PyObject       *PyTensorAttributes_WrapPyObject(const TensorAttributes *attr);
std::string     PyTensorAttributes_GetLogString();
void            PyTensorAttributes_SetDefaults(const TensorAttributes *atts);

#endif

