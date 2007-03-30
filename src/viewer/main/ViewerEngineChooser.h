#ifndef VIEWERENGINECHOOSER_H
#define VIEWERENGINECHOOSER_H
#include <viewer_exports.h>

#include <string>

class HostProfileList;
class EngineProxy;
class ViewerHostProfileSelector;

// ****************************************************************************
//  Class:  ViewerEngineChooser
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
// ****************************************************************************
class VIEWER_API ViewerEngineChooser 
{
  public:
    ~ViewerEngineChooser();

    static ViewerEngineChooser *Instance();
    static void SetNoWinMode(bool nw);
    EngineProxy *GetNewEngine(HostProfileList*, const std::string&, bool skip);
    void ClearCache(const std::string&);

  private:
    static bool nowin;
    static ViewerEngineChooser *instance;

    ViewerHostProfileSelector *selector;
    ViewerEngineChooser();
};

#endif
