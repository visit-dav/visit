#ifndef VIEWER_HOSTPROFILESELECTOR_H
#define VIEWER_HOSTPROFILESELECTOR_H
#include <viewer_exports.h>
#include <HostProfile.h>

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
// ****************************************************************************
class VIEWER_API ViewerHostProfileSelector 
{
  public:
             ViewerHostProfileSelector();
    virtual ~ViewerHostProfileSelector();

    virtual bool SelectProfile(HostProfileList*, const std::string&, bool skip) = 0;

    void AddRestartArgsToCachedProfile(const std::string&,
                                       const std::vector<std::string>&);
    void ClearCache(const std::string&);
    const HostProfile &GetHostProfile(void) { return profile; };

  protected:

    std::vector<const HostProfile*> matchingProfiles;
    HostProfile                     profile;

    std::map<std::string, HostProfile> cachedProfile;

};

#endif
