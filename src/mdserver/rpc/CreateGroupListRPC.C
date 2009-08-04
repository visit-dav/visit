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

#include <CreateGroupListRPC.h>

// ****************************************************************************
// Method: CreateGroupListRPC::CreateGroupListRPC
//
// Purpose: 
//   Constructor for the CreateGroupListRPC class.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:46:58 PST 2001
//
// Modifications:
//   
// ****************************************************************************

CreateGroupListRPC::CreateGroupListRPC() : NonBlockingRPC("ss*")
{
}

// ****************************************************************************
// Method: CreateGroupListRPC::operator()
//
// Purpose: 
//   This is the invokation method for this function object. It copies the
//   arguments into local attributes and calls RPC's Execute method.
//
// Arguments:
//   f  : The filename of the group list to create
//   gl : The group list itself
//
// Returns:    
//
// Note:       
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:47:54 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
CreateGroupListRPC::operator()(const string f, vector<string> gl)
{
    filename = f;
    groupList = gl;

    Execute();
}

// ****************************************************************************
// Method: CreateGroupListRPC::SelectAll
//
// Purpose: 
//   Gets the addresses of the local attributes so the attributes can be
//   transmitted over the network as AttributeGroups.
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 15:22:24 PST 2001
//
// Modifications:
//   
// ****************************************************************************
void
CreateGroupListRPC::SelectAll()
{
    Select(0, (void *)&filename);
    Select(1, (void *)&groupList);
}

// ****************************************************************************
// Method: CreateGroupListRPC::TypeName
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
CreateGroupListRPC::TypeName() const
{
    return "CreateGroupListRPC";
}
