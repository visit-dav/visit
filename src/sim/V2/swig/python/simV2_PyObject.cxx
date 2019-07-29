// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "simV2_PyObject.h"

// --------------------------------------------------------------------------
void simV2_PyObject::Assign(PyObject *rval)
{
    PyObject *tmp = Object;
    Object = rval;
    if (Object) { Py_INCREF(Object); }
    if (tmp) { Py_DECREF(tmp); }
}

// --------------------------------------------------------------------------
simV2_PyObject &simV2_PyObject::operator=(const simV2_PyObject &other)
{
    if (this != &other)
    {
        Assign(other.Object);
    }
    return *this;
}
