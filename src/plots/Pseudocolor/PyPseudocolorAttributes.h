#ifndef PY_PSEUDOCOLORATTRIBUTES_H
#define PY_PSEUDOCOLORATTRIBUTES_H
#include <Python.h>
#include <PseudocolorAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyPseudocolorAttributes_StartUp(PseudocolorAttributes *subj, FILE *logFile);
void            PyPseudocolorAttributes_CloseDown();
PyMethodDef    *PyPseudocolorAttributes_GetMethodTable(int *nMethods);
bool            PyPseudocolorAttributes_Check(PyObject *obj);
PseudocolorAttributes *PyPseudocolorAttributes_FromPyObject(PyObject *obj);
PyObject       *PyPseudocolorAttributes_NewPyObject();
PyObject       *PyPseudocolorAttributes_WrapPyObject(const PseudocolorAttributes *attr);
void            PyPseudocolorAttributes_SetLogging(bool val);
void            PyPseudocolorAttributes_SetDefaults(const PseudocolorAttributes *atts);

PyObject       *PyPseudocolorAttributes_StringRepresentation(const PseudocolorAttributes *atts);

#endif

