// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_POINTGLYPHATTRIBUTES_H
#define PY_POINTGLYPHATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <PointGlyphAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define POINTGLYPHATTRIBUTES_NMETH 14
void VISITPY_API           PyPointGlyphAttributes_StartUp(PointGlyphAttributes *subj, void *data);
void VISITPY_API           PyPointGlyphAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyPointGlyphAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyPointGlyphAttributes_Check(PyObject *obj);
VISITPY_API PointGlyphAttributes *  PyPointGlyphAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyPointGlyphAttributes_New();
VISITPY_API PyObject *     PyPointGlyphAttributes_Wrap(const PointGlyphAttributes *attr);
void VISITPY_API           PyPointGlyphAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyPointGlyphAttributes_SetDefaults(const PointGlyphAttributes *atts);
std::string VISITPY_API    PyPointGlyphAttributes_GetLogString();
std::string VISITPY_API    PyPointGlyphAttributes_ToString(const PointGlyphAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyPointGlyphAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyPointGlyphAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyPointGlyphAttributes_methods[POINTGLYPHATTRIBUTES_NMETH];

#endif

