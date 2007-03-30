#ifndef PY_SLICEATTRIBUTES_H
#define PY_SLICEATTRIBUTES_H
#include <Python.h>
#include <SliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySliceAttributes_StartUp(SliceAttributes *subj, FILE *logFile);
void            PySliceAttributes_CloseDown();
PyMethodDef    *PySliceAttributes_GetMethodTable(int *nMethods);
bool            PySliceAttributes_Check(PyObject *obj);
SliceAttributes *PySliceAttributes_FromPyObject(PyObject *obj);
PyObject       *PySliceAttributes_NewPyObject();
PyObject       *PySliceAttributes_WrapPyObject(const SliceAttributes *attr);
void            PySliceAttributes_SetLogging(bool val);
void            PySliceAttributes_SetDefaults(const SliceAttributes *atts);

#endif

