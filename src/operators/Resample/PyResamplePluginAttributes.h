#ifndef PY_RESAMPLEPLUGINATTRIBUTES_H
#define PY_RESAMPLEPLUGINATTRIBUTES_H
#include <Python.h>
#include <ResamplePluginAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyResamplePluginAttributes_StartUp(ResamplePluginAttributes *subj, FILE *logFile);
void            PyResamplePluginAttributes_CloseDown();
PyMethodDef    *PyResamplePluginAttributes_GetMethodTable(int *nMethods);
bool            PyResamplePluginAttributes_Check(PyObject *obj);
ResamplePluginAttributes *PyResamplePluginAttributes_FromPyObject(PyObject *obj);
PyObject       *PyResamplePluginAttributes_NewPyObject();
PyObject       *PyResamplePluginAttributes_WrapPyObject(const ResamplePluginAttributes *attr);
void            PyResamplePluginAttributes_SetLogging(bool val);
void            PyResamplePluginAttributes_SetDefaults(const ResamplePluginAttributes *atts);

PyObject       *PyResamplePluginAttributes_StringRepresentation(const ResamplePluginAttributes *atts);

#endif

