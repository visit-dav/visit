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

#include <SetMFileOpenOptionsRPCExecutor.h>
#include <SetMFileOpenOptionsRPC.h>
#include <DebugStream.h>
#include <MDServerConnection.h>

// ****************************************************************************
// Method:
//  SetMFileOpenOptionsRPCExecutor::SetMFileOpenOptionsRPCExecutor
//
// Purpose: 
//   Constructor for the SetMFileOpenOptionsRPCExecutor class.
//
// Arguments:
//   p       : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

SetMFileOpenOptionsRPCExecutor::SetMFileOpenOptionsRPCExecutor(
    MDServerConnection *p, Subject *s) : Observer(s)
{
    parent = p;
}

// ****************************************************************************
// Method:
//  SetMFileOpenOptionsRPCExecutor::~SetMFileOpenOptionsRPCExecutor
//
// Purpose: 
//   Destructor for the SetMFileOpenOptionsRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

SetMFileOpenOptionsRPCExecutor::~SetMFileOpenOptionsRPCExecutor()
{
}

// ****************************************************************************
// Method: SetMFileOpenOptionsRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to change
//   the current directory.
//
// Arguments:
//   s : A pointer to the SetMFileOpenOptionsRPC that caused this method to
//       be called.
//
// Programmer: Jeremy Meredith
// Creation:   January 18, 2008
//
// Modifications:
//
// ****************************************************************************

void
SetMFileOpenOptionsRPCExecutor::Update(Subject *s)
{
    SetMFileOpenOptionsRPC *rpc = (SetMFileOpenOptionsRPC *)s;
#ifdef DEBUG
    debug2 << "SetMFileOpenOptionsRPCExecutor::Update" << endl;
#endif
    // Either send a successful reply or send an error.
    parent->SetDefaultFileOpenOptions(rpc->GetFileOpenOptions());
    rpc->SendReply();
}
