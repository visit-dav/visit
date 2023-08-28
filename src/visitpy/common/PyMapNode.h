// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_MAP_NODE_H
#define PY_MAP_NODE_H
#include <string>
#include <Python.h>
#include <MapNode.h>
#include <visitpy_exports.h>

//
// Helper to convert a VisIt MapNode to a python dictonary.
//
VISITPY_API PyObject *PyMapNode_Wrap(const MapNode&);

//
// Helper to convert a Python Dictionary  to a VisIt MapNode.
//
static std::string PyMapNode_VoidString = "__void__";
VISITPY_API bool PyDict_To_MapNode(PyObject *, MapNode&, std::string& errmsg = PyMapNode_VoidString);

#endif

