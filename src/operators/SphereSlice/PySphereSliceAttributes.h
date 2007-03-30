#ifndef PY_SPHERESLICEATTRIBUTES_H
#define PY_SPHERESLICEATTRIBUTES_H
#include <Python.h>
#include <SphereSliceAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySphereSliceAttributes_StartUp(SphereSliceAttributes *subj, FILE *logFile);
void            PySphereSliceAttributes_CloseDown();
PyMethodDef    *PySphereSliceAttributes_GetMethodTable(int *nMethods);
bool            PySphereSliceAttributes_Check(PyObject *obj);
SphereSliceAttributes *PySphereSliceAttributes_FromPyObject(PyObject *obj);
PyObject       *PySphereSliceAttributes_NewPyObject();
PyObject       *PySphereSliceAttributes_WrapPyObject(const SphereSliceAttributes *attr);
void            PySphereSliceAttributes_SetLogging(bool val);
void            PySphereSliceAttributes_SetDefaults(const SphereSliceAttributes *atts);

#endif

