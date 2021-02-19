// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <Py2and3Support.h>
#include <PyVariant.h>


// ****************************************************************************
// Method: Wrap_BoolVector
//
// Purpose: 
//   Wraps a bool vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_BoolVector(const boolVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyInt_FromLong((long)vec[i]);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_UnsignedCharVector
//
// Purpose: 
//   Wraps a unsigned char vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_UnsignedCharVector(const unsignedCharVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyInt_FromLong((long)vec[i]);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_IntVector
//
// Purpose: 
//   Wraps a int vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_IntVector(const intVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyInt_FromLong((long)vec[i]);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_LongVector
//
// Purpose: 
//   Wraps a long vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_LongVector(const longVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyInt_FromLong(vec[i]);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_FloatVector
//
// Purpose: 
//   Wraps a float vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_FloatVector(const floatVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyFloat_FromDouble((double)vec[i]);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_DoubleVector
//
// Purpose: 
//   Wraps a double vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_DoubleVector(const doubleVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyFloat_FromDouble(vec[i]);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_CharVector
//
// Purpose: 
//   Wraps a Char vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_CharVector(const charVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyString_FromStringAndSize(&vec[i],1);
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}

// ****************************************************************************
// Method: Wrap_StringVector
//
// Purpose: 
//   Wraps a string vector into a python tuple.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:38:26 PST 2007
//
// ****************************************************************************

PyObject *
Wrap_StringVector(const stringVector &vec)
{
    size_t vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(size_t i = 0; i < vsize; i++)
    {
        PyObject *item = PyString_FromString(vec[i].c_str());
        if(item == NULL)
            continue;
        PyTuple_SET_ITEM(tuple, i, item);
    }
    return tuple;
}


///////////////////////////////////////////////////////////////////////////////
//
// Methods exposed to visit module.
//
///////////////////////////////////////////////////////////////////////////////


// ****************************************************************************
// Method: PyVariant_Wrap
//
// Purpose: 
//   Converts a variant to a python object. 
//
// Programmer: Cyrus Harrison
// Creation:   Mon Dec 17 15:20:25 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Jan  7 16:06:16 PST 2009
//   The enum that identifies the variant type is now in the Variant namespace.
//
// ****************************************************************************
PyObject *
PyVariant_Wrap(const Variant &var)
{
    // int types
    if(var.Type() == Variant::BOOL_TYPE)
    {
        return PyInt_FromLong((long) var.AsBool());
    }
    else if(var.Type() == Variant::UNSIGNED_CHAR_TYPE)
    {
        return PyInt_FromLong((long) var.AsUnsignedChar());
    }
    else if(var.Type() == Variant::INT_TYPE)
    {
        return PyInt_FromLong((long) var.AsInt());
    }
    else if(var.Type() == Variant::LONG_TYPE)
    {
        return PyInt_FromLong(var.AsLong());
    }
    else if(var.Type() == Variant::FLOAT_TYPE) // floating point types
    {
        return PyFloat_FromDouble((double)var.AsFloat());
    }
    else if(var.Type() == Variant::DOUBLE_TYPE)
    {
        return PyFloat_FromDouble(var.AsDouble());
    }
    else if(var.Type() == Variant::CHAR_TYPE) // string types
    {
        return PyString_FromStringAndSize(&var.AsChar(),1);
    }
    else if(var.Type() == Variant::STRING_TYPE)
    {
        return PyString_FromString(var.AsString().c_str());
    }
    if(var.Type() == Variant::BOOL_VECTOR_TYPE) // int vectors
    {
        return Wrap_BoolVector(var.AsBoolVector());
    }
    else if(var.Type() == Variant::UNSIGNED_CHAR_VECTOR_TYPE)
    {
        return Wrap_UnsignedCharVector(var.AsUnsignedCharVector());
    }
    else if(var.Type() == Variant::INT_VECTOR_TYPE)
    {
        return Wrap_IntVector(var.AsIntVector());
    }
    else if(var.Type() == Variant::LONG_VECTOR_TYPE)
    {
        return Wrap_LongVector(var.AsLongVector());
    }
    else if(var.Type() == Variant::FLOAT_VECTOR_TYPE) // floating point vectors
    {
        return Wrap_FloatVector(var.AsFloatVector());
    }
    else if(var.Type() == Variant::DOUBLE_VECTOR_TYPE)
    {
        return Wrap_DoubleVector(var.AsDoubleVector());
    }
    else if(var.Type() == Variant::CHAR_VECTOR_TYPE) // string vectors
    {
        return Wrap_CharVector(var.AsCharVector());
    }
    else if(var.Type() == Variant::STRING_VECTOR_TYPE)
    {
        return Wrap_StringVector(var.AsStringVector());
    }
    else // return none if error
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

