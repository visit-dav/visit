// ************************************************************************* //
//                              ViewerQuery.C                                //
// ************************************************************************* //

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <ViewerQuery.h>


#include <ViewerAnimation.h>
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
// ***********************************************************************

ViewerQuery::ViewerQuery(ViewerWindow *origWin, ViewerWindow *resWin, 
                         Line *lA) : SimpleObserver()
{
    resPlotQueryInfo = 0;
    width = height = 0.;
    planeAtts = 0;

    lineAtts = new Line();
    originatingWindow = origWin;
    resultsWindow = resWin;
    lineAtts->CopyAttributes(lA);
    CreateLineout();

    //  
    // Retrieve the interactivity from Lineout Ops.
    //
    int id = resultsPlot->GetNOperators();
    Line *temp = (Line *)resultsPlot->GetOperator(id-1)->GetOperatorAtts()->
                   CreateCompatible("Line");
    lineAtts->SetInteractive(temp->GetInteractive()); 
    lineAtts->SetReflineLabels(temp->GetReflineLabels()); 
    //
    // Ensure that lineAtts sent to VisWindow will have the correct value
    // for ReflineLabels. 
    //
    lA->SetReflineLabels(temp->GetReflineLabels()); 
    delete temp;

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
// ****************************************************************************

void
ViewerQuery::CreateLineout()
{
    //
    //  Grab information from the originating window.
    //
    ViewerPlotList *origList = originatingWindow->GetAnimation()->GetPlotList();
    int origPlotID = origList->GetPlotID();
    originatingPlot = origList->GetPlot(origPlotID);
    int currentFrame = originatingWindow->GetAnimation()->GetFrameIndex();
    int nFrames = originatingWindow->GetAnimation()->GetNFrames();
    const char *vName;
    if (strcmp(lineAtts->GetVarName().c_str(), "default") == 0)
        vName  = originatingPlot->GetVariableName();
    else 
        vName  = lineAtts->GetVarName().c_str();
    std::string hdbName(originatingPlot->GetHostName());
    hdbName += ":";
    hdbName += originatingPlot->GetDatabaseName();
    bool replacePlots  = ViewerWindowManager::Instance()->GetClientAtts()->GetReplacePlots();
 
    int plotType = PlotPluginManager::Instance()->GetEnabledIndex("Curve_1.0");
    ViewerPlotList *plotList =  resultsWindow->GetAnimation()->GetPlotList();
    resultsWindow->GetAnimation()->SetNFrames(nFrames);
    resultsWindow->GetAnimation()->SetFrameIndex(currentFrame);
    plotList->SetHostDatabaseName(hdbName.c_str());
 
    int pid = plotList->AddPlot(plotType, vName, replacePlots, false);
    resultsPlot = plotList->GetPlot(pid);
 
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
    resultsPlot->AddOperator(operatorType);

    //
    // Update the view for the new window.
    //

    int ts = originatingWindow->GetAnimation()->GetFrameIndex();
    int spatDim = originatingPlot->GetSpatialDimension(ts);
    double *bounds = originatingPlot->GetSpatialExtents(ts);
    double *range = originatingPlot->GetDataExtents(ts);


    //
    // Width and height are used for determining how to scale the y-axis on a 
    // curve plot.  Presumably, consecutive lineouts will be from the same 
    // originating plot, so the max width of the curve plot is ~= the longest 
    // line that could be drawn through the plot: the diagonal from the 
    // minBounds to maxBounds (ignoring zoomed plots).
    // The maximum height of the Curve will be the Scalar range of the variable 
    // being plotted.
    // 
    width = 0., height = 0.;
    for (int i = 0; i < spatDim; i++)
    {
        width += ((bounds[2*i+1] - bounds[2*i]) *
                  (bounds[2*i+1] - bounds[2*i]));
    }
    width = sqrt(width);
    //
    //  Lineout restricted to scalar-variables, so data dim should be 2!
    //
    height = range[1] - range[0];

    //
    //  Allocated during GetSpatialExtents/GetDataExtents
    delete [] bounds;
    delete [] range;
 
    //
    //  The client atts were already copied created,
    //  so now update them with the lineAtts.
    //
    resultsPlot->SetPlotAtts(0, lineAtts);
    int id = resultsPlot->GetNOperators() -1;
    resultsPlot->GetOperator(id)->SetOperatorAtts(lineAtts);

    plotList->RealizePlots();
    SendVisualCue();
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
                 //  If endpoints, or reflineLabels have changed, update the refline. 
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
                             PlaneAttributes *planeAtts = (PlaneAttributes *)oper-> 
                                 GetOperatorAtts()->CreateCompatible("PlaneAttributes");

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
//  Method: ViewerQuery::GetWidth
//
//  Purpose:  
//    Retreives the width of this query.
//
//  Returns:  
//    The width. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   July 11, 2002 
//
// ****************************************************************************

double
ViewerQuery::GetWidth() const
{
    return width;
}


// ****************************************************************************
//  Method: ViewerQuery::GetHeight
//
//  Purpose:  
//    Retreives the height of this query.
//
//  Returns:  
//    The height. 
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   July 11, 2002 
//
// ****************************************************************************

double
ViewerQuery::GetHeight() const
{
    return height;
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
// ****************************************************************************
 
bool
ViewerQuery::CanHandleTool()
{
    bool retval = false;
    if (lineAtts->GetInteractive())
    {
        // Retrieve the plot ID for the current active,non-hidden, realized plot in
        // the plot list where my results are drawn.

        int activePlotId = resultsWindow->GetAnimation()->GetPlotList()->GetPlotID();
        //
        //  Does this plot match my results?
        retval = MatchResultsPlot(resultsWindow->GetAnimation()
                 ->GetPlotList()->GetPlot(activePlotId));
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
// ****************************************************************************

void
ViewerQuery::ReCreateLineout()
{
    StopObservingPlot();
    resultsWindow->GetAnimation()->GetPlotList()->DeletePlot(resultsPlot, false);   
    CreateLineout();
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

    avtMatrix M1, M2, M3;

    // Create conversion between Cartesian and plane1 frame.
    M1.MakeCartesianToFrameConversion(u1, v1, w1, o1);

    // Create conversion between plane2 and Cartesian frame.
    M2.MakeFrameToCartesianConversion(u2, v2, w2, o2);

    // Create composition matrix.
    M3 = M2 * M1;

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



