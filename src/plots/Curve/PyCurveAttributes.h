#ifndef PY_CURVEATTRIBUTES_H
#define PY_CURVEATTRIBUTES_H
#include <Python.h>
#include <CurveAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyCurveAttributes_StartUp(CurveAttributes *subj, FILE *logFile);
void            PyCurveAttributes_CloseDown();
PyMethodDef    *PyCurveAttributes_GetMethodTable(int *nMethods);
bool            PyCurveAttributes_Check(PyObject *obj);
CurveAttributes *PyCurveAttributes_FromPyObject(PyObject *obj);
PyObject       *PyCurveAttributes_NewPyObject();
PyObject       *PyCurveAttributes_WrapPyObject(const CurveAttributes *attr);
void            PyCurveAttributes_SetLogging(bool val);
void            PyCurveAttributes_SetDefaults(const CurveAttributes *atts);

#endif

