#ifndef PY_TRANSFORMATTRIBUTES_H
#define PY_TRANSFORMATTRIBUTES_H
#include <Python.h>
#include <TransformAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyTransformAttributes_StartUp(TransformAttributes *subj, void *data);
void            PyTransformAttributes_CloseDown();
PyMethodDef    *PyTransformAttributes_GetMethodTable(int *nMethods);
bool            PyTransformAttributes_Check(PyObject *obj);
TransformAttributes *PyTransformAttributes_FromPyObject(PyObject *obj);
PyObject       *PyTransformAttributes_NewPyObject();
PyObject       *PyTransformAttributes_WrapPyObject(const TransformAttributes *attr);
std::string     PyTransformAttributes_GetLogString();
void            PyTransformAttributes_SetDefaults(const TransformAttributes *atts);

#endif

