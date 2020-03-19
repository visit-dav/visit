// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerHostProfileSelector.h>

// ****************************************************************************
//  Constructor:  ViewerHostProfileSelector::ViewerHostProfileSelector
//
//  Programmer:  Kathleen Bonnell
//  Creation:    February 5, 2003 
//
//  Modifications:
//
// ****************************************************************************

ViewerHostProfileSelector::ViewerHostProfileSelector()
{
}

// ****************************************************************************
//  Destructor:  ViewerHostProfileSelector::~ViewerHostProfileSelector
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//
// ****************************************************************************

ViewerHostProfileSelector::~ViewerHostProfileSelector()
{
}

// ****************************************************************************
//  Method:  ViewerHostProfileSelector::ClearCache
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
// ****************************************************************************

void
ViewerHostProfileSelector::ClearCache(const std::string &hostName)
{
    cachedProfile.erase(hostName);
}


// ****************************************************************************
//  Method:  ViewerHostProfileSelector::AddRestartArgsToCachedProfile
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
//  Modifications:
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Jeremy Meredith, Tue Jul 13 16:16:45 EDT 2010
//    Sometimes we just cache a dummy machine profile.  In these cases, we
//    still need to be able to save off command line args.  (These coincide
//    when someone adds -np 4 to the command line, for instance.)  So we
//    create an empty launch profile to use for this case.
//
// ****************************************************************************
void
ViewerHostProfileSelector::AddRestartArgsToCachedProfile(
                                          const std::string &hostName,
                                          const std::vector<std::string> &args)
{
    if (cachedProfile.count(hostName))
    {
        // If we don't have any launch profiles, add one!
        if (cachedProfile[hostName].GetNumLaunchProfiles() == 0)
        {
            cachedProfile[hostName].AddLaunchProfiles(LaunchProfile());
            cachedProfile[hostName].SetActiveProfile(0);
        }

        // This had better be true now......
        if (cachedProfile[hostName].GetActiveLaunchProfile())
        {
            std::vector<std::string> &a =
             cachedProfile[hostName].GetActiveLaunchProfile()->GetArguments();
            a.insert(a.end(), args.begin(), args.end());
        }
    }
}
