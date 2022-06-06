// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TRUECOLORATTRIBUTES_H
#define PY_TRUECOLORATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TruecolorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TRUECOLORATTRIBUTES_NMETH 6
void           PyTruecolorAttributes_StartUp(TruecolorAttributes *subj, void *data);
void           PyTruecolorAttributes_CloseDown();
PyMethodDef *  PyTruecolorAttributes_GetMethodTable(int *nMethods);
bool           PyTruecolorAttributes_Check(PyObject *obj);
TruecolorAttributes *  PyTruecolorAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTruecolorAttributes_New();
PyObject *     PyTruecolorAttributes_Wrap(const TruecolorAttributes *attr);
void           PyTruecolorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTruecolorAttributes_SetDefaults(const TruecolorAttributes *atts);
std::string    PyTruecolorAttributes_GetLogString();
std::string    PyTruecolorAttributes_ToString(const TruecolorAttributes *, const char *, const bool=false);
PyObject *     PyTruecolorAttributes_getattr(PyObject *self, char *name);
int            PyTruecolorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTruecolorAttributes_methods[TRUECOLORATTRIBUTES_NMETH];

#endif

