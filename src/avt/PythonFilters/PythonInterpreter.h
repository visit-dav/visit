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
