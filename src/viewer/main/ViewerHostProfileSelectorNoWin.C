#include <ViewerHostProfileSelectorNoWin.h>

#include <HostProfileList.h>

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
// ****************************************************************************

bool
ViewerHostProfileSelectorNoWin::SelectProfile(
    HostProfileList *hostProfileList, const string &hostName, bool skipChooser)
{
    int  i;
    profile = HostProfile();

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
        matchingProfiles = 
                    hostProfileList->FindAllMatchingProfileForHost(hostName);

        if (matchingProfiles.size() > 0)
        {
            profile = *matchingProfiles[0];
        }
        
        if (matchingProfiles.size() > 1 ||
            (matchingProfiles.size() == 1 &&
             matchingProfiles[0]->GetParallel()))
        {
            for (i = 0; i < matchingProfiles.size(); i++)
            {
                if (matchingProfiles[i]->GetActive())
                {
                    profile.SetNumProcessors(matchingProfiles[i]->GetNumProcessors());
                    profile.SetNumNodes(matchingProfiles[i]->GetNumNodes());
                    profile.SetBank(matchingProfiles[i]->GetBank());
                    profile.SetTimeLimit(matchingProfiles[i]->GetTimeLimit());
                }
            }
        }

        // Save it for use later
        cachedProfile[hostName] = profile;
    }
    return true;
}

