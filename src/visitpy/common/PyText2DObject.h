// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TEXT2DOBJECT_H
#define PY_TEXT2DOBJECT_H
#include <Python.h>
#include <AnnotationObject.h>

//
// Functions exposed to the VisIt module.
//
bool              PyText2DObject_Check(PyObject *obj);
AnnotationObject *PyText2DObject_FromPyObject(PyObject *obj);
PyObject         *PyText2DObject_NewPyObject();
PyObject         *PyText2DObject_WrapPyObject(AnnotationObject *attr);

#endif

