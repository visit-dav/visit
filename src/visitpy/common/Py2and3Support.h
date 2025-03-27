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
Py_FindMethod(PyMethodDef *table, PyObject *obj, char *name)
{
    // search our static table for a named method
    // recipe adapted from:
    // https://github.com/encukou/py3c/issues/22

    PyObject * res = NULL;
    for (int i = 0; table[i].ml_name != NULL; ++i)
    {
        if (strcmp(name, table[i].ml_name) == 0)
        {
            res = PyCFunction_New(&table[i], obj);
            if(res!= NULL)
            {
                Py_INCREF(res);
                return res;
            }
        }
    }

    if(res == NULL)
    {
        char msg[1024];
        snprintf(msg, 1024, "Could not find method with name `%s`", name);
        // set error if we didn't find the method we expected to find
        PyErr_SetString(PyExc_ValueError, msg);
    }
    return NULL;
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


#if defined(IS_PY3K) /* python 3 */
#define  PY_VISIT_TPFLAGS_DEFAULT Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE
#else /* python 2 */
#define  PY_VISIT_TPFLAGS_DEFAULT Py_TPFLAGS_CHECKTYPES
#endif

//-----------------------------------------------------------------------------
// PyVarObject_TAIL is used at the end of each PyVarObject def
// to make sure we have the correct number of initializers across python
// versions.
//-----------------------------------------------------------------------------
#ifdef Py_TPFLAGS_HAVE_FINALIZE
#define PyVarObject_TAIL ,0
#else
#define PyVarObject_TAIL
#endif

//
// Mark C. Miller, Wed Mar 26 18:58:25 PDT 2025
// Build a block of code representing the default set of .tp_xxx slot methods
// our standard objects need to define. Its likely overly cautious but the reason
// for the funky retval incrimenting here is to ensure there is no way any compiler
// optimizations could maybe decide the function in which this code block appears
// could ever be optimized away because its return value could maybe be computed at
// compile time.
//
// For non-standard objects, the correct approach is to #undef this macro and
// then re-define it to what is needed AHEAD of invoking VISIT_PY_TYPE_OBJ().
//
// We make prolific use of the CPP paste operator (##) here to enforce naming
// consistency.
//
#define VISIT_PY_TYPE_OBJ_TP_SLOTS(VSObjName)                                \
    Py##VSObjName##Type.tp_dealloc = Py##VSObjName##_dealloc;                \
    retval += ((void*) Py##VSObjName##Type.tp_dealloc != (void*)0);          \
    Py##VSObjName##Type.tp_repr = Py##VSObjName##_str;                       \
    Py##VSObjName##Type.tp_str = Py##VSObjName##_str;                        \
    retval += ((void*) Py##VSObjName##Type.tp_str != (void*)0);              \
    Py##VSObjName##Type.tp_getattro = Py##VSObjName##_getattro;              \
    retval += ((void*) Py##VSObjName##Type.tp_getattro != (void*)0);         \
    Py##VSObjName##Type.tp_setattro = Py##VSObjName##_setattro;              \
    retval += ((void*) Py##VSObjName##Type.tp_setattro != (void*)0);         \
    Py##VSObjName##Type.tp_richcompare = Py##VSObjName##_richcompare;        \
    retval += ((void*) Py##VSObjName##Type.tp_richcompare != (void*)0);      \
    Py##VSObjName##Type.tp_methods = Py##VSObjName##_methods;                \
    retval += ((void*) Py##VSObjName##Type.tp_methods != (void*)0)

// 2023/01/27 CYRUSH Note:
// tp_print / tp_vectorcall_offset slot is not used by python 3.0 - 3.7
// it should not be used for python 3.8 > unless Py_TPFLAGS_HAVE_VECTORCALL
// is passed, but we choose to always set to 0
//
// Also we use VPY_STR to implement both str() and repr(), as this
// preserves the Python 2 cli behavior to echo object contents via repr()
//
// Mark C. Miller, Mon Mar 24 16:07:55 PDT 2025
// Adjust VISIT_PY_TYPE_OBJ to initialize our python objects in two steps.
// The first is an *assignment* of the PyTypeObject struct with mostly zero
// entries and other critical boilerplate stuff. The second is the assignment
// to a dummy static variable of the result of calling a static initialization
// function for the type. It is in that function where the remaining .tp_xxx
// slots are set up according to the (above) VISIT_PY_TYPE_OBJ_TP_SLOTS macro. 
// To override the default behavior, simply #undef VISIT_PY_TYPE_OBJ_TP_SLOTS
// and then re-define it to what is needed before invoking VISIT_PY_TYPE_OBJ.
//
// We make prolific use of the CPP paste operator (##) here to enforce naming
// consistency.
//
#define VISIT_PY_TYPE_OBJ(VSObjName)                                         \
static PyTypeObject Py##VSObjName##Type =                                    \
{                                                                            \
    PyVarObject_HEAD_INIT(&PyType_Type, 0)                                   \
    #VSObjName,                 /* tp_name */                                \
    sizeof(Py##VSObjName##Object),/* tp_basicsize */                         \
    0,                          /* tp_itemsize */                            \
    0,                          /* tp_dealloc */                             \
    0,                          /* tp_print (py2 and < py3.8)... */          \
                                /* ...or tp_vectorcall_offset (>py3.8 */     \
    0,                          /* tp_getattr */                             \
    0,                          /* tp_setattr */                             \
    0,                          /* tp_reserved */                            \
    0,                          /* tp_repr */                                \
    0,                          /* tp_as_number */                           \
    0,                          /* tp_as_sequence */                         \
    0,                          /* tp_as_mapping */                          \
    0,                          /* tp_hash  */                               \
    0,                          /* tp_call */                                \
    0,                          /* tp_str */                                 \
    0,                          /* tp_getattro */                            \
    0,                          /* tp_setattro */                            \
    0,                          /* tp_as_buffer */                           \
    PY_VISIT_TPFLAGS_DEFAULT,   /* tp_flags */                               \
    Py##VSObjName##_purpose,    /* tp_doc */                                 \
    0,                          /* tp_traverse */                            \
    0,                          /* tp_clear */                               \
    0,                          /* tp_richcompare */                         \
    0,                          /* tp_weaklistoffset */                      \
    0,                          /* tp_iter */                                \
    0,                          /* tp_iternext */                            \
    0,                          /* tp_methods */                             \
    0,                          /* tp_members */                             \
    0,                          /* tp_getset */                              \
    0,                          /* tp_base */                                \
    0,                          /* tp_dict */                                \
    0,                          /* tp_descr_get */                           \
    0,                          /* tp_descr_set */                           \
    0,                          /* tp_dictoffset */                          \
    0,                          /* tp_init */                                \
    0,                          /* tp_alloc */                               \
    0,                          /* tp_new */                                 \
    0,                          /* tp_free */                                \
    0,                          /* tp_is_gc */                               \
    0,                          /* tp_bases */                               \
    0,                          /* tp_mro */                                 \
    0,                          /* tp_cache */                               \
    0,                          /* tp_subclasses */                          \
    0,                          /* tp_weaklist */                            \
    0,                          /* tp_del */                                 \
    0                           /* tp_version_tag */                         \
    PyVarObject_TAIL                                                         \
};                                                                           \
static int VSObjName##_init(void) {                                          \
    static int retval = 0;                                                   \
    VISIT_PY_TYPE_OBJ_TP_SLOTS(VSObjName);                                   \
    return retval;                                                           \
}                                                                            \
static int VSObjName##_initialized = VSObjName##_init()

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// End Functions to help with Python 2/3 Compatibility.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#endif
