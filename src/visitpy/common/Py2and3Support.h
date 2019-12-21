// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_2_AND_3_SUPPORT
#define PY_2_AND_3_SUPPORT
#include <Python.h>
#include <visitpy_exports.h>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Begin Functions to help with Python 2/3 Compatibility.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if defined(IS_PY3K)

//-----------------------------------------------------------------------------
int VISITPY_API PyString_Check(PyObject *o);

//-----------------------------------------------------------------------------
char * VISITPY_API PyString_AsString(PyObject *py_obj);

//-----------------------------------------------------------------------------
void VISITPY_API PyString_AsString_Cleanup(char *bytes);

//-----------------------------------------------------------------------------
PyObject * VISITPY_API PyString_FromString(const char *s);

//-----------------------------------------------------------------------------
PyObject * VISITPY_API PyString_FromStringAndSize(const char *v, Py_ssize_t len);

//-----------------------------------------------------------------------------
PyObject * VISITPY_API PyUnicode_From_UTF32_Unicode_Buffer(const char *unicode_buffer,
                                               int string_len);

//-----------------------------------------------------------------------------
int VISITPY_API PyInt_Check(PyObject *o);

//-----------------------------------------------------------------------------
PyObject * VISITPY_API PyInt_FromLong(long value);

//-----------------------------------------------------------------------------
long VISITPY_API PyInt_AsLong(PyObject *o);

//-----------------------------------------------------------------------------
long VISITPY_API PyInt_AS_LONG(PyObject *o);

//-----------------------------------------------------------------------------
long VISITPY_API PyInt_AS_LONG(PyObject *o);

//-----------------------------------------------------------------------------
PyObject *Py_FindMethod(PyMethodDef table[], PyObject *ob, char *name);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#else // python 2.6+
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#define PyString_AsString_Cleanup(c) { /* noop */ }



//-----------------------------------------------------------------------------
PyObject * VISITPY_API PyUnicode_From_UTF32_Unicode_Buffer(const char *unicode_buffer,
                                                           int string_len);
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// End Functions to help with Python 2/3 Compatibility.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif

