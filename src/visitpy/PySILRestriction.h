#ifndef PY_SILRESTRICTION_H
#define PY_SILRESTRICTION_H
#include <Python.h>
#include <avtSILRestriction.h>

//
// Functions exposed to the VisIt module.
//
PyMethodDef         *PySILRestriction_GetMethodTable(int *nMethods);
bool                 PySILRestriction_Check(PyObject *obj);
avtSILRestriction_p  PySILRestriction_FromPyObject(PyObject *obj);
PyObject            *PySILRestriction_NewPyObject();

#endif
