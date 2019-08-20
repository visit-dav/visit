// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DATAREQUEST_H
#define PY_DATAREQUEST_H

#include <avtDataRequest.h>

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif


//
// Functions used to interface with Python Filter environment.
//

bool                 PyDataRequest_Check(PyObject *obj);
avtDataRequest_p     PyDataRequest_FromPyObject(PyObject *obj);
PyObject            *PyDataRequest_Wrap(avtDataRequest_p request);

#endif
