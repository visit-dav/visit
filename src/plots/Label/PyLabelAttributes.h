#ifndef PY_LABELATTRIBUTES_H
#define PY_LABELATTRIBUTES_H
#include <Python.h>
#include <LabelAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyLabelAttributes_StartUp(LabelAttributes *subj, FILE *logFile);
void            PyLabelAttributes_CloseDown();
PyMethodDef    *PyLabelAttributes_GetMethodTable(int *nMethods);
bool            PyLabelAttributes_Check(PyObject *obj);
LabelAttributes *PyLabelAttributes_FromPyObject(PyObject *obj);
PyObject       *PyLabelAttributes_NewPyObject();
PyObject       *PyLabelAttributes_WrapPyObject(const LabelAttributes *attr);
void            PyLabelAttributes_SetLogging(bool val);
void            PyLabelAttributes_SetDefaults(const LabelAttributes *atts);

PyObject       *PyLabelAttributes_StringRepresentation(const LabelAttributes *atts);

#endif

