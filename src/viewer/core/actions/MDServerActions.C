// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <MDServerActions.h>
#include <ViewerFileServerInterface.h>

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: OpenMDServerAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::OpenMDServerRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
OpenMDServerAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    const std::string &hostName = args.GetProgramHost();
    const stringVector &options = args.GetProgramOptions();
    GetViewerFileServer()->NoFaultStartServer(hostName, options);
}
