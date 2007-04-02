#ifndef PY_SUBDIVIDEQUADSATTRIBUTES_H
#define PY_SUBDIVIDEQUADSATTRIBUTES_H
#include <Python.h>
#include <SubdivideQuadsAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PySubdivideQuadsAttributes_StartUp(SubdivideQuadsAttributes *subj, void *data);
void            PySubdivideQuadsAttributes_CloseDown();
PyMethodDef    *PySubdivideQuadsAttributes_GetMethodTable(int *nMethods);
bool            PySubdivideQuadsAttributes_Check(PyObject *obj);
SubdivideQuadsAttributes *PySubdivideQuadsAttributes_FromPyObject(PyObject *obj);
PyObject       *PySubdivideQuadsAttributes_NewPyObject();
PyObject       *PySubdivideQuadsAttributes_WrapPyObject(const SubdivideQuadsAttributes *attr);
std::string     PySubdivideQuadsAttributes_GetLogString();
void            PySubdivideQuadsAttributes_SetDefaults(const SubdivideQuadsAttributes *atts);

#endif

