#ifndef PY_THREESLICEATTRIBUTES_H
#define PY_THREESLICEATTRIBUTES_H
#include <Python.h>
#include <ThreeSliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyThreeSliceAttributes_StartUp(ThreeSliceAttributes *subj, FILE *logFile);
void            PyThreeSliceAttributes_CloseDown();
PyMethodDef    *PyThreeSliceAttributes_GetMethodTable(int *nMethods);
bool            PyThreeSliceAttributes_Check(PyObject *obj);
ThreeSliceAttributes *PyThreeSliceAttributes_FromPyObject(PyObject *obj);
PyObject       *PyThreeSliceAttributes_NewPyObject();
PyObject       *PyThreeSliceAttributes_WrapPyObject(const ThreeSliceAttributes *attr);
void            PyThreeSliceAttributes_SetLogging(bool val);
void            PyThreeSliceAttributes_SetDefaults(const ThreeSliceAttributes *atts);

#endif

