// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  Modifications:
//
//    Kathleen Bonnell, Mon Mar 15 13:50:27 MST 2010
//    Use AVTPYTHON_FILTERS_API for proper symbol export an all platforms. 
//
// ****************************************************************************


#ifndef PY_CONTRACT_H
#define PY_CONTRACT_H

#include <avtContract.h>
#include <python_filters_exports.h>

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif


//
// Functions used to interface with Python Filter environment.
//

bool                 PyContract_Check(PyObject *obj);
avtContract_p        PyContract_FromPyObject(PyObject *obj);

PyObject AVTPYTHON_FILTERS_API *PyContract_Wrap(avtContract_p contract);

#endif
