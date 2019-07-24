// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
