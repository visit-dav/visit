#ifndef PY_VIEWCURVEATTRIBUTES_H
#define PY_VIEWCURVEATTRIBUTES_H
#include <Python.h>
#include <ViewCurveAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyViewCurveAttributes_StartUp(ViewCurveAttributes *subj, void *data);
void            PyViewCurveAttributes_CloseDown();
PyMethodDef    *PyViewCurveAttributes_GetMethodTable(int *nMethods);
bool            PyViewCurveAttributes_Check(PyObject *obj);
ViewCurveAttributes *PyViewCurveAttributes_FromPyObject(PyObject *obj);
PyObject       *PyViewCurveAttributes_NewPyObject();
PyObject       *PyViewCurveAttributes_WrapPyObject(const ViewCurveAttributes *attr);
std::string     PyViewCurveAttributes_GetLogString();
void            PyViewCurveAttributes_SetDefaults(const ViewCurveAttributes *atts);

#endif

