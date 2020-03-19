// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_SILREQUEST_H
#define PY_SILREQUEST_H

#include <avtDataRequest.h>

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

//
// Functions used to interface with Python Filter environment.
//

bool                 PySILRequest_Check(PyObject *obj);
PyObject            *PySILRequest_Wrap(avtSILSpecification *request);


#endif
