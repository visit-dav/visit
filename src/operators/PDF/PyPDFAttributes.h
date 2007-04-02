#ifndef PY_PDFATTRIBUTES_H
#define PY_PDFATTRIBUTES_H
#include <Python.h>
#include <PDFAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyPDFAttributes_StartUp(PDFAttributes *subj, FILE *logFile);
void            PyPDFAttributes_CloseDown();
PyMethodDef    *PyPDFAttributes_GetMethodTable(int *nMethods);
bool            PyPDFAttributes_Check(PyObject *obj);
PDFAttributes *PyPDFAttributes_FromPyObject(PyObject *obj);
PyObject       *PyPDFAttributes_NewPyObject();
PyObject       *PyPDFAttributes_WrapPyObject(const PDFAttributes *attr);
void            PyPDFAttributes_SetLogging(bool val);
void            PyPDFAttributes_SetDefaults(const PDFAttributes *atts);

PyObject       *PyPDFAttributes_StringRepresentation(const PDFAttributes *atts);

#endif

