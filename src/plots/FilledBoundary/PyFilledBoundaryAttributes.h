#ifndef PY_FILLEDBOUNDARYATTRIBUTES_H
#define PY_FILLEDBOUNDARYATTRIBUTES_H
#include <Python.h>
#include <FilledBoundaryAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyFilledBoundaryAttributes_StartUp(FilledBoundaryAttributes *subj, FILE *logFile);
void            PyFilledBoundaryAttributes_CloseDown();
PyMethodDef    *PyFilledBoundaryAttributes_GetMethodTable(int *nMethods);
bool            PyFilledBoundaryAttributes_Check(PyObject *obj);
FilledBoundaryAttributes *PyFilledBoundaryAttributes_FromPyObject(PyObject *obj);
PyObject       *PyFilledBoundaryAttributes_NewPyObject();
PyObject       *PyFilledBoundaryAttributes_WrapPyObject(const FilledBoundaryAttributes *attr);
void            PyFilledBoundaryAttributes_SetLogging(bool val);
void            PyFilledBoundaryAttributes_SetDefaults(const FilledBoundaryAttributes *atts);

#endif

