#ifndef PY_EXPORTDBATTRIBUTES_H
#define PY_EXPORTDBATTRIBUTES_H
#include <Python.h>
#include <ExportDBAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyExportDBAttributes_StartUp(ExportDBAttributes *subj, FILE *logFile);
void            PyExportDBAttributes_CloseDown();
PyMethodDef    *PyExportDBAttributes_GetMethodTable(int *nMethods);
bool            PyExportDBAttributes_Check(PyObject *obj);
ExportDBAttributes *PyExportDBAttributes_FromPyObject(PyObject *obj);
PyObject       *PyExportDBAttributes_NewPyObject();
PyObject       *PyExportDBAttributes_WrapPyObject(const ExportDBAttributes *attr);
void            PyExportDBAttributes_SetLogging(bool val);
void            PyExportDBAttributes_SetDefaults(const ExportDBAttributes *atts);

PyObject       *PyExportDBAttributes_StringRepresentation(const ExportDBAttributes *atts);

#endif

