// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_ANNOTATIONATTRIBUTES_H
#define PY_ANNOTATIONATTRIBUTES_H
#include <Python.h>
#include <Py2and3Support.h>
#include <AnnotationAttributes.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//
#define ANNOTATIONATTRIBUTES_NMETH 44
void VISITPY_API           PyAnnotationAttributes_StartUp(AnnotationAttributes *subj, void *data);
void VISITPY_API           PyAnnotationAttributes_CloseDown();
VISITPY_API PyMethodDef *  PyAnnotationAttributes_GetMethodTable(int *nMethods);
bool VISITPY_API           PyAnnotationAttributes_Check(PyObject *obj);
VISITPY_API AnnotationAttributes *  PyAnnotationAttributes_FromPyObject(PyObject *obj);
VISITPY_API PyObject *     PyAnnotationAttributes_New();
VISITPY_API PyObject *     PyAnnotationAttributes_Wrap(const AnnotationAttributes *attr);
void VISITPY_API           PyAnnotationAttributes_SetParent(PyObject *obj, PyObject *parent);
void VISITPY_API           PyAnnotationAttributes_SetDefaults(const AnnotationAttributes *atts);
std::string VISITPY_API    PyAnnotationAttributes_GetLogString();
std::string VISITPY_API    PyAnnotationAttributes_ToString(const AnnotationAttributes *, const char *, const bool=false);
VISITPY_API PyObject *     PyAnnotationAttributes_getattr(PyObject *self, char *name);
int VISITPY_API            PyAnnotationAttributes_setattr(PyObject *self, char *name, PyObject *args);
VISITPY_API extern PyMethodDef PyAnnotationAttributes_methods[ANNOTATIONATTRIBUTES_NMETH];

#endif

