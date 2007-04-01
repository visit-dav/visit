#ifndef PY_QUERYOVERTIMEATTRIBUTES_H
#define PY_QUERYOVERTIMEATTRIBUTES_H
#include <Python.h>
#include <QueryOverTimeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyQueryOverTimeAttributes_StartUp(QueryOverTimeAttributes *subj, FILE *logFile);
void            PyQueryOverTimeAttributes_CloseDown();
PyMethodDef    *PyQueryOverTimeAttributes_GetMethodTable(int *nMethods);
bool            PyQueryOverTimeAttributes_Check(PyObject *obj);
QueryOverTimeAttributes *PyQueryOverTimeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyQueryOverTimeAttributes_NewPyObject();
PyObject       *PyQueryOverTimeAttributes_WrapPyObject(const QueryOverTimeAttributes *attr);
void            PyQueryOverTimeAttributes_SetLogging(bool val);
void            PyQueryOverTimeAttributes_SetDefaults(const QueryOverTimeAttributes *atts);

PyObject       *PyQueryOverTimeAttributes_StringRepresentation(const QueryOverTimeAttributes *atts);

#endif

