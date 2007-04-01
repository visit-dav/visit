// ************************************************************************* //
//                                ViewerPlot.C                               //
// ************************************************************************* //

#include <ViewerPlot.h>

#include <string.h>
#include <snprintf.h>

#include <avtActor.h>
#include <avtDataObjectReader.h>
#include <avtDatabaseMetaData.h>
#include <avtDatasetExaminer.h>
#include <avtPlot.h>
#include <avtSILRestrictionTraverser.h>
#include <avtToolInterface.h>
#include <avtTypes.h>

#include <AttributeSubject.h>
#include <AttributeSubjectMap.h>
#include <CompactSILRestrictionAttributes.h>
#include <DatabaseAttributes.h>
#include <OperatorPluginManager.h>
#include <Plot.h>
#include <PlotPluginInfo.h>
#include <PlotQueryInfo.h>
#include <ViewerEngineManager.h>
#include <ParsingExprList.h>
#include <ViewerFileServer.h>
#include <ViewerMessaging.h>
#include <ViewerOperator.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPlotList.h>
#include <ViewerSubject.h>
#include <ViewerWindowManager.h>

#include <DebugStream.h>
#include <DataNode.h>
#include <InvalidLimitsException.h>
#include <NoInputException.h>
#include <ImproperUseException.h>
#include <AbortException.h>
#include <InvalidColortableException.h>
#include <InvalidVariableException.h>
#include <Expression.h>

extern ViewerSubject *viewerSubject;   // FIX_ME This is a hack.

//
// Local macros.
//
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

//
// This is a static member of ViewerPlot.
//
avtActor_p ViewerPlot::nullActor((avtActor *)0);
avtDataObjectReader_p ViewerPlot::nullReader((avtDataObjectReader *)0);
vector<double> ViewerPlot::nullDataExtents;

// ****************************************************************************
//  Method: ViewerPlot constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Dec 8 17:13:20 PST 2000
//    I added arguments to the constructor and added more initialization code.
//
//    Eric Brugger, Tue Mar 08 15:08:26 PST 2001
//    I modified the routine to work with plot plugins.
//
//    Brad Whitlock, Tue Apr 24 14:43:55 PST 2001
//    I added errorFlag.
//
//    Hank Childs, Wed Jun 13 11:05:41 PDT 2001
//    Added silr_.
//
//    Jeremy Meredith, Thu Jul 26 09:53:57 PDT 2001
//    Renamed plugin info to include the word "plot".
//
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Kathleen Bonnell, Wed Sep 26 15:14:07 PDT 2001 
//    Initialize bgColor, fgColor. 
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Jeremy Meredith, Fri Nov  9 10:08:50 PST 2001
//    Added a network id.
//
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Initialize queryAtts. 
//
//    Hank Childs, Mon Jul 15 16:13:33 PDT 2002
//    Initialize spatialExtentsType.
//
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Fri Apr 4 10:24:54 PDT 2003
//    I added the nStates argument so that the plot can be created over
//    frame0 to frame1 but we use nStates-1 as the state at frame1 always.
//    This enables single timestep databases to exist over any number of
//    frames without requiring them to have some large number of database
//    states.
//
//    Brad Whitlock, Fri Apr 11 09:48:58 PDT 2003
//    I initialized some new members.
//
//    Kathleen Bonnell, Wed Aug 27 15:45:45 PDT 2003 
//    Initialize 'isMesh'. 
//
//    Hank Childs, Wed Sep 17 10:20:23 PDT 2003
//    Added pointer to ViewerPlotList.
//
//    Brad Whitlock, Fri Mar 26 08:10:16 PDT 2004
//    Changed to use more strings. I also passed in the initial plot state.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map this plot to the engine used to create it.
//
//    Kathleen Bonnell, Wed Mar 31 16:46:13 PST 2004 
//    Added clonedNetworkId. 
//
//    Brad Whitlock, Thu Apr 8 15:27:48 PST 2004
//    Added support for keyframing back in.
//
//    Kathleen Bonnell, Tue Jan 11 16:16:48 PST 2005
//    Initialize 'isLabel'. 
//
// ****************************************************************************

ViewerPlot::ViewerPlot(const int type_,ViewerPlotPluginInfo *viewerPluginInfo_,
    const EngineKey &ek_, const std::string &hostName_,
    const std::string &databaseName_, const std::string &variableName_,
    avtSILRestriction_p silr_,
    const int plotState,    // The initial database state for the plot
    const int nStates,      // The number of total database states
    const int cacheIndex_,  // The initial active cache index
    const int nCacheEntries // The number of cache entries.
    ) : engineKey(ek_), hostName(hostName_), databaseName(databaseName_),
        variableName(variableName_)
{
    //
    // Make sure the state is not negative.
    //
    int state = (plotState < 0) ? 0 : plotState;

    //
    // Initialize some values.
    //
    type                = type_;
    viewerPluginInfo    = viewerPluginInfo_;
    isMesh = (strcmp(viewerPluginInfo->GetName(), "Mesh") == 0); 
    isLabel = (strcmp(viewerPluginInfo->GetName(), "Label") == 0); 
    followsTime         = true;
    expandedFlag        = false;
    errorFlag           = false;
    networkID           = -1;
    clonedNetworkId     = -1;
    queryAtts           = 0;               
    viewerPlotList      = NULL;
    bgColor[0] = bgColor[1] = bgColor[2] = 1.0; 
    fgColor[0] = fgColor[1] = fgColor[2] = 0.0; 
    spatialExtentsType = AVT_ORIGINAL_EXTENTS;

    //
    // Initialize operator related members.
    //
    nOperators          = 0;
    nOperatorsAlloc     = 0;
    operators           = 0;
    activeOperatorIndex = -1;

    //
    // Allocate some plot attributes
    //
    curPlotAtts         = viewerPluginInfo->AllocAttributes();

    //
    // Set up keyframe attributes so the current plot attributes are
    // set at the first keyframe. Set the last keyframe so that it
    // works like a PaddedIndex correlation would work.
    //
    cacheIndex          = cacheIndex_;
    cacheSize           = nCacheEntries;
    databaseAtts        = new AttributeSubjectMap;
    curDatabaseAtts     = new DatabaseAttributes;
    curDatabaseAtts->SetState(0);
    databaseAtts->SetAtts(0, curDatabaseAtts);
    curDatabaseAtts->SetState(nStates - 1);
    beginCacheIndex = 0;
    endCacheIndex = cacheSize - 1;
    int ci = min(endCacheIndex, nStates - 1);
    databaseAtts->SetAtts(ci, curDatabaseAtts);

    //
    // Initialize the caches so that they are the right size
    // and we have the right active cache entry.
    //
    plotList            = new avtPlot_p[cacheSize];
    actorList           = new avtActor_p[cacheSize];
    readerList          = new avtDataObjectReader_p[cacheSize];
    for(int i = 0; i < cacheSize; ++i)
    {
        plotList[i]   = 0;
        actorList[i]  = 0;
        readerList[i] = 0;
    }

    //
    // Use the constructor's arguments to initialize the object further.
    // Once the object is initialized, set the initialized attributes into
    // the plot keyframes.
    //
    const avtDatabaseMetaData *md = ViewerFileServer::Instance()->
        GetMetaDataForState(hostName, databaseName, state);
    if(md == 0)
        EXCEPTION1(InvalidVariableException, variableName);
    viewerPluginInfo->InitializePlotAtts(curPlotAtts, md,
                                         variableName.c_str());
    plotAtts            = new AttributeSubjectMap;
    plotAtts->SetAtts(0, curPlotAtts);

    //
    // Initialize the plot's SIL restriction.
    //
    silr = silr_;
}

// ****************************************************************************
// Method: ViewerPlot::ViewerPlot
//
// Purpose: 
//   Copy constructor for the ViewerPlot class.
//
// Arguments:
//   obj : The ViewerPlot object to copy.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 2 10:54:56 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

ViewerPlot::ViewerPlot(const ViewerPlot &obj) : engineKey(obj.engineKey),
    hostName(obj.hostName), databaseName(obj.databaseName),
    variableName(obj.variableName)
{
    CopyHelper(obj);
}

// ****************************************************************************
//  Method: ViewerPlot destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Eric Brugger, Tue Mar 08 15:08:26 PST 2001
//    I modified the routine to work with plot plugins.
//
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Delete queryAtts. 
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002
//    Added call to ReleaseData. 
//
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Fri Mar 26 08:13:23 PDT 2004
//    Deleted code to delete host, db arrays since they are now strings.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map this plot to the engine used to create it.
//
// ****************************************************************************

ViewerPlot::~ViewerPlot()
{
    if (networkID != -1)
        ViewerEngineManager::Instance()->ReleaseData(engineKey,
                                                     networkID);

    //
    // Delete the operators.
    //
    if (nOperators > 0)
    {
        for (int i = 0; i < nOperators; i++)
        {
            delete operators[i];
        }
        delete [] operators;
    }
    
    //
    // Delete the database attributes.
    //
    delete databaseAtts;
    delete curDatabaseAtts;

    //
    // Delete the plot attributes.
    //
    delete plotAtts;
    delete curPlotAtts;

    //
    // Delete the list of plots, actors and readers.
    //
    ClearActors();
    delete [] plotList;
    delete [] actorList;
    delete [] readerList;
    if (queryAtts != 0)
        delete queryAtts;
}

// ****************************************************************************
// Method: ViewerPlot::operator =
//
// Purpose: 
//   Assignment operator.
//
// Arguments:
//   obj : The ViewerPlot to copy into this plot.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 2 10:53:13 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::operator = (const ViewerPlot &obj)
{
    engineKey = obj.engineKey;
    hostName = obj.hostName;
    databaseName = obj.hostName;
    variableName = obj.variableName;

    CopyHelper(obj);
}

// ****************************************************************************
// Method: ViewerPlot::CopyHelper
//
// Purpose: 
//   Helps copy most fields into this ViewerPlot.
//
// Arguments:
//   obj : The ViewerPlot to copy.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 2 10:53:56 PDT 2004
//
// Modifications:
//    Kathleen Bonnell, Tue Jan 11 16:16:48 PST 2005
//    Initialize 'isLabel'. 
//   
// ****************************************************************************

void
ViewerPlot::CopyHelper(const ViewerPlot &obj)
{
    int i;

    //
    // Initialize all pointers to 0 so if an exception is thrown out of
    // this function, the object can still be destructed safely.
    //
    queryAtts = 0;
    databaseAtts = 0;
    curDatabaseAtts = 0;
    silr = 0;
    operators = 0;
    plotList = 0;
    actorList = 0;
    readerList = 0;

    //
    // Initialize some values.
    //
    type                = obj.type;
    viewerPluginInfo    = obj.viewerPluginInfo;
    isMesh              = obj.isMesh;
    isLabel             = obj.isLabel;
    followsTime         = obj.followsTime;
    errorFlag           = false;
    networkID           = -1;
    clonedNetworkId     = -1;
    queryAtts           = (obj.queryAtts != 0) ?
        new PlotQueryInfo(*(obj.queryAtts)) : 0;
    viewerPlotList      = obj.viewerPlotList;
    spatialExtentsType  = obj.spatialExtentsType;
    for(i = 0; i < 3; ++i)
    {
        bgColor[i] = obj.bgColor[i];
        fgColor[i] = obj.fgColor[i];
    }

    // Copy the database attributes
    databaseAtts        = new AttributeSubjectMap(*(obj.databaseAtts));
    curDatabaseAtts     = new DatabaseAttributes(*(obj.curDatabaseAtts));
    silr                = new avtSILRestriction(obj.silr);

    // Copy the operators
    nOperators          = obj.nOperators;
    nOperatorsAlloc     = obj.nOperatorsAlloc;
    operators           = new ViewerOperator*[nOperatorsAlloc];
    for(i = 0; i < nOperatorsAlloc; ++i)
    {
        if(i < nOperators)
        {
            operators[i] = new ViewerOperator(*(obj.operators[i]));
            operators[i]->SetPlot(this);
        }
        else
            operators[i] = 0;
    }
    activeOperatorIndex = obj.activeOperatorIndex;
    expandedFlag        = obj.expandedFlag;

    // Copy the plot attributes
    curPlotAtts         = obj.curPlotAtts->NewInstance(true);
    plotAtts            = new AttributeSubjectMap(*(obj.plotAtts));

    // Copy the cache stuff.
    cacheIndex          = obj.cacheIndex;
    beginCacheIndex     = obj.beginCacheIndex;
    endCacheIndex       = obj.endCacheIndex;
    cacheSize           = obj.cacheSize;
    plotList            = new avtPlot_p[cacheSize];
    actorList           = new avtActor_p[cacheSize];
    readerList          = new avtDataObjectReader_p[cacheSize];
    for(i = 0; i < cacheSize; ++i)
    {
        plotList[i]   = 0;
        actorList[i]  = 0;
        readerList[i] = 0;
    }
}

// ****************************************************************************
// Method: ViewerPlot::GetState
//
// Purpose: 
//   Returns the database state currently being used by the plot.
//
// Returns:    The plot's current database state.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:28:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlot::GetState() const
{
    int dbState(cacheIndex);

    //
    // If we're in keyframe mode then the database state is what is
    // stored in the keyFrame
    //
    if(viewerPlotList != 0 && viewerPlotList->GetKeyframeMode())
    {
        databaseAtts->GetAtts(cacheIndex, curDatabaseAtts);
        dbState = curDatabaseAtts->GetState();
    }

    return dbState;
}

// ****************************************************************************
// Method: ViewerPlot::SetCacheIndex
//
// Purpose: 
//   Sets the plot's cacheIndex, which is the index of the actor that is being
//   shown. The cache index corresponds to the time slider.
//
// Arguments:
//   newCacheIndex : The new cache index.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:29:12 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Thu Feb  3 16:02:01 PST 2005
//   Update queryAtts so that lineouts can update when time changes.
//   
// ****************************************************************************

void
ViewerPlot::SetCacheIndex(int newCacheIndex)
{
    if(FollowsTime() &&
       newCacheIndex >= 0 &&
       newCacheIndex < cacheSize)
    {
        if (queryAtts != 0 && newCacheIndex != cacheIndex)
        {
            queryAtts->SetChangeType(PlotQueryInfo::CacheIndex);
            queryAtts->SetOldFrameIndex(cacheIndex);
            queryAtts->SetNewFrameIndex(newCacheIndex);
            queryAtts->Notify();
        }
        cacheIndex = newCacheIndex;
    }
}

// ****************************************************************************
// Method: ViewerPlot::GetCacheIndex
//
// Purpose: 
//   Returns the plot's cache index.
//
// Returns:    The plot's cache index.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:30:11 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlot::GetCacheIndex() const
{
    return cacheIndex;
}

// ****************************************************************************
// Method: ViewerPlot::FollowsTime
//
// Purpose: 
//   Returns whether or not the plot follows a time slider.
//
// Returns:    True if the plot follows a time slider; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:30:30 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::FollowsTime() const
{
    return followsTime;
}

// ****************************************************************************
// Method: ViewerPlot::SetFollowsTime
//
// Purpose: 
//   Sets whether or not the plot follows a time slider.
//
// Arguments:
//   val : Whether the plot follows a time slider.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:31:02 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::SetFollowsTime(bool val)
{
    followsTime = val;
}

// ****************************************************************************
// Method: ViewerPlot::SetKeyframeMode
//
// Purpose: 
//   This method shifts the plot into of out of keyframing mode.
//
// Arguments:
//   kfMode : The keyframe mode.
//
// Note:       When this mode switch is made, the actor cache is blown away
//             and resized. When entering into keyframe mode, simple keyframes
//             are added to approximate what the user sees in the window. When
//             we leave keyframe mode, all keyframes are removed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:31:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::SetKeyframeMode(bool kfMode)
{
    // Clear the actor caches.
    ClearActors();

    // Get the number of states.
    int nStates = 1;
    const avtDatabaseMetaData *md = GetMetaData();
    if(md != 0)
        nStates = md->GetNumStates();
    int cs = 1;

    if(kfMode)
    {
        // Clear the plot and database keyframes.
        plotAtts->ClearAtts();
        databaseAtts->ClearAtts();

        cs = viewerPlotList->GetNKeyframes();

        // If we enter keyframing mode, reinitialize the keyframes.
        plotAtts->SetAtts(0, curPlotAtts);
        curDatabaseAtts->SetState(0);
        databaseAtts->SetAtts(0, curDatabaseAtts);
        curDatabaseAtts->SetState(nStates - 1);
        int ci = min(cs - 1, nStates - 1);
        databaseAtts->SetAtts(ci, curDatabaseAtts);
    }
    else
    {
        // Use the number of states for the plot's database as the new
        // cache size.
        cs = nStates;

        // Clear the plot and database keyframes.
        plotAtts->ClearAtts();
        databaseAtts->ClearAtts();
    }

    // Resize the cache so it will handle the right number if indices.
    ResizeCache(cs);

    cacheIndex = 0;
    beginCacheIndex = 0;
    endCacheIndex = cs - 1;
}

// ****************************************************************************
// Method: ViewerPlot::ResizeCache
//
// Purpose: 
//   Resizes the actor cache and adjusts the plot range if needed.
//
// Arguments:
//   cs : The new cache size.
//
// Notes:      The actors are copied to the resized cache.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:34:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::ResizeCache(int cs)
{
    if(cs != cacheSize)
    {
        avtPlot_p *newPlotList=0;
        avtActor_p *newActorList=0;
        avtDataObjectReader_p *newReaderList=0;
        newPlotList   = new avtPlot_p[cs];
        newActorList  = new avtActor_p[cs];
        newReaderList = new avtDataObjectReader_p[cs];

        //
        // Copy the old entries that fit into the new lists.
        //
        int s_min = min(cs, cacheSize);
        for(int i = 0; i < cs; ++i)
        {
            if(i < s_min)
            {
                newPlotList[i] = plotList[i];
                newActorList[i] = actorList[i];
                newReaderList[i] = readerList[i];
            }
            else
            {
                newPlotList[i] = 0;
                newActorList[i] = 0;
                newReaderList[i] = 0;
            }
        }

        //
        // Swap the new lists for the existing lists.
        //
        delete [] plotList;
        plotList = newPlotList;
        delete [] actorList;
        actorList = newActorList;
        delete [] readerList;
        readerList = newReaderList;

        //
        // Set the new size and make sure that the start, end indices are
        // within the new bounds.
        //
        cacheSize = cs;
        if(cacheIndex >= cacheSize)
            cacheIndex = 0;
        if(beginCacheIndex >= cacheSize)
            beginCacheIndex = 0;
        if(endCacheIndex >= cacheSize)
            endCacheIndex = cacheSize-1;
    }
}

// ****************************************************************************
// Method: ViewerPlot::UpdateCacheSize
//
// Purpose: 
//   This method is called when we do a replace, reopen, etc. It enlarges the
//   actor caches so we can fit more.
//
// Arguments:
//   kfMode      : Whether we're in keyframing mode.
//   clearActors : Whether we should clear the actors before changing the
//                 cache size.
//   size        : The new number of keyframes if we're in keyframe mode.
//                 Otherwise the argument is ignored.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 5 11:28:27 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::UpdateCacheSize(bool kfMode, bool clearActors, int size)
{
    //
    // We were told to clear the actors so clear them now. 
    //
    if(clearActors)
        ClearActors();

    //
    // Get the number of states in the updated database.
    //
    const avtDatabaseMetaData *md = GetMetaData();
    int nStates = md->GetNumStates();

    //
    // We need to update the database keyframes so we have a plot that
    // exists over all of the cache indices.
    //
    if(kfMode)
    {
        DeleteDatabaseKeyframe(cacheSize - 1);
        if(size > 0)
            ResizeCache(size);
        AddDatabaseKeyframe(cacheSize - 1, nStates - 1);
    }
    else
    {
        // We're not in keyframing mode so we only have to update the cache
        // size. There's no need to clear actors, etc. We also don't need to
        // set any keyframes because we're not in kf mode.
        ResizeCache(nStates);
        endCacheIndex = nStates - 1;
    }
}

// ****************************************************************************
// Method: ViewerPlot::SetRange
//
// Purpose: 
//   Sets the cache indices over which a plot is valid.
//
// Arguments:
//   i0 : The starting cache index.
//   i1 : The ending cache index.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:36:46 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::SetRange(const int i0, const int i1)
{
    //
    // Check the index values.
    //
    if (i0 < 0 || i1 < i0)
    {
        debug1 << "ViewerPlot::SetFrameRange: Invalid frame range.\n";
        return;
    }

    if(i1 >= cacheSize)
    {
        debug1 << "ViewerPlot::SetFrameRange: Can't set the frame range "
               << "beyond the end cache index of: " << cacheSize-1 <<".\n";
        return;
    }

    // Clear the actors at the ends of the range.
    ClearActors(0, i0);
    ClearActors(i1, cacheSize-1);

    beginCacheIndex = i0;
    endCacheIndex = i1;
}

// ****************************************************************************
//  Method: ViewerPlot::IsInRange
//
//  Purpose:
//    Return a boolean indicating if the current cache indexs is in the
//    range that the plot is defined over.
//
//  Returns:    A boolean indicating if the current index is within the range.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Sat Jan 31 22:46:11 PST 2004
//    I removed the frame argument.
//
//    Brad Whitlock, Fri Apr 2 11:35:57 PDT 2004
//    I changed some member names and the method name.
//
// ****************************************************************************

bool
ViewerPlot::IsInRange() const
{
    return (cacheIndex >= beginCacheIndex) && (cacheIndex <= endCacheIndex);
}

// ****************************************************************************
//  Method: ViewerPlot::SetHostDatabaseName
//
//  Purpose:
//    Set the host and database names for the plot.
//
//  Arguments:
//    host      The host name to set for the plot.
//    database  The database name to set for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   August 31, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Dec  4 17:59:44 PST 2002
//    Allow the plotAtts to be reinitialzied if a new database is set.
//
//    Kathleen Bonnell, Wed Dec  4 17:59:44 PST 2002
//    Using IntializePlotAtts to reInitialize atts caused problems if
//    user had changed the atts from default.  Call ReInitializePlotAtts
//    instead. 
//
//    Eric Brugger, Thu Dec 19 12:29:22 PST 2002
//    I added keyframing support.
//    
//    Kathleen Bonnell, Tue Mar  4 15:34:36 PST 2003  
//    Update queryAtts. 
//
//    Brad Whitlock, Fri Mar 26 08:03:51 PDT 2004
//    Changed interface to ReInitializePlotAtts. Made host and db name be
//    stored as strings.
//
//    Eric Brugger, Fri Apr 16 13:58:25 PDT 2004
//    I removed the coding which caused the routine to be exited early if
//    the new host and database names matched the existing ones.
//
//    Brad Whitlock, Mon May 3 09:55:24 PDT 2004
//    I added code to set the engine key to be the new host if the host
//    is changing.
//
// ****************************************************************************

void
ViewerPlot::SetHostDatabaseName(const EngineKey &key, 
    const std::string &database)
{
    bool reInit = false;
    //
    // If the host name is already set, clear the actor list and
    // indicate that the plot needs reinitializing.
    //
    if (hostName != "")
    {
        ClearActors();
        reInit = true;
    }

    //
    // If the host and database are different then we need to clear the
    // database keyframes.
    // 
    if(key != engineKey || database != databaseName)
    {
        databaseAtts->ClearAtts();
        if(viewerPlotList->GetKeyframeMode())
        {
            // Add in some new keyframes.
            curDatabaseAtts->SetState(0);
            databaseAtts->SetAtts(0, curDatabaseAtts);
        }
    }

    //
    // Set the engine key if the host name is changing.
    //
    engineKey = key;

    //
    // Set the host and database names.
    //
    hostName = key.OriginalHostName();
    databaseName = database;
    if (reInit)
    {
        viewerPluginInfo->ReInitializePlotAtts(curPlotAtts, GetMetaData(),
            variableName.c_str());

        delete plotAtts;
        plotAtts = new AttributeSubjectMap;
        plotAtts->SetAtts(0, curPlotAtts);
        if (queryAtts != 0)
        {
            queryAtts->SetChangeType(PlotQueryInfo::Database);
            queryAtts->Notify();
        }
    }
}

// ****************************************************************************
//  Method: ViewerPlot::GetHostName
//
//  Purpose:
//    Return the host name for the plot.
//
//  Returns:    The host name for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 25, 2000
//
// Modifications:
//    Brad Whitlock, Fri Mar 26 08:03:13 PDT 2004
//    Changed to string.
//
// ****************************************************************************

const std::string &
ViewerPlot::GetHostName() const
{
    return hostName;
}

// ****************************************************************************
//  Method: ViewerPlot::GetDatabaseName
//
//  Purpose:
//    Return the database name for the plot.
//
//  Returns:    The database name for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   August 31, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Mar 26 08:03:13 PDT 2004
//    Changed to string.
//
// ****************************************************************************

const std::string &
ViewerPlot::GetDatabaseName() const
{
    return databaseName;
}

// ****************************************************************************
// Method: ViewerPlot::GetSource
//
// Purpose: 
//   Returns the plot's source.
//
// Returns:    The plot's source, which is the host + database name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 23:53:37 PST 2004
//
// Modifications:
//   
// ****************************************************************************

std::string
ViewerPlot::GetSource() const
{
    return ViewerFileServer::ComposeDatabaseName(hostName, databaseName);
}

// ****************************************************************************
// Method: ViewerPlot::GetPlotName
//
// Purpose: 
//   Returns the name of the plot.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 7 11:32:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

const char *
ViewerPlot::GetPlotName() const
{
    return viewerPluginInfo->GetName();
}

// ****************************************************************************
// Method: ViewerPlot::GetPluginID
//
// Purpose: 
//   Returns the plugin ID of the plot 
//
// Programmer: Mark C. Miller 
// Creation:   16Apr03 
//   
// ****************************************************************************

const char *
ViewerPlot::GetPluginID() const
{
    return viewerPluginInfo->GetID();
}

// ****************************************************************************
// Method: ViewerPlot::GetMetaData
//
// Purpose: 
//   Returns the metadata for the plot at its current time state.
//
// Returns:    A pointer to the metadata for the plot at its current time
//             state.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 10:35:01 PDT 2004
//
// Modifications:
//   Brad Whitlock, Thu Apr 1 17:42:10 PST 2004
//   I made it use GetState to get the right database so we can return the
//   right state information even when we're keyframing.
//
// ****************************************************************************

const avtDatabaseMetaData *
ViewerPlot::GetMetaData() const
{
    return ViewerFileServer::Instance()->GetMetaDataForState(hostName,
        databaseName, GetState());
}

// ****************************************************************************
// Method: ViewerPlot::GetExpressions
//
// Purpose: 
//   Gets the user-defined expressions plus the expressions from the plot's
//   database at the current database time state.
//
// Returns:    An expression list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 10:11:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

ExpressionList
ViewerPlot::GetExpressions() const
{
    ExpressionList newList;
    ViewerFileServer::Instance()->GetAllExpressions(newList, hostName,
        databaseName, GetState());
    return newList;
}

// ****************************************************************************
//  Method: ViewerPlot::SetVariableName
//
//  Purpose:
//    Set the variable name for the plot.
//
//  Arguments:
//    name      The variable name to set for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Jun  5 20:30:36 PDT 2001
//    Added code to set the variable name of an existing plot.
//
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Eric Brugger, Thu Sep  6 14:16:43 PDT 2001
//    I modified the routine to also set the variable name for any existing
//    plots.
//
//    Brad Whitlock, Fri Sep 21 11:54:13 PDT 2001
//    I added code to update the SIL restriction of the plot if the new
//    variable has a different top set.
//    
//    Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001 
//    Allow the plotAtts to be reset if necessary. 
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Wed Dec 31 14:20:50 PST 2003
//    Added TRY/CATCH block around the call to avtDatabaseMetaData::MeshForVar.
//
//    Jeremy Meredith, Fri Mar 26 15:15:15 PST 2004
//    Added code to print the error message received from an
//    InvalidVariableException.
//
//    Brad Whitlock, Fri Mar 26 08:14:28 PDT 2004
//    I made it use ViewerPlot::GetMetaData and strings.
//
//    Hank Childs, Mon Apr  5 20:41:54 PDT 2004
//    If you are changing the SIL, make a copy, because you might be changing
//    a SIL that is referenced elsewhere.  ['4716]
//
//    Jeremy Meredith, Fri Apr 16 17:52:04 PDT 2004
//    Removed firstTime.  It was probably supposed to be a class member, not
//    a static variable, so I don't think it was ever doing what it was
//    intended to do.  Furthermore, this method is no longer called by the
//    constructor, so the first time anyone tried to change variables, it
//    ignored their request to change important things like subset names.
//
//    Kathleen Bonnell, Tue Nov  2 11:13:15 PST 2004 
//    Added call to set the avtPlot's mesh type. 
// 
//    Kathleen Bonnell, Wed Nov  3 16:51:24 PST 2004 
//    Removed call to set the avtPlot's mesh type. 
// 
// ****************************************************************************

bool
ViewerPlot::SetVariableName(const std::string &name)
{
    bool retval = false;
    bool notifyQuery = false;

    //
    // If the variable name is already set, check if the names match and
    // return if they do, otherwise deallocate the existing name and clear
    // the actor list.
    //
    if (variableName != "")
    {
        if (variableName == name)
            return retval;

        ClearActors();
        notifyQuery = true;

        //
        // Determine if we need to also set a new SIL restriction.
        //
        if(silr->GetWholes().size() > 0)
        {
            TRY
            {
                avtDatabaseMetaData *md = (avtDatabaseMetaData *)GetMetaData();
                if(md != 0)
                {
                    //
                    // Get the Mesh for the desired variable.
                    //
                    std::string meshName;
                    TRY
                    {
                        meshName = md->MeshForVar(std::string(name));
                    }
                    CATCH2(InvalidVariableException,e)
                    {
                        Error(e.Message().c_str());
                        CATCH_RETURN2(2, false);
                    }
                    ENDTRY

                    //
                    // The new variable has a different top set from the
                    // old variable. Set the top set in the SIL restriction.
                    //
                    avtSILSet_p current = silr->GetSILSet(silr->GetTopSet());
                    if (meshName != current->GetName())
                    {
                        avtSILRestriction_p new_sil = 
                                                   new avtSILRestriction(silr);
                        int topSet = 0;
                        for (int i = 0; i < new_sil->GetWholes().size(); i++)
                        {
                            current = 
                                   new_sil->GetSILSet(new_sil->GetWholes()[i]);
                            if(meshName == current->GetName())
                            {
                                topSet = new_sil->GetWholes()[i];
                                break;
                            }
                        }
                        //
                        // Change the top set in the current SIL restriction.
                        // This is sufficient due to the previous call to
                        // ClearActors(). Note that we must select all sets
                        // under the new top set.
                        //
                        new_sil->SetTopSet(topSet);
                        new_sil->TurnOffAll();
                        new_sil->TurnOnSet(topSet);
                        silr = new_sil;

                        //
                        // Set a flag to return that indicates the SIL
                        // restriction was changed.
                        //
                        retval = true;
                    }
                }
            }
            CATCH(VisItException)
            {
                // nothing.
            }
            ENDTRY
        }
    }

    // Save the new variable name.
    variableName = name;


    if (curPlotAtts->VarChangeRequiresReset())
    { 
        const avtDatabaseMetaData *md;
        if((md = GetMetaData()) != 0)
        {
            viewerPluginInfo->ResetPlotAtts(curPlotAtts, md,
                variableName.c_str());
            viewerPluginInfo->SetClientAtts(curPlotAtts);

            delete plotAtts;
            plotAtts = new AttributeSubjectMap;
            plotAtts->SetAtts(0, curPlotAtts);
        }
    }

    //
    // Set the variable name for any existing plots.
    //
    for (int i = 0; i < cacheSize; i++)
    {
        if (*plotList[i] != 0)
        {
            plotList[i]->SetVarName(variableName.c_str());
        }
    }

    if (notifyQuery && queryAtts != 0)
    {
        queryAtts->SetChangeType(PlotQueryInfo::VarName);
        queryAtts->Notify();
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerPlot::GetVariableName
//
//  Purpose:
//    Return the variable name for the plot.
//
//  Returns:    The variable name for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 6, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Mar 26 08:20:59 PDT 2004
//    Made it use string.
//
// ****************************************************************************

const std::string &
ViewerPlot::GetVariableName() const
{
    return variableName;
}

// ****************************************************************************
//  Method: ViewerPlot::DeleteKeyframe
//
//  Purpose:
//    Delete the keyframe located at the specified frame.
//
//  Arguments:
//    frame     The frame at which to delete the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Apr 5 11:53:36 PDT 2004
//    I changed the implementation to not be so "frame-centric".
//
// ****************************************************************************
 
void
ViewerPlot::DeleteKeyframe(const int index)
{
    //
    // Check that the index is within range.
    //
    if ((index < 0) || (index >= cacheSize))
    {
        debug1 << "DeleteKeyframe: The frame is out of range. index=" << index
               << ", indices=[0," << cacheSize-1 << "]" << endl;
        return;
    }

    //
    // Delete the keyframe at the specified frame.  DeleteAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int i0, i1;
    if (!plotAtts->DeleteAtts(index, i0, i1))
        return;

    i1 = (i1 < cacheSize) ? i1 : (cacheSize - 1);
    CheckCache(i0, i1, false);
}

// ****************************************************************************
//  Method: ViewerPlot::MoveKeyframe
//
//  Purpose:
//    Move the position of a keyframe.
//
//  Arguments:
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 29, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Apr 5 11:56:24 PDT 2004
//    I made it not be so "frame-centric".
//
// ****************************************************************************
 
void
ViewerPlot::MoveKeyframe(int oldIndex, int newIndex)
{
    //
    // Check that the frames are within range.
    //
    if ((oldIndex < 0) || (oldIndex >= cacheSize) ||
        (newIndex < 0) || (newIndex >= cacheSize))
    {
        debug1 << "MoveKeyframe: The index is out of range. "
               << "newIndex=" << newIndex
               << ", indices=[0," << cacheSize-1 << "]" << endl;
        return;
    }

    //
    // Move the keyframe at oldFrame to newFrame.  MoveAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int i0, i1;
    if (!plotAtts->MoveAtts(oldIndex, newIndex, i0, i1))
        return;

    i1 = (i1 < cacheSize) ? i1 : (cacheSize - 1);
    CheckCache(i0, i1, false);
}

// ****************************************************************************
//  Method: ViewerPlot::DeleteDatabaseKeyframe
//
//  Purpose:
//    Delete the database keyframe at the specified frame for the plot.
//
//  Arguments:
//    index     The index at which to delete the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   December 26, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Apr 5 14:02:39 PST 2004
//    Changed variable names.
//
// ****************************************************************************

void
ViewerPlot::DeleteDatabaseKeyframe(const int index)
{
    //
    // Check that the frame is within range.
    //
    if ((index < 0) || (index >= cacheSize))
    {
        debug1 << "DeleteDatabaseKeyframe: The index is out of range. "
               << "index=" << index
               << ", indices=[0," << cacheSize-1 << "]" << endl;
        return;
    }

    //
    // Delete the keyframe at the specified frame.  DeleteAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int i0, i1;
    if (!databaseAtts->DeleteAtts(index, i0, i1))
    {
        return;
    }

    i1 = (i1 < cacheSize) ? i1 : (cacheSize - 1); 
    CheckCache(i0, i1, true);
}

// ****************************************************************************
// Method: ViewerPlot::AddDatabaseKeyframe
//
// Purpose: 
//   Adds a database keyframe at the specified index.
//
// Arguments:
//   index : The cache index.
//   state : The database state to use at the specified cache index.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 2 15:13:24 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::AddDatabaseKeyframe(int index, int state)
{
    if(index < 0 || index >= cacheSize)
    {
        debug1 << "AddDatabaseKeyframe: The index " << index
               << " is out of range [0," << cacheSize-1 << "]." << endl;
        return;
    }

    int i0, i1;
    curDatabaseAtts->SetState(state);
    databaseAtts->SetAtts(index, curDatabaseAtts, i0, i1);
    i1 = (i1 < cacheSize) ? i1 : (cacheSize-1);

    CheckCache(i0, i1, true);
}

// ****************************************************************************
//  Method: ViewerPlot::MoveDatabaseKeyframe
//
//  Purpose:
//    Move the position of a database keyframe.
//
//  Arguments:
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 29, 2003
//
//  Modifications:
//    Brad Whitlock, Thu Apr 8 15:38:22 PST 2004
//    I changed the method to suite the new caching scheme.
//
// ****************************************************************************

void
ViewerPlot::MoveDatabaseKeyframe(int oldIndex, int newIndex)
{
    //
    // Check that the frames are within range.
    //
    if ((oldIndex < 0) || (oldIndex >= cacheSize) ||
        (newIndex < 0) || (newIndex >= cacheSize))
    {
        debug1 << "MoveDatabaseKeyframe: The new index is out of range. "
               << "index=" << newIndex
               << ", indices=[0," << cacheSize-1 << "]" << endl;
        return;
    }

    //
    // Move the keyframe at oldFrame to newFrame.  MoveAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int i0, i1;
    if (!databaseAtts->MoveAtts(oldIndex, newIndex, i0, i1))
        return;

    i1 = (i1 < cacheSize) ? i1 : (cacheSize-1);
    CheckCache(i0, i1, true);
}

// ****************************************************************************
//  Method: ViewerPlot::SetSILRestriction
//
//  Purpose:
//    Set the SIL restriction for the plot.
//
//  Arguments:
//    s         The SIL restriction for the plot.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jun 21 15:40:17 PST 2001
//    I added code to clear the actors.
//
//    Eric Brugger, Fri Sep  7 13:18:53 PDT 2001
//    I replaced the code that unconditionally cleared the actor cache
//    with code that only clears actors whose SIL differs from the new SIL.
//
//    Hank Childs, Fri Nov 22 17:08:31 PST 2002
//    Use a SIL traverser to determine if the restrictions are equal.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

void
ViewerPlot::SetSILRestriction(avtSILRestriction_p s)
{
    //
    // Assign the new SIL restriction to the plot.
    //
    silr = s;

    //
    // Loop over the existing plots and delete any cached actors whose SIL
    // restriction is different from the new one.
    //
    for (int i = 0; i < cacheSize; ++i)
    {
        if (*plotList[i] != 0)
        {
            avtSILRestriction_p silp =
                (*plotList[i])->GetCurrentSILRestriction();

            avtSILRestrictionTraverser trav(silp);
            if ( (*silp != NULL) && (!(trav.Equal(silr))) )
            {
                plotList[i]   = (avtPlot *)0;
                actorList[i]  = (avtActor *)0;
                readerList[i] = (avtDataObjectReader *)0;
            }
        }
    }
}

// ****************************************************************************
//  Method: ViewerPlot::GetSILRestriction
//
//  Purpose:
//    Return the SIL restriction for the plot.
//
//  Returns:    The SIL restriction for the plot.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2001
//
// ****************************************************************************

avtSILRestriction_p
ViewerPlot::GetSILRestriction() const
{
    return silr;
}

// ****************************************************************************
// Method: ViewerPlot::GetErrorFlag
//
// Purpose: 
//   Returns the error flag of the plot.
//
// Returns:    The error flag of the plot.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 24 14:44:37 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::GetErrorFlag() const
{
    return errorFlag;
}

// ****************************************************************************
// Method: ViewerPlot::SetErrorFlag
//
// Purpose: 
//   Sets the plot's error flag.
//
// Arguments:
//   val : The new error flag value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 26 16:58:23 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::SetErrorFlag(bool val)
{
    errorFlag = val;
}

// ****************************************************************************
//  Method: ViewerPlot::AddOperator
//
//  Purpose:
//    Add the specified operator type to the plot.
//
//  Arguments:
//    type      The operator type to add to the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 13 11:23:19 PST 2000
//    Made it also use a plot when creating an operator.
//
//    Jeremy Meredith, Thu Jul 26 03:18:52 PDT 2001
//    Removed reference to OperType.
//
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//
//    Kathleen Bonnell, Tue Mar  4 15:20:25 PST 2003
//    Update queryAtts before the actors are cleared. 
//
//    Brad Whitlock, Thu Apr 10 11:41:49 PDT 2003
//    I got rid of a bad check that used an actual operator name. I added
//    code to set the active operator index.
//
//    Brad Whitlock, Thu Jul 17 10:05:04 PDT 2003
//    I made it return the index of the new operator in the operators array.
//
//    Kathleen Bonnell, Thu Sep 11 11:50:02 PDT 2003 
//    Added optional bool arg, indicates whether the operator
//    should be initialized from its default or client atts. 
//
//    Mark C. Miller, Wed Aug 11 17:07:46 PDT 2004
//    Added warning for multiple operators of same type
//
// ****************************************************************************

int
ViewerPlot::AddOperator(const int type, const bool fromDefault)
{
    if (nOperators > 0)
    {
        if (!operators[nOperators-1]->AllowsSubsequentOperators())    
        {
            char msg[200];
            SNPRINTF(msg, sizeof(msg), "VisIt cannot apply other operators after a "
                     "%s operator.", operators[nOperators-1]->GetName());
            Error(msg);
            return -1;
        }

        for (int i = 0; i < nOperators; i++)
        {
            if (type == operators[i]->GetType())
            {
                char msg[500];
                SNPRINTF(msg, sizeof(msg), "You have added the \"%s\" operator "
                    "multiple times. For some operators, like \"Slice\", this "
                    "can lead to an empty plot but is otherwise harmless.",
                    operators[i]->GetName());
                Warning(msg);
                break;
            }
        }
    }

    //
    // Create the operator.
    //
    ViewerOperator *oper =
        viewerSubject->GetOperatorFactory()->CreateOperator(type,this,fromDefault);

    //
    // Expand the list of operators if necessary.
    //
    if (nOperators >= nOperatorsAlloc)
    {
        ViewerOperator **operatorsNew=0;

        nOperatorsAlloc += 4;

        operatorsNew = new ViewerOperator*[nOperatorsAlloc];
        memcpy(operatorsNew, operators, nOperators*sizeof(ViewerOperator*));

        delete [] operators;
        operators = operatorsNew;
    }

    //
    // Add the operator to the list.
    //
    operators[nOperators] = oper;
    activeOperatorIndex = nOperators;
    nOperators++;

    if (queryAtts != 0)
    {
        queryAtts->SetChangeType(PlotQueryInfo::AddOp);
        queryAtts->Notify();
    }

    //
    // Clear the actor cache.
    //
    ClearActors();

    return nOperators-1;
}

// ****************************************************************************
// Method: ViewerPlot::MoveOperator
//
// Purpose: 
//   Moves an operator within the plot's operator list.
//
// Arguments:
//   operatorIndex : The index of the operator that we want to move.
//   promote       : Whether we're promoting or demoting.
//
// Returns:    True if operator order was changed; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 12:15:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::MoveOperator(const int operatorIndex, bool promote)
{
    bool retval = false;

    //
    // Only try to remove the operator if we have operators and the
    // one we want to remove is within range.
    //
    if(nOperators > 0 && operatorIndex >= 0 && operatorIndex < nOperators)
    {
        char msg[100];

        bool isFirst = (!promote && operatorIndex == 0);
        bool isLast = (promote && operatorIndex == nOperators - 1);

        if(isFirst || isLast)
        {
            SNPRINTF(msg, 100, "VisIt cannot move the %s operator because "
                "it is already the %s operator.",
                operators[operatorIndex]->GetName(),
                isFirst ? "firat":"last");
            Error(msg);
        }
        else if(operators[operatorIndex]->Moveable())
        {
            int newOperatorIndex = promote ? operatorIndex + 1 :
                operatorIndex - 1;

            // Swap the operator pointers in the list to reorder them.
            ViewerOperator *tmp = operators[newOperatorIndex];
            operators[newOperatorIndex] = operators[operatorIndex];
            operators[operatorIndex] = tmp;

            // If we're moving the active operator, update the active
            // operator index.
            if(operatorIndex == activeOperatorIndex)
                activeOperatorIndex = newOperatorIndex;

            if (queryAtts != 0)
            {
                queryAtts->SetChangeType(PlotQueryInfo::MoveOperator);
                queryAtts->Notify();
            }

            //
            // Clear the actor cache so the plot will have to be regenerated.
            //
            ClearActors();

            retval = true;
        }
        else
        {
            SNPRINTF(msg, 100, "VisIt cannot move a %s operator.",
                     operators[operatorIndex]->GetName());
            Error(msg);
        }
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerPlot::PromoteOperator
//
// Purpose: 
//   Promotes the specified operator within the plot's operator list.
//
// Arguments:
//   operatorIndex : The index of the operator to promote.
//
// Returns:    True if the operator was promoted; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 12:16:56 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::PromoteOperator(const int operatorIndex)
{
    return MoveOperator(operatorIndex, true);
}

// ****************************************************************************
// Method: ViewerPlot::DemoteOperator
//
// Purpose: 
//   Demotes the specified operator within the plot's operator list.
//
// Arguments:
//   operatorIndex : The index of the operator to demote.
//
// Returns:    True if the operator was demoted; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 12:17:51 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::DemoteOperator(const int operatorIndex)
{
    return MoveOperator(operatorIndex, false);
}

// ****************************************************************************
// Method: ViewerPlot::RemoveOperator
//
// Purpose: 
//   Removes an operator from the plot's operator list.
//
// Arguments:
//   operatorIndex : The index of the operator to remove.
//
// Returns:    True if an operator was removed; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 10:59:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::RemoveOperator(const int operatorIndex)
{
    bool retval = false;

    //
    // Only try to remove the operator if we have operators and the
    // one we want to remove is within range.
    //
    if(nOperators > 0 && operatorIndex >= 0 && operatorIndex < nOperators)
    {
        if(operators[operatorIndex]->Removeable())
        {
            // Delete the specified operator.
            delete operators[operatorIndex];
            // Remove the gap from the operator list.
            for(int i = operatorIndex; i < nOperators - 1; ++i)
                operators[i] = operators[i + 1];
            operators[nOperators - 1] = 0;
            // Decrement the operator count
            --nOperators;

            //
            // If we deleted the active operator, decrement the active 
            // operator index so it points to an operator that is still
            // in the list.
            //
            if(activeOperatorIndex == operatorIndex)
            {
                --activeOperatorIndex;
                if(nOperators > 0)
                    activeOperatorIndex = 0;
            }               

            if (queryAtts != 0)
            {
                queryAtts->SetChangeType(PlotQueryInfo::RemoveOperator);
                queryAtts->Notify();
            }

            //
            // Clear the actor cache so the plot will have to be regenerated.
            //
            ClearActors();

            retval = true;
        }
        else
        {
            char msg[100];
            SNPRINTF(msg, 100, "VisIt cannot remove a %s operator.",
                     operators[operatorIndex]->GetName());
            Error(msg);
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerPlot::RemoveLastOperator
//
//  Purpose:
//    Remove the last operator from the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//
//    Kathleen Bonnell, Tue Mar  4 15:24:19 PST 2003 
//    Update queryAtts before actors have been cleared. 
//
//    Brad Whitlock, Thu Apr 10 11:36:45 PDT 2003
//    Recoded how operators tell the client if they can be removed to get rid
//    of a bad comparison that used an actual operator name. I added code
//    to set the active operator index.
//
// ****************************************************************************

void
ViewerPlot::RemoveLastOperator()
{
    //
    // Return if there are no operators.
    //
    if (nOperators == 0)
    {
        return;
    }

    if (operators[nOperators-1]->Removeable())    
    {
        //
        // Remove the last operator.
        //
        nOperators--;
        delete operators[nOperators];
         
        //
        // If we deleted the active operator, decrement the active operator
        // index so it points to an operator that is still in the list.
        //
        if(activeOperatorIndex == nOperators)
            --activeOperatorIndex;

        if (queryAtts != 0)
        {
            queryAtts->SetChangeType(PlotQueryInfo::RemoveLast);
            queryAtts->Notify();
        }

        //
        // Clear the actor cache.
        //
        ClearActors();
    }
    else
    {
        char msg[100];
        SNPRINTF(msg, 100, "VisIt cannot remove a %s operator.",
                 operators[nOperators-1]->GetName());
        Error(msg);
    }
}

// ****************************************************************************
//  Method: ViewerPlot::RemoveAllOperators
//
//  Purpose:
//    Remove all the operators from the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//
//    Kathleen Bonnell, Tue Mar  4 15:24:19 PST 2003 
//    Update queryAtts before the actors are cleared.
//
//    Brad Whitlock, Thu Apr 10 11:36:45 PDT 2003
//    Recoded how operators tell the client if they can be removed to get rid
//    of a bad comparison that used an actual operator name.
//
// ****************************************************************************

void
ViewerPlot::RemoveAllOperators()
{
    //
    // Return if there are no operators.
    //
    if (nOperators == 0)
    {
        return;
    }

    if (operators[nOperators-1]->Removeable())    
    {
        //
        // Remove all the operators.
        //
        for (int i = 0; i < nOperators; i++)
        {
            delete operators[i];
        }
        nOperators = 0;
        activeOperatorIndex = -1;

        if (queryAtts != 0)
        {
            queryAtts->SetChangeType(PlotQueryInfo::RemoveAll);
            queryAtts->Notify();
        }

        //
        // Clear the actor cache.
        //
        ClearActors();
    }
    else
    {
        char msg[100];
        SNPRINTF(msg, 100, "VisIt cannot remove a %s operator.",
                 operators[nOperators-1]->GetName());
        Error(msg);
    }
}

// ****************************************************************************
//  Method: ViewerPlot::SetOperatorAttsFromClient
//
//  Purpose:
//    Set the attributes of any operators that match the type.
//
//  Arguments:
//    type       : The type of the operator to set the attributes for.
//
//  Programmer: Eric Brugger
//  Creation:   September 20, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:19:13 PDT 2001
//    Removed reference to OperType.
//
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//
//    Kathleen Bonnell, Tue Mar  4 15:24:19 PST 2003 
//    Update queryAtts before the actors are cleared.
//
//    Brad Whitlock, Fri Apr 11 10:00:22 PDT 2003
//    I changed the routine so only the active operator gets set if the
//    plot is expanded.
//
//    Jeremy Meredith, Wed May 21 13:06:28 PDT 2003
//    Made it check to see if the operator needs recalculation before
//    reporting that things have changed.
//
//    Brad Whitlock, Fri Mar 12 15:23:08 PST 2004
//    I changed the code so it sets the active operator's attributes if the
//    active operator is the same type as the operator whose attributes we're
//    setting. I also made it work if there is only one operator of the type
//    that we're setting. If there is more than one operator of the type
//    that we're setting and it is not the active operator a warning message
//    is now issued.
//
// ****************************************************************************

void
ViewerPlot::SetOperatorAttsFromClient(const int type)
{
    //
    // If there are no operators return.
    //
    if (nOperators == 0)
    {
        return;
    }

    //
    // Determine how many operators match the operator type and also check
    // to see if the active operator has the same type.
    //
    int nInstances = 0;
    int firstIndex = -1;
    bool activeOperatorMatchesType = false;
    for(int i = 0; i < nOperators; ++i)
    {
        bool typeMatches = (operators[i]->GetType() == type);
        nInstances += (typeMatches ? 1 : 0);
        if(typeMatches && firstIndex == -1)
            firstIndex = i;
        if(activeOperatorIndex == i)
            activeOperatorMatchesType = typeMatches;
    }

    //
    // Change the attributes of any operators that match the type.
    //
    bool changed = false;
    if(activeOperatorMatchesType)
    {
        // The active operator was of the type that we're trying to
        // set so set the operator attributes for the active operator.
        operators[activeOperatorIndex]->SetOperatorAttsFromClient();
        changed |= operators[activeOperatorIndex]->NeedsRecalculation();
    }
    else if(nInstances > 1)
    {
        // The active operator was not of the type that we're trying
        // to set AND there is more than one operator of that type.
        // Warn the user to select one of the operators and don't set
        // anything rather than clobber the operator settings.
        char msg[400];
        const char *oName = operators[firstIndex]->GetName();
        SNPRINTF(msg, 400, "You have more than one %s operator applied to a "
            "plot but none of the %s operators is the active operator. "
            "Please make one of the %s operators be the active operator by "
            "expanding the plot and clicking one of its %s operators so "
            "VisIt will apply the operator settings to the correct %s "
            "operator.", oName, oName, oName, oName, oName);
        Warning(msg);
    }
    else if(firstIndex != -1)
    {
        // There's only 1 instance of the operator that we're trying
        // to set and it is not the active operator. Oh well. Just set
        // the operator attributes anyway.
        operators[firstIndex]->SetOperatorAttsFromClient();
        changed |= operators[firstIndex]->NeedsRecalculation();
    }

    //
    // Clear the actor cache if any operator attributes where changed.
    //
    if (changed)
    {
        if (queryAtts != 0)
        {
            queryAtts->SetChangeType(PlotQueryInfo::OpAtts);
            queryAtts->Notify();
        }
        ClearActors();
    }
}

// ****************************************************************************
//  Method: ViewerPlot::GetNOperators
//
//  Purpose:
//    Return the number of operators associatied with the plot.
//
//  Returns:    The number of operators associated with the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
// ****************************************************************************

int
ViewerPlot::GetNOperators() const
{
    return nOperators;
}

// ****************************************************************************
//  Method: ViewerPlot::GetOperator
//
//  Purpose:
//    Return a pointer to the specified operator associated with the plot.
//
//  Arguments:
//    i         A 0-origin index indicating the operator to return.
//
//  Returns:    A pointer to the specified operator associated with the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
// ****************************************************************************

ViewerOperator *
ViewerPlot::GetOperator(const int i) const
{
    //
    // Check that the index is valid.
    //
    if (i < 0 || i >= nOperators)
    {
        return 0;
    }

    //
    // Return the operator.
    //
    return operators[i];
}

// ****************************************************************************
//  Method: ViewerPlot::SetActor
//
//  Purpose:
//    Set the actor for the plot at the specified frame.
//
//  Arguments:
//    actor     The actor to save for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Made use of debug stream.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Sat Jan 31 23:26:10 PST 2004
//    I removed the frame.
//
// ****************************************************************************

void
ViewerPlot::SetActor(const avtActor_p actor)
{
    actorList[cacheIndex] = actor;
}

// ****************************************************************************
//  Method: ViewerPlot::GetActor
//
//  Purpose:
//    Return the actor for the plot at the specified frame.
//
//  Arguments:
//    frame   The frame to return the actor for.
//
//  Returns:    The actor for the plot at the specified frame.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Nov 8 15:58:24 PST 2000
//    I made it return a reference.
//
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Made use of debug stream.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Sat Jan 31 22:06:30 PST 2004
//    Removed frame argument.
//
// ****************************************************************************

avtActor_p &
ViewerPlot::GetActor() const
{
    return actorList[cacheIndex];
}

// ****************************************************************************
//  Method: ViewerPlot::NoActorExists
//
//  Purpose: 
//    This is a convenience method that returns whether or not the specified
//    frame has an actor.
//
//  Returns:    True if there is no actor, False if there is an actor.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jul 25 14:34:13 PST 2002
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
// 
//    Brad Whitlock, Thu Jan 29 19:06:13 PST 2004
//    I removed the frame argument.
//
// ****************************************************************************

bool
ViewerPlot::NoActorExists() const
{
    return (*GetActor() == 0);
}

// ****************************************************************************
//  Method: ViewerPlot::GetReader
//
//  Purpose:
//    Return the reader for the plot at the current plot state.
//
//  Returns:    The reader for the plot at the current plot state.
//
//  Programmer: Jeremy Meredith
//  Creation:   June  5, 2001
//
//  Modifications:
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Made use of debug stream.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Thu Jan 29 19:10:17 PST 2004
//    I removed the frame argument.
//
// ****************************************************************************

avtDataObjectReader_p &
ViewerPlot::GetReader() const
{
    return readerList[cacheIndex];
}

// ****************************************************************************
//  Method: ViewerPlot::CreateActor
//
//  Purpose:
//    Create the actor for the plot for the specified frame.
//
//  Arguments:
//
//  Programmer: Eric Brugger
//  Creation:   March 9, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 19 18:00:41 PST 2001
//    Changed code so avtPlot::SetAtts is called before its Execute method.
//    This fixes a problem that causes the surface plot to crash the viewer.
//
//    Brad Whitlock, Tue Apr 24 13:38:25 PST 2001
//    Added code to handle NoInputException.
//
//    Kathleen Bonnell, Wed Apr 25 14:28:22 PDT 2001 
//    Added code to handle InvalidLimitsException.
//
//    Brad Whitlock, Fri Apr 27 14:28:05 PST 2001
//    Moved some exception handling code to prevent a memory error.
//
//    Kathleen Bonnell, Wed May  2 14:18:47 PDT 2001 
//    Added try-catch to handle exceptions on engine-portion of this
//    method.  Added another catch to viewer portion to handle
//    general VisItExceptions. 
//
//    Jeremy Meredith, Tue Jun  5 20:31:38 PDT 2001
//    Added code to reuse an existing plot if is already exists and does
//    not need recalculation on the engine.
//
//    Jeremy Meredith, Tue Jul  3 15:05:08 PDT 2001
//    Added handling of execution interruption (Abort).
//
//    Hank Childs, Mon Aug 20 14:59:02 PDT 2001
//    Incorporated SIL restrictions into deciding whether or not a plot needed
//    to be recalculated.
//
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Eric Brugger, Thu Sep  6 14:16:43 PDT 2001
//    I modified the routine to only set the plot attributes and variable
//    name for the avt plot when creating the avt plot.  I also modified
//    the routine to only execute the plot if the recalculate flag is set.
//
//    Eric Brugger, Fri Sep  7 12:48:45 PDT 2001
//    I modified the routine so that it always regenerates the plot.  The
//    routine no longer gets called if a plot doesn't need recalculation.
//
//    Kathleen Bonnell, Wed Sep 26 15:09:58 PDT 2001 
//    Added calls to plot's SetBackgroundColor and SetForegroundColor.  
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
//
//    Hank Childs, Tue Nov 20 14:50:32 PST 2001
//    Register a network index with the plot.
//
//    Brad Whitlock, Thu Jul 25 16:57:57 PST 2002
//    I made it rethrow AbortException.
//
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Hank Childs, Wed Sep 17 09:31:59 PDT 2003
//    If the meta-data varies over time, don't re-use the SIL.
//
//    Mark C. Miller, Wed Oct 29 15:38:31 PST 2003
//    Added optional bool 'createNew' (which is true by default) which
//    controls how the data object reader is obtained from the viewer
//    engine manager. Also added bool for whether switching into or out
//    of scalable rendering.
//
//    Mark C. Miller, Tue Nov 11 12:27:33 PST 2003
//    Added code to temporarily disable external render requets in all windows
//
//    Hank Childs, Sun Nov 16 13:32:12 PST 2003
//    Tell UI processes when the SIL has changed out from underneath it.
//
//    Brad Whitlock, Sat Jan 31 22:23:26 PST 2004
//    I removed the frame argument. I also changed the code so it is more
//    responsible about when it tells other routines the plot state.
//
//    Mark C. Miller, Thu Mar 18 21:12:45 PST 2004
//    Added code to check newsilr for NULL and throw exception instead of
//    possibly hitting a SEGV
//
//    Mark C. Miller, Wed Mar 24 19:02:35 PST 2004
//    Moved declaration for 'invariantMetaData' outside of TRY block
//
//    Eric Brugger, Tue Mar 30 15:26:33 PST 2004
//    Added code to set the plot data extents if maintain data limits is set.
//
//    Mark C. Miller, Mon Apr 19 12:00:52 PDT 2004
//    Added code to issue a warning message if actor has no data
//
//    Mark C. Miller, Mon Apr 19 16:17:37 PDT 2004
//    Added returned bool argument, actorHasNoData and removed code that 
//    issues the no data warning message to the caller of this method
//
//    Mark C. Miller, Wed Jul  7 11:42:09 PDT 2004
//    Added call to re-enable external render request status of all windows
//    at all early returns points.
//
//    Brad Whitlock, Thu Jul 22 14:41:22 PST 2004
//    Added code to set the actor's units if applicable.
//
//    Kathleen Bonnell, Tue Nov  2 11:13:15 PST 2004 
//    Added call to set the avtPlot's mesh type. 
// 
//    Kathleen Bonnell, Wed Nov  3 16:51:24 PST 2004 
//    Removed call to set the avtPlot's mesh type. Added test for valid
//    active variable before attempting to retrieve Units, to prevent 
//    unnecessary Exception handling.
// 
//    Kathleen Bonnell, Fri Jan  7 13:00:32 PST 2005 
//    Removed unnecessary TRY-CATCH block surrounding retrieval of units. 
//
// ****************************************************************************

void
ViewerPlot::CreateActor(bool createNew,
                        bool turningOffScalableRendering,
                        bool& actorHasNoData)
{
    avtDataObjectReader_p reader;

    std::vector<bool> oldAble;
    ViewerWindowManager::Instance()->DisableExternalRenderRequestsAllWindows(oldAble);

    bool invariantMetaData = true;

    // Get a data reader.
    TRY
    {
        //
        // Determine if the file has time varying metadata.
        //
        ViewerFileServer *server = ViewerFileServer::Instance();
        invariantMetaData = server->MetaDataIsInvariant(GetHostName(),
            GetDatabaseName(), GetState());

        // The following code is necessary to support time-varying SILs
        if (!invariantMetaData)
        {
            if (viewerPlotList == NULL)
                EXCEPTION0(ImproperUseException);

            viewerPlotList->ClearDefaultSILRestrictions(GetHostName(),
                                                        GetDatabaseName());
            avtSILRestriction_p newsilr =
                viewerPlotList->GetDefaultSILRestriction(GetHostName(),
                                                         GetDatabaseName(),
                                                         GetVariableName(),
                                                         GetState());
            if (*newsilr == NULL)
                EXCEPTION0(ImproperUseException);

            newsilr->SetFromCompatibleRestriction(GetSILRestriction());
            SetSILRestriction(newsilr);

            // We have changed the SIL.  Make sure to tell the GUI or CLI
            // that it has changed out from underneath them.
            viewerPlotList->UpdateSILRestrictionAtts();
        }

        if (!createNew)
        {
            reader = ViewerEngineManager::Instance()->
                         UseDataObjectReader(this,turningOffScalableRendering);
        }
        else
        {
            reader = ViewerEngineManager::Instance()->
                         GetDataObjectReader(this);
        }
    }
    CATCH(AbortException)
    {
        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(nullActor);
        
        // re-instate correct external render request state on all windows
        ViewerWindowManager::Instance()->EnableExternalRenderRequestsAllWindows(oldAble);

        // Rethrow the exception.
        RETHROW;
    }
    CATCH2(VisItException, e)
    {
        char message[2048];
        if (e.GetExceptionType() == "InvalidLimitsException" ||
            e.GetExceptionType() == "InvalidDimensionsException")
        {
            //
            //  Create message for the gui that includes the plot name
            //  and message.
            //
            SNPRINTF(message, sizeof(message), "%s:  %s", 
                     viewerPluginInfo->GetName(),
                     e.Message().c_str());
        }
        else
        {
            //
            // Add as much information to the message as we can,
            // including plot name, exception type and exception 
            // message.
            // 
            SNPRINTF(message, sizeof(message), "%s:  (%s)\n%s", 
                     viewerPluginInfo->GetName(),
                     e.GetExceptionType().c_str(),
                     e.Message().c_str());
        }
     
        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(nullActor);

        // re-instate correct external render request state on all windows
        ViewerWindowManager::Instance()->EnableExternalRenderRequestsAllWindows(oldAble);

        CATCH_RETURN(1);

    }
    ENDTRY

    // If we do not have a data reader, send a message back to the GUI and get
    // out of this routine since it causes a memory error if we send a NULL
    // data reader through avtPlot.
    if(*reader == NULL)
    {
        // Create a message to send to the GUI.
        char message[500];
        SNPRINTF(message, 500, "The %s plot of \"%s\" for the file \"%s\" could "
                 "not be generated by the compute engine on host \"%s\".",
                 viewerPluginInfo->GetName(),
                 variableName.c_str(), databaseName.c_str(), hostName.c_str());
        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(nullActor);

        // re-instate correct external render request state on all windows
        ViewerWindowManager::Instance()->EnableExternalRenderRequestsAllWindows(oldAble);

        return;
    }

    // Save the reader.
    readerList[cacheIndex] = reader;

    // Get the keyframed attributes for the current state.  The data
    // extents must be set before the attributes.
    plotList[cacheIndex] = viewerPluginInfo->AllocAvtPlot();
    plotAtts->GetAtts(cacheIndex, curPlotAtts);

    if (viewerPlotList->GetMaintainDataMode())
    {
        plotList[cacheIndex]->SetDataExtents(dataExtents);
    }

    if (!invariantMetaData)
    {
        if (viewerPlotList == NULL)
            EXCEPTION0(ImproperUseException);

        if (!viewerPlotList->GetKeyframeMode())
        {
            if (plotList[cacheIndex]->AttributesDependOnDatabaseMetaData())
            {
                viewerPluginInfo->ReInitializePlotAtts(curPlotAtts,
                    GetMetaData(), variableName.c_str());
                viewerPluginInfo->SetClientAtts(curPlotAtts);

                // ok, now set the plotAtts map, being careful not to introduce
                // new keyframes in the process. So, we use the 'Le' method
                plotAtts->SetAttsLe(cacheIndex, curPlotAtts);
            }
        }
    }

    plotList[cacheIndex]->SetAtts(curPlotAtts);
    plotList[cacheIndex]->SetVarName(variableName.c_str());
    plotList[cacheIndex]->SetBackgroundColor(bgColor);
    plotList[cacheIndex]->SetForegroundColor(fgColor);
    plotList[cacheIndex]->SetIndex(networkID);
    plotList[cacheIndex]->SetCurrentSILRestriction(silr);

    // assume the actor has data
    actorHasNoData = false;
    TRY
    {
        avtActor_p actor = plotList[cacheIndex]->Execute(reader);
        bool countPolysOnly = false;
        if (actor->GetDataObject()->GetNumberOfCells(countPolysOnly) == 0)
            actorHasNoData = true;

        //
        // Set the actor's units from the data attributes.
        //
        if (actor->GetDataObject()->GetInfo().GetAttributes().ValidActiveVariable())
        {
            std::string units(actor->GetDataObject()->GetInfo().GetAttributes().
            GetVariableUnits());
            if(units != "")
                plotList[cacheIndex]->SetVarUnits(units.c_str());
            else
                plotList[cacheIndex]->SetVarUnits(0);
        }
        else
        {
            plotList[cacheIndex]->SetVarUnits(0);
        }

        this->SetActor(actor);

        // Indicate that this plot has no error.
        this->errorFlag = false;
    }
    CATCH2(InvalidLimitsException, e)
    {
        // Create a message to send to the GUI,
        // Stating plot name and message. 
        char message[2048];
        SNPRINTF(message, sizeof(message), "%s:  %s", 
                 viewerPluginInfo->GetName(),
                 e.Message().c_str());
        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(nullActor);
    }
    CATCH2(VisItException, e)
    {
        // Create a message to send to the GUI, with as
        // much info as we can, including plot name,
        // exception type and exception message. 
        char message[2048];
        SNPRINTF(message, sizeof(message), "%s (%s):  %s", 
                 viewerPluginInfo->GetName(),
                 e.GetExceptionType().c_str(),
                 e.Message().c_str());

        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(nullActor);
    }
    ENDTRY

    // re-instate correct external render request state on all windows
    ViewerWindowManager::Instance()->EnableExternalRenderRequestsAllWindows(oldAble);
}

// ****************************************************************************
//  Method: ViewerPlot::ClearActors
//
//  Purpose:
//    Clear all the actors associated with the plot.
//
//  Programmer: Eric Brugger
//  Creation:   September 6, 2000
//
//  Modifications:
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Eric Brugger, Fri Sep  7 12:48:45 PDT 2001
//    I had the routine also clear the associated data reader.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Mon Apr 5 09:39:22 PDT 2004
//    I changed how we iterate over the cache.
//
// ****************************************************************************

void
ViewerPlot::ClearActors()
{
    //
    // Delete all the actors.
    //
    for (int i = 0; i < cacheSize; i++)
    {
        debug5 << GetPlotName() << ": Clearing actor at state " << i << endl;
        plotList[i]   = (avtPlot *)0;
        actorList[i]  = (avtActor *)0;
        readerList[i] = (avtDataObjectReader *)0;
    }
}

// ****************************************************************************
// Method: ViewerPlot::ClearCurrentActor
//
// Purpose: 
//   Clears the current actor, plot, and reader.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jan 31 23:20:57 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::ClearCurrentActor()
{
    plotList[cacheIndex]   = (avtPlot *)0;
    actorList[cacheIndex]  = (avtActor *)0;
    readerList[cacheIndex] = (avtDataObjectReader *)0;
}

// ****************************************************************************
//  Method: ViewerPlot::ClearActors
//
//  Purpose:
//    Clear all the actors associated with the plot over the specified
//    range.
//
//  Arguments:
//    i0        The first index in the range (inclusive).
//    i1        The last index in the range (inclusive).
//
//  Programmer: Eric Brugger
//  Creation:   September 6, 2000
//
//  Modifications:
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Made use of debug stream.
//
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Eric Brugger, Fri Sep  7 12:48:45 PDT 2001
//    I had the routine also clear the associated data reader.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Mon Apr 5 09:45:23 PDT 2004
//    I changed how we handle the range.
//
// ****************************************************************************

void
ViewerPlot::ClearActors(const int i0, const int i1)
{
    //
    // Check that the range is valid.
    //
    if(i0 < 0 || i1 >= cacheSize || i1 < i0)
    {
        debug1 << "Invalid cache index range." << endl;
        return;
    }

    //
    // Delete the actors within the range.
    //
    for (int i = i0; i <= i1; ++i)
    {
        plotList[i]   = (avtPlot *)0;
        actorList[i]  = (avtActor *)0;
        readerList[i] = (avtDataObjectReader *)0;
    }
}

// ****************************************************************************
//  Method: ViewerPlot::TransmuteActor
//
//  Purpose: This method supports smooth transitions into and out of scalable
//  rendering. When switching into scalable rendering, we clear all the actors
//  and then re-acquire each one again from the engine but setting the 
//  'respondWithNullData' flag to true. When swithcing out of scalable
//  rendering, we clear all the actors and then re-acquire each one again
//  but setting 'respondWithNullData' to false. In either case, it is assumed
//  the associated networks already exist in cache on the engine.
//  
//  Programmer: Mark C. Miller
//  Creation:   October 29, 2003 
//
//  Modifications:
//    Brad Whitlock, Sat Jan 31 22:22:26 PST 2004
//    I removed the frame argument.
//
// ****************************************************************************

void
ViewerPlot::TransmuteActor(bool turningOffScalableRendering)
{
    bool dummyBool;
    CreateActor(false, turningOffScalableRendering, dummyBool);
} 


// ****************************************************************************
//  Method: ViewerPlot::GetBlockOrigin
//
//  Purpose:
//    Get the block origin for the plot.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 29, 2004 
//
//  Modifications:
//
// ****************************************************************************

int
ViewerPlot::GetBlockOrigin() const
{
    int retval = 0;

    if(readerList != NULL)
    {
        if(*(readerList[cacheIndex]) != NULL)
        {
            avtDataAttributes &atts = readerList[cacheIndex]->
                GetInfo().GetAttributes();
            retval = atts.GetBlockOrigin();
        }
    }

    return retval;
}


// ****************************************************************************
//  Method: ViewerPlot::GetSpatialDimension
//
//  Purpose:
//    Get the spatial dimension for the plot at the specified frame.
//
//  Arguments:
//    frame     The frame to get the dimension for.
//
//  Programmer: Eric Brugger
//  Creation:   April 24, 2001
//
//  Modifications:
//    Hank Childs, Tue Sep  4 16:14:49 PDT 2001
//    Reflect new interface for avtDataAttributes.
//
//    Brad Whitlock, Thu Jul 18 12:08:00 PDT 2002
//    Make it return zero if the reader list is NULL.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Sat Jan 31 23:32:18 PST 2004
//    I removed the frame argument.
//
// ****************************************************************************

int
ViewerPlot::GetSpatialDimension() const
{
    int retval = 0;

    if(readerList != NULL)
    {
        if(*(readerList[cacheIndex]) != NULL)
        {
            avtDataAttributes &atts = readerList[cacheIndex]->
                GetInfo().GetAttributes();
            retval = atts.GetSpatialDimension();
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerPlot::GetSpatialExtents
//
//  Purpose:
//    Get the spatial extents for the plot at the specified frame.
//
//  Arguments:
//    frame     The frame to get the extents for.
//    extsType  The flavor of spatial extents to get.
//
//  Notes:      Calling function must free return value.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2000
//
//  Modifications:
//    Hank Childs, Sun Mar 25 13:01:19 PST 2001
//    Added logic to try and get the current extents and to get them from
//    the input if they don't exist.
//
//    Brad Whitlock, Thu May 10 13:09:23 PST 2001
//    Fixed the code that returns the extents to prevent a crash.
//
//    Hank Childs, Tue Sep  4 17:08:57 PDT 2001
//    Made calling function responsible for freeing memory.
//
//    Brad Whitlock, Mon Mar 4 11:08:55 PDT 2002
//    Added check to see if reader exists.
//
//    Hank Childs, Fri Mar 15 19:42:03 PST 2002
//    Account for dataset examiner.
//
//    Kathleen Bonnell, Thu May 16 10:13:56 PDT 2002 
//    Use extentSize for size of buffer (instead of 2*extentSize). 
//
//    Hank Childs, Mon Jul 15 15:58:39 PDT 2002
//    Allow for different flavors of spatial extents.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Sat Jan 31 23:33:10 PST 2004
//    I removed the frame argument.
//
// ****************************************************************************

double *
ViewerPlot::GetSpatialExtents(avtExtentType extsType) const
{
    //
    // If extsType is sent in as AVT_UNKNOWN_EXTENT_TYPE, then that is a signal
    // that we should be using our own data member instead.
    //
    avtExtentType realExtsType = (extsType == AVT_UNKNOWN_EXTENT_TYPE 
                                  ? spatialExtentsType : extsType);
   
    // Return early if the reader does not exist.
    if(*(readerList[cacheIndex]) == 0)
        return 0;

    // Populate some local variables.
    avtDataAttributes &atts = readerList[cacheIndex]->
        GetInfo().GetAttributes();
    int dim = atts.GetSpatialDimension();
    int extentSize = ((dim * 2) < 6) ? 6 : (dim * 2);
    double *buffer = new double[extentSize];

    if (realExtsType == AVT_ORIGINAL_EXTENTS)
    {
        if (atts.GetSpatialExtents(buffer))
        {
            return buffer;
        }
    }
    else if (realExtsType == AVT_ACTUAL_EXTENTS)
    {
        if (atts.GetCurrentSpatialExtents(buffer))
        {
            return buffer;
        }
    }

    if (! readerList[cacheIndex]->InputIsDataset())
    {
        //
        // This means that the input is an image, but the extents didn't get
        // set for the image.  This is a serious problem.  Fake it the best
        // we can and log it, since it should never occur.
        //
        debug1 << "Image sent across the network without setting extents"
               << endl;

        for (int i = 0 ; i < dim ; i++)
        {
            buffer[2*i] = 0.;
            buffer[2*i+1] = 1.;
        }
        return buffer;
    }

    avtDataset_p ds = readerList[cacheIndex]->GetDatasetOutput();

    //
    // The dataset will have the mesh limits.
    //
    avtDatasetExaminer::GetSpatialExtents(ds, buffer);

    return buffer;
}

// ****************************************************************************
//  Method: ViewerPlot::SetSpatialExtentsType
//
//  Purpose:
//      Tells the view plot what type of spatial extents we should be using in
//      the future.
//
//  Arguments:
//    extsType  The flavor of spatial extents to get.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2002
//
// ****************************************************************************

void
ViewerPlot::SetSpatialExtentsType(avtExtentType extsType)
{
    spatialExtentsType = extsType;
}

// ****************************************************************************
//  Method: ViewerPlot::ExecuteEngineRPC
//
//  Purpose:
//    Execute the engine RPC to create the plot.
//
//  Arguments:
//    frame     The frame number.
//
//  Programmer: Eric Brugger
//  Creation:   March 9, 2001
//
//  Modifications:
//    Kathleen Bonnell, Wed May  2 15:16:16 PDT 2001
//    Added try-catch block so that any VisItExceptions 
//    could be rethrown.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Brad Whitlock, Mon Oct 22 18:51:19 PST 2001
//    Removed the exception handling code since it was completely pointless.
//
//    Jeremy Meredith, Wed Nov  7 10:29:20 PST 2001
//    Made it set the network ID from the result of MakePlot.
//
//    Brad Whitlock, Fri Feb 22 15:03:50 PST 2002
//    Rewrote the method so it goes through the engine manager.
//
//    Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002
//    Added call to ReleaseData.
//
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Fri Mar 26 14:33:36 PST 2004
//    Made it use strings.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map this plot to the engine used to create it.
//
//    Eric Brugger, Tue Mar 30 15:26:33 PST 2004
//    Added the data extents to the call to MakePlot.
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added WindowID to calls to MakePlot
//
// ****************************************************************************

bool
ViewerPlot::ExecuteEngineRPC()
{
    //
    //  Release data on previous network.
    //
    if (networkID != -1)
        ViewerEngineManager::Instance()->ReleaseData(engineKey,
                                                     networkID);

    ViewerEngineManager *engineMgr = ViewerEngineManager::Instance();
    plotAtts->GetAtts(cacheIndex, curPlotAtts);
    bool successful;
    if (viewerPlotList->GetMaintainDataMode())
    {
        successful = engineMgr->MakePlot(engineKey, viewerPluginInfo->GetID(),
            curPlotAtts, dataExtents, GetWindowId(), &networkID);
    }
    else
    {
        successful = engineMgr->MakePlot(engineKey, viewerPluginInfo->GetID(),
            curPlotAtts, nullDataExtents, GetWindowId(), &networkID);
    }

    if(!successful)
    {
        networkID = -1;
        errorFlag = true;
    }

    return successful;
}

// ****************************************************************************
//  Method: ViewerPlot::GetType
//
//  Purpose:
//    Return the type of the plot.
//
//  Returns:    The type of the plot.
//
//  Programmer: Eric Brugger
//  Creation:   March 9, 2001
//
// ****************************************************************************

int
ViewerPlot::GetType() const
{
    return type;
}

// ****************************************************************************
//  Method: ViewerPlot::SetClientAttsFromPlot
//
//  Purpose:
//    Set the client attributes based on the plot attributes.
//
//  Programmer: Eric Brugger
//  Creation:   March 9, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Sun Feb 1 00:34:15 PDT 2004
//    I removed the frame argument.
//
// ****************************************************************************

void
ViewerPlot::SetClientAttsFromPlot()
{
    //
    // Set the client attributes.
    //
    plotAtts->GetAtts(cacheIndex, curPlotAtts);
    viewerPluginInfo->SetClientAtts(curPlotAtts);
}

// ****************************************************************************
//  Method: ViewerPlot::SetPlotAttsFromClient
//
//  Purpose:
//    Set the plot attributes for the entire plot based on the client
//    attributes.
//
//  Programmer: Eric Brugger
//  Creation:   December 27, 2002
//
// ****************************************************************************

void
ViewerPlot::SetPlotAttsFromClient()
{
    viewerPluginInfo->GetClientAtts(curPlotAtts);
    SetPlotAtts(curPlotAtts);
}

// ****************************************************************************
// Method: ViewerPlot::SetPlotAtts
//
// Purpose: 
//   Tries to set the attributes for the entire plot. This only happens
//   if they are compatible types.
//
// Arguments:
//   atts  : The new attributes.
//
// Programmer: Eric Brugger
// Creation:   December 27, 2002
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 14:05:33 PST 2004
//   Changed the code to use the new cache indexing.
//
//   Brad Whitlock, Fri Apr 16 14:07:48 PST 2004
//   I fixed a bug that happened when trying to set the atts on the first
//   frame of a keyframe animation.
//
// ****************************************************************************

bool
ViewerPlot::SetPlotAtts(const AttributeSubject *atts)
{
    //
    // Copy the attributes to the curPlotAtts unless this routine is
    // called internally with curPlotAtts.
    //
    if (atts != curPlotAtts)
    {
        if (!curPlotAtts->CopyAttributes(atts))
        { 
            return false;
        }
    }

    if(viewerPlotList->GetKeyframeMode())
    {
        //
        // Set the plot attributes for the current plot state.  SetAtts
        // returns the range of plots that were invalidated.  The
        // maximum value is clamped to frame1 since SetAtts may return
        // INT_MAX to indicate the end of the plot.
        //
        int i0, i1;
        plotAtts->SetAtts(cacheIndex, curPlotAtts, i0, i1);
        i1 = (i1 <= cacheIndex) ? i1 : cacheIndex;

        // Invalidate the cache if necessary for items i0..i1
        CheckCache(i0, i1, false);
    }
    else
    {
        //
        // Set the plot attributes for the entire plot.
        //
        plotAtts->SetAtts(curPlotAtts);

        // Invalidate the cache if necessary for all items.
        CheckCache(0, cacheSize-1, false);
    }

    return true;
}

// ****************************************************************************
// Method: ViewerOperator::GetPlotAtts
//
// Purpose: 
//   Returns a const pointer to the current plot attributes.
//
// Returns:    A const pointer to the plot attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 8 09:42:52 PDT 2002
//
// Modifications:
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Mon Apr 5 12:21:34 PDT 2004
//    I made it return the current plot attributes like it originally did.
// ****************************************************************************

const AttributeSubject *
ViewerPlot::GetPlotAtts() const
{
    return curPlotAtts;
}

// ****************************************************************************
//  Method: ViewerPlot::UpdateColorTable
//
//  Purpose: 
//    Passes the color table name along to the avtplots. If the plot can
//    use a color table and it uses the one by the name of ctName, its
//    colors will be updated and the plot will be redrawn.
//
//  Arguments:
//    ctName    The name of the color table to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Jun 14 16:40:00 PST 2001
//
//  Modifications:
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    Modified the routine to set the color table for all the plots.
//   
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
//    Brad Whitlock, Mon Jul 14 13:50:54 PST 2003
//    I made it catch InvalidColortableException.
//
// ****************************************************************************

bool
ViewerPlot::UpdateColorTable(const char *ctName)
{
    //
    // Set the color table for any existing plots.
    //
    bool      retval = false;
    int       i;

    for (i = 0; i < cacheSize; i++)
    {
        if (*plotList[i] != NULL)
        {
            TRY
            {
                retval |= (*plotList[i])->SetColorTable(ctName);
            }
            CATCH(InvalidColortableException)
            {
            }
            ENDTRY
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerPlot::SetBackgroundColor
//
//  Purpose: 
//    Passes the background color along to the avtplots. If the plot can
//    use the background color, the plot will be redrawn.
//
//  Arguments:
//    bg        The background color. 
//
//  Returns:    True if any of the existing plots need to be redrawn.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 25, 2001 
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//   
// ****************************************************************************

bool
ViewerPlot::SetBackgroundColor(const double *bg)
{
    bool retval = false;
    bgColor[0]  = bg[0];
    bgColor[1]  = bg[1];
    bgColor[2]  = bg[2];
    for (int i = 0; i < cacheSize; i++)
    {
        if (*plotList[i] != NULL)
        {
            retval |= (*plotList[i])->SetBackgroundColor(bgColor);
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerPlot::SetForegroundColor
//
//  Purpose: 
//    Passes the foreground color along to the avtplots. If the plot can
//    use the foreground color, the plot will be redrawn.
//
//  Arguments:
//    fg        The foreground color. 
//
//  Returns:    True if any of the existing plots need to be redrawn.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 25, 2001 
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//   
// ****************************************************************************

bool
ViewerPlot::SetForegroundColor(const double *fg)
{
    bool retval = false;
    fgColor[0]  = fg[0];
    fgColor[1]  = fg[1];
    fgColor[2]  = fg[2];
    for (int i = 0; i < cacheSize; i++)
    {
        if (*plotList[i] != NULL)
        {
            retval |= (*plotList[i])->SetForegroundColor(fgColor);
        }
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerPlot::HandleTool
//
// Purpose: 
//   Sets the attributes for the entire plot from the tool.
//
// Arguments:
//   ti : A reference to the tool interface.
//
// Programmer: Eric Brugger
// Creation:   December 27, 2002
//
// Modifications:
//   Brad Whitlock, Thu Apr 17 09:13:13 PDT 2003
//   I changed the code so it uses the active operator. I also changed it so
//   it will not attempt to give tool attributes to operators if they've
//   already been given to the plot.
//
// ****************************************************************************

bool
ViewerPlot::HandleTool(const avtToolInterface &ti)
{
    //
    // Set the plot attributes from the tool attributes.
    //
    bool val = SetPlotAtts(ti.GetAttributes());

    //
    // Set the operator attributes from the tool attributes.
    //
    for(int i = 0; i < nOperators && !val; ++i)
    {
        if(!expandedFlag || (i == activeOperatorIndex))
            val |= operators[i]->SetOperatorAtts(ti.GetAttributes());
    }

    return val;
}

// ****************************************************************************
// Method: ViewerPlot::InitializeTool
//
// Purpose: 
//   Initializes the tool with all of the operator attributes.
//
// Arguments:
//   ti : The tool interface that we're initializing.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 11 14:20:46 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Oct 8 15:34:06 PST 2002
//   I made the plot initialize the tool too.
//
//   Brad Whitlock, Thu Apr 17 08:55:25 PDT 2003
//   I changed the code to take the active operator into account.
//
// ****************************************************************************

bool
ViewerPlot::InitializeTool(avtToolInterface &ti)
{
    bool retval = false;

    //
    // Try and convert the plot attributes object into a type that
    // is compatible with the tool. Note that we must do it like this
    // so the generic tool attributes do not need to know about the
    // operator attributes. This is important because the plot
    // attributes exist only in plugins.
    //
    std::string tname(ti.GetAttributes()->TypeName());
    AttributeSubject *atts = plotAtts->CreateCompatible(tname);
    if(atts != 0)
    {
        retval |= ti.GetAttributes()->CopyAttributes(atts);
        delete atts;
    }

    for(int i = 0; i < nOperators; ++i)
    {
        //
        // Try and convert the operator attributes object into a type that
        // is compatible with the tool. Note that we must do it like this
        // so the generic tool attributes do not need to know about the
        // operator attributes. This is important because the operator
        // attributes exist only in plugins.
        //
        const AttributeSubject *operatorAtts = operators[i]->GetOperatorAtts();
        AttributeSubject *atts = operatorAtts->CreateCompatible(tname);

        if(atts != 0)
        {
            // If the plot is expanded, only attempt to use the 
            if(expandedFlag)
            {
                if(!retval || (i == activeOperatorIndex))
                    retval |= ti.GetAttributes()->CopyAttributes(atts);
            }
            else if(!retval)
            {
                retval |= ti.GetAttributes()->CopyAttributes(atts);
            }

            delete atts;
        }
    }

    return retval;
}

// ****************************************************************************
//  Method:  ViewerPlot::GetNetworkID
//
//  Purpose:
//    Returns the network ID of the current plot.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  6, 2001
//
// ****************************************************************************

int
ViewerPlot::GetNetworkID() const
{
    return networkID;
}

// ****************************************************************************
//  Method:  ViewerPlot::SetNetworkID
//
//  Purpose:
//    Sets the network ID of the current plot.
//
//  Programmer:  Brad Whitlock
//  Creation:    Fri Feb 22 16:19:54 PST 2002
//
// ****************************************************************************

void
ViewerPlot::SetNetworkID(int id)
{
    networkID = id;
}

// ****************************************************************************
// Method: ViewerPlot::SetActiveOperatorIndex
//
// Purpose: 
//   Sets the active operator index.
//
// Arguments:
//   index : The index of the new active operator.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 09:52:53 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::SetActiveOperatorIndex(int index)
{
    if(nOperators > 0 && index >= 0 && index < nOperators)
        activeOperatorIndex = index;
    else if(nOperators > 0)
        activeOperatorIndex = 0;
    else
        activeOperatorIndex = -1;
}

// ****************************************************************************
// Method: ViewerPlot::GetActiveOperatorIndex
//
// Purpose: 
//   Returns the active operator index.
//
// Returns:    The active operator index.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 09:53:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlot::GetActiveOperatorIndex() const
{
    return activeOperatorIndex;
}

// ****************************************************************************
// Method: ViewerPlot::SetExpanded
//
// Purpose: 
//   Sets the plot's expanded flag.
//
// Arguments:
//   val : The plot's new expanded flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 09:54:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::SetExpanded(bool val)
{
    expandedFlag = val;
}

// ****************************************************************************
// Method: ViewerPlot::GetExpanded
//
// Purpose: 
//   Returns the plot's expanded flag.
//
// Returns:    The plot's expanded flag.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 09:54:45 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::GetExpanded() const
{
    return expandedFlag;
}

// ****************************************************************************
//  Method:  ViewerPlot::StartPick
//
//  Purpose:
//    Tells the engine to start pick mode.  If points were transformed,
//    re-execute the pipeline by re-creating the actors.
//
//  Returns:     True if the pipeline needed to be re-exeucted false otherwise.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November  15, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Feb 22 16:26:43 PST 2002
//    Made the engine RPC go through the engine manager.
//
//    Kathleen Bonnell, Tue Mar 26 10:15:00 PST 2002 
//    Pushed back the creation of new actors to ViewerPlotList. 
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//   
//    Kathleen Bonnell, Wed Mar 26 14:37:23 PST 2003  
//    GetTransformedPoints renamed RequiresReExecuteForQuery. 
//
//    Brad Whitlock, Sat Jan 31 22:45:06 PST 2004
//    I removed the frame argument and made it use strings.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map this plot to the engine used to create it.
//
//    Brad Whitlock, Mon Apr 5 14:07:54 PST 2004
//    Changed to use new indexing.
//
//    Kathleen Bonnell, Tue Jun  1 17:57:52 PDT 2004 
//    Added bool args. 
//
// ****************************************************************************

bool
ViewerPlot::StartPick(const bool needZones, const bool needInvTransform)
{
    bool needsUpdate = false;

    //
    //  Don't necessarily want to send the StartPick to the engine
    //  for each and every plot, but how else to manage this?  Really
    //  want to do this only if there are different engines for different
    //  plots.  But how to know from ViewerPlotList??
    //
    if (ViewerEngineManager::Instance()->StartPick(engineKey, needZones,
                                                   true, networkID))
    {
        if (IsInRange() && *plotList[cacheIndex] != NULL)
        {
            needsUpdate |= (*plotList[cacheIndex])->
                           RequiresReExecuteForQuery(needInvTransform, needZones);
        }
        if (needsUpdate)
        {
            ClearCurrentActor();
        }
    }
    else
    {
        debug1 << "An error occurred when starting the pick." << endl;
    }

    return needsUpdate;
}

// ****************************************************************************
//  Method:  ViewerPlot::StopPick
//
//  Purpose:
//    Tells the engine to stop pick mode.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November  15, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Feb 22 16:26:43 PST 2002
//    Made the engine RPC go through the engine manager.
//
//    Brad Whitlock, Fri Mar 26 14:35:45 PST 2004
//    Changed to use strings.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map this plot to the engine used to create it.
//
// ****************************************************************************

void
ViewerPlot::StopPick()
{
    //
    //  Don't necessarily want to send the StopPick to the engine
    //  for each and every plot, but how else to manage this?  Really
    //  want to do this only if there are different engines for different
    //  plots.  But how to know from ViewerPlotList??
    //
    if(!ViewerEngineManager::Instance()->StartPick(engineKey, false,
                                                   false, networkID))
    {
        debug1 << "An error occurred when stopping the pick." << endl;
    }
}

// ****************************************************************************
// Method: ViewerPlot::ResetNetworkIds
//
// Purpose: 
//   Resets the network ids for pick if the specified engine key matches the
//   plot's engine key.
//
// Arguments:
//   key : The engine key to compare against.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 3 14:23:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::ResetNetworkIds(const EngineKey &key)
{
    if(key == engineKey)
    {
        networkID = -1;
        clonedNetworkId = -1;
    }
}

// ****************************************************************************
//  Method:  ViewerPlot::GetPlotQueryInfo
//
//  Purpose:
//    Returns queryAtts that an observer may watch. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2002 
//
// ****************************************************************************

PlotQueryInfo *
ViewerPlot::GetPlotQueryInfo()
{
    //
    // If the client attributes haven't been allocated then do so.
    //
    if (queryAtts == 0)
    {
        queryAtts  = new PlotQueryInfo;
    }
    return queryAtts;
}

// ****************************************************************************
//  Method:  ViewerPlot::GetVarType
//
//  Purpose:
//    Returns the type of the variable.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 19, 2002 
//
//  Modifications:
//    Sean Ahern, Fri Jun 28 14:45:26 PDT 2002
//    Added support for expressions.
//
//    Brad Whitlock, Thu Jul 18 14:45:09 PST 2002
//    I moved a bunch of the expression code into ViewerExpressionList.
//
//    Sean Ahern, Thu Oct 17 17:22:52 PDT 2002
//    Changed the ViewerExpressionList interface slightly.
//
//    Sean Ahern, Wed Feb  5 14:34:38 PST 2003
//    Removed the ViewerExpressionList for the more general ParsingExprList.
//
//    Brad Whitlock, Mon Dec 8 15:23:08 PST 2003
//    Added code to handle a possible InvalidVariableException that can be
//    thrown out of md->DetermineVarType.
//
//    Brad Whitlock, Fri Mar 26 08:09:26 PDT 2004
//    Made it use ViewerFileServer::DetermineVarType.
//
//    Brad Whitlock, Mon Apr 5 12:32:51 PDT 2004
//    I made it use GetState to determine the database state.
//
// ****************************************************************************

avtVarType 
ViewerPlot::GetVarType() const
{
    return GetVarType(variableName);
}

avtVarType 
ViewerPlot::GetVarType(const std::string &var) const
{
    return ViewerFileServer::Instance()->DetermineVarType(hostName,
        databaseName, var, GetState());
}

// ****************************************************************************
//  Method:  ViewerPlot::CheckCache
//
//  Purpose:
//    Check the cache between the specified frames and clear any frames
//    that need recalculation.
//
//  Arguments:
//    i0         The first inclusive, plot index to check.
//    i1         The last inclusive, plot index to check.
//    force      Flag indicating if the cache should be cleared
//               unconditionally, regardless of the whether setting
//               the plot attributes requires it.
//
//  Programmer:  Eric Brugger
//  Creation:    December 30, 2002 
//
//  Modifications:
//    Brad Whitlock, Fri Apr 25 10:55:38 PDT 2003
//    I made it so if there is an error setting any of the frames' attributes
//    the plot's error flag is set to true.
//
//    Brad Whitlock, Tue Apr 6 09:15:42 PDT 2004
//    I renamed some variables.
//
// ****************************************************************************

void
ViewerPlot::CheckCache(const int i0, const int i1, const bool force)
{
    //
    // Set the plot attributes for any existing plots and delete any
    // cached actors that need recalculation.
    //
    bool handledFrame = false;
    bool errorOnFrame = false;
    for (int i = i0; i <= i1; i++)
    {
        if (*plotList[i] != 0)
        {
            handledFrame = true;
            TRY
            {
                plotAtts->GetAtts(i, curPlotAtts);
                plotList[i]->SetAtts(curPlotAtts);
                if (force || (*plotList[i])->NeedsRecalculation())
                {
                    plotList[i]   = (avtPlot *)0;
                    actorList[i]  = (avtActor *)0;
                    readerList[i] = (avtDataObjectReader *)0;
                }
            }
            CATCH2(VisItException, e)
            {
                // Create a message to send to the GUI,
                // Stating plot name and message.
                char message[2048];
                SNPRINTF(message, sizeof(message), "%s:  %s",
                         viewerPluginInfo->GetName(),
                         e.Message().c_str());
                Error(message);

                // Indicate that this plot has an error.
                errorOnFrame = true;
            }
            ENDTRY
        }
    }

    if(handledFrame)
        this->errorFlag = errorOnFrame;

    if (queryAtts != 0)
    {
        queryAtts->SetChangeType(PlotQueryInfo::PlotAtts);
        queryAtts->Notify();
    }
}

// ****************************************************************************
// Method: ViewerPlot::CreateNode
//
// Purpose: 
//   Lets the plot save its information for a config file's DataNode.
//
// Arguments:
//   parentNode : The node to which we're saving information.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:09:04 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Dec 15 16:28:58 PST 2003
//   I added the completeSave flag to AttributeSubject::CreateNode to
//   force the plot attributes to write out all of their fields to avoid
//   unwanted settings from the system configs. This makes the session file
//   reproduce the same thing each time without having to run -noconfig.
//
//   Brad Whitlock, Thu Mar 18 09:35:46 PDT 2004
//   I moved some of the settings from ViewerPlotList to here. I also changed
//   the code to not save out beginFrame and endFrame unless we're in
//   keyframing mode because they end up making the session files harder to
//   modify by hand if you use them with larger databases.
//
//   Brad Whitlock, Mon Apr 5 11:38:14 PDT 2004
//   I changed the names of certain fields that are saved out.
//
// ****************************************************************************

void
ViewerPlot::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *plotNode = new DataNode("ViewerPlot");
    parentNode->AddNode(plotNode);

    //
    // Add information specific to the plot.
    //
    plotNode->AddNode(new DataNode("cacheIndex", cacheIndex));
    // Only save beginning and end frames if the plot is keyframed.
    if(viewerPlotList->GetKeyframeMode())
    {
        plotNode->AddNode(new DataNode("beginCacheIndex", beginCacheIndex));
        plotNode->AddNode(new DataNode("endCacheIndex", endCacheIndex));
    }
    plotNode->AddNode(new DataNode("spatialExtentsType",
        avtExtentType_ToString(spatialExtentsType)));
    plotNode->AddNode(new DataNode("bgColor", bgColor, 3));
    plotNode->AddNode(new DataNode("fgColor", fgColor, 3));
    plotNode->AddNode(new DataNode("expandedFlag", expandedFlag));

    //
    // Store the current plot attributes.
    //
    curPlotAtts->CreateNode(plotNode, true, true);

    //
    // Store the operators
    //
    if(nOperators > 0)
    {
        DataNode *operatorNode = new DataNode("Operators");
        operatorNode->AddNode(new DataNode("activeOperatorIndex",
            activeOperatorIndex));

        for(int i = 0; i < nOperators; ++i)
        {
            char tmp[20];
            SNPRINTF(tmp, 20, "operator%02d", i);
            DataNode *opNode = new DataNode(std::string(tmp));
            operatorNode->AddNode(opNode);
            opNode->AddNode(new DataNode("operatorType",
                std::string(operators[i]->GetPluginID())));
            operators[i]->CreateNode(opNode);
        }

        plotNode->AddNode(operatorNode);
    }

    //
    // Store the keyframed plot attributes.
    //
    DataNode *plotKFNode = new DataNode("plotKeyframes");
    if(plotAtts->CreateNode(plotKFNode))
        plotNode->AddNode(plotKFNode);
    else
        delete plotKFNode;

    //
    // Store the keyframed database attributes.
    //
    DataNode *databaseKFNode = new DataNode("databaseKeyframes");
    if(databaseAtts->CreateNode(databaseKFNode))
        plotNode->AddNode(databaseKFNode);
    else
        delete databaseKFNode;

    //
    // Store the SIL restriction.
    //
    CompactSILRestrictionAttributes *csilr = silr->MakeCompactAttributes();
    csilr->CreateNode(plotNode, false, true);
    delete csilr;
}

// ****************************************************************************
// Method: ViewerPlot::SetFromNode
//
// Purpose: 
//   Lets the plot reset its values from a config file.
//
// Arguments:
//   parentNode : The config file information DataNode pointer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 16 13:10:51 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Mar 18 09:02:46 PDT 2004
//   Moved the code to set the frame range, plot state, and SIL restriction
//   here from ViewerPlotList.
//
//   Brad Whitlock, Mon Apr 5 11:41:57 PDT 2004
//   I changed the name of certain fields.
//
// ****************************************************************************

void
ViewerPlot::SetFromNode(DataNode *parentNode)
{
    DataNode *node;

    if(parentNode == 0)
        return;

    DataNode *plotNode = parentNode->GetNode("ViewerPlot");
    if(plotNode == 0)
        return;

    //
    // Get and set the start and end frames.
    //
    bool haveFrameNumbers = true;
    int beginIndex = 0, endIndex = 0;
    if((node = plotNode->GetNode("endFrame")) != 0)
        endIndex = (node->AsInt() < 0) ? 0 : node->AsInt();
    else if((node = plotNode->GetNode("endCacheIndex")) != 0)
        endIndex = (node->AsInt() < 0) ? 0 : node->AsInt();
    else
        haveFrameNumbers = false;
    if((node = plotNode->GetNode("beginFrame")) != 0)
    {
        int f = node->AsInt();
        beginIndex = (f < 0 || f > endIndex) ? 0 : f;
    }
    else if((node = plotNode->GetNode("beginCacheIndex")) != 0)
    {
        int f = node->AsInt();
        beginIndex = (f < 0 || f > endIndex) ? 0 : f;
    }
    else
        haveFrameNumbers = false;
    if(haveFrameNumbers)
        SetRange(beginIndex, endIndex);

    //
    // Set the plot's state and make sure it is within the
    // range of the plot databases's correlation.
    //
    int ci = 0, tsNStates = 1;
    viewerPlotList->GetTimeSliderStates(GetSource(), ci, tsNStates);
    if((node = plotNode->GetNode("cacheIndex")) != 0)
    {
        ci = node->AsInt();
        if(ci < 0)
            ci = 0;
        if(ci >= tsNStates)
            ci = tsNStates - 1;
        SetCacheIndex(ci);
    }

    // Read in some plot attributes.
    if((node = plotNode->GetNode("spatialExtentsType")) != 0)
    {
        avtExtentType t;
        if(avtExtentType_FromString(node->AsString(), t))
            SetSpatialExtentsType(t);
    }
    if((node = plotNode->GetNode("bgColor")) != 0)
        SetBackgroundColor(node->AsDoubleArray());
    if((node = plotNode->GetNode("fgColor")) != 0)
        SetForegroundColor(node->AsDoubleArray());
    if((node = plotNode->GetNode("expandedFlag")) != 0)
        expandedFlag = node->AsBool();

    // Read in the current plot attributes.
    curPlotAtts->SetFromNode(plotNode);
    SetPlotAtts(curPlotAtts);

    //
    // Read in the plot keyframes.
    //
    if((node = plotNode->GetNode("plotKeyframes")) != 0)
        plotAtts->SetFromNode(node, curPlotAtts);

    //
    // Read in the database keyframes.
    //
    if((node = plotNode->GetNode("databaseKeyframes")) != 0)
        databaseAtts->SetFromNode(node, curDatabaseAtts);

    //
    // Add operators.
    //
    DataNode *operatorNode = plotNode->GetNode("Operators");
    if(operatorNode)
    {
        bool addOperator = true;
        for(int i = 0; addOperator; ++i)
        {
            char key[20];
            SNPRINTF(key, 20, "operator%02d", i);
            DataNode *opNode = operatorNode->GetNode(key);
            if(opNode)
            {
                //
                // Add the operator.
                //
                if((node = opNode->GetNode("operatorType")) != 0) 
                {
                    int type = OperatorPluginManager::Instance()->
                        GetEnabledIndex(node->AsString());
                    if(type != -1)
                    {
                        int index = AddOperator(type);

                        // Let the operator finish initializing itself.
                        if(index != -1)
                            operators[index]->SetFromNode(opNode);
                    }
                }
            }
            else
                addOperator = false;
        }

        // Now that operators are created, set the active operator index.
        if((node = operatorNode->GetNode("activeOperatorIndex")) != 0)
        {
            if(node->AsInt() < nOperators && node->AsInt() >= 0)
                activeOperatorIndex = node->AsInt();
        }
    }

    //
    // Read the SIL restriction
    //
    if((node = plotNode->GetNode("CompactSILRestrictionAttributes")) != 0)
    {
        CompactSILRestrictionAttributes csilr;
        csilr.SetFromNode(plotNode);

        //
        // Try to initialize the plot's SIL restriction using the compact
        // SIL restriction.
        //
        TRY
        {
            avtSILRestriction_p newsilr = new avtSILRestriction(*silr, csilr);
            SetSILRestriction(newsilr);
        }
        CATCH(VisItException)
        {
            debug1 << "Could not use the SIL restriction from the "
                      "session file." << endl;
        }
        ENDTRY
    }
}

// ****************************************************************************
// Method: ViewerPlot::InitializePlot
//
// Purpose: 
//   This method initializes a Plot object with the ViewerPlot's attributes
//   so we can send the Plot object back to the client.
//
// Arguments:
//   plot : The Plot object to initialize.
//
// Notes:      This code was moved from ViewerPlotList but I changed it so I
//             could remove some methods that should not have been public in
//             ViewerPlot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 5 09:57:43 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlot::InitializePlot(Plot &plot) const
{
    // Set the plot type.
    plot.SetPlotType(type);
    // Set the database name and add the plot to the plot list.
    plot.SetDatabaseName(GetSource());
    // Set the plot variable.
    plot.SetPlotVar(variableName);
    plot.SetExpandedFlag(expandedFlag);
    plot.SetBeginFrame(beginCacheIndex);
    plot.SetEndFrame(endCacheIndex);
    plot.SetIsFromSimulation(engineKey.IsSimulation());
 
    // Set the keyframe indices.
    int j, nIndices;
    const int *indices = plotAtts->GetIndices(nIndices);
    intVector ivec;
    for (j = 0; j < nIndices; j++)
        ivec.push_back(indices[j]);
    plot.SetKeyframes(ivec);

    // Set the database keyframe indices.
    ivec.clear();
    indices = databaseAtts->GetIndices(nIndices);
    for (j = 0; j < nIndices; j++)
        ivec.push_back(indices[j]);
    plot.SetDatabaseKeyframes(ivec);

    // Set the operators that are applied to the plot
    for (int op_index = 0; op_index < GetNOperators(); ++op_index)
    {
        plot.AddOperator(GetOperator(op_index)->GetType());
    }
    plot.SetActiveOperator(activeOperatorIndex);
}

// ****************************************************************************
// Method: ViewerPlot::SetOpaqueMeshIsAppropriate
//
// Purpose: 
//   Notifies avtPlot if opaque mesh is appropriate. 
//
// Arguments:
//   val :  True if opaque mesh is appropriate, false otherwise.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 27, 2003 
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 29 12:31:18 PDT 2003
//   Updated client atts, too.
//
//   Brad Whitlock, Mon Apr 5 09:30:08 PDT 2004
//   I rewrote it since I changed how it is called.
//
//   Kathleen Bonnell, Tue Aug 24 16:19:00 PDT 2004 
//   Send MeshType as arg to SetOpaqueMeshIsAppropriate. 
//
//   Kathleen Bonnell, Tue Nov  2 11:13:15 PST 2004 
//   Removed MeshType arg from SetOpaqueMeshIsAppropriate.  (MeshType is now
//   sent to the avtPlot.)
//
// ****************************************************************************

void
ViewerPlot::SetOpaqueMeshIsAppropriate(bool val)
{
    if(isMesh)
    {
        if(*plotList[cacheIndex] != 0)
        {
            const AttributeSubject *atts = plotList[cacheIndex]->
                SetOpaqueMeshIsAppropriate(val);
            if(atts != 0)
            {
                // Set the attributes into the avtPlot.
                plotList[cacheIndex]->SetAtts(atts);
                // Set the attributes into the current plot attributes.
                curPlotAtts->CopyAttributes(atts);
            }
        }
    }
}


// ****************************************************************************
// Method: ViewerPlot::IsMesh
//
// Purpose: 
//   Returns whether or not the plot type is Mesh Plot.
//
// Returns:
//   true if this is a mesh plot, false otherwise.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 27, 2003 
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::IsMesh()
{
    return isMesh;
}


// ****************************************************************************
// Method: ViewerPlot::IsLabel
//
// Purpose: 
//   Returns whether or not the plot type is Label Plot.
//
// Returns:
//   true if this is a label plot, false otherwise.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 11, 2005 
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlot::IsLabel()
{
    return isLabel;
}


// ****************************************************************************
//  Method:  ViewerPlot::GetEngineKey
//
//  Purpose:
//    Returns the engine key where this plot originated.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 29, 2004
//
// ****************************************************************************
const EngineKey &
ViewerPlot::GetEngineKey() const
{
    return engineKey;
}

// ****************************************************************************
//  Method: ViewerPlot::GetVariableCentering
//
//  Purpose:
//    Get the centering the plot's variable. 
//
//  Returns:  the variable's centering
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 2, 2004
//
//  Modifications:
//
// ****************************************************************************

const avtCentering
ViewerPlot::GetVariableCentering() const
{
    avtCentering retval = AVT_UNKNOWN_CENT; 

    if(readerList != NULL)
    {
        if(*(readerList[cacheIndex]) != NULL)
        {
            avtDataAttributes &atts = readerList[cacheIndex]->
                GetInfo().GetAttributes();
            retval = atts.GetCentering(variableName.c_str());
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerPlot::UpdateDataExtents
//
//  Purpose:
//    Update the plot's data extents.
//
//  Programmer: Eric Brugger
//  Creation:   March 30, 2004
//
//  Modifications:
//    Brad Whitlock, Fri Apr 9 09:05:37 PDT 2004
//    Made it use cacheIndex.
//
// ****************************************************************************

void
ViewerPlot::UpdateDataExtents()
{
    dataExtents.clear();
    plotList[cacheIndex]->GetDataExtents(dataExtents);
}

// ****************************************************************************
//  Method: ViewerPlot::GetWindowId
//
//  Purpose: Return the window Id for this plot
//
//  Programmer: Mark C. Miller 
//  Creation:   June 8, 2004
//
// ****************************************************************************

int
ViewerPlot::GetWindowId() const
{
    if (viewerPlotList == NULL)
        EXCEPTION0(ImproperUseException);

    return viewerPlotList->GetWindowId();
}


// ****************************************************************************
//  Method: ViewerPlot::GetMeshType
//
//  Purpose: Return the mesh type for this plot. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 24, 2004
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  2 11:13:15 PST 2004
//    Return unknown mesh type if MeshMetaData is NULL.
//
// ****************************************************************************

avtMeshType
ViewerPlot::GetMeshType() const
{
    const avtDatabaseMetaData *md = GetMetaData();
    string meshName = md->MeshForVar(variableName);
    const avtMeshMetaData *mmd = md->GetMesh(meshName);
    if (mmd)
        return mmd->meshType;
    else 
        return AVT_UNKNOWN_MESH;
}
