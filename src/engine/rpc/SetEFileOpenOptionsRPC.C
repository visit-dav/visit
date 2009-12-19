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

#include <SetEFileOpenOptionsRPC.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: SetEFileOpenOptionsRPC::SetEFileOpenOptionsRPC
//
// Purpose: 
//   This is the constructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

SetEFileOpenOptionsRPC::SetEFileOpenOptionsRPC()
    : BlockingRPC("a",NULL)
{
}


// *******************************************************************
// Denstructor: SetEFileOpenOptionsRPC::~SetEFileOpenOptionsRPC
//
// Purpose: 
//   This is the denstructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

SetEFileOpenOptionsRPC::~SetEFileOpenOptionsRPC()
{
}


// *******************************************************************
// Method: SetEFileOpenOptionsRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   opts : These are the new options.
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

void
SetEFileOpenOptionsRPC::operator()(const FileOpenOptions &opts)
{
    debug3 << "Executing SetEFileOpenOptionsRPC\n";

    SetFileOpenOptions(opts);
    Execute();
}


// *******************************************************************
// Method: SetEFileOpenOptionsRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

void
SetEFileOpenOptionsRPC::SelectAll()
{
    Select(0, (void*)&fileOpenOptions);
}


// *******************************************************************
// Method: SetEFileOpenOptionsRPC::SetDirectory
//
// Purpose: 
//   Set the directory argument of the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

void
SetEFileOpenOptionsRPC::SetFileOpenOptions(const FileOpenOptions &opts)
{
    fileOpenOptions = opts;
}

// *******************************************************************
// Method: SetEFileOpenOptionsRPC::GetDirectory
//
// Purpose: 
//   Get the file options for the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// *******************************************************************

const FileOpenOptions &
SetEFileOpenOptionsRPC::GetFileOpenOptions() const
{
    return fileOpenOptions;
}

// ****************************************************************************
// Method: SetEFileOpenOptionsRPC::TypeName
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
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//   
// ****************************************************************************

const std::string
SetEFileOpenOptionsRPC::TypeName() const
{
    return "SetEFileOpenOptionsRPC";
}
