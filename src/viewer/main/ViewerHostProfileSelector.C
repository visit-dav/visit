#include <ViewerHostProfileSelector.h>
#include <string>


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
// ****************************************************************************
void
ViewerHostProfileSelector::AddRestartArgsToCachedProfile(
                                          const std::string &hostName,
                                          const std::vector<std::string> &args)
{
    if (cachedProfile.count(hostName))
    {
        std::vector<std::string> &a = cachedProfile[hostName].GetArguments();
        a.insert(a.end(), args.begin(), args.end());
    }
}
