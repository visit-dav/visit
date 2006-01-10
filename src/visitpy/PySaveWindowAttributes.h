#ifndef PY_SAVEWINDOWATTRIBUTES_H
#define PY_SAVEWINDOWATTRIBUTES_H
#include <Python.h>
#include <SaveWindowAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySaveWindowAttributes_StartUp(SaveWindowAttributes *subj, void *data);
void            PySaveWindowAttributes_CloseDown();
PyMethodDef    *PySaveWindowAttributes_GetMethodTable(int *nMethods);
bool            PySaveWindowAttributes_Check(PyObject *obj);
SaveWindowAttributes *PySaveWindowAttributes_FromPyObject(PyObject *obj);
PyObject       *PySaveWindowAttributes_NewPyObject();
PyObject       *PySaveWindowAttributes_WrapPyObject(const SaveWindowAttributes *attr);
void            PySaveWindowAttributes_SetDefaults(const SaveWindowAttributes *atts);
std::string     PySaveWindowAttributes_GetLogString();
std::string     PySaveWindowAttributes_ToString(const SaveWindowAttributes *, const char *);

#endif

