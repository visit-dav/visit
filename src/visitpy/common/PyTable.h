// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              PyTable.h                                    //
// ************************************************************************* //

#ifndef PY_TABLE_H
#define PY_TABLE_H
#include <Python.h>
#include <visitpy_exports.h>

//
// Functions exposed to the VisIt module.
//

bool PyTable_Check(const PyObject*);

// Creates a new PyTable PyObject, copying the data from the given buffer.
// Shape is an array of size 2 [nrows, ncolumns], row major ordering assumed.
PyObject *PyTable_Create(const float *data,
                         const long *shape);
PyObject *PyTable_Create(const double *data,
                         const long *shape);

#endif
