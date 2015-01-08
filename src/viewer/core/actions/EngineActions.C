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
#include <EngineActions.h>

#include <ViewerEngineManagerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerProperties.h>
#include <ViewerState.h>
#include <ViewerText.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <ProcessAttributes.h>
#include <VisItInit.h>

#ifdef _WIN32
#include <process.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#endif

//
// These methods were adapted from ViewerSubject handlers.
//

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearCacheForAllEnginesAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ClearCacheForAllEnginesRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ClearCacheForAllEnginesAction::Execute()
{
    GetViewerEngineManager()->ClearCacheForAllEngines();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ClearCacheAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::ClearCacheRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ClearCacheAction::Execute()
{
    const std::string &hostName = args.GetProgramHost();
    const std::string &simName  = args.GetProgramSim();
    GetViewerEngineManager()->ClearCache(EngineKey(hostName, simName), std::string());
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CloseComputeEngineAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::CloseComputeEngineRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
CloseComputeEngineAction::Execute()
{
    //
    // We're closing the engine so reset all of the network ids for plots that
    // are on the specified engine. This ensures that pick, etc works when
    // we use a new compute engine.
    //
    EngineKey key(args.GetProgramHost(), args.GetProgramSim());
    windowMgr->ResetNetworkIds(key);
    GetViewerEngineManager()->CloseEngine(key);
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: GetProcInfoAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::GetProcInfoRPC
//
// Programmer: Mark Miller
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
GetProcInfoAction::Execute()
{
    ProcessAttributes tmpAtts;

    std::string componentName = VisItInit::ComponentIDToName(args.GetIntArg1());
    if (componentName == "engine")
    {
        const std::string &hostName = args.GetProgramHost();
        const std::string &simName  = args.GetProgramSim();

        GetViewerEngineManager()->GetProcInfo(EngineKey(hostName, simName), tmpAtts);
    }
    else if (componentName == "viewer")
    {
#if defined(_WIN32)
        int pid = _getpid();
        int ppid = -1;
#else
        int pid = getpid();
        int ppid = getppid();
#endif
        char myHost[256];
        gethostname(myHost, sizeof(myHost));

        std::vector<int> tmpPids;
        tmpPids.push_back(pid);

        std::vector<int> tmpPpids;
        tmpPpids.push_back(ppid);

        std::vector<std::string> tmpHosts;
        tmpHosts.push_back(myHost);

        tmpAtts.SetPids(tmpPids);
        tmpAtts.SetPpids(tmpPpids);
        tmpAtts.SetHosts(tmpHosts);
        tmpAtts.SetIsParallel(false); // would be better to check for threads
    }
    else
    {
        GetViewerMessaging()->Warning(
            TR("Currently, GetProcessAttributes() works only for "
               "\"engine\" or \"viewer\""));
        return;
    }

    *GetViewerState()->GetProcessAttributes() = tmpAtts;
    GetViewerState()->GetProcessAttributes()->SelectAll();
    GetViewerState()->GetProcessAttributes()->Notify();
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: OpenComputeEngineAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::OpenComputeEngineRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
OpenComputeEngineAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    const std::string  &hostName = args.GetProgramHost();
    const stringVector &options  = args.GetProgramOptions();

    //
    // Perform the rpc.
    //
    bool givenOptions = (options.size() > 0);
    bool givenCLArgs  = (!GetViewerProperties()->GetEngineParallelArguments().empty());

    EngineKey key(hostName, "");
    if(GetViewerEngineManager()->EngineExists(key))
    {
        GetViewerMessaging()->Warning(
            TR("VisIt did not open a new compute engine on host %1 "
               "because a compute engine is already running there.").
            arg(hostName));
    }
    else if (givenOptions)
    {
        GetViewerEngineManager()->CreateEngine(
            key,
            options,
            true,
            GetViewerProperties()->GetNumEngineRestarts());
    }
    else if (GetViewerProperties()->GetNowin() && givenCLArgs)
    {
        GetViewerEngineManager()->CreateEngine(
            key,
            GetViewerProperties()->GetEngineParallelArguments(),
            true,
            GetViewerProperties()->GetNumEngineRestarts());
    }
    else
    {
        GetViewerEngineManager()->CreateEngine(
            key,
            options,
            false,
            GetViewerProperties()->GetNumEngineRestarts());
    }
}

///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SendSimulationCommandAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SendSimulationCommandRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SendSimulationCommandAction::Execute()
{
    //
    // Get the rpc arguments.
    //
    const std::string &hostName = args.GetProgramHost();
    const std::string &simName  = args.GetProgramSim();
    const std::string &command  = args.GetStringArg1();
    const std::string &argument = args.GetStringArg2();

    //
    // Perform the RPC.
    //
    GetViewerEngineManager()->SendSimulationCommand(
        EngineKey(hostName, simName), command, argument);
}
