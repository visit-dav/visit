#include "ViewerRemoteProcessChooser.h"


#include <HostProfileList.h>
#include <ViewerHostProfileSelectorNoWin.h>
#include <ViewerHostProfileSelectorWithWin.h>
#include <RemoteProxyBase.h>

#include <stdio.h>
#include <snprintf.h>

#include <DebugStream.h>

#include <string>
using std::string;

bool ViewerRemoteProcessChooser::nowin = false;
ViewerRemoteProcessChooser *ViewerRemoteProcessChooser::instance = NULL;

// ****************************************************************************
//  Constructor:  ViewerRemoteProcessChooser::ViewerRemoteProcessChooser
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Rework to use ViewerHostProfileSelector. 
//
// ****************************************************************************

ViewerRemoteProcessChooser::ViewerRemoteProcessChooser()
{
    if (nowin)
        selector = new ViewerHostProfileSelectorNoWin();
    else
        selector = new ViewerHostProfileSelectorWithWin();
}

// ****************************************************************************
//  Destructor:  ViewerRemoteProcessChooser::~ViewerRemoteProcessChooser
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003 
//    Rework to use ViewerHostProfileSelector. 
//
// ****************************************************************************

ViewerRemoteProcessChooser::~ViewerRemoteProcessChooser()
{
    if (selector)
        delete selector;
}


// ****************************************************************************
//  Method:  ViewerRemoteProcessChooserNoWin::SelectProfile
//
//  Purpose:
//    Calls the chosen style of host profile selector.
//
//  Arguments:
//    hostProfileList : all host profiles
//    hostName        : the host name
//    skipChooser     : true if no window should appear
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 26, 2003
//
//  Modifications:
// ****************************************************************************

bool
ViewerRemoteProcessChooser::SelectProfile(HostProfileList *hostProfileList,
                                          const string &hostName,
                                          bool skipChooser)
{
    // sets profile
    return selector->SelectProfile(hostProfileList, hostName, skipChooser);
}

// ****************************************************************************
//  Method:  ViewerRemoteProcessChooserNoWin::AddProfileArguments
//
//  Purpose:
//    Adds the appropriate arguments to a remote proxy.
//
//  Arguments:
//    proxy           : the remote process proxy
//    addParallelArgs : true if this process is going to launch itself
//                      in parallel, and false if the vcl has already
//                      created a parallel job and we just need to
//                      choose the parallel engine when needed
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 26, 2003
//
//  Modifications:
// ****************************************************************************

void
ViewerRemoteProcessChooser::AddProfileArguments(RemoteProxyBase *proxy,
                                                bool addParallelArgs)
{
    int  i;

    const HostProfile profile = selector->GetHostProfile();

    //
    // Set the user's login name.
    //
    proxy->SetRemoteUserName(profile.GetUserName());

    //
    // Add the parallel arguments.
    //
    if (profile.GetParallel())
    {
        char temp[10];
        if (!addParallelArgs)
        {
            proxy->AddArgument("-par");
        }

        SNPRINTF(temp, 10, "%d", profile.GetNumProcessors());
        if (addParallelArgs)
        {
            proxy->AddArgument("-np");
            proxy->AddArgument(temp);
        }
        proxy->SetNumProcessors(profile.GetNumProcessors());

        if (profile.GetNumNodesSet() &&
            profile.GetNumNodes() > 0)
        {
            SNPRINTF(temp, 10, "%d", profile.GetNumNodes());
            if (addParallelArgs)
            {
                proxy->AddArgument("-nn");
                proxy->AddArgument(temp);
            }
            proxy->SetNumNodes(profile.GetNumNodes());
        }

        if (profile.GetPartitionSet() &&
            profile.GetPartition().length() > 0)
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-p");
                proxy->AddArgument(profile.GetPartition());
            }
        }

        if (profile.GetBankSet() &&
            profile.GetBank().length() > 0)
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-b");
                proxy->AddArgument(profile.GetBank());
            }
        }

        if (profile.GetTimeLimitSet() &&
            profile.GetTimeLimit().length() > 0)
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-t");
                proxy->AddArgument(profile.GetTimeLimit());
            }
        }

        if (profile.GetLaunchMethodSet() &&
            profile.GetLaunchMethod().length() > 0)
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-l");
                proxy->AddArgument(profile.GetLaunchMethod());
            }
        }

        if (profile.GetLaunchArgsSet() &&
            profile.GetLaunchArgs().length() > 0)
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-la");
                proxy->AddArgument(profile.GetLaunchArgs());
            }
        }

#if 0 // disabling dynamic load balancing for now
        if (profile.GetForceStatic())
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-forcestatic");
            }
            proxy->SetLoadBalancing(0);
        }

        if (profile.GetForceDynamic())
        {
            if (addParallelArgs)
            {
                proxy->AddArgument("-forcedynamic");
            }
            proxy->SetLoadBalancing(1);
        }
#else
        // force all static until speed issues are resolved
        if (addParallelArgs)
        {
            proxy->AddArgument("-forcestatic");
        }
        proxy->SetLoadBalancing(0);
#endif
    }

    // Add the timeout argument
    char temp[10];
    SNPRINTF(temp, 10, "%d", profile.GetTimeout());
    proxy->AddArgument("-timeout");
    proxy->AddArgument(temp);

    //
    // Add any additional arguments specified in the profile
    //
    for (i = 0; i < profile.GetArguments().size(); ++i)
        proxy->AddArgument(profile.GetArguments()[i]);
}


// ****************************************************************************
//  Method:  ViewerRemoteProcessChooser::ClearCache
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
ViewerRemoteProcessChooser::ClearCache(const std::string &hostName)
{
    if (selector)
        selector->ClearCache(hostName);
}


// ****************************************************************************
//  Method:  ViewerRemoteProcessChooser::Instance
//
//  Purpose:
//    Gets (and creates on demand) the global instance of the dialog.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
// ****************************************************************************

ViewerRemoteProcessChooser *
ViewerRemoteProcessChooser::Instance()
{
    if (!instance)
        instance = new ViewerRemoteProcessChooser();
    return instance;
}


// ****************************************************************************
//  Method:  ViewerRemoteProcessChooser::SetNoWinMode
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
ViewerRemoteProcessChooser::SetNoWinMode(bool nw)
{
    nowin = nw;
}
