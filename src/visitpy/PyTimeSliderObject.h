#ifndef PY_AnnotationObject_H
#define PY_AnnotationObject_H
#include <Python.h>
#include <AnnotationObject.h>

//
// Functions exposed to the VisIt module.
//
bool              PyTimeSliderObject_Check(PyObject *obj);
AnnotationObject *PyTimeSliderObject_FromPyObject(PyObject *obj);
PyObject         *PyTimeSliderObject_NewPyObject();
PyObject         *PyTimeSliderObject_WrapPyObject(AnnotationObject *attr);

#endif

