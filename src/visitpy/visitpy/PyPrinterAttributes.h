// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PRINTERATTRIBUTES_H
#define PY_PRINTERATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PrinterAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define PRINTERATTRIBUTES_NMETH 22
void VISITPY_API           PyPrinterAttributes_StartUp(PrinterAttributes *subj, void *data);
void VISITPY_API           PyPrinterAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyPrinterAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyPrinterAttributes_Check(PyObject *obj);
VISITPY_API PrinterAttributes *  PyPrinterAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyPrinterAttributes_New();
VISITPY_API PyObject *     PyPrinterAttributes_Wrap(const PrinterAttributes *attr);
void VISITPY_API           PyPrinterAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyPrinterAttributes_SetDefaults(const PrinterAttributes *atts);
std::string VISITPY_API    PyPrinterAttributes_GetLogString();
std::string VISITPY_API    PyPrinterAttributes_ToString(const PrinterAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyPrinterAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyPrinterAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyPrinterAttributes_methods[PRINTERATTRIBUTES_NMETH];

#endif

