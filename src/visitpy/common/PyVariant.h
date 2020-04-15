// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_VARIANT_H
#define PY_VARIANT_H
#include <Python.h>
#include <Variant.h>
#include "Py2and3Support.h"

//
// Helper to convert a VisIt variant to a builtin python type.
//

PyObject *PyVariant_Wrap(const Variant &);

#endif

