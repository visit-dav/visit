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

#include <LauncherProxy.h>

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
//   
// ****************************************************************************

LauncherProxy::LauncherProxy() : RemoteProxyBase("-vcl")
{
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
    xfer.Add(&launchRPC);
    xfer.Add(&connectSimRPC);
}

// ****************************************************************************
// Method: LauncherProxy::LaunchProcess
//
// Purpose: 
//   This method tells the VisIt component launcher to launch a program
//   that connects directly back to the component that launched the launcher.
//
// Arguments:
//   programArgs : The program arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:41:13 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherProxy::LaunchProcess(const stringVector &programArgs)
{
    launchRPC(programArgs);
}

// ****************************************************************************
//  Method:  LauncherProxy::ConnectSimulation
//
//  Purpose:
//    Connect to a running simulation
//
//  Arguments:
//    programArgs : The program arguments.
//    simHost     : The host where the simulation is running
//    simPort     : The port where the simulation is listening
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

void
LauncherProxy::ConnectSimulation(const stringVector &programArgs,
                                 const std::string &simHost, int simPort,
                                 const std::string &simSecurityKey)
{
    connectSimRPC(programArgs, simHost, simPort, simSecurityKey);
}
