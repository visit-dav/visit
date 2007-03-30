// ************************************************************************* //
//                               VisWinView.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_VIEW_H
#define VIS_WIN_VIEW_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


// ****************************************************************************
//  Class: VisWinView
//
//  Purpose:
//      Handles the view information and resetting of cameras.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Nov 10 10:19:14 PST 2000
//    Added code to support turning perspective projections on and off.
//
//    Hank Childs, Fri Jan  5 17:55:42 PST 2001
//    Removed antiquated method SetCameraToView.
//
//    Eric Brugger, Mon Mar 12 14:48:34 PST 2001
//    I removed the Get/SetPerspectiveProjection methods.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Added support for curve mode. 
//
// ****************************************************************************

class VISWINDOW_API VisWinView : public VisWinColleague
{
  public:
                         VisWinView(VisWindowColleagueProxy &);
    virtual             ~VisWinView() {;};

    const avtViewInfo   &GetViewInfo(void);
    void                 SetViewInfo(const avtViewInfo &);

    void                 ResetView(void);

    virtual void         Start2DMode(void);
    virtual void         Stop2DMode(void);

    virtual void         StartCurveMode(void);
    virtual void         StopCurveMode(void);

  protected:
    avtViewInfo          viewInfo;
};


#endif


