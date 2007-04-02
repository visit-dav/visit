/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#include <GetSILRPC.h>
#include <GetMetaDataException.h>
#include <DebugStream.h>
#include <string>
using std::string;

// ****************************************************************************
// Method: GetSILRPC::GetSILRPC
//
// Purpose: 
//   Constructor for the GetSILRPC class.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//
// ****************************************************************************

GetSILRPC::GetSILRPC() : BlockingRPC("si",&sil)
{
    timeState = 0;
}

// ****************************************************************************
// Method: GetSILRPC::~GetSILRPC
//
// Purpose: 
//   Destructor for the GetSILRPC class.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//
// ****************************************************************************

GetSILRPC::~GetSILRPC()
{
}

// ****************************************************************************
// Method: GetSILRPC::TypeName
//
// Purpose: 
//   Returns the RPC name,
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:33:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const string
GetSILRPC::TypeName() const
{
    return "GetSILRPC";
}

// ****************************************************************************
//  Method: GetSILRPC::operator()
//
//  Purpose: 
//    This is the () operator for the GetSILRPC class. This method
//    makes the objects of this class function objects. This method
//    executes the RPC to get the sil from a file and returns
//    a pointer to it.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Feb 7 11:12:11 PDT 2002
//    Added the error message to the new exception.
//
//    Brad Whitlock, Tue May 13 15:33:32 PST 2003
//    Added timeState.
//
// ****************************************************************************

const SILAttributes *
GetSILRPC::operator()(const string &f, int ts)
{
    debug3 << "Executing GetSIL RPC on file " << f.c_str()
           << ", timestate = " << ts << endl;

    SetFile(f);
    SetTimeState(ts);

    // Try to execute the RPC.
    Execute();

    // If the RPC returned an error, throw an exception.
    if(GetReply()->GetStatus() == error)
    {
        EXCEPTION1(GetMetaDataException, GetReply()->Message());
    }

    return &sil;
}

// ****************************************************************************
// Method: GetSILRPC::SelectAll
//
// Purpose: 
//   This method selects all of the components in the GetSILRPC
//   before the RPC is executed. This is so RPC function parameters
//   would be communicated.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   Brad Whitlock, Tue May 13 15:34:13 PST 2003
//   Added timeState.
//
// ****************************************************************************

void
GetSILRPC::SelectAll()
{
    Select(0, (void*)&file);
    Select(1, (void*)&timeState);
}

// ****************************************************************************
// Method: GetSILRPC::SetFile
//
// Purpose: 
//   This sets the file name from which to get the SIL.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   
// ****************************************************************************

void
GetSILRPC::SetFile(const std::string &f)
{
    file = f;
    Select(0, (void*)&file);
}

// ****************************************************************************
// Method: GetSILRPC::SetTimeState
//
// Purpose: 
//   This sets the time state for which to get the SIL.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   
// ****************************************************************************

void
GetSILRPC::SetTimeState(int ts)
{
    timeState = ts;
    Select(1, (void*)&timeState);
}

// ****************************************************************************
// Method: GetSILRPC::GetFile
//
// Purpose: 
//   This gets the file name from which to get the metadata.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   
// ****************************************************************************

string
GetSILRPC::GetFile() const
{
    return file;
}

// ****************************************************************************
// Method: GetSILRPC::GetTimeState
//
// Purpose: 
//   Gets the timestate.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 13 15:35:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

int
GetSILRPC::GetTimeState() const
{
    return timeState;
}
