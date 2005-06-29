#ifndef PY_IMAGEOBJECT_H
#define PY_IMAGEOBJECT_H

/* CUSTOM - Renamed ImageObject to AnnotationObject everywhere. */

#include <Python.h>
#include <AnnotationObject.h>

//
// Functions exposed to the VisIt module.
//
void            PyImageObject_StartUp(AnnotationObject *subj, FILE *logFile);
void            PyImageObject_CloseDown();
PyMethodDef    *PyImageObject_GetMethodTable(int *nMethods);
bool            PyImageObject_Check(PyObject *obj);
AnnotationObject *PyImageObject_FromPyObject(PyObject *obj);
PyObject       *PyImageObject_NewPyObject();
PyObject       *PyImageObject_WrapPyObject(const AnnotationObject *attr);
void            PyImageObject_SetLogging(bool val);
void            PyImageObject_SetDefaults(const AnnotationObject *atts);

PyObject       *PyImageObject_StringRepresentation(const AnnotationObject *atts);

#endif

