// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerHostProfileSelectorNoWin.h>

#include <HostProfileList.h>
#include <MachineProfile.h>

#include <stdio.h>

#include <DebugStream.h>

#include <string>
using std::string;


// ****************************************************************************
//  Constructor:  ViewerHostProfileSelectorNoWin::ViewerHostProfileSelectorNoWin
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    Februrary 5, 2003 
//
//  Modifications:
//
// ****************************************************************************

ViewerHostProfileSelectorNoWin::ViewerHostProfileSelectorNoWin()
{
}

// ****************************************************************************
//  Destructor:  ViewerHostProfileSelectorNoWin::~ViewerHostProfileSelectorNoWin
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
// ****************************************************************************

ViewerHostProfileSelectorNoWin::~ViewerHostProfileSelectorNoWin()
{
}

// ****************************************************************************
//  Method:  ViewerHostProfileSelectorNoWin::SelectProfile
//
//  Purpose:
//    Selects a host profile based on the host profile list and the hostName.
//
//  Arguments:
//    hostProfileList : a list of possible profiles
//    hostName        : the host name
//    skipChooser     : if true, a default host profile is returned. 
//
//  Returns:  true if a host profile could be selected, false otherwise.
//
//  Notes:
//    Extracted from ViewerEngineChooser::GetNewEngine by Jeremy Meredith.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  2 14:30:11 PST 2004
//    Added caching of the empty profile when we skip the chooser.  The reason
//    this is useful is because I added a method to save the manual arguments
//    with a cached profile, but this needed to work even if we skipped the
//    chooser.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//    Simplified this function a bit.
//
// ****************************************************************************

bool
ViewerHostProfileSelectorNoWin::SelectProfile(
    HostProfileList *hostProfileList, const string &hostName, bool skipChooser)
{
    profile = MachineProfile();

    if (skipChooser)
    {
        // do nothing; leave the profile completely blank
        // Save it so we can cache the extra arguments, though
        cachedProfile[hostName] = profile;
    }
    else if (cachedProfile.count(hostName))
    {
        profile = cachedProfile[hostName];
    }
    else
    {
        MachineProfile *mp = hostProfileList->GetMachineProfileForHost(hostName);

        if (mp)
        {
            profile = *mp;
        }

        // Save it for use later
        cachedProfile[hostName] = profile;
    }
    return true;
}

