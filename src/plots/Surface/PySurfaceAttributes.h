#ifndef PY_SURFACEATTRIBUTES_H
#define PY_SURFACEATTRIBUTES_H
#include <Python.h>
#include <SurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySurfaceAttributes_StartUp(SurfaceAttributes *subj, FILE *logFile);
void            PySurfaceAttributes_CloseDown();
PyMethodDef    *PySurfaceAttributes_GetMethodTable(int *nMethods);
bool            PySurfaceAttributes_Check(PyObject *obj);
SurfaceAttributes *PySurfaceAttributes_FromPyObject(PyObject *obj);
PyObject       *PySurfaceAttributes_NewPyObject();
PyObject       *PySurfaceAttributes_WrapPyObject(const SurfaceAttributes *attr);
void            PySurfaceAttributes_SetLogging(bool val);
void            PySurfaceAttributes_SetDefaults(const SurfaceAttributes *atts);

#endif

