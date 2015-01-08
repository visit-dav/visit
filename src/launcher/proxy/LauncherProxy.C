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

#include <visit-config.h>
#include <LauncherProxy.h>
#include <RemoteProcess.h>
#include <Utility.h>

// ****************************************************************************
// Method: LauncherProxy::LauncherProxy
//
// Purpose: 
//   Constructor for the LauncherProxy class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:38:42 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Nov 21 13:25:27 PST 2007
//   Added a second write socket.
//
// ****************************************************************************

LauncherProxy::LauncherProxy() : RemoteProxyBase("-vcl")
{
    // Indicate that we want 2 write sockets from VCL.
    nWrite = 2;
    state = new LauncherState();
    methods = new LauncherMethods(state);
}

// ****************************************************************************
// Method: LauncherProxy::~LauncherProxy
//
// Purpose: 
//   Destructor for the LauncherProxy class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:39:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

LauncherProxy::~LauncherProxy()
{
    delete state;
    delete methods;
}

// ****************************************************************************
// Method: LauncherProxy::GetComponentName
//
// Purpose: 
//   Returns the name of the component.
//
// Returns:    The component name.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:58:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
LauncherProxy::GetComponentName() const
{
    return "component launcher";
}

// ****************************************************************************
// Method: LauncherProxy::Create
//
// Purpose: 
//   Create the engine.
//
// Arguments:
//   profile             : The profile to use when launching the engine.
//   connectCallback     : A callback function.
//   connectCallbackData : Data for the callback function.
//   createAsThoughLocal : Whether to create local arguments.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 15 11:21:21 PDT 2012
//
// Modifications:
//   
// ****************************************************************************

void
LauncherProxy::Create(const MachineProfile &profile, 
    ConnectCallback *connectCallback, void *connectCallbackData,
    bool createAsThoughLocal)
{
    RemoteProxyBase::Create(profile, 
        connectCallback, connectCallbackData, createAsThoughLocal);
    methods->SetRemoteProcess(component);
}

// ****************************************************************************
// Method: LauncherProxy::SetupComponentRPCs
//
// Purpose: 
//   Sets up the RPC's used by this proxy.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:40:45 PST 2003
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 10:07:06 PST 2004
//    I added connectSimRPC.
//
// ****************************************************************************

void
LauncherProxy::SetupComponentRPCs()
{
    state->SetupComponentRPCs(&xfer);
}

// ****************************************************************************
//  Method:  LauncherProxy::GetPortTunnelMap
//
//  Purpose:
//    Retrieve the SSH tunneling local-to-remote port map from the
//    remote process object.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 24, 2007
//
//  Modifications:
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3) 
//   
// ****************************************************************************

std::map<int,int>
LauncherProxy::GetPortTunnelMap()
{
    return component->GetPortTunnelMap();
}
