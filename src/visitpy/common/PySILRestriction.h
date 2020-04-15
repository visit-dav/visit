// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SILRESTRICTION_H
#define PY_SILRESTRICTION_H
#include <Python.h>
#include "Py2and3Support.h"

//
// Functions exposed to the VisIt module.
//

PyMethodDef         *PySILRestriction_GetMethodTable(int *nMethods);
PyObject            *PySILRestriction_NewPyObject();

#endif
