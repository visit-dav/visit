// ************************************************************************* //
//                              ViewerPlotList.C                             //
// ************************************************************************* //

#ifdef VIEWER_MT
#include <pthread.h>
#endif
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <snprintf.h>

#include <ViewerSubject.h>

#include <AbortException.h>
#include <CompactSILRestrictionAttributes.h>
#include <DataNode.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <PickAttributes.h>
#include <Plot.h>
#include <PlotList.h>
#include <PlotPluginManager.h>
#include <RecursiveExpressionException.h>
#include <SILRestrictionAttributes.h>
#include <ViewerAnimation.h>
#include <ViewerEngineManager.h>
#include <ParsingExprList.h>
#include <ViewerFileServer.h>
#include <ViewerMessaging.h>
#include <ViewerOperator.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlot.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <avtDatabaseMetaData.h>
#include <avtPlot.h>
#include <avtToolInterface.h>
#include <ExprNode.h>

#include <DebugStream.h>

#include <set>
using std::set;
using std::string;

//
// Storage for static data elements.
//
PlotList                 *ViewerPlotList::clientAtts=0;
SILRestrictionAttributes *ViewerPlotList::clientSILRestrictionAtts=0;
SILRestrictionMap         ViewerPlotList::SILRestrictions;
int                       ViewerPlotList::lastPlotId=0;

//
// Global variables.  These should be removed.
//
extern ViewerSubject  *viewerSubject;

typedef struct
{
    ViewerAnimation *animation;
    ViewerPlot      *plot;
    ViewerPlotList  *plotList;
} PlotInfo;

//
// Local macros.
//
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

//
// Local variables.
//
#ifdef VIEWER_MT
static pthread_attr_t plotThreadAttr;
static int            plotThreadAttrInit=0;
#endif

//
// Local prototypes.
//
static void *CreatePlot(void *info);
#ifdef VIEWER_MT
static void PthreadCreate(pthread_t *new_thread_ID, const pthread_attr_t *attr,
    void * (*start_func)(void *), void *arg);
static void PthreadAttrInit(pthread_attr_t *attr);
#endif

// ****************************************************************************
//  Method: ViewerPlotList constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep 26 15:02:36 PDT 2001
//    Initialize bgColor, fgColor.
//
//    Hank Childs, Mon Jul 15 16:52:06 PDT 2002
//    Initialize spatialExtentsType.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Tue Feb 11 11:22:39 PDT 2003
//    I made some char * strings into STL strings to avoid memory problems.
//
// ****************************************************************************

ViewerPlotList::ViewerPlotList(ViewerAnimation *const viewerAnimation) : 
    hostDatabaseName(), hostName(), databaseName()
{
    animation        = viewerAnimation;
    plots            = 0;
    nPlots           = 0;
    nPlotsAlloc      = 0;

    bgColor[0] = bgColor[1] = bgColor[2] = 1.0;
    fgColor[0] = fgColor[1] = fgColor[2] = 0.0;
    spatialExtentsType = AVT_ORIGINAL_EXTENTS;

    keyframeMode = false;
}

// ****************************************************************************
//  Method: ViewerPlotList destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 19 17:50:21 PDT 2002 
//    Notify ViewerQueryManager that plots are being deleted.
//
//    Brad Whitlock, Tue Feb 11 11:23:06 PDT 2003
//    I removed some code to delete char* strings since I made them STL strings.
//
// ****************************************************************************

ViewerPlotList::~ViewerPlotList()
{
    //
    // Delete the list and any plots in the plot list.
    //
    if (nPlotsAlloc > 0)
    {
        for (int i = 0; i < nPlots; i++)
        {
            // Tell the query that this plot is being deleted. 
            ViewerQueryManager::Instance()->Delete(plots[i].plot);
            delete plots[i].plot;
        }

        delete [] plots;
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::GetClientAtts
//
//  Purpose:
//    Return a pointer to the client attributes attribute subject.
//
//  Returns:    A pointer to the client attributes attribute subject.
//
//  Programmer: Eric Brugger
//  Creation:   September 5, 2000
//
// ****************************************************************************

PlotList *
ViewerPlotList::GetClientAtts()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (clientAtts == 0)
    {
        clientAtts  = new PlotList;
    }

    return clientAtts;
}

// ****************************************************************************
//  Method: ViewerPlotList::GetClientSILRestrictionAtts
//
//  Purpose: 
//    Return a pointer to the client SIL restriction attributes.
//
//  Returns:    A pointer to the client SIL restriction attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 21 14:06:38 PST 2001
//
// ****************************************************************************

SILRestrictionAttributes *
ViewerPlotList::GetClientSILRestrictionAtts()
{
    //
    // If the client SIL restriction attributes haven't been allocated
    // then do so.
    //
    if (clientSILRestrictionAtts == 0)
    {
        clientSILRestrictionAtts  = new SILRestrictionAttributes;
    }

    return clientSILRestrictionAtts;
}

// ****************************************************************************
//  Method: ViewerPlotList::SetHostDatabaseName
//
//  Purpose:
//    Set the default host and database names associatied with the plot list.
//
//  Arguments:
//    database  The host database string to use for setting the host
//              and database names.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Jul 30 15:24:13 PST 2002
//    I moved the splitting code into SplitHostDatabase.
//
// ****************************************************************************

void
ViewerPlotList::SetHostDatabaseName(const std::string &database)
{
    //
    // Split the database name into its host and filename components.
    //
    SplitHostDatabase(database, hostName, databaseName);

    //
    // Save the hostDatabaseName.
    //
    hostDatabaseName = hostName + std::string(":") + databaseName;
}

// ****************************************************************************
// Method: ViewerPlotList::SplitHostDatabase
//
// Purpose: 
//   This is a static method that splits a database name into its host and
//   filename components. 
//
// Arguments:
//   database : The input database name.
//   host     : The output host name.
//   db       : The output database name.
//
// Returns:    host, db strings which must be freed by the caller.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 15:20:44 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 11 11:29:37 PDT 2003
//   I rewrote it to use STL strings.
//
// ****************************************************************************

void
ViewerPlotList::SplitHostDatabase(const std::string &database,
    std::string &host, std::string &db)
{
    //
    // If the database string doesn't have a ':' in it then assume that
    // the host name is "localhost" and the database name is the entire
    // string.
    //
    std::string::size_type i = database.find(':');
    if (i == std::string::npos)
    {
        host = "localhost";
        db = database;
    }
    else
    {
        //
        // If the database string does have a ':' in it then the part before
        // it is the host name and the part after it is the database name.
        //
        host = database.substr(0, i);
        db = database.substr(i + 1);
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::GetHostDatabaseName
//
//  Purpose:
//    Return the host/database name associated with the plot list.
//
//  Returns:    The host/database name associated with the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   September 26, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Feb 11 11:36:05 PDT 2003
//    I changed the return type.
//
// ****************************************************************************

const std::string &
ViewerPlotList::GetHostDatabaseName() const
{
    return hostDatabaseName;
}

// ****************************************************************************
// Method: ViewerPlotList::SetDatabaseName
//
// Purpose: 
//   Sets just the database name preserving the existing host.
//
// Arguments:
//   database : The new database name.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 15:24:24 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 11 11:37:52 PDT 2003
//   I rewrote it so it uses STL strings.
//
// ****************************************************************************

void
ViewerPlotList::SetDatabaseName(const std::string &database)
{
    //
    // Save the database name
    //
    databaseName = database;

    //
    // Save the hostDatabaseName.
    //
    hostDatabaseName = hostName + std::string(":") + databaseName;
}

// ****************************************************************************
//  Method: ViewerPlotList::GetDatabaseName
//
//  Purpose:
//    Return the database name associated with the plot list.
//
//  Returns:    The database name associated with the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 31, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Feb 11 11:36:05 PDT 2003
//    I changed the return type.
//
// ****************************************************************************

const std::string &
ViewerPlotList::GetDatabaseName() const
{
    return databaseName;
}

// ****************************************************************************
// Method: ViewerPlotList::GetPlotHostDatabase
//
// Purpose: 
//   Returns the host and database name of the first selected plot.
//
// Arguments:
//   host : The return variable for the hostname.
//   db   : The return variable for the database.
// 
// Returns:    Whether or not any plot was selected.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 7 16:39:44 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::GetPlotHostDatabase(std::string &host, std::string &db) const
{
    //
    // Find the list of selected plots.
    //
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active)
        {
            host = std::string(plots[i].plot->GetHostName());
            db = std::string(plots[i].plot->GetDatabaseName());
            return true;
        }
    }

    return false;
}

// ****************************************************************************
// Method: ViewerPlotList::FileInUse
//
// Purpose: 
//   Searches for a plot having the specified host and database. If a plot is
//   found that uses those values, then the function returns true.
//
// Arguments:
//   host     : The host we're looking for.
//   database : The database we're looking for.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 8 10:41:25 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::FileInUse(const char *host, const char *database) const
{
    for (int i = 0; i < nPlots; ++i)
    {
        if (strcmp(host, plots[i].plot->GetHostName()) == 0 &&
           strcmp(database, plots[i].plot->GetDatabaseName()) == 0)
        {
            return true;
        }
    }

    return false;
}

// ****************************************************************************
//  Method: ViewerPlotList::GetHostName
//
//  Purpose:
//    Return the host name associated with the plot list.
//
//  Returns:    The host name associated with the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   September 25, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Feb 11 11:38:30 PDT 2003
//    I changed the return type.
//
// ****************************************************************************

const std::string &
ViewerPlotList::GetHostName() const
{
    return hostName;
}

// *******************************************************************
//  Method: ViewerPlot::GetNumPlots
//
//  Purpose: 
//    Returns the number of plots in the list.
//
//  Returns:    Returns the number of plots in the list.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 16:14:58 PST 2000
//
// *******************************************************************

int
ViewerPlotList::GetNumPlots() const
{
    return nPlots;
}

// *******************************************************************
//  Method: ViewerPlot::GetNumRealizedPlots
//
//  Purpose: 
//    Returns the number of realized plots in the list.
//
//  Returns:    Returns the number of realized plots in the list.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
// *******************************************************************

int
ViewerPlotList::GetNumRealizedPlots() const
{
    int       i;
    int       nRealizedPlots = 0;

    for (i = 0; i < nPlots; i++)
    {
        if (plots[i].realized)
        {
            nRealizedPlots++;
        }
    }

    return nRealizedPlots;
}

// ****************************************************************************
//  Method: ViewerPlotList::AddPlot
//
//  Purpose:
//    Add a plot to the plot list.
//
//  Arguments:
//    plot         : The plot to add to the plot list.
//    replacePlots : Whether or not to delete all plots before adding a new one.
//
//  Returns:
//    The index of the added plot. (-1 if unsuccessful).
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Sep 11 13:29:09 PST 2000
//    Made all plots but the new one inactive.
//
//    Brad Whitlock, Thu Jun 21 15:54:52 PST 2001
//    Added code to update the SIL restriction attributes.
//
//    Kathleen Bonnell, Wed Sep 26 15:02:36 PDT 2001 
//    Set plot's background and foreground colors. 
//
//    Brad Whitlock, Thu Mar 7 14:40:20 PST 2002
//    Modified the method so it creates the plot rather than having it passed
//    in.
//
//    Brad Whitlock, Thu Apr 11 17:47:05 PST 2002
//    Added the applyOperators medthod.
//
//    Kathleen Bonnell, Wed May 29 10:42:28 PDT 2002  
//    Made this method return the new plot's index.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Tue Feb 11 11:40:43 PDT 2003
//    I made it use STL strings.
//
//    Jeremy Meredith, Fri Aug 15 11:20:10 PDT 2003
//    Added the ability to catch exceptions from NewPlot.  If it catches one,
//    it displays the exception's error message, and a generic one otherwise.
//
//    Walter Herrera, Thu Sep 04 16:13:43 PST 2003
//    I made it capable of creating plots with default attributes
//
// ****************************************************************************

int
ViewerPlotList::AddPlot(int type, const std::string &var, bool replacePlots,
    bool applyOperators, DataNode *attributesNode)
{
    if (databaseName.size() < 1)
    {
        Error("Cannot create a plot until a database has been opened.");
        return -1;
    }

    //
    // Try and create the new plot.
    //
    ViewerPlot *newPlot = NULL;
    bool hadError = false;
    TRY
    {
        newPlot = NewPlot(type, hostName, databaseName, var,
                                      applyOperators);
        if (newPlot == 0)
        {
            Error("VisIt could not create the desired plot.");
            hadError = true;
        }
    }
    CATCH2(VisItException, e)
    {
        Error(e.Message().c_str());
        hadError = true;
    }
    ENDTRY

    if (hadError)
    {
        return -1;
    }

    //
    // Apply the attributes to the new plot
    //
    if(attributesNode != 0) 
    {
        newPlot->SetFromNode(attributesNode);
    }

    //
    // Add the new plot to the plot list.
    //
    int plotId = SimpleAddPlot(newPlot, replacePlots);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts();

    //
    // Find a compatible plot to set the new plot's SIL restriction with
    //
    int compatiblePlotIndex = FindCompatiblePlot(newPlot);

    if (compatiblePlotIndex > -1)
    {
        avtSILRestriction_p new_silr = GetDefaultSILRestriction(
                                          newPlot->GetHostName(),
                                          newPlot->GetDatabaseName(),
                                          newPlot->GetVariableName());

        ViewerPlot *matchedPlot = plots[compatiblePlotIndex].plot;
        new_silr->SetFromCompatibleRestriction(matchedPlot->GetSILRestriction());
        newPlot->SetSILRestriction(new_silr);
    }

    UpdateSILRestrictionAtts();

    return plotId;
}

// ****************************************************************************
//  Method: ViewerPlotList::SetPlotFrameRange
//
//  Purpose:
//    Set the frame range for the specified plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    frame0    : The start frame of the plot.
//    frame1    : The end frame of the plot.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************

void
ViewerPlotList::SetPlotFrameRange(int plotId, int frame0, int frame1)
{
    //
    // Check that the plot id is within range.
    //
    if (plotId < 0 || plotId >= nPlots)
    {
        debug1 << "The plot identifier is out of range." << endl;
        return;
    }

    //
    // Set the frame range for the specified plot.
    //
    plots[plotId].plot->SetFrameRange(frame0, frame1);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts(false);
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
// Method: ViewerPlotList::GetMaximumStates
//
// Purpose: 
//   Returns the maximum number of states in the plot list's open database
//   and all of the plot's databases.
//
// Returns:    The maximum number of time states.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 3 10:48:29 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlotList::GetMaximumStates() const
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    const avtDatabaseMetaData *md;
    int maxFrames = 0;

    // Start with the number of frames in the open database.
    if(hostName.size() > 0 && databaseName.size() > 0)
    {
        md = fs->GetMetaData(hostName, databaseName);
        if(md)
            maxFrames = md->GetNumStates();
    }

    // Check each of the plot's databases for their number of
    // time states and increase maxFrames if any have a larger
    // number of time states.
    for (int i = 0; i < nPlots; ++i)
    {
        ViewerPlot *plot = plots[i].plot;
        md = fs->GetMetaData(plot->GetHostName(), plot->GetDatabaseName());
        if(md)
        {
            int nStates = md->GetNumStates();
            maxFrames = (maxFrames < nStates) ? nStates : maxFrames;
        }
    }

    return maxFrames;
}

// ****************************************************************************
//  Method: ViewerPlotList::DeletePlotKeyframe
//
//  Purpose:
//    Delete the keyframe from the specified plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    frame     : The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************

void
ViewerPlotList::DeletePlotKeyframe(int plotId, int frame)
{
    //
    // Check that the plot id is within range.
    //
    if (plotId < 0 || plotId >= nPlots)
    {
        debug1 << "The plot identifier is out of range." << endl;
        return;
    }

    //
    // Delete the keyframe from the specified plot.
    //
    plots[plotId].plot->DeleteKeyframe(frame);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts(false);
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}
 
// ****************************************************************************
//  Method: ViewerPlotList::MovePlotKeyframe
//
//  Purpose:
//    Move the position of a keyframe for the specified plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    oldFrame  : The old location of the keyframe.
//    newFrame  : The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 29, 2003
//
// ****************************************************************************

void
ViewerPlotList::MovePlotKeyframe(int plotId, int oldFrame, int newFrame)
{
    //
    // Check that the plot id is within range.
    //
    if (plotId < 0 || plotId >= nPlots)
    {
        debug1 << "The plot identifier is out of range." << endl;
        return;
    }

    //
    // Move the keyframe for the specified plot.
    //
    plots[plotId].plot->MoveKeyframe(oldFrame, newFrame);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts(false);
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}
 
// ****************************************************************************
//  Method: ViewerPlotList::SetPlotDatabaseState
//
//  Purpose:
//    Set the database state associated with the specified frame and plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    frame     : The frame number.
//    state     : The state to associate with the frame.
//
//  Programmer: Eric Brugger
//  Creation:   December 30, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Apr 16 13:49:19 PST 2003
//    I removed the unnecessary call to UpdatePlotAtts.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotDatabaseState(int plotId, int frame, int state)
{
    //
    // Check that the plot id is within range.
    //
    if (plotId < 0 || plotId >= nPlots)
    {
        debug1 << "The plot identifier is out of range." << endl;
        return;
    }

    //
    // Set the database state for the specified plot and frame.
    //
    plots[plotId].plot->SetDatabaseState(frame, state);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}
 
// ****************************************************************************
//  Method: ViewerPlotList::DeletePlotDatabaseKeyframe
//
//  Purpose:
//    Delete the database keyframe from the specified plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    frame     : The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   December 30, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Apr 16 13:49:19 PST 2003
//    I removed the unnecessary call to UpdatePlotAtts.
//
// ****************************************************************************

void
ViewerPlotList::DeletePlotDatabaseKeyframe(int plotId, int frame)
{
    //
    // Check that the plot id is within range.
    //
    if (plotId < 0 || plotId >= nPlots)
    {
        debug1 << "The plot identifier is out of range." << endl;
        return;
    }

    //
    // Delete the database keyframe for the specified plot.
    //
    plots[plotId].plot->DeleteDatabaseKeyframe(frame);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}
 
// ****************************************************************************
//  Method: ViewerPlotList::MovePlotDatabaseKeyframe
//
//  Purpose:
//    Move the the position of a database keyframe for the specified plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    oldFrame  : The old location of the keyframe.
//    newFrame  : The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 29, 2003
//
//  Modifications:
//    Brad Whitlock, Wed Apr 16 13:49:19 PST 2003
//    I removed the unnecessary call to UpdatePlotAtts.
//
// ****************************************************************************

void
ViewerPlotList::MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame)
{
    //
    // Check that the plot id is within range.
    //
    if (plotId < 0 || plotId >= nPlots)
    {
        debug1 << "The plot identifier is out of range." << endl;
        return;
    }

    //
    // Move the database keyframe for the specified plot.
    //
    plots[plotId].plot->MoveDatabaseKeyframe(oldFrame, newFrame);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}
 
// ****************************************************************************
//  Method: ViewerPlotList::CopyFrom
//
//  Purpose: 
//    Copies the plots from one plot list to another plot list.
//
//  Arguments:
//    pl : The plot list that is being copied.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 15 17:10:59 PST 2002
//
//  Modifications:
//    Brad Whitlock, Mon Nov 4 10:36:10 PDT 2002
//    I made it copy the active frame to the new animation.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Mon Nov 25 16:29:38 PST 2002
//    I added code to copy the host and database from the source plot list.
//
//    Brad Whitlock, Wed Feb 12 10:58:43 PDT 2003
//    I renamed the method and got rid of the code to set the animation frames.
//
//    Brad Whitlock, Fri Apr 4 10:39:08 PDT 2003
//    I added code to set the database state in the copied plot.
//
//    Hank Childs, Wed Sep 17 10:33:05 PDT 2003
//    Register plot list with plots.
//
// ****************************************************************************

void
ViewerPlotList::CopyFrom(const ViewerPlotList *pl)
{
    //
    // If the plot list being copied is in keyframe mode then put the
    // current plot list in keyframe mode.
    //
    if(pl->keyframeMode)
        keyframeMode = true;

    //
    // Copy the database and the host database.
    //
    SetHostDatabaseName(pl->GetHostDatabaseName());

    //
    // Copy the plots from the input plot list (pl) to this plot list.
    //
    int plotsAdded = 0;
    for (int i = 0; i < pl->GetNumPlots(); ++i)
    {
         ViewerPlot *src = pl->GetPlot(i);

         //
         // Try and create a copy of the i'th plot.
         //
         ViewerPlotFactory *plotFactory = viewerSubject->GetPlotFactory();
         ViewerPlot *dest = NULL;
         TRY
         {
             int f0, f1, s0, s1;
             f0 = src->GetBeginFrame();
             f1 = src->GetEndFrame();
             s0 = src->GetDatabaseState(f0);
             s1 = src->GetDatabaseState(f1);
             dest = plotFactory->CreatePlot(src->GetType(), src->GetHostName(),
                 src->GetDatabaseName(), src->GetVariableName(),
                 src->GetSILRestriction(), f0, f1, s1 - s0);
             dest->SetDatabaseState(f0, s0);
             dest->SetDatabaseState(f1, s1);
             dest->RegisterViewerPlotList(this);
         }
         CATCH(VisItException)
         {
            if (dest)
            {
                delete dest;
                dest = NULL;
            }
         }
         ENDTRY

         if (dest != NULL)
         {
             //
             // Apply the same plot attributes as the old plot.
             //
             dest->SetPlotAtts(src->GetPlotAtts());

             //
             // Apply the same database attributes as the old plot.
             //
             dest->SetDatabaseAtts(src->GetDatabaseAtts());

             //
             // Apply the same operators that are on the old plot to
             // the new plot.
             //
             for (int j = 0; j < src->GetNOperators(); ++j)
             {
                 ViewerOperator *op = src->GetOperator(j);
                 dest->AddOperator(op->GetType());
                 ViewerOperator *newOp = dest->GetOperator(j);
                 newOp->SetOperatorAtts(op->GetOperatorAtts());
             }

             //
             // Add the new plot to the plot list.
             //
             SimpleAddPlot(dest, false);
             ++plotsAdded;
         }
         else
         {
             Error("VisIt could not copy plots.");
             return;
         }
    }

    //
    // Update the client attributes.
    //
    if (plotsAdded > 0)
    {
        UpdatePlotList();
        UpdatePlotAtts();
        UpdateSILRestrictionAtts();
    }
}

// ****************************************************************************
// Method: ViewerPlotList::SimpleAddPlot
//
// Purpose: 
//   Adds the plot to the plot list.
//
// Arguments:
//   plot         : The created plot.
//   replacePlots : Whether or not to clear the plots first.
//
// Returns:
//   The index of the new plot.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 16:36:43 PST 2002
//
// Modifications:
//
//    Kathleen Bonnell, Wed May 29 10:42:28 PDT 2002  
//    Made this method return the new plot's index.
//   
//    Hank Childs, Mon Jul 15 16:52:06 PDT 2002
//    Set whether the plot should work by original or actual extents.
//
//    Eric Brugger, Fri Apr 11 15:25:48 PDT 2003
//    I removed the test for increasing the size of the plot list from
//    the else clause of the replace plots test.  The previous logic failed
//    when replacePlots was true and nPlots was zero since the plot list
//    didn't get allocated.
//
// ****************************************************************************

int
ViewerPlotList::SimpleAddPlot(ViewerPlot *plot, bool replacePlots)
{
    int i;
    int plotId = -1;

    //
    // If we're replacing plots then we want to delete all of the
    // plots before adding a new plot.
    //
    if (replacePlots)
    {
        for (i = 0; i < nPlots; ++i)
        {
            delete plots[i].plot;
            plots[i].active = false;
            plots[i].realized = false;
        }
        nPlots = 0;
    }

    //
    // Expand the list of plots if necessary.
    //
    if (nPlots >= nPlotsAlloc)
    {
        ViewerPlotListElement *plotsNew=0;

        nPlotsAlloc += 10;

        plotsNew = new ViewerPlotListElement[nPlotsAlloc];
        memcpy(plotsNew, plots, nPlots*sizeof(ViewerPlotListElement));

        delete [] plots;
        plots = plotsNew;
    }

    //
    // Make all of the plots inactive except for the new plot.
    //
    for (i = 0; i < nPlots; ++i)
        plots[i].active = false;

    //
    // Add the plot to the list.
    //
    plotId = nPlots;
    plots[nPlots].plot     = plot;
    plots[nPlots].hidden   = false;
    plots[nPlots].active   = true;
    plots[nPlots].realized = false;
    plots[nPlots].id       = lastPlotId++;
    plots[nPlots].plot->SetBackgroundColor(bgColor);
    plots[nPlots].plot->SetForegroundColor(fgColor);
    plots[nPlots].plot->SetSpatialExtentsType(spatialExtentsType);
    nPlots++;
    return plotId;
}

// ****************************************************************************
// Method: ViewerPlotList::NewPlot
//
// Purpose: 
//   Creates an initialized ViewerPlot object of the specified type.
//
// Arguments:
//   type : The type of plot.
//   host : The host where the database is stored.
//   db   : The database.
//   var  : The variable for the plot.
//
// Returns:    An initialized viewer plot or NULL.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 16:29:26 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Apr 11 17:43:49 PST 2002
//   Added a flag that determines if we copy operators from the first plot
//   to the new plot.
//
//   Brad Whitlock, Thu Jul 18 12:57:58 PDT 2002
//   I added code to catch an invalid variable exception that occurs when
//   the plot is being created. If such an exception happens, return a null
//   pointer. This indicates that the plot failed to be created.
//
//   Brad Whitlock, Tue Jul 23 15:21:47 PST 2002
//   I made the code catch VisItException instead of just the invalid
//   variable exception.
//
//   Eric Brugger, Mon Dec 23 09:44:59 PST 2002
//   Correct a bug where the plot frame range is set from 0 -> nstates
//   instead of 0 -> nstates - 1.
//
//   Eric Brugger, Wed Jan  8 16:11:31 PST 2003
//   I added support for keyframing.
//
//   Brad Whitlock, Tue Feb 11 11:41:16 PDT 2003
//   I made it use STL strings.
//
//   Brad Whitlock, Thu Apr 3 09:37:46 PDT 2003
//   I made it smarter about the time range over which a plot can exist.
//
//   Jeremy Meredith, Fri Aug 15 11:21:30 PDT 2003
//   Made it ignore exceptions from GetDefaultSILRestriction.  They
//   are now caught and displayed properly at a higher level.
//
//   Hank Childs, Wed Sep 17 10:33:05 PDT 2003
//   Register plot list with plots.
//
// ****************************************************************************

ViewerPlot *
ViewerPlotList::NewPlot(int type, const std::string &host, const std::string &db,
    const std::string &var, bool applyOperators)
{
    //
    // Get the default SIL restriction.
    //
    avtSILRestriction_p silr(0);
    silr = GetDefaultSILRestriction(host, db, var);

    if (*silr == 0)
    {
        char str[400];
        SNPRINTF(str, 400, "VisIt could not create a SIL restriction for %s. "
                     "The plot of \"%s\" cannot be added.", db.c_str(), var.c_str());
        Error(str);
        return 0;
    }

    //
    // Determine the number of states over which the plot can exist. We assume that
    // the plot can exist over the animation's entire frame range unless the number 
    // of database states is less than the number of animation frames.
    //
    const avtDatabaseMetaData *md = ViewerFileServer::Instance()->GetMetaData(host, db);
    int nFrames = animation->GetNFrames();
    int nStates = md ? md->GetNumStates() : 1;
    if(!GetKeyframeMode())
        nFrames = nStates;

    //
    // Create the initialized plot.
    //
    ViewerPlotFactory *plotFactory = viewerSubject->GetPlotFactory();
    ViewerPlot *plot = 0;
    TRY
    {
        plot = plotFactory->CreatePlot(type, host.c_str(), db.c_str(), var.c_str(),
                                       silr, 0, nFrames - 1, nStates);
        plot->RegisterViewerPlotList(this);
    }
    CATCH(VisItException)
    {
        if (plot)
            delete plot;
        plot = 0;
    }
    ENDTRY

    //
    // Apply the same operators that are on the old plot to the new plot.
    //
    if (plot && applyOperators && (nPlots > 0))
    {
        for (int j = 0; j < plots[0].plot->GetNOperators(); ++j)
        {
             ViewerOperator *op = plots[0].plot->GetOperator(j);
             plot->AddOperator(op->GetType());
             ViewerOperator *newOp = plot->GetOperator(j);
             newOp->SetOperatorAtts(op->GetOperatorAtts());
        }
    }

    return plot;
}

// ****************************************************************************
//  Method: ViewerPlotList::ClearPlots
//
//  Purpose:
//    Clear the actors associated with the plots in the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 22, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Jun 21 15:54:25 PST 2001
//    I added code to update the SIL restriction attributes.
//
//    Mark C. Miller, Thu Oct 30 08:24:25 PST 2003
//    Added optional bool to only clear the actors, nothing else
//
// ****************************************************************************

void
ViewerPlotList::ClearPlots()
{
    //
    // Clear the actors associated with the plots and make them all
    // unrealized.
    //
    for (int i = 0; i < nPlots; i++)
    {
        plots[i].plot->ClearActors();
        plots[i].realized = false;
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::DeletePlot
//
//  Purpose:
//    Removes the specified plot from the list.
//
//  Arguments:
//    whichOne  The plot to be deleted.
//    doUpdate  Specifies whether or not an update should be performed after
//              the plot has been removed.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003
//
// ****************************************************************************

void
ViewerPlotList::DeletePlot(ViewerPlot *whichOne, bool doUpdate)
{
    int  nPlotsNew = 0;
    bool plotDeleted = false;

    //
    // Loop over the list deleting the designated plot.  As it traverses
    // the list it compresses out the deleted plot in place by copying
    // all other plots into their new position.
    //
    for (int i = 0; i < nPlots; i++)
    {
        //
        // If the plot matches the one to be deleted, delete the plot.
        // Otherwise copy it down the list.
        //
        if (plots[i].plot == whichOne)
        {
            delete plots[i].plot;
            plotDeleted = true;
        }
        else
        {
            plots[nPlotsNew] = plots[i];
            nPlotsNew++;
        }
    }
    nPlots = nPlotsNew;
 
    if (plotDeleted && doUpdate)
    {
        //
        // Update the client attributes.
        //
        UpdatePlotList();
        UpdatePlotAtts();
        UpdateSILRestrictionAtts();
 
        //
        // Update the frame.
        //
        animation->UpdateFrame();
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::DeleteActivePlots
//
//  Purpose:
//    Delete the active plots from the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Jun 21 15:53:59 PST 2001
//    I added code to update the SIL restriction attributes.
//
//    Brad Whitlock, Mon Mar 4 14:35:43 PST 2002
//    Added code to make the first plot active.
//
//    Brad Whitlock, Thu May 2 14:52:48 PST 2002
//    Added code to delete the plot if it is the only plot in the list.
//
//    Kathleen Bonnell, Wed Jun 19 17:50:21 PDT 2002 
//    Notify ViewerQueryManager that plots are being deleted.
//
//    Brad Whitlock, Wed Jul 24 10:58:16 PDT 2002
//    Added code to stop the animation from playing if there are no plots.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Kathleen Bonnell, Thu Aug 28 10:10:35 PDT 2003 
//    Added call to CanMeshPlotBeOpaque.
//
//    Brad Whitlock, Fri Oct 24 17:39:38 PST 2003
//    I made it update the expression list.
//
// ****************************************************************************

void
ViewerPlotList::DeleteActivePlots()
{
    //
    // Loop over the list deleting any active plots.  As it traverses
    // the list it compresses out the deleted plots in place by copying
    // any non-active plots into their new position.
    //
    int       nPlotsNew;

    nPlotsNew = 0;
    for (int i = 0; i < nPlots; i++)
    {
        //
        // If the plot is active or there is only one plot, delete the plot.
        // Otherwise copy it down the list.
        //
        if (plots[i].active == true || nPlots == 1)
        {
            // Tell the query that this plot is being deleted. 
            ViewerQueryManager::Instance()->Delete(plots[i].plot);
            delete plots[i].plot;
        }
        else
        {
            plots[nPlotsNew] = plots[i];
            nPlotsNew++;
        }
    }
    nPlots = nPlotsNew;

    //
    // If there are any plots left, make the first plot active.
    //
    if (nPlots > 0)
    {
        plots[0].active = true;
        CanMeshPlotBeOpaque();
    }
    else
    {
        // If there are no plots, make sure we stop animation.
        animation->Stop();
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts();
    UpdateSILRestrictionAtts();
    UpdateExpressionList(true);

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::FindCompatiblePlot
//
//  Purpose: This method finds a plot in the plot list that is compatible
//  with the one pased in. Compatibility means it *must* be have the same host
//  name and database name. Beyond that, any plots which share more of their
//  other features are considered more compatible.
//
//  Returns: -1 on failure, valid plot id otherwise
//
//  Programmer: Mark C. Miller
//  Creation:   November 19, 2003 
//
// ****************************************************************************

int
ViewerPlotList::FindCompatiblePlot(ViewerPlot *givenPlot)
{
    int i;
    int compatibleIndex = -1;
    int maxFeaturesMatched = -1;

    for (i = 0; i < nPlots; ++i)
    {
        bool basicCompatiblity = false;
        int numFeaturesMatched = 0;

        // ignore the the plot in the list that is the same as the given plot
        if (plots[i].plot == givenPlot)
            continue;

        // check basic compatibility
        if ((strcmp(plots[i].plot->GetHostName(),
                    givenPlot->GetHostName()) == 0) &&
            (strcmp(plots[i].plot->GetDatabaseName(),
                    givenPlot->GetDatabaseName()) == 0))
            basicCompatiblity = true;

        // check for compatibility in other features
        if (strcmp(plots[i].plot->GetPlotName(),givenPlot->GetPlotName()) == 0)
            numFeaturesMatched++;
        if (strcmp(plots[i].plot->GetPluginID(),givenPlot->GetPluginID()) == 0)
            numFeaturesMatched++;
        if (strcmp(plots[i].plot->GetVariableName(),
                   givenPlot->GetVariableName()) == 0)
            numFeaturesMatched++;
        if (plots[i].plot->GetType() == givenPlot->GetType())
            numFeaturesMatched++;
        if (plots[i].plot->GetNetworkID() == givenPlot->GetNetworkID())
            numFeaturesMatched++;
        if (plots[i].plot->GetVarType() == givenPlot->GetVarType())
            numFeaturesMatched++;

        if (basicCompatiblity && (numFeaturesMatched > maxFeaturesMatched))
        {
            maxFeaturesMatched = numFeaturesMatched;
            compatibleIndex = i;
        }
    }

    return compatibleIndex;
}

// ****************************************************************************
//  Method: ViewerPlotList::TransmutePlots
//
//  Purpose: This method is used during transitions into and out of scalable
//  rendering mode. First, all plot's actors are cleared regardless of their
//  current state. Then, all plots that are realized, in frame range, not
//  hidden, not in error and have an actor have their actors transmutted. 
//
//  When we transmute a plot, we're either destroying or re-creating the
//  problem-sized data for the actor(s) of the plot. This, in turn, entails
//  communication with the engine. In all other respects, the plot is
//  unchanged.
//
//  Programmer: Mark C. Miller
//  Creation:   October 29, 2003 
//
//  Modifications:
//    Brad Whitlock, Tue Nov 11 13:39:58 PST 2003
//    I made i be declared outside of the for loop so it builds on Windows.
//
// ****************************************************************************

void
ViewerPlotList::TransmutePlots(int frame, bool turningOffScalableRendering)
{
    //
    // First, clear all plot's actors
    //
    int i;
    for (i = 0; i < nPlots; i++)
    {
        plots[i].plot->ClearActors();
    }

    //
    // transmute the actors associated with the plots
    //
    for (i = 0; i < nPlots; i++)
    {
        if (plots[i].realized &&
           !plots[i].hidden &&
            plots[i].plot->IsInFrameRange(frame) &&
           !plots[i].plot->GetErrorFlag())
        {
            plots[i].plot->TransmuteActor(frame, turningOffScalableRendering);
        }
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::HideActivePlots
//
//  Purpose:
//    Hide the active plots from the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   September 6, 2000
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug 28 10:10:35 PDT 2003 
//    Added call to CanMeshPlotBeOpaque.
//
// ****************************************************************************

void
ViewerPlotList::HideActivePlots()
{
    //
    // Loop over the list, toggling the hide flag on any active plots.
    //
    for (int i = 0; i < nPlots; i++)
    {
        //
        // If the plot is active toggle the hide flag.
        //
        if (plots[i].active == true)
        {
            plots[i].hidden = plots[i].hidden == true ? false : true;
        }
    }
  
    CanMeshPlotBeOpaque();
 
    //
    // Update the client attributes.
    //
    UpdatePlotList();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::RealizePlots
//
//  Purpose:
//    Realize any unrealized plots in the list.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Jun 21 15:53:05 PST 2001
//    Added code to update the SIL restriction attributes.
//
//    Brad Whitlock, Thu Jul 25 16:31:12 PST 2002
//    I added code to set the error flag to false in all plots. This way
//    we force the UpdatePlots method to try and regenerate them.
//
// ****************************************************************************

void
ViewerPlotList::RealizePlots()
{
    //
    // Loop through the list setting the realized flag for each plot.
    //
    for (int i = 0; i < nPlots; i++)
    {
        plots[i].realized = true;
        plots[i].plot->SetErrorFlag(false);
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}


// ****************************************************************************
//  Method: ViewerPlotList::SetErrorFlagAllPlots()
//
//  Purpose:
//    Indicate all plots are in error
//
//  Programmer: Mark C. Miller 
//  Creation:   Novembedr 25, 2003
//
// ****************************************************************************

void
ViewerPlotList::SetErrorFlagAllPlots(bool errorFlag)
{
    //
    // Loop through the list setting the realized flag for each plot.
    //
    for (int i = 0; i < nPlots; i++)
    {
        plots[i].plot->SetErrorFlag(errorFlag);
    }

    UpdatePlotList();
    UpdateSILRestrictionAtts();
}


// ****************************************************************************
//  Method: ViewerPlotList::SetPlotVar
//
//  Purpose:
//    Set the variable for any active plots.
//
//  Programmer: Eric Brugger
//  Creation:   September 6, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Sep 21 12:07:12 PDT 2001
//    Added code to update the SIL restriction attributes if the SIL
//    restriction changed when we changed the variable.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotVar(const char *variable)
{
    //
    // Loop over the list, setting the plot variable on any active plots.
    //
    bool SILChanged = false;
    for (int i = 0; i < nPlots; i++)
    {
        //
        // If the plot is active, set the plot variable.
        //
        if (plots[i].active == true)
        {
            SILChanged |= plots[i].plot->SetVariableName(variable);
        }
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();

    //
    // Update the client SIL restriction attributes.
    // 
    if (SILChanged)
        UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::SetPlotAtts
//
//  Purpose:
//    Set the plot attributes for any active plots for the specified
//    plot type.
//
//  Arguments:
//    plotType  The plot type to set the attributes for.
//
//  Programmer: Eric Brugger
//  Creation:   August 29, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 14:52:34 PST 2001
//    Change the type of the plot type to an integer.
//
//    Eric Brugger, Mon Oct 22 11:33:26 PDT 2001
//    I added code to update the plot list so that any plots whose attributes
//    changed would change color in the GUI plot list.
//
//    Brad Whitlock, Wed Jun 26 13:19:51 PST 2002
//    I made it issue a warning message if no plot of the specified type is
//    selected.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Jeremy Meredith, Mon Jun 23 16:17:42 PDT 2003
//    Changed GetAllID to GetEnabledID.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotAtts(const int plotType)
{
    //
    // Loop through the list setting the plot attributes from the client
    // for any plots that are active and match the type.
    //
    int selectedCount = 0;
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active == true && plots[i].plot->GetType() == plotType)
        {
            if (keyframeMode == false)
                plots[i].plot->SetPlotAttsFromClient();
            else
                plots[i].plot->SetPlotAttsFromClient(
                    animation->GetFrameIndex());
            ++selectedCount;
        }
    }

    //
    // If plots were selected, update the frame.
    //
    if (selectedCount > 0)
    {
        animation->UpdateFrame();
    }
    else
    {
        PlotPluginManager *pMgr = PlotPluginManager::Instance();
        std::string msg;
        msg = std::string("VisIt cannot set the ") + 
              pMgr->GetPluginName(pMgr->GetEnabledID(plotType)) + 
              std::string(" plot attributes since no"
                          " plots of that type are selected."
                          " Please select a ") +
              pMgr->GetPluginName(pMgr->GetEnabledID(plotType)) + 
              std::string(" plot before trying to modify its plot"
                          " attributes.");

        Warning(msg.c_str());
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::SetPlotOperatorAtts
//
//  Purpose:
//    Set the plot operator attributes for any active plots for the
//    specified operator type.
//
//  Arguments:
//    operatorType  The operator type to set the attributes for.
//
//  Programmer: Eric Brugger
//  Creation:   September 20, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:17:18 PDT 2001
//    Removed all references to OperType.
//
//    Eric Brugger, Thu Oct 25 08:25:11 PDT 2001
//    I added code to update the plot list so that any plots whose operator
//    attributes changed would change color in the GUI plot list.
//
//    Brad Whitlock, Thu Apr 11 17:26:14 PST 2002
//    I added a flag that allows the operator attributes to be applied to
//    the plot even if it is not selected.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotOperatorAtts(const int operatorType, bool applyToAll)
{
    //
    // Loop through the list setting the plot operator attributes from the
    // client for any plots that are active and match the type.
    //
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            plots[i].plot->SetOperatorAttsFromClient(operatorType);
        }
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
// Method: ViewerPlotList::ReplaceDatabase
//
// Purpose: 
//   Replaces the database used in the current plots with a new database and
//   regenerates the plots.
//
// Arguments:
//   host     : The host on which the new database is located.
//   database : The new database.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 11:34:47 PDT 2002
//
// Modifications:
//   Brad Whitlock, Mon Jul 29 15:40:24 PST 2002
//   I changed the logic so it can replace files that match the host and 
//   database in addition to those that do not. They are mutually exclusive
//   modes. I also cleared the actors for plots that successfully get a new
//   database so that they are guaranteed to be regenerated.
//
//   Brad Whitlock, Tue Feb 11 11:44:11 PDT 2003
//   Made it use STL strings.
//
//   Brad Whitlock, Thu Apr 3 10:27:54 PDT 2003
//   I added code to adjust the plot's time range if we're not in keyframing
//   mode.
//
//   Brad Whitlock, Fri Oct 24 17:40:36 PST 2003
//   I made it update the expression list.
//
//   Brad Whitlock, Mon Nov 3 10:06:02 PDT 2003
//   I added timeState and setTimeState so we can change the animation's
//   time state if we need to change time states before updating the frame.
//   This lets us change animation time states before having to execute the 
//   pipeline so it is somewhat cheaper to replace files with a database
//   that has a new active time state. This approach also lets us just change
//   the animation's time state if we're replacing with the same database.
//
// ****************************************************************************

void
ViewerPlotList::ReplaceDatabase(const std::string &host, const std::string &database,
    int timeState, bool setTimeState, bool onlyReplaceSame)
{
    //
    // Loop through the list replacing the plot's database.
    //
    bool defaultChanged = false;
    bool plotsReplaced = false;
    for (int i = 0; i < nPlots; i++)
    {
        //
        // Decide which files to replace.
        //
        ViewerPlot *plot = plots[i].plot;
        bool sameHost = (host == plot->GetHostName());
        bool sameDB = (database == plot->GetDatabaseName());
        bool doReplace;
        if (onlyReplaceSame)
            doReplace = sameHost && sameDB;
        else
            doReplace = !(sameHost && sameDB);

        //
        // Replace the database in the plot.
        //
        if (doReplace)
        {
            //
            // Get a new SIL restriction for the plot.
            //
            TRY
            {
                avtSILRestriction_p silr = GetDefaultSILRestriction(host,
                    database, plot->GetVariableName());
                if (*silr != 0)
                {
                    //
                    // Try and set the new sil restriction from the old.
                    // This is useful for related files that have not been
                    // grouped.
                    //
                    if(silr->SetFromCompatibleRestriction(plot->GetSILRestriction()))
                    {
                         //
                         // If the default has not been changed then make the
                         // default SIL restriction have the same settings as
                         // the new SIL restriction.
                         //
                         if (!defaultChanged)
                         {
                             defaultChanged = true;
                             std::string key(SILRestrictionKey(host, database, silr->GetTopSet()));
                             SILRestrictions[key] = new avtSILRestriction(silr);
                         }
                    }
                    //
                    // Set the new host, database and SIL restriction.
                    //
                    plot->SetHostDatabaseName(host.c_str(), database.c_str());
                    plot->SetSILRestriction(silr);
                    plot->ClearActors();
                    plotsReplaced = true;

                    //
                    // If we're not in keyframing mode, then we should set
                    // the plot's frame range in the animation to the maximum
                    // number of time states in the plot's database.
                    //
                    if(!GetKeyframeMode())
                    {
                        ViewerFileServer *fs = ViewerFileServer::Instance();
                        const avtDatabaseMetaData *md = fs->GetMetaData(
                            plot->GetHostName(), plot->GetDatabaseName());
                        if(md)
                        {
                            int f0 = plot->GetBeginFrame();
                            int f1 = plot->GetEndFrame();
                            int newf1 = md->GetNumStates() - 1;
                            if(f0 > newf1)
                                f0 = newf1 - 1;
                            if(f0 < 0)
                                f0 = 0;
                            plot->SetFrameRange(f0, newf1);
                            plot->DeleteDatabaseKeyframe(f1);
                            plot->SetDatabaseState(newf1, newf1);
                        }
                    }
                }
            }
            CATCH(InvalidVariableException)
            {
                char str[1024];
                SNPRINTF(str, 1024, "The %s plot of \"%s\" cannot be regenerated "
                             "using the database: %s since the variable "
                             "is not contained in the new database.",
                             plot->GetPlotName(),
                             plot->GetVariableName(),
                             database.c_str());
                Error(str);
            }
            ENDTRY
        }
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateExpressionList(true);

    //
    // Update the SIL restriction attributes if necessary.
    //
    if (plotsReplaced)
        UpdateSILRestrictionAtts();

    //
    // Update the number of frames in the animation.
    //
    animation->UpdateNFrames();

    //
    // Update the frame.
    //
    if(setTimeState)
        animation->SetFrameIndex(timeState);
    else
        animation->UpdateFrame();
}

// ****************************************************************************
// Method: ViewerPlotList::OverlayDatabase
//
// Purpose: 
//   Creates new plots based on the current plots and adds them to the plot
//   list.
//
// Arguments:
//   host     : The host for the database file.
//   database : The database that we want to use.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 16:13:32 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 8 09:47:09 PDT 2002
//   Fixed a bug where the plot attributes were not copied.
//
//   Brad Whitlock, Thu Apr 11 17:46:07 PST 2002
//   Added to the interface of NewPlot().
//
// ****************************************************************************

void
ViewerPlotList::OverlayDatabase(const std::string &host, const std::string &database)
{
    //
    // Loop over the initial list of plots and add new plots based on them
    // which use the new data files.
    //
    int nInitialPlots = nPlots;
    for (int i = 0; i < nInitialPlots; ++i)
    {
        //
        // Create a new plot based on the old plot. Then copy the old plot's
        // plot attributes into the new plot.
        //
        ViewerPlot *newPlot = NewPlot(plots[i].plot->GetType(),
                                      host, database,
                                      plots[i].plot->GetVariableName(),
                                      false);
        //
        // If the plot was created, add it to the plot list.
        //
        if (newPlot)
        {
             //
             // Apply the same plot attributes as the old plot.
             //
             newPlot->SetPlotAtts(plots[i].plot->GetPlotAtts());

             //
             // Apply the same operators that are on the old plot to the new plot.
             //
             for (int j = 0; j < plots[i].plot->GetNOperators(); ++j)
             {
                 ViewerOperator *op = plots[i].plot->GetOperator(j);
                 newPlot->AddOperator(op->GetType());
                 ViewerOperator *newOp = newPlot->GetOperator(j);
                 newOp->SetOperatorAtts(op->GetOperatorAtts());
             }

             //
             // Add the new plot to the plot list.
             //
             SimpleAddPlot(newPlot, false);
        }
        else
        {
            char str[1024];
            SNPRINTF(str, 1024, "The %s plot of \"%s\" cannot be overlayed "
                         "using the database: %s since the variable "
                         "is not contained in the new database.",
                         plots[i].plot->GetPlotName(),
                         plots[i].plot->GetVariableName(),
                         database.c_str());
            Error(str);
        }
    }

    //
    // Realize the plots.
    //
    RealizePlots();
}

// ****************************************************************************
// Method: ViewerPlotList::GetDefaultSILRestriction
//
// Purpose: 
//   Returns the topSet for the variable in the specified database.
//
// Arguments:
//   host     : The host on which the database is located.
//   database : The name of the database.
//   var      : The variable name for which we want the top set.
//
// Returns:    A pointer to a new SIL restriction that the caller must delete.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 09:58:45 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Jul 18 15:32:58 PST 2002
//   I modified the code so it will always check to see if the variable is
//   valid if the variable is not an expression. This ensures that file
//   replacement works properly.
//
//   Sean Ahern, Wed Dec 11 16:52:30 PST 2002
//   Changed the interface for expressions to be easier.
//
//   Sean Ahern, Thu Mar  6 01:45:41 America/Los_Angeles 2003
//   Added support to find the "real variable" from an expression.
//
//   Jeremy Meredith, Fri Aug 15 11:22:18 PDT 2003
//   Made it properly catch errors from parsing expressions.
//   Made it handle (and abort from) infinitely recursive expressions.
//
//   Jeremy Meredith, Wed Oct 29 12:31:52 PST 2003
//   Added code to make sure varLeaves was non-empty before accessing it.
//
//   Jeremy Meredith, Fri Oct 31 13:06:08 PST 2003
//   Made the error message for no-real-variables more informative.
//
// ****************************************************************************
avtSILRestriction_p
ViewerPlotList::GetDefaultSILRestriction(const std::string &host,
    const std::string &database, const std::string &var)
{
    avtSILRestriction_p silr(0);
    ViewerFileServer *server = ViewerFileServer::Instance();
    int topSet = 0;
    char str[400];

    //
    // Get the SIL from the file server.
    //
    const avtSIL *sil = server->GetSIL(host, database);
    if (sil == 0)
    {
        SNPRINTF(str, 400, "VisIt could not read the SIL for %s.", database.c_str());
        Error(str);
        return silr;
    }

    // Check if the variable is an expression.  If it is, walk down the
    // parse tree until we have a "real" variable to work with.
    string realvar = var;
    set<string> expandedVars;
    expandedVars.insert(realvar);
    Expression *expr = ParsingExprList::GetExpression(realvar);
    while (expr)
    {
        ExprNode *tree = ParsingExprList::GetExpressionTree(expr);
        if (!tree)
        {
            // There was a parse error
            return silr;
        }
        const set<string> &varLeaves = tree->GetVarLeaves();
        if (varLeaves.empty())
        {
            EXCEPTION1(ImproperUseException,
                       "After parsing, expression has no real variables.");
        }
        realvar = *varLeaves.begin();
        if (expandedVars.count(realvar))
        {
            EXCEPTION1(RecursiveExpressionException, realvar);
        }
        expandedVars.insert(realvar);
        expr = ParsingExprList::GetExpression(realvar);
    }

    // Figure out the top set for the SIL restriction. If there is more 
    // than one top set, we use the variable name to lookup the mesh name
    // and then we use that mesh name to set the appropriate top set.

    //
    // Try and read the metadata since we'll need it to get the mesh for
    // the plot variable.
    //

    avtDatabaseMetaData *md =
        (avtDatabaseMetaData *)server->GetMetaData(host, database);
    if (md == 0)
    {
        SNPRINTF(str, 400, "VisIt could not read the MetaData for %s.", database.c_str());
        Error(str);
        return silr;
    }

    //
    // Get the Mesh for the desired variable. We bother to do this because
    // we need it to determine the top set for the variable if there is more
    // than one top set. The reason we always do it is that it also determines
    // if the given variable is valid for the specified database. If the
    // variable is invalid, an InvalidVariableException is thrown.
    //
    std::string meshName(md->MeshForVar(realvar));

    //
    // If there is more than one top set, try and find the right one.
    //
    const std::vector<int> &wholes = sil->GetWholes();
    if (wholes.size() > 1)
    {
        //
        // Look through the SIL's whole sets for a name that matches meshName.
        //
        for (int i = 0; i < wholes.size(); ++i)
        {
            avtSILSet_p current = sil->GetSILSet(wholes[i]);
            if (meshName == current->GetName())
            {
                topSet = wholes[i];
                break;
            }
        }
    }

    //
    // Now that we have the topset, look in the SILRestrictions lookup
    // to see if there is a default SIL restriction.
    //
    std::string key(SILRestrictionKey(host, database, topSet));
    SILRestrictionMap::iterator pos = SILRestrictions.find(key);
    if (pos == SILRestrictions.end())
    {
        // Create a new SIL restriction that is based on the SIL and put it
        // in the default SIL restrictions.
        SILRestrictions[key] = new avtSILRestriction((avtSIL *)sil);

        // Create a new SIL restriction for the plot.
        silr = new avtSILRestriction((avtSIL *)sil);
    }
    else
    {
        // Set the SIL restriction to a predefined restriction.
        silr = new avtSILRestriction(pos->second);
    }

    //
    // Set the appropriate top set.
    //
    silr->SetTopSet(topSet);

    return silr;
}

// ****************************************************************************
// Method: ViewerPlotList::ClearDefaultSILRestrictions
//
// Purpose: 
//   Clears the default SIL restrictions for the specified host and
//   database.
//
// Arguments:
//   host     : The name of the host where the database is located.
//   database : The name of the database.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 29 15:57:38 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::ClearDefaultSILRestrictions(const std::string &host,
    const std::string &database)
{
    ViewerFileServer *server = ViewerFileServer::Instance();

    //
    // Get the SIL from the file server.
    //
    const avtSIL *sil = server->GetSIL(host, database);
    if (sil == 0)
    {
        char str[400];
        SNPRINTF(str, 400, "VisIt could not read the SIL for %s.", database.c_str());
        Error(str);
        return;
    }

    //
    // For each top set in the SIL, see if there is a default SIL restriction.
    // If there is a default SIL restriction for the database and top set,
    // delete it.
    //
    const std::vector<int> &wholes = sil->GetWholes();
    for (int i = 0; i < wholes.size(); ++i)
    {
        std::string key(SILRestrictionKey(host, database, wholes[i]));
        SILRestrictionMap::iterator pos = SILRestrictions.find(key);
        if (pos != SILRestrictions.end())
            SILRestrictions.erase(pos);
    }
}

// ****************************************************************************
// Method: ViewerPlotList::SILRestrictionKey
//
// Purpose: 
//   Creates a key that is used in the default SIL restriction map.
//
// Arguments:
//   host   : The host where the database is.
//   db     : The database.
//   topSet : The top set for the sil restriction.
//
// Returns:    A key composed of all of the inputs.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 11:57:04 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Jul 30 16:31:01 PST 2002
//   I made it static and removed the const qualifier.
//
// ****************************************************************************

std::string
ViewerPlotList::SILRestrictionKey(const std::string &host, const std::string &db,
    int topSet)
{
    char key[1024];
    SNPRINTF(key, 1024, "%s:%s:%d", host.c_str(), db.c_str(), topSet);
    return std::string(key);
}

// ****************************************************************************
//  Method: ViewerPlotList::SetActivePlots
//
//  Purpose:
//    Set the active plots in the plot list.
//
//  Arguments:
//    activePlots     : The indices of the plots to make active.
//    activeOperators : The indices of the operators to make active in the
//                      plot list.
//    expandedPlots   : The plots that we want to be expanded.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Jun 21 15:52:11 PST 2001
//    Added code to update the SIL restriction attributes.
//
//    Brad Whitlock, Wed Sep 19 13:39:16 PST 2001
//    Made the indices zero-origin.
//
//    Kathleen Bonnell, Thu Aug 15 18:37:59 PDT 2002  
//    Update SILRestrictionAtts before PlotAtts. 
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Thu Dec 26 17:15:11 PST 2002
//    I made it use an intVector.
//
//    Brad Whitlock, Fri Apr 11 09:35:55 PDT 2003
//    I added activeOperators, expandedPlots, and moreThanPlotsValid arguments.
//
//    Brad Whitlock, Fri Oct 24 16:19:58 PST 2003
//    I added code to update the expression list.
//
// ****************************************************************************

void
ViewerPlotList::SetActivePlots(const intVector &activePlots,
    const intVector &activeOperators, const intVector &expandedPlots,
    bool moreThanPlotsValid)
{
    //
    // First make all the plots inactive and then make the listed plots
    // active.
    //
    int i;
    for (i = 0; i < nPlots; i++)
    {
        plots[i].active = false;
    }

    //
    // Make the selected plots be active.
    //
    for (i = 0; i < activePlots.size(); i++)
    {
        if (activePlots[i] < nPlots)
        {
            plots[activePlots[i]].active = true;
        }
    }

    //
    // Set the active operators and the expanded flag for each plot.
    //
    if(moreThanPlotsValid &&
       activeOperators.size() == nPlots &&
       expandedPlots.size() == nPlots)
    {
        for(i = 0; i < activeOperators.size(); ++i)
        {
            plots[i].plot->SetActiveOperatorIndex(activeOperators[i]);
            plots[i].plot->SetExpanded(expandedPlots[i] > 0);
        }
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();
    UpdatePlotAtts();
    UpdateExpressionList(true);
}

// ****************************************************************************
//  Method: ViewerPlotList::SetPlotSILRestriction
//
//  Purpose: 
//    Sets the client SIL restriction attributes into the plots in the plot
//    list that are selected.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 21 14:10:37 PST 2001
//
//  Modifications:
//    Brad Whitlock, Fri Sep 21 11:08:22 PDT 2001
//    Made setting the SIL restriction take into account the variable top set.
//
//    Eric Brugger, Thu Oct 25 08:25:11 PDT 2001
//    I added code to update the plot list so that any plots whose SIL
//    restriction changed would change color in the GUI plot list.
//
//    Brad Whitlock, Thu Feb 7 16:43:32 PST 2002
//    Modified the method so the new SIL restriction is passed in.
//
//    Brad Whitlock, Thu Mar 7 11:01:13 PDT 2002
//    Modified the method so it caches the SIL restriction.
//
//    Brad Whitlock, Tue Jul 23 10:38:40 PDT 2002
//    I added an optional argument that makes the method try to set the
//    SIL restriction for all plots.
//
//    Brad Whitlock, Mon Aug 19 17:29:45 PST 2002
//    I made sure that when the SIL restriction is applied to all plots,
//    we compare against the first selected plot rather than the first plot
//    in the "activePlots" list that we create. This must be done to ensure
//    that we don't mismatch the SIL restriction with the plot database.
//
//    Brad Whitlock, Fri Aug 30 15:43:36 PST 2002
//    I fixed a serious and embarassing bug that I introduced.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotSILRestriction(bool applyToAll)
{
    //
    // Create a new SIL restriction based on the client SIL restriction.
    //
    avtSILRestriction_p silr = new avtSILRestriction(*(GetClientSILRestrictionAtts()));

    //
    // Find the list of selected plots and the index of the first selected plot.
    //
    int i, firstSelected = -1;
    std::vector<int> activePlots;
    for (i = 0; i < nPlots; ++i)
    {
        if (plots[i].active)
        {
            activePlots.push_back(i);
            if (firstSelected == -1)
                firstSelected = activePlots.size() - 1;
        }
        else if (applyToAll)
            activePlots.push_back(i);
    }

    if (firstSelected < 0)
        firstSelected = 0;

    //
    // Set the SIL restriction into each selected file that has the same
    // database as the first selected plot.
    //
    bool needsUpdate = false;
    for (i = 0; i < activePlots.size(); ++i)
    {
        ViewerPlot *plot0 = plots[activePlots[firstSelected]].plot;
        ViewerPlot *ploti = plots[activePlots[i]].plot;
        bool sameDB =  (strcmp(plot0->GetDatabaseName(),
                               ploti->GetDatabaseName()) == 0);
        bool sameTopSet = (plot0->GetSILRestriction()->GetTopSet() ==
                           ploti->GetSILRestriction()->GetTopSet());

        if (sameDB && sameTopSet)
        {
            //
            // Set a new SIL restriction into the plot.
            //
            avtSILRestriction_p newSILR = new avtSILRestriction(silr);
            plots[activePlots[i]].plot->SetSILRestriction(newSILR);

            //
            // Indicate that the viewer window needs to be updated.
            //
            needsUpdate = true;
        }
    }

    if (needsUpdate)
    {
        //
        // Update the client attributes.
        //
        UpdatePlotList();

        //
        // Update the frame.
        //
        animation->UpdateFrame();
    }

    //
    // Set the SIL restriction as the default SIL restriction for this
    // database and top set.
    //
    string key(SILRestrictionKey(hostName, databaseName, silr->GetTopSet()));
    SILRestrictions[key] = silr;
}

// ****************************************************************************
//  Method: ViewerPlotList::AddOperator
//
//  Purpose:
//    Add the operator type to the active plots.
//
//  Arguments:
//    type       : The operator type to add to the active plots.
//    applyToAll : A flag that indicates we should ignore the selected plots
//                 and apply the operator to all plots.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Dec 13 14:26:45 PST 2000
//    Added code to update the operator client attributes for the operator
//    that gets added. Note that the operator attributes that are sent to the
//    client are for the first plot in the list.
//
//    Jeremy Meredith, Thu Jul 26 03:17:48 PDT 2001
//    Removed all references to OperType.
//
//    Brad Whitlock, Tue Feb 26 16:04:28 PST 2002
//    Added a new argument that lets us apply the operator to all plots.
//
//    Kathleen Bonnell, Thu Sep 11 11:47:16 PDT 2003 
//    Added optional bool argument, indicates whether the operator should be 
//    initialized from its Default or Client atts. 
//
// ****************************************************************************

void
ViewerPlotList::AddOperator(const int type, bool applyToAll, const bool fromDefault)
{
    //
    // Loop through the list adding the operator to the active plots.
    //
    bool notUpdatedClientAtts = true;

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            plots[i].plot->AddOperator(type, fromDefault);

            // Update the new operator's client attributes.
            if (notUpdatedClientAtts)
            {
                notUpdatedClientAtts = false;

                // Look for the last operator of the specified type.
                for (int j = plots[i].plot->GetNOperators() - 1; j >= 0; --j)
                {
                    ViewerOperator *oper = plots[i].plot->GetOperator(j);
                    if (oper->GetType() == type)
                    {
                        oper->SetClientAttsFromOperator();
                        // break out of the loop.
                        j = -1;
                    }
                }
            } // end if notUpdatedClientAtts
        }
    }

    //
    // Update the client attributes. (the plot list)
    //
    UpdatePlotList();

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
// Method: ViewerPlotList::PromoteOperator
//
// Purpose: 
//   Promotes the specified operator in the plots' operator lists.
//
// Arguments:
//   operatorId : The index of the operator to promote.
//   applyToAll : Whether to apply this to all plots or only to the selected
//                plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 10:10:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::PromoteOperator(int operatorId, bool applyToAll)
{
    //
    // Loop through the list removing the operator from the active plots.
    //
    bool doUpdate = false;
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            // Try and promote the operator.
            bool promoted = plots[i].plot->PromoteOperator(operatorId);

            // If we promoted the operator, send the plot back to the new state.
            if(promoted)
                plots[i].realized = false;

            doUpdate |= promoted;
        }
    }

    if(doUpdate)
    {
        //
        // Update the client attributes.
        //
        UpdatePlotList();
    }
}

// ****************************************************************************
// Method: ViewerPlotList::DemoteOperator
//
// Purpose: 
//   Demotes the specified operator in the plots' operator lists.
//
// Arguments:
//   operatorId : The index of the operator to demote.
//   applyToAll : Whether to apply this to all plots or only to the selected
//                plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 10:10:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::DemoteOperator(int operatorId, bool applyToAll)
{
    //
    // Loop through the list removing the operator from the active plots.
    //
    bool doUpdate = false;
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            // Try and promote the operator.
            bool demoted = plots[i].plot->DemoteOperator(operatorId);

            // If we promoted the operator, send the plot back to the new state.
            if(demoted)
                plots[i].realized = false;

            doUpdate |= demoted;
        }
    }

    if(doUpdate)
    {
        //
        // Update the client attributes.
        //
        UpdatePlotList();
    }
}

// ****************************************************************************
// Method: ViewerPlotList::RemoveOperator
//
// Purpose: 
//   Removes the specified operator from the plots' operator lists.
//
// Arguments:
//   operatorId : The index of the operator to remove.
//   applyToAll : Whether to apply this to all plots or only to the selected
//                plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 10:10:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::RemoveOperator(int operatorId, bool applyToAll)
{
    //
    // Loop through the list removing the operator from the active plots.
    //
    bool doUpdate = false;
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            bool removed = plots[i].plot->RemoveOperator(operatorId);

            // If we removed an operator, send the plot back to the new state.
            if(removed)
                plots[i].realized = false;

            doUpdate |= removed;
        }
    }

    if(doUpdate)
    {
        //
        // Update the client attributes.
        //
        UpdatePlotList();
        UpdatePlotAtts(false);
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::RemoveLastOperator
//
//  Purpose:
//    Remove the last operator from the active plots.
//
//  Arguments:
//    applyToAll : Causes the routine to ignore plot selection.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Feb 26 16:44:57 PST 2002
//    Added the applyToAll argument.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
// ****************************************************************************

void
ViewerPlotList::RemoveLastOperator(bool applyToAll)
{
    //
    // Loop through the list removing the last operator from the active plots.
    //
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            plots[i].plot->RemoveLastOperator();
        }
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts(false);

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::RemoveAllOperators
//
//  Purpose:
//    Remove all the operators from the active plots.
//
//  Arguments:
//    applyToAll : Causes the routine to ignore plot selection.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications;
//    Brad Whitlock, Tue Feb 26 16:43:37 PST 2002
//    Added the applyToAll argument.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
// ****************************************************************************

void
ViewerPlotList::RemoveAllOperators(bool applyToAll)
{
    //
    // Loop through the list removing all the operators from the active plots.
    //
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            plots[i].plot->RemoveAllOperators();
        }
    }

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts(false);

    //
    // Update the frame.
    //
    animation->UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::ArePlotsUpToDate
//
//  Purpose:
//    Return a boolean indicating if the plots are up-to-date.
//
//  Arguments:
//    frame     The frame to use to check if the plots are up-to-date.
//
//  Returns:    Boolean indicating if the plots are up-to-date.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Hank Childs, Fri Feb 15 15:23:31 PST 2002
//    Added support for a degenerate case.  [HYPer03715].
//
//    Brad Whitlock, Tue May 7 14:00:50 PST 2002
//    Added code to allow hidden plots to be considered up to date.
//
//    Brad Whitlock, Thu Jul 25 14:38:08 PST 2002
//    I modified the function so it works in all cases.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
// ****************************************************************************

bool
ViewerPlotList::ArePlotsUpToDate(const int frame) const
{
    for (int i = 0; i < nPlots; i++)
    {
        //
        // If the frame is not in range for this plot then skip to the next
        // plot because this plot doesn't need to ruin it for the other plots
        // which may be defined at this frame.
        //
        if (!plots[i].plot->IsInFrameRange(frame))
            continue;
#ifdef VIEWER_MT
        //
        // An unfortunate degenerate case.  Trying to execute the plot
        // caused an exception.  If the plot was executed previously, then
        // we need to clear it out of the ViewerWindow, so pretend that
        // it is up-to-date.
        //
        if (plots[i].plot->NoActorExists(frame) && 
            plots[i].plot->GetErrorFlag())
        {
            return true;
        }
#endif
        //
        // The real issue is that a plot must want to be drawn (realized is
        // true), it must not be hidden (hidden is false). If a plot meets
        // those conditions and it has no actor or it has its error flag set
        // then it is not up to date we return false.
        //
        bool isCandidate = plots[i].realized && !plots[i].hidden;
        bool needsGenerated = (plots[i].plot->NoActorExists(frame) ||
                               plots[i].plot->GetErrorFlag());
        if (isCandidate && needsGenerated)
        {
            return false;
        }
    }

    return true;
}

// ****************************************************************************
// Method: ViewerPlotList::InterruptUpdatePlotList
//
// Purpose: 
//   Interrupts the UpdatePlots method if it is processing multiple plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 25 17:01:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::InterruptUpdatePlotList()
{
    interrupted = true;
}

// ****************************************************************************
//  Method: ViewerPlotList::UpdatePlots
//
//  Purpose:
//    Update the plots for the specified frame.
//
//  Arguments:
//    frame     The frame to use for updating the plots.
//
//  Returns:    False if the function executed with more than one thread.
//              The function returns True otherwise.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Apr 24 15:01:06 PST 2001
//    Added some code to look through the plots and update the plot list if
//    any of the plots have errors.
//
//    Brad Whitlock, Wed Feb 27 15:11:49 PST 2002
//    Made the animation update its windows only after all plots have been
//    generated when we have a single threaded viewer. This ensures that
//    we try to generate all plots even if one or more of them along the 
//    way have errors.
//
//    Brad Whitlock, Tue May 7 14:01:26 PST 2002
//    Added code to skip hidden plots so they are not generated even if they
//    need to be generated. They get generated later when they are made
//    visible.
//
//    Brad Whitlock, Thu Jul 25 16:26:06 PST 2002
//    I removed a bunch of code that is no longer necessary and made it so
//    plots that have an error are not regenerated. This forces the user to
//    click the gui's "Draw" button. I made it so interrupting an engine
//    stops the loop.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Fri Jan 17 09:57:43 PDT 2003
//    I added code to run through the plot list again if any plots had errors
//    when we're animating.
//
//    Kathleen Bonnell, Thu Aug 28 10:10:35 PDT 2003 
//    Added call to CanMeshPlotBeOpaque.
//
// ****************************************************************************

bool
ViewerPlotList::UpdatePlots(const int frame, bool animating)
{
    interrupted = false;

#ifndef VIEWER_MT
    int attempts = animating ? nPlots : 1;
    while(attempts > 0 && !interrupted)
    {
#endif
        //
        // Create any missing plots.
        //
        for (int i = 0; i < nPlots; i++)
        {
            if (plots[i].realized &&
                plots[i].plot->IsInFrameRange(frame) &&
                !plots[i].hidden &&
                !plots[i].plot->GetErrorFlag()
#ifndef JEREMY_CACHE_KLUDGE
                && plots[i].plot->NoActorExists(frame)
#endif
                )
            {
                if(interrupted)
                {
                    plots[i].plot->SetErrorFlag(true);
                    continue;
                }

                PlotInfo  *info=0;
    
                info = new PlotInfo;
                info->animation = animation;
                info->plot = plots[i].plot;
                info->plotList = this;

#ifdef VIEWER_MT
                pthread_t tid;

                if (plotThreadAttrInit == 0)
                {
                    PthreadAttrInit(&plotThreadAttr);
                    pthread_attr_setdetachstate(&plotThreadAttr,
                        PTHREAD_CREATE_JOINABLE);
                    pthread_attr_setscope(&plotThreadAttr, PTHREAD_SCOPE_PROCESS);
                    plotThreadAttrInit = 1;
                }
                PthreadCreate(&tid, &plotThreadAttr, CreatePlot, (void *)info);
#else
                CreatePlot((void *)info);
#endif
            }
        }

#ifndef VIEWER_MT
        if(animating && attempts > 0 && !interrupted)
        {
            bool plotsWithErrors = false;
            for(int i = 0; i < nPlots; ++i)
            {
                // See if a plot has errors.
                plotsWithErrors |= plots[i].plot->GetErrorFlag();

                // Set the plot's error flag to false so if we have to
                // go through the loop again we generate the plot.
                plots[i].plot->SetErrorFlag(false);
            }

            // Determine the number of tries that we have left before we
            // either have no errors or we give up.
            attempts = (!plotsWithErrors) ? 0 : (attempts - 1);

            if(attempts > 0)
            {
                Message("Some plots had errors. VisIt will try to regenerate "
                        "those plots");
            }
        }
        else
            attempts = 0;
    }
#endif

    CanMeshPlotBeOpaque();

#ifdef VIEWER_MT
    return false;
#else
    return true;
#endif
}

// ****************************************************************************
//  Method: ViewerPlotList::UpdateWindow
//
//  Purpose:
//    Update the specified window with the plots from the specified frame.
//
//  Arguments:
//    window          : The window to update.
//    frame           : The frame to use for the plot.
//    nFrames         : The number of frames in the animation.
//    immediateUpdate : Whether the window should be updated immediately or
//                      later.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Jeremy Meredith, Mon Sep 11 12:31:10 PDT 2000
//    Changed it to add a 3D actor instead of 2D.
//
//    Brad Whitlock, Fri Oct 27 15:54:50 PST 2000
//    I changed it so the viewerSubject messages the rendering thread.
//
//    Eric Brugger, Thu Dec 28 13:33:38 PST 2000
//    I modified the routine to set the view after adding the plots to
//    the window.
//
//    Eric Brugger, Wed Feb 21 08:32:41 PST 2001
//    Replace the use of VisWindow with ViewerWindow.
//
//    Eric Brugger, Tue Apr 24 14:55:39 PDT 2001
//    I replaced some code which updated the viewer window's view information
//    with a method invocation.
//
//    Eric Brugger, Tue Apr 24 16:47:00 PDT 2001
//    Modify the routine to determine the dimension of the window and
//    pass it to the window.
//
//    Brad Whitlock, Thu Apr 26 17:11:44 PST 2001
//    Modified an error message so it gets routed to the GUI.
//
//    Brad Whitlock, Fri Jun 15 13:31:27 PST 2001
//    Replaced the updateWindow message with a call to
//    ViewerWindow::SendUpdateMessage which does the same thing.
//
//    Eric Brugger, Wed Aug 22 10:12:35 PDT 2001
//    Removed call to UpdateViewInfo, which is no longer needed.
//
//    Hank Childs, Thu Sep  6 16:23:54 PDT 2001
//    Freed plot extents.
//
//    Brad Whitlock, Wed Sep 19 16:21:15 PST 2001
//    Added code to check to see if updates are enabled before we send an
//    update message to the rendering thread.
//
//    Hank Childs, Fri Feb 15 15:26:24 PST 2002
//    Be more tolerant of plots that do not have actors.  This is necessary
//    because plots that had exceptions when their attributes were changed
//    must be given an opportunity to clean themselves up.
//
//    Brad Whitlock, Wed Feb 27 15:08:34 PST 2002
//    Added some code to mark a plot that has no actor or reader as a plot
//    that has an error.
//
//    Brad Whitlock, Mon Mar 4 10:32:38 PDT 2002
//    I made it okay to not have an actor or reader, the plot just gets
//    skipped instead of being marked as having an error.
//
//    Brad Whitlock, Thu Jul 25 16:35:19 PST 2002
//    I added the immediateUpdate argument that specifies how the redraw
//    window action should be accomplished. If the window updates immediately
//    we are calling from the Qt event loop thread and we tell the window to
//    update. Otherwise, we are running multithreaded and send an update
//    message to be handled by the Qt event loop thread.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Eric Brugger, Wed Apr 23 14:52:32 PDT 2003
//    I added a call to turn off the merging of view limits.
//
//    Eric Brugger, Wed Aug 20 11:02:14 PDT 2003
//    I modified the routine to use the window mode instead of the view
//    dimension.
//
//    Brad Whitlock, Thu Nov 6 11:07:27 PDT 2003
//    I added the nFrames argument.
//
// ****************************************************************************

void
ViewerPlotList::UpdateWindow(ViewerWindow *const window, const int frame,
    const int nFrames, bool immediateUpdate)
{
    //
    // Clear the window.  Disable updates so that the window isn't updated
    // as each actor is removed.  The updates will be enabled in the
    // rendering thread when it recieves the message to render the window.
    //
    bool updatesEnabled = window->UpdatesEnabled();
    window->DisableUpdates();
    window->ClearPlots();

    //
    // Loop over the plots, calculating their global window mode, their global
    // extents, and adding their actors to the window.  The window mode of the
    // window is taken to be the window mode of the first plot encountered.
    // If any subsequent plots don't match that window mode then they are not
    // added to the window.  Hidden plots are not included in the calculation
    // of the global extents.
    //
    WINDOW_MODE globalWindowMode;
    double      globalExtents[6];
    int         errorCount = 0;
    int         startFrame = -1;
    int         endFrame = -1;
    int         startState = -1;
    int         curState = -1;
    int         endState = -1;

    globalWindowMode = WINMODE_NONE;
    globalExtents[0] = DBL_MAX; globalExtents[1] = -DBL_MAX;
    globalExtents[2] = DBL_MAX; globalExtents[3] = -DBL_MAX;
    globalExtents[4] = DBL_MAX; globalExtents[5] = -DBL_MAX;

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].plot->GetErrorFlag())
        {
            // Don't draw bad plots.
            continue;
        }

        //
        // If the reader or the actor is bad then mark the plot as bad. This
        // usually happens when a plot generated before the current plot has
        // had an error and the current plot has not been generated.
        //
        if (*(plots[i].plot->GetReader(frame)) == 0 ||
            plots[i].plot->NoActorExists(frame))
        {
            continue;
        }

        if (plots[i].plot->IsInFrameRange(frame) &&
            plots[i].realized == true && plots[i].hidden == false)
        {
            avtActor_p &actor = plots[i].plot->GetActor(frame);
            WINDOW_MODE plotWindowMode = actor->GetWindowMode();
            int plotDimension = plots[i].plot->GetSpatialDimension(frame);

            if (globalWindowMode == WINMODE_NONE)
                globalWindowMode = plotWindowMode;

            if (plotWindowMode != globalWindowMode)
            {
                if (errorCount == 0)
                {
                    Error("The plot dimensions do not match.");
                    ++errorCount;
                }

                // Indicate that the plot has an error.
                plots[i].plot->SetErrorFlag(true);
            }
            else
            {
                // Indicate that the plot has no error.
                plots[i].plot->SetErrorFlag(false);

                double *plotExtents = plots[i].plot->GetSpatialExtents(frame);

                switch (plotDimension)
                {
                  case 3:
                    globalExtents[4] = min(globalExtents[4], plotExtents[4]);
                    globalExtents[5] = max(globalExtents[5], plotExtents[5]);
                  case 2:
                    globalExtents[2] = min(globalExtents[2], plotExtents[2]);
                    globalExtents[3] = max(globalExtents[3], plotExtents[3]);
                  case 1:
                    globalExtents[0] = min(globalExtents[0], plotExtents[0]);
                    globalExtents[1] = max(globalExtents[1], plotExtents[1]);
                }
                delete [] plotExtents;
                   
                window->AddPlot(actor);

                //
                // Get the plot's database states for the start, current, and
                // end frames.
                //
                if(startFrame == -1)
                {
                    startFrame = plots[i].plot->GetBeginFrame();
                    endFrame = plots[i].plot->GetEndFrame();
                    startState = plots[i].plot->GetDatabaseState(startFrame);
                    curState = plots[i].plot->GetDatabaseState(frame);
                    endState = plots[i].plot->GetDatabaseState(endFrame);
                }
            }
        }
    }

    //
    // If there were any plots then update the view attributes.
    //
    if (globalExtents[0] != DBL_MAX)
    {
        window->UpdateView(globalWindowMode, globalExtents);
    }

    //
    // Update the vis window's current frame and number of frames, etc so that
    // actors that need that information will have it.
    //
    if(startFrame != -1)
    {
        window->SetFrameAndState(nFrames, startFrame, frame, endFrame,
                                 startState, curState, endState);
    }

    //
    // Clear the merge view limits flag.  This must be done after calling
    // UpdateView above.
    //
    window->SetMergeViewLimits(false);

    if (updatesEnabled)
    {
        if (immediateUpdate)
        {
            //
            // Enable updates in the window now.
            //
            window->EnableUpdates();
        }
        else
        {
            //
            // Send a message to the rendering thread to render the window.
            //
            window->SendUpdateMessage();
        }
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::UpdateColorTable
//
//  Purpose: 
//    Updates the color table for each plot in the plot list.
//
//  Arguments:
//    ctName : The name of the updated color table.
//
//  Returns:    Returns true if any plots updated their colortables.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 14 16:35:48 PST 2001
//
// ****************************************************************************

bool
ViewerPlotList::UpdateColorTable(const char *ctName)
{
    bool retval = false;

    for (int i = 0; i < nPlots; i++)
    {
        ViewerPlot *plot = plots[i].plot;
        retval |= plot->UpdateColorTable(ctName);
    }

    return retval;
}


// ****************************************************************************
//  Method: ViewerPlotList::SetBackgroundColor
//
//  Purpose: 
//    Sets the background color for each plot in the plot list.
//
//  Arguments:
//    bg        The background color. 
//
//  Returns:    Returns true if any plots need an update as a result of setting
//              their bg color.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 25, 2001 
//
// ****************************************************************************

bool
ViewerPlotList::SetBackgroundColor(const double *bg)
{
    bool retval = false;
    for (int i = 0; i < nPlots; i++)
    {
        ViewerPlot *plot = plots[i].plot;
        retval |= plot->SetBackgroundColor(bg);
    }
    bgColor[0] = bg[0]; bgColor[1] = bg[1]; bgColor[2] = bg[2];
    return retval;
}


// ****************************************************************************
//  Method: ViewerPlotList::SetForegroundColor
//
//  Purpose: 
//    Sets the foreground color for each plot in the plot list.
//
//  Arguments:
//    fg        The foregound color. 
//
//  Returns:    Returns true if any plots need an update as a result of setting
//              their fg color.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 26, 2001 
//
// ****************************************************************************

bool
ViewerPlotList::SetForegroundColor(const double *fg)
{
    bool retval = false;
    for (int i = 0; i < nPlots; i++)
    {
        ViewerPlot *plot = plots[i].plot;
        retval |= plot->SetForegroundColor(fg);
    }
    fgColor[0] = fg[0]; fgColor[1] = fg[1]; fgColor[2] = fg[2];
    return retval;
}


// ****************************************************************************
//  Method: ViewerPlotList::UpdatePlotAtts
//
//  Purpose:
//    Update the client plot attributes subject attributes.
//
//  Arguments:
//    updateThoseNotRepresented : Tells the routine to send updates for
//                                plots and operators that are not represented
//                                in the plot list. Defaults to true.
//
//  Programmer: Eric Brugger
//  Creation:   August 28, 2000
//
//  Modifications:
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Fri Apr 11 11:19:51 PDT 2003
//    I added support for an active operator. This lets us see attributes
//    for multiple operators of the same type applied to a single plot. I also
//    added the updateThoseNotRepresented argument
//
// ****************************************************************************

void
ViewerPlotList::UpdatePlotAtts(bool updateThoseNotRepresented) const
{
    int frame = animation->GetFrameIndex();

    //
    // Return if this isn't the active animation.
    //
    if (animation != ViewerWindowManager::Instance()->GetActiveAnimation())
    {
        return;
    }

    //
    // Form a list of the counts of each plot that is referenced in
    // the animation.  First we zero the list.  Then we loop over all
    // the plots and increment the count for any plots which are defined
    // at the current frame and are active.  If the plot is referenced for
    // the first frame then the client plot attributes are set based on
    // that plot. Then go over the list of counts and set the client
    // plot attributes based on the default attributes for any plot which
    // hasn't been referenced.
    //
    int       i;
    ViewerPlotFactory *plotFactory = viewerSubject->GetPlotFactory();
    int       nPlotType = plotFactory->GetNPlotTypes();
    int       *plotCount = new int[nPlotType];
    ViewerOperatorFactory *operatorFactory =
               viewerSubject->GetOperatorFactory();
    int       nOperatorType = operatorFactory->GetNOperatorTypes();
    int       *operatorCountInCurrentPlot = new int[nOperatorType];
    ViewerOperator **operatorForType = new ViewerOperator*[nOperatorType];

    for (i = 0; i < nPlotType; i++)
    {
        plotCount[i] = 0;
    }
    for (i = 0; i < nOperatorType; i++)
    {
        operatorForType[i] = 0;
    }

    for (i = 0; i < nPlots; i++)
    {
        ViewerPlot *plot = plots[i].plot;
        int         plotType = plot->GetType();

        //
        // If the plot is within range of the frame and is active, bump
        // the count and set the plot attributes if this is the first
        // frame the plot type is encountered.
        //
        if (plot->IsInFrameRange(frame) == true && plots[i].active == true)
        {
            plotCount[plotType]++;
            if (plotCount[plotType] == 1)
            {
                plot->SetClientAttsFromPlot(frame);
            }

            // Reset the count array for this plot.
            int j;
            for (j = 0; j < nOperatorType; ++j)
                operatorCountInCurrentPlot[j] = 0;

            //
            // Loop over all the operators, counting the types of operators
            // that are applied to this plot.
            //
            for (j = 0; j < plot->GetNOperators(); j++)
            {
                ViewerOperator *oper = plot->GetOperator(j);
                int             operType = oper->GetType();
                operatorCountInCurrentPlot[operType]++;
            }

            //
            // Now that we have a count of how many operators of each type are
            // applied to the current plot, pick the correct operator to send
            // back to the client.
            //
            for (j = 0; j < plot->GetNOperators(); ++j)
            {
                ViewerOperator *oper = plot->GetOperator(j);
                int             operType = oper->GetType();

                // If we already have a pointer to the operator that we want,
                // skip to the next operator for consideration.
                if(operatorForType[operType])
                    continue;

                if (operatorCountInCurrentPlot[operType] > 1)
                {
                    int firstIndex = -1;
                    int activeIndex = -1; 

                    //
                    // We have more than one operator of this type, we need to
                    // go through and see if one of these operators is the
                    // active operator. If one of these operators is the
                    // active operator, set the attributes using it. Otherwise, 
                    // if none of these operators is the active operator, use
                    // the first one of this type.
                    //
                    for(int k = 0; k < plot->GetNOperators(); ++k)
                    {
                        oper = plot->GetOperator(k);
                        int operType2 = oper->GetType();

                        if(operType == operType2)
                        {
                            if(firstIndex == -1)
                                firstIndex = k;
                            if(k == plot->GetActiveOperatorIndex())
                            {
                                activeIndex = k;
                                break;
                            }
                        }
                    }

                    //
                    // None of the operators of this type were the active
                    // operator so use the first instance of this operator type.
                    //
                    if(activeIndex == -1)
                        activeIndex = firstIndex;

                    operatorForType[operType] = plot->GetOperator(activeIndex);
                }
                else if(operatorCountInCurrentPlot[operType] > 0)
                {
                    operatorForType[operType] = plot->GetOperator(j);
                }
            }
        }
    }

    //
    // Send the default attributes for any plots which have not been
    // referenced in the plot list in we're sending updates for plots
    // that have not been referenced.
    //
    if(updateThoseNotRepresented)
    {
        for (i = 0; i < nPlotType; i++)
        {
            if (plotCount[i] == 0)
                plotFactory->SetClientAttsFromDefault(i);
        }
    }

    //
    // Send the operator attributes for referenced operators or send the
    // defaults for operators that have not been referenced if we're sending
    // defaults.
    //
    for (i = 0; i < nOperatorType; i++)
    {
        if(operatorForType[i])
            operatorForType[i]->SetClientAttsFromOperator();
        else if (updateThoseNotRepresented)
            operatorFactory->SetClientAttsFromDefault(i);
    }

    delete [] plotCount;
    delete [] operatorForType;
    delete [] operatorCountInCurrentPlot;
}

// ****************************************************************************
//  Method: ViewerPlotList::GetCurrentPlotAtts
//
//  Purpose:
//     Populate an STL vector with the current attributes of every active,
//     realized, non-hidden plot in the plot list AND in the current frame.
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03 
//
// ****************************************************************************

void
ViewerPlotList::GetCurrentPlotAtts(
   std::vector<const char*>&             pluginIDsList,
   std::vector<std::string>&             hostsList,
   std::vector<int>&                     plotIdsList,
   std::vector<const AttributeSubject*>& attsList) const
{
    int frame = animation->GetFrameIndex();

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].plot->IsInFrameRange(frame) && !plots[i].hidden &&
            plots[i].realized)
        {
           ViewerPlot *plot = plots[i].plot;

           pluginIDsList.push_back(plot->GetPluginID());
           hostsList.push_back(std::string(plot->GetHostName()));
           plotIdsList.push_back(plot->GetNetworkID());
           attsList.push_back(plot->GetCurrentPlotAtts());

        }
    }
}


// ****************************************************************************
//  Method: ViewerPlotList::UpdatePlotList
//
//  Purpose:
//    Update the client plot list attributes subject attributes.
//
//  Programmer: Eric Brugger
//  Creation:   September 5, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Sep 8 16:39:31 PST 2000
//    Added code to set the completion state of the plot and its 
//    database name. Changed to use PC_PLOT.
//
//    Brad Whitlock, Mon Sep 25 12:56:18 PDT 2000
//    I added code to store the ViewerPlot's operator list each Plot that
//    is returned in the PlotList. I also added code to prepend the hostname
//    onto the database name that is returned to the GUI.
//
//    Brad Whitlock, Tue Apr 24 14:49:33 PST 2001
//    I added code to set the plot's completion state when it has an error.
//
//    Brad Whitlock, Tue Nov 12 14:25:13 PST 2002
//    I moved the tool update code to the start of the method.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Brad Whitlock, Thu Apr 3 16:50:46 PST 2003
//    I made the plot be pending only if it is within the time range.
//
//    Brad Whitlock, Fri Apr 11 09:27:52 PDT 2003
//    I added expandedFlag and activeOperator to the plot objects that
//    get returned to the client.
//
// ****************************************************************************

void
ViewerPlotList::UpdatePlotList() const
{
    //
    // Update the tools.
    //
    animation->UpdateTools();

    //
    // Return if this isn't the active animation.
    //
    if (animation != ViewerWindowManager::Instance()->GetActiveAnimation())
    {
        return;
    }

    //
    // Update the client PlotList attribute subject.
    //
    clientAtts->ClearPlots();
    for (int i = 0; i < nPlots; i++)
    {
        int       j;
        Plot      plot;

        plot.SetPlotType(plots[i].plot->GetType());
        plot.SetActiveFlag(plots[i].active);
        plot.SetHiddenFlag(plots[i].hidden);
        plot.SetExpandedFlag(plots[i].plot->GetExpanded());
        plot.SetBeginFrame(plots[i].plot->GetBeginFrame());
        plot.SetEndFrame(plots[i].plot->GetEndFrame());
 
        // Set the keyframe indices.
        int nIndices;
        const int *keyframeIndices=plots[i].plot->GetKeyframeIndices(nIndices);
        intVector keyframeIndices2;
        for (j = 0; j < nIndices; j++)
            keyframeIndices2.push_back(keyframeIndices[j]);
        plot.SetKeyframes(keyframeIndices2);
        // Set the database keyframe indices.
        const int *databaseKeyframeIndices=
            plots[i].plot->GetDatabaseKeyframeIndices(nIndices);
        intVector databaseKeyframeIndices2;
        for (j = 0; j < nIndices; j++)
        {
            databaseKeyframeIndices2.push_back(databaseKeyframeIndices[j]);
        }
        plot.SetDatabaseKeyframes(databaseKeyframeIndices2);

        // Figure out the stage of completion that the plot is at.
        if (plots[i].plot->GetErrorFlag())
        {
            plot.SetStateType(Plot::Error);
        }
        else
        {
            if (plots[i].realized)
            {
                if(plots[i].plot->IsInFrameRange(animation->GetFrameIndex()) &&
                   plots[i].plot->NoActorExists(animation->GetFrameIndex()))
                    plot.SetStateType(Plot::Pending);
                else
                    plot.SetStateType(Plot::Completed);
            }
            else
                plot.SetStateType(Plot::NewlyCreated);
        }

        // Set the plot variable.
        plot.SetPlotVar(plots[i].plot->GetVariableName());

        // Set the operators that are applied to the plot
        for (int op_index = 0; op_index < plots[i].plot->GetNOperators();
            ++op_index)
        {
            plot.AddOperator(plots[i].plot->GetOperator(op_index)->GetType());
        }
        plot.SetActiveOperator(plots[i].plot->GetActiveOperatorIndex());

        // Set the database name and add the plot to the plot list.
        std::string hostDatabaseName(plots[i].plot->GetHostName());
        hostDatabaseName += ":";
        hostDatabaseName += plots[i].plot->GetDatabaseName();
        plot.SetDatabaseName(hostDatabaseName);
        clientAtts->AddPlot(plot, plots[i].id);
    }
    clientAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerPlotList::UpdateSILRestrictionAtts
//
//  Purpose: 
//    Updates the client SIL restriction attributes. This has the effect of
//    sending the new SIL restriction attributes to the client.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 21 14:09:00 PST 2001
//
// ****************************************************************************

void
ViewerPlotList::UpdateSILRestrictionAtts()
{
    // Find the first selected plot.
    int index = -1;
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active)
        {
            index = i;
            break;
        }
    }

    // If a plot was active, put its SIL restriction into the
    // clientSILRestrictionAtts and send them to the client.
    if (index > -1)
    {
        avtSILRestriction_p sr = plots[index].plot->GetSILRestriction();
        SILRestrictionAttributes *sra = sr->MakeAttributes();

        // Copy the SIL restriction into clientSILRestrictionAtts.
        *GetClientSILRestrictionAtts() = *sra;
        GetClientSILRestrictionAtts()->Notify();

        // Delete the sra now that we no longer need it.
        delete sra;
    }
    else
    {
        // There was no selected plot. Send an empty SIL restriction
        // to the client.
        SILRestrictionAttributes emptySRA;
        *GetClientSILRestrictionAtts() = emptySRA;
        GetClientSILRestrictionAtts()->Notify();
    }
}

// ****************************************************************************
// Method: ViewerPlotList::UpdateExpressionList
//
// Purpose: 
//   Sends an updated expression list to the client.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 24 16:47:12 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::UpdateExpressionList(bool considerPlots)
{
    ExpressionList *exprList = ParsingExprList::Instance()->GetList();

    //
    // Create a new expression list that contains all of the expressions
    // from the main expression list that are not expressions that come
    // from databases.
    //
    ExpressionList newList;
    for(int i = 0; i < exprList->GetNumExpressions(); ++i)
    {
        const Expression &expr = exprList->GetExpression(i);
        if(!expr.GetFromDB())
            newList.AddExpression(expr);
    }

    //
    // If there are selected plots, use the database from the first selected
    // plot. Otherwise, use the "open" database.
    //
    std::string host(hostName), db(databaseName);
    int t = animation->GetFrameIndex();
    if(considerPlots && nPlots > 0)
    {
        for(int i = 0; i < nPlots; ++i)
        {
            if(plots[i].active)
            {
                host = plots[i].plot->GetHostName();
                db = plots[i].plot->GetDatabaseName();
                t = plots[i].plot->GetDatabaseState(animation->GetFrameIndex());
                break;
            }
        }
    }

    //
    // Try and get the metadata for the database.
    //
    if(host.size() > 0 && db.size() > 0)
    {
        ViewerFileServer *fileServer = ViewerFileServer::Instance();
        const avtDatabaseMetaData *md = fileServer->GetMetaData(host, db, t);
        if(md != 0)
        {
            // Add the expressions for the database.
            for (int j = 0 ; j < md->GetNumberOfExpressions(); ++j)
                newList.AddExpression(*(md->GetExpression(j)));
        }
    }

    //
    // If the new expression list is different from the expression list
    // that we already have, save the new expression list and send it to
    // the client.
    //
    if(newList != *exprList)
    {
        *exprList = newList;
        exprList->Notify();
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::GetPlotLimits
//
//  Purpose:
//    Get the limits of the plots in the plot list.
//
//  Arguments:
//    frame     The frame for which to evaluate the limits.
//    limits    The limits of the plots in the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   February 23, 2001
//
//  Modifications:
//    Hank Childs, Thu Sep  6 16:23:54 PDT 2001
//    Freed plot extents.
//
//    Brad Whitlock, Mon Mar 4 11:04:00 PDT 2002
//    Added a check to prevent a NULL pointer from being used.
//
//    Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//    Fix UMR. 
//
//    Brad Whitlock, Thu Jul 18 12:11:42 PDT 2002
//    Added a check so it does not use plots that have an error when
//    trying to calculate the limits.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
//    Eric Brugger, Mon Jan 13 16:31:06 PST 2003
//    I added the nDimensions argument to avoid exceeding the bounds of
//    limits when it was 2 dimensional.
//
// ****************************************************************************

void
ViewerPlotList::GetPlotLimits(int frame, int nDimensions, double *limits) const
{
    //
    // Loop over the plots, calculating their extents.  Note that hidden
    // are included in the calculation of the extents so that the view
    // doesn't change as plots are shown and hidden.
    //
    for (int j = 0; j < nDimensions; j++)
    {
        limits[2*j]   =  DBL_MAX;
        limits[2*j+1] = -DBL_MAX;
    }

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].plot->IsInFrameRange(frame) &&
            !plots[i].plot->GetErrorFlag() &&
            plots[i].realized == true)
        {
            double *plotExtents = plots[i].plot->GetSpatialExtents(frame);

            if (plotExtents)
            {
                for (int j = 0; j < nDimensions; j++)
                {
                    limits[2*j]   = min(limits[2*j],   plotExtents[2*j]);
                    limits[2*j+1] = max(limits[2*j+1], plotExtents[2*j+1]);
                }

                delete [] plotExtents;
            }
        }
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::SetSpatialExtentsType
//
//  Purpose: 
//    Sets the flavor of spatial extents to use.
//
//  Arguments:
//    setype    The spatial extents type. 
//
//  Programmer: Hank Childs 
//  Creation:   July 15, 2002
//
// ****************************************************************************

void
ViewerPlotList::SetSpatialExtentsType(avtExtentType setype)
{
    spatialExtentsType = setype;    
    for (int i = 0; i < nPlots; i++)
    {
        ViewerPlot *plot = plots[i].plot;
        plot->SetSpatialExtentsType(setype);
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::HandleTool
//
//  Purpose: 
//    Handles a request by a tool to change plot attributes.
//
//  Arguments:
//    ti : A reference to a tool interface object.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 9 14:49:54 PST 2001
//
//  Modifications:
//    Eric Brugger, Thu Oct 25 08:25:11 PDT 2001
//    I added code to update the plot list so that any plots whose operator
//    attributes changed as a result of the tool changing would change color
//    in the GUI plot list.
//
//    Brad Whitlock, Mon Feb 11 14:03:11 PST 2002
//    Modified the code so it only updates the plot list if some of the
//    plots were modified by the tool. Only selected plots are modified now.
//
//    Brad Whitlock, Thu Apr 11 17:31:34 PST 2002
//    Added applyToAll.
//
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
// ****************************************************************************

void
ViewerPlotList::HandleTool(const avtToolInterface &ti, bool applyToAll)
{
    bool val = false;

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].active || applyToAll)
        {
            if (keyframeMode)
                val |= plots[i].plot->HandleTool(animation->GetFrameIndex(), ti);
            else
                val |= plots[i].plot->HandleTool(ti);
        }
    }

    if (val)
    {
        //
        // Update the client attributes.
        //
        UpdatePlotList();
        UpdatePlotAtts(false);

        //
        // Update the frame.
        //
        animation->UpdateFrame();
    }
}


// ****************************************************************************
// Method: ViewerPlotList::InitializeTool
//
// Purpose: 
//   Initializes a tool as it is being turned on.
//
// Arguments:
//   ti : The tool interface that we're going to use for initialization.
//
// Returns: Whether or not the tool was initialized.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 11 14:09:16 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::InitializeTool(avtToolInterface &ti)
{
    bool retval = false;

    for (int i = 0; i < nPlots && !retval; i++)
    {
        if (plots[i].active)
            retval |= plots[i].plot->InitializeTool(ti);
    }

    return retval;
}


// ****************************************************************************
//  Method: ViewerPlotList::ClearPipelines
//
//  Purpose: 
//    Clear the pipelines in the inclusive frame interval.
//
//  Arguments:
//    f0 : The start frame.
//    f1 : The end frame.
//
//  Programmer: Eric Brugger
//  Creation:   November 21, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov 18 07:50:16 PST 2002
//    I added support for keyframing.
//
// ****************************************************************************

void
ViewerPlotList::ClearPipelines(const int f0, const int f1)
{
    for (int i = 0; i < nPlots; i++)
    {
        plots[i].plot->ClearActors(f0, f1);
    }
}

#ifdef VIEWER_MT
// ****************************************************************************
//  Function: PThreadCreate
//
//  Purpose:
//    Execute a function on a new thread.
//
//  Arguments:
//    new_thread_ID  The id of the new thread.
//    attr           The attributes for creating the thread.
//    start_func     The function to execute in the thread.
//    arg            Data to send the function.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

static void
PthreadCreate(pthread_t *new_thread_ID, const pthread_attr_t *attr,
    void * (*start_func)(void *), void *arg)
{
    int err;

    if (!new_thread_ID)
    {
        printf("TID cannot be NULL.\n");
        abort();
    }
    if (err = pthread_create(new_thread_ID, attr, start_func, arg))
    {
        printf("%s\n", strerror(err));
        abort();
    }
}
#endif

#ifdef VIEWER_MT
// ****************************************************************************
//  Function: PthreadAttrInit
//
//  Purpose:
//    Initialize the pthread attributes.
//
//  Arguments:
//    attr      The pthread attributes.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
// ****************************************************************************

static void
PthreadAttrInit(pthread_attr_t *attr)
{
    int err;

    if (err = pthread_attr_init(attr))
    {
        printf("%s\n", strerror(err));
        abort();
    }
}
#endif

// ****************************************************************************
//  Method: CreatePlot
//
//  Purpose:
//    Create the actor for a plot.  This function exists so that the plot
//    can be made on a new thread.
//
//  Arguments:
//    info      Information about the plot to create.
//
//  Programmer: Eric Brugger
//  Creation:   August 3, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Nov 2 11:52:20 PDT 2000
//    I made the call to delete use the plotInfo pointer to eliminate a
//    compiler warning.
//
//    Jeremy Meredith, Fri Nov  9 10:06:34 PST 2001
//    Added a call to set the current window attributes on engine.
//
//    Brad Whitlock, Fri Feb 22 16:49:37 PST 2002
//    Removed engine proxy references.
//
//    Brad Whitlock, Wed Feb 27 15:09:53 PST 2002
//    Made the animation update its windows only when the viewer is
//    multithreaded. This should make it not update the window when creating
//    each plot in the plot list.
//
//    Brad Whitlock, Thu Jul 25 16:51:31 PST 2002
//    I added code to catch AbortException which is now rethrown from
//    the call to CreateActor.
//
// ****************************************************************************

void *
CreatePlot(void *info)
{
    PlotInfo  *plotInfo=(PlotInfo *)info;

    TRY
    {
        plotInfo->animation->SetWindowAtts(plotInfo->plot->GetHostName());
        plotInfo->plot->CreateActor(plotInfo->animation->GetFrameIndex());
    }
    CATCH(AbortException)
    {
        plotInfo->plotList->InterruptUpdatePlotList();
    }
    ENDTRY

#ifdef VIEWER_MT
    plotInfo->animation->UpdateWindows(false);
    plotInfo->plotList->UpdatePlotList();
#endif

    delete plotInfo;

    return (void *) 0;
}


// ****************************************************************************
//  Method: ViewerPlotList::StartPick
//
//  Purpose: Start pick mode.
//
//  Arguments:
//    frame    The frame to use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 26, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 26 10:15:00 PST 2002 
//    Removed return value, properly use update methods.
//
//    Kathleen Bonnell, Wed Mar 26 17:07:31 PST 2003 
//    Send user a message when re-execution is necessary, and when
//    it is finished.
//
//    Brad Whitlock, Wed Apr 16 14:03:07 PST 2003
//    I removed an unneccessary call to UpdatePlotAtts.
//
// ****************************************************************************

void 
ViewerPlotList::StartPick(const int frame)
{
    bool needsUpdate = false;
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active && !plots[i].hidden)
        {
            needsUpdate |= plots[i].plot->StartPick(frame); 
        }
    }
    if (needsUpdate)
    {
        char msg[350];
        SNPRINTF(msg, 350, "%s%s%s%s%s%s", "VisIt does not have all the ",
                 "information it needs to perform a pick.  Please wait ",
                 "while the necessary information is calculated.  All ",
                 "current pick selections have been cached and will be ",
                 "performed when calculations are complete.  VisIt will ",
                 "notify you when it is fully ready for more picks.");
        //
        //  Using "Warning" instead of "Message" so that it pops up.
        //
        Warning(msg);
  
        UpdatePlotList();
        animation->UpdateFrame();
        Warning("Pick mode now fully ready." );
    }
}


// ****************************************************************************
//  Method: ViewerPlotList::StopPick
//
//  Purpose: Stop pick mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 26, 2001 
//
// ****************************************************************************

void 
ViewerPlotList::StopPick()
{
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active && !plots[i].hidden)
        {
            plots[i].plot->StopPick(); 
        }
    }
}


// ****************************************************************************
//  Method: ViewerPlotList::GetVarName
//
//  Purpose:    Retrieve the variable name associated with the
//              first active, realized non-hidden plot in the list.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 19, 2002 
//
// ****************************************************************************

const char * 
ViewerPlotList::GetVarName()
{
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active && plots[i].realized && !plots[i].hidden)
        {
            return plots[i].plot->GetVariableName();
        }
    }
    return NULL;
}

// ****************************************************************************
//  Method: ViewerPlotList::GetPlot
//
//  Purpose:
//    Return a pointer to the specified plot.
//
//  Arguments:
//    id        A 0-origin index indicating the plot to return.
//
//  Returns:    A pointer to the specified plot.  NULL if id is out of range.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 28, 2002
//
// ****************************************************************************

ViewerPlot *
ViewerPlotList::GetPlot(const int id) const
{
    if (id >= 0 && id < nPlots)
        return plots[id].plot;

    return NULL;
}


// ****************************************************************************
// Method: ViewerPlotList::GetActivePlotIDs
//
// Purpose: 
//   Returns the index of all the active, realized, non-hidden plots.
//
// Returns:    
//   The 0-origin index of all the active, realized, non-hidden plots.
//
// Programmer: Kathleen Bonnell 
// Creation:   May 28, 2002 
//
// Modifications:
//
//   Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//   Renamed from GetPlotID.  Made it return a vector of ids.
//
// ****************************************************************************

void
ViewerPlotList::GetActivePlotIDs(std::vector<int> &ids) const
{
    ids.clear();
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active && plots[i].realized && !plots[i].hidden)
        {
            ids.push_back(i);
        }
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::SetKeyframeMode
//
//  Purpose:
//    Set the keyframe mode for the plot list.
//
//  Arguments:
//    mode      The new keyframe mode.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
 
void
ViewerPlotList::SetKeyframeMode(const bool mode)
{
    keyframeMode = mode;
}

// ****************************************************************************
//  Method: ViewerPlotList::GetKeyframeMode
//
//  Purpose:
//    Get the keyframe mode of the plot list.
//
//  Returns:    The keyframe mode of the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
 
bool
ViewerPlotList::GetKeyframeMode() const
{
    return keyframeMode;
}

// ****************************************************************************
// Method: ViewerPlotList::CreateNode
//
// Purpose: 
//   Lets the plot list save its information for a config file's DataNode.
//
// Arguments:
//   parentNode : The node to which we're saving information.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:09:04 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *plotlistNode = new DataNode("ViewerPlotList");
    parentNode->AddNode(plotlistNode);

    //
    // Add information specific to the animation.
    //
    plotlistNode->AddNode(new DataNode("hostDatabaseName", hostDatabaseName));
    plotlistNode->AddNode(new DataNode("hostName", hostName));
    plotlistNode->AddNode(new DataNode("databaseName", databaseName));
    plotlistNode->AddNode(new DataNode("nPlots", nPlots));
    plotlistNode->AddNode(new DataNode("keyframeMode", keyframeMode));

    //
    // Let all of the plots save themselves to the config file.
    //
    for(int i = 0; i < nPlots; ++i)
    {
        char tmp[20];
        SNPRINTF(tmp, 20, "plot%02d", i);
        DataNode *plotNode = new DataNode(tmp);
        plotlistNode->AddNode(plotNode);

        //
        // Store the plot's attributes up one level, here, so when we read
        // the state back in SetFromNode, it is easier to get at the fields
        // that we need to recreate the right type of ViewerPlot.
        //
        plotNode->AddNode(new DataNode("pluginID",
            std::string(plots[i].plot->GetPluginID())));
        plotNode->AddNode(new DataNode("hostName",
            std::string(plots[i].plot->GetHostName())));
        plotNode->AddNode(new DataNode("databaseName",
            std::string(plots[i].plot->GetDatabaseName())));
        plotNode->AddNode(new DataNode("variableName",
            std::string(plots[i].plot->GetVariableName())));
        plotNode->AddNode(new DataNode("beginFrame", plots[i].plot->GetBeginFrame()));
        plotNode->AddNode(new DataNode("endFrame", plots[i].plot->GetEndFrame()));
        plotNode->AddNode(new DataNode("active", plots[i].active));
        plotNode->AddNode(new DataNode("hidden", plots[i].hidden));
        plotNode->AddNode(new DataNode("realized", plots[i].realized));

        // Store the SIL restriction
        CompactSILRestrictionAttributes *csilr =
            plots[i].plot->GetSILRestriction()->MakeCompactAttributes();
        csilr->CreateNode(plotNode, true);
        delete csilr;

        // Let the plot add its attributes to the node.
        plots[i].plot->CreateNode(plotNode);
    }
}

// ****************************************************************************
// Method: ViewerPlotList::SetFromNode
//
// Purpose: 
//   Lets the plot list reset its values from a config file.
//
// Arguments:
//   parentNode : The config file information DataNode pointer.
//
// Returns:    True if we have created plots that should be realized;
//             False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:10:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::SetFromNode(DataNode *parentNode)
{
    DataNode *node;

    if(parentNode == 0)
        return false;

    DataNode *plotlistNode = parentNode->GetNode("ViewerPlotList");
    if(plotlistNode == 0)
        return false;

    if((node = plotlistNode->GetNode("hostDatabaseName")) != 0)
        hostDatabaseName = node->AsString();
    if((node = plotlistNode->GetNode("hostName")) != 0)
        hostName = node->AsString();
    if((node = plotlistNode->GetNode("databaseName")) != 0)
        databaseName = node->AsString();
    int expectedPlots = 0;
    if((node = plotlistNode->GetNode("nPlots")) != 0)
    {
        // Delete any plots that we may have.
        while(nPlots > 0)
            DeletePlot(plots[0].plot, false);

        // Set the number of plots that we expect to create.
        expectedPlots = (node->AsInt() < 0) ? 0 : node->AsInt();
    }
    if((node = plotlistNode->GetNode("keyframeMode")) != 0)
        keyframeMode = node->AsBool();

    //
    // Try and recreate the plots
    //
    bool sendUpdateFrame = false;
    bool createdPlots = false;
    intVector plotSelected;
    for(int i = 0; i < expectedPlots; ++i)
    {
        char key[20];
        SNPRINTF(key, 20, "plot%02d", i);
        DataNode *plotNode = plotlistNode->GetNode(key);
        if(plotNode == 0)
            continue;

        //
        // Look for the required bits of information to recreate the plot.
        //
        bool haveRequiredFields = true;
        std::string pluginID, plotHost, plotDB, plotVar;        
        if((node = plotNode->GetNode("pluginID")) != 0)
            pluginID = node->AsString();
        else
            haveRequiredFields = false;
        if((node = plotNode->GetNode("hostName")) != 0)
            plotHost = node->AsString();
        else
            haveRequiredFields = false;
        if((node = plotNode->GetNode("databaseName")) != 0)
            plotDB = node->AsString();
        else
            haveRequiredFields = false;
        if((node = plotNode->GetNode("variableName")) != 0)
            plotVar = node->AsString();
        else
            haveRequiredFields = false;

        bool createdPlot = false;
        if(haveRequiredFields)
        {
            //
            // Use the plot plugin manager to get the plot type index from
            // the plugin id.
            //
            int type = PlotPluginManager::Instance()->GetEnabledIndex(pluginID);
            if(type != -1)
            {
                //
                // Try and create the plot.
                //
                ViewerPlot *plot = NewPlot(type, plotHost, plotDB, plotVar, false);

                if(plot)
                {
                    //
                    // Get the start and end frames.
                    //
                    bool haveFrameNumbers = true;
                    int beginFrame = 0, endFrame = 0;
                    if((node = plotNode->GetNode("endFrame")) != 0)
                        endFrame = (node->AsInt() < 0) ? 0 : node->AsInt();
                    else
                        haveFrameNumbers = false;
                    if((node = plotNode->GetNode("beginFrame")) != 0)
                    {
                        int f = node->AsInt();
                        beginFrame = (f < 0 || f > endFrame) ? 0 : f;
                    }
                    else
                        haveFrameNumbers = false;
                    if(haveFrameNumbers)
                        plot->SetFrameRange(beginFrame, endFrame);

                    // Let the plot finish initializing itself.
                    plot->SetFromNode(plotNode);

                    // Add the plot to the plot list.
                    int plotId = SimpleAddPlot(plot, false);

                    // Set the active, hidden flags for the new plot.
                    if((node = plotNode->GetNode("active")) != 0)
                        plotSelected.push_back(node->AsBool()?1:0);
                    else
                        plotSelected.push_back(0);
                    if((node = plotNode->GetNode("hidden")) != 0)
                        plots[plotId].hidden = node->AsBool();
                    if((node = plotNode->GetNode("realized")) != 0)
                    {
                        plots[plotId].realized = node->AsBool();
                        sendUpdateFrame |= node->AsBool();
                    }

                    // Read the SIL restriction
                    if((node = plotNode->GetNode("CompactSILRestrictionAttributes")) != 0)
                    {
                        CompactSILRestrictionAttributes csilr;
                        csilr.SetFromNode(plotNode);

                        // If the sil restriction from the config file has the same
                        // number of sets, then initialize the plot's real SIL restriction
                        // from the compact SIL restriction.
                        avtSILRestriction_p silr = plot->GetSILRestriction();
                        const unsignedCharVector &usedSets = csilr.GetUseSet();
                        int nSets = silr->GetNumSets();
                        if(nSets == usedSets.size())
                        {                     
                            silr->SuspendCorrectnessChecking();
                            silr->TurnOffAll();
                            silr->SetTopSet(csilr.GetTopSet());
                            for(int i = 0; i < nSets; ++i)
                            {
                                if(usedSets[i] == 2 /* AllUsed */)
                                    silr->TurnOnSet(i);
                            }
                            silr->EnableCorrectnessChecking();
                        }
                        else
                        {
                            debug1 << "Could not use the stored SIL restriction because "
                                   << "it was not the right size." << endl;
                        }
                    }

                    createdPlot = true;
                    createdPlots = true;
                }
            }
        }

        // If we could not create the plot, record it in the logs.
        if(!createdPlot)
        {
            debug1 << "Could not create \"" << pluginID.c_str()
                   << "\" plot of: " << plotHost.c_str()
                   << ":" << plotDB.c_str() << " (" << plotVar.c_str() << ")"
                   << endl;
        }
    } // end for

    // Now that all of the plots are added, set the selected flag on each plot.
    // We can't do it as the plots are added because SimpleAddPlot contains
    // code to set all plots but the new one to inactive.
    for(int j = 0; j < nPlots; ++j)
        plots[j].active = (plotSelected[j] > 0);

    return sendUpdateFrame;
}


// ****************************************************************************
// Method: ViewerPlotList::CanMeshPlotBeOpaque
//
// Purpose: 
//   Determines whether or not a mesh plot can honor its opaque-mode flag. 
//   If there are any non-mesh, non-hidden plots, then the mesh plot should
//   not be opaque.
//
// Programmer: Kathleen Bonnell
// Creation:   Thu Aug 28 09:09:25 PDT 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::CanMeshPlotBeOpaque()
{
    int i;
    bool canBeOpaque = true;
    for (i = 0; i < nPlots && canBeOpaque; ++i)
    {
        if (!plots[i].hidden && !plots[i].plot->IsMesh())
            canBeOpaque = false;
    }
    for (i = 0; i < nPlots; ++i)
        plots[i].plot->SetOpaqueMeshIsAppropriate(canBeOpaque);
}
