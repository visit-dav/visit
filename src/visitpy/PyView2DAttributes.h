#ifndef PY_VIEW2DATTRIBUTES_H
#define PY_VIEW2DATTRIBUTES_H
#include <Python.h>
#include <View2DAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyView2DAttributes_StartUp(View2DAttributes *subj, FILE *logFile);
void            PyView2DAttributes_CloseDown();
PyMethodDef    *PyView2DAttributes_GetMethodTable(int *nMethods);
bool            PyView2DAttributes_Check(PyObject *obj);
View2DAttributes *PyView2DAttributes_FromPyObject(PyObject *obj);
PyObject       *PyView2DAttributes_NewPyObject();
PyObject       *PyView2DAttributes_WrapPyObject(const View2DAttributes *attr);
void            PyView2DAttributes_SetLogging(bool val);
void            PyView2DAttributes_SetDefaults(const View2DAttributes *atts);

PyObject       *PyView2DAttributes_StringRepresentation(const View2DAttributes *atts);

#endif

