// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWERREMOTEPROCESSCHOOSER_H
#define VIEWERREMOTEPROCESSCHOOSER_H
#include <viewercore_exports.h>
#include <ViewerBase.h>

#include <string>
#include <vector>

class HostProfileList;
class MachineProfile;
class RemoteProxyBase;
class ViewerHostProfileSelector;

// ****************************************************************************
//  Class:  ViewerRemoteProcessChooser
//
//  Purpose:
//    Picks a host profile and some parallel settings to launch the engine.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 14, 2002
//
//  Modifications:
//    Jeremy Meredith, Sat Aug 17 11:03:20 PDT 2002
//    Added nowin support.
//
//    Jeremy Meredith, Wed Dec 18 17:20:23 PST 2002
//    Added support for skipping the chooser when launching engines.
//
//    Kathleen Bonnell, Wed Feb  5 09:40:21 PST 2003   
//    Moved host-profile selection code to ViewerHostProfileSelector. Added
//    member selector. 
//
//    Jeremy Meredith, Thu Jun 26 10:54:37 PDT 2003
//    Renamed to ViewerRemoteProcessChooser.  Split GetNewEngine into two
//    functions (SelectProfile and AddProfileArguments) so that the caller
//    could create a new proxy itself.  This disassociates this object
//    from the Engine and lets it be used with the VCL (for example).
//
//    Jeremy Meredith, Fri Apr  2 14:35:24 PST 2004
//    Added AddRestartArgsToCachedProfile.  This is a better way to save
//    the arguments than what the ViewerEngineManager was doing for
//    normal engine launches.
//
//    Brad Whitlock, Wed Aug 4 17:38:16 PST 2004
//    I made the selected host profile get passed out of SelectProfile. I also
//    moved AddProfileArguments into RemoteProxyBase.
//
//    Brad Whitlock, Tue Apr 14 11:42:03 PDT 2009
//    Inherit from ViewerBase.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
// ****************************************************************************

class VIEWERCORE_API ViewerRemoteProcessChooser : public ViewerBase
{
  public:
    virtual ~ViewerRemoteProcessChooser();

    static ViewerRemoteProcessChooser *Instance();
    static void SetNoWinMode(bool nw);
    bool SelectProfile(HostProfileList*, const std::string&, bool skip,
                       MachineProfile &profile);
    void AddRestartArgsToCachedProfile(const std::string&,
                                       const std::vector<std::string>&);
    void ClearCache(const std::string&);

  private:
    static ViewerRemoteProcessChooser *instance;

    ViewerHostProfileSelector *selector;
    ViewerRemoteProcessChooser();
};

#endif
