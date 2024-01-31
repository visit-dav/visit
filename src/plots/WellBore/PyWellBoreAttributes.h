// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_WELLBOREATTRIBUTES_H
#define PY_WELLBOREATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <WellBoreAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define WELLBOREATTRIBUTES_NMETH 38
void           PyWellBoreAttributes_StartUp(WellBoreAttributes *subj, void *data);
void           PyWellBoreAttributes_CloseDown();
PyMethodDef *  PyWellBoreAttributes_GetMethodTable(int *nMethods);
bool           PyWellBoreAttributes_Check(PyObject *obj);
WellBoreAttributes *  PyWellBoreAttributes_FromPyObject(PyObject *obj);
PyObject *     PyWellBoreAttributes_New();
PyObject *     PyWellBoreAttributes_Wrap(const WellBoreAttributes *attr);
void           PyWellBoreAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyWellBoreAttributes_SetDefaults(const WellBoreAttributes *atts);
std::string    PyWellBoreAttributes_GetLogString();
std::string    PyWellBoreAttributes_ToString(const WellBoreAttributes *, const char *, const bool=false);
PyObject *     PyWellBoreAttributes_getattr(PyObject *self, char *name);
int            PyWellBoreAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyWellBoreAttributes_methods[WELLBOREATTRIBUTES_NMETH];

#endif

