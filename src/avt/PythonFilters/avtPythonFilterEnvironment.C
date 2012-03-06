/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include "avtPythonFilterEnvironment.h"
#include "avtParallel.h"

#include <visit-config.h>
#include <sstream>
#include <DebugStream.h>
#include <Environment.h>
#include <StringHelpers.h>
#include <InstallationFunctions.h>

using namespace std;

// static vars
PythonInterpreter *avtPythonFilterEnvironment::pyi=NULL;

// (pickle related)
bool      avtPythonFilterEnvironment::pickleReady=false;
PyObject *avtPythonFilterEnvironment::pickleDumps=NULL;
PyObject *avtPythonFilterEnvironment::pickleLoads=NULL;


// ****************************************************************************
//  Method:  avtPythonFilterEnvironment constructor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Feb  2 13:14:44 PST 2010
//
//  Modifications:
//   Cyrus Harrison, Fri Jul  9 10:31:03 PDT 2010
//   Init singleton instance of the python interpreter.
//
// ****************************************************************************

avtPythonFilterEnvironment::avtPythonFilterEnvironment()
: pyFilter(NULL)
{
    if(pyi == NULL)
        pyi = new PythonInterpreter();
}

// ****************************************************************************
//  Method:  avtPythonFilterEnvironment destructor
//
//  Programmer:  Cyrus Harrison
//  Creation:    Tue Feb  2 13:14:44 PST 2010
//
//  Modifications:
//   Cyrus Harrison, Fri Jul  9 10:31:03 PDT 2010
//   Handle filter cleanup. This was previously done in the 'Shutdown'
//   method, which was removed due to the singleton use of the python
//   interpreter.
//
// ****************************************************************************

avtPythonFilterEnvironment::~avtPythonFilterEnvironment()
{
    if(pyFilter)
        delete pyFilter; // calls decref
}


// ****************************************************************************
//  Method: avtPythonFilterEnvironment::Initialize
//
//  Purpose:
//      Setups up the python environment.
//
//      This includes setting proper system paths to use vtk, pyavt & mpicom
//      python modules.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 24 16:17:23 MST 2010
//    Retrieve VISITARCHHOME via GetVisItArchitectureDirectory.
//    Remove slash from end of paths passed to AddSystemPath.
//
// ****************************************************************************

bool
avtPythonFilterEnvironment::Initialize()
{
    // init the interpreter
    if(!pyi->Initialize())
        return false;
    // setup pyavt env:
    // add system paths: $VISITARCHOME/lib & $VISITARCHOME/lib/site-packages
    string varchdir = GetVisItArchitectureDirectory();
    string vlibdir = varchdir + VISIT_SLASH_CHAR + "lib";
    string vlibsp  = vlibdir  + VISIT_SLASH_CHAR + "site-packages";

    if(!pyi->AddSystemPath(vlibdir))
        return false;
    if(!pyi->AddSystemPath(vlibsp)) // vtk module is symlinked here
        return false;

    // import pyavt and vtk
    if(!pyi->RunScript("from pyavt.filters import *\n"))
        return false;
    if(!pyi->RunScript("import vtk\n"))
        return false;

#ifdef PARALLEL
    // init mpicom w/ visit's communicator
    if(!pyi->RunScript("import mpicom\n"))
        return false;
    ostringstream oss;
    oss << (void*)VISIT_MPI_COMM_PTR;
    string comm_addy = oss.str();
    if(!pyi->RunScript("mpicom.init(caddy='" + comm_addy + "')\n"))
        return false;
#else
    // import pyavt.mpistub as mpicom
    if(!pyi->RunScript("import mpistub as mpicom\n"))
        return false;
#endif
    return true;
}


// ****************************************************************************
//  Method: avtPythonFilterEnvironment::LoadFilter
//
//  Purpose:
//      Executes given python script and attempt to load a python filter.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
bool
avtPythonFilterEnvironment::LoadFilter(const string &py_script)
{
    // check if the filter was already initialized
    if(pyFilter != NULL)
    {
        debug5 << "avtPythonFilterEnvironment::LoadFilter Error - "
               << "filter already loaded." << endl;
        return false;
    }

    // py_filter in global name space should define filter class
    if(!pyi->RunScript("py_filter = None\n"))
        return false;
    if(!pyi->RunScript(py_script))
        return false;
    PyObject *py_class = pyi->GetGlobalObject("py_filter");
    // TODO borrowed?

    // check if fclass Py None
    if( py_class == NULL || py_class == Py_None)
    {
        debug5 << "avtPythonFilterEnvironment::LoadFilter Error - "
               << "py_filter is 'None'" << endl;
        return false;
    }

    // check if fclass is callable (to create a filter instance)
    if(PyCallable_Check(py_class) == 0)
    {
        debug5 << "avtPythonFilterEnvironment::LoadFilter Error - "
               << "py_filter is not callable." << endl;
        return false;
    }

    // check if fclass is subclass of pyavt.filters.PythonFilter?

    // call fclass and create filter instance
    PyObject *py_obj= PyObject_CallObject(py_class, NULL);
    if(py_obj == NULL)
    {
        debug5 << "avtPythonFilterEnvironment::LoadFilter Error - "
               << "could not create instance of py_filter." << endl;
        return false;
    }


    pyFilter = new avtPythonFilter(py_obj);

    // return true b/c filter creation seems ok.
    return true;
}


// ****************************************************************************
//  Method: avtPythonFilterEnvironment::WrapVTKObject
//
//  Purpose:
//      Create a vtk python object of given type wrapping given address.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
//  Modifications:
//    Kathleen Bonnell, Wed Mar 24 17:58:11 MST 2010
//    Check for existence of '0x' at beginning of obj address before attempting
//    to remove it.
//
// ****************************************************************************
PyObject *
avtPythonFilterEnvironment::WrapVTKObject(void *obj,
                                          const string &obj_type)
{
    ostringstream oss;
    string addy_str;
    // vtk constructor needs a string of the objects address.
    oss << (void *) obj;
    // remove 0x from front of string

    if (oss.str().substr(0, 2) == "0x")   
        addy_str = oss.str().substr(2);
    else 
        addy_str = oss.str();

    if(!pyi->RunScript("_vtkobj = vtk." + obj_type + "('" + addy_str + "')\n"))
        return NULL;
    PyObject *res=pyi->GetGlobalObject("_vtkobj");
    if(res == NULL)
        return NULL;
    Py_INCREF(res);
    if(!pyi->RunScript("del _vtkobj"))
        return NULL;
    return res;
}

// ****************************************************************************
//  Method: avtPythonFilterEnvironment::UnwrapVTKObject
//
//  Purpose:
//      Unwrap a vtk python object of given type and return C++ address. 
//
//  Programmer:   Cyrus Harrison
//  Creation:     Tue Feb  2 13:14:44 PST 2010
//
// ****************************************************************************
void *
avtPythonFilterEnvironment::UnwrapVTKObject(PyObject *obj,
                                           const string &obj_type)
{
    if(!pyi->SetGlobalObject(obj,"_vtkobj"))
        return NULL;
    if(!pyi->RunScript("_vtkaddy = _vtkobj.GetAddressAsString('"
                   + obj_type + "')\n"))
        return NULL;
    if(!pyi->RunScript("_vtkaddy = int(_vtkaddy[5:],16)\n"))
        return NULL;

    PyObject *py_addy_int = pyi->GetGlobalObject("_vtkaddy");
    if(py_addy_int == NULL || ! PyInt_Check(py_addy_int))
        return NULL;
    long addy = PyInt_AsLong(py_addy_int);
    if(!pyi->RunScript("del _vtkaddy\n"))
        return NULL;
    // dec the extra ref we created.
    if(!pyi->RunScript("del _vtkobj\n"))
        return NULL;

    return (void*) addy;
}



// ****************************************************************************
//  Method: avtPythonScriptExpression::FetchPythonError
//
//  Purpose:
//      If an error occoured returns true & provides the error message string.
//
//  Programmer:   Cyrus Harrison
//  Creation:    Tue Feb  2 14:09:13 PST 2010
//
// ****************************************************************************
bool
avtPythonFilterEnvironment::FetchPythonError(string &msg_out)
{
    bool res= false;
    if(pyi->CheckError())
    {
        res = true;
        msg_out = pyi->ErrorMessage();
        debug5 << "avtPythonFilterEnvironment::Python Error - "
               << msg_out << endl;
        pyi->ClearError();
    }
    return res;
}


// ****************************************************************************
//  Method: avtPythonScriptExpression::Pickle
//
//  Purpose:
//      Pickles a python object to a string.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Fri Jul  9 13:54:40 PDT 2010
//
// ****************************************************************************

std::string
avtPythonFilterEnvironment::Pickle(PyObject *py_obj)
{
    if(!pickleReady)
        PickleInit();

    PyObject *res_obj = PyObject_CallFunctionObjArgs(pickleDumps,py_obj,NULL);
    if(res_obj == NULL)
    {
        debug5 << "avtPythonFilterEnvironment::Pickle Error - "
               << "could not pickle object." << endl;
        return std::string("");
    }

    char *res_cstr = PyString_AS_STRING(res_obj);
    std::string res(res_cstr);
    Py_DECREF(res_obj);
    return res;
}

// ****************************************************************************
//  Method: avtPythonScriptExpression::Unpickle
//
//  Purpose:
//      Unpickles a python object from a string.
//
//  Programmer:   Cyrus Harrison
//  Creation:     Fri Jul  9 13:54:40 PDT 2010
//
// ****************************************************************************

PyObject *
avtPythonFilterEnvironment::Unpickle(const std::string &s)
{
    PyObject *res = NULL;
    if(!pickleReady)
        PickleInit();

    PyObject *py_str_obj = PyString_FromString(s.c_str());
    res = PyObject_CallFunctionObjArgs(pickleLoads,py_str_obj,NULL);

    if(res == NULL)
    {
        debug5 << "avtPythonFilterEnvironment::Pickle Error - "
               << "could not unpickle given string." << endl;
    }

    Py_DECREF(py_str_obj);
    return res;
}

// ****************************************************************************
//  Method: avtPythonScriptExpression::PickleInit
//
//  Purpose:
//      Loads the pickle module for use with Pickle() & Unpickle()
//
//  Programmer:   Cyrus Harrison
//  Creation:     Fri Jul  9 13:54:40 PDT 2010
//
// ****************************************************************************

void
avtPythonFilterEnvironment::PickleInit()
{
    if(!pickleReady)
    {
        PyObject *pickleModule = PyImport_ImportModule("pickle"); // new ref
        PyObject *pickleDict   = PyModule_GetDict(pickleModule);  // borrowed

        pickleDumps  = PyDict_GetItemString(pickleDict, "dumps"); // borrowed
        pickleLoads  = PyDict_GetItemString(pickleDict, "loads"); // borrowed
        Py_INCREF(pickleDumps); 
        Py_INCREF(pickleLoads);

        Py_DECREF(pickleModule);
        pickleReady = true;
    }
}
