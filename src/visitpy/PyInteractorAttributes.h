#ifndef PY_INTERACTORATTRIBUTES_H
#define PY_INTERACTORATTRIBUTES_H
#include <Python.h>
#include <InteractorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyInteractorAttributes_StartUp(InteractorAttributes *subj, FILE *logFile);
void            PyInteractorAttributes_CloseDown();
PyMethodDef    *PyInteractorAttributes_GetMethodTable(int *nMethods);
bool            PyInteractorAttributes_Check(PyObject *obj);
InteractorAttributes *PyInteractorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyInteractorAttributes_NewPyObject();
PyObject       *PyInteractorAttributes_WrapPyObject(const InteractorAttributes *attr);
void            PyInteractorAttributes_SetLogging(bool val);
void            PyInteractorAttributes_SetDefaults(const InteractorAttributes *atts);

PyObject       *PyInteractorAttributes_StringRepresentation(const InteractorAttributes *atts);

#endif

