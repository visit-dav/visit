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

#include <GetPluginErrorsRPCExecutor.h>
#include <MDServerConnection.h>
#include <GetPluginErrorsRPC.h>

// ****************************************************************************
// Method: GetPluginErrorsRPCExecutor::GetPluginErrorsRPCExecutor
//
// Purpose: 
//   Constructor for the GetPluginErrorsRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// ****************************************************************************

GetPluginErrorsRPCExecutor::GetPluginErrorsRPCExecutor(
    MDServerConnection *parent_, Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetPluginErrorsRPCExecutor::~GetPluginErrorsRPCExecutor
//
// Purpose: 
//   Destructor for the GetPluginErrorsRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//   
// ****************************************************************************

GetPluginErrorsRPCExecutor::~GetPluginErrorsRPCExecutor()
{
}

// ****************************************************************************
// Method: GetPluginErrorsRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to return
//   errors generated during plugin initialization.
//
// Arguments:
//   s : A pointer to the GetPluginErrorsRPC that caused this method to
//       be called.
//
// Programmer: Jeremy Meredith
// Creation:   February  7, 2005
//
// Modifications:
//
// ****************************************************************************

void
GetPluginErrorsRPCExecutor::Update(Subject *s)
{
    GetPluginErrorsRPC *rpc = (GetPluginErrorsRPC *)s;
    GetPluginErrorsRPC::PluginErrors err;
    err.errorString = parent->GetPluginErrors();
    rpc->SendReply(&err);
}
