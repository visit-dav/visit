#ifndef PY_SMOOTHOPERATORATTRIBUTES_H
#define PY_SMOOTHOPERATORATTRIBUTES_H
#include <Python.h>
#include <SmoothOperatorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySmoothOperatorAttributes_StartUp(SmoothOperatorAttributes *subj, void *data);
void            PySmoothOperatorAttributes_CloseDown();
PyMethodDef    *PySmoothOperatorAttributes_GetMethodTable(int *nMethods);
bool            PySmoothOperatorAttributes_Check(PyObject *obj);
SmoothOperatorAttributes *PySmoothOperatorAttributes_FromPyObject(PyObject *obj);
PyObject       *PySmoothOperatorAttributes_NewPyObject();
PyObject       *PySmoothOperatorAttributes_WrapPyObject(const SmoothOperatorAttributes *attr);
std::string     PySmoothOperatorAttributes_GetLogString();
void            PySmoothOperatorAttributes_SetDefaults(const SmoothOperatorAttributes *atts);

#endif

