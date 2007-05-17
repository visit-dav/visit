/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              ViewerQuery.h                                //
// ************************************************************************* //

#ifndef VIEWER_QUERY_H
#define VIEWER_QUERY_H

#include <viewer_exports.h>
#include <SimpleObserver.h>
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
// ****************************************************************************


class VIEWER_API ViewerQuery : public SimpleObserver
{
  public:
                     ViewerQuery(ViewerWindow *, ViewerWindow *, Line *,
                                 const bool fromDefault = true,
                                 const bool forceSampling = false);
                     ViewerQuery(const ViewerQuery *obj, int ts); 
                    ~ViewerQuery();

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

