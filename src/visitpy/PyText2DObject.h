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

