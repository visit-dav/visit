// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_HOSTPROFILESELECTOR_H
#define VIEWER_HOSTPROFILESELECTOR_H
#include <viewercore_exports.h>
#include <MachineProfile.h>
#include <LaunchProfile.h>

#include <string>
#include <vector>
#include <map>

class HostProfileList;

// ****************************************************************************
//  Class:  ViewerHostProfileSelector
//
//  Purpose:
//    Picks a host profile. 
//
//  Notes:
//    Copied from ViewerEngineChooser by Jeremy Meredith.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  2 14:43:23 PST 2004
//    Added AddRestartArgsToCachedProfile.  This is a better way to save
//    the arguments than what the ViewerEngineManager was doing for
//    normal engine launches.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
// ****************************************************************************
class VIEWERCORE_API ViewerHostProfileSelector
{
  public:
             ViewerHostProfileSelector();
    virtual ~ViewerHostProfileSelector();

    virtual bool SelectProfile(HostProfileList*, const std::string&, bool skip) = 0;

    void AddRestartArgsToCachedProfile(const std::string&,
                                       const std::vector<std::string>&);
    void ClearCache(const std::string&);
    const MachineProfile &GetMachineProfile(void) { return profile; };

  protected:

    MachineProfile                  profile;

    std::map<std::string, MachineProfile> cachedProfile;

};

#endif
