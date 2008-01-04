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

#include <ExpandPathRPC.h>
#include <DebugStream.h>
#include <VisItException.h>

// ****************************************************************************
// Constructor: ExpandPathRPC::ExpandPathRPC
//
// Purpose: 
//   This is the constructor.
//
// Programmer: Brad Whitlock
// Creation:   August 29, 2000
//
// Modifications:
//
// ****************************************************************************

ExpandPathRPC::ExpandPathRPC() : BlockingRPC("a", &path)
{
}


// ****************************************************************************
// Denstructor: ExpandPathRPC::~ExpandPathRPC
//
// Purpose: 
//   This is the denstructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:42:29 PST 2002
//
// Modifications:
//
// ****************************************************************************

ExpandPathRPC::~ExpandPathRPC()
{
}


// ****************************************************************************
// Method: ExpandPathRPC::operator()
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
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:42:29 PST 2002
//
// Modifications:
//
// ****************************************************************************

std::string
ExpandPathRPC::operator()(const std::string &s)
{
    debug3 << "Executing ExpandPath RPC" 
           << "\n\t directory='" << s.c_str() << "'"
           << endl;

    SetPath(s);
    Execute();
    return path.name;
}


// ****************************************************************************
// Method: ExpandPathRPC::SelectAll
//
// Purpose: 
//   Select all attributes for sending as parameters.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 13:42:29 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
ExpandPathRPC::SelectAll()
{
    Select(0, (void*)&path);
}

// ****************************************************************************
// Method: ExpandPathRPC::SetPath
//
// Purpose: 
//   Sets the path that we're going to expand.
//
// Arguments:
//   p : The path
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 15:37:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ExpandPathRPC::SetPath(const std::string &p)
{
    path.name = p;
    path.SelectAll();
    SelectAll();
}

// ****************************************************************************
// Method: ExpandPathRPC::TypeName
//
// Purpose: 
//   Returns the RPC name.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec  7 11:09:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
ExpandPathRPC::TypeName() const
{
    return "ExpandPathRPC";
}

// ****************************************************************************
// Method: ExpandPathRPC::PathName::PathName
//
// Purpose: 
//   Constructor for the ExpandPathRPC::PathName class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:44:30 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ExpandPathRPC::PathName::PathName() : AttributeSubject("s"),
    name()
{
}

// ****************************************************************************
// Method: ExpandPathRPC::PathName::~PathName
//
// Purpose: 
//   Destructor for the ExpandPathRPC::PathName class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:44:30 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ExpandPathRPC::PathName::~PathName()
{
}

// ****************************************************************************
// Method: ExpandPathRPC::PathName::SelectAll
//
// Purpose: 
//   Selects all the attributes in the object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:44:30 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ExpandPathRPC::PathName::SelectAll()
{
    Select(0, (void *)&name);
}

// ****************************************************************************
// Method: ExpandPathRPC::PathName::TypeName
//
// Purpose: 
//   Returns the RPC name.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec  7 11:09:23 PST 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
ExpandPathRPC::PathName::TypeName() const
{
    return "ExpandPathRPC::PathName";
}
