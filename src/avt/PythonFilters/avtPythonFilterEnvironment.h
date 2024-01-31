// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_PYTHON_FILTER_ENVIROMENT_H
#define AVT_PYTHON_FILTER_ENVIROMENT_H
#include <python_filters_exports.h>

#include <PythonInterpreter.h>
#include <avtPythonFilter.h>

#include <iostream>
#include <vector>


// ****************************************************************************
//  Class:  avtPythonFilterEnvironment
//
//  Purpose:
//    Wraps shared logic for avt/python filter handshaking.
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Feb  2 13:14:44 PST 2010
//
//  Modifications:
//   Cyrus Harrison, Fri Jul  9 10:31:03 PDT 2010
//   Use a singleton instance of the interpreter b/c python setup & teardown
//   does not work reliably w/ c modules that use static vars.
//
//   Cyrus Harrison,
//   Added pickle support.
//
//   Eric Brugger, Tue Jan 26 13:17:19 PST 2021
//   Modified Unpickle to take a char vector instead of a string.
//
// ****************************************************************************
class AVTPYTHON_FILTERS_API avtPythonFilterEnvironment
{
public:
                 avtPythonFilterEnvironment();
    virtual     ~avtPythonFilterEnvironment();

    bool         Initialize();
    bool         LoadFilter(const std::string &py_script);
    bool         FetchPythonError(std::string &msg_out);

    std::string  Pickle(PyObject *py_obj);
    PyObject    *Unpickle(const std::vector<char> &s);

    PyObject    *WrapVTKObject(void *obj,
                               const std::string &obj_type);

    void        *UnwrapVTKObject(PyObject *,
                                 const std::string &obj_type);

    PythonInterpreter  *Interpreter() { return pyi;}
    avtPythonFilter    *Filter() { return pyFilter;}

private:
    static void                PickleInit();

    static PythonInterpreter  *pyi;

    static bool                pickleReady;
    static PyObject           *pickleLoads;
    static PyObject           *pickleDumps;

    avtPythonFilter           *pyFilter;


};


#endif
