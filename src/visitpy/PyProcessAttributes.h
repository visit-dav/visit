#ifndef PY_PROCESSATTRIBUTES_H
#define PY_PROCESSATTRIBUTES_H
#include <Python.h>
#include <ProcessAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyProcessAttributes_StartUp(ProcessAttributes *subj, FILE *logFile);
void            PyProcessAttributes_CloseDown();
PyMethodDef    *PyProcessAttributes_GetMethodTable(int *nMethods);
bool            PyProcessAttributes_Check(PyObject *obj);
ProcessAttributes *PyProcessAttributes_FromPyObject(PyObject *obj);
PyObject       *PyProcessAttributes_NewPyObject();
PyObject       *PyProcessAttributes_WrapPyObject(const ProcessAttributes *attr);
void            PyProcessAttributes_SetLogging(bool val);
void            PyProcessAttributes_SetDefaults(const ProcessAttributes *atts);

PyObject       *PyProcessAttributes_StringRepresentation(const ProcessAttributes *atts);

#endif

