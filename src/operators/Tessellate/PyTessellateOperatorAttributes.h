// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TESSELLATEOPERATORATTRIBUTES_H
#define PY_TESSELLATEOPERATORATTRIBUTES_H
#include <Python.h>
#include <TessellateOperatorAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TESSELLATEOPERATORATTRIBUTES_NMETH 6
void           PyTessellateOperatorAttributes_StartUp(TessellateOperatorAttributes *subj, void *data);
void           PyTessellateOperatorAttributes_CloseDown();
PyMethodDef *  PyTessellateOperatorAttributes_GetMethodTable(int *nMethods);
bool           PyTessellateOperatorAttributes_Check(PyObject *obj);
TessellateOperatorAttributes *  PyTessellateOperatorAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTessellateOperatorAttributes_New();
PyObject *     PyTessellateOperatorAttributes_Wrap(const TessellateOperatorAttributes *attr);
void           PyTessellateOperatorAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTessellateOperatorAttributes_SetDefaults(const TessellateOperatorAttributes *atts);
std::string    PyTessellateOperatorAttributes_GetLogString();
std::string    PyTessellateOperatorAttributes_ToString(const TessellateOperatorAttributes *, const char *);
PyObject *     PyTessellateOperatorAttributes_getattr(PyObject *self, char *name);
int            PyTessellateOperatorAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTessellateOperatorAttributes_methods[TESSELLATEOPERATORATTRIBUTES_NMETH];

#endif

