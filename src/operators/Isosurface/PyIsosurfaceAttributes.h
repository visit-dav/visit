#ifndef PY_ISOSURFACEATTRIBUTES_H
#define PY_ISOSURFACEATTRIBUTES_H
#include <Python.h>
#include <IsosurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyIsosurfaceAttributes_StartUp(IsosurfaceAttributes *subj, void *data);
void            PyIsosurfaceAttributes_CloseDown();
PyMethodDef    *PyIsosurfaceAttributes_GetMethodTable(int *nMethods);
bool            PyIsosurfaceAttributes_Check(PyObject *obj);
IsosurfaceAttributes *PyIsosurfaceAttributes_FromPyObject(PyObject *obj);
PyObject       *PyIsosurfaceAttributes_NewPyObject();
PyObject       *PyIsosurfaceAttributes_WrapPyObject(const IsosurfaceAttributes *attr);
std::string     PyIsosurfaceAttributes_GetLogString();
void            PyIsosurfaceAttributes_SetDefaults(const IsosurfaceAttributes *atts);

#endif

