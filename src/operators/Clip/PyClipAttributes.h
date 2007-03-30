#ifndef PY_CLIPATTRIBUTES_H
#define PY_CLIPATTRIBUTES_H
#include <Python.h>
#include <ClipAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyClipAttributes_StartUp(ClipAttributes *subj, FILE *logFile);
void            PyClipAttributes_CloseDown();
PyMethodDef    *PyClipAttributes_GetMethodTable(int *nMethods);
bool            PyClipAttributes_Check(PyObject *obj);
ClipAttributes *PyClipAttributes_FromPyObject(PyObject *obj);
PyObject       *PyClipAttributes_NewPyObject();
PyObject       *PyClipAttributes_WrapPyObject(const ClipAttributes *attr);
void            PyClipAttributes_SetLogging(bool val);
void            PyClipAttributes_SetDefaults(const ClipAttributes *atts);

#endif

