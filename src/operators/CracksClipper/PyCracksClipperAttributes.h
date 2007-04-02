#ifndef PY_CRACKSCLIPPERATTRIBUTES_H
#define PY_CRACKSCLIPPERATTRIBUTES_H
#include <Python.h>
#include <CracksClipperAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyCracksClipperAttributes_StartUp(CracksClipperAttributes *subj, FILE *logFile);
void            PyCracksClipperAttributes_CloseDown();
PyMethodDef    *PyCracksClipperAttributes_GetMethodTable(int *nMethods);
bool            PyCracksClipperAttributes_Check(PyObject *obj);
CracksClipperAttributes *PyCracksClipperAttributes_FromPyObject(PyObject *obj);
PyObject       *PyCracksClipperAttributes_NewPyObject();
PyObject       *PyCracksClipperAttributes_WrapPyObject(const CracksClipperAttributes *attr);
void            PyCracksClipperAttributes_SetLogging(bool val);
void            PyCracksClipperAttributes_SetDefaults(const CracksClipperAttributes *atts);

PyObject       *PyCracksClipperAttributes_StringRepresentation(const CracksClipperAttributes *atts);

#endif

