#include "ViewerEngineChooser.h"


#include <HostProfileList.h>
#include <EngineProxy.h>
#include <ViewerHostProfileSelectorNoWin.h>
#include <ViewerHostProfileSelectorWithWin.h>

#include <stdio.h>
#include <snprintf.h>

#include <DebugStream.h>

#include <string>
using std::string;

bool ViewerEngineChooser::nowin = false;
ViewerEngineChooser *ViewerEngineChooser::instance = NULL;

// ****************************************************************************
//  Constructor:  ViewerEngineChooser::ViewerEngineChooser
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Rework to use ViewerHostProfileSelector. 
//
// ****************************************************************************

ViewerEngineChooser::ViewerEngineChooser()
{
    if (nowin)
        selector = new ViewerHostProfileSelectorNoWin();
    else
        selector = new ViewerHostProfileSelectorWithWin();
}

// ****************************************************************************
//  Destructor:  ViewerEngineChooser::~ViewerEngineChooser
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Rework to use ViewerHostProfileSelector. 
//
// ****************************************************************************

ViewerEngineChooser::~ViewerEngineChooser()
{
    if (selector)
        delete selector;
}


// ****************************************************************************
//  Method:  ViewerEngineChooserNoWin::GetNewEngine
//
//  Purpose:
//    Launches an engine on a host with the extra given args.
//
//  Arguments:
//    hostName    : the host name
//    arguments   : the arguments
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Jeremy Meredith, Sat Aug 17 10:59:48 PDT 2002
//    Added support for nowin mode.
//
//    Brad Whitlock, Wed Nov 27 13:39:50 PST 2002
//    I added code to set certain parallel options into the EngineProxy
//    so that they can be queried easily later.
//
//    Jeremy Meredith, Wed Dec 18 17:18:05 PST 2002
//    Added skip option so that we completely avoid the chooser.
//    Also added code to temporarily block socket signals to the viewer.
//    Not doing so was breaking synchronization with the CLI through
//    a race condition to grab the signals.
//
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003  
//    Moved host-profile-selection code to ViewerHostProfileSelector. 
//    
//    Jeremy Meredith, Fri Jan 24 15:23:54 PST 2003
//    Added optional arguments to the parallel launcher program.
//
// ****************************************************************************

EngineProxy *
ViewerEngineChooser::GetNewEngine(HostProfileList *hostProfileList,
                             const string &hostName, bool skipChooser)
{
    int  i;

    // sets profile
    if (!selector->SelectProfile(hostProfileList, hostName, skipChooser))
    {
        return NULL;
    }

    const HostProfile profile = selector->GetHostProfile();

    EngineProxy *newEngine = new EngineProxy;

    //
    // Set the user's login name.
    //
    newEngine->SetRemoteUserName(profile.GetUserName().c_str());

    //
    // Add the parallel arguments.
    //
    if (profile.GetParallel())
    {
        char temp[10];
        SNPRINTF(temp, 10, "%d", profile.GetNumProcessors());
        newEngine->AddArgument("-np");
        newEngine->AddArgument(temp);
        newEngine->SetNumProcessors(profile.GetNumProcessors());

        if (profile.GetNumNodesSet() &&
            profile.GetNumNodes() > 0)
        {
            SNPRINTF(temp, 10, "%d", profile.GetNumNodes());
            newEngine->AddArgument("-nn");
            newEngine->AddArgument(temp);
            newEngine->SetNumNodes(profile.GetNumNodes());
        }

        if (profile.GetPartitionSet() &&
            profile.GetPartition().length() > 0)
        {
            newEngine->AddArgument("-p");
            newEngine->AddArgument(profile.GetPartition().c_str());
        }

        if (profile.GetBankSet() &&
            profile.GetBank().length() > 0)
        {
            newEngine->AddArgument("-b");
            newEngine->AddArgument(profile.GetBank().c_str());
        }

        if (profile.GetTimeLimitSet() &&
            profile.GetTimeLimit().length() > 0)
        {
            newEngine->AddArgument("-t");
            newEngine->AddArgument(profile.GetTimeLimit().c_str());
        }

        if (profile.GetLaunchMethodSet() &&
            profile.GetLaunchMethod().length() > 0)
        {
            newEngine->AddArgument("-l");
            newEngine->AddArgument(profile.GetLaunchMethod().c_str());
        }

        if (profile.GetLaunchArgsSet() &&
            profile.GetLaunchArgs().length() > 0)
        {
            newEngine->AddArgument("-la");
            newEngine->AddArgument(profile.GetLaunchArgs().c_str());
        }

#if 0 // disabling dynamic load balancing for now
        if (profile.GetForceStatic())
        {
            newEngine->AddArgument("-forcestatic");
            newEngine->SetLoadBalancing(0);
        }

        if (profile.GetForceDynamic())
        {
            newEngine->AddArgument("-forcedynamic");
            newEngine->SetLoadBalancing(1);
        }
#else
        // force all static until speed issues are resolved
        newEngine->AddArgument("-forcestatic");
        newEngine->SetLoadBalancing(0);
#endif
    }

    // Add the timeout argument
    char temp[10];
    SNPRINTF(temp, 10, "%d", profile.GetTimeout());
    newEngine->AddArgument("-timeout");
    newEngine->AddArgument(temp);

    //
    // Add any additional arguments specified in the profile
    //
    for (i = 0; i < profile.GetArguments().size(); ++i)
        newEngine->AddArgument(profile.GetArguments()[i].c_str());

    return newEngine;
}


// ****************************************************************************
//  Method:  ViewerEngineChooser::ClearCache
//
//  Purpose:
//    Erases previous settings for re-launching engines.
//    This will typically be called if the previous launch failed
//    or if the user closed the engine manually.
//
//  Arguments:
//    hostName   the host name of the remote engine
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Use selector.
//
// ****************************************************************************

void
ViewerEngineChooser::ClearCache(const std::string &hostName)
{
    if (selector)
        selector->ClearCache(hostName);
}


// ****************************************************************************
//  Method:  ViewerEngineChooser::Instance
//
//  Purpose:
//    Gets (and creates on demand) the global instance of the dialog.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
// ****************************************************************************

ViewerEngineChooser *
ViewerEngineChooser::Instance()
{
    if (!instance)
        instance = new ViewerEngineChooser();
    return instance;
}


// ****************************************************************************
//  Method:  ViewerEngineChooser::SetNoWinMode
//
//  Purpose:
//    Sets nowin mode.
//
//  Arguments:
//    nw         nowin mode
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 17, 2002
//
// ****************************************************************************

void
ViewerEngineChooser::SetNoWinMode(bool nw)
{
    nowin = nw;
}
