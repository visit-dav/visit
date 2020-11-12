// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "Python.h"
#include <Py2and3Support.h>
#include "avtPythonFilter.h"

#include <visit-config.h>
#include <sstream>
#include <Environment.h>

using namespace std;

// ****************************************************************************
//  Method:  avtPythonFilter constructor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************

avtPythonFilter::avtPythonFilter(PyObject *obj)
: pyObject(obj)
{

}

// ****************************************************************************
//  Method:  avtPythonFilter destructor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************

avtPythonFilter::~avtPythonFilter()
{
    Py_DECREF(pyObject);
}


// ****************************************************************************
//  Method: avtPythonFilterEnvironment::FetchAttribute
//
//  Purpose:
//      Helper for obtaining a python object pointer to named filter attribute.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
PyObject *
avtPythonFilter::FetchAttribute(const string &attr_name)
{
    PyObject *py_obj = PyObject_GetAttrString(pyObject,attr_name.c_str());
    // don't want to create a new ref, so by decrementing the ref count
    // we effectivly create a "borrowed" ref here
    if(py_obj)
        Py_DECREF(py_obj);
    return py_obj;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::GetAttribute
//
//  Purpose:
//      Helper for obtaining an integer value from named filter attribute.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::GetAttribute(const string &attr_name, int &res)
{
    PyObject *py_obj = PyObject_GetAttrString(pyObject,attr_name.c_str());
    if(py_obj== NULL)
        return false;

    if(!PyInt_Check(py_obj))
    {
        Py_DECREF(py_obj);
        return false;
    }

    long lres = PyInt_AsLong(py_obj);
    Py_DECREF(py_obj);
    //if(lres == -1); // check for python error.

    res = int(lres);
    return true;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::GetAttribute
//
//  Purpose:
//      Helper for obtaining a double value from named filter attribute.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::GetAttribute(const string &attr_name, double &res)
{
    PyObject *py_obj = PyObject_GetAttrString(pyObject,attr_name.c_str());
    if(py_obj == NULL)
        return false;

    if(!PyFloat_Check(py_obj))
    {
        Py_DECREF(py_obj);
        return false;
    }

    res = PyFloat_AsDouble(py_obj);
    Py_DECREF(py_obj);

    return true;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::GetAttribute
//
//  Purpose:
//      Helper for obtaining a string value from named filter attribute.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::GetAttribute(const string &attr_name, string &res)
{
    PyObject *py_obj = PyObject_GetAttrString(pyObject,attr_name.c_str());
    if(py_obj == NULL)
        return false;

    if(!PyString_Check(py_obj))
    {
        Py_DECREF(py_obj);
        return false;
    }

    res = PyString_AsString(py_obj);
    Py_DECREF(py_obj);

    return true;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::GetAttribute
//
//  Purpose:
//      Helper for obtaining a boolean value from named filter attribute.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::GetAttribute(const string &attr_name, bool &res)
{
    PyObject *py_obj = PyObject_GetAttrString(pyObject,attr_name.c_str());
    if(py_obj == NULL)
        return false;

    if(!PyBool_Check(py_obj))
    {
        Py_DECREF(py_obj);
        return false;
    }

    if(py_obj == Py_True)
        res = true;
    else if(py_obj == Py_False)
        res = false;

    Py_DECREF(py_obj);

    return true;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::SetAttribute
//
//  Purpose:
//      Helper for setting a named filter attribute to an interger value.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::SetAttribute(const string &attr_name, int val)
{
    PyObject *py_val = PyInt_FromLong(long(val));
    if(PyObject_SetAttrString(pyObject, attr_name.c_str(),py_val) != -1)
        return true;
    else
        return false;

}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::SetAttribute
//
//  Purpose:
//      Helper for setting a named filter attribute to a double value.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::SetAttribute(const string &attr_name, double val)
{
    PyObject *py_val = PyFloat_FromDouble(val);
    if(PyObject_SetAttrString(pyObject, attr_name.c_str(),py_val) != -1)
        return true;
    else
        return false;

}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::SetAttribute
//
//  Purpose:
//      Helper for setting a named filter attribute to a string value.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::SetAttribute(const string &attr_name, const string &val)
{
    PyObject *py_val = PyString_FromString(val.c_str());
    if(PyObject_SetAttrString(pyObject, attr_name.c_str(),py_val) != -1)
        return true;
    else
        return false;

}


// ****************************************************************************
//  Method: avtPythonFilterEnvironment::SetAttribute
//
//  Purpose:
//      Helper for setting a named filter attribute to a boolean value.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::SetAttribute(const string &attr_name, bool val)
{
    PyObject *py_val = NULL;
    if(val)
        py_val = Py_True;
    else
        py_val = Py_False;

    Py_INCREF(py_val);

    if(PyObject_SetAttrString(pyObject, attr_name.c_str(),py_val) != -1)
        return true;
    else
        return false;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::SetAttribute
//
//  Purpose:
//      Helper for setting a named filter attribute to a python object.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilter::SetAttribute(const string &attr_name, PyObject *obj)
{
    if(PyObject_SetAttrString(pyObject, attr_name.c_str(),obj) != -1)
        return true;
    else
        return false;
}

