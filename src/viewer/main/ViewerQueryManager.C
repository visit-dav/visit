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
#include <avtSILRestrictionTraverser.h>
#include <avtTypes.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DataNode.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <GlobalAttributes.h>
#include <LineoutListItem.h>
#include <LostConnectionException.h>
#include <NoEngineException.h>
#include <OperatorPluginManager.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>
#include <PlotPluginManager.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <QueryOverTimeAttributes.h>
#include <ViewerActionManager.h>
#include <ViewerEngineManager.h>
#include <ParsingExprList.h>
#include <ViewerFileServer.h>
#include <ViewerOperator.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQuery.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <GlobalLineoutAttributes.h>
#include <VisItException.h>

#include <ViewerSubject.h>
extern ViewerSubject *viewerSubject;


// File wide modifications:
//   Brad Whitlock, Tue Jan 27 16:41:32 PST 2004
//   I removed all instances of ViewerAnimation.
//
// ****************************************************************************


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Mon Jul 25 08:23:55 PDT 2005
//    I added QUERY_ARRAY_VAR.  Note the ordering of the defines here must 
//    exactly match that in avtTypes.h.
//
// ****************************************************************************

#define QUERY_MESH_VAR              0x0001
#define QUERY_SCALAR_VAR            0x0002
#define QUERY_VECTOR_VAR            0x0004
#define QUERY_TENSOR_VAR            0x0008
#define QUERY_SYMMETRIC_TENSOR_VAR  0x0010
#define QUERY_ARRAY_VAR             0x0020
#define QUERY_LABEL_VAR             0x0040
#define QUERY_MATERIAL_VAR          0x0080
#define QUERY_MATSPECIES_VAR        0x0100
#define QUERY_CURVE_VAR             0x0200
#define QUERY_UNKNOWN_VAR           0x0400




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
QueryOverTimeAttributes *ViewerQueryManager::timeQueryAtts=0;
QueryOverTimeAttributes *ViewerQueryManager::timeQueryDefaultAtts=0;
QueryOverTimeAttributes *ViewerQueryManager::timeQueryClientAtts=0;

void
GetUniqueVars(const stringVector &vars, const string &activeVar, 
              stringVector &uniqueVars,
              const avtDatabaseMetaData *md = NULL);
string
CreateExtentsString(const double * extents, 
                    const int dim,
                    const char *type,
                    const string &float_format);

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
//    Kathleen Bonnell, Thu Jul 22 15:43:56 PDT 2004 
//    Initialize globalLineoutAtts, initialize resWinId in lineoutCache.
//
//    Kathleen Bonnell, Wed Jul 27 15:47:34 PDT 2005 
//    Added suppressQueryOutput. 
//
//    Kathleen Bonnell, Wed Aug 10 16:46:17 PDT 2005 
//    Added activePlotsChanged. 
//
//    Brad Whitlock, Mon Feb 12 17:45:24 PST 2007
//    Added ViewerBase base class.
//
//    Cyrus Harrison, Tue Sep 18 11:01:57 PDT 2007
//    Added init of floatFormat.
//
// ****************************************************************************

ViewerQueryManager::ViewerQueryManager() : ViewerBase(0)
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
    timeQueryAtts = new QueryOverTimeAttributes();
    globalLineoutAtts = new GlobalLineoutAttributes();

    lineoutCache.origWin = NULL;
    lineoutCache.resWinId = -1;

    suppressQueryOutput = false;
    activePlotsChanged = true;
    floatFormat = "%g";
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
//    Brad Whitlock, Fri Mar 26 08:42:11 PDT 2004
//    I made it use strings.
//
//    Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//    Made call to GetLineoutWindow fail if the window doesn't already exist
//
//    Kathleen Bonnell, Thu Jul 22 15:38:39 PDT 2004
//    Change call from GetLineoutWindow to GetWindow, as the needed window
//    for Lineout should already exist at this point, and its ID is stored
//    in lineoutCache.
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Added optional bool arg forceSampling. 
//
//    Brad Whitlock, Tue Apr 29 16:29:13 PDT 2008
//    Support for internationalization.
//
// ****************************************************************************

void
ViewerQueryManager::AddQuery(ViewerWindow *origWin, Line *lineAtts,
                             const bool fromDefault, const bool forceSampling)
{
    intVector plotIDs;
    origWin->GetPlotList()->GetActivePlotIDs(plotIDs);
    //
    // Is there an active non-hidden plot in the originating window?
    //
    if (plotIDs.size() == 0)
    {
        QString msg = tr("Lineout requires an active non-hidden plot. "
                         "Please select a plot and try again.");
        Error(msg);
        return;
    }
    // Use the first plot.
    int plotId = plotIDs[0];

    //
    // Is there a valid variable?
    //
    ViewerPlot *oplot = origWin->GetPlotList()->GetPlot(plotId);
    string vname(lineAtts->GetVarName());
    if (vname == "default")
        vname = oplot->GetVariableName();
    avtVarType varType = oplot->GetVarType(vname);
    if (varType != AVT_SCALAR_VAR &&
        varType != AVT_MATSPECIES)
    {
        QString message = tr("Lineout requires scalar variable.  "
                             "%1 is not scalar.").arg(vname.c_str());
        Error(message);
        return;
    }

    //
    // Can we get a lineout window?
    //
    ViewerWindow *resWin = ViewerWindowManager::Instance()->
        GetWindow(lineoutCache.resWinId);
    if (resWin == NULL)
    {
        ResetLineoutCache();
        return;
    }

    //
    // Set the color to use for the query.
    //
    lineAtts->SetColor(GetColor());
    lineAtts->SetDesignator(designator);


    ViewerQuery_p newQuery = new ViewerQuery(origWin, resWin, lineAtts, 
        fromDefault, forceSampling);
    if(*newQuery == NULL)
    {
        Error(tr("VisIt could not create the desired plot."));
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
//   Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005
//   Tell the lineouts whether or not to follow time, and whether or not to
//   use a time slider.
//
//   Kathleen Bonnell, Tue Jan 17 11:30:15 PST 2006
//   Removed call to SetTimeSlider.
//
//   Kathleen Bonnell, Thu Nov  2 13:52:00 PST 2006
//   Added test for FreezeInTime when determining if lineout FollwowsTime.
//
// ****************************************************************************

void
ViewerQueryManager::SimpleAddQuery(ViewerQuery_p query, ViewerPlot *oplot,
                                   ViewerWindow *owin, ViewerWindow *rwin)
{
    int i, index = -1;

    GlobalLineoutAttributes *gla = GetGlobalLineoutAtts();
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
        if (gla->GetDynamic())
        {
            lineoutList[index]->ObserveOriginatingPlot();
        }
        nLineouts++;
    }

    //
    // Add the query to the correct list.
    //
    lineoutList[index]->AddQuery(query);

    //
    // Tell it whether or not to follow time.
    //
    if (gla->GetDynamic() &&
        gla->GetCurveOption() == GlobalLineoutAttributes::CreateCurve)
    {
        lineoutList[index]->SetLineoutsFollowTime(false);
    }
    else if (gla->GetFreezeInTime())
    {
        lineoutList[index]->SetLineoutsFollowTime(false);
    }
    else
    {
        lineoutList[index]->SetLineoutsFollowTime(true);
    }
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
    intVector plotIDs;
    oWin->GetPlotList()->GetActivePlotIDs(plotIDs);
    if ((nLineouts == 0) || (plotIDs.size() == 0)) 
    {
        return;
    }
    // Use the first plot.
    int plotId = plotIDs[0];
    
    ViewerPlot *oPlot = oWin->GetPlotList()->GetPlot(plotId);
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

    intVector plotIDs;
    oWin->GetPlotList()->GetActivePlotIDs(plotIDs);
    if ((nLineouts == 0) || (plotIDs.size() == 0))
    {
        return false;
    }
    // Use the first plot.
    int plotId = plotIDs[0];

    bool retval = false;
    ViewerPlot *oPlot = oWin->GetPlotList()->GetPlot(plotId);
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
//    Kathleen Bonnell, Fri Mar  5 15:48:44 PST 2004 
//    Only DetermineVarTypes if necessary, and added more TRY-CATCH blocks. 
// 
//    Brad Whitlock, Wed Mar 10 10:06:36 PDT 2004
//    Changed to match other viewer changes related to plots and time.
//
//    Kathleen Bonnell, Tue Mar 23 15:31:32 PST 2004 
//    Restructured try-catch block around actual query execution, to catch
//    ImproperUseException which can occur if engine has been closed prior
//    to initiation of query.  
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map plots to the engine used to create them.
//
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004
//    Added bool arg to support queries-over-time. 
//
//    Jeremy Meredith, Mon Apr  5 14:15:56 PDT 2004
//    Added a line to clear out status in case of an exception occuring
//    during a query.
//
//    Kathleen Bonnell, Tue May 25 16:09:15 PDT 2004 
//    Made arg1 be Element and arg2 Domain, to be consistent with Pick'
//    default ordering (domain can be left out, so should be second arg). 
//  
//    Kathleen Bonnell, Thu Jun  3 14:40:47 PDT 2004 
//    Split verification of query info into SingleInput and MultipleInput,
//    added calls to DoSpatialExtentsQuery, VerifySingleInputQuery,
//    and VerifyMultipleInputQuery.
//   
//    Kathleen Bonnell, Fri Sep  3 09:59:25 PDT 2004 
//    Modified handling of 'NonQueryableInputException'. 
//
//    Kathleen Bonnell, Thu Dec 16 17:32:49 PST 2004 
//    Added 'elmentIsGlobal' arg, use it to set new QueryAttriubte 
//    'useGlobalId'.
//
//    Eric Brugger, Thu Jan  6 08:58:28 PST 2005
//    Corrected a misuse of the CATCH_RETURN macro.
//
//    Hank Childs, Fri Jan 28 11:55:12 PST 2005
//    Make sure returns inside 'try' are wrapped.
//
//    Hank Childs, Wed Mar  2 11:56:35 PST 2005
//    If we have plots that were generated dynamically, regenerate them before
//    performing our query.
//
//    Kathleen Bonnell, Tue Mar 15 17:39:33 PST 2005 
//    Only set 'DataType' from arg1 if the named query is an 'actual/original'
//    type of query, otherwise set DataType to 'Actual'.
//
//    Kathleen Bonnell, Thu Jul 14 09:16:22 PDT 2005 
//    Test for existence of engine before proceeding. 
//    
//    Kathleen Bonnell, Wed Jul 27 15:47:34 PDT 2005
//    Don't send message if QueryOutput is suppressed. 
//
//    Kathleen Bonnell, Tue Aug 16 10:03:27 PDT 2005
//    Moved warning/error condition detection code to beginning of method.
//    Check for non-hidden active plot before checking if EngineExists. 
//
//    Kathleen Bonnell, Tue Jan 31 15:45:23 PST 2006
//    If points were transformed, perform OrignalData SpatialExtents query
//    like other queries. 
//
//    Hank Childs, Wed May 10 09:52:54 PDT 2006
//    Don't crash if we can't get the query to complete ('7092).
//
//    Hank Childs, Tue Jul 11 14:34:06 PDT 2006
//    Added double arguments.
//
//    Dave Bremer, Fri Dec  8 17:52:22 PST 2006
//    Changed the double arguments to vectors of doubles.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Brad Whitlock, Tue Apr 29 16:31:58 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Fri Apr 17 09:58:52 PDT 2009
//    Send updated expressions to the engine before we execute the query in
//    case the query involves expressions that have not been sent to the engine
//    yet.
//
// ****************************************************************************

void         
ViewerQueryManager::DatabaseQuery(ViewerWindow *oWin, const string &qName,
                            const stringVector &vars, const bool doTimeQuery,
                            const int arg1, const int arg2,
                            const bool elementIsGlobal,
                            const doubleVector darg1, const doubleVector darg2)
{
    queryClientAtts->SetResultsMessage("");
    queryClientAtts->SetResultsValue(0.);
    if (!queryTypes->QueryExists(qName, QueryList::DatabaseQuery))
    {
        // we've reset some values, notify clients
        queryClientAtts->Notify();
        QString msg = tr("%1 is not a valid query name.\n").arg(qName.c_str());
        Error(msg);
        return;
    }
    if (!doTimeQuery && !queryTypes->RegularQueryAvailable(qName))
    {
        QString msg = tr("Regular (non-time) query is not available for %1.").
                      arg(qName.c_str());
        Error(msg);
        return;
    }

    // test for non-hidden active plot and running engine
    ViewerPlotList *plist = oWin->GetPlotList();
    intVector plotIds;
    plist->GetActivePlotIDs(plotIds);
    if (plotIds.size() > 0)
    {
        if (!EngineExistsForQuery(plist->GetPlot(plotIds[0])))
        {
            return;
        }
    }
    else
    {
        queryClientAtts->Notify();
        QString msg = tr("%1 requires an active non-hidden Plot.\n"
                         "Please select a plot and try again.\n").
                      arg(qName.c_str());
        Error(msg);
        return ;
    }


    int numInputs = queryTypes->NumberOfInputsForQuery(qName);
    if (numInputs <= 0)
    {
        // we've reset some values, notify clients
        queryClientAtts->Notify();
        QString msg = tr("%1 has had an internal error when processing.\n").
                      arg(qName.c_str());
        Error(msg);
        return;
    }

    bool canBeDLBPlots = true;
    if (canBeDLBPlots)
    {
        //
        // If we have plots generated by DLB, then they won't be suitable
        // for queries.  Regenerate them with SLB.
        //
        bool clearedActor = false;
        for (int i = 0 ; i < plotIds.size() ; i++)
        {
            int plotId = plotIds[i];
            ViewerPlot *plot = plist->GetPlot(plotId);
            const EngineKey   &engineKey = plot->GetEngineKey();
            int networkId = plot->GetNetworkID();
            avtDataObjectReader_p rdr = plot->GetReader();
            avtDataObject_p dob = rdr->GetOutput();
            bool isDyn = dob->GetInfo().GetValidity().AreWeStreaming();
            if (isDyn)
            {
                ViewerEngineManager::Instance()->StartQuery(engineKey, true,
                                                            networkId);
                plot->ClearCurrentActor();
                clearedActor = true;
            }
        }
        if (clearedActor)
        {
            plist->UpdatePlotList();
            plist->UpdateFrame();
        }
    }

    if (qName == "SpatialExtents")
    {
        //
        // NO NEED TO GO TO THE ENGINE FOR THIS INFORMATION, AS
        // IT IS AVAILABLE FROM THE PLOT
        //
        if (arg1 == AVT_ORIGINAL_EXTENTS)
        {

            avtDataObjectReader_p rdr = plist->GetPlot(plotIds[0])->GetReader();
            avtDataObject_p dob = rdr->GetOutput();
            if (!dob->GetInfo().GetValidity().GetPointsWereTransformed())
            {
                DoSpatialExtentsQuery(plist->GetPlot(plotIds[0]), arg1);
                return;
            }
        }
        else
        {
            DoSpatialExtentsQuery(plist->GetPlot(plotIds[0]), arg1);
            return;
        }
    }

    QueryAttributes qa;
    qa.SetFloatFormat(floatFormat);
    qa.SetName(qName);
    qa.SetUseGlobalId(elementIsGlobal);

    // Right now, use of Element and DataType are mutually
    // exclusive, and we don't necessarily have to know thich one
    // the query will use, so go ahead and use arg1 to set both atts.

    if (queryTypes->GetWindowType(qName) == QueryList::ActualData ||
        queryTypes->GetWindowType(qName) == QueryList::ActualDataVars)
    {
        if (arg1)
            qa.SetDataType(QueryAttributes::ActualData);
        else
            qa.SetDataType(QueryAttributes::OriginalData);
    }
    else
    {
        qa.SetDataType(QueryAttributes::ActualData);
    }

    qa.SetElement(arg1);
    qa.SetDomain(arg2);
    qa.SetDarg1(darg1);
    qa.SetDarg2(darg2);
    if (qName == "Variable by Zone")
        qa.SetElementType(QueryAttributes::Zone);
    else if (qName == "Variable by Node")
        qa.SetElementType(QueryAttributes::Node);

    if (numInputs == 1)
    {
        //
        // We have a convention of just using the first active plot
        // when there are multiple active plots selected for a single
        // input query.  BY CONVENTION, THIS IS NOT AN ERROR.
        //
        int firstPlot = plotIds[0];
        plotIds.clear();  
        plotIds.push_back(firstPlot);
        if (!VerifySingleInputQuery(plist, firstPlot, qName, vars, qa))
            return;
    }
    else 
    {
        if (!VerifyMultipleInputQuery(plist, numInputs, qName, vars, qa))
            return;
    }

    //
    //  Single input queries need only one engineKey, multiple input
    //  queries currently require all inputs to be on same host,
    //  so go ahead and grab engineKey from first active plot.
    //
    EngineKey engineKey = plist->GetPlot(plotIds[0])->GetEngineKey();

    bool retry; 
    int numAttempts = 0;
    bool success = false;

    do
    {
        retry = false;
        intVector networkIds;
        for (int i = 0 ; i < plotIds.size() ; i++)
        {
            int plotId = plotIds[i];
            ViewerPlot *oplot = plist->GetPlot(plotId);
            networkIds.push_back(oplot->GetNetworkID());
            if (qa.GetTimeStep() == -1)
                qa.SetTimeStep(oplot->GetState());
        }

        TRY
        {
            if(plotIds.size() > 0)
            {
                ViewerPlot *p = plist->GetPlot(plotIds[0]);
                ViewerEngineManager::Instance()->UpdateExpressionsFromPlot(p);
            }

            if (doTimeQuery)
            {
                DoTimeQuery(oWin, &qa);
                CATCH_RETURN(1);
            }

            if (ViewerEngineManager::Instance()->Query(engineKey, networkIds, 
                   &qa, qa))
            {
                qa.SetVariables(vars);
               *queryClientAtts = qa;
                queryClientAtts->Notify();
                if (!suppressQueryOutput) 
                    Message(qa.GetResultsMessage().c_str());
            }
            else
            {
                queryClientAtts->Notify();
                QString msg = tr("VisIt could not satisfy the query %1").
                              arg(qName.c_str());
                Error(msg);
            }
            success = true;
        }
        CATCH2(VisItException, e)
        {
            char message[2048];
            if (e.GetExceptionType() == "LostConnectionException" ||
                e.GetExceptionType() == "NoEngineException" ||
                e.GetExceptionType() == "ImproperUseException" )
            {
                //
                // Queries access the cached network used by the queried plot.
                // Simply relaunching the engine does not work, as no network
                // is created. This situation requires re-execution of the 
                // plot that is being queried.
                //
                for (int i = 0 ; i < plotIds.size() ; i++)
                    plist->GetPlot(plotIds[i])->ClearCurrentActor();
                oWin->GetPlotList()->UpdateFrame(); 
                numAttempts++; 
                retry = true;
            }
            else if ((e.GetExceptionType() == "InvalidDimensionsException") ||
                     (e.GetExceptionType() == "NonQueryableInputException"))
            {
                //
                //  Create message for the gui that includes the query name
                //  and message.
                //
                SNPRINTF(message, sizeof(message), "%s:  %s", qName.c_str(),
                         e.Message().c_str());
            }
            else
            {
                //
                // Add as much information to the message as we can,
                // including query name, exception type and exception
                // message.
                //
                SNPRINTF(message, sizeof(message), "%s:  (%s)\n%s", qName.c_str(),
                         e.GetExceptionType().c_str(),
                         e.Message().c_str());

            }
            ClearStatus(engineKey.ID().c_str());
            if (!retry)
            {
                queryClientAtts->Notify();
                Error(message, false);
                CATCH_RETURN(1);
            }
        }
        ENDTRY
    } while (retry && numAttempts < 2);

    if (!success)
    {
        Error(tr("VisIt was not able to execute the query.  Please "
                 "contact a VisIt developer."));
        return;
    }
    
    if (canBeDLBPlots)
    {
        //
        // If we had to regenerate plots done with streaming, tell
        // them that we are done with queries.
        //
        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        ViewerPlotList *plist = win->GetPlotList();
        intVector plotIDs;
        plist->GetActivePlotIDs(plotIDs);
        for (int i = 0 ; i < plotIDs.size() ; i++)
        {
            int plotId = plotIDs[i];
            ViewerPlot *plot = plist->GetPlot(plotId);
            const EngineKey   &engineKey = plot->GetEngineKey();
            int networkId = plot->GetNetworkID();
            avtDataObjectReader_p rdr = plot->GetReader();
            avtDataObject_p dob = rdr->GetOutput();
            bool isDyn = dob->GetInfo().GetValidity().AreWeStreaming();
            if (isDyn)
            {
                ViewerEngineManager::Instance()->StartQuery(engineKey, false, 
                                                            networkId);
            }
        }
    }
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
//    Kathleen Bonnell, Fri Jul  9 14:33:17 PDT 2004  
//    Renamed to StartLineQuery.  Attempt to get LineoutWindow. Save info 
//    necessary for Lineout to lineoutCache.
// 
//    Kathleen Bonnell, Thu Jul 22 15:43:56 PDT 2004 
//    Added useThisId arg to GetLineoutWindow call, so that a user-specified
//    window can be retrieved/created.  Save resWin's Id in lineoutCache
//    for use during Finish routine.
//
//    Kathleen Bonnell, Mon Jul 26 17:45:12 PDT 2004
//    Suspend socket signals in the viewer so that Lineout does not cause 
//    synchronization events to be processed before we are ready for them. 
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Added optional bool arg forceSampling. 
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
//    Brad Whitlock, Tue Apr 29 16:36:19 PDT 2008
//    Support for internationalization.
//
// ****************************************************************************

void         
ViewerQueryManager::StartLineQuery(const char *qName, const double *pt1, 
                    const double *pt2, const stringVector &vars,
                    const int samples, const bool forceSampling)
{
    if (strcmp(qName, "Lineout") == 0)
    {
        viewerSubject->BlockSocketSignals(true);
        //
        // Can we get a lineout window? 
        //
        int useThisId = (globalLineoutAtts->GetCreateWindow() ? -1 :
                         globalLineoutAtts->GetWindowId()-1);
        ViewerWindow *resWin = 
            ViewerWindowManager::Instance()->GetLineoutWindow(useThisId);
        if (resWin == NULL)
        {
            ResetLineoutCache();
            return;
        }

        ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
        if (win->GetWindowMode() == WINMODE_CURVE)
        {
            Error(tr("Lineout cannot be performed on curve windows."));
            return;
        }
        else if (win->GetWindowMode() == WINMODE_AXISARRAY)
        {
            Error(tr("Lineout cannot be performed on axis array windows."));
            return;
        }
        else if ((win->GetWindowMode() == WINMODE_2D) &&
                 (pt1[2] != 0 || pt2[2] != 0))
        {
            Error(tr("Only 2D points allowed for 2D lineouts. "
                     "Please set z-coord to 0."));
            return;
        }

        stringVector uniqueVars; 
        // GetUniqueVars sets 'default' to the passed 'activeVar'.
        // Since we don't want to go through the trouble of figuring what
        // that is right now, send 'default' as the active var.
        string activeVar("default");
        GetUniqueVars(vars, activeVar, uniqueVars);

        Line line;
        line.SetPoint1(pt1);
        line.SetPoint2(pt2);
        line.SetNumSamples(samples);

        lineoutCache.origWin = win;
        lineoutCache.line = line;
        lineoutCache.fromDefault = true;
        lineoutCache.vars = uniqueVars;
        lineoutCache.resWinId = resWin->GetWindowId();
        lineoutCache.forceSampling = forceSampling;
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
//    Brad Whitlock, Sat Jan 31 22:51:24 PST 2004
//    I made it so we don't have to pass the frame to StartPick.
//
//    Kathleen Bonnell, Tue Mar 16 16:02:05 PST 2004 
//    Determine VarTypes, and only pass along to Pick the valid ones.
//    Set invalidVars in PickAtts so that user will get a message. 
//
//    Kathleen Bonnell, Tue Mar 23 15:31:32 PST 2004 
//    Restructured try-catch block around actual pick execution, to catch
//    ImproperUseException which can occur if engine has been closed prior
//    to initiation of pick.  
//
//    Brad Whitlock, Fri Mar 26 09:35:15 PDT 2004
//    I made it use ViewerPlot::GetVarType because it knows the right time
//    state for which to ask for metadata. Down here we don't know it as well.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map plots to the engine used to create them.
//
//    Kathleen Bonnell, Tue May  4 14:41:50 PDT 2004 
//    Changed Message to Warning for pick failure, utilize new pic atts 
//    error message. 
//
//    Kathleen Bonnell, Wed Jun  2 10:00:35 PDT 2004 
//    Determine whether this pick needs Original Zones or Nodes, and
//    InverseTransform or Transform.   Determine if all materials are used
//    by plot being picked.
//
//    Mark C. Miller, Tue Aug 24 20:11:52 PDT 2004
//    Added code to set pick attributes' block/group piece names
//
//    Kathleen Bonnell, Tue Aug 24 14:59:16 PDT 2004 
//    Send MetaData to GetUniqueVars, so that all vars defined on mesh
//    can be retrieved if requested. 
//
//    Kathleen Bonnell, Wed Aug 25 18:13:21 PDT 2004 
//    Ensure that MeshMetaData is not null before attempting to dereference. 
//
//    Kathleen Bonnell, Tue Oct 12 16:31:46 PDT 2004 
//    Added support for picking on Vector plots -- calls a new ViewerWindow
//    method 'GlyphPick' to get the intersected domain & zone/node.  The 
//    intersection test is performed on data from the Renderer, where the
//    glyphs live.
//
//    Kathleen Bonnell, Tue Oct 12 16:31:46 PDT 2004 
//    Expand 'GlyphPick' to include non-LabelPlot point meshes. 
//
//    Kathleen Bonnell, Mon Nov  8 15:47:32 PST 2004
//    Moved full-frame test to before 'GlyphPick' test.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added window id to Pick RPC
//
//    Kathleen Bonnell, Tue Mar  1 11:20:15 PST 2005 
//    Ensure that the engine has the correct expression list from the plot
//    that we are picking, so that pick will not fail. 
//
//    Kathleen Bonnell, Tue Jul  5 15:38:58 PDT 2005 
//    When setting pickAtts 'MatSelected' state, use the plot's 'real' var,
//    in case there is an expression involved whose 'real' var is material. 
//
//    Kathleen Bonnell, Wed Aug 10 16:46:17 PDT 2005 
//    For StartPick, added test for changed active plots.
//
//    Kathleen Bonnell, Tue Aug 30 15:20:40 PDT 2005 
//    For StartPick, don't set needZone to 'true' if a nodal-var Tensor or
//    Vector is plotted.   Do GlyphPickf or Tensor plots.
//
//    Kathleen Bonnell, Thu Nov 17 13:39:42 PST 2005 
//    Retrieve topodim from plot and store in pickatts.  It is more reliable
//    than attempting retrieval from pipeline.  For the same reason,
//    determine if the input is 'LinesData' from the plot (spatdim = 2,
//    and plot is Boundary or Contour).  Save in pickatts.
//
//    Hank Childs, Thu May 25 11:29:49 PDT 2006
//    If we are picking on an array variable, make a histogram plot.
//
//    Kathleen Bonnell, Mon Oct 23 08:45:18 PDT 2006 
//    Ensure error message gets passed to clients. 
//
//    Hank Childs, Fri Jan  5 12:51:07 PST 2007
//    When we pick on an array variable (to make a histogram plot), make sure
//    the time state is correct.
//
//    Kathleen Bonnell, Wed Mar 28 09:56:11 PDT 2007 
//    Due to changes in NoWin rendering, certain glyphPicks must alwasy be
//    performed on the engine, add test for this. 
//
//    Hank Childs, Mon Aug 27 13:52:56 PDT 2007
//    Add support for picking creating a spreadsheet.
//
//    Gunther H. Weber, Fri Sep 14 11:20:07 PDT 2007
//    Modification of Hank's code for creating a spreadsheet. Do not switch
//    the domain of an existing spreadsheet. Create a new one instead.
//
//    Hank Childs, Mon Oct  8 19:51:21 PDT 2007
//    If the pick is of a mesh plot, make a spreadsheet anyway.
//
//    Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007 
//    Return early if time curve and preserving picked coordinates.
//
//    Brad Whitlock, Thu Jan 24 11:57:14 PDT 2008
//    Added new arguments to ViewerPlotList::AddPlot.
//
//    Gunther H. Weber, Wed Mar 19 18:50:34 PDT 2008
//    Added logic for spreadsheet pick mode.
//
//    Brad Whitlock, Tue Apr 29 16:37:08 PDT 2008
//    Support for internationalization.
//
//    Gunther H. Weber, Fri Aug 15 10:18:03 PDT 2008
//    Check whether reusePickLetter flag in PickAttributes is set and
//    do not update pick letter if it is.
//
// ****************************************************************************

bool
ViewerQueryManager::ComputePick(PICK_POINT_INFO *ppi, const int dom,
                                const int el)
{
    bool retval = false;

    //
    //  Keep local copy, due to caching issues.
    // 
    PICK_POINT_INFO pd = *ppi;
    if (preparingPick)
    {
        debug4 << "Caching pick point info." << endl;
        pickCache.push_back(pd);
        return retval;
    }
    if (initialPick || activePlotsChanged)
    {
        preparingPick = true;
        ViewerWindow *win = (ViewerWindow *)pd.callbackData;
        ViewerPlotList *plist = win->GetPlotList();
        intVector plotIds;
        plist->GetActivePlotIDs(plotIds);
        ViewerPlot *plot = plist->GetPlot(plotIds[0]);
 
        int pickType = pickAtts->GetPickType();
    
        bool needZones = (pickType == PickAttributes::Zone ||
                          pickType == PickAttributes::DomainZone);
        bool needInvTrans = (pickType == PickAttributes::Node ||
                             pickType == PickAttributes::Zone);

        if ((strcmp(plot->GetPlotTypeName(), "Vector") == 0) ||
            (strcmp(plot->GetPlotTypeName(), "Tensor") == 0)) 
        {
            if ( plot->GetVariableCentering() == AVT_NODECENT)
                needZones = false;
        }
   
        plist->StartPick(needZones, needInvTrans);
        initialPick = false;
        preparingPick = false;
        activePlotsChanged = false;
    }

    if (pd.validPick)
    {
        //
        // Clean-up varInfos, and reset certain ivars that get set/read
        // elsewhere in the pick process.
        //
        pickAtts->PrepareForNewPick();

        ViewerWindow *win = (ViewerWindow *)pd.callbackData;

        ViewerPlotList *plist = win->GetPlotList();
        intVector plotIDs;
        plist->GetActivePlotIDs(plotIDs);
        //
        // Is there an active non-hidden plot in the originating window?
        //
        if (plotIDs.size() == 0)
        {
            Error(tr("PICK requires an active non-hidden Plot.\n"
                     "Please select a plot and try again.\n"));
            return retval ;
        }
        // Use the first plot.
        int plotId = plotIDs[0];
        ViewerPlot *plot = plist->GetPlot(plotId);
        const EngineKey   &engineKey = plot->GetEngineKey();
        const string &db = plot->GetDatabaseName();
        const string &activeVar = plot->GetVariableName();
        pickAtts->SetActiveVariable(activeVar);
        const avtDatabaseMetaData *md = plot->GetMetaData();
        string meshForVar = md->MeshForVar(activeVar);
        const avtMeshMetaData *mmd = md->GetMesh(meshForVar);
        if (mmd != NULL)
        {
            pickAtts->SetBlockPieceName(mmd->blockPieceName);
            pickAtts->SetGroupPieceName(mmd->groupPieceName);
        }

        avtSILRestrictionTraverser trav(plot->GetSILRestriction());
        bool usesAllMaterials = trav.UsesAllMaterials();
        //
        // Retrieve plot's actual extents. 
        //
        double *dext = plot->GetSpatialExtents(AVT_ACTUAL_EXTENTS);
        if (dext)
        {
            int dim = plot->GetSpatialDimension();
            double pb[6] = { 0., 0., 0., 0., 0., 0.};
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
        stringVector userVars = pickAtts->GetVariables();
        stringVector uniqueVars; 
        GetUniqueVars(userVars, activeVar, uniqueVars, plot->GetMetaData());
        stringVector validVars;
        stringVector invalidVars;
        for (int i = 0; i < uniqueVars.size(); i++)
        {
            if (plot->GetVarType(uniqueVars[i]) != AVT_UNKNOWN_TYPE)
                validVars.push_back(uniqueVars[i]);
            else 
                invalidVars.push_back(uniqueVars[i]);
        }
        pickAtts->SetVariables(validVars);

        //
        // Use the 'real' var from the plot -- the plot's var may be
        // an expression whose base is a material var.
        //
        pickAtts->SetMatSelected(!usesAllMaterials || 
                                 plot->GetRealVarType() == AVT_MATERIAL);
        if (!pickAtts->GetReusePickLetter()) pickAtts->SetPickLetter(designator);
        pickAtts->SetTimeStep(plot->GetState());
        pickAtts->SetDatabaseName(db);
        if (win->GetWindowMode() == WINMODE_CURVE)
        {
            if (dom != -1 && el != -1)
            {
                // doing a PickByNode or PickByZone
                if (pickAtts->GetPickType() == PickAttributes::DomainZone)
                    pickAtts->SetPickType(PickAttributes::CurveZone);
                else 
                    pickAtts->SetPickType(PickAttributes::CurveNode);
            }
            else
                pickAtts->SetPickType(PickAttributes::CurveNode);
        }
        double *rp1 = pd.rayPt1;
        double *rp2 = pd.rayPt2;
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

        bool doGlyphPick = 
                  (strcmp(plot->GetPlotTypeName(), "Vector") == 0) ||
                  (strcmp(plot->GetPlotTypeName(), "Tensor") == 0) ||
                  ((plot->GetMeshType() == AVT_POINT_MESH) &&
                   (strcmp(plot->GetPlotTypeName(), "Label") != 0));

        bool mustGlyphPickOnEngine = doGlyphPick && 
                   GetViewerProperties()->GetNowin() && 
                  ((plot->GetMeshType() == AVT_POINT_MESH) &&
                   (strcmp(plot->GetPlotTypeName(), "Mesh") != 0));


        bool isLinesData = (plot->GetSpatialDimension() == 2) &&
                  ((strcmp(plot->GetPlotTypeName(), "Boundary") == 0) ||
                   (strcmp(plot->GetPlotTypeName(), "Contour") == 0));
        pickAtts->SetLinesData(isLinesData);
        pickAtts->SetInputTopoDim(plot->GetTopologicalDimension());
                  

        if (mustGlyphPickOnEngine || 
            (doGlyphPick && win->GetScalableRendering() && 
            (dom ==-1 || el == -1)))
        {
            pickAtts->SetRequiresGlyphPick(true);
        }
        else if (doGlyphPick && !(win->GetScalableRendering())) 
        {
            int d = -1, e = -1;
            bool forCell = false;
            if (dom == -1 || el == -1)
            {
                //
                // We only want to find an intersection  with the currently
                // active plot, so make it the only pickable actor in the 
                // renderer. Perform the intersection test, then make it
                // unpickable again.
                //
                plot->GetActor()->MakePickable();
                win->GlyphPick(rp1, rp2, d, e, forCell);
                plot->GetActor()->MakeUnPickable();
                if (d != -1)
                    d += plot->GetBlockOrigin();
                // 
                // Due to the nature of the glyphs, the pick type MUST match
                // the variable centering.
                // 
                if (forCell)
                    pickAtts->SetPickType(PickAttributes::DomainZone);
                else 
                    pickAtts->SetPickType(PickAttributes::DomainNode);
            }
            else // PickByNode or PickByZone
            {
                d = dom;
                e = el;
            }
            if (d != -1 && e != -1)
            {
                pickAtts->SetDomain(d);
                pickAtts->SetElementNumber(e);
                double dummyPt[3] = { FLT_MAX, 0., 0.};
                pickAtts->SetPickPoint(dummyPt);
                pickAtts->SetCellPoint(dummyPt);
            }
            else
            {
                Warning(tr("Glyph pick could not find a valid intersection."));
                return false;
            }
        }

        //
        // Most of the time, these will be -1, Except when picking
        // via PickByZone or PickByNode.
        //
        if (dom != -1 && el != -1)
        {
            pickAtts->SetDomain(dom);
            pickAtts->SetElementNumber(el);
            double dummyPt[3] = { FLT_MAX, 0., 0.};
            pickAtts->SetPickPoint(dummyPt);
            pickAtts->SetCellPoint(dummyPt);
        }

        if (pickAtts->GetDoTimeCurve() && pickAtts->GetTimePreserveCoord())
            return true;

        bool retry;
        int numAttempts = 0; 

        do
        {   
            retry = false;
            int networkId = plot->GetNetworkID();
            int windowId = plot->GetWindowId();

            TRY
            {
                PickAttributes pa = *pickAtts;
                bool createSpreadsheetSave = pa.GetCreateSpreadsheet();
                if (win->GetInteractionMode() == SPREADSHEET_PICK)
                    pa.SetCreateSpreadsheet(true);
                ViewerEngineManager::Instance()->UpdateExpressionsFromPlot(plot);
                ViewerEngineManager::Instance()->Pick(engineKey,
                                                      networkId, windowId,
                                                      &pa, pa);
                pa.SetCreateSpreadsheet(createSpreadsheetSave);
                if (pa.GetFulfilled())
                {
                   *pickAtts = pa;

                   //
                   // Reset the vars to what the user actually typed.
                   //
                   pickAtts->SetVariables(userVars);
                   pickAtts->SetInvalidVars(invalidVars);

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
                   if (pa.GetError())
                   {
                       pickAtts->SetError(pa.GetError());
                       pickAtts->SetErrorMessage(pa.GetErrorMessage());
                   }
                   UpdatePickAtts();
               
                   //SEND ERROR MESSAGE TO GUI WINDOW FOR DISPLAY
                   if (!pa.GetError())
                       Warning(tr("Pick failed with an internal error. "
                                  "Please contact a VisIt developer."));
                   else 
                       Warning(pa.GetErrorMessage().c_str(), false);
                }
            }
            CATCH2(VisItException, e)
            {
                if (e.GetExceptionType() == "NoEngineException" ||
                    e.GetExceptionType() == "LostConnectionException" ||
                    e.GetExceptionType() == "ImproperUseException")
                {
   
                    // Queries access the cached network used by the queried 
                    // plot.  Simply relaunching the engine does not work,
                    // as no network is created. This situation requires 
                    // re-execution of the plot that is being queried.
                    plot->ClearCurrentActor();
                    win->GetPlotList()->UpdateFrame(); 
                    retry = true;
                    numAttempts++; 
                }
                else 
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
                             e.Message().c_str());

                    Error(message, false);
                }
            }
            ENDTRY
        } while (retry && numAttempts < 2);
        if (numAttempts == 2 && !pickCache.empty())
        {
            // If Exceptions were encountered too many times, don't attempt
            // to process any more picks. 
            pickCache.clear();
            handlingCache = false;
        }
        else
        {
            // If there was an error, then there's no need to go on.
            if (pickAtts->GetError())
                return retval;

            // 
            // See if we are supposed to make a spreadsheet when we pick.
            // If so, detect to see if there is a spreadsheet plot in the 
            // current window.  If so, use it.  If not, create it.
            //
            bool doSpreadsheet = (pickAtts->GetCreateSpreadsheet()) ||
                (win->GetInteractionMode() == SPREADSHEET_PICK);
            if (doSpreadsheet)
            {
                int  i;

                if (!GetPlotPluginManager()->PluginAvailable(
                                                            "Spreadsheet_1.0"))
                {
                    static bool issuedWarning = false;
                    Error(tr("Could not create a spreadsheet with the pick, "
                          "because the spreadsheet plugin is not available."));
                    issuedWarning = true;
                    return retval;
                }
    
                // Get a reference to the plot we picked on.
                intVector plotIds;
                plist->GetActivePlotIDs(plotIds);
                ViewerPlot *plotWePicked = plist->GetPlot(plotIds[0]);

                string varname = plotWePicked->GetVariableName();
                bool useAnyVar = false;
                if (plotWePicked->IsMesh() || plotWePicked->IsLabel())
                {
                    useAnyVar = true;
                    varname = "";
                }

                // Decide if we should add a new spreadsheet, or re-use
                // an existing one.
                ViewerPlot *spreadsheet = NULL;
                for (i = 0 ; i < plist->GetNumPlots() ; i++)
                {
                     ViewerPlot *plot = plist->GetPlot(i);
                     if (strcmp(plot->GetPlotTypeName(), "Spreadsheet") != 0)
                         continue;
                     if (plot->GetHostName() != plotWePicked->GetHostName())
                         continue;
                     if (plot->GetDatabaseName() != 
                                               plotWePicked->GetDatabaseName())
                         continue;
            
                     // If we have a mesh or label plot, then accept any
                     // variable.  Otherwise require that the variables match.
                     if (!useAnyVar)
                         if (plot->GetVariableName() != 
                                               plotWePicked->GetVariableName())
                         continue;

                     // Make sure they are operating on the same domain.
                     const AttributeSubject *plotAtts = plot->GetPlotAtts();
                     PickAttributes *p = (PickAttributes *)
                         plotAtts->CreateCompatible("PickAttributes");
                     if (p != NULL)
                         if (p->GetSubsetName() != pickAtts->GetSubsetName())
                         {
                            // Special logic for one domain meshes, which
                            // use the special keyword "Whole", which comes
                            // from the spreadsheet attributes.
                            if ((p->GetSubsetName() != "Whole")
                                || (pickAtts->GetSubsetName() != ""))
                             continue;
                         }

                     // It is a spreadsheet plot of the same variable and the same
                     // domain for the same database from the same host. Re-use it.
                     spreadsheet = plot;
                }

                if (spreadsheet == NULL && useAnyVar)
                {
                    // If we have a mesh plot or label plot, then find a 
                    // suitable variable to plot.  Choose the variable of 
                    // any PC or contour plot.
                    for (i = 0 ; i < plist->GetNumPlots() ; i++)
                    {
                         ViewerPlot *plot = plist->GetPlot(i);
                         if (plot->GetHostName() !=plotWePicked->GetHostName())
                             continue;
                         if (plot->GetDatabaseName() != 
                                               plotWePicked->GetDatabaseName())
                             continue;
                         if (strcmp(plot->GetPlotTypeName(), "Pseudocolor")==0)
                         {
                             varname = plot->GetVariableName();
                             break;
                         }
                         if (strcmp(plot->GetPlotTypeName(), "Contour")==0)
                         {
                             varname = plot->GetVariableName();
                         }
                    }
                }

                if (spreadsheet == NULL && varname == "")
                {
                    // We didn't find a candidate spreadsheet and we couldn't
                    // come up with a candidate variable name.  Give up.
                    return retval;
                }

                ViewerWindow *win = (ViewerWindow *)pd.callbackData;
                ViewerPlotList *plotList =  win->GetPlotList();
                if (spreadsheet == NULL)
                {
                    int plotType = GetPlotPluginManager()
                                          ->GetEnabledIndex("Spreadsheet_1.0");
                    bool replacePlots = false;
                    int pid = plotList->AddPlot(plotType, varname,
                                                replacePlots, false, true);
                    vector<int> opListBogus;
                    vector<int> exListBogus;
                    bool shouldUseBogusInfo = false;
                    plist->SetActivePlots(plotIds, opListBogus, exListBogus,
                                          shouldUseBogusInfo);
                    spreadsheet = plotList->GetPlot(pid);
                }

                if (spreadsheet == NULL)
                {
                    // There was an error somewhere else ... varname not valid?
                    return retval;
                }

                // The spreadsheet plot can orient its normal to fit the
                // plot we picked.  So, for example, if we picked on a slice,
                // have the normal of the spreadsheet be the same as that
                // slice.
                for (int i = 0 ; i < plotWePicked->GetNOperators() ; i++)
                {
                     ViewerOperator *op = plotWePicked->GetOperator(i);
                     const AttributeSubject *atts = op->GetOperatorAtts();
                     AttributeSubject *s = 
                                     atts->CreateCompatible("PlaneAttributes");
                     if (s != NULL)
                         spreadsheet->SetPlotAtts(s);
                }

                // Now send in the pick attributes.
                spreadsheet->SetPlotAtts(pickAtts);

                TRY
                {
                    plotList->RealizePlots();
                    //
                    // If there was an error, the bad plot should not be left
                    // around muddying up the waters.
                    //
                    if (spreadsheet->GetErrorFlag())
                        plotList->DeletePlot(spreadsheet, false);
                }
                CATCH(VisItException)
                {
                    plotList->DeletePlot(spreadsheet, false);
                }
                ENDTRY
            }

            //
            // See if we got an array variable. If so, create a histogram plot.
            // Note we will only create one histogram. So if there are multiple
            // array variables, just choose the last one.
            //
            int np = pickAtts->GetNumVarInfos();
            bool haveArray = false;
            string arrayname;
            for (int i = 0 ; i < np ; i++)
            {
                PickVarInfo &info = pickAtts->GetVarInfo(i);
                if (info.GetVariableType() == "array")
                {
                    haveArray = true;
                    arrayname = info.GetVariableName();
                }
            }
            if (haveArray)
            {
                int winId = win->GetWindowId();
                ViewerWindow *resWin =
                       ViewerWindowManager::Instance()->GetTimeQueryWindow(-1);
                if (resWin == NULL)
                {
                    Error(tr("Please choose a different window to place the "
                             "histogram of the array variable into."));
                    return retval;
                }
        
                if (!GetPlotPluginManager()->PluginAvailable(
                                                              "Histogram_1.0"))
                {
                    static bool issuedWarning = false;
                    Error(tr("Could not create a histogram of the array "
                             "variable, because the Histogram plugin is not "
                             "available."));
                    issuedWarning = true;
                    return retval;
                }
    
                int plotType = GetPlotPluginManager()
                                            ->GetEnabledIndex("Histogram_1.0");
                ViewerPlotList *plotList =  resWin->GetPlotList();
        
                string hdbName = plot->GetSource();
                plotList->SetHostDatabaseName(hdbName);
                plotList->SetEngineKey(engineKey);
                bool replacePlots = true;
                if (plist->TimeSliderExists(hdbName))
                {
                    int state, nStates;
                    plist->GetTimeSliderStates(hdbName, state, nStates);
                    plotList->CreateTimeSlider(hdbName, state);
                    plotList->SetActiveTimeSlider(hdbName);
                }
                int pid = plotList->AddPlot(plotType,arrayname,
                                            replacePlots,false, true);
                ViewerPlot *resultsPlot = plotList->GetPlot(pid);
    
                // This is a bit subtle.  We are sending the pick attributes in
                // as the plot attributes.  Normally, plot plugins blindly copy
                // the attributes from this method.  But the histogram plot is
                // smart enough to check to see if the atts are histogram atts.
                // If they are not histogram atts, and they *are* pick atts, 
                // then it will pull out the zone and domain.  Note that this 
                // is a very similar game plan to what lineout does for the 
                // curve plot.
                resultsPlot->SetPlotAtts(pickAtts);
    
                if (resultsPlot == NULL)
                {
                    // There was an error somewhere else...arrayname not valid?
                    return retval;
                }
                TRY
                {
                    plotList->RealizePlots();
                    //
                    // If there was an error, the bad curve plot should not be 
                    // left around muddying up the waters.
                    //
                    if (resultsPlot->GetErrorFlag())
                        plotList->DeletePlot(resultsPlot, false);
                }
                CATCH(VisItException)
                {
                    plotList->DeletePlot(resultsPlot, false);
                }
                ENDTRY
            }
        }
    }
    else
    {
        Warning(tr("The picked point is not contained in a surface"));
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
//   Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004
//   Call PickThroughTime when necessary.
//   
//   Kathleen Bonnell, Wed Jun  2 10:00:35 PDT 2004 
//   Only add a pick letter if a valid position could be determined. 
//   
//   Kathleen Bonnell, Tue Oct 12 16:01:49 PDT 2004 
//   Save and restore the current pick type (certain types of pick may
//   change it during execution). 
//   
//   Kathleen Bonnell, Tue Dec 28 14:36:44 PST 2004
//   Allow pick letter to NOT be displayed. 
//
//   Kathleen Bonnell, Thu Jul 14 09:16:22 PDT 2005 
//   Test for existence of engine before proceeding. 
//    
//   Kathleen Bonnell, Tue Aug 16 10:03:27 PDT 2005
//   Check for non-hidden active plot before checking if EngineExists. 
//
//   Kathleen Bonnell, Fri Dec  7 10:43:07 PST 2007 
//   Don't send message if QueryOutput is suppressed. 
//
//   Brad Whitlock, Tue Apr 29 16:40:45 PDT 2008
//   Support for internationalization.
//
//   Gunther H. Weber, Fri Aug 15 10:17:05 PDT 2008
//   Check whether reusePickLetter flag in PickAttributes is set and  
//   do not advance pick designtor if it is.
//
// ****************************************************************************

void
ViewerQueryManager::Pick(PICK_POINT_INFO *ppi, const int dom, const int el)
{
    // test for non-hidden active plot and running engine
    ViewerWindow *win = (ViewerWindow *)ppi->callbackData;
    ViewerPlotList *plist = win->GetPlotList();
    intVector plotIds;
    plist->GetActivePlotIDs(plotIds);
    if (plotIds.size() > 0)
    {
        if (!EngineExistsForQuery(plist->GetPlot(plotIds[0])))
        {
            return;
        }
    }
    else
    {
        Error(tr("Pick requires an active non-hidden Plot.\n"
                 "Please select a plot and try again.\n"));
        return ;
    }
 
    if (pickAtts->GetDoTimeCurve())
    {
        PickThroughTime(ppi, dom, el);
        return;
    }
    PickAttributes::PickType oldPickType = pickAtts->GetPickType();
    if(ComputePick(ppi, dom, el))
    {
        //
        // Add a pick point to the window
        //
        if (pickAtts->GetDisplayPickLetter() && 
            pickAtts->GetPickPoint()[0] != FLT_MAX)
        {
            win->ValidateQuery(pickAtts, NULL);
        } // else no valid position could be determined, data was transfored

        //
        // Send pick attributes to the client.
        //
        string msg;
        pickAtts->CreateOutputString(msg);
        if ( pickAtts->GetPickPoint()[0] == FLT_MAX &&
             pickAtts->GetPickType() != PickAttributes::CurveNode &&
             pickAtts->GetPickType() != PickAttributes::CurveZone)
        {
            string append;
            if (pickAtts->GetPickType() == PickAttributes::Zone  ||
                pickAtts->GetPickType() == PickAttributes::DomainZone) 
            {
                append = "Mesh was transformed and chosen zone is not " 
                       "part of transformed mesh.\nNo pick letter will " 
                       "be displayed.";
            }
            else 
            {
                append = "Mesh was transformed and chosen node is not " 
                       "part of transformed mesh.\nNo pick letter will " 
                       "be displayed.";
            }
            msg += append;
        }
        if (!suppressQueryOutput)
            Message(msg.c_str(), false); 
        UpdatePickAtts();

        //
        // 
        //
        //
        // If we are not reusing a pick letter, make the pick label ready for the next pick point.
        //
        if (!pickAtts->GetReusePickLetter()) UpdateDesignator();
    }

    //
    // In case it was changed in the process. (Mostly likely by picking
    // on a VectorPlot or a PointMesh.
    //
    pickAtts->SetPickType(oldPickType);

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
//   Kathleen Bonnell, Fri Jul  9 16:55:32 PDT 2004 
//   Renamed to 'StartLineout'.   Attempt to get Lineout window.  Save off
//   information to lineoutCache for use during FinishLineout.
// 
//    Kathleen Bonnell, Mon Jul 26 17:45:12 PDT 2004
//    Suspend socket signals in the viewer so that Lineout does not cause 
//    synchronization events to be processed before we are ready for them. 
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
//    Brad Whitlock, Tue Apr 29 16:42:21 PDT 2008
//    Support for internationalization.
//
// ****************************************************************************

void
ViewerQueryManager::StartLineout(ViewerWindow *win, bool fromDefault)
{
    viewerSubject->BlockSocketSignals(true);
    if(operatorFactory == 0)
        return;

    if (win->GetWindowMode() == WINMODE_CURVE)
    {
        Error(tr("Lineout cannot be performed on curve windows."));
        return;
    }
    if (win->GetWindowMode() == WINMODE_AXISARRAY)
    {
        Error(tr("Lineout cannot be performed on axis array windows."));
        return;
    }

    //
    // Can we get a lineout window? 
    //
    int useThisId = (globalLineoutAtts->GetCreateWindow() ? -1 :
                     globalLineoutAtts->GetWindowId() -1);
    ViewerWindow *resWin = 
        ViewerWindowManager::Instance()->GetLineoutWindow(useThisId);
    if (resWin == NULL)
    {
        ResetLineoutCache();
        return;
    }

    int type = GetOperatorPluginManager()->GetEnabledIndex("Lineout_1.0"); 
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
        delete line;
        Error(tr("Only 2D points allowed for 2D lineouts. "
                 "Please set z-coord to 0."));
        return;
    }

    // Save the information necessary for the lineout to finish.
    lineoutCache.origWin = win;
    lineoutCache.line = *line;
    lineoutCache.fromDefault = fromDefault;
    lineoutCache.resWinId = resWin->GetWindowId();

    delete line;
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
//  Modifications:
//   Kathleen Bonnell, Fri Feb  4 07:10:27 PST 2005
//   Use new GlobalLineout atts to set flags for lineouts, for following time,
//   and using a time slider.
//
//   Kathleen Bonnell, Tue Jan 17 11:30:15 PST 2006
//   Removed call to SetLineoutsTimeSlider.
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
        bool goingDynamic = globalLineoutClientAtts->GetDynamic();
        bool creatingCurve = globalLineoutClientAtts->GetCurveOption() ==
                             GlobalLineoutAttributes::CreateCurve;
        if (globalLineoutAtts->GetDynamic() != goingDynamic)
        {
            SetDynamicLineout(goingDynamic);
            SetLineoutsFollowTime(!(goingDynamic && creatingCurve));
        }
        else if (globalLineoutAtts->GetCurveOption() !=
            globalLineoutClientAtts->GetCurveOption())
        {
            SetLineoutsFollowTime(!(goingDynamic && creatingCurve));
        }
        *globalLineoutAtts = *globalLineoutClientAtts;
    }
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
//    Kathleen Bonnell, Wed Jun  2 10:00:35 PDT 2004
//    Allow 'initialPick' to be true when pick mode changes from a zone-type
//    to a node-type or vice-versa, as each type has different requirements
//    for re-execution of picked plot.
//
//    Kathleen Bonnell, Wed Nov 28 09:58:23 PST 2007 
//    Set pick type in client atts, too, in case pick atts change while
//    still in pick mode, so we don't lose the pick type. 
//
// ****************************************************************************

void
ViewerQueryManager::StartPickMode(const bool firstEntry, const bool zonePick)
{
    if (firstEntry)
        initialPick = true;
    else
    {
        int ptype = pickAtts->GetPickType();
        bool isZone = (ptype == PickAttributes::Zone ||
                      ptype == PickAttributes::DomainZone ||
                      ptype == PickAttributes::CurveZone );
        if (isZone != zonePick)
        {
            initialPick = true;
        }
    }

    if (zonePick)
    {
        pickAtts->SetPickType(PickAttributes::Zone);
        pickClientAtts->SetPickType(PickAttributes::Zone);
    }
    else 
    {
        pickAtts->SetPickType(PickAttributes::Node);
        pickClientAtts->SetPickType(PickAttributes::Node);
    }
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
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004
//    Added bool arg to support queries-over-time. 
//
//    Kathleen Bonnell, Wed Sep  8 09:36:30 PDT 2004 
//    Renamed 'Pick' to 'ScreenZonePick', 'NodePick' to 'ScreenNodePick' and
//    'WorldPick' to 'Pick', 'WorldNodePick' to 'NodePick'.
//    
//    Kathleen Bonnell, Thu Dec 16 17:32:49 PST 2004 
//    Added 'elementIsGlobal' arg, use to set same attribute in pickAtts. 
//
//    Kathleen Bonnell, Thu Jul 14 09:16:22 PDT 2005 
//    Test for existence of engine before proceeding. 
//    
//    Kathleen Bonnell, Tue Aug 16 10:03:27 PDT 2005
//    Check for non-hidden active plot before checking if EngineExists. 
//
//    Kathleen Bonnell, Tue May  9 15:45:04 PDT 2006 
//    Added a couple of Pick aliases. 
//
//    Kathleen Bonnell, Wed Nov 28 16:25:54 PST 2007 
//    Added timeCurve bool.  For Node/Zone picks-through-time, preserve current
//    values of pickAtts DoTimeCurve and TimePreserveCoord as they both should
//    be true in this instance.  For PickByNode/Zone, TimePreserveCoord should
//    be false.
// 
//    Brad Whitlock, Tue Apr 29 16:43:32 PDT 2008
//    Support for internationalization.
//
//    Kathleen Bonnell, Tue Jun 24 09:04:23 PDT 2008 
//    Reset the vars in pickAtts at the end of this method, to prevent
//    query-set-vars to be applied to picks.
//
// ****************************************************************************

void         
ViewerQueryManager::PointQuery(const string &qName, const double *pt, 
                    const stringVector &vars, const int arg1, const int arg2,
                    const bool doTime, const bool elementIsGlobal)
{
    // test for non-hidden active plot and running engine
    ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerPlotList *plist = win->GetPlotList();
    intVector plotIds;
    plist->GetActivePlotIDs(plotIds);
    bool timeCurve = doTime || pickAtts->GetDoTimeCurve();
    if (plotIds.size() > 0)
    {
        if (!EngineExistsForQuery(plist->GetPlot(plotIds[0])))
        {
            return;
        }
    }
    else
    {
        Error(tr("Pick requires an active non-hidden Plot.\n"
                 "Please select a plot and try again.\n"));
        return ;
    }
    pickAtts->SetElementIsGlobal(elementIsGlobal);
    if (qName == "ScreenZonePick")
    {
        if (!vars.empty())
        {
            pickAtts->SetVariables(vars);
        }
        win->Pick((int)pt[0], (int)pt[1], ZONE_PICK);
    }
    else if (qName == "ScreenNodePick") 
    {
        if (!vars.empty())
        {
            pickAtts->SetVariables(vars);
        }
        win->Pick((int)pt[0], (int)pt[1], NODE_PICK);
    }
    else if (qName == "Pick" || qName == "ZonePick (aka Pick)" || 
             qName == "NodePick" || qName == "PointPick (aka NodePick)")
    {
        if (!vars.empty())
        {
            pickAtts->SetVariables(vars);
        }

        INTERACTION_MODE imode  = win->GetInteractionMode();
        if (qName == "Pick" || qName == "ZonePick (aka Pick)")
            win->SetInteractionMode(ZONE_PICK);
        else
            win->SetInteractionMode(NODE_PICK);

        PICK_POINT_INFO ppi;
        ppi.callbackData = win;
        ppi.rayPt1[0] = ppi.rayPt2[0] = pt[0];
        ppi.rayPt1[1] = ppi.rayPt2[1] = pt[1];
        ppi.rayPt1[2] = ppi.rayPt2[2] = pt[2];
        ppi.validPick = true;
        if (!timeCurve)
            Pick(&ppi);
        else
        {
            bool tpc = pickAtts->GetTimePreserveCoord();
            bool tc = pickAtts->GetDoTimeCurve();
            pickAtts->SetTimePreserveCoord(true);
            pickAtts->SetDoTimeCurve(true);
            PickThroughTime(&ppi);
            pickAtts->SetDoTimeCurve(tc);
            pickAtts->SetTimePreserveCoord(tpc);
        }

        win->SetInteractionMode(imode);
    }
    else if (qName == "PickByZone"  || qName == "PickByNode") 
    {
        if (!vars.empty())
        {
            pickAtts->SetVariables(vars);
        }

        INTERACTION_MODE imode  = win->GetInteractionMode();
        if (qName == "PickByZone") 
        {
            win->SetInteractionMode(ZONE_PICK);
            pickAtts->SetPickType(PickAttributes::DomainZone);
        }
        else
        {
            win->SetInteractionMode(NODE_PICK);
            pickAtts->SetPickType(PickAttributes::DomainNode);
        }
        PICK_POINT_INFO ppi;
        ppi.callbackData = win;
        ppi.rayPt1[0] = ppi.rayPt2[0] = pt[0];
        ppi.rayPt1[1] = ppi.rayPt2[1] = pt[1];
        ppi.rayPt1[2] = ppi.rayPt2[2] = pt[2];
        ppi.validPick = true;

        if (!timeCurve)
            Pick(&ppi, arg2, arg1);
        else
        {
            bool tpc = pickAtts->GetTimePreserveCoord();
            bool tc = pickAtts->GetDoTimeCurve();
            pickAtts->SetTimePreserveCoord(false);
            pickAtts->SetDoTimeCurve(true);
            PickThroughTime(&ppi, arg2, arg1);
            pickAtts->SetTimePreserveCoord(tpc);
            pickAtts->SetDoTimeCurve(tc);
        }

        win->SetInteractionMode(imode);
    }
    pickAtts->SetElementIsGlobal(false);
    stringVector emptyVars;
    emptyVars.push_back("default");
    pickAtts->SetVariables(emptyVars);
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
//   Kathleen Bonnell, Fri Jul  9 16:55:32 PDT 2004 
//   Renamed to 'StartLineout'.   Save off information to lineoutCache 
//   for use during FinishLineout.
//
//   Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//   Added call to create (get) the lineout window
//
//   Kathleen Bonnell, Thu Jul 22 15:43:56 PDT 2004 
//   Added useThisId arg to GetLineoutWindow call, so that a user-specified
//   window can be retrieved/created. Save resWin's id in lineoutCache for
//   use later by Finish routine.
//
//    Kathleen Bonnell, Mon Jul 26 17:45:12 PDT 2004
//    Suspend socket signals in the viewer so that Lineout does not cause 
//    synchronization events to be processed before we are ready for them. 
//
// ****************************************************************************

void
ViewerQueryManager::StartLineout(ViewerWindow *origWin, Line *lineAtts)
{
    viewerSubject->BlockSocketSignals(true);
    //
    // Can we get a lineout window? 
    //
    int useThisId = (globalLineoutAtts->GetCreateWindow() ? -1 :
                     globalLineoutAtts->GetWindowId()-1);

    ViewerWindow *resWin = 
        ViewerWindowManager::Instance()->GetLineoutWindow(useThisId);
    if (resWin == NULL)
    {
        ResetLineoutCache();
        return;
    }

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
    lineoutCache.origWin = origWin;
    lineoutCache.line = *lineAtts;
    lineoutCache.fromDefault = true;
    lineoutCache.resWinId = resWin->GetWindowId();
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
//   parentNode    : The node where the query manager's data is stored.
//   configVersion : The version from the config file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 22 10:16:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerQueryManager::SetFromNode(DataNode *parentNode, 
    const string &configVersion)
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
//    Kathleen Bonnell, Tue Aug 24 14:59:16 PDT 2004
//    If one of the vars is 'all', reteive all variables defined on Mesh,
//    and all expression vars, too, and add them to the list.
// 
// ****************************************************************************

void
GetUniqueVars(const stringVector &vars, const string &activeVar,
              stringVector &uniqueVars, const avtDatabaseMetaData *md)
{
    if (vars.size() == 0)
    {
        uniqueVars.push_back(activeVar);
        return;
    }
    int i, j;
    set<string> uniqueVarsSet;
    for (i = 0; i < vars.size(); i++)
    {
        string v = vars[i];
        if (v == "default")
        {
            v = activeVar;
        }
        if ((v == "all" || v == "All") && md != NULL)
        {
            stringVector dbVars = md->GetAllVariableNames(activeVar);
            const ExpressionList *exprList = ParsingExprList::Instance()->GetList();
            stringVector exprVars = exprList->GetAllVarNames(md->GetDatabaseName());
            for (j = 0; j < dbVars.size(); j++)
            {
                if (uniqueVarsSet.count(dbVars[j]) == 0) 
                {
                    uniqueVars.push_back(dbVars[j]);
                    uniqueVarsSet.insert(dbVars[j]); 
                }
            }
            for (j = 0; j < exprVars.size(); j++)
            {
                if (uniqueVarsSet.count(exprVars[j]) == 0) 
                {
                    uniqueVars.push_back(exprVars[j]);
                    uniqueVarsSet.insert(exprVars[j]); 
                }
            }
        }
        else if (uniqueVarsSet.count(v) == 0)
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
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004
//    Made some queries 'time' queries (can perform queries-over-time). 
//   
//    Hank Childs, Tue Apr 13 12:45:33 PDT 2004
//    Do a better job distinguishing between 2D and 3D area.
//
//    Kathleen Bonnell, Tue May 25 16:09:15 PDT 2004 
//    Added Zone center.
//
//    Kathleen Bonnell, Sat Sep  4 11:44:00 PDT 2004 
//    Parameters required for AddQuery have changed. 
//
//    Kathleen Bonnell, Wed Sep  8 10:33:24 PDT 2004 
//    Removed references to QueryList::CoordinateRepresentation, no longer 
//    exists.   Removed screen-coords pick 'Pick' and 'NodePick', changed
//    world-space picks 'WorldPick'  and 'WorldNodePick' to 'Pick' and
//    'NodePick' respectively.
//
//    Kathleen Bonnell, Thu Dec 16 17:32:49 PST 2004 
//    Added two new window states, to distinguish between Domain-zone/node
//    queries that take vars and those that do not.  Removed 'Variabley by
//    Zone' and 'Variable by Node' queries, as those are coved by 
//    PickByNode and PickByZone.
//    exists.   Removed screen-coords pick 'Pick' and 'NodePick', changed
//
//    Hank Childs, Thu May 19 14:26:48 PDT 2005
//    Added centroid, moment of inertia queries.
//
//    Hank Childs, Fri Aug  5 09:49:12 PDT 2005
//    Added kurtosis, skewness.
//
//    Hank Childs, Fri Sep 23 16:16:28 PDT 2005
//    Added watertight query.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Reflect changes to QueryList, timeQuery is now QueryMode,
//    Queries can specify the number of vars required. 
//    Added TrajectoryByNode/Zone.
//
//    Brad Whitlock, Thu Nov 17 10:20:06 PDT 2005
//    Added Best Fit Line query.
//
//    Hank Childs, Sat Apr 29 14:40:47 PDT 2006
//    Added localized and elliptical compactness factor queries.
//
//    Kathleen Bonnell, Tue May  9 15:45:04 PDT 2006 
//    Added a couple of Pick aliases. 
//
//    Hank Childs, Thu May 11 13:24:34 PDT 2006
//    Added average mean curvature query.
//
//    Hank Childs, Sat Jul  8 11:24:45 PDT 2006
//    Added chord length distribution query.
//
//    Hank Childs, Thu Jul 20 11:23:07 PDT 2006
//    Add mass distribution query.
//
//    Hank Childs, Mon Aug  7 18:05:38 PDT 2006
//    Add distance from boundary query.
//
//    Hank Childs, Fri Aug 25 15:42:12 PDT 2006
//    Add expected value query.
//
//    Hank Childs, Mon Aug 28 16:56:43 PDT 2006
//    Added the "ShapeRelated" group of queries and added some new chord 
//    and ray length distribution queries.
//
//    Dave Bremer, Fri Sep  8 11:32:33 PDT 2006
//    Added line scan transform.
//
//    Kathleen Bonnell, Fri Sep 15 09:23:50 PDT 2006
//    Added Volume2, which calculates hex-volumes differently than
//    Volume (which uses verdict).
//
//    Hank Childs, Fri Nov  3 15:49:40 PST 2006
//    Added total length query.
//
//    Dave Bremer, Fri Dec  8 17:52:22 PST 2006
//    Added hohlraum flux query.
//
//    Cyrus Harrison, Tue Feb 20 15:27:36 PST 2007
//    Added connected components queries
//    
//    Cyrus Harrison,  Thu Mar  1 16:16:58 PST 2007
//    Added connected components summary query
//
//    Kathleen Bonnell, Thu Aug  9 08:40:56 PDT 2007 
//    Made 'Revolved surface area' and 'Revolved volume' be time-enabled. 
//
//    Kathleen Bonnell, Fri Sep 28 14:52:09 PDT 2007 
//    Made 'Volume2' be a non-public query (won't be available in window).
//
//    Cyrus Harrison, Tue Dec 18 14:18:07 PST 2007
//    Added Shapelet Decomposition Query
//
//    Cyrus Harrison, Wed Mar  5 16:07:09 PST 2008
//    Added Memory Usage Query.
//
//    Cyrus Harrison, Wed Mar  5 16:07:09 PST 2008
//    Added Memory Usage Query.
//
//    Cyrus Harrison, Wed Jul 16 12:05:54 PDT 2008
//    Moved the Shapelet Decomposition Query into the Shape-Related category.
//
//    Jeremy Meredith, Wed Mar 11 17:53:13 EDT 2009
//    Added Sample and Population statistics queries.
//
// ****************************************************************************

void
ViewerQueryManager::InitializeQueryList()
{
    //
    // Args, in order, for QueryList::AddQuery:
    //    string                                 queryName
    //    QueryList::QueryType                   queryType
    //    QueryList::Groups                      group
    //    QueryList::WindowType                  winType
    //    int                                    numInputs
    //    int                                    allowedVarTypes
    //    QueryList::QueryMode                   queryMode
    //    int                                    numRequiredVars (optional)
    //                                           default is 1
    //

    QueryList::QueryType pq = QueryList::PointQuery;
    QueryList::QueryType dq = QueryList::DatabaseQuery;
    QueryList::QueryType lq = QueryList::LineQuery;

    QueryList::Groups cr = QueryList::CurveRelated;
    QueryList::Groups mr = QueryList::MeshRelated;
    QueryList::Groups pr = QueryList::PickRelated;
    QueryList::Groups tr = QueryList::TimeRelated;
    QueryList::Groups vr = QueryList::VariableRelated;
    QueryList::Groups sr = QueryList::ShapeRelated;
    QueryList::Groups ccl_r  = QueryList::ConnectedComponentsRelated;
    QueryList::Groups misc_r = QueryList::Miscellaneous;
    
    QueryList::WindowType basic = QueryList::Basic;
    QueryList::WindowType sp  = QueryList::SinglePoint;
    QueryList::WindowType dp  = QueryList::DoublePoint;
    QueryList::WindowType dn  = QueryList::DomainNode;
    QueryList::WindowType dnv = QueryList::DomainNodeVars;
    QueryList::WindowType dz  = QueryList::DomainZone;
    QueryList::WindowType dzv = QueryList::DomainZoneVars;
    QueryList::WindowType ad  = QueryList::ActualData;
    QueryList::WindowType ld  = QueryList::LineDistribution;
    QueryList::WindowType hf  = QueryList::HohlraumFlux;
    QueryList::WindowType ccls_wt = QueryList::ConnCompSummary;
    QueryList::WindowType shp_wt  = QueryList::ShapeletsDecomp;
    //QueryList::WindowType av = QueryList::ActualDataVars;

    QueryList::QueryMode qo = QueryList::QueryOnly;
    QueryList::QueryMode qt = QueryList::QueryAndTime;
    QueryList::QueryMode to = QueryList::TimeOnly;

    if (GetPlotPluginManager()->PluginAvailable("Curve_1.0") &&
        GetOperatorPluginManager()->PluginAvailable("Lineout_1.0"))
    {
        queryTypes->AddQuery("Lineout", lq, vr, dp, 1, 0, qo);
    }
    queryTypes->AddQuery("Eulerian", dq, mr, basic, 1, 0, qo);
    queryTypes->AddQuery("Compactness", dq, sr, basic, 1, 0, qo);
    queryTypes->AddQuery("Cycle", dq, tr, basic, 1, 0, qo);
    queryTypes->AddQuery("Time", dq, tr, basic, 1, 0, qo);
    queryTypes->AddQuery("L2Norm", dq, cr, basic, 1, 0, qo);
    queryTypes->AddQuery("Kurtosis", dq, cr, basic, 1, 0, qo);
    queryTypes->AddQuery("Chord Length Distribution (individual)", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Chord Length Distribution (aggregate)", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Ray Length Distribution (individual)", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Ray Length Distribution (aggregate)", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Mass Distribution", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Distance From Boundary", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Line Scan Transform", dq, sr, ld, 1, 0, qo);
    queryTypes->AddQuery("Hohlraum Flux", dq, sr, hf, 1, 0, qt, 0);
    queryTypes->AddQuery("Skewness", dq, cr, basic, 1, 0, qo);
    queryTypes->AddQuery("Integrate", dq, cr, basic, 1, 0, qo);
    queryTypes->AddQuery("Expected Value", dq, cr, basic, 1, 0, qo);
    queryTypes->AddQuery("L2Norm Between Curves", dq, cr, basic, 2, 0, qo);
    queryTypes->AddQuery("Area Between Curves", dq, cr, basic, 2, 0, qo);
    queryTypes->AddQuery("Revolved volume", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("Revolved surface area", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("2D area", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("3D surface area", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("Volume", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("Volume2", dq, mr, basic, 1, 0, qt);
    intVector &cbp = queryTypes->GetCanBePublic();
    cbp[cbp.size()-1] = 0;
    queryTypes->AddQuery("Total Length", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("Moment of Inertia", dq, mr, basic, 1, 0, qo);
    queryTypes->AddQuery("Centroid", dq, mr, basic, 1, 0, qo);
    queryTypes->AddQuery("Localized Compactness Factor", dq, sr, basic, 1, 0, qt);
    queryTypes->AddQuery("Elliptical Compactness Factor", dq, sr, basic, 1, 0, qt);
    queryTypes->AddQuery("Spherical Compactness Factor", dq, sr, basic, 1, 0, qt);
    queryTypes->AddQuery("Average Mean Curvature", dq, mr, basic, 1, 0, qt);
    queryTypes->AddQuery("Variable Sum", dq, vr, basic, 1, 0, qt);
    queryTypes->AddQuery("Watertight", dq, mr, basic, 1, 0, qo);
    queryTypes->AddQuery("Weighted Variable Sum", dq, vr, basic, 1, 0, qt);
    queryTypes->AddQuery("Pick", pq, pr, sp, 1, 0, qt);
    queryTypes->AddQuery("ZonePick (aka Pick)", pq, pr, sp, 1, 0, qt);
    queryTypes->AddQuery("NodePick", pq, pr, sp, 1, 0, qt);
    queryTypes->AddQuery("PointPick (aka NodePick)", pq, pr, sp, 1, 0, qt);

    queryTypes->AddQuery("Number of Connected Components", dq, ccl_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Connected Component Area", dq, ccl_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Connected Component Centroids", dq, ccl_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Connected Component Volume", dq, ccl_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Connected Component Variable Sum", dq, ccl_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Connected Component Weighted Variable Sum", dq, ccl_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Connected Components Summary", dq, ccl_r, ccls_wt, 1, 0, qo);
    
    queryTypes->AddQuery("Shapelet Decomposition", dq, sr, shp_wt, 1, 0, qo);


    int MinMaxVars = QUERY_SCALAR_VAR | QUERY_TENSOR_VAR | QUERY_VECTOR_VAR |
            QUERY_SYMMETRIC_TENSOR_VAR | QUERY_MATSPECIES_VAR | QUERY_CURVE_VAR;

    queryTypes->AddQuery("MinMax", dq, vr, ad, 1, MinMaxVars, qo);
    queryTypes->AddQuery("Min", dq, vr, ad, 1, MinMaxVars, qt);
    queryTypes->AddQuery("Max", dq, vr, ad, 1, MinMaxVars, qt);
    queryTypes->AddQuery("SpatialExtents", dq, mr, ad, 1, 0, qo);
    queryTypes->AddQuery("NumNodes", dq, mr, ad, 1, 0, qo);
    queryTypes->AddQuery("NumZones", dq, mr, ad, 1, 0, qo);
    queryTypes->AddQuery("PickByZone", pq, pr, dzv, 1, 0, qt);
    queryTypes->AddQuery("PickByNode", pq, pr, dnv, 1, 0, qt);
    queryTypes->AddQuery("Zone Center", dq, mr, dz, 1, 0, qo);
    queryTypes->AddQuery("Node Coords", dq, mr, dn, 1, 0, qo);
    int TrajVars = QUERY_SCALAR_VAR;
    queryTypes->AddQuery("TrajectoryByZone", dq, vr, dzv, 1, TrajVars, to, 2);
    queryTypes->AddQuery("TrajectoryByNode", dq, vr, dnv, 1, TrajVars, to, 2);
    queryTypes->AddQuery("Best Fit Line", dq, mr, basic, 1, 0, qo);
    queryTypes->AddQuery("Memory Usage", dq, misc_r, basic, 1, 0, qo);
    queryTypes->AddQuery("Sample Statistics", dq, vr, ad, 1, 0, qo);
    queryTypes->AddQuery("Population Statistics", dq, vr, ad, 1, 0, qo);
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
//    Eric Brugger, Tue Jul 27 09:31:29 PDT 2004
//    Change an integer constant to a double constant to fix a compile error.
//
// ****************************************************************************

int
ViewerQueryManager::VerifyQueryVariables(const string &qName, 
     const intVector &varTypes)
{
    int i, badIndex = -1;
    int allowedTypes = queryTypes->AllowedVarsForQuery(qName);
    if (allowedTypes > 0)
    {
        for (i = 0; i < varTypes.size() && badIndex == -1; i++)
        {
           int vt = (int) pow(2.0, varTypes[i]);
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
//    extents       The extents array to be converted to a string.
//    dim           The dimension of the extents.
//    type          The type of the extents.
//    float_format  Floating point format string.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 26, 2003 
//
//  Modifications:
//
//    Cyrus Harrison, Wed Sep 19 08:34:46 PDT 2007
//    Added support for user settable floating point format string
//
// ****************************************************************************

string
CreateExtentsString(const double * extents, 
                    const int dim, 
                    const char *type,
                    const string &float_format)
{
    string format ="";
    char msg[1024];
    if (dim == 1)
    {
        format = "The %s extents are (" + float_format + ", "
                                        + float_format  +")";
        
        SNPRINTF(msg, 1024, format.c_str(), type,
                extents[0], extents[1]);
    }
    else if (dim == 2)
    {
        format = "The %s extents are (" + float_format + ", "
                                        + float_format + ", "
                                        + float_format + ", "
                                        + float_format  +")";
        SNPRINTF(msg, 1024, format.c_str(), type,
            extents[0], extents[1], extents[2], extents[3]);
    }
    else if (dim == 3)
    {
        format = "The %s extents are (" + float_format + ", "
                                        + float_format + ", "
                                        + float_format + ", "
                                        + float_format + ", "
                                        + float_format + ", "                                                                          + float_format  +")";
        SNPRINTF(msg, 1024, format.c_str(), type,
            extents[0], extents[1], extents[2], extents[3], extents[4], extents[5]);
    }
    string msg2 = msg;
    return msg2;
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetQueryOverTimeDefaultAtts
//
//  Purpose:
//    Returns a pointer to the default query over time attributes.
//
//  Returns:    A pointer to the default query over timeattributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

QueryOverTimeAttributes *
ViewerQueryManager::GetQueryOverTimeDefaultAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (timeQueryDefaultAtts == 0)
    {
        timeQueryDefaultAtts = new QueryOverTimeAttributes;
    }
    return timeQueryDefaultAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::GetQueryOverTimeClientAtts
//
//  Purpose:
//    Returns a pointer to the queryovertime client attributes.
//
//  Returns:    A pointer to the queryovertime client attributes.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

QueryOverTimeAttributes *
ViewerQueryManager::GetQueryOverTimeClientAtts()
{
    //
    // If the attributes haven't been allocated then do so.
    //
    if (timeQueryClientAtts == 0)
    {
        timeQueryClientAtts = new QueryOverTimeAttributes;
    }
    return timeQueryClientAtts;
}


// ****************************************************************************
// Method: ViewerQueryManager::GetQueryOverTimeAtts
//
// Purpose: 
//   Returns the time query attributes.
//
// Returns:    A pointer to the time query attributes.
//
// Programmer: Kathleen Bonnell 
// Creation:   March 24, 2004 
//
// Modifications:
//   
// ****************************************************************************

QueryOverTimeAttributes *
ViewerQueryManager::GetQueryOverTimeAtts()
{
    if(timeQueryAtts == 0)
        timeQueryAtts = new QueryOverTimeAttributes;

    return timeQueryAtts;
}


// ****************************************************************************
//  Method: ViewerQueryManager::SetClientQueryOverTimeAttsFromDefault
//
//  Purpose:
//    Copies the default time query attributes into the client time query
//     attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerQueryManager::SetClientQueryOverTimeAttsFromDefault()
{
    if (timeQueryDefaultAtts != 0 && timeQueryClientAtts != 0)
    {
        *timeQueryAtts = *timeQueryDefaultAtts;

        *timeQueryClientAtts = *timeQueryDefaultAtts;
         timeQueryClientAtts->Notify();
    }
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetDefaultQueryOverTimeAttsFromClient
//
//  Purpose:
//    Sets the default timeQueryAtts using the client time query attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerQueryManager::SetDefaultQueryOverTimeAttsFromClient()
{
    if (timeQueryDefaultAtts != 0 && timeQueryClientAtts != 0)
    {
        *timeQueryDefaultAtts = *timeQueryClientAtts;
    }
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetQueryOverTimeAttsFromClient
//
//  Purpose:
//    Sets the timeQueryAtts using the client time query attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerQueryManager::SetQueryOverTimeAttsFromClient()
{
    if (timeQueryAtts == 0)
    {
        timeQueryAtts = new QueryOverTimeAttributes;
    }

    *timeQueryAtts = *timeQueryClientAtts;
}

// ****************************************************************************
//  Method: ViewerQueryManager::SetQueryOverTimeAttsFromDefault
//
//  Purpose:
//    Sets the timeQueryAtts using the default time query attributes. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerQueryManager::SetQueryOverTimeAttsFromDefault()
{
    if (timeQueryAtts == 0)
    {
        timeQueryAtts = new QueryOverTimeAttributes;
    }

    *timeQueryAtts = *timeQueryDefaultAtts;
    UpdateQueryOverTimeAtts();
}


// ****************************************************************************
//  Method: ViewerQueryManager::UpdateQueryOverTimeAtts
//
//  Purpose:
//    Causes the timeQueryAtts to be sent to the client. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerQueryManager::UpdateQueryOverTimeAtts()
{
    if (timeQueryClientAtts != 0 && timeQueryAtts != 0)
    {
        *timeQueryClientAtts = *timeQueryAtts;
        timeQueryClientAtts->Notify();
    }
}


// ***********************************************************************
//  Method: ViewerQueryManager::DoTimeQuery 
//
//  Arguments:
//    origWin   A pointer to the window that originated the query.
//    queryAtts The query attributes to be used for the TimeQuery.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 22, 2004 
//
//  Modifications:
//    Brad Whitlock, Fri Apr 2 08:49:03 PDT 2004
//    I changed how the number of states are determined so the metadata
//    is used instead of the number of "frames" in the viewer plot since
//    that is totally unrelated thanks to keyframing.
//
//    Kathleen Bonnell, Fri Apr  2 13:18:08 PST 2004
//    Delay request for Results window until most possible error states
//    have been processed.  As 'Times' as request from DatabaseMetaDAta
//    may not be accurate here, moved code determing Times/Cycles to
//    engine-portion.  Added Warnings and Errors related to settings
//    of start and end times.  Add call to UpdateActions so that the
//    new window will be updated correctly.
//
//    Kathleen Bonnell, Fri Apr  2 17:03:50 PST 2004
//    Modify the way centering-consistency-checks are performed, so
//    that expressions will get evaluated correctly.  
//    
//    Kathleen Bonnell, Wed Apr 14 15:38:56 PDT 2004 
//    Modify the way centering-consistency-checks are performed, so
//    that Material vars will get evaluated correctly.  
//
//    Kathleen Bonnell, Wed Apr 28 11:11:28 PDT 2004 
//    Added retry capability if engine is dead. 
//    
//    Kathleen Bonnell, Tue Jul 20 10:47:26 PDT 2004
//    Modified retrieval of TimeQueryWindow. 
//    
//    Eric Brugger, Tue Jul 27 09:31:29 PDT 2004
//    Add a cast to fix a compile error.
//    
//    Kathleen Bonnell, Thu Dec 16 17:32:49 PST 2004 
//    Allow 'Variable by Zone' and 'Variable by Node', even though they
//    are no longer part of the QueryList, because Pick uses these
//    queries to do a pick-through-time.
//    
//    Eric Brugger, Thu Jan  6 08:58:28 PST 2005
//    Corrected a misuse of the CATCH_RETURN macro.
//
//    Kathleen Bonnell, Tue Oct 24 18:54:56 PDT 2006 
//    Send pick atts to the Time Query for Variable by Zone/Node. 
//
//    Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007 
//    Add "Locate and Pick Zone/Node" options. 
//
//    Brad Whitlock, Tue Apr 29 16:47:37 PDT 2008
//    Support for internationalization.
//
//    Kathleen Bonnell, Tue Jun 24 09:04:23 PDT 2008 
//    Use the query var name when creating the results plot, not the 
//    originating plot's var name. 
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
// ***********************************************************************

void
ViewerQueryManager::DoTimeQuery(ViewerWindow *origWin, QueryAttributes *qA)
{
    string qName = qA->GetName();
    if (!queryTypes->TimeQueryAvailable(qName))
    {
        if (qName != "Variable by Zone" && 
            qName != "Variable by Node" &&
            qName != "Locate and Pick Zone" &&
            qName != "Locate and Pick Node") 
        {
            Error(tr("Time history query is not available for %1.").arg(qName.c_str()));
            return;
        }
    }
    //
    //  Grab information from the originating window.
    //
    ViewerPlotList *origList = origWin->GetPlotList();
    intVector plotIDs;
    origList->GetActivePlotIDs(plotIDs);
    int origPlotID = (plotIDs.size() > 0 ? plotIDs[0] : -1);
    ViewerPlot *origPlot = origList->GetPlot(origPlotID);

    const avtDatabaseMetaData *md = origPlot->GetMetaData();
    int nStates = md->GetNumStates();
    if (nStates <= 1)
    {
        Error(tr("Cannot create a time query curve with 1 time state.")); 
        return;
    }
    string vName  = origPlot->GetVariableName();
    string hdbName = origPlot->GetSource();
    bool replacePlots = ViewerWindowManager::Instance()->
                        GetClientAtts()->GetReplacePlots();

    string qvarName = qA->GetVariables()[0];
  
    //
    // For certain queries, if we are querying the plot's current variable, 
    // check for centering consistency.
    //
    if (qName == "Variable by Zone" || 
        qName == "Variable by Node" ||
        qName == "Locate and Pick Zone" ||
        qName == "Locate and Pick Node") 
    {
        bool issueWarning = false;
        bool zoneQuery = (qName == "Variable by Zone" ||
                          qName == "Locate and Pick Zone");

        avtVarType varType = origPlot->GetVarType();
        if (varType == AVT_MATERIAL )
        { 
            if (!zoneQuery)
                issueWarning = true;
        }
        else if (varType != AVT_MESH && varType != AVT_UNKNOWN_TYPE &&
                 qvarName == vName)
        {
            TRY
            {
                avtCentering centering = origPlot->GetVariableCentering();
                if ((centering == AVT_ZONECENT  && !zoneQuery) ||
                    (centering == AVT_NODECENT  && zoneQuery))
                {
                    issueWarning = true;
                }
            }
            CATCHALL
            {
                debug4 << "ViewerQueryManager::DoTimeQuery could not perform "
                       << "centering consistency check." << endl;
            }
            ENDTRY
        }
        if (issueWarning)
        {        
            if (zoneQuery)
            {
                Warning(tr("The centering of the query (zone) does not match "
                        "the centering of the plot's current variable "
                        "(node).  Please try again with the appropriately "
                        "centered query: 'Variable By Node'"));
            }
            else 
            {
                Warning(tr("The centering of the query (node) does not match "
                        "the centering of the plot's current variable "
                        "(zone).  Please try again with the appropriately "
                        "centered query: 'Variable By Zone'"));
            }
            return;
        }
    }

    // 
    // Create a list of timesteps for the query. 
    // 

    int startT = timeQueryAtts->GetStartTimeFlag() ? 
                 timeQueryAtts->GetStartTime() : 0; 
    int endT   = timeQueryAtts->GetEndTimeFlag() ? 
                 timeQueryAtts->GetEndTime() : nStates-1;

    if (startT >= endT)
    {
        Error(tr("Query over time: start time must be smaller than end time"
              " please correct and try again.")); 
        return;
    }
    int nUserFrames = (int) ceil(double((endT - startT)/
                                        timeQueryAtts->GetStride()))+1;
    if (nUserFrames <= 1)
    {
        Error(tr("Query over time requires more than 1 frame, "
              "please correct start and end times try again.")); 
        return;
    }
    if (startT < 0)
    {
        Warning(tr("Clamping start time to 0."));
        startT = 0;
    }
    if (endT > nStates-1)
    {
        Warning(tr("Clamping end time to number of available timesteps."));
        endT = nStates-1;
    }
    timeQueryAtts->SetStartTime(startT); 
    timeQueryAtts->SetEndTime(endT); 


    //
    //  See if we can get a window in which to place the resulting curve.
    //
    ViewerWindow *resWin = NULL;

    int winId = (timeQueryAtts->GetCreateWindow() ? -1 : 
                 timeQueryAtts->GetWindowId() -1);
    resWin = ViewerWindowManager::Instance()->GetTimeQueryWindow(winId);

    if (resWin == NULL)
    {
        Error(tr("Please choose a different window method for the time query"));
        return;
    }

    int plotType = GetPlotPluginManager()->GetEnabledIndex("Curve_1.0");
    ViewerPlotList *plotList =  resWin->GetPlotList();

    plotList->SetHostDatabaseName(hdbName);
    plotList->SetEngineKey(origPlot->GetEngineKey());

    if (qvarName == "default")
        qvarName = vName;
    int pid = plotList->AddPlot(plotType, qvarName, replacePlots, false,false);
    ViewerPlot *resultsPlot = plotList->GetPlot(pid);

    timeQueryAtts->SetQueryAtts(*qA);
    if (qName == "Variable by Zone"     || qName == "Variable by Node" ||
        qName == "Locate and Pick Zone" || qName == "Locate and Pick Node") 
    {
        timeQueryAtts->SetPickAtts(*pickAtts);
    }
    bool retry = false;
    int numAttempts = 0;

    EngineKey engineKey =  origPlot->GetEngineKey();

    do
    {
        retry = false;

        // Make sure we will be using a cloned network
        resultsPlot->SetCloneId(origPlot->GetNetworkID());
        resultsPlot->SetSILRestriction(origPlot->GetSILRestriction()); 
        TRY
        {
            ViewerEngineManager::Instance()->CloneNetwork(engineKey,
                origPlot->GetNetworkID(), timeQueryAtts);
            plotList->RealizePlots();
            // 
            // If there was an error, the bad curve plot should not be left
            // around muddying up the waters.
            // 
            if (resultsPlot->GetErrorFlag())
                plotList->DeletePlot(resultsPlot, false);
        }
        CATCH2(VisItException, e)
        {
            char message[2048];
            if (e.GetExceptionType() == "LostConnectionException" ||
                e.GetExceptionType() == "NoEngineException" ||
                e.GetExceptionType() == "ImproperUseException" )
            {
                //
                // Queries access the cached network used by the queried plot.
                // Simply relaunching the engine does not work, as no network
                // is created. This situation requires re-execution of the 
                // plot that is being queried.
                //
                origPlot->ClearCurrentActor();
                origList->UpdateFrame(); 
                retry = true;
                numAttempts++; 
            }
            else if (e.GetExceptionType() == "InvalidDimensionsException")
            {
                //
                //  Create message for the gui that includes the query name
                //  and message.
                //
                SNPRINTF(message, sizeof(message), "%s:  %s", qName.c_str(),
                         e.Message().c_str());
            }
            else if (e.GetExceptionType() == "NonQueryableInputException")
            {
                //
                //  Create message.
                //
                SNPRINTF(message, sizeof(message), "%s%s",
                         "The currently active plot is non-queryable.\n",
                         "Please select a different plot and try again.");
            }
            else
            {
                //
                // Add as much information to the message as we can,
                // including query name, exception type and exception
                // message.
                //
                SNPRINTF(message, sizeof(message), "%s:  (%s)\n%s", qName.c_str(),
                         e.GetExceptionType().c_str(),
                         e.Message().c_str());

            }
            ClearStatus(engineKey.ID().c_str());
            if (!retry)
            {
                // 
                // If there was an error, the bad curve plot should not be left
                // around muddying up the waters.
                // 
                plotList->DeletePlot(resultsPlot, false);
                CATCH_RETURN(1);
            }
        }
        ENDTRY
    } while (retry && numAttempts < 2);

    //
    // Update the actions so the menus and the toolbars have the right state.
    //
    ViewerWindowManager::Instance()->UpdateActions();

    // The following is here because it was necessary in the Lineout case,
    // and time-queries are similar, so do it here, too.
    //
    // Brad Whitlock, Fri Apr 4 15:24:28 PST 2003
    // Do it again because for some reason, even though the curve vis window
    // gets the right values, the actual toolbar and popup menu are not
    // updating. If we ever figure out the problem, remove this code.
    resWin->GetActionManager()->UpdateSingleWindow();
}


// ****************************************************************************
//  Method: ViewerQueryManager::PickThroughTime 
//
//  Arguments:
//    ppi       The struct containing picked point and window information.
//    dom       The domain (optional). 
//    el        The element number (optional). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 31, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr  2 17:03:50 PST 2004
//    Modify the way centering-consistency-checks are performed, so
//    that expressions will get evaluated correctly.  
//
//    Kathleen Bonnell, Wed Apr 14 15:38:56 PDT 2004 
//    Modify the way centering-consistency-checks are performed, so
//    that Material vars will get evaluated correctly.  
//
//    Kathleen Bonnell, Wed Jun  2 10:00:35 PDT 2004 
//    Support new pick types. 
//
//    Kathleen Bonnell, Tue Nov 27 15:44:08 PST 2007 
//    Support new pick-through-time options which perserve picked coordinates
//    instead of picked element.
//
//    Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
ViewerQueryManager::PickThroughTime(PICK_POINT_INFO *ppi, const int dom, 
                                    const int el)
{
    ViewerWindow *origWin = (ViewerWindow *)ppi->callbackData;

    ViewerPlotList *origList = origWin->GetPlotList();
    intVector plotIDs;
    origList->GetActivePlotIDs(plotIDs);
    int origPlotID = (plotIDs.size() > 0 ? plotIDs[0] : -1);
    ViewerPlot *origPlot = origList->GetPlot(origPlotID);
    string vName = origPlot->GetVariableName();

    // 
    //  We can only do one variable (for now) for a time query,
    //  so make sure we have the right one.
    // 
    string pvarName = pickAtts->GetVariables()[0];
    if (pvarName == "default")
        pvarName = vName;

    //
    // If we are querying the plot's current variable, check for
    // centering consistency.
    //
    int type = pickAtts->GetPickType();
    avtVarType varType = origPlot->GetVarType();

    bool issueWarning = false;
    if (varType == AVT_MATERIAL) 
    {
        if (type != PickAttributes::Zone && type != PickAttributes::DomainZone)
            issueWarning = true;
    }
    else if (varType != AVT_MESH && varType != AVT_UNKNOWN_TYPE &&
             pvarName == vName)
    {
        TRY
        {
            avtCentering centering = origPlot->GetVariableCentering();
            if (centering == AVT_ZONECENT || centering == AVT_NODECENT)
            {
                if ((type == PickAttributes::Zone || 
                     type == PickAttributes::DomainZone) && 
                    centering != AVT_ZONECENT)
                {
                    issueWarning = true;
                }
                else if ((type == PickAttributes::Node || 
                          type == PickAttributes::DomainNode) && 
                         centering != AVT_NODECENT)
                {
                    issueWarning = true;
                }
            }
        }
        CATCHALL
        {
            debug4 << "ViewerQueryManager::PickThroughTime could not perform "
                   << " centering consistency check." << endl;
        }
        ENDTRY
    }
    if (issueWarning)
    {
        if (type == PickAttributes::Zone || type == PickAttributes::DomainZone) 
        {
            Warning(tr("The centering of the pick-through-time (zone) does "
                    "not match the centering of the plot's current "
                    "variable (node).  Please try again with the "
                    "appropriately centered Pick"));
            return;
        }
        else 
        if (type == PickAttributes::Node || type == PickAttributes::DomainNode)
        {
            Warning(tr("The centering of the pick-through-time (node) does "
                    "not match the centering of the plot's current "
                    "variable (zone).  Please try again with the "
                    "appropriately centered Pick"));
            return;
        }
    }

    int valid = ComputePick(ppi, dom, el);
    if (valid)
    {
        QueryAttributes qatts;
        if (pickAtts->GetTimePreserveCoord())
        {
            doubleVector d(3);
            d[0] = ppi->rayPt1[0];
            d[1] = ppi->rayPt1[1];
            d[2] = ppi->rayPt1[2];
            qatts.SetDarg1(d);
            d[0] = ppi->rayPt2[0];
            d[1] = ppi->rayPt2[1];
            d[2] = ppi->rayPt2[2];
            qatts.SetDarg2(d);
        }
        else
        {
            qatts.SetDomain(pickAtts->GetDomain() >= 0 ? 
                            pickAtts->GetDomain() : 0);
            qatts.SetElement(pickAtts->GetElementNumber());
        }
        qatts.SetDataType(QueryAttributes::OriginalData);
        stringVector vars;
        vars.push_back(pvarName);
        qatts.SetVariables(vars);
        if (type == PickAttributes::Zone || type == PickAttributes::DomainZone) 
        {
            if (pickAtts->GetTimePreserveCoord())
            {
                qatts.SetName("Locate and Pick Zone");
            }
            else
            {
                qatts.SetName("Variable by Zone");
            }
            qatts.SetElementType(QueryAttributes::Zone);
        }
        else 
        if (type == PickAttributes::Node || type == PickAttributes::DomainNode)
        {
            if (pickAtts->GetTimePreserveCoord())
            {
                qatts.SetName("Locate and Pick Node");
            }
            else
            {
                qatts.SetName("Variable by Node");
            }
            qatts.SetElementType(QueryAttributes::Node);
        }

        DoTimeQuery(origWin, &qatts);
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::VerifySingleInputQuery
//
//  Purpose:
//    Verify that all necessary information for a single-input query is
//    available. 
//
//  Arguments:
//    plist      The plot list containing plots to be queried.
//    plotId     The plot Id of the plot to be queried. 
//    qName      The name of the query. 
//    vars       The variable names on which the query will operate. 
//    qa         The (modifiable) query attributes controlling the query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 3, 2004 
//
//  Modifications:
//    Eric Brugger, Thu Jan  6 08:58:28 PST 2005
//    Corrected a misuse of the CATCH_RETURN2 macro.
//  
//    Hank Childs, Fri Jan 28 11:55:12 PST 2005
//    Make sure returns inside 'try' are wrapped.
//
//    Kathleen Bonnell, Tue Nov  8 11:48:24 PST 2005 
//    Ensure that number of vars required by query are present. 
//
// ****************************************************************************

bool         
ViewerQueryManager::VerifySingleInputQuery(ViewerPlotList *plist, const int plotId,
                            const string &qName, const stringVector &vars, 
                            QueryAttributes &qa) 
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    string host, dbname;
    EngineKey engineKey;
    stringVector uniqueVars;
    stringVector tmp = vars;
    intVector varTypes;
    int state = -1;

    TRY
    {
        //
        // Determine the host, database, and list of variables that will be 
        // used for the query.
        //
        ViewerPlot *oplot = plist->GetPlot(plotId);
        host = oplot->GetHostName();
        engineKey = oplot->GetEngineKey();
        dbname = oplot->GetDatabaseName();
        state = oplot->GetState();
        const string &activeVar = oplot->GetVariableName();
        GetUniqueVars(tmp, activeVar, uniqueVars);
        tmp = uniqueVars;

        //
        // If the state was not determined by the above code, then there
        // were no plots as input to the query. In that case, use the
        // state for the current time slider if its correlation uses
        // the host + dbname. If it does not use the host + dbname then
        // try the time slider for host + dbname. If that does not
        // work then use zero.
        //
        if (state < 0)
        {
            if(plist->HasActiveTimeSlider())
            {
                DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
                string tsName(plist->GetActiveTimeSlider());
                string dbName(fs->ComposeDatabaseName(host, dbname));
                DatabaseCorrelation *c = cL->FindCorrelation(tsName);
                int nStates;
                if(c != 0)
                {
                    // Get the state for the active time slider.
                    plist->GetTimeSliderStates(tsName, state, nStates);

                    // Does the correlation for the active time slider use
                    // the query database?
                    int cts = c->GetCorrelatedTimeState(dbName, state);
                    if(cts >= 0)
                        state = cts;
                }

                // If we didn't have a correlation that uses the query
                // database, try using the time slider for the query
                // database if there is one. If there is not one, then
                // we'll use state 0.
                if(state < 0)
                    plist->GetTimeSliderStates(dbName, state, nStates);
            }

            state = (state < 0) ? 0 : state;
        }

        if (queryTypes->AllowedVarsForQuery(qName) > 0)
        {
            //
            // Get the var type for the specified state. This code will have
            // to be modified to potentially use a different state for each
            // dbname/var pair.
            //
            for (int j = 0; j < uniqueVars.size(); j++)
            {
                varTypes.push_back((int)
                    fs->DetermineVarType(host, dbname, uniqueVars[j], state));
            }
 
            int badVarType =  VerifyQueryVariables(qName, varTypes); 
            if (badVarType != -1)
            {
                queryClientAtts->Notify();
                QString msg = tr("Cannot perform a %1 query on variable %2.\n").
                              arg(qName.c_str()).
                              arg(uniqueVars[badVarType].c_str());
                Error(msg);
                CATCH_RETURN2(1, false);
            }
        }
        int nv = queryTypes->NumberOfVarsForQuery(qName);
        if (nv > 1 && nv != uniqueVars.size())
        {
            queryClientAtts->Notify();
            QString msg = tr("Cannot perform a %1 query with only %2 "
                             "variables, it requires %3 variables.\n").
                          arg(qName.c_str()).
                          arg(uniqueVars.size()).
                          arg(nv);
            Error(msg);
            CATCH_RETURN2(1, false);
        }
        qa.SetVariables(uniqueVars);
        qa.SetVarTypes(varTypes);
        qa.SetTimeStep(state);
        CATCH_RETURN2(1, true);
    }
    CATCH2(VisItException, e)
    {
        //
        // Add as much information to the message as we can,
        // including query name, exception type and exception
        // message.
        //
        string msg = qName + ":  (" + e.GetExceptionType() + ")\n" + 
            e.Message() + "\nThis is probably an internal Query error," +
            " please contact a VisIt developer.\n";
        queryClientAtts->Notify();
        Error(msg.c_str(), false);
        CATCH_RETURN2(1, false);
    }
    ENDTRY
}


// ****************************************************************************
//  Method: ViewerQueryManager::VerifyMultipleInputQuery
//
//  Purpose:
//    Verify that all necessary information for a multiple-input query is
//    available. 
//
//  Arguments:
//    plist      The plot list containing plots to be queried.
//    numInputs  The number of inputs for the query. 
//    qName      The name of the query. 
//    <unused>   The variable names on which the query will operate. 
//    <unused>   The (modifiable) query attributes controlling the query.
//
//  Returns:    True if no error condition, false otherwise.
//
//  Notes:  In the future, as more multiple-input queries are created,
//          this section of code may need to account for variables,
//          variable types, and/or plot state.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 3, 2004 
//
//  Modifications:
//    Eric Brugger, Thu Jan  6 08:58:28 PST 2005
//    Corrected a misuse of the CATCH_RETURN2 macro.
//  
//    Hank Childs, Fri Jan 28 11:55:12 PST 2005
//    Make sure returns inside 'try' are wrapped.
//
//    Brad Whitlock, Tue Apr 29 16:55:30 PDT 2008
//    Support for internationalization.
//
// ****************************************************************************


bool         
ViewerQueryManager::VerifyMultipleInputQuery(ViewerPlotList *plist, 
                            const int numInputs, const string &qName, 
                            const stringVector &, QueryAttributes &)
{
    intVector plotIds;
    EngineKey engineKey;

    TRY
    {
        // Get a list of the active plots.
        plist->GetActivePlotIDs(plotIds);

        //
        // Make sure the number of active plots jives with the expected number
        // of inputs for the query.
        //
        if (plotIds.size() != numInputs)
        {
            queryClientAtts->Notify();
            QString msg = tr("%1 requires exactly %2 plots to be selected, "
                             "realized, and drawn. Please select them and "
                             "try again.\n").
                          arg(qName.c_str()).
                          arg(numInputs);
            Error(msg);
            CATCH_RETURN2(1, false);
        }

        //
        // Determine if the engineKeys for all inputs match. 
        //
        for (int i = 0 ; i < plotIds.size() ; i++)
        {
            int plotId = plotIds[i];
            ViewerPlot *oplot = plist->GetPlot(plotId);
            if (i != 0 && engineKey != oplot->GetEngineKey())
            {
                queryClientAtts->Notify();
                Error(tr("Multiple input queries require all their inputs "
                         "to be on the same host.\n"));
                CATCH_RETURN2(1, false);
            }
            engineKey = oplot->GetEngineKey();
        }
        //
        //  In future, use the passed queryAtts object to set 
        //  anything specific to mulitple-input queries: vars,
        //  state, etc.
        //
        CATCH_RETURN2(1, true);
    }
    CATCH2(VisItException, e)
    {
        //
        // Add as much information to the message as we can,
        // including query name, exception type and exception
        // message.
        //
        string msg = qName + ":  (" + e.GetExceptionType() + ")\n" + 
            e.Message() + "\nThis is probably an internal Query error," +
            " please contact a VisIt developer.\n";
        queryClientAtts->Notify();
        Error(msg.c_str(), false);
        CATCH_RETURN2(1, false);
    }
    ENDTRY
}


// ****************************************************************************
//  Method: ViewerQueryManager::DoSpatialExtentsQuery
//
//  Purpose:
//    Perform a spatial extents query on the specified plot.
//
//  Arguments:
//    oplot      The plot to be queried. 
//    actualData The type of extents to retrieve.
//
//  Notes:  code moved from DatabaseQuery method.

//  Programmer: Kathleen Bonnell
//  Creation:   June 3, 2004 
//
//  Modifications:
//    Eric Brugger, Thu Jan  6 08:58:28 PST 2005
//    Corrected a misuse of the CATCH_RETURN macro.
//
//    Hank Childs, Tue Jun 28 09:39:14 PDT 2005
//    Create the correct return type so CLI's GetQueryOutputValue will work.
//
//    Cyrus Harrison, Wed Sep 19 08:40:27 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Bonnell, Fri Dec  7 10:43:07 PST 2007 
//    Don't send message if QueryOutput is suppressed. 
//
// ****************************************************************************

void
ViewerQueryManager::DoSpatialExtentsQuery(ViewerPlot *oplot, bool actualData)
{
    TRY
    {
        int dim = oplot->GetSpatialDimension();
        double *ext;
        string s;
        if (!actualData)
        {
            ext = oplot->GetSpatialExtents(AVT_ORIGINAL_EXTENTS);
            s = CreateExtentsString(ext, dim, "original",floatFormat);
        }
        else
        {
            ext = oplot->GetSpatialExtents(AVT_ACTUAL_EXTENTS);
            s = CreateExtentsString(ext, dim, "actual",floatFormat);
        }

        queryClientAtts->SetResultsMessage(s);

        doubleVector d;
        for (int i = 0 ; i < 2*dim ; i++)
            d.push_back(ext[i]);
        queryClientAtts->SetResultsValue(d);
        delete [] ext;
        queryClientAtts->Notify();
        if (!suppressQueryOutput)
            Message(s.c_str());
    }
    CATCH2(VisItException, e)
    {
        //
        // Add as much information to the message as we can,
        // including query name, exception type and exception
        // message.
        //
        string msg = "SpatialExtents:  (" + e.GetExceptionType() + ")\n" + 
            e.Message() + "\nThis is probably an internal Query error," +
            " please contact a VisIt developer.\n";
        queryClientAtts->Notify();
        Error(msg.c_str(), false);
        CATCH_RETURN(1);
    }
    ENDTRY
}


// ****************************************************************************
//  Method: ViewerQueryManager::ResetLineoutCache
//
//  Purpose:
//    Resets lineoutCache information to default values. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jul 22 15:38:39 PDT 2004
//    Reset resWinId.
//
// ****************************************************************************

void
ViewerQueryManager::ResetLineoutCache()
{
    lineoutCache.origWin = NULL;
    lineoutCache.fromDefault = true;
    lineoutCache.resWinId = -1;
    if (!lineoutCache.vars.empty())
        lineoutCache.vars.clear(); 
}


// ****************************************************************************
//  Method: ViewerQueryManager::FinishLineout
//
//  Purpose:
//    Completes the Lineout by calling AddQuery with the cached info. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 26 17:45:12 PDT 2004
//    Resume socket signals in the viewer so that synchronization events 
//    can be processed. 
//
// ****************************************************************************

void
ViewerQueryManager::FinishLineout()
{
    if (lineoutCache.origWin != NULL)
    {
        AddQuery(lineoutCache.origWin, &lineoutCache.line, lineoutCache.fromDefault);
        ResetLineoutCache();  
    }
    viewerSubject->BlockSocketSignals(false);
}


// ****************************************************************************
//  Method: ViewerQueryManager::FinishLineQuery
//
//  Purpose:
//    Completes the LineQuery.
//    For Lineout, this is done by calling AddQuery with the cached info. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 9, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 26 17:45:12 PDT 2004
//    Resume socket signals in the viewer so that synchronization events 
//    can be processed. 
//
//    Kathleen Bonnell, Tue May 15 14:04:22 PDT 2007 
//    Send lineoutCache.forceSampling to AddQuery. 
//
// ****************************************************************************

void
ViewerQueryManager::FinishLineQuery()
{
    if (lineoutCache.origWin != NULL)
    {
        for (int i = 0; i < lineoutCache.vars.size(); i++)
        {
            lineoutCache.line.SetVarName(lineoutCache.vars[i]);
            AddQuery(lineoutCache.origWin, &lineoutCache.line,
                     lineoutCache.fromDefault, lineoutCache.forceSampling);
        }
        ResetLineoutCache();  
    }
    viewerSubject->BlockSocketSignals(false);
}


// ****************************************************************************
//  Method: ViewerQueryManager::ResetLineoutColor
//
//  Purpose:
//    Resets colorIndex to the default state. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 5, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerQueryManager::ResetLineoutColor()
{
    colorIndex     = 0;
}


// ****************************************************************************
//  Method: ViewerQueryManager::SetLineoutsFollowTime
//
//  Purpose:
//    Tells the lineout lists to follow/not follow time.
//    based on the passed value. 
//
//  Arguments:
//    followTime   True if lineout's resplot should follow time. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 3, 2005 
//
// ****************************************************************************

void
ViewerQueryManager::SetLineoutsFollowTime(bool followTime)
{
    for (int i = 0; i < nLineouts; ++i)
    {
        lineoutList[i]->SetLineoutsFollowTime(followTime);
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::ClearRefLines
//
//  Purpose:
//    Tells the lineout lists to stop observing plots, because the reflines
//    have been cleared. 
//
//  Arguments:
//    origWin   The window from which reflines have been cleared. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 23, 2005 
//
// ****************************************************************************

void
ViewerQueryManager::ClearRefLines(ViewerWindow *origWin)
{
    for (int i = 0; i < nLineouts; i++)
    {
        if (lineoutList[i]->MatchOriginatingWindow(origWin))
        {
            lineoutList[i]->StopObservingPlot();
        }
    }
}


// ****************************************************************************
//  Method: ViewerQueryManager::EngineExistsForQuery
//
//  Purpose:
//    Tests for a running engine associated with the active plot.
//    Issues warning if the engine does not exist. 
//
//  Arguments:
//    plot     The active plot that will be queried.
//
//  Returns:
//    true if an engine exists for the active plot, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 14, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 16 10:03:27 PDT 2005
//    Changed arg from ViewerWindow to ViewerPlot, removed code that
//    retrieved the active plot (now the arg).
//
// ****************************************************************************

bool
ViewerQueryManager::EngineExistsForQuery(ViewerPlot *plot)
{
    bool engineExists = false;
    EngineKey key = plot->GetEngineKey();
    engineExists = ViewerEngineManager::Instance()->EngineExists(key);
    if (!engineExists)
    {
        Warning(tr("There is no running engine with which to perform the query.  "
                   "Please ensure the plot has finished drawing and try again."));
    }
    return engineExists;
}

 
// ****************************************************************************
//  Method: ViewerQueryManager::CloneQuery
//
//  Purpose:
//    Clones an existing query and give the clone a new time state.
//
//  Arguments:
//    toBeCloned   The query to be cloned.
//    ts           The time step for the cloned query.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 22, 2006 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerQueryManager::CloneQuery(ViewerQuery *toBeCloned, int newTS, int oldTS)
{
    ViewerQuery *clone = new ViewerQuery(toBeCloned, newTS);

    ViewerPlot   *oplot = clone->GetOriginatingPlot();
    ViewerWindow *owin  = clone->GetOriginatingWindow();
    ViewerWindow *rwin  = clone->GetResultsWindow();
    //
    //  Determine the correct list in which to store the query.
    //
    int i, index = -1;
    for (i = 0; i < nLineouts && index == -1; i++)
    {
        if (lineoutList[i]->Matches(oplot, owin, rwin))
        {
            index = i;
        }
    }
    if (index != -1)
    {
        lineoutList[index]->AddQuery(clone);
    }
}
