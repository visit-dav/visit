#ifndef PY_PROJECTATTRIBUTES_H
#define PY_PROJECTATTRIBUTES_H
#include <Python.h>
#include <ProjectAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyProjectAttributes_StartUp(ProjectAttributes *subj, void *data);
void            PyProjectAttributes_CloseDown();
PyMethodDef    *PyProjectAttributes_GetMethodTable(int *nMethods);
bool            PyProjectAttributes_Check(PyObject *obj);
ProjectAttributes *PyProjectAttributes_FromPyObject(PyObject *obj);
PyObject       *PyProjectAttributes_NewPyObject();
PyObject       *PyProjectAttributes_WrapPyObject(const ProjectAttributes *attr);
std::string     PyProjectAttributes_GetLogString();
void            PyProjectAttributes_SetDefaults(const ProjectAttributes *atts);

#endif

