#ifndef PY_BOUNDARYATTRIBUTES_H
#define PY_BOUNDARYATTRIBUTES_H
#include <Python.h>
#include <BoundaryAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyBoundaryAttributes_StartUp(BoundaryAttributes *subj, FILE *logFile);
void            PyBoundaryAttributes_CloseDown();
PyMethodDef    *PyBoundaryAttributes_GetMethodTable(int *nMethods);
bool            PyBoundaryAttributes_Check(PyObject *obj);
BoundaryAttributes *PyBoundaryAttributes_FromPyObject(PyObject *obj);
PyObject       *PyBoundaryAttributes_NewPyObject();
PyObject       *PyBoundaryAttributes_WrapPyObject(const BoundaryAttributes *attr);
void            PyBoundaryAttributes_SetLogging(bool val);
void            PyBoundaryAttributes_SetDefaults(const BoundaryAttributes *atts);

PyObject       *PyBoundaryAttributes_StringRepresentation(const BoundaryAttributes *atts);

#endif

