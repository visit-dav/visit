#ifndef PY_CURVEATTRIBUTES_H
#define PY_CURVEATTRIBUTES_H
#include <Python.h>
#include <CurveAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyCurveAttributes_StartUp(CurveAttributes *subj, void *data);
void            PyCurveAttributes_CloseDown();
PyMethodDef    *PyCurveAttributes_GetMethodTable(int *nMethods);
bool            PyCurveAttributes_Check(PyObject *obj);
CurveAttributes *PyCurveAttributes_FromPyObject(PyObject *obj);
PyObject       *PyCurveAttributes_NewPyObject();
PyObject       *PyCurveAttributes_WrapPyObject(const CurveAttributes *attr);
std::string     PyCurveAttributes_GetLogString();
void            PyCurveAttributes_SetDefaults(const CurveAttributes *atts);

#endif

