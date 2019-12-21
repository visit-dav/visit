#include <Py2and3Support.h>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Begin Functions to help with Python 2/3 Compatibility.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if defined(IS_PY3K)

//-----------------------------------------------------------------------------
int
PyString_Check(PyObject *o)
{
    return PyUnicode_Check(o);
}

//-----------------------------------------------------------------------------
char *
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
PyObject *
PyString_FromString(const char *s)
{
    return PyUnicode_FromString(s);
}

//-----------------------------------------------------------------------------
void
PyString_AsString_Cleanup(char *bytes)
{
    free(bytes);
}

//-----------------------------------------------------------------------------
PyObject*
PyString_FromStringAndSize(const char *v, Py_ssize_t len)
{
    return PyUnicode_FromStringAndSize(v,len);
}

//-----------------------------------------------------------------------------
PyObject *
PyUnicode_From_UTF32_Unicode_Buffer(const char *unicode_buffer,
                                    int string_len)
{
    return PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND,
                                     unicode_buffer,
                                     string_len);
}

//-----------------------------------------------------------------------------
int
PyInt_Check(PyObject *o)
{
    return PyLong_Check(o);
}

//-----------------------------------------------------------------------------
PyObject *PyInt_FromLong(long value)
{
    return PyLong_FromLong(value);
}

//-----------------------------------------------------------------------------
long
PyInt_AsLong(PyObject *o)
{
    return PyLong_AsLong(o);
}


//-----------------------------------------------------------------------------
long
PyInt_AS_LONG(PyObject *o)
{
    return PyLong_AS_LONG(o);
}

//-----------------------------------------------------------------------------
PyObject *
Py_FindMethod(PyMethodDef /*table[] */, PyObject *ob, char *name)
{
    PyObject *py_name_str = PyString_FromString(name);
    PyObject *res = PyObject_GenericGetAttr(ob, py_name_str);
    Py_DECREF(py_name_str);
    return res;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#else // python 2.6+
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define PyString_AsString_Cleanup(c) { /* noop */ }



//-----------------------------------------------------------------------------
PyObject *
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