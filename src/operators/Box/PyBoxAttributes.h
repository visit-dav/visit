#ifndef PY_BOXATTRIBUTES_H
#define PY_BOXATTRIBUTES_H
#include <Python.h>
#include <BoxAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyBoxAttributes_StartUp(BoxAttributes *subj, void *data);
void            PyBoxAttributes_CloseDown();
PyMethodDef    *PyBoxAttributes_GetMethodTable(int *nMethods);
bool            PyBoxAttributes_Check(PyObject *obj);
BoxAttributes *PyBoxAttributes_FromPyObject(PyObject *obj);
PyObject       *PyBoxAttributes_NewPyObject();
PyObject       *PyBoxAttributes_WrapPyObject(const BoxAttributes *attr);
std::string     PyBoxAttributes_GetLogString();
void            PyBoxAttributes_SetDefaults(const BoxAttributes *atts);

#endif

