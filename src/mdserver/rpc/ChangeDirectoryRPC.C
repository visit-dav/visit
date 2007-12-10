/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <ChangeDirectoryRPC.h>
#include <ChangeDirectoryException.h>
#include <DebugStream.h>

using std::string;

// *******************************************************************
// Constructor: ChangeDirectoryRPC::ChangeDirectoryRPC
//
// Purpose: 
//   This is the constructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

ChangeDirectoryRPC::ChangeDirectoryRPC() : BlockingRPC("s",NULL)
{
}


// *******************************************************************
// Denstructor: ChangeDirectoryRPC::~ChangeDirectoryRPC
//
// Purpose: 
//   This is the denstructor.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

ChangeDirectoryRPC::~ChangeDirectoryRPC()
{
}


// *******************************************************************
// Method: ChangeDirectoryRPC::operator()
//
// Purpose: 
//   This is the RPC's invocation method.
//
// Arguments:
//   s : The directory we want to change to.
//
// Note:       
//   If the RPC returned an error, throw an exception.
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 18:19:50 PST 2000
//
// Modifications:
//    Brad Whitlock, Tue Aug 29 18:21:04 PST 2000
//    I added code to throw an exception if the reply had an error.
//
//    Jeremy Meredith, Fri Nov 17 16:30:50 PST 2000
//    Made output go to log file instead of cout or cerr.
//
// *******************************************************************

void
ChangeDirectoryRPC::operator()(const string &s)
{
    debug3 << "Executing ChangeDirectory RPC" 
           << "\n\t directory='" << s.c_str() << "'"
           << endl;

    SetDirectory(s);
    Execute();

    // If there was an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(ChangeDirectoryException, s);
    }
}


// *******************************************************************
// Method: ChangeDirectoryRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

void
ChangeDirectoryRPC::SelectAll()
{
    Select(0, (void*)&directory);
}


// *******************************************************************
// Method: ChangeDirectoryRPC::SetDirectory
//
// Purpose: 
//   Set the directory argument of the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//
// *******************************************************************

void
ChangeDirectoryRPC::SetDirectory(const string &s)
{
    directory = s;
}

// *******************************************************************
// Method: ChangeDirectoryRPC::GetDirectory
//
// Purpose: 
//   Get the directory argument for the RPC.
//
// Arguments:
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   Brad Whitlock, Wed Apr 17 10:47:19 PDT 2002
//   Made the function return a reference.
//
// *******************************************************************

const string &
ChangeDirectoryRPC::GetDirectory() const
{
    return directory;
}

// ****************************************************************************
// Method: ChangeDirectoryRPC::TypeName
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
// Programmer: Brad Whitlock
// Creation:   Fri Dec  7 11:05:47 PST 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
ChangeDirectoryRPC::TypeName() const
{
    return "ChangeDirectoryRPC";
}
