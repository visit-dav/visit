#ifndef PY_REMOVECELLSATTRIBUTES_H
#define PY_REMOVECELLSATTRIBUTES_H
#include <Python.h>
#include <RemoveCellsAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyRemoveCellsAttributes_StartUp(RemoveCellsAttributes *subj, void *data);
void            PyRemoveCellsAttributes_CloseDown();
PyMethodDef    *PyRemoveCellsAttributes_GetMethodTable(int *nMethods);
bool            PyRemoveCellsAttributes_Check(PyObject *obj);
RemoveCellsAttributes *PyRemoveCellsAttributes_FromPyObject(PyObject *obj);
PyObject       *PyRemoveCellsAttributes_NewPyObject();
PyObject       *PyRemoveCellsAttributes_WrapPyObject(const RemoveCellsAttributes *attr);
std::string     PyRemoveCellsAttributes_GetLogString();
void            PyRemoveCellsAttributes_SetDefaults(const RemoveCellsAttributes *atts);

#endif

