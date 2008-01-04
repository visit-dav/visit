/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
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

#include <QuitRPC.h>
#include <DebugStream.h>

// ****************************************************************************
//  Constructor: QuitRPC::QuitRPC
//
//  Purpose: 
//    This is the constructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Apr 24 10:26:25 PDT 2003
//    I made it be nonblocking.
//
// ****************************************************************************

QuitRPC::QuitRPC() : NonBlockingRPC("b",NULL)
{
    quit = false;
}

// ****************************************************************************
//  Destructor: QuitRPC::~QuitRPC
//
//  Purpose: 
//    This is the destructor.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

QuitRPC::~QuitRPC()
{
}

// ****************************************************************************
//  Method: QuitRPC::operator()
//
//  Purpose: 
//    This is the RPC's invocation method.
//
//  Arguments:
//    q         the quit state
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Nov 17 16:41:39 PST 2000
//    Made output go to log file instead of cout or cerr.
//
// ****************************************************************************

void
QuitRPC::operator()(bool q)
{
    debug3 << "Executing quit RPC" 
           << "\n\t quit='" << q << "'"
           << endl;

    SetQuit(q);
    Execute();
}

// ****************************************************************************
//  Method: QuitRPC::SelectAll
//
//  Purpose: 
//    This selects all variables for sending.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
QuitRPC::SelectAll()
{
    Select(0, (void*)&quit);
}

// ****************************************************************************
//  Method: QuitRPC::SetQuit
//
//  Purpose: 
//    This sets the quit state.
//
//  Arguments:
//    q         the quit state
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
QuitRPC::SetQuit(bool q)
{
    quit = q;
}

// ****************************************************************************
//  Method: QuitRPC::GetQuit
//
//  Purpose: 
//    This returns the quit state.
//
//  Arguments:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 17, 2000
//
// ****************************************************************************

bool
QuitRPC::GetQuit() const
{
    return quit;
}

// ****************************************************************************
// Method: QuitRPC::TypeName
//
// Purpose: 
//   Returns the RPC name.
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec  7 11:09:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
QuitRPC::TypeName() const
{
    return "QuitRPC";
}
