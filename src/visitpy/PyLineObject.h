#ifndef PY_LINEOBJECT_H
#define PY_LINEOBJECT_H

/* CUSTOM - Renamed LineObject to AnnotationObject everywhere. */

#include <Python.h>
#include <AnnotationObject.h>

//
// Functions exposed to the VisIt module.
//
void            PyLineObject_StartUp(AnnotationObject *subj, FILE *logFile);
void            PyLineObject_CloseDown();
PyMethodDef    *PyLineObject_GetMethodTable(int *nMethods);
bool            PyLineObject_Check(PyObject *obj);
AnnotationObject *PyLineObject_FromPyObject(PyObject *obj);
PyObject       *PyLineObject_NewPyObject();
PyObject       *PyLineObject_WrapPyObject(const AnnotationObject *attr);
void            PyLineObject_SetLogging(bool val);
void            PyLineObject_SetDefaults(const AnnotationObject *atts);

PyObject       *PyLineObject_StringRepresentation(const AnnotationObject *atts);

#endif
