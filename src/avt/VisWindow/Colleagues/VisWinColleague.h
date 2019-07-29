// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           VisWinColleague.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_COLLEAGUE_H
#define VIS_WIN_COLLEAGUE_H
#include <viswindow_exports.h>

#include <vector>
#include <string>

#include <avtActor.h>

class     VisWindowColleagueProxy;
class     ColorAttribute;

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
//    Brad Whitlock, Thu Nov 6 12:10:43 PDT 2003
//    Added virtual method SetFrameAndState.
//
//    Jeremy Meredith, Fri Nov 14 17:13:53 PST 2003
//    Added specular properties.
//
//    Chris Wojtan, Mon Jul 26 16:30:43 PDT 2004
//    Added 4 functions for suspending and resuming opaque and
//    translucent geometry.
//
//    Brad Whitlock, Mon Sep 18 11:11:09 PDT 2006
//    Added color texturing.
//
//    Brad Whitlock, Wed Nov 14 15:23:23 PST 2007
//    Added background image support.
//
//    Jeremy Meredith, Mon Jan 28 17:09:12 EST 2008
//    Added AxisArray mode.
//
//    Eric Brugger, Tue Dec  9 14:25:37 PST 2008
//    Added the ParallelAxes window mode.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Account for 3D axis scaling (3D equivalent of full-frame mode).
//
//    Burlen Loring, Thu Oct  8 12:53:58 PDT 2015
//    Fix a compiler warning
//
// ****************************************************************************

class VISWINDOW_API VisWinColleague
{
  public:
                              VisWinColleague(VisWindowColleagueProxy &);
    virtual                  ~VisWinColleague() {;};

    virtual void              SetBackgroundColor(double, double, double) {;};
    virtual void              SetGradientBackgroundColors(int,
                                                      double, double, double,
                                                      double, double, double)
                                                         {;};
    virtual void              SetBackgroundMode(int) {;};
    virtual void              SetBackgroundImage(const std::string &,int,int) {;};
    virtual void              SetForegroundColor(double, double, double) {;};

    virtual void              SetViewport(double, double, double, double) {;};

    virtual void              Start2DMode(void)    {;};
    virtual void              Start3DMode(void)    {;};
    virtual void              StartCurveMode(void) {;};
    virtual void              StartAxisArrayMode(void) {;};
    virtual void              StartParallelAxesMode(void) {;};
    virtual void              Stop2DMode(void)     {;};
    virtual void              Stop3DMode(void)     {;};
    virtual void              StopCurveMode(void)  {;};
    virtual void              StopAxisArrayMode(void)  {;};
    virtual void              StopParallelAxesMode(void) {;};

    virtual void              EnableUpdates(void)  {;};
    virtual void              DisableUpdates(void) {;};

    virtual void              HasPlots(void)       {;};
    virtual void              NoPlots(void)        {;};

    virtual void              MotionBegin(void)    {;};
    virtual void              MotionEnd(void)      {;};

    virtual void              SetSurfaceRepresentation(int) {;};

    virtual void              SetSpecularProperties(bool,double,double,
                                                    const ColorAttribute&) {;};

    virtual void              SetColorTexturingFlag(bool) {;};

    virtual void              UpdateView(void)     {;};

    virtual void              UpdatePlotList(std::vector<avtActor_p> &) {;};

    virtual void              ReAddToWindow(void)       {;};

    virtual void              FullFrameOn(const double, const int)       {;};
    virtual void              FullFrameOff(void)       {;};

    virtual void              Set3DAxisScalingFactors(bool,const double [3]){}

    virtual void              SetFrameAndState(int, int, int, int,
                                               int, int, int) {;};

    virtual void              SuspendOpaqueGeometry(void)      {;};
    virtual void              SuspendTransparentGeometry(void) {;};
    virtual void              ResumeOpaqueGeometry(void)       {;};
    virtual void              ResumeTransparentGeometry(void)  {;};

  protected:
    VisWindowColleagueProxy  &mediator;
};


#endif


