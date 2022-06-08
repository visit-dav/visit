// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TESSELLATEATTRIBUTES_H
#define PY_TESSELLATEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <TessellateAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define TESSELLATEATTRIBUTES_NMETH 8
void           PyTessellateAttributes_StartUp(TessellateAttributes *subj, void *data);
void           PyTessellateAttributes_CloseDown();
PyMethodDef *  PyTessellateAttributes_GetMethodTable(int *nMethods);
bool           PyTessellateAttributes_Check(PyObject *obj);
TessellateAttributes *  PyTessellateAttributes_FromPyObject(PyObject *obj);
PyObject *     PyTessellateAttributes_New();
PyObject *     PyTessellateAttributes_Wrap(const TessellateAttributes *attr);
void           PyTessellateAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyTessellateAttributes_SetDefaults(const TessellateAttributes *atts);
std::string    PyTessellateAttributes_GetLogString();
std::string    PyTessellateAttributes_ToString(const TessellateAttributes *, const char *, const bool=false);
PyObject *     PyTessellateAttributes_getattr(PyObject *self, char *name);
int            PyTessellateAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyTessellateAttributes_methods[TESSELLATEATTRIBUTES_NMETH];

#endif

