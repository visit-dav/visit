#ifndef PY_HOSTPROFILE_H
#define PY_HOSTPROFILE_H
#include <Python.h>
#include <HostProfile.h>

//
// Functions exposed to the VisIt module.
//
void            PyHostProfile_StartUp(HostProfile *subj, FILE *logFile);
void            PyHostProfile_CloseDown();
PyMethodDef    *PyHostProfile_GetMethodTable(int *nMethods);
bool            PyHostProfile_Check(PyObject *obj);
HostProfile *PyHostProfile_FromPyObject(PyObject *obj);
PyObject       *PyHostProfile_NewPyObject();
PyObject       *PyHostProfile_WrapPyObject(const HostProfile *attr);
void            PyHostProfile_SetLogging(bool val);
void            PyHostProfile_SetDefaults(const HostProfile *atts);

#endif

