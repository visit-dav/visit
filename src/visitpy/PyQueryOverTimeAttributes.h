#ifndef PY_QUERYOVERTIMEATTRIBUTES_H
#define PY_QUERYOVERTIMEATTRIBUTES_H
#include <Python.h>
#include <QueryOverTimeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyQueryOverTimeAttributes_StartUp(QueryOverTimeAttributes *subj, void *data);
void            PyQueryOverTimeAttributes_CloseDown();
PyMethodDef    *PyQueryOverTimeAttributes_GetMethodTable(int *nMethods);
bool            PyQueryOverTimeAttributes_Check(PyObject *obj);
QueryOverTimeAttributes *PyQueryOverTimeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyQueryOverTimeAttributes_NewPyObject();
PyObject       *PyQueryOverTimeAttributes_WrapPyObject(const QueryOverTimeAttributes *attr);
void            PyQueryOverTimeAttributes_SetDefaults(const QueryOverTimeAttributes *atts);
std::string     PyQueryOverTimeAttributes_GetLogString();
std::string     PyQueryOverTimeAttributes_ToString(const QueryOverTimeAttributes *, const char *);

#endif

