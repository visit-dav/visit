#ifndef PY_VIEW3DATTRIBUTES_H
#define PY_VIEW3DATTRIBUTES_H
#include <Python.h>
#include <View3DAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyView3DAttributes_StartUp(View3DAttributes *subj, void *data);
void            PyView3DAttributes_CloseDown();
PyMethodDef    *PyView3DAttributes_GetMethodTable(int *nMethods);
bool            PyView3DAttributes_Check(PyObject *obj);
View3DAttributes *PyView3DAttributes_FromPyObject(PyObject *obj);
PyObject       *PyView3DAttributes_NewPyObject();
PyObject       *PyView3DAttributes_WrapPyObject(const View3DAttributes *attr);
std::string     PyView3DAttributes_GetLogString();
void            PyView3DAttributes_SetDefaults(const View3DAttributes *atts);

#endif

