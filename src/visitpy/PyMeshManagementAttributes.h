#ifndef PY_MESHMANAGEMENTATTRIBUTES_H
#define PY_MESHMANAGEMENTATTRIBUTES_H
#include <Python.h>
#include <MeshManagementAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyMeshManagementAttributes_StartUp(MeshManagementAttributes *subj, FILE *logFile);
void            PyMeshManagementAttributes_CloseDown();
PyMethodDef    *PyMeshManagementAttributes_GetMethodTable(int *nMethods);
bool            PyMeshManagementAttributes_Check(PyObject *obj);
MeshManagementAttributes *PyMeshManagementAttributes_FromPyObject(PyObject *obj);
PyObject       *PyMeshManagementAttributes_NewPyObject();
PyObject       *PyMeshManagementAttributes_WrapPyObject(const MeshManagementAttributes *attr);
void            PyMeshManagementAttributes_SetLogging(bool val);
void            PyMeshManagementAttributes_SetDefaults(const MeshManagementAttributes *atts);

PyObject       *PyMeshManagementAttributes_StringRepresentation(const MeshManagementAttributes *atts);

#endif

