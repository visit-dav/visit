#ifndef PY_SURFACEATTRIBUTES_H
#define PY_SURFACEATTRIBUTES_H
#include <Python.h>
#include <SurfaceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySurfaceAttributes_StartUp(SurfaceAttributes *subj, void *data);
void            PySurfaceAttributes_CloseDown();
PyMethodDef    *PySurfaceAttributes_GetMethodTable(int *nMethods);
bool            PySurfaceAttributes_Check(PyObject *obj);
SurfaceAttributes *PySurfaceAttributes_FromPyObject(PyObject *obj);
PyObject       *PySurfaceAttributes_NewPyObject();
PyObject       *PySurfaceAttributes_WrapPyObject(const SurfaceAttributes *attr);
std::string     PySurfaceAttributes_GetLogString();
void            PySurfaceAttributes_SetDefaults(const SurfaceAttributes *atts);

#endif

