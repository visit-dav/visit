#ifndef PY_SURFCOMPPREPATTRIBUTES_H
#define PY_SURFCOMPPREPATTRIBUTES_H
#include <Python.h>
#include <SurfCompPrepAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySurfCompPrepAttributes_StartUp(SurfCompPrepAttributes *subj, FILE *logFile);
void            PySurfCompPrepAttributes_CloseDown();
PyMethodDef    *PySurfCompPrepAttributes_GetMethodTable(int *nMethods);
bool            PySurfCompPrepAttributes_Check(PyObject *obj);
SurfCompPrepAttributes *PySurfCompPrepAttributes_FromPyObject(PyObject *obj);
PyObject       *PySurfCompPrepAttributes_NewPyObject();
PyObject       *PySurfCompPrepAttributes_WrapPyObject(const SurfCompPrepAttributes *attr);
void            PySurfCompPrepAttributes_SetLogging(bool val);
void            PySurfCompPrepAttributes_SetDefaults(const SurfCompPrepAttributes *atts);

#endif

