#ifndef PY_VIEWATTRIBUTES_H
#define PY_VIEWATTRIBUTES_H
#include <Python.h>
#include <ViewAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyViewAttributes_StartUp(ViewAttributes *subj, FILE *logFile);
void            PyViewAttributes_CloseDown();
PyMethodDef    *PyViewAttributes_GetMethodTable(int *nMethods);
bool            PyViewAttributes_Check(PyObject *obj);
ViewAttributes *PyViewAttributes_FromPyObject(PyObject *obj);
PyObject       *PyViewAttributes_NewPyObject();
PyObject       *PyViewAttributes_WrapPyObject(const ViewAttributes *attr);
void            PyViewAttributes_SetLogging(bool val);
void            PyViewAttributes_SetDefaults(const ViewAttributes *atts);

#endif

