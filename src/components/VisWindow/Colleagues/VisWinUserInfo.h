// ************************************************************************* //
//                              VisWinUserInfo.h                             //
// ************************************************************************* //

#ifndef VIS_WIN_USER_INFO_H
#define VIS_WIN_USER_INFO_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkTextActor;

class VisWindowColleagueProxy;


// ****************************************************************************
//  Class: VisWinUserInfo
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It displays
//      the user info (user name and start time).
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 10:45:43 PDT 2000
//    Added HasPlots/NoPlots and addedUserInfo.
//
//    Hank Childs, Tue Sep 18 11:58:33 PDT 2001
//    Made destructor virtual.
//
//    Brad Whitlock, Thu Jan 10 08:15:14 PDT 2002
//    Added the SetVisibility method.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    vtkTextMapper/vtkScaledTextActor pairs have been replaced by 
//    vtkTextActor which defines its own mapper. (new vtk api).
//
// ****************************************************************************

class VISWINDOW_API VisWinUserInfo : public VisWinColleague
{
  public:
                         VisWinUserInfo(VisWindowColleagueProxy &);
    virtual             ~VisWinUserInfo();

    virtual void         SetForegroundColor(float, float, float);

    virtual void         HasPlots(void);
    virtual void         NoPlots(void);
    void                 SetVisibility(bool);

  protected:
    vtkTextActor        *infoActor;
    char                *infoString;

    bool                 addedUserInfo;

    void                 AddToWindow(void);
    void                 RemoveFromWindow(void);
};


#endif


