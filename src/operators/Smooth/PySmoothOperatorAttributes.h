#ifndef PY_SMOOTHOPERATORATTRIBUTES_H
#define PY_SMOOTHOPERATORATTRIBUTES_H
#include <Python.h>
#include <SmoothOperatorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySmoothOperatorAttributes_StartUp(SmoothOperatorAttributes *subj, FILE *logFile);
void            PySmoothOperatorAttributes_CloseDown();
PyMethodDef    *PySmoothOperatorAttributes_GetMethodTable(int *nMethods);
bool            PySmoothOperatorAttributes_Check(PyObject *obj);
SmoothOperatorAttributes *PySmoothOperatorAttributes_FromPyObject(PyObject *obj);
PyObject       *PySmoothOperatorAttributes_NewPyObject();
PyObject       *PySmoothOperatorAttributes_WrapPyObject(const SmoothOperatorAttributes *attr);
void            PySmoothOperatorAttributes_SetLogging(bool val);
void            PySmoothOperatorAttributes_SetDefaults(const SmoothOperatorAttributes *atts);

PyObject       *PySmoothOperatorAttributes_StringRepresentation(const SmoothOperatorAttributes *atts);

#endif

