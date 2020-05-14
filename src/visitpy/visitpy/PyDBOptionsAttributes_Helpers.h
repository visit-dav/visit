// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_DBOPTIONSATTRIBUTES_HELPERS_H
#define PY_DBOPTIONSATTRIBUTES_HELPERS_H
#include <Python.h>
#include <DBOptionsAttributes.h>
#include <visitpy_exports.h>

//
// Helpers for dealing with dict reps of our DB Options
//
VISITPY_API PyObject *  PyDBOptionsAttributes_CreateDictionaryFromDBOptions(const DBOptionsAttributes &opts, bool show_enum_opts);
VISITPY_API std::string PyDBOptionsAttributes_CreateDictionaryStringFromDBOptions(const DBOptionsAttributes &opts, bool show_enum_opts);

#endif

