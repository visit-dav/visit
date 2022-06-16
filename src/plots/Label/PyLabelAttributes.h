// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LABELATTRIBUTES_H
#define PY_LABELATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <LabelAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define LABELATTRIBUTES_NMETH 28
void           PyLabelAttributes_StartUp(LabelAttributes *subj, void *data);
void           PyLabelAttributes_CloseDown();
PyMethodDef *  PyLabelAttributes_GetMethodTable(int *nMethods);
bool           PyLabelAttributes_Check(PyObject *obj);
LabelAttributes *  PyLabelAttributes_FromPyObject(PyObject *obj);
PyObject *     PyLabelAttributes_New();
PyObject *     PyLabelAttributes_Wrap(const LabelAttributes *attr);
void           PyLabelAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyLabelAttributes_SetDefaults(const LabelAttributes *atts);
std::string    PyLabelAttributes_GetLogString();
std::string    PyLabelAttributes_ToString(const LabelAttributes *, const char *, const bool=false);
PyObject *     PyLabelAttributes_getattr(PyObject *self, char *name);
int            PyLabelAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyLabelAttributes_methods[LABELATTRIBUTES_NMETH];

#endif

