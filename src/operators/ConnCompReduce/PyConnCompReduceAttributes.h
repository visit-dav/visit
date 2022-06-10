// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_CONNCOMPREDUCEATTRIBUTES_H
#define PY_CONNCOMPREDUCEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ConnCompReduceAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define CONNCOMPREDUCEATTRIBUTES_NMETH 4
void           PyConnCompReduceAttributes_StartUp(ConnCompReduceAttributes *subj, void *data);
void           PyConnCompReduceAttributes_CloseDown();
PyMethodDef *  PyConnCompReduceAttributes_GetMethodTable(int *nMethods);
bool           PyConnCompReduceAttributes_Check(PyObject *obj);
ConnCompReduceAttributes *  PyConnCompReduceAttributes_FromPyObject(PyObject *obj);
PyObject *     PyConnCompReduceAttributes_New();
PyObject *     PyConnCompReduceAttributes_Wrap(const ConnCompReduceAttributes *attr);
void           PyConnCompReduceAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyConnCompReduceAttributes_SetDefaults(const ConnCompReduceAttributes *atts);
std::string    PyConnCompReduceAttributes_GetLogString();
std::string    PyConnCompReduceAttributes_ToString(const ConnCompReduceAttributes *, const char *, const bool=false);
PyObject *     PyConnCompReduceAttributes_getattr(PyObject *self, char *name);
int            PyConnCompReduceAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyConnCompReduceAttributes_methods[CONNCOMPREDUCEATTRIBUTES_NMETH];

#endif

