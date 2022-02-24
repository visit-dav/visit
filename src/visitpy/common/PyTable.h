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

// Forward declarations
class QueryAttributes;

//
// Functions exposed to the VisIt module.
//

bool PyTable_Check(const PyObject*);

// Returns a Python dictionary containing a PyTable for node data and
//  a PyTable for zone data (depending on QueryAttributes content).
// Also contains entries for column names of the node/zone data.
PyObject *PyTable_CreateFromFlattenOutput(QueryAttributes&);

#endif
