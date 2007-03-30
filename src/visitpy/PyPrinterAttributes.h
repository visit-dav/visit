#ifndef PY_PRINTERATTRIBUTES_H
#define PY_PRINTERATTRIBUTES_H
#include <Python.h>
#include <PrinterAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyPrinterAttributes_StartUp(PrinterAttributes *subj, FILE *logFile);
void            PyPrinterAttributes_CloseDown();
PyMethodDef    *PyPrinterAttributes_GetMethodTable(int *nMethods);
bool            PyPrinterAttributes_Check(PyObject *obj);
PrinterAttributes *PyPrinterAttributes_FromPyObject(PyObject *obj);
PyObject       *PyPrinterAttributes_NewPyObject();
PyObject       *PyPrinterAttributes_WrapPyObject(const PrinterAttributes *attr);
void            PyPrinterAttributes_SetLogging(bool val);
void            PyPrinterAttributes_SetDefaults(const PrinterAttributes *atts);

#endif

