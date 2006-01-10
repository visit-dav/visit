#ifndef PY_CONEATTRIBUTES_H
#define PY_CONEATTRIBUTES_H
#include <Python.h>
#include <ConeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyConeAttributes_StartUp(ConeAttributes *subj, void *data);
void            PyConeAttributes_CloseDown();
PyMethodDef    *PyConeAttributes_GetMethodTable(int *nMethods);
bool            PyConeAttributes_Check(PyObject *obj);
ConeAttributes *PyConeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyConeAttributes_NewPyObject();
PyObject       *PyConeAttributes_WrapPyObject(const ConeAttributes *attr);
std::string     PyConeAttributes_GetLogString();
void            PyConeAttributes_SetDefaults(const ConeAttributes *atts);

#endif

