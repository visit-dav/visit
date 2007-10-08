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

#include <avtDatabase.h>
#include <DBPluginInfoAttributes.h>
#include <GetDBPluginInfoRPCExecutor.h>
#include <GetDBPluginInfoRPC.h>
#include <MDServerConnection.h>
#include <DatabaseException.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: GetDBPluginInfoRPCExecutor::GetDBPluginInfoRPCExecutor
//
// Purpose: 
//   Constructor for the GetDBPluginInfoRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// ****************************************************************************

GetDBPluginInfoRPCExecutor::GetDBPluginInfoRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetDBPluginInfoRPCExecutor::~GetDBPluginInfoRPCExecutor
//
// Purpose: 
//   Destructor for the GetDBPluginInfoRPCExecutor class.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// ****************************************************************************

GetDBPluginInfoRPCExecutor::~GetDBPluginInfoRPCExecutor()
{
}

// ****************************************************************************
// Method: GetDBPluginInfoRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to get the
//   DB options for a plugin type.
//
// Arguments:
//   s : A pointer to the GetDBPluginInfoRPC that caused this method to
//       be called.
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// Modifications:
//   Mark C. Miller, Mon Oct  8 13:44:36 PDT 2007
//   Put resulting atts from GetDBPluginInfo into local var so we could
//   properly delete it -- fixed leak.
//
// ****************************************************************************

void
GetDBPluginInfoRPCExecutor::Update(Subject *s)
{
    GetDBPluginInfoRPC *rpc = (GetDBPluginInfoRPC *)s;

    debug2 << "GetDBPluginInfoRPCExecutor::Update\n";
    debug2.flush();

    TRY
    {
        // Either send a successful reply or send an error.
	DBPluginInfoAttributes *atts = parent->GetDBPluginInfo();
        rpc->SendReply(atts);
	delete atts;
    }
    CATCH2(DatabaseException, dbe)
    {
        rpc->SendError(dbe.Message(), dbe.GetExceptionType());
    }
    CATCH2(VisItException, ve)
    {
        rpc->SendError("An unknown error has occurred", ve.GetExceptionType());
    }
    ENDTRY
}
