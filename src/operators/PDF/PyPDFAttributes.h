// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PDFATTRIBUTES_H
#define PY_PDFATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PDFAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define PDFATTRIBUTES_NMETH 56
void           PyPDFAttributes_StartUp(PDFAttributes *subj, void *data);
void           PyPDFAttributes_CloseDown();
PyMethodDef *  PyPDFAttributes_GetMethodTable(int *nMethods);
bool           PyPDFAttributes_Check(PyObject *obj);
PDFAttributes *  PyPDFAttributes_FromPyObject(PyObject *obj);
PyObject *     PyPDFAttributes_New();
PyObject *     PyPDFAttributes_Wrap(const PDFAttributes *attr);
void           PyPDFAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyPDFAttributes_SetDefaults(const PDFAttributes *atts);
std::string    PyPDFAttributes_GetLogString();
std::string    PyPDFAttributes_ToString(const PDFAttributes *, const char *, const bool=false);
PyObject *     PyPDFAttributes_getattr(PyObject *self, char *name);
int            PyPDFAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyPDFAttributes_methods[PDFATTRIBUTES_NMETH];

#endif

