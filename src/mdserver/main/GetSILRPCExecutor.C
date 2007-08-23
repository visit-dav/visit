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
#include <SILAttributes.h>
#include <GetSILRPCExecutor.h>
#include <GetSILRPC.h>
#include <MDServerConnection.h>
#include <DatabaseException.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: GetSILRPCExecutor::GetSILRPCExecutor
//
// Purpose: 
//   Constructor for the GetSILRPCExecutor class.
//
// Arguments:
//   parent_ : A pointer to the object that created this RPC executor.
//   s       : A pointer to the rpc that invokes this executor.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// ****************************************************************************

GetSILRPCExecutor::GetSILRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetSILRPCExecutor::~GetSILRPCExecutor
//
// Purpose: 
//   Destructor for the GetSILRPCExecutor class.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// ****************************************************************************

GetSILRPCExecutor::~GetSILRPCExecutor()
{
}

// ****************************************************************************
// Method: GetSILRPCExecutor::Update
//
// Purpose: 
//   Performs the work of the RPC executor which, in this case, is to change
//   the return metadata for the specified file.
//
// Arguments:
//   s : A pointer to the GetSILRPC that caused this method to
//       be called.
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 7 11:07:00 PDT 2002
//   Added code to handle exceptions.
//
//   Brad Whitlock, Tue May 13 15:40:51 PST 2003
//   I added timeState.
//
//   Jeremy Meredith, Wed Aug 25 11:40:22 PDT 2004
//   Handle errors through exceptions instead of error codes.   This allows
//   real error messages to make it to the user.
//
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Added treatAllDBsAsTimeVarying
// ****************************************************************************

void
GetSILRPCExecutor::Update(Subject *s)
{
    GetSILRPC *rpc = (GetSILRPC *)s;

    debug2 << "GetSILRPCExecutor::Update - file="<<rpc->GetFile().c_str()<<"\n";
    debug2.flush();

    TRY
    {
        // Either send a successful reply or send an error.
        parent->ReadSIL(rpc->GetFile(), rpc->GetTimeState(),
	    rpc->GetTreatAllDBsAsTimeVarying());
#ifdef DEBUG
        debug2 << "SIL=" << endl;
        parent->GetCurrentSIL()->Print(debug2);
#endif
        rpc->SendReply(parent->GetCurrentSIL());
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
