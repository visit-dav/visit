// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_LINEOBJECT_H
#define PY_LINEOBJECT_H
#include <Python.h>
#include <AnnotationObject.h>
#include "Py2and3Support.h"

//
// Functions exposed to the VisIt module.
//
bool              PyLineObject_Check(PyObject *obj);
AnnotationObject *PyLineObject_FromPyObject(PyObject *obj);
PyObject         *PyLineObject_NewPyObject();
PyObject         *PyLineObject_WrapPyObject(AnnotationObject *annot);

#endif
