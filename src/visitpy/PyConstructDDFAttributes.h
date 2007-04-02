#ifndef PY_CONSTRUCTDDFATTRIBUTES_H
#define PY_CONSTRUCTDDFATTRIBUTES_H
#include <Python.h>
#include <ConstructDDFAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyConstructDDFAttributes_StartUp(ConstructDDFAttributes *subj, void *data);
void            PyConstructDDFAttributes_CloseDown();
PyMethodDef    *PyConstructDDFAttributes_GetMethodTable(int *nMethods);
bool            PyConstructDDFAttributes_Check(PyObject *obj);
ConstructDDFAttributes *PyConstructDDFAttributes_FromPyObject(PyObject *obj);
PyObject       *PyConstructDDFAttributes_NewPyObject();
PyObject       *PyConstructDDFAttributes_WrapPyObject(const ConstructDDFAttributes *attr);
void            PyConstructDDFAttributes_SetDefaults(const ConstructDDFAttributes *atts);
std::string     PyConstructDDFAttributes_GetLogString();
std::string     PyConstructDDFAttributes_ToString(const ConstructDDFAttributes *, const char *);

#endif

