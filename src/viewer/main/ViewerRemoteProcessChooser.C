#include "ViewerRemoteProcessChooser.h"

#include <HostProfile.h>
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
//    Brad Whitlock, Thu Aug 5 09:25:20 PDT 2004
//    I made it return the profile.
//
// ****************************************************************************

bool
ViewerRemoteProcessChooser::SelectProfile(HostProfileList *hostProfileList,
    const string &hostName, bool skipChooser, HostProfile &profile)
{
    // sets profile
    bool retval = selector->SelectProfile(hostProfileList, hostName, skipChooser);

    // If a profile was selected, return it here.
    if(retval)
        profile = selector->GetHostProfile();

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
