#ifndef PY_RENDERINGATTRIBUTES_H
#define PY_RENDERINGATTRIBUTES_H
#include <Python.h>
#include <RenderingAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyRenderingAttributes_StartUp(RenderingAttributes *subj, FILE *logFile);
void            PyRenderingAttributes_CloseDown();
PyMethodDef    *PyRenderingAttributes_GetMethodTable(int *nMethods);
bool            PyRenderingAttributes_Check(PyObject *obj);
RenderingAttributes *PyRenderingAttributes_FromPyObject(PyObject *obj);
PyObject       *PyRenderingAttributes_NewPyObject();
PyObject       *PyRenderingAttributes_WrapPyObject(const RenderingAttributes *attr);
void            PyRenderingAttributes_SetLogging(bool val);
void            PyRenderingAttributes_SetDefaults(const RenderingAttributes *atts);

PyObject       *PyRenderingAttributes_StringRepresentation(const RenderingAttributes *atts);

#endif

