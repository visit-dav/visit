#ifndef PY_INDEXSELECTATTRIBUTES_H
#define PY_INDEXSELECTATTRIBUTES_H
#include <Python.h>
#include <IndexSelectAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyIndexSelectAttributes_StartUp(IndexSelectAttributes *subj, FILE *logFile);
void            PyIndexSelectAttributes_CloseDown();
PyMethodDef    *PyIndexSelectAttributes_GetMethodTable(int *nMethods);
bool            PyIndexSelectAttributes_Check(PyObject *obj);
IndexSelectAttributes *PyIndexSelectAttributes_FromPyObject(PyObject *obj);
PyObject       *PyIndexSelectAttributes_NewPyObject();
PyObject       *PyIndexSelectAttributes_WrapPyObject(const IndexSelectAttributes *attr);
void            PyIndexSelectAttributes_SetLogging(bool val);
void            PyIndexSelectAttributes_SetDefaults(const IndexSelectAttributes *atts);

PyObject       *PyIndexSelectAttributes_StringRepresentation(const IndexSelectAttributes *atts);

#endif

