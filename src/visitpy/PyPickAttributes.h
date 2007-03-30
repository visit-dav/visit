#ifndef PY_PICKATTRIBUTES_H
#define PY_PICKATTRIBUTES_H
#include <Python.h>
#include <PickAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyPickAttributes_StartUp(PickAttributes *subj, FILE *logFile);
void            PyPickAttributes_CloseDown();
PyMethodDef    *PyPickAttributes_GetMethodTable(int *nMethods);
bool            PyPickAttributes_Check(PyObject *obj);
PickAttributes *PyPickAttributes_FromPyObject(PyObject *obj);
PyObject       *PyPickAttributes_NewPyObject();
PyObject       *PyPickAttributes_WrapPyObject(const PickAttributes *attr);
void            PyPickAttributes_SetLogging(bool val);
void            PyPickAttributes_SetDefaults(const PickAttributes *atts);

#endif

