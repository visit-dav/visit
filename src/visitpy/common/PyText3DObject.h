// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_TEXT3DOBJECT_H
#define PY_TEXT3DOBJECT_H
#include <Python.h>
#include <AnnotationObject.h>
#include "Py2and3Support.h"

//
// Functions exposed to the VisIt module.
//
bool              PyText3DObject_Check(PyObject *obj);
AnnotationObject *PyText3DObject_FromPyObject(PyObject *obj);
PyObject         *PyText3DObject_NewPyObject();
PyObject         *PyText3DObject_WrapPyObject(AnnotationObject *attr);

#endif

