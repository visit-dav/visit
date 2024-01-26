// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PYTHON_INTERPRETER_H
#define PYTHON_INTERPRETER_H
#include <python_filters_exports.h>
#include <iostream>

// Forward Declare PyObject*
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif


// ****************************************************************************
//  Class:  PythonInterpreter
//
//  Purpose:
//    Simple embeddable python interprter.
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 2, 2008
//
//  Modifications:
//    Kathleen Biagas, Wed Jan 24, 2024
//    Added new method 'AddDLLPath'.
//
// ****************************************************************************
class AVTPYTHON_FILTERS_API PythonInterpreter
{
public:
                 PythonInterpreter();
    virtual     ~PythonInterpreter();

    bool         Initialize(int argc=0,char **argv=NULL);
    bool         IsRunning() { return running;}
    void         Reset();
    void         Shutdown();

    bool         AddSystemPath(const std::string &path);
#ifdef _WIN32
    bool         AddDLLPath(const std::string &path);
#endif
    bool         RunScript(const std::string &script);
    bool         RunScriptFile(const std::string &fname);

    bool         SetGlobalObject(PyObject *obj,
                                 const std::string &name);
    PyObject    *GetGlobalObject(const std::string &name);

    PyObject    *GlobalDict() {return globalDict;}

    bool         CheckError();
    void         ClearError();
    std::string  ErrorMessage() const { return errorMsg; }

    static bool  PyObjectToDouble(PyObject *,double &);
    static bool  PyObjectToString(PyObject *,std::string &);
    static bool  PyObjectToInteger(PyObject *,int &);

private:
    bool         PyTracebackToString(PyObject *,PyObject *,PyObject *,
                                     std::string &);

    bool         running;
    bool         error;
    std::string  errorMsg;

    PyObject    *mainModule;
    PyObject    *globalDict;

    PyObject    *traceModule;
    PyObject    *sioModule;
    PyObject    *tracePrintException;
    PyObject    *sioClass;

};


#endif
