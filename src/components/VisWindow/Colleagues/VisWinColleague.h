// ************************************************************************* //
//                           VisWinColleague.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_COLLEAGUE_H
#define VIS_WIN_COLLEAGUE_H
#include <viswindow_exports.h>


#include <vector>

#include <avtActor.h>

class     VisWindowColleagueProxy;


// ****************************************************************************
//  Class: VisWinColleague
//
//  Purpose:
//      Defines an abstract type that all colleagues should be derived from.
//      This module follows the mediator/colleague model described in
//      "Design Patterns".  The mediator is a VisWindow and its colleagues are
//      all derived from this class.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 10:41:09 PDT 2000
//    Added virtual functions for whether or not updates are enabled and
//    whether or not there are plots.
//
//    Brad Whitlock, Wed Aug 29 15:34:40 PST 2001
//    Added virtual methods for setting the gradient background color and mode.
//
//    Brad Whitlock, Tue Feb 12 11:43:04 PDT 2002
//    Added a virtual destructor so destructors for derived classes will
//    be called.
//
//    Hank Childs, Tue Mar 12 17:04:02 PST 2002
//    Added virtual method UpdatePlotList.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Added Start/Stop CurveMode. 
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Added support for tracking motion.
//
//    Brad Whitlock, Mon Sep 23 15:41:50 PST 2002
//    I added support for setting the surface representation and the immediate
//    rendering mode.
//
//    Kathleen Bonnell, Wed May 28 15:52:32 PDT 2003  
//    Added virtual method ReAddToWindow.  
//    
//    Kathleen Bonnell, Fri Jun  6 08:57:33 PDT 2003 
//    Added virtual methods FullFrameOn, FullFrameOff.  
//    
// ****************************************************************************

class VISWINDOW_API VisWinColleague
{
  public:
                              VisWinColleague(VisWindowColleagueProxy &);
    virtual                  ~VisWinColleague() {;};

    virtual void              SetBackgroundColor(float, float, float) {;};
    virtual void              SetGradientBackgroundColors(int,
                                                          float, float, float,
                                                          float, float, float)
                                                         {;};
    virtual void              SetBackgroundMode(int) {;};
    virtual void              SetForegroundColor(float, float, float) {;};

    virtual void              SetViewport(float, float, float, float) {;};

    virtual void              Start2DMode(void)    {;};
    virtual void              Start3DMode(void)    {;};
    virtual void              StartCurveMode(void) {;};
    virtual void              Stop2DMode(void)     {;};
    virtual void              Stop3DMode(void)     {;};
    virtual void              StopCurveMode(void)  {;};

    virtual void              EnableUpdates(void)  {;};
    virtual void              DisableUpdates(void) {;};

    virtual void              HasPlots(void)       {;};
    virtual void              NoPlots(void)        {;};

    virtual void              MotionBegin(void)    {;};
    virtual void              MotionEnd(void)      {;};

    virtual void              SetSurfaceRepresentation(int) {;};
    virtual void              SetImmediateModeRendering(bool) {;};

    virtual void              UpdateView(void)     {;};

    virtual void              UpdatePlotList(std::vector<avtActor_p> &) {;};

    virtual void              ReAddToWindow(void)       {;};

    virtual void              FullFrameOn(const double, const int)       {;};
    virtual void              FullFrameOff(void)       {;};

  protected:
    VisWindowColleagueProxy  &mediator;
};


#endif


