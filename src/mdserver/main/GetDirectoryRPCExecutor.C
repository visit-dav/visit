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

#include <DebugStream.h>
#include <GetDirectoryRPCExecutor.h>
#include <GetDirectoryRPC.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method: GetDirectoryRPCExecutor::GetDirectoryRPCExecutor
//
// Purpose:
//   Constructor for the GetDirectoryRPCExecutor class.
//
// Arguments:
//   parent_ : The object that created this RPC executor.
//   s       : A pointer to the RPC that will use this RPC executor.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GetDirectoryRPCExecutor::GetDirectoryRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetDirectoryRPCExecutor::~GetDirectoryRPCExecutor
//
// Purpose:
//   Destructor for the GetDirectoryRPCExecutor class.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

GetDirectoryRPCExecutor::~GetDirectoryRPCExecutor()
{
}

// ****************************************************************************
// Method: GetDirectoryRPCExecutor::Update
//
// Purpose:
//   This method executes the RPC's work.
//
// Arguments:
//   s : A pointer to the RPC that initiated the call to this method.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
GetDirectoryRPCExecutor::Update(Subject *s)
{
     GetDirectoryRPC *rpc = (GetDirectoryRPC *)s;
#ifdef DEBUG
     debug2 << "GetDirectoryRPCExecutor::Update\n"; debug2.flush();
#endif
     GetDirectoryRPC::DirectoryName dir;
     dir.name = parent->GetCurrentWorkingDirectory();
     rpc->SendReply(&dir);
}
