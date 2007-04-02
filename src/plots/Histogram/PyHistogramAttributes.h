#ifndef PY_HISTOGRAMATTRIBUTES_H
#define PY_HISTOGRAMATTRIBUTES_H
#include <Python.h>
#include <HistogramAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyHistogramAttributes_StartUp(HistogramAttributes *subj, void *data);
void            PyHistogramAttributes_CloseDown();
PyMethodDef    *PyHistogramAttributes_GetMethodTable(int *nMethods);
bool            PyHistogramAttributes_Check(PyObject *obj);
HistogramAttributes *PyHistogramAttributes_FromPyObject(PyObject *obj);
PyObject       *PyHistogramAttributes_NewPyObject();
PyObject       *PyHistogramAttributes_WrapPyObject(const HistogramAttributes *attr);
void            PyHistogramAttributes_SetDefaults(const HistogramAttributes *atts);
std::string     PyHistogramAttributes_GetLogString();
std::string     PyHistogramAttributes_ToString(const HistogramAttributes *, const char *);

#endif

