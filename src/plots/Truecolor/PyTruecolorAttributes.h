#ifndef PY_TRUECOLORATTRIBUTES_H
#define PY_TRUECOLORATTRIBUTES_H
#include <Python.h>
#include <TruecolorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyTruecolorAttributes_StartUp(TruecolorAttributes *subj, FILE *logFile);
void            PyTruecolorAttributes_CloseDown();
PyMethodDef    *PyTruecolorAttributes_GetMethodTable(int *nMethods);
bool            PyTruecolorAttributes_Check(PyObject *obj);
TruecolorAttributes *PyTruecolorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyTruecolorAttributes_NewPyObject();
PyObject       *PyTruecolorAttributes_WrapPyObject(const TruecolorAttributes *attr);
void            PyTruecolorAttributes_SetLogging(bool val);
void            PyTruecolorAttributes_SetDefaults(const TruecolorAttributes *atts);

PyObject       *PyTruecolorAttributes_StringRepresentation(const TruecolorAttributes *atts);

#endif

