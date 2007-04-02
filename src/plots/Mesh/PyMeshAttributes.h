#ifndef PY_MESHATTRIBUTES_H
#define PY_MESHATTRIBUTES_H
#include <Python.h>
#include <MeshAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyMeshAttributes_StartUp(MeshAttributes *subj, void *data);
void            PyMeshAttributes_CloseDown();
PyMethodDef    *PyMeshAttributes_GetMethodTable(int *nMethods);
bool            PyMeshAttributes_Check(PyObject *obj);
MeshAttributes *PyMeshAttributes_FromPyObject(PyObject *obj);
PyObject       *PyMeshAttributes_NewPyObject();
PyObject       *PyMeshAttributes_WrapPyObject(const MeshAttributes *attr);
std::string     PyMeshAttributes_GetLogString();
void            PyMeshAttributes_SetDefaults(const MeshAttributes *atts);

#endif

