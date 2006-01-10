#ifndef PY_INTERACTORATTRIBUTES_H
#define PY_INTERACTORATTRIBUTES_H
#include <Python.h>
#include <InteractorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyInteractorAttributes_StartUp(InteractorAttributes *subj, void *data);
void            PyInteractorAttributes_CloseDown();
PyMethodDef    *PyInteractorAttributes_GetMethodTable(int *nMethods);
bool            PyInteractorAttributes_Check(PyObject *obj);
InteractorAttributes *PyInteractorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyInteractorAttributes_NewPyObject();
PyObject       *PyInteractorAttributes_WrapPyObject(const InteractorAttributes *attr);
void            PyInteractorAttributes_SetDefaults(const InteractorAttributes *atts);
std::string     PyInteractorAttributes_GetLogString();
std::string     PyInteractorAttributes_ToString(const InteractorAttributes *, const char *);

#endif

