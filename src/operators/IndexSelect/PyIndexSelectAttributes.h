// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_INDEXSELECTATTRIBUTES_H
#define PY_INDEXSELECTATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <IndexSelectAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define INDEXSELECTATTRIBUTES_NMETH 42
void           PyIndexSelectAttributes_StartUp(IndexSelectAttributes *subj, void *data);
void           PyIndexSelectAttributes_CloseDown();
PyMethodDef *  PyIndexSelectAttributes_GetMethodTable(int *nMethods);
bool           PyIndexSelectAttributes_Check(PyObject *obj);
IndexSelectAttributes *  PyIndexSelectAttributes_FromPyObject(PyObject *obj);
PyObject *     PyIndexSelectAttributes_New();
PyObject *     PyIndexSelectAttributes_Wrap(const IndexSelectAttributes *attr);
void           PyIndexSelectAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyIndexSelectAttributes_SetDefaults(const IndexSelectAttributes *atts);
std::string    PyIndexSelectAttributes_GetLogString();
std::string    PyIndexSelectAttributes_ToString(const IndexSelectAttributes *, const char *, const bool=false);
PyObject *     PyIndexSelectAttributes_getattr(PyObject *self, char *name);
int            PyIndexSelectAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyIndexSelectAttributes_methods[INDEXSELECTATTRIBUTES_NMETH];

#endif

