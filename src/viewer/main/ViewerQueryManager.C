// ************************************************************************* //
//                          ViewerQueryManager.C                             //
// ************************************************************************* //

#include <stdio.h>
#include <math.h>
#include <float.h>
#include <snprintf.h>
#include <string>

#include <ViewerQueryManager.h>

#include <avtColorTables.h>
#include <avtDatabaseMetaData.h>
#include <avtToolInterface.h>
#include <avtTypes.h>
#include <DebugStream.h>
#include <Line.h>
#include <LineoutListItem.h>
#include <LostConnectionException.h>
#include <NoEngineException.h>
#include <OperatorPluginManager.h>
#include <PickAttributes.h>
#include <PlotPluginManager.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <ViewerActionManager.h>
#include <ViewerAnimation.h>
#include <ViewerEngineManager.h>
#include <ViewerExpressionList.h>
#include <ViewerFileServer.h>
#include <ViewerMessaging.h>
#include <ViewerOperator.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerQuery.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <GlobalLineoutAttributes.h>
#include <VisItException.h>

using std::vector;
using std::string;

//
// Storage for static data elements.
//

ViewerQueryManager *ViewerQueryManager::instance = 0;   

QueryAttributes *ViewerQueryManager::queryClientAtts=0;
PickAttributes *ViewerQueryManager::pickAtts=0;
GlobalLineoutAttributes *ViewerQueryManager::globalLineoutAtts=0;
GlobalLineoutAttributes *ViewerQueryManager::globalLineoutClientAtts=0;


// ****************************************************************************
//  Method: ViewerQueryManager constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Brad Whitlock, Fri Sep 6 14:06:57 PST 2002
//    I added the queryTypes member.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002 
//    Add Revolved volume to query list, comment out currently unsupported
//    queries WorldPick, WorldLineout and MinMax. 
//
//    Kathleen Bonnell, Fri Nov 15 17:44:44 PST 2002  
//    Add Eulerian to query list. 
//
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002  
//    Enable WorldLineout in the query list.  Initialize new members
//    baseDesignator, cycleDesignator, designator.
//
//    Kathleen Bonnell, Mon Dec 23 13:19:38 PST 2002
//    Removed ScreenSpace designation from Lineout query.  Removed WorldLineout
//    from query list.
//
//    Kathleen Bonnell, Tue Feb 25 17:11:18 PST 2003
//    Check for loaded Curve and Lineout plugins before adding Lineout to the
//    query list. 
//    
//    Jeremy Meredith, Fri Feb 28 12:36:21 PST 2003
//    Made it use PluginAvailable instead of PluginLoaded so that it could
//    attempt to load the plugins on demand.
//
//    Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003  
//    Store Lineout queries (ViewerQuery) in LineoutListItem.
//
//    Brad Whitlock, Wed Mar 12 14:41:24 PST 2003
//    I initialized the operatorFactory member.
//
//    Hank Childs, Tue Mar 18 21:33:09 PST 2003
//    Added revolved surface area.
//
//    Kathleen Bonnell, Wed Mar 26 14:37:23 PST 2003 
//    Initialize preparingPick, handlingCache, initialPick. 
//
//    Jeremy Meredith, Sat Apr 12 11:31:53 PDT 2003
//    Added compactness queries.
//
// ****************************************************************************

ViewerQueryManager::ViewerQueryManager()
{
    lineoutList    = 0;
    nLineouts      = 0;
    nLineoutsAlloc = 0;
    colorIndex     = 0;

    baseDesignator = 'A';
    cycleDesignator = false;
    designator = new char[4];
    SNPRINTF(designator, 4, "%c", baseDesignator);

    // Create the query list.
    queryTypes = new QueryList;
#if 1
    // Kathleen - populate this somewhere.
    queryTypes->AddQuery("Pick", QueryList::PointQuery, QueryList::ScreenSpace);
    if (PlotPluginManager::Instance()->PluginAvailable("Curve_1.0") &&
        OperatorPluginManager::Instance()->PluginAvailable("Lineout_1.0")) 
    {
        queryTypes->AddQuery("Lineout", QueryList::LineQuery);
    }
    queryTypes->AddQuery("Eulerian", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Compactness", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Revolved volume", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Revolved surface area", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Surface area", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Volume", QueryList::DatabaseQuery);
    //queryTypes->AddQuery("MinMax", QueryList::DatabaseQuery);
    //queryTypes->AddQuery("WorldPick", QueryList::PointQuery);
#endif
    queryTypes->SelectAll();

    operatorFactory = 0;


    initialPick = false;
    preparingPick = false;
    handlingCache = false;
}


// ****************************************************************************
//  Method: ViewerQueryManager::Instance
//
//  Purpose:
//    Return a pointer to the sole instance of the ViewerQueryManager
//    class.
//
//  Returns:    A pointer to the sole instance of the ViewerQueryManager
//              class.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
// ****************************************************************************
 
ViewerQueryManager *
ViewerQueryManager::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new ViewerQueryManager;
    }
 
    return instance;
}


// ****************************************************************************
//  Method: ViewerQueryManager destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Brad Whitlock, Fri Sep 6 14:10:42 PST 2002
//    I added the queryTypes member.
//
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002  
//    Delete designator. 
//
// ****************************************************************************

ViewerQueryManager::~ViewerQueryManager()
{
    int i;
    //
    // Delete the list and any queries in the queries list.
    //
    if (nLineoutsAlloc > 0)
    {
        for (i = 0; i < nLineouts; i++)
        {
            delete lineoutList[i];
        }
        delete [] lineoutList;
    }

    delete queryTypes;
    delete [] designator;
}

// ****************************************************************************
// Method: ViewerQueryManager::SetOperatorFactory
//
// Purpose: 
//   Sets the query manager's operator factory pointer.
//
// Arguments:
//   factory : A pointer to the operator factory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 15:33:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::SetOperatorFactory(ViewerOperatorFactory *factory)
{
    operatorFactory = factory;
}

// ****************************************************************************
//  Method: ViewerQueryManager::AddQuery
//
//  Purpose:
//    Add a query to the query list.
//
//  Arguments:
//    origWin   A pointer to the window that originated the query. 
//    lineAtts  The Line attributes to be used for the query.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002 
//    Added call to UpdateScaleFactor.
//
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002  
//    Changed argument to ValidateQuery from color to Line*, to convey more
//    information. 
//
//    Brad Whitlock, Wed Nov 20 13:28:18 PST 2002
//    Changed how colors are selected.
//
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002   
//    Added calls to set the designator for lineAtts, and to UpdateDesignator.
//    Added argument to ValidateQuery call.
//
//    Brad Whitlock, Thu Feb 27 14:55:42 PST 2003
//    I added code to update the actions so the toolbars and menus in the
//    new window get updated properly.
//
//    Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003 
//    Updated to use ViewerQuery_p, added more args to SimpleAddQuery. 
//    Removed call to ValidateQuery, now handled in ViewerQuery.
//    
//    Kathleen Bonnell, Fri Mar 14 17:11:42 PST 2003 
//    Added test of variable passed in LineAtts. 
//    
//    Kathleen Bonnell, Wed Apr 23 11:38:47 PDT 2003 
//    Allow MATSPECIES var type. 
//    
// ****************************************************************************

void
ViewerQueryManager::AddQuery(ViewerWindow *origWin, Line *lineAtts)
{
    int plotId = origWin->GetAnimation()->GetPlotList()->GetPlotID(); 
    //
    // Is there an active non-hidden plot in the originating window? 
    //
    if (plotId == -1)
    {
        string msg("Lineout requires an active non-hidden Plot.\n");
        msg += "Please select a plot and try again.\n";
        Error(msg.c_str());
        return;
    }
    //
    // Is there a valid variable? 
    //
    ViewerPlot *oplot = origWin->GetAnimation()->GetPlotList()->GetPlot(plotId); 
    const char *hname = oplot->GetHostName();
    const char *dname = oplot->GetDatabaseName();
    const char *vname = lineAtts->GetVarName().c_str();
    if (strcmp(vname, "default") == 0)
        vname = oplot->GetVariableName();
    int varType = DetermineVarType(hname, dname, vname);
    if (varType != AVT_SCALAR_VAR &&
        varType != AVT_MATSPECIES) 
    {
        char message[100];
        SNPRINTF(message, 100, "Lineout requires scalar variable.  %s is not scalar.", vname);
        Error(message);
        return;
    }
    //
    // Can we get a lineout window? 
    //
    ViewerWindow *resWin = ViewerWindowManager::Instance()->GetLineoutWindow();
    if (resWin == NULL)
    {
        return;
    }

    //
    // Set the color to use for the query.
    //
    lineAtts->SetColor(GetColor());
    lineAtts->SetDesignator(designator);


    ViewerQuery_p newQuery = new ViewerQuery(origWin, resWin, lineAtts);
    if(*newQuery == NULL)
    {
        Error("VisIt could not create the desired plot.");
        return;
    }

    //
    // Add the new query to the lineout list.
    //
    SimpleAddQuery(newQuery, oplot, origWin, resWin);

    UpdateScaleFactor(resWin);
    UpdateDesignator();

    //
    // Update the actions so the menus and the toolbars have the right state.
    //
    ViewerWindowManager::Instance()->UpdateActions();

    // Brad Whitlock, Fri Apr 4 15:24:28 PST 2003
    // Do it again because for some reason, even though the curve vis window
    // gets the right values, the actual toolbar and popup menu are not
    // updating. If we ever figure out the problem, remove this code.
    resWin->GetActionManager()->UpdateSingleWindow();
}


// ****************************************************************************
// Method: ViewerQueryManager::SimpleAddQuery
//
// Purpose: 
//   Adds the query to the lineout list.  Expand the list if necessary.
//
// Arguments:
//   query     The created query.
//   oplot     The plot that originated the query. 
//   owin      The window that originated the query. 
//   rwin      The window where the results will be drawn. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 10, 2002 
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 31 16:43:43 PDT 2002 
//   Fix bad argument to sizeof for memcpy.
//
//   Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003  
//   Reworked to reflect that lineout queries stored in LineoutList.
//
// ****************************************************************************

void
ViewerQueryManager::SimpleAddQuery(ViewerQuery_p query, ViewerPlot *oplot, 
                                   ViewerWindow *owin, ViewerWindow *rwin)
{
    int i, index = -1;

    //
    //  Determine the correct list.
    //
    for (i = 0; i < nLineouts && index == -1; i++)
    {
        if (lineoutList[i]->Matches(oplot, owin, rwin))
        {
            index = i;
        }
    }
    if (index == -1)
    {
        if (nLineouts >= nLineoutsAlloc)
        {
            //
            // Expand the lineout list if necessary.
            //
            LineoutListItem **lineoutsNew=0;

            nLineoutsAlloc += 2;
            lineoutsNew= new LineoutListItem*[nLineoutsAlloc];
            for (i = 0; i < nLineoutsAlloc; i++)
            {
                lineoutsNew[i] = NULL; 
            }

            if (nLineouts > 0)
            {
                for (i = 0; i < nLineouts; i++)
                {
                    lineoutsNew[i] = new LineoutListItem(*(lineoutList[i]));
                    delete lineoutList[i];
                }
                delete [] lineoutList;
            }

            lineoutList = lineoutsNew;
        }
        index = nLineouts;
        LineoutListItem loli(oplot, owin, rwin);
        if (lineoutList[index] == NULL)
        {
            lineoutList[index] = new LineoutListItem(loli);
        }
        else
        {
            *(lineoutList[index]) = loli;
        }
        //
        // Observe the originating plot if necessary. 
        //
        if (GetGlobalLineoutAtts()->GetDynamic())
        {
            lineoutList[index]->ObserveOriginatingPlot();
        }
        nLineouts++;
    }

    //
    // Add the query to the correct list.
    //
    lineoutList[index]->AddQuery(query);
}


// ****************************************************************************
//  Method: ViewerQueryManager::Delete
//
//  Purpose:
//    Handles pointer-referencing issues when the originating or 
//    results plot of a query are deleted. 
//
//  Arguments:
//    vp        A pointer to the ViewerPlot about to be deleted.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002 
//    Added call to UpdateScaleFactor.
//
//   Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003  
//   Reworked to reflect that lineout queries stored in LineoutList.
//
// ****************************************************************************

void
ViewerQueryManager::Delete(ViewerPlot *vp)
{
    if (nLineouts == 0)
    {
        return;
    }

    bool needPhysicalDelete = true;
    int i;
    for (i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingPlot(vp))
        {
            lineoutList[i]->DeleteOriginatingPlot();
            needPhysicalDelete = false;
        }
    }
 
    if (!needPhysicalDelete)
    {
        return;
    }

    //
    // Delete the query whose resultsPlot == vp; 
    //
    int nLineoutsNew = nLineouts;
    ViewerWindow *resWin = 0; 
    //
    //  There should be only one match, so stop once we found it.
    //
    for (i = 0;  i < nLineouts && resWin == 0; ++i)
    {
        if (lineoutList[i]->DeleteResultsPlot(vp))
        {
            resWin = lineoutList[i]->GetResultsWindow();
            if (lineoutList[i]->IsEmpty())
            {
                nLineoutsNew--;
                if (nLineoutsNew > 0 )
                {
                    *(lineoutList[i]) = *(lineoutList[nLineoutsNew]);
                }
                delete lineoutList[nLineoutsNew];
                lineoutList[nLineoutsNew] = NULL;
            }
        }
    }
    nLineouts = nLineoutsNew;
    if (resWin)
    {
        UpdateScaleFactor(resWin);
    }
} 


// ****************************************************************************
//  Method: ViewerQueryManager::Delete
//
//  Purpose:
//    Handles pointer-referencing issues when the originating or 
//    results window of a query are deleted. 
//
//  Arguments:
//    vw        A pointer to the ViewerWindow about to be deleted.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 10, 2002 
//
//   Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003  
//   Reworked to reflect that lineout queries stored in LineoutList.
//
//   Kathleen Bonnell, Fri Mar  7 16:27:04 PST 2003 
//   Removed call to lineoutList->DeleteResultsWindow (redundant since
//   the list item will be deleted anyway).  Set lineoutList[i] to NULL
//   after delete. 
//
// ****************************************************************************

void
ViewerQueryManager::Delete(ViewerWindow *vw)
{
    if (nLineouts == 0)
    {
        return;
    }

    bool needPhysicalDelete = true;
    int i;
    for (i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingWindow(vw))
        {
            lineoutList[i]->DeleteOriginatingWindow();
            needPhysicalDelete = false;
        }
    }
 
    if (!needPhysicalDelete)
    {
        return;
    }

    //
    // Delete the query whose resultsWindow == vw; 
    //
    int nLineoutsNew = 0;
    for (i = 0;  i < nLineouts; i++)
    {
        if (!(lineoutList[i]->MatchResultsWindow(vw)))
        {
            lineoutList[nLineoutsNew] = lineoutList[i];
            nLineoutsNew++;
        }
    }
    for (i = nLineoutsNew; i < nLineouts; i++)
    {
        delete lineoutList[i];
        lineoutList[i] = NULL;
    }
    nLineouts = nLineoutsNew;
} 


// ****************************************************************************
//  Method: ViewerQueryManager::UpdateScaleFactor
//
//  Purpose:
//    Determines what scale factor should be used to make the curves
//    look nice in the vis window.  Uses the max width/height from
//    all queries in the specified window in making the computation. 
//    This method is called when new lineouts are added or removed.
//
//  Arguments:
//    vw        A pointer to the ViewerWindow that should have its curves
//              scaled.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002
//    Only update ViewCurve and scale plots if we've found a match
//    to the passed window.
//
// ****************************************************************************

void
ViewerQueryManager::UpdateScaleFactor(ViewerWindow *vw) 
{
    if (nLineouts == 0)
    {
        return;
    }
 
    double width  = -DBL_MAX;
    double height = -DBL_MAX;
    double scale  = 1.;
    int i;
    double w, h;
    for (i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchResultsWindow(vw))
        {
            w = lineoutList[i]->GetMaxQueryWidth();
            h = lineoutList[i]->GetMaxQueryHeight();
            width  = (width  > w ? width : w);
            height = (height > h ? height : h);
        }
    }

    if (height > 0. && width > 0.)  // we found a match
    {
        scale =  width / height;
        avtViewCurve viewCurve= vw->GetViewCurve();
        float vec[3] = {1., scale, 1.};
        vw->ScalePlots(vec);
        viewCurve.yScale = scale;
        vw->SetViewCurve(viewCurve);
        vw->ResetView();
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::HandleTool
//
//  Purpose:
//    Handles a request by a tool to change plot attributes.
//
//  Arguments:
//    ow : The window where the tool is active. 
//    ti : A reference to a tool interface object.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 13, 2002
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar  4 13:36:54 PST 2003  
//    Have lineout lists handle the tool.
//
// ****************************************************************************
 
void
ViewerQueryManager::HandleTool(ViewerWindow *oWin, const avtToolInterface &ti)
{
    int plotId = oWin->GetAnimation()->GetPlotList()->GetPlotID();
    if ((nLineouts == 0) || (plotId == -1)) 
    {
        return;
    }
    
    ViewerPlot *oPlot = oWin->GetAnimation()->GetPlotList()->GetPlot(plotId);
    for (int i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingPlot(oPlot))
        {
            lineoutList[i]->HandleTool(ti);
        }
    }
}


// ****************************************************************************
// Method: ViewerQueryManager::InitializeTool
//
// Purpose:
//   Initializes a tool as it is being turned on.
//
// Arguments:
//   ow : The window where the tool is being turned on. 
//   ti : The tool interface that we're going to use for initialization.
//
// Returns: Whether or not the tool was initialized.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 20, 2002 
//
// Modifications:
//    Kathleen Bonnell, Tue Mar  4 13:36:54 PST 2003  
//    Have lineout lists handle the tool.
//
// ****************************************************************************
 
bool
ViewerQueryManager::InitializeTool(ViewerWindow *oWin, avtToolInterface &ti)
{
    if (ti.GetAttributes()->TypeName() != "Line")
    {
        return false;
    }

    int plotId = oWin->GetAnimation()->GetPlotList()->GetPlotID();
    if ((nLineouts == 0) || (plotId == -1))
    {
        return false;
    }

    bool retval = false;
    ViewerPlot *oPlot = oWin->GetAnimation()->GetPlotList()->GetPlot(plotId);
    for (int i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingPlot(oPlot)) 
        {
            retval = lineoutList[i]->InitializeTool(ti);
            break;
        }
    }

    return retval;
}


// ****************************************************************************
// Method: ViewerQueryManager::DisableTool
//
// Purpose:
//   Tell the queries that the tool has been turned off.
//
// Arguments:
//   ow : The window where the tool was active. 
//   ti : The tool interface from the tool that is being turned off, 
//
// Programmer: Kathleen Bonnell 
// Creation:   July 13, 2002 
//
// Modifications:
//    Kathleen Bonnell, Tue Mar  4 13:36:54 PST 2003  
//    Have lineout lists handle the tool.
//
// ****************************************************************************
 
void
ViewerQueryManager::DisableTool(ViewerWindow *oWin, avtToolInterface &ti)
{
    if (ti.GetAttributes()->TypeName() != "Line")
    {
        return;
    }

    for (int i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingWindow(oWin)) 
        {
            lineoutList[i]->DisableTool();
        }
    }
}

 
// ****************************************************************************
//  Method: ViewerQueryManager::GetQueryClientAtts
//
//  Purpose:
//    Returns a pointer to the query attributes.
//
//  Returns:    A pointer to the query attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 16, 2002 
//
// ****************************************************************************
 
QueryAttributes *
ViewerQueryManager::GetQueryClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (queryClientAtts == 0)
    {
        queryClientAtts = new QueryAttributes;
    }
 
    return queryClientAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::DatabaseQuery
//
//  Purpose:
//    Perform a one-off query, one that does not need to be stored in the
//    query list.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 16, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 23 18:10:26 PDT 2002     
//    Add specific handling of certain exceptions.
// 
//    Kathleen Bonnell, Wed Feb 26 12:47:55 PST 2003 
//    Catch NoEngine and LostConnection Exceptions, and retry the query
//    after the queried plot re-executes. 
//    
// ****************************************************************************

void         
ViewerQueryManager::DatabaseQuery(ViewerWindow *oWin, const string &qName,
                            const vector<string> &vars)
{
    ViewerPlotList *olist = oWin->GetAnimation()->GetPlotList();
    int plotId = olist->GetPlotID();
    //
    // Is there an active non-hidden plot in the originating window?
    //
    if (plotId == -1)
    {
        string msg(qName);
        msg += " requires an active non-hidden Plot.\n";
        msg += "Please select a plot and try again.\n";
        Error(msg.c_str());
        return;
    }

    ViewerPlot *oplot = olist->GetPlot(plotId);
    const char *host = oplot->GetHostName();
    bool retry;
    int numAttempts = 0;
    do
    {
        retry = false;
        int networkId = oplot->GetNetworkID();

        TRY
        { 
            ViewerEngineManager *eM = ViewerEngineManager::Instance();
            QueryAttributes qa;
            qa.SetName(qName);
            qa.SetVariables(vars);
            if (eM->Query(host, networkId, &qa, qa))
            {
               *queryClientAtts = qa;
                queryClientAtts->Notify();
                Message(qa.GetResultsMessage().c_str());
            }
            else
            {
                char message[500];
                SNPRINTF(message, 500, "VisIt could not satisfy the query %s", 
                        qName.c_str());
                Error(message);
            }

        }
        CATCH(NoEngineException)
        {
            // Queries access the cached network used by the queried plot.
            // Simply relaunching the engine does not work, as no network
            // is created. This situation requires re-execution of the 
            // plot that is being queried.
            int curFrame = oWin->GetAnimation()->GetFrameIndex();
            oplot->ClearActors(curFrame, curFrame);
            oWin->GetAnimation()->UpdateFrame(); 
            retry = true;
            numAttempts++; 
        }
        CATCH(LostConnectionException)
        {
            // Queries access the cached network used by the queried plot.
            // Simply relaunching the engine does not work, as no network
            // is created. This situation requires re-execution of the 
            // plot that is being queried.
            int curFrame = oWin->GetAnimation()->GetFrameIndex();
            oplot->ClearActors(curFrame, curFrame);
            oWin->GetAnimation()->UpdateFrame(); 
            retry = true;
            numAttempts++; 
        }
        CATCH2(VisItException, e)
        {
            char message[500];

            if (e.GetExceptionType() == "InvalidDimensionsException")
            {
                //
                //  Create message for the gui that includes the query name
                //  and message.
                //
                SNPRINTF(message, 500, "%s:  %s", qName.c_str(),
                         e.GetMessage().c_str());
            }
            else if (e.GetExceptionType() == "NonQueryableInputException")
            {
                //
                //  Create message for the gui that includes the plot name
                //  and message.
                //
                SNPRINTF(message, 500, "%s%s",
                         "The currently active plot is non-queryable.\n",
                         "Please select a different plot and try again.");
            }
            else
            {
                //
                // Add as much information to the message as we can,
                // including plot name, exception type and exception
                // message.
                //
                SNPRINTF(message, 500, "%s:  (%s)\n%s", qName.c_str(),
                         e.GetExceptionType().c_str(),
                         e.GetMessage().c_str());

            }
            Error(message);
        }
        ENDTRY
    } while (retry && numAttempts < 2);
}


// ****************************************************************************
//  Method: ViewerQueryManager::LineQuery
//
//  Purpose:
//    Perform a line query.
//
//  Arguments:
//    qName     The name of this query.
//    pt1       The first endpoint of the line. 
//    pt2       The second endpoint of the line. 
//    vars      The variables to be used for this query.
//    
//  Programmer: Kathleen Bonnell
//  Creation:   December 20, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec 23 13:19:38 PST 2002
//    Removed Screen-space lineout, renamed WorldLineout to Lineout.
// 
//    Kathleen Bonnell, Fri Mar 14 17:11:42 PST 2003  
//    Allow multiple-vars.
// 
// ****************************************************************************

void         
ViewerQueryManager::LineQuery(const char *qName, const double *pt1, 
                    const double *pt2, const vector<string> &vars)
{
    if (strcmp(qName, "Lineout") == 0)
    {
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        for (int i = 0; i < vars.size(); i++)
            Lineout(win, pt1, pt2, vars[i]);
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetPickAtts
//
//  Purpose:
//    Returns a pointer to the pick attributes.
//
//  Returns:    A pointer to the pick attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002 
//
// ****************************************************************************

PickAttributes *
ViewerQueryManager::GetPickAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (pickAtts == 0)
    {
        pickAtts = new PickAttributes;
    }
 
    return pickAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::ClearPickPoints
//
//  Purpose:
//    Notifies any observers of PickAttributes that pick points should be 
//    cleared.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002 
//
// ****************************************************************************

void
ViewerQueryManager::ClearPickPoints()
{
   // tell any observers that the pick points should be cleared
   pickAtts->SetClearWindow(true);
   pickAtts->SetFulfilled(false);
   pickAtts->Notify();
   // reset the clear window flag.
   pickAtts->SetClearWindow(false);
   pickAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerQueryManager::Pick
//
//  Purpose:
//    Perform a Pick with the passed information.
//
//  Arguments:
//    pd        The struct containing picked point and window information.
//
//  Notes:      Created from old ViewerWindow::PerformPickCallback and 
//              ViewerPlot::Pick.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec  6 16:25:20 PST 2002
//    Set pickAtts active variable, and set nodes to an empty int vector.
//
//    Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002    
//    Set pickAtts pickLetter from new member designator.  Added call to
//    UpdateDesignator.  Added argument to ValidateQuery call.
//
//    Kathleen Bonnell, Mon Dec 30 09:50:28 PST 2002  
//    Moved call to UpdateDesignator to after the query is validated, 
//    so that VisWin gets the correct designator with which to label the pick. 
//    
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003   
//    Pick now uses ray endpoints (instead of computed pick point). Add
//    pickAtts to ValidateQuery call. 
//    
//    Kathleen Bonnell, Wed Feb 26 12:47:55 PST 2003 
//    Catch NoEngine and LostConnection Exceptions, and retry the pick
//    after the queried plot re-executes. 
//    
//    Kathleen Bonnell, Wed Feb 26 12:47:55 PST 2003 
//    If this is the first pick, re-execute pipeline if necessary (via
//    ViewerPlotList::StartPick).  Cache incoming pick information if 
//    the pipeline is re-executing, and handle when re-execution completed.
//
//    Kathleen Bonnell, Thu Apr 17 08:54:59 PDT 2003 
//    This is the wrong place to set pick's dimension var. 
//    
//    Kathleen Bonnell, Fri Jun  6 16:06:25 PDT 2003 
//    Added support for full-frame mode. 
//    
// ****************************************************************************

void
ViewerQueryManager::Pick(PICK_POINT_INFO *ppi)
{
    //
    //  Keep local copy, due to caching issues.
    // 
    PICK_POINT_INFO pd = *ppi;
    if (preparingPick)
    {
        debug5 << "Caching pick point info." << endl;
        pickCache.push_back(pd);
        return;
    }
    if (initialPick)
    {
        preparingPick = true;
        ViewerWindow *win = (ViewerWindow *)pd.callbackData;
        ViewerPlotList *plist = win->GetAnimation()->GetPlotList();
        int t = win->GetAnimation()->GetFrameIndex();
        plist->StartPick(t);
        initialPick = false;
        preparingPick = false;
    }
    if (pd.validPick)
    {
        string msg;
        pickAtts->ClearPickVarInfos();
        ViewerWindow *win = (ViewerWindow *)pd.callbackData;

        ViewerPlotList *plist = win->GetAnimation()->GetPlotList();
        int plotId = plist->GetPlotID();
        //
        // Is there an active non-hidden plot in the originating window?
        //
        if (plotId == -1)
        {
            msg = "PICK requires an active non-hidden Plot.\n";
            msg += "Please select a plot and try again.\n";
            Error(msg.c_str());
            return;
        }
        ViewerPlot *plot = plist->GetPlot(plotId);
        const char *host = plot->GetHostName();
        const char *db = plot->GetDatabaseName();
        const char *activeVar = plot->GetVariableName();
        pickAtts->SetActiveVariable(activeVar);
        intVector n;
        pickAtts->SetNodes(n);
        int t = win->GetAnimation()->GetFrameIndex();
        //
        // Remove duplicate vars, so that query doesn't report them twice.
        //
        vector<string> userVars = pickAtts->GetUserSelectedVars();
        set<string> uniqueVarsSet;
        vector<string> uniqueVars; 
        for (int i = 0; i < userVars.size(); i++)
        {
            string v = userVars[i];
            if (strcmp(v.c_str(), "default") == 0)
            {
                v = activeVar;
            }
            if (uniqueVarsSet.count(v) == 0)
            {
                uniqueVars.push_back(v);
                uniqueVarsSet.insert(v); 
            }
        }
        pickAtts->SetUserSelectedVars(uniqueVars);
        pickAtts->SetPickLetter(designator);
        pickAtts->SetZoneNumber(-1);
        pickAtts->SetDomain(-1);
        pickAtts->SetTimeStep(t);
        pickAtts->SetFulfilled(false);
        pickAtts->SetDatabaseName(db);

        float *rp1 = pd.rayPt1;
        float *rp2 = pd.rayPt2;
        //
        // If in full-frame mode on a 2d plot, the ray points were computed
        // in the scaled full-frame space.  Reverse the scaling to get the 
        // correct ray points. 
        //
        if (win->GetFullFrameMode() && win->GetViewDimension() == 2)
        {
            double scale;
            int type;
            win->GetScaleFactorAndType(scale, type);
            if (type == 0 && scale != 0.) // x_axis
            {
                rp1[0] /= scale;
                rp2[0] /= scale;
            }
            else if (type == 1 && scale != 0.) // y_axis 
            {
                rp1[1] /= scale;
                rp2[1] /= scale;
            }
        }

        pickAtts->SetRayPoint1(rp1);
        pickAtts->SetRayPoint2(rp2);


        bool retry;
        int numAttempts = 0; 
        do 
        {   
            retry = false;
            int networkId = plot->GetNetworkID();

            TRY
            {
                PickAttributes pa = *pickAtts;
                ViewerEngineManager::Instance()->Pick(host, networkId, 
                     &pa, pa);
                if (pa.GetFulfilled())
                {
                   *pickAtts = pa;
                   //
                   // Reset the vars to what the user actually typed.
                   //
                   pickAtts->SetUserSelectedVars(userVars);
                   win->ValidateQuery(pickAtts, NULL);
                   //SEND PICKATTS TO GUI WINDOW FOR DISPLAY
                   pickAtts->CreateOutputString(msg);
                   Message(msg.c_str()); 
                   pickAtts->Notify();
                   UpdateDesignator();
                }
                else 
                {
                   //
                   // Reset vars to what the user actually typed.
                   //
                   pickAtts->SetUserSelectedVars(userVars);
                   //SEND ERROR MESSAGE TO GUI WINDOW FOR DISPLAY
                   Message("Pick not valid for current plot" );
                }
            }
            CATCH(NoEngineException)
            {
                // Queries access the cached network used by the queried plot.
                // Simply relaunching the engine does not work, as no network
                // is created. This situation requires re-execution of the 
                // plot that is being queried.
                plot->ClearActors(t, t);
                win->GetAnimation()->UpdateFrame(); 
                retry = true;
                numAttempts++; 
            }
            CATCH(LostConnectionException)
            {
                // Queries access the cached network used by the queried plot.
                // Simply relaunching the engine does not work, as no network
                // is created. This situation requires re-execution of the 
                // plot that is being queried.
                plot->ClearActors(t, t);
                win->GetAnimation()->UpdateFrame(); 
                retry = true;
                numAttempts++; 
            }
            CATCH2(VisItException, e)
            {
                //
                // Reset the vars to what the user actually typed.
                //
                pickAtts->SetUserSelectedVars(userVars);
                char message[500];
                //
                // Add as much information to the message as we can,
                // including exception type and exception message.
                //
                SNPRINTF(message, 500, "Pick:  (%s)\n%s", 
                         e.GetExceptionType().c_str(),
                         e.GetMessage().c_str());

                Error(message);
            }
            ENDTRY
        } while (retry && numAttempts < 2);
    }
    else
    {
       Message("The picked point is not contained in a surface");
    }

    //
    //  Perform any picks that have been cached.
    //
    if (!handlingCache)
        HandlePickCache();
}

// ****************************************************************************
// Method: ViewerQueryManager::GetColor
//
// Purpose: 
//   Gets the color to use for the refline.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 13:20:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

ColorAttribute
ViewerQueryManager::GetColor()
{
    ColorAttribute c;
    unsigned char rgb[3] = {0,0,0};

    //
    // Try and get the color for the colorIndex'th color in the default
    // discrete color table.
    //
    avtColorTables *ct = avtColorTables::Instance();
    if(ct->GetControlPointColor(ct->GetDefaultDiscreteColorTable(),
       colorIndex, rgb))
    {
        c.SetRed(int(rgb[0]));
        c.SetGreen(int(rgb[1]));
        c.SetBlue(int(rgb[2]));
    }

    // Increment the color index.
    ++colorIndex;

    return c;
}


// ****************************************************************************
// Method: ViewerQueryManager::Lineout
//
// Purpose: 
//   Performs a lineout on the currently active plot using the
//   passed endpoints.
//
// Arguments:
//   win       The window that originated the lineout.
//   pt1       The first endpoint of the refline in world coordinates. 
//   pt2       The second endpoint of the refline in world coordinates. 
//   vName     The variable to use for this lineout. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 19, 2002 
//
// Modifications:
//   Kathleen Bonnell, Mon Dec 23 08:23:26 PST 2002
//   Allow 3D lineouts.
//   
//   Kathleen Bonnell, Fri Mar 14 17:11:42 PST 2003 
//   Added win and var arguments.  SetVarName of line with the passed vName. 
//   
// ****************************************************************************

void
ViewerQueryManager::Lineout(ViewerWindow *win, const double pt1[3], 
                            const double pt2[3], const string &vName)
{
    if (win->GetTypeIsCurve())
    {
        Error("Lineout cannot be performed on curve windows.");
    }
    else if ((win->GetViewDimension() == 2) && (pt1[2] != 0 || pt2[2] != 0))
    {
        string msg = "Only 2D points allowed for 2D lineouts. ";
        msg += "Please set z-coord to 0.";
        Error(msg.c_str());
    }
    else
    {
        INTERACTION_MODE imode = win->GetInteractionMode();
        win->SetInteractionMode(LINEOUT);
        Line line;
        line.SetPoint1(pt1);
        line.SetPoint2(pt2);
        line.SetVarName(vName);
        AddQuery(win, &line);
        win->SetInteractionMode(imode);
    }
}

// ****************************************************************************
// Method: ViewerQueryManager::Lineout
//
// Purpose: 
//   Performs a lineout on the currently active plot using the
//   default Lineout attributes stored in ViewerOperatorFactory. 
//
// Arguments:
//   win : The window in which we're doing the lineout.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 20, 2002 
//
// Modifications:
//   Kathleen Bonnell, Mon Dec 23 08:23:26 PST 2002
//   Allow 3D lineouts.
//
//   Brad Whitlock, Wed Mar 12 14:42:59 PST 2003
//   I passed in the window in which to do the lineout.
//
//   Jeremy Meredith, Tue Jun 17 19:29:00 PDT 2003
//   Changed GetAllIndex to GetEnabledIndex.
//
// ****************************************************************************

void
ViewerQueryManager::Lineout(ViewerWindow *win)
{
    if(operatorFactory == 0)
        return;

    if (win->GetTypeIsCurve())
    {
        Error("Lineout cannot be performed on curve windows.");
    }
    else
    {
        int type = OperatorPluginManager::Instance()->GetEnabledIndex("Lineout_1.0"); 
        AttributeSubject *atts = operatorFactory->GetDefaultAtts(type);
        Line *line = (Line*)atts->CreateCompatible("Line");
        double *pt1 = line->GetPoint1();
        double *pt2 = line->GetPoint2();
        if ((win->GetViewDimension() == 2) && (pt1[2] != 0 || pt2[2] != 0))
        {
            string msg = "Only 2D points allowed for 2D lineouts. ";
            msg += "Please set z-coord to 0.";
            Error(msg.c_str());
        }
        else
        {
            INTERACTION_MODE imode = win->GetInteractionMode();
            win->SetInteractionMode(LINEOUT);
            AddQuery(win, line);
            win->SetInteractionMode(imode);
        }
        delete line;
    }
}


// ****************************************************************************
// Method: ViewerQueryManager::UpdateDesignator
//
// Purpose: 
//   Increments the baseDesignator.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 19, 2002 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::UpdateDesignator()
{
    baseDesignator++;
    //
    // Keep to alpha chars only.
    //
    if (baseDesignator > 'Z' && baseDesignator < 'a')
    {
        baseDesignator = 'a';
    }
    else if (baseDesignator > 'z')
    {
        baseDesignator = 'A';
        cycleDesignator = !cycleDesignator;
    }

    if (cycleDesignator)
    {
        SNPRINTF(designator, 4, "%c%c", baseDesignator, baseDesignator); 
    }
    else 
    {
        SNPRINTF(designator, 4, "%c", baseDesignator); 
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetGlobalLineoutAtts
//
//  Purpose:
//    Returns a pointer to the global lineout attributes.
//
//  Returns:    A pointer to the global lineout attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 13, 2003 
//
// ****************************************************************************

GlobalLineoutAttributes *
ViewerQueryManager::GetGlobalLineoutAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (globalLineoutAtts == 0)
    {
        globalLineoutAtts = new GlobalLineoutAttributes;
    }
 
    return globalLineoutAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetGlobalLineoutClientAtts
//
//  Purpose:
//    Returns a pointer to the global lineout client attributes.
//
//  Returns:    A pointer to the global lineout client attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 13, 2003 
//
// ****************************************************************************

GlobalLineoutAttributes *
ViewerQueryManager::GetGlobalLineoutClientAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (globalLineoutClientAtts == 0)
    {
        globalLineoutClientAtts = new GlobalLineoutAttributes;
    }
 
    return globalLineoutClientAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::SetDynamicLineout
//
//  Purpose:
//    Tells the lineout lists to Start/Stop observing the originating plot
//    based on the passed value. 
//
//  Arguments:
//    newMode   True if DynamicLineout is turned on, false otherwise. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 13, 2003 
//
// ****************************************************************************

void
ViewerQueryManager::SetDynamicLineout(bool newMode)
{
    for (int i = 0; i < nLineouts; ++i)
    {
        if (newMode)
            lineoutList[i]->ObserveOriginatingPlot();
        else 
            lineoutList[i]->StopObservingPlot();
    }

}

// ****************************************************************************
//  Method: ViewerQueryManager::SetGlobalLineoutAttsFromClient
//
//  Purpose:
//    Sets the defualt lineout atts from the client lineout atts. 
//    based on the passed value. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   January 13, 2003 
//
// ****************************************************************************

void
ViewerQueryManager::SetGlobalLineoutAttsFromClient()
{
    if (globalLineoutAtts == 0)
    {
        globalLineoutAtts = new GlobalLineoutAttributes;
    }

    if (globalLineoutClientAtts != 0)
    {
        if (globalLineoutAtts->GetDynamic() != globalLineoutClientAtts->GetDynamic())
        {
            SetDynamicLineout(globalLineoutClientAtts->GetDynamic());
        }
        *globalLineoutAtts = *globalLineoutClientAtts;
    }
}


// ****************************************************************************
//  Method:  ViewerQueryManager::DetermineVarType
//
//  Purpose:
//    Returns the type of the passed variable.
//  
//  Arguments:
//    hName     The host name where the variable can be found.
//    dbName    The database where the variable can be found.
//    varName   The variable name.
//
//  Returns:
//    The type of the variable.
//
//  Progammer:  Kathleen S. Bonnell
//  Creation:   March 14, 2003
//
//  Modifications:
// 
// ****************************************************************************

avtVarType
ViewerQueryManager::DetermineVarType(const char *hName, const char *dbName, const char *varName)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    ViewerExpressionList *elist = ViewerExpressionList::Instance();
    if (elist->VariableIsExpression(varName))
    {
        // Get the expression type.
        retval = elist->GetExpressionType(varName);
    }
    else
    {
        ViewerFileServer *s = ViewerFileServer::Instance();
        avtDatabaseMetaData *md =
            (avtDatabaseMetaData *) s->GetMetaData(string(hName),
                                                   string(dbName));
        if (md != 0)
        {
            // 
            // Get the type for the variable.
            // 
            retval = md->DetermineVarType(string(varName));
        }
    }

    return retval;
}


// ****************************************************************************
//  Method:  ViewerQueryManager::StartPickMode
//
//  Purpose:
//    Notify this class the pick mode has begun. 
//  
//  Progammer:  Kathleen S. Bonnell
//  Creation:   March 26, 2003
//
//  Modifications:
// 
// ****************************************************************************

void
ViewerQueryManager::StartPickMode()
{
    initialPick = true;
}


// ****************************************************************************
//  Method:  ViewerQueryManager::StopPickMode
//
//  Purpose:
//    Notify this class the pick mode has ended. 
//  
//  Progammer:  Kathleen S. Bonnell
//  Creation:   March 26, 2003
//
//  Modifications:
// 
// ****************************************************************************

void
ViewerQueryManager::StopPickMode()
{
    initialPick = false;
}


// ****************************************************************************
//  Method:  ViewerQueryManager::HandlePickCache
//
//  Purpose:
//    Calls Pick on all items in the pick cache, and empties the cache on
//    completion.
//  
//  Progammer:  Kathleen S. Bonnell
//  Creation:   March 26, 2003
//
//  Modifications:
// 
// ****************************************************************************

void
ViewerQueryManager::HandlePickCache()
{
    handlingCache = true;
    for (int i = 0; i < pickCache.size(); i++)
    {
        Pick(&pickCache[i]);
    }
    pickCache.clear();
    handlingCache = false;
}


// ****************************************************************************
//  Method: ViewerQueryManager::PointQuery
//
//  Purpose:
//    Perform a point query.
//
//  Arguments:
//    qName     The name of this query.
//    pt1       The point to use for query. 
//    vars      The variables to be used for this query.
//    
//  Programmer: Kathleen Bonnell
//  Creation:   May 14, 2003 
//
// ****************************************************************************

void         
ViewerQueryManager::PointQuery(const string &qName, const double *pt, 
                    const vector<string> &vars)
{
    if (strcmp(qName.c_str(), "Pick") == 0)
    {
        PickAttributes *pa = GetPickAtts();
        pa->SetUserSelectedVars(vars);
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        win->Pick((int)pt[0], (int)pt[1]);
    }
}


// ****************************************************************************
// Method: ViewerQueryManager::Lineout
//
// Purpose: 
//   Performs a lineout on the currently active plot using the
//   passed line attributes.
//
// Arguments:
//   win       The window that originated the lineout.
//   lineAtts  The attributes necessary for performing lineout. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::Lineout(ViewerWindow *origWin, Line *lineAtts)
{
    //
    // If in full-frame mode on a 2d plot, the end points were computed
    // in the scaled full-frame space.  Reverse the scaling to get the 
    // correct end points. 
    //
    if (origWin->GetFullFrameMode() && origWin->GetViewDimension() == 2)
    {
        double *pt1 = lineAtts->GetPoint1();
        double *pt2 = lineAtts->GetPoint2();
        double scale;
        int type;
        origWin->GetScaleFactorAndType(scale, type);
        if (type == 0 && scale != 0.) // x_axis
        {
            pt1[0] /= scale;
            pt2[0] /= scale;
        }
        else if (type == 1 && scale != 0.) // y_axis 
        {
            pt1[1] /= scale;
            pt2[1] /= scale;
        }
    }
    AddQuery(origWin, lineAtts);
}
