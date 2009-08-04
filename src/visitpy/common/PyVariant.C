/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <PyVariant.h>
#include <snprintf.h>


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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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
    int vsize = vec.size();
    PyObject *tuple = PyTuple_New(vsize);
    
    for(int i = 0; i < vsize; i++)
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

