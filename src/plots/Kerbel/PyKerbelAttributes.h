#ifndef PY_KERBELATTRIBUTES_H
#define PY_KERBELATTRIBUTES_H
#include <Python.h>
#include <KerbelAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyKerbelAttributes_StartUp(KerbelAttributes *subj, FILE *logFile);
void            PyKerbelAttributes_CloseDown();
PyMethodDef    *PyKerbelAttributes_GetMethodTable(int *nMethods);
bool            PyKerbelAttributes_Check(PyObject *obj);
KerbelAttributes *PyKerbelAttributes_FromPyObject(PyObject *obj);
PyObject       *PyKerbelAttributes_NewPyObject();
PyObject       *PyKerbelAttributes_WrapPyObject(const KerbelAttributes *attr);
void            PyKerbelAttributes_SetLogging(bool val);
void            PyKerbelAttributes_SetDefaults(const KerbelAttributes *atts);

#endif

