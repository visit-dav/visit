// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PYTHON_FILTER_H
#define AVT_PYTHON_FILTER_H
#include <python_filters_exports.h>

#include <PythonInterpreter.h>

#include <iostream>


// ****************************************************************************
//  Class:  avtPythonFilter
//
//  Purpose:
//    Wraps an instance of pyavt.PythonFilter.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
class AVTPYTHON_FILTERS_API avtPythonFilter
{
public:
                 avtPythonFilter(PyObject *obj);
    virtual     ~avtPythonFilter();

    // helpers to handle get/set of attribute values.
    PyObject    *FetchAttribute(const std::string &att_name);

    bool         GetAttribute(const std::string &attr_name,int  &res);
    bool         GetAttribute(const std::string &attr_name,double &res);
    bool         GetAttribute(const std::string &attr_name,std::string &res);
    bool         GetAttribute(const std::string &attr_name,bool &res);

    bool         SetAttribute(const std::string &attr_name,int  val);
    bool         SetAttribute(const std::string &attr_name,double val);
    bool         SetAttribute(const std::string &attr_name,const std::string &val);
    bool         SetAttribute(const std::string &attr_name,bool val);

    bool         SetAttribute(const std::string &attr_name,PyObject *obj);


    PyObject    *PythonObject() {return pyObject;}

private:
    PyObject    *pyObject;

};


#endif
