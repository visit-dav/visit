#ifndef PY_MOLECULEATTRIBUTES_H
#define PY_MOLECULEATTRIBUTES_H
#include <Python.h>
#include <MoleculeAttributes.h>

//
// Functions exposed to the VisIt module.
//
void            PyMoleculeAttributes_StartUp(MoleculeAttributes *subj, void *data);
void            PyMoleculeAttributes_CloseDown();
PyMethodDef    *PyMoleculeAttributes_GetMethodTable(int *nMethods);
bool            PyMoleculeAttributes_Check(PyObject *obj);
MoleculeAttributes *PyMoleculeAttributes_FromPyObject(PyObject *obj);
PyObject       *PyMoleculeAttributes_NewPyObject();
PyObject       *PyMoleculeAttributes_WrapPyObject(const MoleculeAttributes *attr);
void            PyMoleculeAttributes_SetDefaults(const MoleculeAttributes *atts);
std::string     PyMoleculeAttributes_GetLogString();
std::string     PyMoleculeAttributes_ToString(const MoleculeAttributes *, const char *);

#endif

