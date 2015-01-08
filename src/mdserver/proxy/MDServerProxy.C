/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ************************************************************************* //
//                              MDServerProxy.C                              //
// ************************************************************************* //
#include "MDServerProxy.h"

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <visit-config.h>

#include <MDServerState.h>
#include <MDServerMethods.h>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: MDServerProxy constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Nov 16 16:21:15 PST 2000
//    I added connectRPC and the launch argument.
//
//    Brad Whitlock, Mon Nov 27 17:43:45 PST 2000
//    Added initialization of argc, argv.
//
//    Sean Ahern, Wed Feb 28 14:34:51 PST 2001
//    Added the CreateGroupListRPC.
//
//    Brad Whitlock, Tue Feb 12 13:58:14 PST 2002
//    Added ExpandPathRPC.
//
//    Brad Whitlock, Mon Feb 25 15:31:25 PST 2002
//    Initialized server and xfer pointers in case Create is not a success
//    before the destructor is called.
//
//    Brad Whitlock, Fri May 3 17:25:27 PST 2002
//    Added remoteUserName.
//
//    Brad Whitlock, Tue Jul 30 10:53:59 PDT 2002
//    Added closeDatabaseRPC.
//
//    Brad Whitlock, Mon Aug 26 15:54:57 PST 2002
//    Added separator and separatorString.
//
//    Brad Whitlock, Fri Sep 27 15:35:37 PST 2002
//    Added launch progress callbacks.
//
//    Brad Whitlock, Fri May 2 15:09:04 PST 2003
//    I made it inherit from the new RemoteProxyBase class.
//
//    Hank Childs, Mon May 23 13:40:35 PDT 2005
//    Added constructors for new RPC.
//
// ****************************************************************************

MDServerProxy::MDServerProxy() : RemoteProxyBase("-mdserver")
{
    state = new MDServerState();
    methods = new MDServerMethods(state);
}

// ****************************************************************************
//  Method: MDServerProxy destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************

MDServerProxy::~MDServerProxy()
{
    delete state;
    delete methods;
}

// ****************************************************************************
// Method: MDServerProxy::GetComponentName
//
// Purpose:
//   Returns the name of the component.
//
// Returns:    The name of the component.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:17:54 PDT 2003
//
// Modifications:
//
// ****************************************************************************

std::string
MDServerProxy::GetComponentName() const
{
    return "metadata server";
}

// ****************************************************************************
// Method: MDServerProxy::SetupComponentRPCs
//
// Purpose:
//   This method connects the MDServer RPC's to the xfer object. It's called
//   from RemoteProxyBase::Create.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:10:40 PST 2003
//
// Modifications:
//
//   Hank Childs, Thu Jan 22 21:02:56 PST 2004
//   Added loadPluginsRPC.
//
//   Jeremy Meredith, Tue Feb  8 08:53:15 PST 2005
//   Added the ability to query for errors detected during plugin
//   initialization.
//
//   Hank Childs, Mon May 23 13:40:35 PDT 2005
//   Added new RPC.
//
// ****************************************************************************

void
MDServerProxy::SetupComponentRPCs()
{
    state->SetupComponentRPCs(&xfer);
    state->DetermineSeparator();
}
