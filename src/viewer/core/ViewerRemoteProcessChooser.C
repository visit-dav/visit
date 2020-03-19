// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "ViewerRemoteProcessChooser.h"

#include <MachineProfile.h>
#include <HostProfileList.h>

#include <ViewerFactory.h>
#include <ViewerHostProfileSelector.h>
//#include <RemoteProxyBase.h>

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
//    Brad Whitlock, Tue Apr 14 11:42:27 PDT 2009
//    Inherit from ViewerBase.
//
// ****************************************************************************

ViewerRemoteProcessChooser::ViewerRemoteProcessChooser() : ViewerBase()
{
    selector = GetViewerFactory()->CreateHostProfileSelector();
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
//    Brad Whitlock, Thu Aug 5 09:25:20 PDT 2004
//    I made it return the profile.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
// ****************************************************************************

bool
ViewerRemoteProcessChooser::SelectProfile(HostProfileList *hostProfileList,
    const std::string &hostName, bool skipChooser, MachineProfile &profile)
{
    // sets profile
    bool retval = selector->SelectProfile(hostProfileList, hostName, skipChooser);

    // If a profile was selected, return it here.
    if(retval)
        profile = selector->GetMachineProfile();

    return retval;
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
//  Method:  ViewerRemoteProcessChooser::AddRestartArgsToCachedProfile
//
//  Purpose:
//    Modify a cached profile to include the given arguments.
//
//  Arguments:
//    hostName   the host name of the remote engine
//    args       the arguments to save
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  2, 2004
//
// ****************************************************************************
void
ViewerRemoteProcessChooser::AddRestartArgsToCachedProfile(
                                          const std::string &hostName,
                                          const std::vector<std::string> &args)
{
    if (selector)
        selector->AddRestartArgsToCachedProfile(hostName, args);
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
