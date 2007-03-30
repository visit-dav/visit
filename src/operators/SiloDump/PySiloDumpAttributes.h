#ifndef PY_SILODUMPATTRIBUTES_H
#define PY_SILODUMPATTRIBUTES_H
#include <Python.h>
#include <SiloDumpAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySiloDumpAttributes_StartUp(SiloDumpAttributes *subj, FILE *logFile);
void            PySiloDumpAttributes_CloseDown();
PyMethodDef    *PySiloDumpAttributes_GetMethodTable(int *nMethods);
bool            PySiloDumpAttributes_Check(PyObject *obj);
SiloDumpAttributes *PySiloDumpAttributes_FromPyObject(PyObject *obj);
PyObject       *PySiloDumpAttributes_NewPyObject();
PyObject       *PySiloDumpAttributes_WrapPyObject(const SiloDumpAttributes *attr);
void            PySiloDumpAttributes_SetLogging(bool val);
void            PySiloDumpAttributes_SetDefaults(const SiloDumpAttributes *atts);

#endif

