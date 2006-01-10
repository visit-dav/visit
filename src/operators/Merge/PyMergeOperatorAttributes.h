#ifndef PY_MERGEOPERATORATTRIBUTES_H
#define PY_MERGEOPERATORATTRIBUTES_H
#include <Python.h>
#include <MergeOperatorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyMergeOperatorAttributes_StartUp(MergeOperatorAttributes *subj, void *data);
void            PyMergeOperatorAttributes_CloseDown();
PyMethodDef    *PyMergeOperatorAttributes_GetMethodTable(int *nMethods);
bool            PyMergeOperatorAttributes_Check(PyObject *obj);
MergeOperatorAttributes *PyMergeOperatorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyMergeOperatorAttributes_NewPyObject();
PyObject       *PyMergeOperatorAttributes_WrapPyObject(const MergeOperatorAttributes *attr);
std::string     PyMergeOperatorAttributes_GetLogString();
void            PyMergeOperatorAttributes_SetDefaults(const MergeOperatorAttributes *atts);

#endif

