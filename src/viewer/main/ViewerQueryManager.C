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
#include <DataNode.h>
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
#include <ParsingExprList.h>
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



#define QUERY_MESH_VAR              0x0001
#define QUERY_SCALAR_VAR            0x0002
#define QUERY_VECTOR_VAR            0x0004
#define QUERY_TENSOR_VAR            0x0008
#define QUERY_SYMMETRIC_TENSOR_VAR  0x0010
#define QUERY_MATERIAL_VAR          0x0020
#define QUERY_MATSPECIES_VAR        0x0040
#define QUERY_CURVE_VAR             0x0080
#define QUERY_UNKNOWN_VAR           0x0100




using std::vector;
using std::string;

//
// Storage for static data elements.
//

ViewerQueryManager *ViewerQueryManager::instance = 0;   

QueryAttributes *ViewerQueryManager::queryClientAtts=0;
PickAttributes *ViewerQueryManager::pickAtts=0;
PickAttributes *ViewerQueryManager::pickDefaultAtts=0;
PickAttributes *ViewerQueryManager::pickClientAtts=0;
GlobalLineoutAttributes *ViewerQueryManager::globalLineoutAtts=0;
GlobalLineoutAttributes *ViewerQueryManager::globalLineoutClientAtts=0;

void
GetUniqueVars(const stringVector &vars, const string &activeVar, 
              stringVector &uniqueVars);
string
CreateExtentsString(const double * extents, const int dim, const char *type);

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
//    Kathleen Bonnell, Fri Jun 27 15:54:30 PDT 2003
//    Renamed Pick query to ZonePick, added NodePick query. 
//    Initialize pickAtts. 
//
//    Kathleen Bonnell, Wed Jul 23 16:26:34 PDT 2003 
//    Added 'Variable by Zone', 'WorldPick' and 'WorldNodePick'. 
//
//    Kathleen Bonnell, Mon Sep 15 13:09:19 PDT 2003 
//    Moved initialization of queryTypes to its own method, so that it can
//    be called after plugins are loaded (lineout dependent upon plugins). 
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

    operatorFactory = 0;


    initialPick = false;
    preparingPick = false;
    handlingCache = false;
    pickAtts = new PickAttributes();
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
//    Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//    I removed a call to UpdateScaleFactor since it no longer exists.
//
//    Kathleen Bonnell, Thu Sep 11 12:04:26 PDT 2003 
//    Added optional bool arg that indicates if lineout should be initialized
//    with its default atts or its client atts.
//
//    Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//    Account for multiple active plots.
//
// ****************************************************************************

void
ViewerQueryManager::AddQuery(ViewerWindow *origWin, Line *lineAtts,
                             const bool fromDefault)
{
    std::vector<int> plotIDs;
    origWin->GetAnimation()->GetPlotList()->GetActivePlotIDs(plotIDs);
    //
    // Is there an active non-hidden plot in the originating window? 
    //
    if (plotIDs.size() == 0)
    {
        string msg("Lineout requires an active non-hidden Plot.\n");
        msg += "Please select a plot and try again.\n";
        Error(msg.c_str());
        return;
    }
    // Use the first plot.
    int plotId = plotIDs[0];

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


    ViewerQuery_p newQuery = new ViewerQuery(origWin, resWin, lineAtts, fromDefault);
    if(*newQuery == NULL)
    {
        Error("VisIt could not create the desired plot.");
        return;
    }

    //
    // Add the new query to the lineout list.
    //
    SimpleAddQuery(newQuery, oplot, origWin, resWin);

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
//    Kathleen Bonnell, Thu Mar  6 15:21:45 PST 2003  
//    Reworked to reflect that lineout queries stored in LineoutList.
//
//    Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//    I removed a call to UpdateScaleFactor since it no longer exists.
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
//    Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//    Account for multiple active plots.
//
// ****************************************************************************
 
void
ViewerQueryManager::HandleTool(ViewerWindow *oWin, const avtToolInterface &ti)
{
    std::vector<int> plotIDs;
    oWin->GetAnimation()->GetPlotList()->GetActivePlotIDs(plotIDs);
    if ((nLineouts == 0) || (plotIDs.size() == 0)) 
    {
        return;
    }
    // Use the first plot.
    int plotId = plotIDs[0];
    
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
//    Hank Childs, Thu Oct  2 14:22:16 PDT 200
//    Account for multiple plots.
//
// ****************************************************************************
 
bool
ViewerQueryManager::InitializeTool(ViewerWindow *oWin, avtToolInterface &ti)
{
    if (ti.GetAttributes()->TypeName() != "Line")
    {
        return false;
    }

    std::vector<int> plotIDs;
    oWin->GetAnimation()->GetPlotList()->GetActivePlotIDs(plotIDs);
    if ((nLineouts == 0) || (plotIDs.size() == 0))
    {
        return false;
    }
    // Use the first plot.
    int plotId = plotIDs[0];

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
//    Kathleen Bonnell, Fri Jul 11 10:16:34 PDT 2003 
//    Verify query exists before sending request to engine. 
// 
//    Kathleen Bonnell, Wed Jul 23 16:26:34 PDT 2003 
//    Added call to GetUniqueVars, so that 'default' will get set to
//    the active var, and no duplicates will be in the list when passed
//    to queryAtts.
// 
//    Hank Childs, Thu Oct  2 13:51:31 PDT 2003
//    Add support for queries with multiple inputs.
//
//    Kathleen Bonnell, Wed Oct 29 16:06:23 PST 2003 
//    Rework code around GetUniqueVars to ensure the vars are truly
//    unique and the "default" gets translated to the active variable. 
//
//    Kathleen Bonnell, Wed Nov 19 15:41:11 PST 2003
//    Added logic to verify that all variables the named query will process
//    are allowed by the query. 
//
//    Kathleen Bonnell, Wed Nov 26 16:08:29 PST 2003 
//    Added logic to handle SpatialExtents query.
// 
//    Kathleen Bonnell, Tue Feb  3 17:43:12 PST 2004 
//    Use arg1 to set queryAtts.CurrentPlotOnly var. 
// 
// ****************************************************************************

void         
ViewerQueryManager::DatabaseQuery(ViewerWindow *oWin, const string &qName,
                            const vector<string> &vars, const int arg1,
                            const int arg2)
{
    queryClientAtts->SetResultsMessage("");
    queryClientAtts->SetResultsValue(0.);
    if (!queryTypes->QueryExists(qName, QueryList::DatabaseQuery))
    {
        // we've reset some values, notify clients
        queryClientAtts->Notify();
        string msg(qName);
        msg += " is not a valid query name.\n";
        Error(msg.c_str());
        return;
    }

    int numInputs = queryTypes->NumberOfInputsForQuery(qName);
    if (numInputs <= 0)
    {
        // we've reset some values, notify clients
        queryClientAtts->Notify();
        string msg(qName);
        msg += " has had an internal error when processing.\n";
        Error(msg.c_str());
        return;
    }

    // Get a list of the active plots.
    ViewerPlotList *olist = oWin->GetAnimation()->GetPlotList();
    std::vector<int> plotIds;
    olist->GetActivePlotIDs(plotIds);
    int t = oWin->GetAnimation()->GetFrameIndex();

    //
    // Make sure the number of active plots jives with the expected number
    // of inputs for the query.
    //
    if (plotIds.size() != numInputs)
    {
        if (plotIds.size() == 0)
        {
            queryClientAtts->Notify();
            string msg(qName);
            msg += " requires an active non-hidden Plot.\n";
            msg += "Please select a plot and try again.\n";
            Error(msg.c_str());
            return;
        }
        else if (numInputs == 1)
        {
            //
            // We have a convention of just using the first active plot
            // when there are multiple active plots selected for a single
            // input query.  BY CONVENTION, THIS IS NOT AN ERROR.
            //
            int firstPlot = plotIds[0];
            plotIds.clear();
            plotIds.push_back(firstPlot);
        }
        else
        {
            queryClientAtts->Notify();
            string msg(qName);
            char num[32];
            sprintf(num, "%d", numInputs);
            msg += " requires exactly ";
            msg += num;
            msg += " plots to be selected, realized, and drawn.";
            msg += "   Please select them and try again.\n";
            Error(msg.c_str());
            return;
        }
    }

    stringVector uniqueVars; 
    stringVector tmp = vars;
    const char *host = NULL;
    const char *dbname = NULL;
    for (int i = 0 ; i < plotIds.size() ; i++)
    {
        int plotId = plotIds[i];
        ViewerPlot *oplot = olist->GetPlot(plotId);
        if (host != NULL && (strcmp(host, oplot->GetHostName()) != 0))
        {
            queryClientAtts->Notify();
            string msg = "Multiple input queries require all their inputs ";
            msg += "to be on the same host.\n";
            Error(msg.c_str());
            return;
        }
        host = oplot->GetHostName();
        dbname = oplot->GetDatabaseName();
        const char *activeVar = oplot->GetVariableName();
        GetUniqueVars(tmp, activeVar, uniqueVars);
        tmp = uniqueVars;
    }

    intVector varTypes;
    for (int j = 0; j < uniqueVars.size(); j++)
    {
        varTypes.push_back((int)DetermineVarType(host, dbname, uniqueVars[j].c_str()));
    }
 
    int badVarType =  VerifyQueryVariables(qName, varTypes); 
    if (badVarType != -1)
    {
        queryClientAtts->Notify();
        string msg = "Cannot perform a " + qName  + " query on variable  ";
        msg += uniqueVars[badVarType] + ".\n";
        Error(msg.c_str());
        return;
    }
    
    bool retry; 
    int numAttempts = 0;

    if (strcmp(qName.c_str(), "SpatialExtents") == 0)
    {
        //
        // NO NEED TO GO TO THE ENGINE FOR THIS INFORMATION, AS
        // IT IS AVAILABLE FROM THE PLOT
        //
        int plotId = plotIds[0];
        ViewerPlot *oplot = olist->GetPlot(plotId);
        int dim = oplot->GetSpatialDimension(t);
        double *ext;
        string s;
        if (arg1 == 0) // We want original extents
        {
            ext = oplot->GetSpatialExtents(t, AVT_ORIGINAL_EXTENTS);
            s = CreateExtentsString(ext, dim, "original");
        }
        else
        {
            ext = oplot->GetSpatialExtents(t, AVT_ACTUAL_EXTENTS);
            s = CreateExtentsString(ext, dim, "actual");
        }

        queryClientAtts->SetResultsMessage(s);
        queryClientAtts->SetResultsValues(ext, dim);
        delete [] ext;
        queryClientAtts->Notify();
        Message(s.c_str());
        return; 
    }

    do
    {
        retry = false;
        std::vector<int> networkIds;
        for (int i = 0 ; i < plotIds.size() ; i++)
        {
            int plotId = plotIds[i];
            ViewerPlot *oplot = olist->GetPlot(plotId);
            networkIds.push_back(oplot->GetNetworkID());
        }

        TRY
        { 
            ViewerEngineManager *eM = ViewerEngineManager::Instance();
            QueryAttributes qa;
            qa.SetName(qName);
            qa.SetVariables(uniqueVars);
            qa.SetVarTypes(varTypes);
            // Right now, use of Domain and DataType are mutually
            // exclusive, and we don't necessarily have to know thich one
            // the query will use, so go ahead and use arg1 to set both
            // atts.
            if (arg1) 
                qa.SetDataType(QueryAttributes::ActualData);
            else      
                qa.SetDataType(QueryAttributes::OriginalData); 
            qa.SetDomain(arg1);
            qa.SetElement(arg2);
            qa.SetTimeStep(t);
            if (strcmp(qName.c_str(), "Variable by Zone") == 0)
                qa.SetElementType(QueryAttributes::Zone);
            else if (strcmp(qName.c_str(), "Variable by Node") == 0)
                qa.SetElementType(QueryAttributes::Node);

            if (eM->Query(host, networkIds, &qa, qa))
            {
                qa.SetVariables(vars);
               *queryClientAtts = qa;
                queryClientAtts->Notify();
                Message(qa.GetResultsMessage().c_str());
            }
            else
            {
                queryClientAtts->Notify();
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
            for (int i = 0 ; i < plotIds.size() ; i++)
            {
                int plotId = plotIds[i];
                ViewerPlot *oplot = olist->GetPlot(plotId);
                oplot->ClearActors(curFrame, curFrame);
            }
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
            for (int i = 0 ; i < plotIds.size() ; i++)
            {
                int plotId = plotIds[i];
                ViewerPlot *oplot = olist->GetPlot(plotId);
                oplot->ClearActors(curFrame, curFrame);
            }
            oWin->GetAnimation()->UpdateFrame(); 
            retry = true;
            numAttempts++; 
        }
        CATCH2(VisItException, e)
        {
            char message[2048];

            if (e.GetExceptionType() == "InvalidDimensionsException")
            {
                //
                //  Create message for the gui that includes the query name
                //  and message.
                //
                SNPRINTF(message, sizeof(message), "%s:  %s", qName.c_str(),
                         e.GetMessage().c_str());
            }
            else if (e.GetExceptionType() == "NonQueryableInputException")
            {
                //
                //  Create message for the gui that includes the plot name
                //  and message.
                //
                SNPRINTF(message, sizeof(message), "%s%s",
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
                SNPRINTF(message, sizeof(message), "%s:  (%s)\n%s", qName.c_str(),
                         e.GetExceptionType().c_str(),
                         e.GetMessage().c_str());

            }
            queryClientAtts->Notify();
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
//    Kathleen Bonnell, Wed Jul 23 16:26:34 PDT 2003 
//    Remove duplicate vars, call GetUniqueVars. 
// 
// ****************************************************************************

void         
ViewerQueryManager::LineQuery(const char *qName, const double *pt1, 
                    const double *pt2, const vector<string> &vars,
                    const int samples)
{
    if (strcmp(qName, "Lineout") == 0)
    {
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        stringVector uniqueVars; 
        // GetUniqueVars sets 'default' to the passed 'activeVar'.
        // Since we don't want to go through the trouble of figuring what
        // that is right now, send 'default' as the active var.
        string activeVar("default");
        GetUniqueVars(vars, activeVar, uniqueVars);
        for (int i = 0; i < uniqueVars.size(); i++)
            Lineout(win, pt1, pt2, uniqueVars[i], samples);
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetPickDefaultAtts
//
//  Purpose:
//    Returns a pointer to the default pick attributes.
//
//  Returns:    A pointer to the default pick attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002 
//
// ****************************************************************************

PickAttributes *
ViewerQueryManager::GetPickDefaultAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (pickDefaultAtts == 0)
    {
        pickDefaultAtts = new PickAttributes;
    }
    return pickDefaultAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetPickClientAtts
//
//  Purpose:
//    Returns a pointer to the pick client attributes.
//
//  Returns:    A pointer to the pick client attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 30, 2003 
//
// ****************************************************************************

PickAttributes *
ViewerQueryManager::GetPickClientAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (pickClientAtts == 0)
    {
        pickClientAtts = new PickAttributes;
    }
    return pickClientAtts;
}

// ****************************************************************************
// Method: ViewerQueryManager::GetPickAtts
//
// Purpose: 
//   Returns the pick attributes.
//
// Returns:    A pointer to the pick attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 6 12:29:38 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

PickAttributes *
ViewerQueryManager::GetPickAtts()
{
    if(pickAtts == 0)
        pickAtts = new PickAttributes;

    return pickAtts;
}

// ****************************************************************************
//  Method: ViewerQueryManager::ResetPickLetter
//
//  Purpose:
//    Resets pickAtts to the default state. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26 , 2003 
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec 17 14:45:22 PST 2003
//    Renamed from ResetPickAttributes.  Only call ResetDesignator.
//
// ****************************************************************************

void
ViewerQueryManager::ResetPickLetter()
{
    ResetDesignator();
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetClientPickAttsFromDefault
//
//  Purpose:
//    Copies the default pick attributes into the client pick attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 9, 2003
//
// ****************************************************************************

void
ViewerQueryManager::SetClientPickAttsFromDefault()
{
    if (pickDefaultAtts != 0 && pickClientAtts != 0)
    {
        *pickAtts = *pickDefaultAtts;
        *pickClientAtts = *pickDefaultAtts;
         pickClientAtts->Notify();
    }
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetDefaultPickAttsFromClient
//
//  Purpose:
//    Sets the default pickAtts using the client pick attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 9, 2003
//
// ****************************************************************************

void
ViewerQueryManager::SetDefaultPickAttsFromClient()
{
    if (pickDefaultAtts != 0 && pickClientAtts != 0)
    {
        *pickDefaultAtts = *pickClientAtts;
    }
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetPickAttsFromClient
//
//  Purpose:
//    Sets the pickAtts using the client pick attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 30, 2003 
//
// ****************************************************************************

void
ViewerQueryManager::SetPickAttsFromClient()
{
    if (pickAtts == 0)
    {
        pickAtts = new PickAttributes;
    }

    *pickAtts = *pickClientAtts;
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetPickAttsFromDefault
//
//  Purpose:
//    Sets the pickAtts using the default pick attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 9, 2003 
//
// ****************************************************************************

void
ViewerQueryManager::SetPickAttsFromDefault()
{
    if (pickAtts == 0)
    {
        pickAtts = new PickAttributes;
    }

    *pickAtts = *pickDefaultAtts;
    UpdatePickAtts();
}


// ****************************************************************************
//  Method: ViewerQueryManager::UpdatePickAtts
//
//  Purpose:
//    Causes the pickAtts to be sent to the client. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 30, 2003 
//
// ****************************************************************************

void
ViewerQueryManager::UpdatePickAtts()
{
    if (pickClientAtts != 0 && pickAtts != 0)
    {
        *pickClientAtts = *pickAtts;
        pickClientAtts->Notify();
    }
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
//  Modifications:
//    Kathleen Bonnell, Tue Jul  1 09:21:57 PDT 2003
//    Use pickClientAtts instead of pickAtts.
//
// ****************************************************************************

void
ViewerQueryManager::ClearPickPoints()
{
   // tell any observers that the pick points should be cleared
   pickClientAtts->SetClearWindow(true);
   pickClientAtts->SetFulfilled(false);
   pickClientAtts->Notify();
   // reset the clear window flag.
   pickClientAtts->SetClearWindow(false);
   pickClientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerQueryManager::ComputePick
//
//  Purpose:
//    Calculates pick attributes for a point.
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
//    Kathleen Bonnell, Fri Jun 27 15:54:30 PDT 2003 
//    Moved clean-up of pickAtts ivars to PickAttributes::PrepareForNewPick. 
//    Added call to UpdatePickAtts.
//    
//    Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//    I replaced the use of GetViewDimension with GetWindowMode. 
//   
//    Kathleen Bonnell, Thu Sep 18 16:29:43 PDT 2003 
//    Don't scale the ray points if in world-pick mode.
//   
//    Kathleen Bonnell, Wed Nov  5 17:09:00 PST 2003 
//    Retrieve plot's actual extents and store them in pickAtts. 
//   
//    Kathleen Bonnell, Mon Dec  1 18:04:41 PST 2003 
//    Added 'dom' and 'el' args, to accommodate PickByNode, PickByZone. 
//   
//    Kathleen Bonnell, Tue Dec  2 17:31:17 PST 2003 
//    Code changes to allow picking curve plots. 
//
//    Brad Whitlock, Tue Jan 6 09:54:12 PDT 2004
//    I renamed the routine to ComputePick, made it return a bool, and moved
//    the code that adds the pick point to the window to the Pick method.
//
// ****************************************************************************

bool
ViewerQueryManager::ComputePick(PICK_POINT_INFO *ppi, const int dom, const int el)
{
    bool retval = false;

    //
    //  Keep local copy, due to caching issues.
    // 
    PICK_POINT_INFO pd = *ppi;
    if (preparingPick)
    {
        debug5 << "Caching pick point info." << endl;
        pickCache.push_back(pd);
        return retval;
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
        //
        // Clean-up varInfos, and reset certain ivars that get set/read
        // elsewhere in the pick process.
        //
        pickAtts->PrepareForNewPick();

        string msg;
        ViewerWindow *win = (ViewerWindow *)pd.callbackData;

        ViewerPlotList *plist = win->GetAnimation()->GetPlotList();
        std::vector<int> plotIDs;
        plist->GetActivePlotIDs(plotIDs);
        //
        // Is there an active non-hidden plot in the originating window?
        //
        if (plotIDs.size() == 0)
        {
            msg = "PICK requires an active non-hidden Plot.\n";
            msg += "Please select a plot and try again.\n";
            Error(msg.c_str());
            return retval ;
        }
        // Use the first plot.
        int plotId = plotIDs[0];
        ViewerPlot *plot = plist->GetPlot(plotId);
        const char *host = plot->GetHostName();
        const char *db = plot->GetDatabaseName();
        const char *activeVar = plot->GetVariableName();
        pickAtts->SetActiveVariable(activeVar);
        int t = win->GetAnimation()->GetFrameIndex();

        //
        // Retrieve plot's actual extents. 
        //
        double *dext = plot->GetSpatialExtents(t, AVT_ACTUAL_EXTENTS);
        if (dext)
        {
            int dim = plot->GetSpatialDimension(t);
            float pb[6] = { 0., 0., 0., 0., 0., 0.};
            for (int i = 0; i < 2*dim; i++)
            {
                pb[i] = dext[i];
            }
            pickAtts->SetPlotBounds(pb);
            delete [] dext;
        }

        //
        // Remove duplicate vars, so that query doesn't report them twice.
        //
        vector<string> userVars = pickAtts->GetVariables();
        vector<string> uniqueVars; 
        GetUniqueVars(userVars, activeVar, uniqueVars);
        pickAtts->SetVariables(uniqueVars);
        pickAtts->SetPickLetter(designator);
        pickAtts->SetTimeStep(t);
        pickAtts->SetDatabaseName(db);
        if (win->GetWindowMode() == WINMODE_CURVE)
        {
            if (dom != -1 && el != -1)
            {
                // doing a PickByNode or PickByZone
                if (pickAtts->GetPickType() == PickAttributes::Zone)
                    pickAtts->SetPickType(PickAttributes::CurveZone);
                else 
                    pickAtts->SetPickType(PickAttributes::CurveNode);
            }
            else 
                pickAtts->SetPickType(PickAttributes::CurveNode);
   
        }

        float *rp1 = pd.rayPt1;
        float *rp2 = pd.rayPt2;
        //
        // If in full-frame mode on a 2d plot, the ray points were computed
        // in the scaled full-frame space.  Reverse the scaling to get the 
        // correct ray points.  -- But only if our ray points aren't equal
        // (they are set equivalent for WorldPick).
        //
        bool ptsEqual  = 
           (rp1[0] == rp2[0] && rp1[1] == rp2[1] && rp1[2] == rp2[2]);

        if ((win->GetFullFrameMode() && win->GetWindowMode() == WINMODE_2D &&
            !ptsEqual) || 
            (win->GetWindowMode() == WINMODE_CURVE))
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
   
        //
        // Most of the time, these will be -1, Except when picking
        // via PickByZone or PickByNode.
        //
        if (dom != -1 && el != -1)
        {
            pickAtts->SetDomain(dom);
            pickAtts->SetElementNumber(el);
            float dummyPt[3] = { FLT_MAX, 0., 0.};
            pickAtts->SetPickPoint(dummyPt);
            pickAtts->SetCellPoint(dummyPt);
        }

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
                   pickAtts->SetVariables(userVars);

                   //
                   // At this point, pickAtts contains information for a 
                   // valid pick point. We can return true.
                   //
                   retval = true;
                }
                else 
                {
                   //
                   // Reset vars to what the user actually typed.
                   //
                   pickAtts->SetVariables(userVars);
                   UpdatePickAtts();
               
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
                pickAtts->SetVariables(userVars);
                char message[2048];
                //
                // Add as much information to the message as we can,
                // including exception type and exception message.
                //
                SNPRINTF(message, sizeof(message), "Pick:  (%s)\n%s", 
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

    return retval;
}

// ****************************************************************************
// Method: ViewerQueryManager::Pick
//
// Purpose: 
//   Performs a pick, which if it is successful adds a pick letter to the
//   vis window.
//
// Arguments:
//   ppi : Information about the pick point.
//   dom : The domain number of the pick point or -1 to determine the domain.
//   el  : The element number for the pick or -1 to determine the number.
//
// Note:       This code was moved from the ComputePick method and was
//             reformulated to call ComputePick.
//
// Programmer: Kathleen Bonnell, Brad Whitlock
// Creation:   Tue Jan 6 09:59:18 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::Pick(PICK_POINT_INFO *ppi, const int dom, const int el)
{
    if(ComputePick(ppi, dom, el))
    {
        //
        // Add a pick point to the window
        //
        ViewerWindow *win = (ViewerWindow *)ppi->callbackData;
        win->ValidateQuery(pickAtts, NULL);

        //
        // Send pick attributes to the client.
        //
        std::string msg;
        pickAtts->CreateOutputString(msg);
        Message(msg.c_str()); 
        UpdatePickAtts();

        //
        // Make the pick label ready for the next pick point.
        //
        UpdateDesignator();
    }

    //
    //  Perform any picks that have been cached.
    //
    if (!handlingCache)
        HandlePickCache();
}

// ****************************************************************************
// Method: ViewerQueryManager::NoGraphicsPick
//
// Purpose: 
//   Computes the pick attributes for a pick point but does not add a pick
//   label to the vis window.
//
// Arguments:
//   ppi : Information about the pick.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 6 10:03:03 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::NoGraphicsPick(PICK_POINT_INFO *ppi)
{
    ppi->validPick = ComputePick(ppi);
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
//   samples   The number of sample points along the line.
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
//   Kathleen Bonnell, Wed Jul 23 16:51:18 PDT 2003 
//   Added samples argument.  Removed calls to win->SetInteractionMode.
//   
//   Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//   Replaced references to GetTypeIsCurve and GetViewDimension with
//   GetWindowMode.
//
// ****************************************************************************

void
ViewerQueryManager::Lineout(ViewerWindow *win, const double pt1[3], 
                            const double pt2[3], const string &vName,
                            const int samples)
{
    if (win->GetWindowMode() == WINMODE_CURVE)
    {
        Error("Lineout cannot be performed on curve windows.");
    }
    else if ((win->GetWindowMode() == WINMODE_2D) &&
             (pt1[2] != 0 || pt2[2] != 0))
    {
        string msg = "Only 2D points allowed for 2D lineouts. ";
        msg += "Please set z-coord to 0.";
        Error(msg.c_str());
    }
    else
    {
        Line line;
        line.SetPoint1(pt1);
        line.SetPoint2(pt2);
        line.SetVarName(vName);
        line.SetNumSamples(samples);
        AddQuery(win, &line);
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
//   Kathleen Bonnell, Wed Jul 23 16:51:18 PDT 2003 
//   Removed calls to win->SetInteractionMode.
//   
//   Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//   Replaced references to GetTypeIsCurve and GetViewDimension with
//   GetWindowMode.
//
//   Kathleen Bonnell, Thu Sep 11 12:04:26 PDT 2003 
//   Added optional bool arg that indicates if lineout should be initialized
//   from its default or its client atts.
//   
// ****************************************************************************

void
ViewerQueryManager::Lineout(ViewerWindow *win, const bool fromDefault)
{
    if(operatorFactory == 0)
        return;

    if (win->GetWindowMode() == WINMODE_CURVE)
    {
        Error("Lineout cannot be performed on curve windows.");
    }
    else
    {
        int type = OperatorPluginManager::Instance()->GetEnabledIndex("Lineout_1.0"); 
        AttributeSubject *atts;
        if (fromDefault)
           atts = operatorFactory->GetDefaultAtts(type);
        else 
           atts = operatorFactory->GetClientAtts(type);
        Line *line = (Line*)atts->CreateCompatible("Line");
        double *pt1 = line->GetPoint1();
        double *pt2 = line->GetPoint2();
        if ((win->GetWindowMode() == WINMODE_2D) &&
            (pt1[2] != 0 || pt2[2] != 0))
        {
            string msg = "Only 2D points allowed for 2D lineouts. ";
            msg += "Please set z-coord to 0.";
            Error(msg.c_str());
        }
        else
        {
            AddQuery(win, line, fromDefault);
        }
        delete line;
    }
}


// ****************************************************************************
// Method: ViewerQueryManager::ResetDesignator
//
// Purpose: 
//   Resets the baseDesignator to the default starting value. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 26, 2003 
//
// Modifications:
//   
// ****************************************************************************
void
ViewerQueryManager::ResetDesignator()
{
    baseDesignator = 'A'; 
    cycleDesignator = false;
    SNPRINTF(designator, 4, "%c", baseDesignator); 
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
//      Sean Ahern, Mon Mar 17 22:30:07 America/Los_Angeles 2003
//      Changed to the new expression interface.
// 
// ****************************************************************************

avtVarType
ViewerQueryManager::DetermineVarType(const char *hName, const char *dbName, const char *varName)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    Expression *exp = ParsingExprList::GetExpression(varName);
    if (exp != NULL)
    {
        // Get the expression type.
        retval = ParsingExprList::GetAVTType(exp->GetType());
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
//    Notify this class that pick mode has begun, or has changed from one
//    pickmode type to another. 
//  
//  Progammer:  Kathleen S. Bonnell
//  Creation:   March 26, 2003
//
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 15:54:30 PDT 2003
//    Handle changes from one pick mode to another.
// 
// ****************************************************************************

void
ViewerQueryManager::StartPickMode(const bool firstEntry, const bool zonePick)
{
    if (firstEntry)
        initialPick = true;

    if (zonePick)
        pickAtts->SetPickType(PickAttributes::Zone);
    else 
        pickAtts->SetPickType(PickAttributes::Node);
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
//  Modifications:
//    Kathleen Bonnell, Fri Jun 27 15:54:30 PDT 2003
//    Only set pickAtts' variables if the passed list is not empty.
//    Handle NodePick.
//
//    Kathleen Bonnell, Wed Jul 23 16:56:15 PDT 2003 
//    Added support for WorldPick and WorldNodePick. 
//    
//    Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//    Replaced references to GetTypeIsCurve with GetWindowMode.
//
//    Kathleen Bonnell, Mon Dec  1 18:04:41 PST 2003 
//    Added optional int args, to support PickByDomain, PickByZone.
//   
//    Kathleen Bonnell, Wed Dec  3 13:11:34 PST 2003 
//    Remove no-curve restrictions. 
//   
// ****************************************************************************

void         
ViewerQueryManager::PointQuery(const string &qName, const double *pt, 
                    const vector<string> &vars, const int arg1, const int arg2)
{
    if ((strcmp(qName.c_str(), "ZonePick") == 0) ||
        (strcmp(qName.c_str(), "Pick") == 0))
    {
        if (!vars.empty())
            pickAtts->SetVariables(vars);
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        win->Pick((int)pt[0], (int)pt[1], ZONE_PICK);
    }
    else if (strcmp(qName.c_str(), "NodePick") == 0)
    {
        if (!vars.empty())
            pickAtts->SetVariables(vars);
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        win->Pick((int)pt[0], (int)pt[1], NODE_PICK);
    }
    else if ((strcmp(qName.c_str(), "WorldPick") == 0) ||
             (strcmp(qName.c_str(), "WorldNodePick") == 0))
    {
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        if (!vars.empty())
            pickAtts->SetVariables(vars);

        INTERACTION_MODE imode  = win->GetInteractionMode();
        if (strcmp(qName.c_str(), "WorldPick") == 0)
            win->SetInteractionMode(ZONE_PICK);
        else
            win->SetInteractionMode(NODE_PICK);

        PICK_POINT_INFO ppi;
        ppi.callbackData = win;
        ppi.rayPt1[0] = ppi.rayPt2[0] = pt[0];
        ppi.rayPt1[1] = ppi.rayPt2[1] = pt[1];
        ppi.rayPt1[2] = ppi.rayPt2[2] = pt[2];
        ppi.validPick = true;
        Pick(&ppi);

        win->SetInteractionMode(imode);
    }
    else if ((strcmp(qName.c_str(), "PickByZone") == 0) ||
             (strcmp(qName.c_str(), "PickByNode") == 0))
    {
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        if (!vars.empty())
            pickAtts->SetVariables(vars);

        INTERACTION_MODE imode  = win->GetInteractionMode();
        if (strcmp(qName.c_str(), "PickByZone") == 0)
            win->SetInteractionMode(ZONE_PICK);
        else
            win->SetInteractionMode(NODE_PICK);

        PICK_POINT_INFO ppi;
        ppi.callbackData = win;
        ppi.rayPt1[0] = ppi.rayPt2[0] = pt[0];
        ppi.rayPt1[1] = ppi.rayPt2[1] = pt[1];
        ppi.rayPt1[2] = ppi.rayPt2[2] = pt[2];
        ppi.validPick = true;
        Pick(&ppi, arg1, arg2);

        win->SetInteractionMode(imode);
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
//   Eric Brugger, Wed Aug 20 11:05:54 PDT 2003
//   I replaced the use of GetViewDimension with GetWindowMode. 
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
    if (origWin->GetFullFrameMode() && origWin->GetWindowMode() == WINMODE_2D)
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


// ****************************************************************************
// Method: ViewerQueryManager::ViewDimChanged
//
// Purpose: 
//   Notifies lineouts that the view dim has changed. 
//
// Arguments:
//   modWin    The window whose view dimension is changing.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 9, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::ViewDimChanged(ViewerWindow *modWin)
{
    // 
    // No work to do if there are no lineouts, or if lineouts
    // are in dynamic mode.
    // 
    if (nLineouts == 0 || GetGlobalLineoutAtts()->GetDynamic())
    {
        return;
    }

    for (int i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingWindow(modWin))
        {
            lineoutList[i]->ViewDimChanged();
        }
    }
} 

// ****************************************************************************
// Method: ViewerQueryManager::CreateNode
//
// Purpose: 
//   Lets the query manager add its data to the tree that gets written to
//   the config file.
//
// Arguments:
//   parentNode : The node on which the query manager can add its data.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 22 10:15:14 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = new DataNode("ViewerQueryManager");
    parentNode->AddNode(mgrNode);

    //
    // Add information about the ViewerQueryManager.
    //
    mgrNode->AddNode(new DataNode("baseDesignator", baseDesignator));
    mgrNode->AddNode(new DataNode("cycleDesignator", cycleDesignator));
    mgrNode->AddNode(new DataNode("colorIndex", colorIndex));
}

// ****************************************************************************
// Method: ViewerQueryManager::SetFromNode
//
// Purpose: 
//   Lets the query manager initialize itself from data read in from the
//   config file.
//
// Arguments:
//   parentNode : The node where the query manager's data is stored.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 22 10:16:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *mgrNode = parentNode->GetNode("ViewerQueryManager");
    if(mgrNode == 0)
        return;

    DataNode *node;
    if((node = mgrNode->GetNode("cycleDesignator")) != 0)
        cycleDesignator = node->AsBool();

    if((node = mgrNode->GetNode("baseDesignator")) != 0)
    {
        baseDesignator = node->AsChar() - 1;
        UpdateDesignator();
    }

    if((node = mgrNode->GetNode("colorIndex")) != 0)
        colorIndex = node->AsInt();
}


// ****************************************************************************
//  Method: GetUniqueVars
//
//  Purpose:
//    Return a list of the unique vars in the passed argument. 
//
//  Arguments:
//    vars       The original list.
//    activeVar  The varname to use to replace 'default'.
//    uniqueVars The new list of unique vars. 
//    
//  Programmer: Kathleen Bonnell
//  Creation:   July 23, 2003. 
//
//  Modifications:
//    
// ****************************************************************************

void
GetUniqueVars(const stringVector &vars, const string &activeVar, 
              stringVector &uniqueVars)
{
    if (vars.size() == 0)
    {
        uniqueVars.push_back(activeVar);
        return;
    }
    set<string> uniqueVarsSet;
    for (int i = 0; i < vars.size(); i++)
    {
        string v = vars[i];
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
}


// ****************************************************************************
//  Method: ViewerQueryManager::InitializeQueryList
//
//  Purpose:
//    Initializes queryTypes with the types of queries available.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 15, 2003 
//
//  Modifications:
//    
//    Hank Childs, Thu Oct  2 09:46:27 PDT 2003
//    Add L2Norm, Area Between Curves, more.
//
//    Kathleen Bonnell, Wed Oct 29 16:06:23 PST 2003 
//    Uncomment MinMax, changed its AddQuery arguments.
//
//    Kathleen Bonnell, Mon Dec  1 18:04:41 PST 2003 
//    Added PickByNode, PickByZone. Allow certain queries to set their
//    window type. 
//
//    Hank Childs, Tue Feb  3 17:09:41 PST 2004
//    Added variable summation query.
//
//    Kathleen Bonnell, Tue Feb  3 17:43:12 PST 2004 
//    Rename 'Plot MinMax' to 'MinMax' , and set its window type to 
//    "CurrentPlot".
//
//    Kathleen Bonnell, Fri Feb 20 08:48:50 PST 2004 
//    Added NumNodes, NumZones. 
//
// ****************************************************************************

void
ViewerQueryManager::InitializeQueryList()
{
    queryTypes->AddQuery("ZonePick", QueryList::PointQuery, QueryList::ScreenSpace);
    queryTypes->AddQuery("NodePick", QueryList::PointQuery, QueryList::ScreenSpace);
    if (PlotPluginManager::Instance()->PluginAvailable("Curve_1.0") &&
        OperatorPluginManager::Instance()->PluginAvailable("Lineout_1.0")) 
    {
        queryTypes->AddQuery("Lineout", QueryList::LineQuery);
    }
    queryTypes->AddQuery("Eulerian", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Compactness", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Cycle", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Time", QueryList::DatabaseQuery);
    queryTypes->AddQuery("L2Norm", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Integrate", QueryList::DatabaseQuery);
    queryTypes->AddQuery("L2Norm Between Curves", QueryList::DatabaseQuery,
                                   QueryList::WorldSpace, 2);
    queryTypes->AddQuery("Area Between Curves", QueryList::DatabaseQuery,
                                   QueryList::WorldSpace, 2);
    queryTypes->AddQuery("Revolved volume", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Revolved surface area", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Surface area", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Volume", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Variable Sum", QueryList::DatabaseQuery);
    queryTypes->AddQuery("Weighted Variable Sum", QueryList::DatabaseQuery);
    queryTypes->AddQuery("WorldPick", QueryList::PointQuery);
    queryTypes->AddQuery("WorldNodePick", QueryList::PointQuery);
    queryTypes->AddQuery("Variable by Zone", QueryList::DatabaseQuery);
    queryTypes->SetWindowType("Variable by Zone", QueryList::DomainZone);

    int MinMaxVars = QUERY_SCALAR_VAR | QUERY_TENSOR_VAR | QUERY_VECTOR_VAR | 
            QUERY_SYMMETRIC_TENSOR_VAR | QUERY_MATSPECIES_VAR | QUERY_CURVE_VAR;
    queryTypes->AddQuery("MinMax", QueryList::DatabaseQuery, 
                          QueryList::WorldSpace, 1, MinMaxVars); 
    queryTypes->SetWindowType("MinMax", QueryList::ActualData);

    queryTypes->AddQuery("SpatialExtents", QueryList::DatabaseQuery);
    queryTypes->SetWindowType("SpatialExtents", QueryList::ActualData);
    queryTypes->AddQuery("NumNodes", QueryList::DatabaseQuery);
    queryTypes->SetWindowType("NumNodes", QueryList::ActualData);
    queryTypes->AddQuery("NumZones", QueryList::DatabaseQuery);
    queryTypes->SetWindowType("NumZones", QueryList::ActualData);
    queryTypes->AddQuery("PickByZone", QueryList::PointQuery);
    queryTypes->SetWindowType("PickByZone", QueryList::DomainZone);
    queryTypes->AddQuery("PickByNode", QueryList::PointQuery);
    queryTypes->SetWindowType("PickByNode", QueryList::DomainNode);

    queryTypes->SelectAll();
}


// ****************************************************************************
//  Method: ViewerQueryManager::VerifyQueryVariables
//
//  Purpose:
//    Verifies that the named query allows all the passed var types.
//
//  Returns:
//    The index of a bad var type, if present, -1 otherwise.
//
//  Arguments:
//    qName     The name of the query to check.
//    varTypes  A list of var types that the named query will be operating on.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 18, 2003 
//
//  Modifications:
//
// ****************************************************************************

int
ViewerQueryManager::VerifyQueryVariables(const std::string &qName, 
     const std::vector<int> &varTypes)
{
    int i, badIndex = -1;
    int allowedTypes = queryTypes->AllowedVarsForQuery(qName);
    if (allowedTypes > 0)
    {
        for (i = 0; i < varTypes.size() && badIndex == -1; i++)
        {
           int vt = (int) pow(2, varTypes[i]);
           if (!(allowedTypes & vt))
           {
               badIndex = i;
           }
        }
    }
   return badIndex; 
}


// ****************************************************************************
//  Method: CreateExtentsString
//
//  Purpose:
//    A helper method that takes in extents and creates a string. 
//
//  Returns:
//    A string listing the extents.
//
//  Arguments:
//    extents   The extents array to be converted to a string.
//    dim       The dimension of the extents.
//    type      The type of the extents.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2003 
//
//  Modifications:
//
// ****************************************************************************

string
CreateExtentsString(const double * extents, const int dim, const char *type)
{
    char msg[1024];
    if (dim == 1)
    {
        SNPRINTF(msg, 1024, "The %s extents are (%g, %g)", type,
                extents[0], extents[1]);
    }
    else if (dim == 2)
    {
        SNPRINTF(msg, 1024, "The %s extents are (%g, %g, %g, %g)", type,
            extents[0], extents[1], extents[2], extents[3]);
    }
    else if (dim == 3)
    {
        SNPRINTF(msg, 1024, "The %s extents are (%g, %g, %g, %g, %g, %g)", type,
            extents[0], extents[1], extents[2], extents[3], extents[4], extents[5]);
    }
    string msg2 = msg;
    return msg2;
}
