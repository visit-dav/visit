#ifndef PY_CLIPATTRIBUTES_H
#define PY_CLIPATTRIBUTES_H
#include <Python.h>
#include <ClipAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyClipAttributes_StartUp(ClipAttributes *subj, void *data);
void            PyClipAttributes_CloseDown();
PyMethodDef    *PyClipAttributes_GetMethodTable(int *nMethods);
bool            PyClipAttributes_Check(PyObject *obj);
ClipAttributes *PyClipAttributes_FromPyObject(PyObject *obj);
PyObject       *PyClipAttributes_NewPyObject();
PyObject       *PyClipAttributes_WrapPyObject(const ClipAttributes *attr);
std::string     PyClipAttributes_GetLogString();
void            PyClipAttributes_SetDefaults(const ClipAttributes *atts);

#endif

