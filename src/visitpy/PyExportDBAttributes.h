#ifndef PY_EXPORTDBATTRIBUTES_H
#define PY_EXPORTDBATTRIBUTES_H
#include <Python.h>
#include <ExportDBAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyExportDBAttributes_StartUp(ExportDBAttributes *subj, void *data);
void            PyExportDBAttributes_CloseDown();
PyMethodDef    *PyExportDBAttributes_GetMethodTable(int *nMethods);
bool            PyExportDBAttributes_Check(PyObject *obj);
ExportDBAttributes *PyExportDBAttributes_FromPyObject(PyObject *obj);
PyObject       *PyExportDBAttributes_NewPyObject();
PyObject       *PyExportDBAttributes_WrapPyObject(const ExportDBAttributes *attr);
void            PyExportDBAttributes_SetDefaults(const ExportDBAttributes *atts);
std::string     PyExportDBAttributes_GetLogString();
std::string     PyExportDBAttributes_ToString(const ExportDBAttributes *, const char *);

#endif

