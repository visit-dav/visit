#ifndef PY_REMOVECELLSATTRIBUTES_H
#define PY_REMOVECELLSATTRIBUTES_H
#include <Python.h>
#include <RemoveCellsAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyRemoveCellsAttributes_StartUp(RemoveCellsAttributes *subj, FILE *logFile);
void            PyRemoveCellsAttributes_CloseDown();
PyMethodDef    *PyRemoveCellsAttributes_GetMethodTable(int *nMethods);
bool            PyRemoveCellsAttributes_Check(PyObject *obj);
RemoveCellsAttributes *PyRemoveCellsAttributes_FromPyObject(PyObject *obj);
PyObject       *PyRemoveCellsAttributes_NewPyObject();
PyObject       *PyRemoveCellsAttributes_WrapPyObject(const RemoveCellsAttributes *attr);
void            PyRemoveCellsAttributes_SetLogging(bool val);
void            PyRemoveCellsAttributes_SetDefaults(const RemoveCellsAttributes *atts);

#endif

