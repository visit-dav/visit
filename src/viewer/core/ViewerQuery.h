// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              ViewerQuery.h                                //
// ************************************************************************* //

#ifndef VIEWER_QUERY_H
#define VIEWER_QUERY_H

#include <viewercore_exports.h>
#include <SimpleObserver.h>
#include <ViewerBase.h>
#include <ref_ptr.h>

// Forward declarations.
class Line;
class PlaneAttributes;
class PlotQueryInfo;
class ViewerPlot;
class ViewerWindow;
class avtToolInterface;


// ****************************************************************************
//  Class: ViewerQuery
//
//  Purpose:  
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002
//    Added width & height, for scaling purposes. Allow the
//    results window to be retrieved.
//
//    Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002 
//    Added methods for handling tools. 
//
//    Kathleen Bonnell, Fri Jul 26 15:45:13 PDT 2002
//    Remove unused member origPlotQueryInfo.
//
//    Kathleen Bonnell, Mon Jul 29 09:36:35 PDT 2002  
//    Remove unnecessary methods InteractiveOn, InteractiveOff. 
//
//    Kathleen Bonnell, Thu Mar  6 15:15:30 PST 2003 
//    Added methods GetOriginatingWindow, GetOriginatingPlot, SendVisualCue, 
//    ReCreateLineout, UpdateLineFromSlice, Start/StopObservingPlot. 
//
//    Kathleen Bonnell, Thu Sep 11 12:04:26 PDT 2003 
//    Added optional bool arg to constructor and CreateLineout, indicates
//    whether Lineout should initialize itself with its default atts or
//    its client atts. 
//
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004 
//    Removed width/height GetWidth/GetHeight, no longer used. 
//
//    Kathleen Bonnell, Thu Feb  3 16:03:32 PST 2005 
//    Added new copy constructor, MatchTimeState, SetFollowsTime. 
//
//    Kathleen Bonnell, Wed Jun 21 17:52:26 PDT 2006
//    Added UpdateResultsTime. 
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Added optional bool arg to constructor, CreateLineout. 
//
//    Brad Whitlock, Tue Jun 24 14:51:09 PDT 2008
//    Inherit ViewerBase.
//
// ****************************************************************************


class VIEWERCORE_API ViewerQuery : public SimpleObserver, private ViewerBase
{
  public:
                     ViewerQuery(ViewerWindow *, ViewerWindow *, Line *,
                                 const bool fromDefault = true,
                                 const bool forceSampling = false);
                     ViewerQuery(const ViewerQuery *obj, int ts); 
    virtual         ~ViewerQuery();

    bool             MatchResultsPlot(ViewerPlot *vp) const; 
    bool             MatchOriginatingPlot(ViewerPlot *vp) const; 

    bool             MatchResultsWindow(ViewerWindow *vw) const; 
    bool             MatchOriginatingWindow(ViewerWindow *vw) const; 
    bool             MatchTimeState(int ts) const;

    void             DeleteOriginatingWindow();
    void             DeleteOriginatingPlot();
    void             DeleteVisualCue();
    void             SetFollowsTime(bool);

    virtual void     Update(Subject *) ;

    ViewerWindow    *GetResultsWindow() const;
    ViewerWindow    *GetOriginatingWindow() const;
    ViewerPlot      *GetOriginatingPlot() const;

    bool             CanHandleTool();
    bool             IsHandlingTool();
    bool             InitializeTool(avtToolInterface &ti);
    bool             HandleTool(const avtToolInterface &ti);
    void             DisableTool();
    void             SendVisualCue();
    void             ReCreateLineout();

    bool             UpdateLineFromSlice(PlaneAttributes *);

    void             UpdateResultsTime(int ts);

  private:
    void             CreateLineout(const bool fromDefault = true,
                                   const bool forceSampling = false);
    void             StartObservingPlot();
    void             StopObservingPlot();

    PlotQueryInfo   *resPlotQueryInfo;

    Line            *lineAtts;
    ViewerWindow    *originatingWindow;
    ViewerWindow    *resultsWindow;

    ViewerPlot      *resultsPlot;
    ViewerPlot      *originatingPlot;

    bool             handlingTool;

    PlaneAttributes *planeAtts;
};

typedef ref_ptr<ViewerQuery> ViewerQuery_p; 

#endif

