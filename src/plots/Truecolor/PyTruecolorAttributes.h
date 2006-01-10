#ifndef PY_TRUECOLORATTRIBUTES_H
#define PY_TRUECOLORATTRIBUTES_H
#include <Python.h>
#include <TruecolorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyTruecolorAttributes_StartUp(TruecolorAttributes *subj, void *data);
void            PyTruecolorAttributes_CloseDown();
PyMethodDef    *PyTruecolorAttributes_GetMethodTable(int *nMethods);
bool            PyTruecolorAttributes_Check(PyObject *obj);
TruecolorAttributes *PyTruecolorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyTruecolorAttributes_NewPyObject();
PyObject       *PyTruecolorAttributes_WrapPyObject(const TruecolorAttributes *attr);
std::string     PyTruecolorAttributes_GetLogString();
void            PyTruecolorAttributes_SetDefaults(const TruecolorAttributes *atts);

#endif

