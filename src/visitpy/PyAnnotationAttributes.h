#ifndef PY_ANNOTATIONATTRIBUTES_H
#define PY_ANNOTATIONATTRIBUTES_H
#include <Python.h>
#include <AnnotationAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyAnnotationAttributes_StartUp(AnnotationAttributes *subj, void *data);
void            PyAnnotationAttributes_CloseDown();
PyMethodDef    *PyAnnotationAttributes_GetMethodTable(int *nMethods);
bool            PyAnnotationAttributes_Check(PyObject *obj);
AnnotationAttributes *PyAnnotationAttributes_FromPyObject(PyObject *obj);
PyObject       *PyAnnotationAttributes_NewPyObject();
PyObject       *PyAnnotationAttributes_WrapPyObject(const AnnotationAttributes *attr);
std::string     PyAnnotationAttributes_GetLogString();
void            PyAnnotationAttributes_SetDefaults(const AnnotationAttributes *atts);

#endif

