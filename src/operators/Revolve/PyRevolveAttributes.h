#ifndef PY_REVOLVEATTRIBUTES_H
#define PY_REVOLVEATTRIBUTES_H
#include <Python.h>
#include <RevolveAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyRevolveAttributes_StartUp(RevolveAttributes *subj, void *data);
void            PyRevolveAttributes_CloseDown();
PyMethodDef    *PyRevolveAttributes_GetMethodTable(int *nMethods);
bool            PyRevolveAttributes_Check(PyObject *obj);
RevolveAttributes *PyRevolveAttributes_FromPyObject(PyObject *obj);
PyObject       *PyRevolveAttributes_NewPyObject();
PyObject       *PyRevolveAttributes_WrapPyObject(const RevolveAttributes *attr);
std::string     PyRevolveAttributes_GetLogString();
void            PyRevolveAttributes_SetDefaults(const RevolveAttributes *atts);

#endif

