// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
