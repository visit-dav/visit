#ifndef PY_RENDERINGATTRIBUTES_H
#define PY_RENDERINGATTRIBUTES_H
#include <Python.h>
#include <RenderingAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyRenderingAttributes_StartUp(RenderingAttributes *subj, void *data);
void            PyRenderingAttributes_CloseDown();
PyMethodDef    *PyRenderingAttributes_GetMethodTable(int *nMethods);
bool            PyRenderingAttributes_Check(PyObject *obj);
RenderingAttributes *PyRenderingAttributes_FromPyObject(PyObject *obj);
PyObject       *PyRenderingAttributes_NewPyObject();
PyObject       *PyRenderingAttributes_WrapPyObject(const RenderingAttributes *attr);
void            PyRenderingAttributes_SetDefaults(const RenderingAttributes *atts);
std::string     PyRenderingAttributes_GetLogString();
std::string     PyRenderingAttributes_ToString(const RenderingAttributes *, const char *);

#endif

