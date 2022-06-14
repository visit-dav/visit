// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_REMOVECELLSATTRIBUTES_H
#define PY_REMOVECELLSATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <RemoveCellsAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define REMOVECELLSATTRIBUTES_NMETH 6
void           PyRemoveCellsAttributes_StartUp(RemoveCellsAttributes *subj, void *data);
void           PyRemoveCellsAttributes_CloseDown();
PyMethodDef *  PyRemoveCellsAttributes_GetMethodTable(int *nMethods);
bool           PyRemoveCellsAttributes_Check(PyObject *obj);
RemoveCellsAttributes *  PyRemoveCellsAttributes_FromPyObject(PyObject *obj);
PyObject *     PyRemoveCellsAttributes_New();
PyObject *     PyRemoveCellsAttributes_Wrap(const RemoveCellsAttributes *attr);
void           PyRemoveCellsAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyRemoveCellsAttributes_SetDefaults(const RemoveCellsAttributes *atts);
std::string    PyRemoveCellsAttributes_GetLogString();
std::string    PyRemoveCellsAttributes_ToString(const RemoveCellsAttributes *, const char *, const bool=false);
PyObject *     PyRemoveCellsAttributes_getattr(PyObject *self, char *name);
int            PyRemoveCellsAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyRemoveCellsAttributes_methods[REMOVECELLSATTRIBUTES_NMETH];

#endif

