#ifndef PY_PROJECTATTRIBUTES_H
#define PY_PROJECTATTRIBUTES_H
#include <Python.h>
#include <ProjectAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyProjectAttributes_StartUp(ProjectAttributes *subj, FILE *logFile);
void            PyProjectAttributes_CloseDown();
PyMethodDef    *PyProjectAttributes_GetMethodTable(int *nMethods);
bool            PyProjectAttributes_Check(PyObject *obj);
ProjectAttributes *PyProjectAttributes_FromPyObject(PyObject *obj);
PyObject       *PyProjectAttributes_NewPyObject();
PyObject       *PyProjectAttributes_WrapPyObject(const ProjectAttributes *attr);
void            PyProjectAttributes_SetLogging(bool val);
void            PyProjectAttributes_SetDefaults(const ProjectAttributes *atts);

PyObject       *PyProjectAttributes_StringRepresentation(const ProjectAttributes *atts);

#endif

