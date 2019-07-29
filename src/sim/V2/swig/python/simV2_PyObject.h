// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef simV2_PyObject_h
#define simV2_PyObject_h

#include <Python.h>
#include <cstddef>

// an object to hold and release a reference to a
// PyObject.
class simV2_PyObject
{
public:
    ~simV2_PyObject(){ Reset(); }
    simV2_PyObject() : Object(0) {}
    simV2_PyObject(PyObject *object) : Object(0) { Assign(object); }
    simV2_PyObject(const simV2_PyObject &other) : Object(0) { *this = other; }
    simV2_PyObject &operator=(const simV2_PyObject &other);

    operator bool() const { return Object!=NULL; }
    operator PyObject*() { return Object; }
    PyObject *operator->() { return Object; }

    void Reset(){ Assign(NULL); }

    void SetObject(PyObject *object){ Assign(object); }
    PyObject *GetObject(){ return Object; }

private:
    void Assign(PyObject *rval);

private:
    PyObject *Object;
};

#endif
