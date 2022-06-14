// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_PARALLELCOORDINATESATTRIBUTES_H
#define PY_PARALLELCOORDINATESATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <ParallelCoordinatesAttributes.h>

//
// Functions exposed to the VisIt module.
//
#define PARALLELCOORDINATESATTRIBUTES_NMETH 32
void           PyParallelCoordinatesAttributes_StartUp(ParallelCoordinatesAttributes *subj, void *data);
void           PyParallelCoordinatesAttributes_CloseDown();
PyMethodDef *  PyParallelCoordinatesAttributes_GetMethodTable(int *nMethods);
bool           PyParallelCoordinatesAttributes_Check(PyObject *obj);
ParallelCoordinatesAttributes *  PyParallelCoordinatesAttributes_FromPyObject(PyObject *obj);
PyObject *     PyParallelCoordinatesAttributes_New();
PyObject *     PyParallelCoordinatesAttributes_Wrap(const ParallelCoordinatesAttributes *attr);
void           PyParallelCoordinatesAttributes_SetParent(PyObject *obj, PyObject *parent);
void           PyParallelCoordinatesAttributes_SetDefaults(const ParallelCoordinatesAttributes *atts);
std::string    PyParallelCoordinatesAttributes_GetLogString();
std::string    PyParallelCoordinatesAttributes_ToString(const ParallelCoordinatesAttributes *, const char *, const bool=false);
PyObject *     PyParallelCoordinatesAttributes_getattr(PyObject *self, char *name);
int            PyParallelCoordinatesAttributes_setattr(PyObject *self, char *name, PyObject *args);
extern PyMethodDef PyParallelCoordinatesAttributes_methods[PARALLELCOORDINATESATTRIBUTES_NMETH];

#endif

