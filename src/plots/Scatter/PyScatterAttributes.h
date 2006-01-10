#ifndef PY_SCATTERATTRIBUTES_H
#define PY_SCATTERATTRIBUTES_H
#include <Python.h>
#include <ScatterAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyScatterAttributes_StartUp(ScatterAttributes *subj, void *data);
void            PyScatterAttributes_CloseDown();
PyMethodDef    *PyScatterAttributes_GetMethodTable(int *nMethods);
bool            PyScatterAttributes_Check(PyObject *obj);
ScatterAttributes *PyScatterAttributes_FromPyObject(PyObject *obj);
PyObject       *PyScatterAttributes_NewPyObject();
PyObject       *PyScatterAttributes_WrapPyObject(const ScatterAttributes *attr);
std::string     PyScatterAttributes_GetLogString();
void            PyScatterAttributes_SetDefaults(const ScatterAttributes *atts);

#endif

