// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LEGENDATTRIBUTES_OBJECT_H
#define PY_LEGENDATTRIBUTES_OBJECT_H
#include <Python.h>
#include <AnnotationObject.h>

//
// Functions exposed to the VisIt module.
//
bool              PyLegendAttributesObject_Check(PyObject *obj);
AnnotationObject *PyLegendAttributesObject_FromPyObject(PyObject *obj);
PyObject         *PyLegendAttributesObject_NewPyObject();
PyObject         *PyLegendAttributesObject_WrapPyObject(AnnotationObject *attr);

#endif

