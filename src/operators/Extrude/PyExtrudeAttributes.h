// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_EXTRUDEATTRIBUTES_H
#define PY_EXTRUDEATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ExtrudeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void           PyExtrudeAttributes_StartUp(ExtrudeAttributes *subj, void *data);
void           PyExtrudeAttributes_CloseDown();
PyMethodDef *  PyExtrudeAttributes_GetMethodTable(int *nMethods);
bool           PyExtrudeAttributes_Check(PyObject *obj);
ExtrudeAttributes *  PyExtrudeAttributes_FromPyObject(PyObject *obj);
PyObject *     PyExtrudeAttributes_New();
PyObject *     PyExtrudeAttributes_Wrap(const ExtrudeAttributes *attr);
void           PyExtrudeAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyExtrudeAttributes_SetDefaults(const ExtrudeAttributes *atts);
std::string    PyExtrudeAttributes_GetLogString();
std::string    PyExtrudeAttributes_ToString(const ExtrudeAttributes *, const char *, const bool=false);

#endif

