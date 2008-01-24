/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              ViewerQuery.C                                //
// ************************************************************************* //

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <ViewerQuery.h>

#include <ViewerOperator.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <AttributeSubjectMap.h>
#include <DebugStream.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <Line.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <PlaneAttributes.h>
#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <PlotQueryInfo.h>
#include <avtMatrix.h>
#include <avtToolInterface.h>
#include <avtVector.h>



void CreateBasis(const avtVector &N, const avtVector &UP, 
                 avtVector &u, avtVector &v, avtVector&w);


// ***********************************************************************
//  Method: ViewerQuery constructor
//
//  Arguments:
//    origWin   A pointer to the window that originated the query.
//    resWin    A pointer to the window where results are to be rendered.
//    lA        The line attributes to be used for the Lineout Operator
//              and Curve Plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002    
//    Initialize new member width & height.
//
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002    
//    Initialize new handlingTool.
//
//    Kathleen Bonnell, Fri Jul 26 15:45:13 PDT 2002 
//    Remove unused member origPlotQueryInfo. 
//
//    Kathleen Bonnell, Mon Jul 29 09:36:35 PDT 2002 
//    Replace call to InteractiveOn with the necessary code.
//
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Use new line attributes member ReflineLabels, which allows the 
//    reflines drawn in the originating window to have labels. 
//
//    Kathleen Bonnell, Fri Mar  7 16:27:04 PST 2003 
//    Initialize planeAtts. 
//    
//    Kathleen Bonnell, Thu Sep 11 12:04:26 PDT 2003 
//    Added optional 'fromDefault' flag. 
//    
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004 
//    Removed width & height.
//
//    Kathleen Bonnell, Thu Jul 29 09:49:00 PDT 2004 
//    Set lineAtts SamplingOn from temp. 
//
//    Kathleen Bonnell, Thu Nov 18 17:56:00 PST 2004 
//    Added call to SendVisualCue. 
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Added optional bool arg, forceSampling. 
//
// ***********************************************************************

ViewerQuery::ViewerQuery(ViewerWindow *origWin, ViewerWindow *resWin, 
                         Line *lA, const bool fromDefault,
                         const bool forceSampling) : SimpleObserver()
{
    resPlotQueryInfo = 0;
    planeAtts = 0;

    lineAtts = new Line();
    originatingWindow = origWin;
    resultsWindow = resWin;
    lineAtts->CopyAttributes(lA);

    CreateLineout(fromDefault, forceSampling);

    //  
    // Retrieve the interactivity and sampling setting from LineoutOp Atts.
    //
    int id = resultsPlot->GetNOperators();
    Line *temp = (Line *)resultsPlot->GetOperator(id-1)->GetOperatorAtts()->
                   CreateCompatible("Line");
    lineAtts->SetInteractive(temp->GetInteractive()); 
    lineAtts->SetReflineLabels(temp->GetReflineLabels()); 
    lineAtts->SetSamplingOn(forceSampling || temp->GetSamplingOn()); 
    delete temp;

    SendVisualCue();
    //
    // Connect to the resultsPlot
    //
    StartObservingPlot();
    
    handlingTool = false;
}


// ***********************************************************************
//  Method: ViewerQuery copy constructor
//
//  Arguments:
//    obj       A ViewerQuery to copy. 
//    ts        The time state for this new query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Brad Whitlock, Thu Jan 24 11:55:42 PDT 2008
//    Added new argument to AddPlot.
//
// ***********************************************************************

ViewerQuery::ViewerQuery(const ViewerQuery *obj, int ts) : SimpleObserver()
{
    resPlotQueryInfo = 0;
    planeAtts = 0;

    lineAtts = new Line();
    originatingWindow = obj->originatingWindow;
    resultsWindow = obj->resultsWindow;
    lineAtts->CopyAttributes(obj->lineAtts);

    ViewerQueryManager *vqm = ViewerQueryManager::Instance();
    if (vqm->GetGlobalLineoutAtts()->GetColorOption() ==
        GlobalLineoutAttributes::CreateColor)
    {
        lineAtts->SetColor(vqm->GetColor());
    }

    originatingPlot = obj->originatingPlot;
    std::string vName;
    if (lineAtts->GetVarName() == "default")
        vName = originatingPlot->GetVariableName();
    else 
        vName = lineAtts->GetVarName();

    bool replacePlots = ViewerWindowManager::Instance()->
                        GetClientAtts()->GetReplacePlots();

    ViewerPlotList *plotList = resultsWindow->GetPlotList();

    int plotType = PlotPluginManager::Instance()->GetEnabledIndex("Curve_1.0");

    int pid = plotList->AddPlot(plotType, vName, replacePlots, false, false);
    resultsPlot = plotList->GetPlot(pid);
    resultsPlot->SetSILRestriction(originatingPlot->GetSILRestriction());

    //
    // Copy operators from the originating plot to the results (curve) plot.
    //
    for (int j = 0; j < originatingPlot->GetNOperators(); ++j)
    {
        ViewerOperator *op = originatingPlot->GetOperator(j);
        //
        // Keep a copy of the slice-plane atts, so that the line endpoints
        // may be updated accordingly whent the slice-plane changes.
        //
        if (strcmp(op->GetName(), "Slice") == 0)
        {
            planeAtts = (PlaneAttributes *)op->GetOperatorAtts()->
                        CreateCompatible("PlaneAttributes");
        }
        resultsPlot->AddOperator(op->GetType());
        ViewerOperator *newOp = resultsPlot->GetOperator(j);
        newOp->SetOperatorAtts(op->GetOperatorAtts());
    }
 
    //
    // Add the lineout operator.
    //
    int operatorType = OperatorPluginManager::Instance()->GetEnabledIndex("Lineout_1.0");
    resultsPlot->AddOperator(operatorType, false);

    //
    //  The client atts were already copied created,
    //  so now update them with the lineAtts.
    //
    resultsPlot->SetPlotAtts(lineAtts);

    int id = resultsPlot->GetNOperators() -1;
    resultsPlot->GetOperator(id)->SetOperatorAtts(lineAtts);
    resultsPlot->GetOperator(id)->SetOperatorAtts(
        ViewerQueryManager::Instance()->GetGlobalLineoutAtts());

    resultsPlot->SetFollowsTime(true);
    resultsPlot->SetCacheIndex(ts);
    resultsPlot->SetFollowsTime(false);
    plotList->RealizePlots();

    //
    // Connect to the resultsPlot
    //
    StartObservingPlot();
    
    handlingTool = false;
}


// ****************************************************************************
//  Method: ViewerQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 26 15:45:13 PDT 2002 
//    Remove unused member origPlotQueryInfo. 
//
//    Kathleen Bonnell, Wed Mar  5 08:24:55 PST 2003  
//    Delete planeAtts. 
//
// ****************************************************************************

ViewerQuery::~ViewerQuery()
{
    StopObservingPlot();
    if (lineAtts)
        delete lineAtts;
    if (planeAtts)
        delete planeAtts;
}


// ****************************************************************************
//  Method: ViewerQuery::StartObservingPlot
//
//  Purpose:  Connects to the results plot, so this query can know when
//            certain things change.
//
//  Programmer: Kathleen Bonnell 
//  Creation:    February 14, 2003
//
// ****************************************************************************

void
ViewerQuery::StartObservingPlot()
{
    if (resPlotQueryInfo == 0 && resultsPlot !=0)
    {
        //
        // Connect to the resultsPlot
        //
        resPlotQueryInfo = resultsPlot->GetPlotQueryInfo();
        resPlotQueryInfo->Attach(this);
    }
}


// ****************************************************************************
//  Method: ViewerQuery::StopObservingPlot
//
//  Purpose:  Disconnects from the results plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 14, 2003 
//
// ****************************************************************************

void
ViewerQuery::StopObservingPlot()
{
    if (resPlotQueryInfo != 0)
    {
        resPlotQueryInfo->Detach(this);
        resPlotQueryInfo = 0;
    }
}


// ****************************************************************************
//  Method: ViewerQuery::CreateLineout
//
//  Purpose:  Handles creation of the Curve plot and Lineout operators,
//            copying other operators from the originating plot as necessary.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002    
//    Move calculation of scaling factor and setting it in the Curve View
//    to ViewerQueryManager.
//
//    Kathleen Bonnell, Fri Jul 26 15:45:13 PDT 2002 
//    Get hostdatabasename directly from originating plot, instead of
//    from the plot list, as the plot list may reflect a database from another
//    plot.  Delete bounds, range as the memory is allocated in GetDataExtents/
//    GetSpatialExtents, but not deallocated.  Retrieve plotType operatorType
//    from PluginManager using full name+version instead of just name.
//
//    Kathleen Bonnell, Wed Oct  2 15:33:08 PDT 2002
//    Use NFrames, and FrameIndex from originatingWindow's animation to
//    set the correct frame index in the resultsWindow. 
//
//    Eric Brugger, Wed Dec 11 12:06:57 PST 2002
//    I added keyframing support.
//
//    Kathleen Bonnell, Wed Mar  5 08:24:55 PST 2003  
//    Store the slice-plane attributes if encountered.  Added call to
//    SendVisualCue.
//
//    Kathleen Bonnell, Fri Mar 14 17:11:42 PST 2003  
//    Retrieve variable from lineAtts, unless it is set to "default". 
//
//    Brad Whitlock, Thu Apr 3 11:19:39 PDT 2003
//    I removed nstate stuff.
//
//    Jeremy Meredith, Tue Jun 17 19:29:00 PDT 2003
//    Changed GetAllIndex to GetEnabledIndex.
//
//    Kathleen Bonnell, on Aug  4 17:26:07 PDT 2003 
//    Set animation's FrameIndex only if it is different. 
//
//    Kathleen Bonnell, Thu Sep 11 12:04:26 PDT 2003 
//    Added bool argument, which indicates whether the lineout should
//    be initialized against its default atts or its client atts. 
//
//    Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//    Account for multiple active plots.
//
//    Kathleen Bonnell 
//    Set SILRestriction for the resultsPlot from the SILRestriction of
//    the originatingPlot. 
//
//    Kathleen Bonnell, Thu Mar 11 08:19:10 PST 2004
//    Removed calculation of width and height. 
//
//    Brad Whitlock, Sat Jan 31 23:04:00 PST 2004
//    I made it work without frame numbers.
//
//    Jeremy Meredith, Tue Mar 30 16:18:00 PST 2004
//    Added code to set the engine key for the new plot list.
//
//    Kathleen Bonnell, Thu Nov 18 18:12:56 PST 2004 
//    Removed call to SendVisualCue (called elsewhere).  Added call to 
//    set Lineout operator atts from GlobalLineoutAttributes.
//
//    Kathleen Bonnell, Wed Jun 28 15:40:28 PDT 2006
//    Use timeslider associated with hdbName instead of ActiveTimeSlider.
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Added optional bool arg, forceSampling. Resend lineAtts to Lineout
//    if forceSampling is true. 
//
//    Brad Whitlock, Thu Jan 24 11:56:11 PDT 2008
//    Added new argument to AddPlot.
//
// ****************************************************************************

void
ViewerQuery::CreateLineout(const bool fromDefault, const bool forceSampling)
{
    //
    //  Grab information from the originating window.
    //
    ViewerPlotList *origList = originatingWindow->GetPlotList();
    intVector plotIDs;
    origList->GetActivePlotIDs(plotIDs);
    int origPlotID = (plotIDs.size() > 0 ? plotIDs[0] : -1);
    originatingPlot = origList->GetPlot(origPlotID);

    std::string vName;
    if (lineAtts->GetVarName() == "default")
        vName = originatingPlot->GetVariableName();
    else 
        vName = lineAtts->GetVarName();
    std::string hdbName(originatingPlot->GetSource());
    bool replacePlots = ViewerWindowManager::Instance()->
                        GetClientAtts()->GetReplacePlots();
 
    int plotType = PlotPluginManager::Instance()->GetEnabledIndex("Curve_1.0");
    ViewerPlotList *plotList = resultsWindow->GetPlotList();

    // If the original plot list has a time slider corresponding to the
    // the originating plot's hostdabase name, make sure that
    // the the new plot list uses the same time slider.
    if (origList->TimeSliderExists(hdbName))
    {
        int state, nStates;
        origList->GetTimeSliderStates(hdbName, state, nStates);
        plotList->CreateTimeSlider(hdbName, state);
        plotList->SetActiveTimeSlider(hdbName);
    }

    plotList->SetHostDatabaseName(hdbName);
    plotList->SetEngineKey(originatingPlot->GetEngineKey());
 
    int pid = plotList->AddPlot(plotType, vName, replacePlots, false, false);
    resultsPlot = plotList->GetPlot(pid);

    resultsPlot->SetSILRestriction(originatingPlot->GetSILRestriction()); 
    //
    // Copy operators from the originating plot to the results (curve) plot.
    //
    for (int j = 0; j < originatingPlot->GetNOperators(); ++j)
    {
        ViewerOperator *op = originatingPlot->GetOperator(j);
        //
        // Keep a copy of the slice-plane atts, so that the line endpoints
        // may be updated accordingly whent the slice-plane changes.
        //
        if (strcmp(op->GetName(), "Slice") == 0)
        {
            planeAtts = (PlaneAttributes *)op->GetOperatorAtts()->
                        CreateCompatible("PlaneAttributes");
        }
        resultsPlot->AddOperator(op->GetType());
        ViewerOperator *newOp = resultsPlot->GetOperator(j);
        newOp->SetOperatorAtts(op->GetOperatorAtts());
    }
 
    //
    // Add the lineout operator.
    //
    int operatorType = OperatorPluginManager::Instance()->GetEnabledIndex("Lineout_1.0");
    resultsPlot->AddOperator(operatorType, fromDefault);

    //
    //  The client atts were already copied created,
    //  so now update them with the lineAtts.
    //
    resultsPlot->SetPlotAtts(lineAtts);

    int id = resultsPlot->GetNOperators() -1;
    resultsPlot->GetOperator(id)->SetOperatorAtts(lineAtts);
    resultsPlot->GetOperator(id)->SetOperatorAtts(
        ViewerQueryManager::Instance()->GetGlobalLineoutAtts());
    if (forceSampling)
    {
        lineAtts->SetSamplingOn(true);
        resultsPlot->GetOperator(id)->SetOperatorAtts(lineAtts);
    }

    plotList->RealizePlots();
}


// ***********************************************************************
//  Method: ViewerQuery::Update
//
//  Purpose:
//    This method is called when certains apects of the results Plot
//    are modified.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 26 15:45:13 PDT 2002
//    If lineout endpoints change, allow originatingWindow to update tools.
//
//    Kathleen Bonnell, Fri Jul 26 16:52:47 PDT 2002
//    Allow lineout to always update the visual cue (refline) in the
//    originatingwindow. 
//
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    If reflineLabels changes, allow originatingWindow to update the
//    refline. 
//
//    Kathleen Bonnell, Mon Nov  4 15:30:42 PST 2002   
//    Removed test of designators between temp and lineAtts.  This 
//    attribute is invisible in both curve and lineout gui, and sometimes 
//    does not get set/returned correctly during Set*AttsFromClient 
//    (especially for the Lineout operator).  
//    
//    Kathleen Bonnell, Wed Mar  5 08:24:55 PST 2003 
//    Update line endpoints via slice-plane attributes when they have 
//    changed.  
//    
// ***********************************************************************

void
ViewerQuery::Update(Subject *TheChangedSubject)  
{
    if (TheChangedSubject == resPlotQueryInfo)
    {
        int id;
        Line *temp;
        switch(resPlotQueryInfo->GetChangeType())
        {
            case PlotQueryInfo::PlotAtts: 
                 temp = (Line *)resultsPlot->GetPlotAtts()->
                        CreateCompatible("Line");
                 if (!temp)
                 {
                     debug5 << "ViewerQuery could not create LineAtts "
                            << "from PlotAtts!" << endl;
                     return;
                 }
                 lineAtts->SetColor(temp->GetColor());
                 lineAtts->SetLineStyle(temp->GetLineStyle());
                 lineAtts->SetLineWidth(temp->GetLineWidth());
                 originatingWindow->UpdateQuery(lineAtts);
                 delete temp;          
                 break;
            case PlotQueryInfo::OpAtts:
                 id = resultsPlot->GetNOperators() -1;
                 temp = (Line *)resultsPlot->GetOperator(id)->GetOperatorAtts()
                        ->CreateCompatible("Line");
                 lineAtts->SetInteractive(temp->GetInteractive());
                 //
                 //  If endpoints, or reflineLabels have changed, update the 
                 //  refline. 
                 //
                 if (!(lineAtts->FieldsEqual(0, temp)) ||
                     !(lineAtts->FieldsEqual(1, temp)) ||
                     (lineAtts->GetReflineLabels() != temp->GetReflineLabels()))
                 {
                     lineAtts->SetPoint1(temp->GetPoint1());
                     lineAtts->SetPoint2(temp->GetPoint2());
                     lineAtts->SetReflineLabels(temp->GetReflineLabels());
                     if (handlingTool)
                     {
                         originatingWindow->UpdateTools();
                     }
                     originatingWindow->UpdateQuery(lineAtts);
                 }
                 else // see if slice plane atts changed
                 { 
                     for (int i = 0; i < resultsPlot->GetNOperators()-1; i++)
                     {
                         ViewerOperator *oper = resultsPlot->GetOperator(i);
                         if (strcmp(oper->GetName(), "Slice") == 0)
                         {
                             PlaneAttributes *planeAtts = 
                                 (PlaneAttributes *)oper->GetOperatorAtts()->
                                 CreateCompatible("PlaneAttributes");
                             UpdateLineFromSlice(planeAtts);
                             delete planeAtts;
                             break;
                         }
                     }
                 }
                 delete temp;
                 break;
            default: break;
        }
    }
}


// ****************************************************************************
//  Method: ViewerQuery::MatchResultsPlot
//
//  Purpose:  Determine if the passed ViewerPlot matches the resultsPlot.
// 
//  Arguments:
//    vp        The plot with which to compare. 
//
//  Returns:
//    false if resultPlot is NULL or if  vp != resultsPlot, true otherwise
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
// ****************************************************************************

bool
ViewerQuery::MatchResultsPlot(ViewerPlot *vp) const
{
    return ((vp != 0)  && (vp == resultsPlot));
}


// ****************************************************************************
//  Method: ViewerQuery::MatchOriginatingPlot
//
//  Purpose:  Determine if the passed ViewerPlot matches the originatingPlot.
// 
//  Arguments:
//    vp        The plot with which to compare. 
//
//  Returns:
//    false if resultPlot is NULL or if  vp != originatingPlot, true otherwise
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
// ****************************************************************************

bool
ViewerQuery::MatchOriginatingPlot(ViewerPlot *vp) const
{
    return ((vp != 0)  && (vp == originatingPlot));
}


// ****************************************************************************
//  Method: ViewerQuery::MatchResultsWindow
//
//  Purpose:  Determine if the passed ViewerWindow matches the originatingWin.
// 
//  Arguments:
//    vw        The window with which to compare. 
//
//  Returns:
//    false if resultPlot is NULL or if  vp != originatingWin, true otherwise
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
// ****************************************************************************

bool
ViewerQuery::MatchResultsWindow(ViewerWindow *vw) const
{
    return ((vw != 0)  && (vw == resultsWindow));
}


// ****************************************************************************
//  Method: ViewerQuery::MatchOriginatingWindow
//
//  Purpose:  Determine if the passed ViewerWindow matches the originatingWin.
// 
//  Arguments:
//    vw        The window with which to compare. 
//
//  Returns:
//    false if resultPlot is NULL or if  vp != originatingWin, true otherwise
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
// ****************************************************************************

bool
ViewerQuery::MatchOriginatingWindow(ViewerWindow *vw) const
{
    return ((vw !=0)  && (vw == originatingWindow));
}


// ****************************************************************************
//  Method: ViewerQuery::DeleteOriginatingWindow
//
//  Purpose:  
//    Remove the reference to the originating window.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//    Kathleen Bonnell, Mon Jul 29 09:36:35 PDT 2002 
//    Replace call to InteractiveOff with the necessary code.
//
// ****************************************************************************

void
ViewerQuery::DeleteOriginatingWindow()
{
    StopObservingPlot();
    originatingWindow = 0;
    originatingPlot = 0;
}


// ****************************************************************************
//  Method: ViewerQuery::DeleteOriginatingPlot
//
//  Purpose:  
//    Remove the reference to the originating plot.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//    Kathleen Bonnell, Mon Jul 29 09:36:35 PDT 2002 
//    Replace call to InteractiveOff with the necessary code.
//
//    Kathleen Bonnell, Thu Jan 30 15:54:59 PST 2003  
//    Added call to DeleteVisualCue. 
//
// ****************************************************************************
void
ViewerQuery::DeleteOriginatingPlot()
{
    //
    //  If > 1 plot is in the originating window, then the reference lines
    //  will not be cleared, so ensure it happens here.
    //
    DeleteVisualCue();
    StopObservingPlot();
    originatingPlot = 0;
    originatingWindow = 0;
}


// ****************************************************************************
//  Method: ViewerQuery::DeleteVisualCue
//
//  Purpose:  
//    Remove the visual cue from originating window.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
// ****************************************************************************

void
ViewerQuery::DeleteVisualCue()
{
    if (originatingWindow)
    {
        originatingWindow->DeleteQuery(lineAtts); 
    }
}


// ****************************************************************************
//  Method: ViewerQuery::GetResultsWindow
//
//  Purpose:  
//    Retreives a pointer to the window where this query's results are drawn.
//
//  Returns:  
//    The results window.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   July 11, 2002 
//
// ****************************************************************************

ViewerWindow *
ViewerQuery::GetResultsWindow() const
{
    return resultsWindow;
}

// ****************************************************************************
//  Method: ViewerQuery::GetOriginatingWindow
//
//  Purpose:  
//    Retreives a pointer to the window that originated this query.
//
//  Returns:  
//    The originating window.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   January 16, 2003 
//
// ****************************************************************************

ViewerWindow *
ViewerQuery::GetOriginatingWindow() const
{
    return originatingWindow;
}

// ****************************************************************************
// Method: ViewerPlot::HandleTool
//
// Purpose: 
//   Applies the information from the tool to the stored attributes.
//
// Arguments:
//   ti : A reference to the tool interface.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 13, 2002 
//
// ****************************************************************************

bool
ViewerQuery::HandleTool(const avtToolInterface &ti)
{
    bool val = false;
    Line *temp = (Line *)ti.GetAttributes()->CreateCompatible("Line");
    if (temp && handlingTool)
    {
        if  (!(lineAtts->FieldsEqual(0, temp)) ||
             !(lineAtts->FieldsEqual(1, temp))) 
        {
            // Lineout is always last, get its index. 
            int index = resultsPlot->GetNOperators()-1;
            // Update lineout's atts with the tool atts. 
            val |= resultsPlot->GetOperator(index)->SetOperatorAtts(ti.GetAttributes());
            // Update this query's line atts with the tool atts.
            lineAtts->SetPoint1(temp->GetPoint1());
            lineAtts->SetPoint2(temp->GetPoint2());
            //  Make sure the refline updates, too.
            originatingWindow->UpdateQuery(lineAtts);
        }
        delete temp;
    }
    return val;
}

// ****************************************************************************
// Method: ViewerQuery::InitializeTool
//
// Purpose: 
//   Initializes the tool with all of the operator attributes.
//
// Arguments:
//   ti : The tool interface that we're initializing.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 13, 2002 
//   
// ****************************************************************************

bool
ViewerQuery::InitializeTool(avtToolInterface &ti)
{
    bool retval = false;

    // Lineout is always last.
    int index = resultsPlot->GetNOperators()-1;

    //
    // Try and convert the operator attributes object into a type that
    // is compatible with the tool. Note that we must do it like this
    // so the generic tool attributes do not need to know about the
    // operator attributes. This is important because the operator
    // attributes exist only in plugins.
    //
    std::string tname(ti.GetAttributes()->TypeName());
    const AttributeSubject *operatorAtts = resultsPlot->GetOperator(index)->GetOperatorAtts();
    AttributeSubject *atts = operatorAtts->CreateCompatible(tname);

    if(atts != 0 && ti.GetAttributes()->CopyAttributes(atts))
    {
        retval = true;
        delete atts;
        handlingTool = true;
    }
  
    return retval;
}


// ****************************************************************************
// Method: ViewerQuery::DisableTool
//
// Purpose: 
//   Tells this query that the tool has been turned off. 
//
// Programmer: Kathleen Bonnell 
// Creation:   July 13, 2002 
//   
// ****************************************************************************

void
ViewerQuery::DisableTool()
{
    handlingTool = false;
}

 
// ****************************************************************************
//  Method: ViewerQuery::CanHandleTool
//
//  Purpose:
//    Specifies whether this query is in a state to handle a tool.
//
//  Returns:
//    True if this query can handle a tool, false otherwise. 
//    The query must be in interactive mode and it must be the active plot
//    in order to handle a tool.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 13, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//    Account for multiple active plots.
//
// ****************************************************************************
 
bool
ViewerQuery::CanHandleTool()
{
    bool retval = false;
    if (lineAtts->GetInteractive())
    {
        // Retrieve the plot ID for the current active,non-hidden, realized 
        // plot in the plot list where my results are drawn.
        std::vector<int> plotIDs;
        resultsWindow->GetPlotList()->GetActivePlotIDs(plotIDs);
        int activePlotId = (plotIDs.size() > 0 ? plotIDs[0] : -1);

        //
        //  Does this plot match my results?
        retval = MatchResultsPlot(resultsWindow->GetPlotList()->
                                  GetPlot(activePlotId));
    }
    return retval;
}

// ****************************************************************************
//  Method: ViewerQuery::IsHandlingTool
//
//  Purpose:
//    Specifies whether this query is currently handling a tool. 
//
//  Returns:
//    True if this query is handling a tool, false otherwise. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 13, 2002
//
// ****************************************************************************

bool
ViewerQuery::IsHandlingTool()
{
    return handlingTool;    
}


// ****************************************************************************
//  Method: ViewerQuery::GetOriginatingPlot
//
//  Purpose:  
//    Retreives a pointer to the plot that originated this query. 
//
//  Returns:  
//    The originating plot.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   January 16, 2002 
//
// ****************************************************************************

ViewerPlot *
ViewerQuery::GetOriginatingPlot() const
{
    return originatingPlot;
}


// ****************************************************************************
//  Method: ViewerQuery::SendVisualCue
//
//  Purpose:  
//    Tells the window that originated this query to draw the refline.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
// ****************************************************************************

void
ViewerQuery::SendVisualCue()
{
    originatingWindow->ValidateQuery(NULL, lineAtts);
}


// ****************************************************************************
//  Method: ViewerQuery::ReCreateLineout
//
//  Purpose:  
//    Re-create the lineout without having to re-create the entire query.
//    May be called because the originating plot added/removed an operator. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Nov 18 17:56:00 PST 2004
//    Added  call to SendVisualCue.
// 
// ****************************************************************************

void
ViewerQuery::ReCreateLineout()
{
    StopObservingPlot();
    //  
    // Retrieve the ReflineLabels from Lineout Ops.
    //
    int id = resultsPlot->GetNOperators();
    Line *temp = (Line *)resultsPlot->GetOperator(id-1)->GetOperatorAtts()->
                   CreateCompatible("Line");
    lineAtts->SetReflineLabels(temp->GetReflineLabels()); 
    delete temp;
    resultsWindow->GetPlotList()->DeletePlot(resultsPlot, false);   
    CreateLineout();
    SendVisualCue();
    StartObservingPlot();
}


// ****************************************************************************
//  Method: ViewerQuery::UpdateLineFromSlice
//
//  Purpose:  
//    Updates line endpoints based on a change in slice-plane attributes.
//
//  Aguments:    
//    newPlaneAtts   The updated slice-plane attributes.
//
//  Returns:    
//    True if an update has occurred, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  8 20:27:53 PDT 2003 
//    Added conversions between 3d and 2d.
//
// ****************************************************************************

bool
ViewerQuery::UpdateLineFromSlice(PlaneAttributes *newPlaneAtts)
{
    // 
    // Only update the line if the slice-plane attributes have truly changed.
    // 
    if (*planeAtts == *newPlaneAtts)
        return false;

    int i, opId = -1;
    int lineId = resultsPlot->GetNOperators()-1;
    for (i = 0; i < lineId; i++)
    {
        ViewerOperator *oper = resultsPlot->GetOperator(i);
        if (strcmp(oper->GetName(), "Slice") == 0)
        {
            opId = i;
            break; 
        }
    }

    // 
    //  No need to update if no slice-operator has been applied.
    // 
    if (opId == -1) 
        return false;

    avtVector pt1(lineAtts->GetPoint1()); 
    avtVector pt2(lineAtts->GetPoint2()); 

    avtVector o1(planeAtts->GetOrigin());
    avtVector o2(newPlaneAtts->GetOrigin());
 
    // Define the frame for Plane 1.
    avtVector P1N(planeAtts->GetNormal());
    avtVector P1Up(planeAtts->GetUpAxis());
    avtVector u1, v1, w1;
    CreateBasis(P1N, P1Up, u1, v1, w1);

    // Define the frame for Plane 2.
    avtVector P2N(newPlaneAtts->GetNormal());
    avtVector P2Up(newPlaneAtts->GetUpAxis());
    avtVector u2, v2, w2;
    CreateBasis(P2N, P2Up, u2, v2, w2);

    avtMatrix M1, M2, M3, C;
    int spaceT = 0;
    if (planeAtts->GetThreeSpace() && !newPlaneAtts->GetThreeSpace())
    {
        // convert 3d to 2d
        avtVector zero(0., 0., 0.);
        avtMatrix C1, C2;
        C1.MakeFrameToCartesianConversion(u2, v2, w2, zero);

        C1.Inverse();
        C2.MakeScale(1, 1, 0);

        C = C2 * C1;
        spaceT = 1;
    }
    else if (!planeAtts->GetThreeSpace() && newPlaneAtts->GetThreeSpace())
    {
        // convert 2d to 3d
        avtVector zero(0., 0., 0.);
        avtMatrix C1, C2, C3;
        C1.MakeFrameToCartesianConversion(u1, v1, w1, zero);
        C2.MakeCartesianToFrameConversion(u1, v1, w1, zero);
        avtVector zdim = C1 * o1;
        zdim.x = 0; 
        zdim.y = 0; 
        zdim = C2 * zdim;
        C3.MakeTranslate(zdim.x, zdim.y, zdim.z);
        C = C3 * C1;
        spaceT = 2;
    }

    if ((!planeAtts->FieldsEqual(0, newPlaneAtts)) ||
        (!planeAtts->FieldsEqual(1, newPlaneAtts)) ||
        (!planeAtts->FieldsEqual(2, newPlaneAtts))) 
    {
        // Create conversion between Cartesian and plane1 frame.
        M1.MakeCartesianToFrameConversion(u1, v1, w1, o1);

        // Create conversion between plane2 and Cartesian frame.
        M2.MakeFrameToCartesianConversion(u2, v2, w2, o2);
        // Create composition matrix.
        switch (spaceT)
        {
            case 0 : M3 = M2 * M1;     break; // no dimensionality change
            case 1 : M3 = C * M2 * M1; break; // converted from 3d to 2d
            case 2 : M3 = M2 * M1 * C; break; // converted from 2d to 3d
        }
    }
    else
    {
        M3 = C;
    }

    // Convert points. 
    pt1 = M3 * pt1;
    pt2 = M3 * pt2;

    // Update necessary attributes.
    lineAtts->SetPoint1(pt1.x, pt1.y, pt1.z);
    lineAtts->SetPoint2(pt2.x, pt2.y, pt2.z);
    *planeAtts = *newPlaneAtts;

    // Ensure that both the slice and lineout operators have the correct atts. 
    resultsPlot->GetOperator(lineId)->SetOperatorAtts(lineAtts);
    resultsPlot->GetOperator(opId)->SetOperatorAtts(newPlaneAtts);

    // Update the refline. 
    originatingWindow->UpdateQuery(lineAtts);

    return true;
}


// ****************************************************************************
//  Method: CreateBasis
//
//  Purpose:  
//    Calculates a frame from a Normal and and UP vectors.  Recalculates the
//    UP vector if it is parallel to the Normal. 
//
//  Arguments:
//    N         The normal vector. 
//    UP        The up vector. 
//    u, v, w   A place to store the unit vectors that form the basis.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
// ****************************************************************************

void
CreateBasis(const avtVector &N, const avtVector &UP, 
        avtVector &u, avtVector &v, avtVector&w)
{
   w = N.normalized();
   u = (UP % w).normalized();
   if (u.norm() == 0)
   {
       avtVector origin(0., 0., 0.);
       avtVector y(0., 1., 0.);
       avtMatrix R; R.MakeRBT(origin, N, UP);
       y = R * y;
       u = (y % w).normalized();
   }
   v = (w % u).normalized();
}


// ****************************************************************************
//  Method: ViewerQuery::MatchTimeState
//
//  Purpose:  Determine if the passed timestate matches the resultsPlot's.
// 
//  Arguments:
//    ts        The timestate in question.
//
//  Returns:
//    false if resultPlot is NULL or if  ts != resultsPlot's cache index, 
//    true otherwise
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 3, 2005 
//
// ****************************************************************************

bool
ViewerQuery::MatchTimeState(int ts) const
{
    if (!resultsPlot)
        return false;
    return (resultsPlot->GetCacheIndex() == ts);
}


// ****************************************************************************
//  Method: ViewerQuery::SetFollowsTime
//
//  Purpose:  Sets the follows time flag for the results plot. 
// 
//  Arguments:
//    bool    The new value. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 3, 2005 
//
// ****************************************************************************

void
ViewerQuery::SetFollowsTime(bool val)
{
    if (resultsPlot)
        resultsPlot->SetFollowsTime(val);
}

// ****************************************************************************
//  Method: ViewerQuery::UpdateResultsTime
//
//  Purpose:  Sets the cache index for the results plot. 
// 
//  Arguments:
//    ts        The new time state. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 21, 2006
//
// ****************************************************************************

void
ViewerQuery::UpdateResultsTime(int ts)
{
    if (resultsPlot)
    {
        resultsPlot->SetCacheIndex(ts);
    }
}

