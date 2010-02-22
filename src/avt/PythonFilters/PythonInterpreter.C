/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include "Python.h"
#include "PythonInterpreter.h"
#include <fstream>
#include <sstream>
#include <DebugStream.h>
using namespace std;

// ****************************************************************************
//  Method:  PythonInterpreter constructor
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 2, 2008
//
// ****************************************************************************

PythonInterpreter::PythonInterpreter()
{
    running    = false;
    error      = false;
    mainModule = NULL;
    globalDict = NULL;
}

// ****************************************************************************
//  Method:  PythonInterpreter destructor
//
//  Programmer:  Cyrus Harrison
//  Creation:    May 2, 2008
//
// ****************************************************************************

PythonInterpreter::~PythonInterpreter()
{
    // Shutdown the interpreter if running.
    Shutdown();
}

// ****************************************************************************
//  Method: PythonInterpreter::Initialize
//
//  Purpose:
//      Starts the python interpreter. If no arguments are passed creates
//      suitable dummy arguments.
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 2, 2008
//
// ****************************************************************************

bool
PythonInterpreter::Initialize(int argc, char **argv)
{
    // if already running, ignore
    if(running)
        return true;

    // Init Python
    Py_Initialize();
    PyEval_InitThreads();

    if(argc == 0 || argv == NULL)
    {
        char *progname = (char*)"visit_embedded_py";
        Py_SetProgramName(progname);
        PySys_SetArgv(1, &progname);
    }
    else
    {
        Py_SetProgramName(argv[0]);
        PySys_SetArgv(argc, argv);
    }


    // setup up __main__ and capture StdErr
    PyRun_SimpleString("import os,sys,traceback,StringIO\n");
    if(CheckError())
        return false;

    // all of these PyObject*s are borrowed refs
    mainModule = PyImport_AddModule((char*)"__main__");
    globalDict = PyModule_GetDict(mainModule);

    // get objects that help us print an exception.

    // get ref to traceback.print_exception method
    traceModule = PyImport_AddModule("traceback");
    PyObject *traceDict = PyModule_GetDict(traceModule);
    tracePrintException = PyDict_GetItemString(traceDict,"print_exception");
    // get ref to StringIO.StringIO class
    sioModule   = PyImport_AddModule("StringIO");
    PyObject *sioDict= PyModule_GetDict(sioModule);
    sioClass = PyDict_GetItemString(sioDict,"StringIO");
    running = true;
    
    return true;
}


// ****************************************************************************
//  Method: PythonInterpreter::Reset
//
//  Purpose:
//      Resets the state of the interpreter if it is running.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 2, 2008
//
// ****************************************************************************

void
PythonInterpreter::Reset()
{
    if(running)
    {
        // clean gloal dict.
        PyDict_Clear(globalDict);
    }
}


// ****************************************************************************
//  Method: PythonInterpreter::Shutdown
//
//  Purpose:
//      Shuts down the interpreter if it is running.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 2, 2008
//
// ****************************************************************************

void
PythonInterpreter::Shutdown()
{
    if(running)
    {
        Py_Finalize();
        running = false;
    }
}

// ****************************************************************************
//  Method: PythonInterpreter::AddSystemPath
//
//  Purpose:
//      Adds passed path to "sys.path"
//
//  Arguments:
//      path    Python library path
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 2, 2008
//
// ****************************************************************************

bool
PythonInterpreter::AddSystemPath(const std::string &path)
{
    return RunScript("sys.path.insert(1,'" + path + "')\n");
}

// ****************************************************************************
//  Method: PythonInterpreter::RunScript
//
//  Purpose:
//      Executes passed python script in the interpreter.
//
//  Arguments:
//      script      String containing python code to execute. 
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 2, 2008
//
// ****************************************************************************

bool
PythonInterpreter::RunScript(const std::string &script)
{
    bool res = false;
    if(running)
    {
        debug5 << "PythonInterpreter::RunScript " << script;
        PyRun_String((char*)script.c_str(),
                     Py_file_input,
                     globalDict,
                     globalDict);
        if(!CheckError())
            res = true;
    }
    return res;
}

// ****************************************************************************
//  Method: PythonInterpreter::RunScriptFile
//
//  Purpose:
//      Executes passed python script in the interpreter.
//
//  Arguments:
//      fname      Script file name.
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 2, 2008
//
// ****************************************************************************

bool
PythonInterpreter::RunScriptFile(const std::string &fname)
{
    ifstream ifs(fname.c_str());
    string py_script((istreambuf_iterator<char>(ifs)),
                     istreambuf_iterator<char>());
    ifs.close();
    return RunScript(py_script);
}


// ****************************************************************************
//  Method: PythonInterpreter::SetGlobalObject
//
//  Purpose:
//      Adds C python object to the global dictonary.
//
//  Arguments:
//      obj       Python object to register.
//      py_name   Name for the object in the global dictionary.
//
//  Programmer:   Cyrus Harrison
//  Creation:     March 23, 2009
//
// ****************************************************************************

bool
PythonInterpreter::SetGlobalObject(PyObject *obj,
                                   const string &py_name)
{
    PyDict_SetItemString(globalDict, py_name.c_str(), obj);
    return !CheckError();
}

// ****************************************************************************
//  Method: PythonInterpreter::GetGlobalObject
//
//  Purpose:
//      Get C python object from the global dictonary.
//
//  Arguments:
//      py_name   Name of the object in the global dictionary.
//
//  Programmer:   Cyrus Harrison
//  Creation:     March 23, 2009
//
// ****************************************************************************

PyObject *
PythonInterpreter::GetGlobalObject(const string &py_name)
{
    PyObject *res = PyDict_GetItemString(globalDict, py_name.c_str());
    if(CheckError())
        res = NULL;
    return res;
}

// ****************************************************************************
//  Method: PythonInterpreter::CheckError
//
//  Purpose:
//      Checks python error state and constructs approprate error message
//      if an error did occur. It can be used to check for errors in both
//      python scripts & calls to the C-API. The differnce between these
//      to cases is bascially the existance of a python traceback.
//
//      Note: This method clears the python error state, but it will continue 
//      to return "true" indicating an error until ClearError() is called.
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 7, 2008
//
// ****************************************************************************

bool 
PythonInterpreter::CheckError()
{
    if(PyErr_Occurred())
    {
        error = true;
        errorMsg = "<Unknown Error>";

        string sval ="";
        PyObject *py_etype;
        PyObject *py_eval;
        PyObject *py_etrace;

        PyErr_Fetch(&py_etype,&py_eval,&py_etrace);
        if(py_etype)
        {
            PyErr_NormalizeException(&py_etype,&py_eval,&py_etrace);

            if(PyObjectToString(py_etype,sval))
                errorMsg = sval;

            if(py_eval)
                if(PyObjectToString(py_eval,sval))
                    errorMsg += sval;

            if(py_etrace)
                if(PyTracebackToString(py_etype,py_eval,py_etrace,sval))
                    errorMsg += "\n" + sval;
        }
        PyErr_Restore(py_etype,py_eval,py_etrace);
        PyErr_Clear();
    }

    return error;
}

// ****************************************************************************
//  Method: PythonInterpreter::ClearError
//
//  Purpose:
//      Clears environment error flag and message.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     May 7, 2008
//
// ****************************************************************************

void 
PythonInterpreter::ClearError()
{
    if(error)
    {
        error = false;
        errorMsg = "";
    }
}

// ****************************************************************************
//  Method: PythonInterpreter::PyObjectToDouble
//
//  Purpose:
//      Helper to that calls float() on a python object and returns the result
//      as a double.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     Fri Feb  5 15:35:41 PST 2010
//
// ****************************************************************************

bool
PythonInterpreter::PyObjectToDouble(PyObject *py_obj, double &res)
{
    if(PyFloat_Check(py_obj))
    {
        res = PyFloat_AS_DOUBLE(py_obj);
        return true;
    }

    if(PyInt_Check(py_obj))
    {
        res = (double) PyInt_AS_LONG(py_obj);
        return true;
    }

    if(PyLong_Check(py_obj))
    {
        res = PyLong_AsDouble(py_obj);
        return true;
    }

    if(PyNumber_Check(py_obj) != 1)
        return false;

    PyObject *py_val = PyNumber_Float(py_obj);
    if(py_val == NULL)
        return false;
    res = PyFloat_AS_DOUBLE(py_val);
    Py_DECREF(py_val);
    return true;
}

// ****************************************************************************
//  Method: PythonInterpreter::PyObjectToInteger
//
//  Purpose:
//      Helper to that calls int() on a python object and returns the result
//      as a double.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     April 15, 2009
//
// ****************************************************************************

bool
PythonInterpreter::PyObjectToInteger(PyObject *py_obj, int &res)
{
    if(PyInt_Check(py_obj))
    {
        res = (int)PyInt_AS_LONG(py_obj);
        return true;
    }

    if(PyLong_Check(py_obj))
    {
        res = (int)PyLong_AsLong(py_obj);
        return true;
    }

    if(PyNumber_Check(py_obj) != 1)
        return false;

    PyObject *py_val = PyNumber_Int(py_obj);
    if(py_val == NULL)
        return false;
    res = (int) PyInt_AS_LONG(py_val);
    Py_DECREF(py_val);
    return true;
}

// ****************************************************************************
//  Method: PythonInterpreter::PyObjectToString
//
//  Purpose:
//      Helper to that calls str() on a python object and returns the result
//      as a C++ string.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     April 15, 2009
//
// ****************************************************************************

bool
PythonInterpreter::PyObjectToString(PyObject *py_obj, std::string &res)
{
    PyObject *py_obj_str = PyObject_Str(py_obj);
    if(py_obj_str == NULL)
        return false;

    res = PyString_AS_STRING(py_obj_str);
    Py_DECREF(py_obj_str);
    return true;
}

// ****************************************************************************
//  Method: PythonInterpreter::PyTracebackToString
//
//  Purpose:
//      Helper to turns a python traceback into a human readable string.
//
//
//  Programmer:   Cyrus Harrison
//  Creation:     April 15, 2009
//
// ****************************************************************************

bool
PythonInterpreter::PyTracebackToString(PyObject *py_etype,
                                       PyObject *py_eval,
                                       PyObject *py_etrace,
                                       std::string &res)
{
    if(!py_eval)
        py_eval = Py_None;

    // create a StringIO object "buffer" to print traceback into.
    PyObject *py_args = Py_BuildValue("()");
    PyObject *py_buffer = PyObject_CallObject(sioClass,py_args);
    Py_DECREF(py_args);

    if(!py_buffer)
    {
        PyErr_Print();
        return false;
    }

    // call traceback.print_tb(etrace,file=buffer)
    PyObject *py_res = PyObject_CallFunction(tracePrintException,
                                             (char*)"OOOOO",
                                             py_etype,
                                             py_eval,
                                             py_etrace,
                                             Py_None,
                                             py_buffer);
    if(!py_res)
    {
        PyErr_Print();
        return false;
    }

    // call buffer.getvalue() to get python string object
    PyObject *py_str = PyObject_CallMethod(py_buffer,(char*)"getvalue",NULL);
    

    if(!py_str)
    {
        PyErr_Print();
        return false;
    }

    // convert python string object to std::string
    res = PyString_AS_STRING(py_str);

    Py_DECREF(py_buffer);
    Py_DECREF(py_res);
    Py_DECREF(py_str);

    return true;
}



