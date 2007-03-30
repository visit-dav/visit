#ifndef PY_BOXATTRIBUTES_H
#define PY_BOXATTRIBUTES_H
#include <Python.h>
#include <BoxAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyBoxAttributes_StartUp(BoxAttributes *subj, FILE *logFile);
void            PyBoxAttributes_CloseDown();
PyMethodDef    *PyBoxAttributes_GetMethodTable(int *nMethods);
bool            PyBoxAttributes_Check(PyObject *obj);
BoxAttributes *PyBoxAttributes_FromPyObject(PyObject *obj);
PyObject       *PyBoxAttributes_NewPyObject();
PyObject       *PyBoxAttributes_WrapPyObject(const BoxAttributes *attr);
void            PyBoxAttributes_SetLogging(bool val);
void            PyBoxAttributes_SetDefaults(const BoxAttributes *atts);

#endif

