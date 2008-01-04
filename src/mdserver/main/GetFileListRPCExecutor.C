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
#include <GetFileListRPCExecutor.h>
#include <GetFileListRPC.h>
#include <MDServerConnection.h>
#include <TimingsManager.h>

// ****************************************************************************
// Method: GetFileListRPCExecutor::GetFileListRPCExecutor
//
// Purpose: 
//   Constructor for the GetFileListRPCExecutor class.
//
// Arguments:
//   parent : A pointer to the object that created this executor.
//   s      : A pointer to the RPC that will call this RPC executor.
//
// Returns:    
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

GetFileListRPCExecutor::GetFileListRPCExecutor(MDServerConnection *parent_,
    Subject *s) : Observer(s)
{
    parent = parent_;
}

// ****************************************************************************
// Method: GetFileListRPCExecutor::~GetFileListRPCExecutor
//
// Purpose: 
//   Destructor for the GetFileListRPCExecutor class.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   
// ****************************************************************************

GetFileListRPCExecutor::~GetFileListRPCExecutor()
{
}

// ****************************************************************************
// Method: GetFileListRPCExecutor::Update
//
// Purpose: 
//   Gets the current file list.
//
// Arguments:
//   s : A pointer to the GetFileListRPC that called this method.
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 25 11:00:46 PDT 2003
//   I added support for automatic file grouping.
//
//   Brad Whitlock, Thu Jul 29 12:20:50 PDT 2004
//   I added support for extra smart file grouping.
//
//   Brad Whitlock, Fri Feb 4 15:17:39 PST 2005
//   I changed how the file grouping settings are passed to the connection.
//
//   Brad Whitlock, Thu Dec 15 10:23:46 PDT 2005
//   I added a timer.
//
// ****************************************************************************

void
GetFileListRPCExecutor::Update(Subject *s)
{
    int total = visitTimer->StartTimer();
    GetFileListRPC *rpc = (GetFileListRPC *)s;

    debug2 << "GetFileListRPCExecutor::Update\n"; debug2.flush();

    // Either send a successful reply or send an error.
    if(parent->GetReadFileListReturnValue() == 0)
    {
        if(rpc->GetAutomaticFileGrouping())
        {
            // Get the filtered file list.
            GetFileListRPC::FileList files;
            parent->SetFileGroupingOptions(rpc->GetFilter(),
                rpc->GetSmartFileGrouping());
            parent->GetFilteredFileList(files);

            debug4 << "FILELIST=" << files << endl;
            debug4.flush();
            rpc->SendReply(&files);
        }
        else
        {
            GetFileListRPC::FileList *files = parent->GetCurrentFileList();
            debug4 << "FILELIST=" << files << endl;
            debug4.flush();

            rpc->SendReply(files);
        }
    }
    else
        rpc->SendError();

    visitTimer->StopTimer(total, "GetFileListRPC");
}

