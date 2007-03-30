#ifndef VIEWER_HOSTPROFILESELECTOR_NOWIN_H
#define VIEWER_HOSTPROFILESELECTOR_NOWIN_H
#include <viewer_exports.h>
#include <ViewerHostProfileSelector.h>

#include <string>

class HostProfileList;

// ****************************************************************************
//  Class:  ViewerHostProfileSelectorNoWin
//
//  Purpose:
//    Selects a host profile. 
//  
//  Notes: Extracted from ViewerEngineChooser.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    February 5, 2003 
//
// ****************************************************************************

class VIEWER_API ViewerHostProfileSelectorNoWin : public ViewerHostProfileSelector
{
  public:
             ViewerHostProfileSelectorNoWin();
    virtual ~ViewerHostProfileSelectorNoWin();

    virtual bool SelectProfile(HostProfileList*, const std::string&, bool skip);
};

#endif
