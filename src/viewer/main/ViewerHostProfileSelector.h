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
//
// ****************************************************************************
class VIEWER_API ViewerHostProfileSelector 
{
  public:
             ViewerHostProfileSelector();
    virtual ~ViewerHostProfileSelector();

    virtual bool SelectProfile(HostProfileList*, const std::string&, bool skip) = 0;
    void ClearCache(const std::string&);
    const HostProfile &GetHostProfile(void) { return profile; };

  protected:

    std::vector<const HostProfile*> matchingProfiles;
    HostProfile                     profile;

    std::map<std::string, HostProfile> cachedProfile;

};

#endif
