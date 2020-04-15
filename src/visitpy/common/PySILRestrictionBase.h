// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SILRESTRICTIONBASE_H
#define PY_SILRESTRICTIONBASE_H
#include <Python.h>
#include <avtSILRestriction.h>
#include "Py2and3Support.h"

//
// Functions exposed to the VisIt module.
//

bool                 PySILRestriction_Check(PyObject *obj);
PyObject            *PySILRestriction_Wrap(avtSILRestriction_p restriction);
avtSILRestriction_p  PySILRestriction_FromPyObject(PyObject *obj);


#endif
