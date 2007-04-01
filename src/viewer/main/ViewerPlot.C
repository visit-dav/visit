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
#include <DatabaseAttributes.h>
#include <OperatorPluginManager.h>
#include <PickAttributes.h>
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
// This is a static member of ViewerPlot.
//
avtActor_p ViewerPlot::nullActor((avtActor *)0);
avtDataObjectReader_p ViewerPlot::nullReader((avtDataObjectReader *)0);

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
// ****************************************************************************

ViewerPlot::ViewerPlot(const int type_,
    ViewerPlotPluginInfo *viewerPluginInfo_, const char *hostName_,
    const char *databaseName_, const char *variableName_,
    avtSILRestriction_p silr_, const int frame0_, const int frame1_,
    const int nStates)
{
    //
    // Initialize some values.
    //
    type                = type_;
    viewerPluginInfo    = viewerPluginInfo_;
    isMesh = (strcmp(viewerPluginInfo->GetName(), "Mesh") == 0); 
    hostName            = 0;
    databaseName        = 0;
    variableName        = 0;
    databaseAtts        = new AttributeSubjectMap;
    curDatabaseAtts     = new DatabaseAttributes;
    silr                = 0;
    frame0              = 0;
    frame1              = 0;
    nOperators          = 0;
    nOperatorsAlloc     = 0;
    operators           = 0;
    activeOperatorIndex = -1;
    plotAtts            = new AttributeSubjectMap;
    curPlotAtts         = viewerPluginInfo->AllocAttributes();
    plotList            = new avtPlot_p[1];
    plotList[0]         = 0;
    actorList           = new avtActor_p[1];
    actorList[0]        = 0;
    readerList          = new avtDataObjectReader_p[1];
    readerList[0]       = 0;
    expandedFlag        = false;
    errorFlag           = false;
    networkID           = -1;
    queryAtts           = 0;               
    viewerPlotList      = NULL;

    bgColor[0] = bgColor[1] = bgColor[2] = 1.0; 
    fgColor[0] = fgColor[1] = fgColor[2] = 0.0; 

    //
    // Use the constructor's arguments to initialize the object further.
    //
    viewerPluginInfo->InitializePlotAtts(curPlotAtts, hostName_, databaseName_,
                                         variableName_);
    plotAtts->SetAtts(0, curPlotAtts);

    curDatabaseAtts->SetState(0);
    databaseAtts->SetAtts(0, curDatabaseAtts);
    curDatabaseAtts->SetState(nStates - 1);
    databaseAtts->SetAtts(frame1_ - frame0_, curDatabaseAtts);

    SetFrameRange(frame0_, frame1_);
    SetHostDatabaseName(hostName_, databaseName_);
    SetVariableName(variableName_);
    SetSILRestriction(silr_);
    spatialExtentsType = AVT_ORIGINAL_EXTENTS;
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
// ****************************************************************************

ViewerPlot::~ViewerPlot()
{
    if (networkID != -1)
        ViewerEngineManager::Instance()->ReleaseData(hostName, networkID);

    //
    // Delete the variable name.
    //
    delete [] hostName;
    delete [] databaseName;
    delete [] variableName;

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
//  Method: ViewerPlot::SetFrameRange
//
//  Purpose:
//    Set the inclusive frame range that the plot is defined over.
//
//  Arguments:
//    f0        The first frame at which the plot is defined.
//    f1        The last frame at which the plot is defined.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Made use of debug stream.
//
//    Eric Brugger, Fri Aug 31 09:41:38 PDT 2001
//    I replaced the avtPlot with a list of avtPlots, one per frame.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

void
ViewerPlot::SetFrameRange(const int f0, const int f1)
{
    //
    // Check the frame values.
    //
    if (f0 < 0 || f1 < f0)
    {
        debug1 << "Invalid frame range.\n";
        return;
    }

    //
    // If the frame range length is different, then create new caches,
    // copying the overlap.
    //
    if ((f1 - f0) != (frame1 - frame0))
    {
        //
        // Create a new plotList, actorList and readerList.
        //
        avtPlot_p *newPlotList=0;
        avtActor_p *newActorList=0;
        avtDataObjectReader_p *newReaderList=0;
        newPlotList   = new avtPlot_p[f1 - f0 + 1];
        newActorList  = new avtActor_p[f1 - f0 + 1];
        newReaderList = new avtDataObjectReader_p[f1 - f0 + 1];

        //
        // Copy the overlap from the old lists to the new lists.
        //
        int i;
        int overlap;

        if ((f1 - f0) < (frame1 - frame0))
        {
            ClearActors(frame0 + (f1 - f0 + 1), frame1);
        }

        overlap = (f1 - f0) < (frame1 - frame0) ? (f1 - f0) : (frame1 - frame0);
        for (i = 0; i < overlap + 1; ++i)
        {
            newPlotList[i] = plotList[i];
            newActorList[i] = actorList[i];
            newReaderList[i] = readerList[i];
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
    }

    //
    // Set the new frame range.
    //
    frame0 = f0;
    frame1 = f1;
}

// ****************************************************************************
//  Method: ViewerPlot::IsInFrameRange
//
//  Purpose:
//    Return a boolean indicating if the specified frame is within the range
//    that the plot is defined over.
//
//  Arguments:
//    frame     The frame to check.
//
//  Returns:    A boolean indicating if the frame is within the range.
//
//  Programmer: Eric Brugger
//  Creation:   August 25, 2000
//
//  Modifications:
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

bool
ViewerPlot::IsInFrameRange(const int frame) const
{
    return ((frame >= frame0) && (frame <= frame1)) ? true : false;
}

// ****************************************************************************
//  Method: ViewerPlot::GetBeginFrame
//
//  Purpose:
//    Return the first frame that the plot exists.
//
//  Returns:    The first frame that the plot exists.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
 
int
ViewerPlot::GetBeginFrame() const
{
    return frame0;
}

// ****************************************************************************
//  Method: ViewerPlot::GetEndFrame
//
//  Purpose:
//    Return the last frame that the plot exists.
//
//  Returns:    The last frame that the plot exists.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
 
int
ViewerPlot::GetEndFrame() const
{
    return frame1;
}

// ****************************************************************************
//  Method: ViewerPlot::GetKeyframeIndices
//
//  Purpose:
//    Return the keyframe indices in the plot.
//
//  Arguments:
//    nIndices  The number of keyframe indices in the plot.
//
//  Returns:    The keyframe indices in the plot.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
 
const int *
ViewerPlot::GetKeyframeIndices(int &nIndices) const
{
    return plotAtts->GetIndices(nIndices);
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
// ****************************************************************************
 
void
ViewerPlot::DeleteKeyframe(const int frame)
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Delete the keyframe at the specified frame.  DeleteAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int       f0, f1;

    if (!plotAtts->DeleteAtts(frame - frame0, f0, f1))
        return;
    f1 = f1 < (frame1 - frame0) ? f1 : (frame1 - frame0);

    CheckCache(f0, f1, false);
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
// ****************************************************************************
 
void
ViewerPlot::MoveKeyframe(int oldFrame, int newFrame)
{
    //
    // Check that the frames are within range.
    //
    if ((oldFrame < frame0) || (oldFrame > frame1) ||
        (newFrame < frame0) || (newFrame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Move the keyframe at oldFrame to newFrame.  MoveAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int       f0, f1;

    if (!plotAtts->MoveAtts(oldFrame - frame0, newFrame - frame0, f0, f1))
        return;
    f1 = f1 < (frame1 - frame0) ? f1 : (frame1 - frame0);

    CheckCache(f0, f1, false);
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
// ****************************************************************************

void
ViewerPlot::SetHostDatabaseName(const char *host, const char *database)
{
    bool reInit = false;
    //
    // If the host name is already set, check if the host and database
    // names match and return if they do, otherwise deallocate the existing
    // name and clear the actor list.
    //
    if (hostName != 0)
    {
        if (strcmp(hostName, host) == 0 && strcmp(databaseName, database) == 0)
        {
            return;
        }
        delete [] hostName;
        delete [] databaseName;
        ClearActors();
        reInit = true;
    }

    //
    // Set the host and database names.
    //
    hostName = new char[strlen(host)+1];
    strcpy(hostName, host); 
    databaseName = new char[strlen(database)+1];
    strcpy(databaseName, database);
    if (reInit)
    {
        viewerPluginInfo->ReInitializePlotAtts(curPlotAtts, hostName,
                                               databaseName, variableName);

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
// ****************************************************************************

const char *
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
// ****************************************************************************

const char *
ViewerPlot::GetDatabaseName() const
{
    return databaseName;
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
// ****************************************************************************

bool
ViewerPlot::SetVariableName(const char *name)
{
    bool retval = false;
    bool notifyQuery = false;
    static bool firstTime = true;
    //
    // If the variable name is already set, check if the names match and
    // return if they do, otherwise deallocate the existing name and clear
    // the actor list.
    //
    if (variableName != 0)
    {
        if (strcmp(variableName, name) == 0)
        {
            return retval;
        }

        ClearActors();
        notifyQuery = true;

        //
        // Determine if we need to also set a new SIL restriction.
        //
        if(silr->GetWholes().size() > 0)
        {
            TRY
            {
                ViewerFileServer *s = ViewerFileServer::Instance();
                avtDatabaseMetaData *md = (avtDatabaseMetaData *)s->GetMetaData(
                    std::string(hostName), std::string(databaseName));
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
                    CATCH(InvalidVariableException)
                    {
                        CATCH_RETURN2(2, false);
                    }
                    ENDTRY

                    //
                    // The new variable has a different top set from the
                    // old variable. Set the top set in the SIL restriction.
                    //
                    avtSILSet_p current = silr->GetSILSet(silr->GetTopSet());
                    if(meshName != current->GetName())
                    {
                        int topSet = 0;
                        for(int i = 0; i < silr->GetWholes().size(); ++i)
                        {
                            current = silr->GetSILSet(silr->GetWholes()[i]);
                            if(meshName == current->GetName())
                            {
                                topSet = silr->GetWholes()[i];
                                break;
                            }
                        }
                        //
                        // Change the top set in the current SIL restriction.
                        // This is sufficient due to the previous call to
                        // ClearActors(). Note that we must select all sets
                        // under the new top set.
                        //
                        silr->SetTopSet(topSet);
                        silr->TurnOffAll();
                        silr->TurnOnSet(topSet);

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
    delete [] variableName;
    variableName = new char [strlen(name)+1];
    strcpy (variableName, name);

    if (!firstTime && curPlotAtts->VarChangeRequiresReset())
    { 
        viewerPluginInfo->ResetPlotAtts(curPlotAtts, hostName, databaseName,
                                        variableName);
        viewerPluginInfo->SetClientAtts(curPlotAtts);

        delete plotAtts;
        plotAtts = new AttributeSubjectMap;
        plotAtts->SetAtts(0, curPlotAtts);
    }

    //
    // Set the variable name for any existing plots.
    //
    int       i;
    for (i = 0; i < (frame1 - frame0 + 1); i++)
    {
        if (*plotList[i] != 0)
        {
            plotList[i]->SetVarName(variableName);
        }
    }
    firstTime = false;


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
// ****************************************************************************

const char *
ViewerPlot::GetVariableName() const
{
    return variableName;
}

// ****************************************************************************
//  Method: ViewerPlot::SetDatabaseAtts
//
//  Purpose:
//    Set the database attributes for the plot.
//
//  Arguments:
//    atts      The new database attributes for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   December 26, 2002
//
// ****************************************************************************

void
ViewerPlot::SetDatabaseAtts(const AttributeSubjectMap *atts)
{
    databaseAtts->CopyAttributes(atts);
}

// ****************************************************************************
//  Method: ViewerPlot::GetDatabaseAtts
//
//  Purpose:
//    Get the database attributes for the plot.
//
//  Returns:    The database attributes for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   December 26, 2002
//
// ****************************************************************************

const AttributeSubjectMap *
ViewerPlot::GetDatabaseAtts() const
{
    return databaseAtts;
}

// ****************************************************************************
//  Method: ViewerPlot::SetDatabaseState
//
//  Purpose:
//    Set the database state at the specified frame for the plot.
//
//  Arguments:
//    frame     The frame for which to set the database state.
//    state     The database state to set for the specified frame.
//
//  Programmer: Eric Brugger
//  Creation:   December 26, 2002
//
// ****************************************************************************

void
ViewerPlot::SetDatabaseState(const int frame, const int state)
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Set the database state for the specified frame.  SetAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since SetAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int       f0, f1;

    curDatabaseAtts->SetState(state);
    databaseAtts->SetAtts(frame - frame0, curDatabaseAtts, f0, f1);
    f1 = f1 < (frame1 - frame0) ? f1 : (frame1 - frame0);

    CheckCache(f0, f1, true);
}

// ****************************************************************************
//  Method: ViewerPlot::GetDatabaseState
//
//  Purpose:
//    Get the database state at the specified frame for the plot.
//
//  Arguments:
//    frame     The frame for which to get the database state.
//
//  Returns:    The database state at the specified frame.
//
//  Programmer: Eric Brugger
//  Creation:   December 26, 2002
//
// ****************************************************************************

int
ViewerPlot::GetDatabaseState(const int frame) const
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return 0;
    }

    databaseAtts->GetAtts(frame - frame0, curDatabaseAtts);
    return curDatabaseAtts->GetState();
}

// ****************************************************************************
//  Method: ViewerPlot::GetDatabaseKeyframeIndices
//
//  Purpose:
//    Return the database keyframe indices in the plot.
//
//  Arguments:
//    nIndices  The number of database keyframe indices in the plot.
//
//  Returns:    The database keyframe indices in the plot.
//
//  Programmer: Eric Brugger
//  Creation:   January 2, 2003
//
// ****************************************************************************
 
const int *
ViewerPlot::GetDatabaseKeyframeIndices(int &nIndices) const
{
    return databaseAtts->GetIndices(nIndices);
}

// ****************************************************************************
//  Method: ViewerPlot::DeleteDatabaseKeyframe
//
//  Purpose:
//    Delete the database keyframe at the specified frame for the plot.
//
//  Arguments:
//    frame     The frame at which to delete the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   December 26, 2002
//
// ****************************************************************************

void
ViewerPlot::DeleteDatabaseKeyframe(const int frame)
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Delete the keyframe at the specified frame.  DeleteAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int       f0, f1;

    if (!databaseAtts->DeleteAtts(frame - frame0, f0, f1))
    {
        return;
    }
    f1 = f1 < (frame1 - frame0) ? f1 : (frame1 - frame0);
 
    CheckCache(f0, f1, true);
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
// ****************************************************************************

void
ViewerPlot::MoveDatabaseKeyframe(int oldFrame, int newFrame)
{
    //
    // Check that the frames are within range.
    //
    if ((oldFrame < frame0) || (oldFrame > frame1) ||
        (newFrame < frame0) || (newFrame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    //
    // Move the keyframe at oldFrame to newFrame.  MoveAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since DeleteAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int       f0, f1;

    if (!databaseAtts->MoveAtts(oldFrame - frame0, newFrame - frame0, f0, f1))
        return;
    f1 = f1 < (frame1 - frame0) ? f1 : (frame1 - frame0);
 
    CheckCache(f0, f1, true);
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
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

void
ViewerPlot::SetSILRestriction(avtSILRestriction_p s)
{
    //
    // Assign the new SIL resitriction to the plot.
    //
    silr = s;

    //
    // Loop over the existing plots and delete any cached actors whose SIL
    // restriction is different from the new one.
    //
    int       i;

    for (i = 0; i < (frame1 - frame0 + 1); i++)
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
// ****************************************************************************

int
ViewerPlot::AddOperator(const int type, const bool fromDefault)
{
    if (nOperators > 0)
    {
        if (!operators[nOperators-1]->AllowsSubsequentOperators())    
        {
            char msg[100];
            SNPRINTF(msg, 200, "VisIt cannot apply other operators after a "
                     "%s operator.", operators[nOperators-1]->GetName());
            Error(msg);
            return -1;
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
//    frame     The frame to set the actor for.
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
// ****************************************************************************

void
ViewerPlot::SetActor(const int frame, const avtActor_p actor)
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    actorList[frame-frame0] = actor;
}

// ****************************************************************************
//  Method: ViewerPlot::SetReader
//
//  Purpose:
//    Set the reader for the specified frame.
//
//  Arguments:
//    frame     The frame to set the reader for.
//    reader    The reader that the plot originated from.
//
//  Programmer: Hank Childs
//  Creation:   October 17, 2000
//
//  Modifications:
//    Hank Childs, Mon Aug 20 14:49:44 PDT 2001
//    Made use of debug stream.
//
//    Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

void
ViewerPlot::SetReader(const int frame, avtDataObjectReader_p reader)
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return;
    }

    readerList[frame-frame0] = reader;
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
// ****************************************************************************

avtActor_p &
ViewerPlot::GetActor(const int frame) const
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return nullActor;
    }

    return actorList[frame-frame0];
}

// ****************************************************************************
//  Method: ViewerPlot::NoActorExists
//
//  Purpose: 
//    This is a convenience method that returns whether or not the specified
//    frame has an actor.
//
//  Arguments:
//    frame     The frame that we're checking.
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
// ****************************************************************************

bool
ViewerPlot::NoActorExists(const int frame) const
{
    return (*GetActor(frame) == 0);
}

// ****************************************************************************
//  Method: ViewerPlot::GetReader
//
//  Purpose:
//    Return the reader for the plot at the specified frame.
//
//  Arguments:
//    frame     The frame to return the reader for.
//
//  Returns:    The reader for the plot at the specified frame.
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
// ****************************************************************************

avtDataObjectReader_p &
ViewerPlot::GetReader(const int frame) const
{
    //
    // Check that the frame is within range.
    //
    if ((frame < frame0) || (frame > frame1))
    {
        debug1 << "The frame is out of range." << endl;
        return nullReader;
    }

    return readerList[frame-frame0];
}

// ****************************************************************************
//  Method: ViewerPlot::CreateActor
//
//  Purpose:
//    Create the actor for the plot for the specified frame.
//
//  Arguments:
//    frame     The frame to create the plot for.
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
// ****************************************************************************

// only place in ViewerPlot where ViewerWindowManager is needed
#include <ViewerWindowManager.h>

void
ViewerPlot::CreateActor(const int frame, bool createNew,
                                         bool turningOffScalableRendering)
{
    avtDataObjectReader_p reader;

    std::vector<bool> oldAble;
    ViewerWindowManager::Instance()->DisableExternalRenderRequestsAllWindows(oldAble);

    // Get a data reader.
    TRY
    {
        ViewerFileServer *server = ViewerFileServer::Instance();
        bool invariantMetaData = server->MetaDataIsInvariant(GetHostName(),
                                                            GetDatabaseName());

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
                                                         GetVariableName());
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
                         GetDataObjectReader(this,frame);
        }
    }
    CATCH(AbortException)
    {
        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(frame, nullActor);
        
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
                     e.GetMessage().c_str());
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
                     e.GetMessage().c_str());
        }
     
        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(frame, nullActor);
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
                 variableName, databaseName, hostName);
        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(frame, nullActor);
        return;
    }
    SetReader(frame, reader);

    plotList[frame-frame0] = viewerPluginInfo->AllocAvtPlot();
    plotAtts->GetAtts(frame - frame0, curPlotAtts);
    plotList[frame-frame0]->SetAtts(curPlotAtts);
    plotList[frame-frame0]->SetVarName(variableName);
    plotList[frame-frame0]->SetBackgroundColor(bgColor);
    plotList[frame-frame0]->SetForegroundColor(fgColor);
    plotList[frame-frame0]->SetIndex(networkID);
    plotList[frame-frame0]->SetCurrentSILRestriction(silr);

    TRY
    {
        avtActor_p actor = plotList[frame-frame0]->Execute(reader);
        this->SetActor(frame, actor);

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
                 e.GetMessage().c_str());
        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(frame, nullActor);
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
                 e.GetMessage().c_str());

        Error(message);

        // Indicate that this plot has an error.
        this->errorFlag = true;

        // Use the null actor.
        this->SetActor(frame, nullActor);
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
// ****************************************************************************

void
ViewerPlot::ClearActors()
{
    //
    // Delete all the actors.
    //
    int       i;

    for (i = 0; i < (frame1 - frame0 + 1); i++)
    {
        plotList[i]   = (avtPlot *)0;
        actorList[i]  = (avtActor *)0;
        readerList[i] = (avtDataObjectReader *)0;
    }
}

// ****************************************************************************
//  Method: ViewerPlot::ClearActors
//
//  Purpose:
//    Clear all the actors associated with the plot over the specified
//    range.
//
//  Arguments:
//    f0        The first frame in the range (inclusive).
//    f1        The last frame in the range (inclusive).
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
// ****************************************************************************

void
ViewerPlot::ClearActors(const int f0, const int f1)
{
    //
    // Check that the range is valid.
    //
    if ((f0 < frame0) || (f0 > frame1) || (f1 < frame0) || (f1 > frame1) ||
        (f0 > f1))
    {
        debug1 << "Invalid frame range." << endl;
        return;
    }

    //
    // Delete the actors within the range.
    //
    int       i;

    for (i = f0 - frame0; i <= f1 - frame0; i++)
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
// ****************************************************************************

void
ViewerPlot::TransmuteActor(int frame, bool turningOffScalableRendering)
{
    CreateActor(frame, false, turningOffScalableRendering);
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
// ****************************************************************************

int
ViewerPlot::GetSpatialDimension(const int frame) const
{
    int retval = 0;

    if(readerList != NULL)
    {
        if(*(readerList[frame-frame0]) != NULL)
        {
            avtDataAttributes &atts = readerList[frame-frame0]->
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
// ****************************************************************************

double *
ViewerPlot::GetSpatialExtents(const int frame, avtExtentType extsType) const
{
    //
    // If extsType is sent in as AVT_UNKNOWN_EXTENT_TYPE, then that is a signal
    // that we should be using our own data member instead.
    //
    avtExtentType realExtsType = (extsType == AVT_UNKNOWN_EXTENT_TYPE 
                                  ? spatialExtentsType : extsType);
   
    // Return early if the reader does not exist.
    if(*(readerList[frame-frame0]) == 0)
        return 0;

    // Populate some local variables.
    avtDataAttributes &atts = readerList[frame-frame0]->
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

    if (! readerList[frame-frame0]->InputIsDataset())
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

    avtDataset_p ds = readerList[frame-frame0]->GetDatasetOutput();

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
// ****************************************************************************

bool
ViewerPlot::ExecuteEngineRPC(const int frame)
{
    //
    //  Release data on previous network.
    //
    if (networkID != -1)
        ViewerEngineManager::Instance()->ReleaseData(hostName, networkID);

    ViewerEngineManager *engineMgr = ViewerEngineManager::Instance();
    plotAtts->GetAtts(frame - frame0, curPlotAtts);
    bool successful = engineMgr->MakePlot(hostName, viewerPluginInfo->GetID(),
                                          curPlotAtts, &networkID);
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
// ****************************************************************************

void
ViewerPlot::SetClientAttsFromPlot(int frame)
{
    //
    // If the frame is out of range do nothing.
    //
    if (frame < frame0 || frame > frame1)
        return;

    //
    // Set the client attributes.
    //
    plotAtts->GetAtts(frame - frame0, curPlotAtts);
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
//  Method: ViewerPlot::SetPlotAttsFromClient
//
//  Purpose:
//    Set the plot attributes for the specified frame based on the client
//    attributes.
//
//  Arguments:
//    frame : The frame for which to set the attributes.
//
//  Programmer: Eric Brugger
//  Creation:   March 9, 2001
//
//  Modifications:
//    Eric Brugger, Thu Sep  6 14:16:43 PDT 2001
//    I modified the routine to not clear the actors and instead set the plot
//    attributes for any existing plots.
//
//    Eric Brugger, Fri Sep  7 12:48:45 PDT 2001
//    I added logic to delete the plot and actor if the plot needs
//    recalculation.
//
//    Kathleen Bonnell, Thu Mar 28 08:21:21 PST 2002 
//    Handle exceptions. 
//    
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

void
ViewerPlot::SetPlotAttsFromClient(int frame)
{
    //
    // If the frame is out of range do nothing.
    //
    if (frame < frame0 || frame > frame1)
        return;
 
    viewerPluginInfo->GetClientAtts(curPlotAtts);
    SetPlotAtts(frame, curPlotAtts);
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

    //
    // Set the plot attributes for the entire plot.
    //
    plotAtts->SetAtts(curPlotAtts);
 
    CheckCache(0, frame1 - frame0, false);

    return true;
}

// ****************************************************************************
// Method: ViewerPlot::SetPlotAtts
//
// Purpose: 
//   Tries to set the attributes for the specified frame. This only happens
//   if they are compatible types.
//
// Arguments:
//   frame : The frame for which to set the attributes.
//   atts  : The new attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 8 09:46:30 PDT 2002
//
// Modifications:
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002  
//    Update queryAtts. 
//   
//    Eric Brugger, Mon Nov 18 09:16:38 PST 2002
//    I added keyframing support.
//
// ****************************************************************************

bool
ViewerPlot::SetPlotAtts(int frame, const AttributeSubject *atts)
{
    //
    // If the frame is out of range do nothing.
    //
    if (frame < frame0 || frame > frame1)
        return false;
 
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

    //
    // Set the plot attributes for the specified frame.  SetAtts
    // returns the range of plots that were invalidated.  The
    // maximum value is clamped to frame1 since SetAtts may return
    // INT_MAX to indicate the end of the plot.
    //
    int       f0, f1;

    plotAtts->SetAtts(frame - frame0, curPlotAtts, f0, f1);
    f1 = f1 < (frame1 - frame0) ? f1 : (frame1 - frame0);
 
    CheckCache(f0, f1, false);

    return true;
}

// ****************************************************************************
// Method: ViewerPlot::SetPlotAtts
//
// Purpose: 
//   Tries to copy the incoming atts into the plotAtts. This only happens
//   if they are compatible types.
//
// Arguments:
//   atts : The new attributes.
//
// Programmer: Eric Brugger
// Creation:   December 10, 2002
//
// ****************************************************************************

bool
ViewerPlot::SetPlotAtts(const AttributeSubjectMap *atts)
{
    plotAtts->CopyAttributes(atts);
    return true;
}

// ****************************************************************************
// Method: ViewerOperator::GetPlotAtts
//
// Purpose: 
//   Returns a const pointer to the plot attributes.
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
// ****************************************************************************

const AttributeSubjectMap *
ViewerPlot::GetPlotAtts() const
{
    return plotAtts;
}

// ****************************************************************************
// Method: ViewerOperator::GetPlotAtts
//
// Purpose: 
//   Returns a const pointer to the current plot attributes.
//
// Programmer: Mark C. Miller 
// Creation:   02Apr03 
//
// ****************************************************************************
const AttributeSubject *
ViewerPlot::GetCurrentPlotAtts() const
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

    for (i = 0; i < frame1 - frame0 + 1; i++)
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
    for (int i = 0; i < frame1 - frame0 + 1; i++)
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
    for (int i = 0; i < frame1 - frame0 + 1; i++)
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
// Method: ViewerPlot::HandleTool
//
// Purpose: 
//   Sets the attributes for the specified frame from the tool.
//
// Arguments:
//   frame : The frame for which to set the attributes.
//   ti : A reference to the tool interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 9 15:46:07 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 11 14:02:08 PST 2002
//   Made it return a boolean value.
//
//   Brad Whitlock, Tue Oct 8 08:34:52 PDT 2002
//   I made it set the plot attributes.
//
//   Eric Brugger, Tue Nov 26 10:59:42 PST 2002
//   I added keyframing support.
//
//   Brad Whitlock, Thu Apr 17 09:13:13 PDT 2003
//   I changed the code so it uses the active operator. I also changed it so
//   it will not attempt to give tool attributes to operators if they've
//   already been given to the plot.
//
// ****************************************************************************

bool
ViewerPlot::HandleTool(int frame, const avtToolInterface &ti)
{
    //
    // Set the plot attributes from the tool attributes.
    //
    bool val = SetPlotAtts(frame, ti.GetAttributes());

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
//  Arguments:
//    frame      The frame to use.
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
// ****************************************************************************

bool
ViewerPlot::StartPick(const int frame)
{
    bool needsUpdate = false;

    //
    //  Don't necessarily want to send the StartPick to the engine
    //  for each and every plot, but how else to manage this?  Really
    //  want to do this only if there are different engines for different
    //  plots.  But how to know from ViewerPlotList??
    //
    if (ViewerEngineManager::Instance()->StartPick(hostName, true, networkID))
    {
        if (IsInFrameRange(frame) && *plotList[frame-frame0] != NULL)
        {
            needsUpdate |= (*plotList[frame-frame0])->RequiresReExecuteForQuery();
        }
        if (needsUpdate)
        {
            ClearActors(frame, frame);
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
    if(!ViewerEngineManager::Instance()->StartPick(hostName, false, networkID))
    {
        debug1 << "An error occurred when stopping the pick." << endl;
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
// ****************************************************************************

avtVarType 
ViewerPlot::GetVarType()
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    Expression *exp = ParsingExprList::GetExpression(variableName);
    if (exp != NULL)
    {
        // Get the expression type.
        retval = ParsingExprList::GetAVTType(exp->GetType());
    }
    else
    {
        ViewerFileServer *s = ViewerFileServer::Instance();
        avtDatabaseMetaData *md =
            (avtDatabaseMetaData *) s->GetMetaData(std::string(hostName),
                                                   std::string(databaseName));
        if (md != 0)
        {
            // 
            // Get the type for the variable.
            //
            TRY
            {
                retval = md->DetermineVarType(std::string(variableName));
            }
            CATCH(VisItException)
            {
                std::string message("VisIt was unable to determine the variable type for ");
                message += hostName; 
                message += ":";
                message += databaseName;
                message += "'s ";
                message += variableName;
                message += " variable.";
                Error(message.c_str());
                debug1 << "ViewerPlot::GetVarType: Caught an exception!" << endl;
                retval = AVT_UNKNOWN_TYPE;
            }
            ENDTRY
        }
    }

    return retval;
}

// ****************************************************************************
//  Method:  ViewerPlot::CheckCache
//
//  Purpose:
//    Check the cache between the specified frames and clear any frames
//    that need recalculation.
//
//  Arguments:
//    f0         The first inclusive, plot origin frame to check.
//    f1         The last inclusive, plot origin frame to check.
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
// ****************************************************************************

void
ViewerPlot::CheckCache(const int f0, const int f1, const bool force)
{
    //
    // Set the plot attributes for any existing plots and delete any
    // cached actors that need recalculation.
    //
    bool handledFrame = false;
    bool errorOnFrame = false;
    for (int i = f0; i <= f1; i++)
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
                         e.GetMessage().c_str());
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
// ****************************************************************************

void
ViewerPlot::SetOpaqueMeshIsAppropriate(bool val)
{
    const AttributeSubject *atts = NULL;
    int i, frame;
    for (i = 0; i < (frame1 - frame0 + 1) && !atts; ++i)
    {
        if (*plotList[i] != 0 && isMesh)
        {
            atts = plotList[i]->SetOpaqueMeshIsAppropriate(val);
            frame = i;
        }
    }
    //
    // If the plot's atts have changed as a result, 
    //
    if (atts != NULL)
    {
        SetPlotAtts(atts);
        SetClientAttsFromPlot(frame);
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
