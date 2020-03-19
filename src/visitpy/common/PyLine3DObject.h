// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LINE3DOBJECT_H
#define PY_LINE3DOBJECT_H
#include <Python.h>
#include <AnnotationObject.h>

//
// Functions exposed to the VisIt module.
//
bool              PyLine3DObject_Check(PyObject *obj);
AnnotationObject *PyLine3DObject_FromPyObject(PyObject *obj);
PyObject         *PyLine3DObject_NewPyObject();
PyObject         *PyLine3DObject_WrapPyObject(AnnotationObject *attr);

#endif

