#ifndef PY_CONNCOMPREDUCEATTRIBUTES_H
#define PY_CONNCOMPREDUCEATTRIBUTES_H
#include <Python.h>
#include <ConnCompReduceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyConnCompReduceAttributes_StartUp(ConnCompReduceAttributes *subj, FILE *logFile);
void            PyConnCompReduceAttributes_CloseDown();
PyMethodDef    *PyConnCompReduceAttributes_GetMethodTable(int *nMethods);
bool            PyConnCompReduceAttributes_Check(PyObject *obj);
ConnCompReduceAttributes *PyConnCompReduceAttributes_FromPyObject(PyObject *obj);
PyObject       *PyConnCompReduceAttributes_NewPyObject();
PyObject       *PyConnCompReduceAttributes_WrapPyObject(const ConnCompReduceAttributes *attr);
void            PyConnCompReduceAttributes_SetLogging(bool val);
void            PyConnCompReduceAttributes_SetDefaults(const ConnCompReduceAttributes *atts);

#endif

