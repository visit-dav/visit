#ifndef PY_EXTERNALSURFACEATTRIBUTES_H
#define PY_EXTERNALSURFACEATTRIBUTES_H
#include <Python.h>
#include <ExternalSurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyExternalSurfaceAttributes_StartUp(ExternalSurfaceAttributes *subj, void *data);
void            PyExternalSurfaceAttributes_CloseDown();
PyMethodDef    *PyExternalSurfaceAttributes_GetMethodTable(int *nMethods);
bool            PyExternalSurfaceAttributes_Check(PyObject *obj);
ExternalSurfaceAttributes *PyExternalSurfaceAttributes_FromPyObject(PyObject *obj);
PyObject       *PyExternalSurfaceAttributes_NewPyObject();
PyObject       *PyExternalSurfaceAttributes_WrapPyObject(const ExternalSurfaceAttributes *attr);
std::string     PyExternalSurfaceAttributes_GetLogString();
void            PyExternalSurfaceAttributes_SetDefaults(const ExternalSurfaceAttributes *atts);

#endif

