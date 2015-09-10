/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <RestoreSessionRPC.h>
#include <DebugStream.h>


// *******************************************************************
// Constructor: RestoreSessionRPC::RestoreSessionRPC
//
// Purpose: 
//   This is the constructor.
//
// Arguments:
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

RestoreSessionRPC::RestoreSessionRPC() : BlockingRPC("ss", &sf), sf()
{
}


// *******************************************************************
// Denstructor: RestoreSessionRPC::~RestoreSessionRPC
//
// Purpose: 
//   This is the destructor.
//
// Arguments:
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

RestoreSessionRPC::~RestoreSessionRPC()
{
}


// *******************************************************************
// Method: RestoreSessionRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   filename : Filename to load session from.
//   sessionFile : Load session file into this var.
//
// Note:       
//   If the RPC returned an error, throw an exception.
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

void
RestoreSessionRPC::operator()(const std::string &_filename, 
                              std::string &_sessionFile)
{
    if (DebugStream::Level3())
    {
        debug3  << "Executing RestoreSessionRPC RPC" 
                << "\n\t In Filename ='" << _filename.c_str() << "'"
                << endl;
    }

    filename    = _filename;

    Execute();

    _sessionFile = sf.sessionFile;
}


// *******************************************************************
// Method: RestoreSessionRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//
// *******************************************************************

void
RestoreSessionRPC::SelectAll()
{
    Select(0, (void*)&filename);
    //Select(1, (void*)&sessionFile);
}


// ****************************************************************************
// Method: RestoreSessionRPC::TypeName
//
// Purpose: 
//   Returns the name of the RPC.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

const std::string
RestoreSessionRPC::TypeName() const
{
    return "RestoreSessionRPC";
}

// ****************************************************************************
// Method: RestoreSessionRPC::RestoreSessionFile
//
// Purpose: 
//   Restore session file to file system.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: David Camp
// Creation:   Tue Jul  7 07:56:12 PDT 2015
//
// Modifications:
//   
// ****************************************************************************

void
RestoreSessionRPC::RestoreSessionFile()
{
    if (DebugStream::Level3())
    {
        debug3  << "Executing RestoreSessionFile" << endl;
    }

    ifstream sFile(filename.c_str());
    if (sFile.is_open())
    {
        sf.sessionFile.assign((std::istreambuf_iterator<char>(sFile)), std::istreambuf_iterator<char>());
        sFile.close();

        SendReply(&sf);
    }
    else
    {
        // Send error message with filename.
        std::string errMessage("Failed to load remote session file: ");
        errMessage += filename;

        debug1  << errMessage << endl;
        SendError(errMessage);
    }
}

// *******************************************************************
// Method: RestoreSessionRPC::SessionFile::SessionFile
//
// Purpose:
//   Constructor for the RestoreSessionRPC::SessionFile class.
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// *******************************************************************

RestoreSessionRPC::SessionFile::SessionFile() : AttributeSubject("s"),
    sessionFile()
{
}

// *******************************************************************
// Method: RestoreSessionRPC::SessionFile::~SessionFile
//
// Purpose:
//   Destructor for the RestoreSessionRPC::SessionFile class.
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// *******************************************************************

RestoreSessionRPC::SessionFile::~SessionFile()
{
}

// *******************************************************************
// Method: RestoreSessionRPC::SessionFile::SelectAll
//
// Purpose:
//   Selects all the attributes in the object.
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// *******************************************************************

void
RestoreSessionRPC::SessionFile::SelectAll()
{
    Select(0, (void *)&sessionFile);
}

// ****************************************************************************
// Method: RestoreSessionRPC::SessionFile::TypeName
//
// Purpose:
//   Returns the RPC sessionFile.
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: David Camp
// Creation:   Mon Aug  3 11:50:15 PDT 2015
//
// Modifications:
//
// ****************************************************************************

const std::string
RestoreSessionRPC::SessionFile::TypeName() const
{
    return "RestoreSessionRPC::SessionFile";
}

