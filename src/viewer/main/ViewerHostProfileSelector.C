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


