#ifndef PY_SUBSETATTRIBUTES_H
#define PY_SUBSETATTRIBUTES_H
#include <Python.h>
#include <SubsetAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySubsetAttributes_StartUp(SubsetAttributes *subj, FILE *logFile);
void            PySubsetAttributes_CloseDown();
PyMethodDef    *PySubsetAttributes_GetMethodTable(int *nMethods);
bool            PySubsetAttributes_Check(PyObject *obj);
SubsetAttributes *PySubsetAttributes_FromPyObject(PyObject *obj);
PyObject       *PySubsetAttributes_NewPyObject();
PyObject       *PySubsetAttributes_WrapPyObject(const SubsetAttributes *attr);
void            PySubsetAttributes_SetLogging(bool val);
void            PySubsetAttributes_SetDefaults(const SubsetAttributes *atts);

PyObject       *PySubsetAttributes_StringRepresentation(const SubsetAttributes *atts);

#endif

