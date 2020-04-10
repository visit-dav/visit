// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PY_2_AND_3_SUPPORT
#define PY_2_AND_3_SUPPORT
#include <Python.h>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Begin Functions to help with Python 2/3 Compatibility.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//#############################################################################
//#############################################################################
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Python 3
#if defined(IS_PY3K)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//#############################################################################
//#############################################################################

//-----------------------------------------------------------------------------
static int
PyString_Check(PyObject *o)
{
    return PyUnicode_Check(o);
}

//-----------------------------------------------------------------------------
static char *
PyString_AsString(PyObject *py_obj)
{
    char *res = NULL;
    if(PyUnicode_Check(py_obj))
    {
        PyObject * temp_bytes = PyUnicode_AsEncodedString(py_obj,
                                                          "ASCII",
                                                          "strict"); // Owned reference
        if(temp_bytes != NULL)
        {
            
            res = strdup(PyBytes_AS_STRING(temp_bytes));
            Py_DECREF(temp_bytes);
        }
        else
        {
            // TODO: Error
        }
    }
    else if(PyBytes_Check(py_obj))
    {
        res = strdup(PyBytes_AS_STRING(py_obj));
    }
    else
    {
        // TODO: ERROR or auto convert?
    }
    
    return res;
}

//-----------------------------------------------------------------------------
static PyObject *
PyString_FromString(const char *s)
{
    return PyUnicode_FromString(s);
}

//-----------------------------------------------------------------------------
static Py_ssize_t
PyString_Size(PyObject *s)
{
    return PyUnicode_GetLength(s);
}

//-----------------------------------------------------------------------------
static void
PyString_AsString_Cleanup(char *bytes)
{
    free(bytes);
}

//-----------------------------------------------------------------------------
static PyObject*
PyString_FromStringAndSize(const char *v, Py_ssize_t len)
{
    return PyUnicode_FromStringAndSize(v,len);
}

//-----------------------------------------------------------------------------
static PyObject *
PyUnicode_From_UTF32_Unicode_Buffer(const char *unicode_buffer,
                                    int string_len)
{
    return PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND,
                                     unicode_buffer,
                                     string_len);
}

//-----------------------------------------------------------------------------
static int
PyInt_Check(PyObject *o)
{
    return PyLong_Check(o);
}

//-----------------------------------------------------------------------------
static int
PyInt_CheckExact(PyObject *o)
{
    return PyLong_CheckExact(o);
}

//-----------------------------------------------------------------------------
static PyObject *PyInt_FromLong(long value)
{
    return PyLong_FromLong(value);
}

//-----------------------------------------------------------------------------
static long
PyInt_AsLong(PyObject *o)
{
    return PyLong_AsLong(o);
}


//-----------------------------------------------------------------------------
static long
PyInt_AS_LONG(PyObject *o)
{
    return PyLong_AS_LONG(o);
}

//-----------------------------------------------------------------------------
static PyObject*
PyNumber_Int(PyObject *o)
{
    return PyNumber_Long(o);
}

//-----------------------------------------------------------------------------
// Note: Make sure to use PyMethodDef *, to match PyMethodDef table[]
static PyObject *
Py_FindMethod(PyMethodDef *table, PyObject *ob, char *name)
{
    /* cannot comment out to avoid unused warning b/c C*/
    (void)table;
    
    PyObject *py_name_str = PyString_FromString(name);
    PyObject *res = PyObject_GenericGetAttr(ob, py_name_str);
    Py_DECREF(py_name_str);
    return res;
}


//-----------------------------------------------------------------------------
// ref: https://stackoverflow.com/questions/15962847/what-happened-to-py-flushline-in-python-3-3
static int
Py_FlushLine(void)
{
       PyObject *f = PySys_GetObject("stdout");
       if (f == NULL)
               return 0;
       return PyFile_WriteString("\n", f);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef __cplusplus
// B/c these work as func sign overrides, they can only be used in C++ code
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static void
Py_SetProgramName(char *name)
{
    wchar_t *w_prog_name = Py_DecodeLocale(name, NULL);
    Py_SetProgramName(w_prog_name);
    PyMem_RawFree(w_prog_name);
}

//-----------------------------------------------------------------------------
static void
PySys_SetArgv(int argc, char **argv)
{
    // alloc ptrs for encoded ver
    wchar_t** wargv= (wchar_t**) malloc(sizeof(wchar_t*) * argc);
    // keep things C only since we cases where
    // the cpp compiler is not being used. (py_visit_writer.c)

    // std::vector<wchar_t*> wargv(argc);

    for(int i = 0; i < argc; i++)
    {
        wargv[i] = Py_DecodeLocale(argv[i], NULL);
    }
    
    PySys_SetArgv(argc,&wargv[0]);
    
    for(int i = 0; i < argc; i++)
    {
        PyMem_RawFree(wargv[i]);
    }

    free(wargv);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//#############################################################################
//#############################################################################
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#else // python 2.6+
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//#############################################################################
//#############################################################################

//-----------------------------------------------------------------------------
#define PyString_AsString_Cleanup(c) { /* noop */ }



//-----------------------------------------------------------------------------
static PyObject * 
PyUnicode_From_UTF32_Unicode_Buffer(const char *unicode_buffer,
                                    int string_len)
{
    return PyUnicode_Decode(unicode_buffer,
                             string_len,
                             "utf-32",
                             "strict");
}
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// End Functions to help with Python 2/3 Compatibility.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif

