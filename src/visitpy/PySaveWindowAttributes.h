#ifndef PY_SAVEWINDOWATTRIBUTES_H
#define PY_SAVEWINDOWATTRIBUTES_H
#include <Python.h>
#include <SaveWindowAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySaveWindowAttributes_StartUp(SaveWindowAttributes *subj, FILE *logFile);
void            PySaveWindowAttributes_CloseDown();
PyMethodDef    *PySaveWindowAttributes_GetMethodTable(int *nMethods);
bool            PySaveWindowAttributes_Check(PyObject *obj);
SaveWindowAttributes *PySaveWindowAttributes_FromPyObject(PyObject *obj);
PyObject       *PySaveWindowAttributes_NewPyObject();
PyObject       *PySaveWindowAttributes_WrapPyObject(const SaveWindowAttributes *attr);
void            PySaveWindowAttributes_SetLogging(bool val);
void            PySaveWindowAttributes_SetDefaults(const SaveWindowAttributes *atts);

PyObject       *PySaveWindowAttributes_StringRepresentation(const SaveWindowAttributes *atts);

#endif

