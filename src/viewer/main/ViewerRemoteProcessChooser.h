#ifndef VIEWERREMOTEPROCESSCHOOSER_H
#define VIEWERREMOTEPROCESSCHOOSER_H
#include <viewer_exports.h>

#include <string>

class HostProfileList;
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
// ****************************************************************************
class VIEWER_API ViewerRemoteProcessChooser 
{
  public:
    ~ViewerRemoteProcessChooser();

    static ViewerRemoteProcessChooser *Instance();
    static void SetNoWinMode(bool nw);
    bool SelectProfile(HostProfileList*, const std::string&, bool skip);
    void AddProfileArguments(RemoteProxyBase*, bool addParallelArgs);
    void ClearCache(const std::string&);

  private:
    static bool nowin;
    static ViewerRemoteProcessChooser *instance;

    ViewerHostProfileSelector *selector;
    ViewerRemoteProcessChooser();
};

#endif
