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

#include <qmessagebox.h>

#include <ViewerSubject.h>

#include <AbortException.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DataNode.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <PickAttributes.h>
#include <Plot.h>
#include <PlotList.h>
#include <PlotPluginManager.h>
#include <RecursiveExpressionException.h>
#include <SILRestrictionAttributes.h>


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

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtPlot.h>
#include <avtToolInterface.h>
#include <ExprNode.h>

#include <DebugStream.h>

#include <Init.h>

#include <algorithm>
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
    ViewerPlot      *plot;
    ViewerPlotList  *plotList;
    ViewerWindow    *window;
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

//
// Functions for converting ViewerPlotList::PlaybackMode to/from string.
//

static const char *PlaybackMode_strings[] = {
"Looping", "PlayOnce", "Swing"
};

std::string
PlaybackMode_ToString(ViewerPlotList::PlaybackMode t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return PlaybackMode_strings[index];
}

bool
PlaybackMode_FromString(const std::string &s,
    ViewerPlotList::PlaybackMode &val)
{
    val = ViewerPlotList::Looping;
    for(int i = 0; i < 3; ++i)
    {
        if(s == PlaybackMode_strings[i])
        {
            val = (ViewerPlotList::PlaybackMode)i;
            return true;
        }
    }
    return false;
}

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
//    Brad Whitlock, Sun Jan 25 02:43:51 PDT 2004
//    I added support for multiple time sliders and initialized some other
//    new members.
//
//    Brad Whitlock, Tue Apr 6 23:24:12 PST 2004
//    I added nKeyframesWasUserSet.
//
// ****************************************************************************

ViewerPlotList::ViewerPlotList(ViewerWindow *const viewerWindow) : 
    hostDatabaseName(), hostName(), databaseName(), timeSliders()
{
    window           = viewerWindow;
    plots            = 0;
    nPlots           = 0;
    nPlotsAlloc      = 0;

    bgColor[0] = bgColor[1] = bgColor[2] = 1.0;
    fgColor[0] = fgColor[1] = fgColor[2] = 0.0;
    spatialExtentsType = AVT_ORIGINAL_EXTENTS;

    activeTimeSlider = "";
    animationMode = StopMode;
    playbackMode = Looping;

    keyframeMode = false;
    nKeyframesWasUserSet = false;
    nKeyframes = 1;
    pipelineCaching = false;
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
// Method: ViewerPlotList::GetActiveTimeSlider
//
// Purpose: 
//   Gets the name of the plot list's active time slider if there is one.
//
// Returns:    The name of the active time slider or an empty string if there
//             is no active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:19:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

const std::string &
ViewerPlotList::GetActiveTimeSlider() const
{
    return activeTimeSlider;
}

// ****************************************************************************
// Method: ViewerPlotList::HasActiveTimeSlider
//
// Purpose: 
//   Returns whether the plot list has an active time slider.
//
// Returns:    True if there is an active time slider; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:20:34 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::HasActiveTimeSlider() const
{
    return activeTimeSlider != "" && activeTimeSlider != "notset";
}

// ****************************************************************************
// Method: ViewerPlotList::GetTimeSliderInformation
//
// Purpose: 
//   Gets the list of time sliders that should be made available to the client
//   based on the open sources and the current plots in the plot list.
//
// Arguments:
//   activeTS : The index of the active time slider within the tsNames vector.
//   tsNames  : The list of time slider names that the client needs to know
//              about.
//   timeSliderCurrentStates : The list of time slider states that correspond
//                             to the names in tsNames.
//
// Returns:    
//
// Note:       Not all time sliders or correlations are returned; only the
//             ones that matter.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:21:07 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Apr 2 14:43:29 PST 2004
//   I added keyframing support and added conditionally compiled debugging
//   statements.
//
// ****************************************************************************

void
ViewerPlotList::GetTimeSliderInformation(int &activeTS,
    stringVector &tsNames, intVector &timeSliderCurrentStates)
{
    //
    // If we have no database then we have no time sliders to report.
    //
    if(hostDatabaseName == "")
    { 
        activeTS = -1;
        return;
    }

    //
    // Look at the plot list to determine which of the time sliders
    // we should show.
    //
    StringIntMap uniqueTSNames;
    int index;
    for(index = 0; index < nPlots; ++index)
    {
        //
        // Only add a time slider if the plot source has a time slider.
        // Otherwise assume that the plot source was a single time state
        // and add no time slider for it.
        //
        StringIntMap::const_iterator pos =
            timeSliders.find(plots[index].plot->GetSource());
        if(pos != timeSliders.end())
        {
#ifdef debug_GetTimeSliderInformation
            debug3 << "Adding plot " << index << "'s source:"
                   << pos->first.c_str() << endl;
#endif
            uniqueTSNames[pos->first] = pos->second;
        }
    }

    //
    // Go through the list of correlations and add any correlation that is
    // not already in the list if it uses databases that the plots in the plot
    // list use.
    //
    DatabaseCorrelationList *cL = ViewerFileServer::Instance()->
        GetDatabaseCorrelationList();
    for(index = 0; index < cL->GetNumDatabaseCorrelations(); ++index)
    {
        const DatabaseCorrelation &c = cL->operator[](index);
        StringIntMap::const_iterator p = uniqueTSNames.find(c.GetName());
        if(p == uniqueTSNames.end())
        {
            // See if the correlation uses any databases from the plot list.
            bool usesPlotSources = false;
            for(int i = 0; i < nPlots && !usesPlotSources; ++i)
                usesPlotSources = c.UsesDatabase(plots[i].plot->GetSource());

            //
            // If the correlation uses some of the plot sources and it was
            // not already in the time slider list that we're building,
            // add it.
            //
            if(usesPlotSources)
            {
                // The correlation was not already in the list of unique time
                // sliders so we can add it.
                StringIntMap::const_iterator ts = timeSliders.find(c.GetName());
                if(ts != timeSliders.end())
                {
#ifdef debug_GetTimeSliderInformation
                    debug3 << "Added correlation " << ts->first << endl;
#endif
                    uniqueTSNames[ts->first] = ts->second;
                }
            }
        }
    }

    //
    // If we have an active time slider then make sure that it is in the list.
    //
    if(HasActiveTimeSlider())
    {
        StringIntMap::const_iterator pos = uniqueTSNames.find(activeTimeSlider);
        StringIntMap::const_iterator pos2 = timeSliders.find(activeTimeSlider);
        if(pos == uniqueTSNames.end() && pos2 != timeSliders.end())
        {
#ifdef debug_GetTimeSliderInformation
            debug3 << "Added active time slider: " << pos2->first << endl;
#endif
            uniqueTSNames[pos2->first] = pos2->second;                
        }
    }

    //
    // If there are no time sliders from the plot list or keyframing, then
    // try adding a time slider for the active source. Note that the time
    // slider will only be added for the active source if it has a time slider,
    // which means that single time state databases will never add a time
    // slider.
    //
    if(uniqueTSNames.size() == 0)
    {
        StringIntMap::const_iterator pos = timeSliders.find(hostDatabaseName);
        if(pos != timeSliders.end())
        {
#ifdef debug_GetTimeSliderInformation
            debug3 << "Added active source: " << pos->first << endl;
#endif
            uniqueTSNames[pos->first] = pos->second;
        }
    }

    //
    // If we're in keyframing mode, add a time slider for the animation.
    //
    if(GetKeyframeMode())
        uniqueTSNames[KF_TIME_SLIDER] = timeSliders[KF_TIME_SLIDER];

    //
    // Figure out the active time slider index.
    //
    index = 0;
    activeTS = -1;
    for(StringIntMap::const_iterator pos = uniqueTSNames.begin();
        pos != uniqueTSNames.end(); ++pos, ++index)
    {
        if(pos->first == activeTimeSlider)
            activeTS = index;
        tsNames.push_back(pos->first);
        timeSliderCurrentStates.push_back(pos->second);
    }

    // Print out the time slider list.
#ifdef debug_GetTimeSliderInformation
    debug3 << "*******\nActive time slider index: " << activeTS << endl;
    for(index = 0; index < tsNames.size(); ++index)
    {
        debug3 << "    " << tsNames[index]
               << ", state = " << timeSliderCurrentStates[index] << endl;
    }
#endif

    // Consistency check
    if(activeTS == -1 && uniqueTSNames.size() > 0)
    {
        debug3 << "When trying to determine the active time slider for the "
                  "plot list, the active time slider was not in the list "
                  "but we are sending a time slider list. Set the active "
                  "time slider index to 0" << endl;
        activeTS = 0;
    }
}

// ****************************************************************************
// Method: ViewerPlotList::SetActiveTimeSlider
//
// Purpose: 
//   Sets the plot list's active time slider.
//
// Arguments:
//   newTimeSlider : The name of the time slider to use.
//
// Note:       Issues an error message if an invalid time slider is used.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:23:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetActiveTimeSlider(const std::string &newTimeSlider)
{
    // Figure out the list of time sliders that we can show.
    StringIntMap::const_iterator pos = timeSliders.find(newTimeSlider);
    if(pos != timeSliders.end())
        activeTimeSlider = newTimeSlider;
    else
    {
        char err[200];
        SNPRINTF(err, 200, "There is no time slider called %s. VisIt cannot "
                 "use it as the active time slider.", newTimeSlider.c_str());
        Error(err);
    }    
}

// ****************************************************************************
// Method: ViewerPlotList::CreateTimeSlider
//
// Purpose: 
//   Creates a new time slider and sets its initial state.
//
// Arguments:
//   newTimeSlider : The name of the new time slider.
//   state         : The initial state of the new time slider.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:24:24 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::CreateTimeSlider(const std::string &newTimeSlider, int state)
{
    timeSliders[newTimeSlider] = state;
}

// ****************************************************************************
// Method: ViewerPlotList::ValidateTimeSlider
//
// Purpose: 
//   Makes sure that we don't have a time slider if it is not appropriate to
//   have one.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 19 08:59:42 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::ValidateTimeSlider()
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();

    if(HasActiveTimeSlider())
    {
        // There is an active time slider.
        // If the active source is an MT database and it is not used in the
        // active time slider's correlation, change the active time slider
        // so we use the active source.
        DatabaseCorrelation *tsCorrelation = cL->
            FindCorrelation(activeTimeSlider);
        if(tsCorrelation != 0)
        {
            //
            // Get a list of the MT sources between the plot list and the
            // active source.
            //
            int MTcount = 0;
            bool hostDBIsMT = false;
            if(cL->FindCorrelation(hostDatabaseName) != 0)
            {
                ++MTcount;
                hostDBIsMT = true;
            }

            for(int i = 0; i < nPlots; ++i)
            {
                if(cL->FindCorrelation(plots[i].plot->GetSource()) != 0)
                    ++MTcount;
            }

            if(MTcount == 0)
            {
                debug1 << "There were no MT databases so there can't be "
                          "an active time slider." << endl;
                activeTimeSlider = "";
            }
            else
            {
                // The active source is MT
                if(hostDBIsMT)
                {
                    if(!tsCorrelation->UsesDatabase(hostDatabaseName))
                    {
                        debug1 << "The plot list was using a time slider "
                                  "that was not compatible with the active "
                                  "source: "
                               << hostDatabaseName.c_str()
                               << " so use the active source for the new "
                                  "time slider." << endl;
                        activeTimeSlider = hostDatabaseName;
                    }
                }
                // else do nothing because even though the hostDB is ST
                // there are other plots that are MT so don't change the
                // time slider.
            }
        }
    }
    else
    {
        // If there is no time slider but our active source is MT then
        // set the time slider to be the active source.
        DatabaseCorrelation *srcCorrelation = cL->
            FindCorrelation(hostDatabaseName);
        if(srcCorrelation != 0)
        {
            if(TimeSliderExists(hostDatabaseName))
                activeTimeSlider = hostDatabaseName;
            else
                CreateTimeSlider(hostDatabaseName, 0);
        }
    }
}

// ****************************************************************************
// Method: ViewerPlotList::ResizeTimeSliders
//
// Purpose: 
//   Resizes the time sliders and the actor caches for all plots
//
// Arguments:
//   sliders    : The sliders that need to be checked against their database
//                correlations for the proper length.
//   clearCache : If we end up having to resize the actor caches as a result
//                of checking their length against their database's number
//                of states, this flag indicates whether the actors should
//                be cleared out.
//
// Returns:    A bit field. bit 0 is true when the time slider's time state
//             had to be clipped. bit 1 is true if any plot's actors were
//             cleared.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 27 10:23:03 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlotList::ResizeTimeSliders(const stringVector &sliders, bool clearCache)
{
    const char *mName = "ViewerPlotList::ResizeTimeSliders: ";

    //
    // Make sure that the time sliders' time state values are no longer than
    // the correlations will allow.
    //
    bool tsSizeChanged = false;
    DatabaseCorrelationList *cL = ViewerFileServer::Instance()->
        GetDatabaseCorrelationList();
    for(int slider = 0; slider < sliders.size(); ++slider)
    {
        const std::string &tsName = sliders[slider];
        DatabaseCorrelation *correlation = cL->FindCorrelation(tsName);
        if(correlation != 0 && TimeSliderExists(tsName))
        {
            int state = 0, nStates = 1;
            GetTimeSliderStates(tsName, state, nStates);
            if(state >= correlation->GetNumStates())
            {
                timeSliders[tsName] = correlation->GetNumStates() - 1;
                tsSizeChanged |= (tsName == activeTimeSlider);
                debug4 << mName << "Shortened time slider " << tsName.c_str()
                       << " to " << timeSliders[tsName] << " states." << endl;
            }
        }
    }

    // Resize all of the plots.
    bool actorsCleared = false;
    for(int i = 0; i < nPlots; ++i)
    {
        // Determine if the plot's source has anything to do with the
        // time sliders that are changing.
        bool relatedSource = false;
        for(int slider = 0; slider < sliders.size(); ++slider)
        {
             const std::string &tsName = sliders[slider];
             DatabaseCorrelation *correlation = cL->FindCorrelation(tsName);
             if(correlation != 0)
             {
                 std::string plotSource(plots[i].plot->GetSource());
                 relatedSource |= 
                     correlation->UsesDatabase(plotSource);
             }
        }

        if(relatedSource)
        {
            actorsCleared |= clearCache;
            debug4 << mName << "Plot " << i
                   << "'s source is related to the time sliders that "
                   << "changed. clearCache = "
                   << (clearCache?"true":"false") << endl;
            plots[i].plot->UpdateCacheSize(GetKeyframeMode(), clearCache);
        }
        else
        {
            debug4 << mName << "Plot " << i
                   << "'s source was not related to the time sliders that "
                   << "changed." << endl;
        }
    }

    int flag0 = tsSizeChanged ? 1 : 0;
    int flag1 = actorsCleared ? 1 : 0;
    int retval = flag0 | (flag1 << 1);
    return retval;
}

// ****************************************************************************
// Method: ViewerPlotList::GetTimeSliderStates
//
// Purpose: 
//   Returns the current state and total number of states for the specified
//   time slider.
//
// Arguments:
//   ts      : The name of the time slider for which to get the states.
//   state   : The return value for the time slider's number of states.
//   nStates : The return value for the total number of states.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:26:10 PST 2004
//
// Modifications:
//   Brad Whitlock, Fri Apr 2 14:41:40 PST 2004
//   I added keyframing support.
//
// ****************************************************************************

void
ViewerPlotList::GetTimeSliderStates(const std::string &ts, int &state,
    int &nStates) const
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();

    //
    // Get the current time slider state.
    //
    StringIntMap::const_iterator pos = timeSliders.find(ts);
    if(pos != timeSliders.end())
        state = pos->second;
    else
       state = 0;

    //
    // Set the number of frames.
    //
    if(GetKeyframeMode() && ts == KF_TIME_SLIDER)
    {
        nStates = nKeyframes;
    }
    else
    {
        DatabaseCorrelation *correlation = cL->FindCorrelation(ts);

        if(correlation != 0)
        {
            nStates = correlation->GetNumStates();
        }
        else
        {
            state = 0;
            nStates = 1;
        }
    }
}

// ****************************************************************************
// Method: ViewerPlotList::TimeSliderExists
//
// Purpose: 
//   Returns a boolean indicating if the named time slider exists.
//
// Arguments:
//   ts      : The name of the time slider.
//
// Programmer: Eric Brugger
// Creation:   April 16, 2004
//
// Modifications:
//   Brad Whitlock, Mon Apr 19 08:27:20 PDT 2004
//   Removed unnecessary coding.
//
// ****************************************************************************

bool
ViewerPlotList::TimeSliderExists(const std::string &ts) const
{
    return timeSliders.find(ts) != timeSliders.end();
}

// ****************************************************************************
// Method: ViewerPlotList::SetNextState
//
// Purpose: 
//   Sets the active time slider's state to the specified nextState but also 
//   takes into account the animation mode so it does the right thing.
//
// Arguments:
//   nextState : The next state for the active time slider.
//   boundary  : The boundary state at which the animation will change
//               direction.
//
// Note:       The next state can be the previous state to make the animation
//             go in reverse.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:39:34 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetNextState(int nextState, int boundary)
{
    switch(playbackMode)
    {
    case Looping:
        // Move to the next frame.
        SetTimeSliderState(nextState);
        break;
    case PlayOnce:
        // If we're playing then make sure we stop on the last frame.
        if(animationMode == PlayMode)
        {
            if(nextState == boundary)
                animationMode = StopMode;
            else
            {
                SetTimeSliderState(nextState);
            }
        }
        else if(animationMode == ReversePlayMode)
        {
            if(nextState == boundary)
                animationMode = StopMode;
            else
            {
                SetTimeSliderState(nextState);
            }
        }
        else
        {
            SetTimeSliderState(nextState);
        }
        break;
    case Swing:
        // If we're playing then make sure that we reverse the play direction
        // on the last frame.
        if(animationMode == PlayMode)
        {
            if(nextState == boundary)
                animationMode = ReversePlayMode;
            else
            {
                SetTimeSliderState(nextState);
            }
        }
        else if(animationMode == ReversePlayMode)
        {
            if(nextState == boundary)
                animationMode = PlayMode;
            else
            {
                SetTimeSliderState(nextState);
            }
        }
        else
        {
            SetTimeSliderState(nextState);
        }
    }
}

// ****************************************************************************
// Method: ViewerPlotList::ForwardStep
//
// Purpose: 
//   Advances the active time slider one time state.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:42:28 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::ForwardStep()
{
    int timeSliderCurrentState, timeSliderNStates;

    //
    // Get the current state and number of states for the active time slider.
    //
    GetTimeSliderStates(activeTimeSlider, timeSliderCurrentState, timeSliderNStates);
    if(timeSliderNStates < 2)
        return;

    int nextState = (timeSliderCurrentState + 1) % timeSliderNStates;
    SetNextState(nextState, 0);
}

// ****************************************************************************
// Method: ViewerPlotList::BackwardStep
//
// Purpose: 
//   Moves the active time slider one time state in reverse.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:42:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::BackwardStep()
{
    int timeSliderCurrentState, timeSliderNStates;

    //
    // Get the current state and number of states for the active time slider.
    //
    GetTimeSliderStates(activeTimeSlider, timeSliderCurrentState, timeSliderNStates);
    if(timeSliderNStates < 2)
        return;

    int nextState = (timeSliderCurrentState + timeSliderNStates - 1) % timeSliderNStates;
    SetNextState(nextState, timeSliderNStates - 1);
}

// ****************************************************************************
// Method: ViewerPlotList::SetTimeSliderState
//
// Purpose: 
//   Sets the state for the active time slider and any time sliders having
//   trivial database correlations that can be set by the correlation for
//   the active time slider.
//
// Arguments:
//   state : The new state for the active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:43:18 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetTimeSliderState(int state)
{
    //
    // Get the number of states for the active time slider so, if we have one, 
    // we can set the time for it.
    //
    int timeSliderCurrentState, timeSliderNStates;
    GetTimeSliderStates(activeTimeSlider, timeSliderCurrentState, timeSliderNStates);

    if(state >= 0 && state < timeSliderNStates)
    {
        if (state != timeSliderCurrentState)
            window->ClearPickPoints();

        //
        // Get the correlation for the active time slider.
        //
        if(timeSliderNStates > 1)
        {
            // Set the time slider state.
            timeSliders[activeTimeSlider] = state;

            DatabaseCorrelationList *cL = ViewerFileServer::Instance()->GetDatabaseCorrelationList();
            DatabaseCorrelation *correlation = cL->FindCorrelation(activeTimeSlider);
            if(correlation)
            {
                //
                // We set the state for the active slider and we have the
                // correlation for the active time slider. We also need to update
                // the state in the time sliders that correspond to sources that
                // are present in the correlation so when we change to those time 
                // sliders, after having set the time for the correlated time
                // slider, they have the right states.
                //
                int i;
                const stringVector &correlationDBs = correlation->GetDatabaseNames();
                for(i = 0; i < correlationDBs.size(); ++i)
                {
                    StringIntMap::iterator ts = timeSliders.find(correlationDBs[i]);
                    if(ts != timeSliders.end())
                    {
                        ts->second = correlation->GetCorrelatedTimeState(
                            correlationDBs[i], state);
                    }
                }
            }
        }

        // Set the merge view limits mode.
        window->SetMergeViewLimits(true);

        //
        // Now that the time sliders have been updated, update the state in
        // the plots so they are in the right state.
        //
        UpdatePlotStates();

        //
        // Update the frame, which we mean to be the image on the screen.
        //
        UpdateFrame(false);
    }
}

// ****************************************************************************
// Method: ViewerPlotList::UpdatePlotStates
//
// Purpose: 
//   Updates the plot states for all plots in the plot list so they are up
//   to date with respect to the active time slider.
//
// Returns:    True if any plots had their states updated; false otherwise.
//
// Note:       DON'T EVER MAKE THIS ROUTINE PUBLIC!
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:44:32 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::UpdatePlotStates()
{
    bool different = false;

    //
    // Update each of the plots' state so the image on the screen will
    // be right the next time we go to update it.
    //
    for(int i = 0; i < nPlots; ++i)
        different |= UpdateSinglePlotState(plots[i].plot);

    return different;
}

// ****************************************************************************
// Method: ViewerPlotList::UpdateSinglePlotState
//
// Purpose: 
//   Updates the specified plot's state with respect to the active time slider.
//
// Arguments:
//   plot : The plot whose state we want to update.
//
// Returns:    True if the plot's state was changed; false otherwise.
//
// Note:       DON'T EVER MAKE THIS ROUTINE PUBLIC!
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:45:23 PST 2004
//
// Modifications:
//   Brad Whitlock, Thu Apr 8 15:44:16 PST 2004
//   I added support for keyframing.
//
// ****************************************************************************

bool
ViewerPlotList::UpdateSinglePlotState(ViewerPlot *plot)
{
    bool different = false;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
    DatabaseCorrelation *tsCorrelation = 0;

    //
    // Try and find a correlation for the active time slider if there is
    // an active time slider.
    //
    if(GetKeyframeMode())
    {
        int tsState = 0, tsNStates = 0;
        GetTimeSliderStates(GetActiveTimeSlider(), tsState, tsNStates);
        if(GetActiveTimeSlider() == KF_TIME_SLIDER)
        {
            if(plot->GetCacheIndex() != tsState)
            {
                different = true;

                if(!pipelineCaching)
                    plot->ClearCurrentActor();

                //
                // We changed time states using the keyframe time slider. Change
                // the cache index for the plot to be the same as the keyframe
                // time slider's state.
                //
                plot->SetCacheIndex(tsState);
            }
        }
        else
        {
            int kfIndex = 0, kfNIndices = 0;
            GetTimeSliderStates(KF_TIME_SLIDER, kfIndex, kfNIndices);

            //
            // To get here, the user moved a time slider for a correlation
            // but not for the keyframe time slider. This means that we
            // should take the state for the active time slider and 
            // figure out the plot's state at that state using the
            // correlation and then set a database keyframe using that
            // computed state at the current keyframe index.
            //
            tsCorrelation = cL->FindCorrelation(GetActiveTimeSlider());
            if(tsCorrelation != 0 &&
               tsCorrelation->UsesDatabase(plot->GetSource()))
            {
                int plotState = tsCorrelation->GetCorrelatedTimeState(
                    plot->GetSource(), tsState);
                if(plotState != -1)
                {
                    different = true;
                    plot->AddDatabaseKeyframe(kfIndex, plotState);
                    debug3 << "Added database keyframe. dbkeyframes["
                           << kfIndex << "] = " << plotState << endl;
                }
            }
        }
    }
    else
    {
        if(HasActiveTimeSlider())
            tsCorrelation = cL->FindCorrelation(activeTimeSlider);

        //
        // Set the plot's state according to the time slider.
        //
        DatabaseCorrelation *correlation = 0;
        int plotState = 0, tsState = 0, tsNStates = 0;
        std::string plotSource(plot->GetSource());

        if(tsCorrelation != 0 && tsCorrelation->UsesDatabase(plotSource))
        {
            GetTimeSliderStates(activeTimeSlider, tsState, tsNStates);
            plotState = tsCorrelation->GetCorrelatedTimeState(
                    plotSource, tsState);
        }
        else if((correlation = cL->FindCorrelation(plotSource)) != 0)
        {
            //
            // The time slider did not use the plot's source. Try getting
            // the time state from the plot source's corralation and time
            // slider.
            //
            int tsState = 0, tsNStates = 0;
            GetTimeSliderStates(plotSource, tsState, tsNStates);
            if(tsState >= 0)
            {
                plotState = correlation->GetCorrelatedTimeState(
                    plotSource, tsState);
            }
        }

        // Set the plot's state.
        if(plotState >= 0)
        {
            different = (plot->GetCacheIndex() != plotState);

            //
            // If we're changing to a different plot state and we're not
            // pipeline caching then clear the plot's actor before setting
            // the new state.
            //
            if (different && !pipelineCaching)
                plot->ClearCurrentActor();

            // Set the new state.
            plot->SetCacheIndex(plotState);
        }
    }

    return different;
}

// ****************************************************************************
// Method: ViewerPlotList::UpdateFrame
//
// Purpose: 
//   Generates plots as required and updates the vis window.
//
// Arguments:
//   updatePlotStates : Whether the plots need to have their states updated
//                      before trying to update the plots in the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:48:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::UpdateFrame(bool updatePlotStates)
{
    //
    // We might need to update the window because we had to change
    // the plot states. We don't know until we call UpdatePlotStates.
    //
    bool updateTheWindow = updatePlotStates ? UpdatePlotStates() : false;

    //
    // If we're in keyframing mode, update the plot attributes in 
    // the client. Also update the view in any windows that are
    // in camera mode.
    //
    if(GetKeyframeMode())
    {
        UpdatePlotAtts();
        window->UpdateCameraView();
    }

    //
    // If we have actors for the current state for each plot then just
    // update the windows.
    //
    if (ArePlotsUpToDate())
    {
        updateTheWindow = true;
    }
    else
    {
        //
        // The UpdatePlots method sometimes operates in
        // threaded mode. If no additional threads were spawned,
        // we need to update the windows.
        //
        bool animating = ((animationMode == PlayMode) ||
                          (animationMode == ReversePlayMode));
        if(UpdatePlots(animating))
            updateTheWindow = true;
    }

    if(updateTheWindow)
    {
        UpdateWindow(true);

        //
        // Update the plot list so that the color changes on the plots.
        //
        UpdatePlotList();
    }
}

// ****************************************************************************
// Method: ViewerPlotList::SetAnimationMode
//
// Purpose: 
//   Sets the plot list's animation mode.
//
// Arguments:
//   m : The new animation mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:49:57 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetAnimationMode(ViewerPlotList::AnimationMode m)
{
    animationMode = m;
}

// ****************************************************************************
// Method: ViewerPlotList::GetAnimationMode
//
// Purpose: 
//   Gets the plot list's animation mode.
//
// Returns:    The plot list's animation mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:50:30 PST 2004
//
// Modifications:
//   
// ****************************************************************************

ViewerPlotList::AnimationMode
ViewerPlotList::GetAnimationMode() const
{
    return animationMode;
}

// ****************************************************************************
// Method: ViewerPlotList::SetPlaybackMode
//
// Purpose: 
//   Sets the plot list's playback mode.
//
// Arguments:
//   m : The new playback mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:50:59 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetPlaybackMode(ViewerPlotList::PlaybackMode m)
{
    playbackMode = m;
}

// ****************************************************************************
// Method: ViewerPlotList::GetPlaybackMode
//
// Purpose: 
//   Gets the plot list's playback mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:51:20 PST 2004
//
// Modifications:
//   
// ****************************************************************************

ViewerPlotList::PlaybackMode
ViewerPlotList::GetPlaybackMode() const
{
    return playbackMode;
}

// ****************************************************************************
// Method: ViewerPlotList::AlterTimeSlider
//
// Purpose: 
//   Updates the time slider if the database correlation changed.
//
// Arguments:
//   ts : The name of the time slider to change.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:54:15 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::AlterTimeSlider(const std::string &ts)
{
    // Make sure that the time slider's state is within the bounds for its
    // correlation. If the time slider's correlation has multiple databases,
    // make sure those time sliders are also within range.
    
    DatabaseCorrelationList *cL = ViewerFileServer::Instance()->GetDatabaseCorrelationList();
    DatabaseCorrelation *correlation = cL->FindCorrelation(ts);
    if(correlation)
    {
        StringIntMap::iterator pos = timeSliders.find(ts);
        if(pos != timeSliders.end())
        {
            if(pos->second >= correlation->GetNumStates())
            {
                pos->second = correlation->GetNumStates() - 1;
                ViewerWindowManager::Instance()->UpdateWindowInformation(
                    WINDOWINFO_TIMESLIDERS, window->GetWindowId());
            }
        }
    }

    // 
    // Update the frame because we had to change the state for the active
    // time slider because it was beyond the length of the correlation.
    //
    if(activeTimeSlider == ts)
        UpdateFrame();    
}

// ****************************************************************************
// Method: ViewerPlotList::DeleteTimeSlider
//
// Purpose: 
//   Deletes the specified time slider.
//
// Arguments:
//   ts     : The name of the time slider to delete.
//   update : Whether the client should be notified.
//
// Returns:    True if a time slider was deleted; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 12:49:14 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::DeleteTimeSlider(const std::string &ts, bool update)
{
    // See if we have such a time slider.
    bool retval = false;
    StringIntMap::iterator pos = timeSliders.find(ts);
    if(pos != timeSliders.end())
    {
        // Remove the time slider from the time slider map.
        timeSliders.erase(pos);

        // If the time slider that we deleted was the active time slider,
        // find the most suitable correlation (without new additions) to be
        // the new time slider.
        if(activeTimeSlider == ts)
        {
            DatabaseCorrelation *c = GetMostSuitableCorrelation(
                hostDatabaseName, false);
            if(c)
            {
                activeTimeSlider = c->GetName();
            }
            else
                activeTimeSlider = "";
    
            // Update the frame using the new active time slider.
            UpdateFrame();
        }

        // We deleted a time slider so we need to update the client.
        if(update)
        {
            ViewerWindowManager::Instance()->UpdateWindowInformation(
                WINDOWINFO_TIMESLIDERS, window->GetWindowId());
        }

        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerPlotList::AskForCorrelationPermission
//
// Purpose: 
//   Asks the user whether or not to allow creation of a database correlation.
//
// Arguments:
//   dbs : The databases involved in the new correlation.
//
// Returns:    True if the user wants the correlation; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 3 10:30:07 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Apr 13 23:08:29 PST 2004
//   Added support for making sure that we don't keep asking the user to
//   create a correlation if they have turned it down before.
//
// ****************************************************************************

bool
ViewerPlotList::AskForCorrelationPermission(const stringVector &dbs) const
{
    const char *mName = "ViewerPlotList::AskForCorrelationPermission: ";
    ViewerFileServer *fs = ViewerFileServer::Instance();
    bool permission;

    if(avtCallback::GetNowinMode())
    {
        debug3 << mName << "Don't need to ask for permission. "
               << "We're running -nowin." << endl;
        permission = true;
    }
    else if(fs->PreviouslyDeclinedCorrelationCreation(dbs))
    {
        permission = false;
    }
    else
    {
        // Pop up a Qt dialog to ask the user whether or not to correlate
        // the specified databases.
        QString text("Would you like to create a database correlation "
                     "for the following databases?\n");
        QString fileStr;
        for(int i = 0; i < dbs.size(); ++i)
            fileStr += (QString(dbs[i].c_str()) + QString("\n"));
        text += fileStr;

        debug3 << "Asking for permission to create correlation. Prompt="
               << text.latin1() << endl;

        viewerSubject->BlockSocketSignals(true);
        permission = (QMessageBox::information(0, "Correlate databases?",
            text, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton) ==
            QMessageBox::Yes);

        // The user declined to create the correlation so record that we
        // should not try to correlate these databases in the future.
        if(!permission)
        {
            QString msg("You chose not to create a database correlation. "
                        "VisIt will not prompt you again to create a "
                        "database correlation for:\n");
            msg += fileStr;
            fs->DeclineCorrelationCreation(dbs);
            Warning(msg.latin1());
        }

        viewerSubject->BlockSocketSignals(false);
    }

    return permission;
}

// ****************************************************************************
// Method: ViewerPlotList::AllowAutomaticCorrelation
//
// Purpose: 
//   Determines whether automatic correlation creation is allowed.
//
// Arguments:
//   dbs : The databases that would be involved in a new correlation.
//
// Returns:    True if correlation will be allowed; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 3 10:28:24 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::AllowAutomaticCorrelation(const stringVector &dbs) const
{
    const char *mName = "ViewerPlotList::AllowAutomaticCorrelation: ";

    //
    // Get the correlation preferences from the correlation list.
    // 
    DatabaseCorrelationList *cL = ViewerFileServer::Instance()->
        GetDatabaseCorrelationList();
    DatabaseCorrelationList::WhenToCorrelate when = 
        cL->GetWhenToCorrelate();
    bool needPermission = avtCallback::GetNowinMode() ? false :
        cL->GetNeedPermission();
    bool allowCorrelation;

    if(when == DatabaseCorrelationList::CorrelateAlways)
    {
        debug3 << mName << "Correlate always" << endl;
        if(needPermission)
            allowCorrelation = AskForCorrelationPermission(dbs);
        else
            allowCorrelation = true;
    }
    else if(when == DatabaseCorrelationList::CorrelateNever)
    {
        debug3 << mName << "Correlate never" << endl;
        allowCorrelation = false;
    }
    else if(when == DatabaseCorrelationList::CorrelateOnlyIfSameLength)
    {
        // Check to see if all of the correlations for the dbs in the list
        // are the same length. If they are then we will allow automatic
        // correlation unless we need permission.
        bool sameLength = true;
        int len = -1;
        for(int i = 0; i < dbs.size() && sameLength; ++i)
        {
            DatabaseCorrelation *c = cL->FindCorrelation(dbs[i]);
            if(c)
            {
                if(len == -1)
                    len = c->GetNumStates();
                else
                    sameLength &= (c->GetNumStates() == len);
            }
            else
                sameLength = false;
        }

        debug3 << mName << "Correlate if same length. sameLength="
               << (sameLength ? "true" : "false") << endl;

        if(sameLength)
        {
            if(needPermission)
                allowCorrelation = AskForCorrelationPermission(dbs);
            else
                allowCorrelation = true;
        }
        else
            allowCorrelation = false;
    }
    
    return allowCorrelation;
}

// ****************************************************************************
// Method: ViewerPlotList::GetMostSuitableCorrelation
//
// Purpose: 
//   Returns the most suitable database correlation for the given plotSource
//   along with the sources from all plots in the plot list. If no suitable
//   correlation is found then create one and make it the active time slider.
//
// Arguments:
//   source       : A source that must absolutely be used in the correlation.
//   allowChanges : Whether we're allowed to add new correlations if we
//                  don't find exactly the one that we want.
//
// Returns:    A pointer to the most suitable correlation.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 3 09:17:48 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

DatabaseCorrelation *
ViewerPlotList::GetMostSuitableCorrelation(const std::string &source,
    bool allowChanges)
{
    DatabaseCorrelation *correlation = 0;
    const char *mName = "ViewerPlotList::GetMostSuitableCorrelation: ";
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();

    //
    // Add all of the plot databases that have more than one time step to dbs.
    //
    int i;
    stringVector dbs1, dbs;
    dbs1.push_back(source);
    for(i = 0; i < nPlots; ++i)
    {
        std::string pSource(plots[i].plot->GetSource());
        if(std::find(dbs1.begin(), dbs1.end(), pSource) == dbs1.end())
            dbs1.push_back(pSource);
    }
    for(i = 0; i < dbs1.size(); ++i)
    {
        if(cL->FindCorrelation(dbs1[i]) != 0)
            dbs.push_back(dbs1[i]);
    }

    //
    // If we don't have any databases then return the correlation for the
    // active time slider, if there is one. If we have more than one database
    // and we allow automatic database correlation, then find or create the
    // most suitable correlation for the dbs in the list. Otherwise, return
    // the correlation for the plot source.
    //
    if(dbs.size() == 0)
    {
        debug3 << mName << "No plots have time-varying databases.";
        if(HasActiveTimeSlider())
        {
            debug3 << " Returning correlation for the active time slider: "
                   << activeTimeSlider.c_str() << endl;
            correlation = cL->FindCorrelation(activeTimeSlider);
        }
        else
            debug3 << " No correlation!" << endl;
    }
    else if(dbs.size() > 1)
    {
        debug3 << mName << "Need to "
                  "find a suitable correlation for:" << endl;
        StringIntMap scores;
        for(i = 0; i < dbs.size(); ++i)
        {
            scores[dbs[i]] = 0;
            debug3 << "\t" << dbs[i].c_str() << endl;
        }

        for(i = 0; i < cL->GetNumDatabaseCorrelations(); ++i)
        {
            const DatabaseCorrelation &c = cL->GetDatabaseCorrelation(i);
            for(int j = 0; j < dbs.size(); ++j)
            {
                if(c.UsesDatabase(dbs[j]))
                    ++scores[c.GetName()];
            }
        }
#if 0
        debug3 << mName << " Scores: " << endl;
        for(StringIntMap::const_iterator sIt = scores.begin(); sIt != scores.end(); ++sIt)
            debug3 << "\t" << sIt->first << ", score= " << sIt->second << endl;

#endif
        // Suppose we have a plot from R and an open database S. The above
        // code would say that R and S have equal scores. Look for any
        // databases with a score of dbs.size() and then go down to the
        // correlation with the next highest score. If the score is 1 then
        // we need to create a new correlation and time slider pair. If the
        // score > 1 then we must make sure that all dbs in the list get
        // added to the selected correlation if they are not already in it.
        std::string correlationName;
        int score = 0;
        for(int desiredScore = dbs.size(); desiredScore > 1 && score == 0;
            --desiredScore)
        {
            for(StringIntMap::const_iterator pos = scores.begin();
                pos != scores.end(); ++pos)
            {
                if(pos->second == desiredScore)
                {
                    score = desiredScore;
                    correlationName = pos->first;
                    break;
                }
            }
        }

        if(score < 2)
        {
            debug3 << mName << "We did not"
                      "find a correlation that contained all of the databases"
                      "that we wanted." << endl;

            if(allowChanges && AllowAutomaticCorrelation(dbs))
            {
                // We did not find the score that we wanted, in fact the only
                // correlations that match are the trivial correlations but
                // since we can't modify those, create a new correlation that
                // uses all of the dbs. Also create a new time slider and make
                // it be the active time slider.

                // Create a new database correlation.
                std::string newName(fs->CreateNewCorrelationName());
                correlation = fs->CreateDatabaseCorrelation(newName, dbs,
                    cL->GetDefaultCorrelationMethod());
                if(correlation)
                {
                    // Add the new correlation to the correlation list.
                    cL->AddDatabaseCorrelation(*correlation);
                    cL->Notify();
                    delete correlation; 
                    correlation = cL->FindCorrelation(newName);

                    debug3 << mName << "Created a new correlation called: "
                           << newName.c_str() << endl;

                    //
                    // We have an active time slider that has a time state.
                    // Usually, the active time slider would be the database
                    // for which we want to make a plot. Get the correlated time
                    // state for the active time slider's state and then use the
                    // new correlation to try and find what state we should make
                    // active in the new time slider.
                    //
                    int activeTSState = timeSliders[activeTimeSlider];
                    DatabaseCorrelation *tsCorrelation = cL->FindCorrelation(
                        activeTimeSlider);
                    int state = 0;
                    if(tsCorrelation)
                    {
                        int oldState = tsCorrelation->GetCorrelatedTimeState(activeTimeSlider,
                            activeTSState);
                        for(int index = 0; index < correlation->GetNumStates(); ++index)
                        {
                            int s = correlation->GetCorrelatedTimeState(activeTimeSlider, index);
                            if(s == oldState)
                            {
                                state = index;
                                break;
                            }
                        }
                    }

                    // Create a new time slider for the correlation.
                    CreateTimeSlider(newName, state);
                    SetActiveTimeSlider(newName);
                    ViewerWindowManager::Instance()->UpdateWindowInformation(
                        WINDOWINFO_TIMESLIDERS, window->GetWindowId());
                }
            }
            else
            {
                debug3 << mName << "Automatic correlation was not allowed. "
                       << "Using correlation for " << source.c_str() << endl;
                // The user did not let us automatically correlate. Use
                // the correlation for the plot source.
                correlation = cL->FindCorrelation(source);
            }
        }
        else
        {
            if(score == dbs.size())
            {
                debug3 << mName << "Found a correlation that contains all "
                       << "plot databases. Using " << correlationName.c_str()
                       << endl;

                // We already had a correlation that matched all of the
                // sources so we can use it.
                correlation = cL->FindCorrelation(correlationName);

                //
                // If the correlation is not the active time slider, change
                // active time sliders.
                //
                if(allowChanges && correlationName != activeTimeSlider)
                {
                    SetActiveTimeSlider(correlationName);
                    ViewerWindowManager::Instance()->UpdateWindowInformation(
                        WINDOWINFO_TIMESLIDERS, window->GetWindowId());
                }
            }
            else if(allowChanges && AllowAutomaticCorrelation(dbs))
            {
                debug3 << mName << "Automatic correlation was allowed and "
                       << "we are modifying correlation "
                       << correlationName.c_str() << " (score=" << score
                       << ") to meet our needs."
                       << endl;

                //
                // We have the name of a correlation at this point. Add all
                // of the databases that are missing from it and make it
                // the active time slider if it is not the active time slider.
                //
                bool alteredCorrelation = false;
                correlation = cL->FindCorrelation(correlationName);
                for(i = 0; i < dbs.size(); ++i)
                {
                    if(!correlation->UsesDatabase(dbs[i]))
                    {
                        std::string h, d;
                        ViewerFileServer::SplitHostDatabase(dbs[i], h, d);
                        const avtDatabaseMetaData *md = fs->GetMetaData(h, d);
                        if(md && md->GetNumStates() > 1)
                        {
                            alteredCorrelation = true;
                            correlation->AddDatabase(dbs[i], md->GetNumStates(),
                                md->GetTimes(), md->GetCycles());
                        }
                    }
                }

                //
                // If we had to add a database to the correlation, update
                // the client.
                //
                if(alteredCorrelation)
                {
                    debug3 << mName << "Correlation " << correlationName.c_str()
                           << " after being modified: " << endl
                           << *correlation << endl;
                    cL->Notify();
                }

                //
                // If the correlation is not the active time slider, change
                // active time sliders.
                //
                if(correlation->GetName() != activeTimeSlider)
                {
                    SetActiveTimeSlider(correlation->GetName());
                    ViewerWindowManager::Instance()->UpdateWindowInformation(
                        WINDOWINFO_TIMESLIDERS, window->GetWindowId());
                }
            }
            else
            {
                debug3 << mName << "Automatic correlation was not allowed. "
                       << "Using correlation for " << source.c_str() << endl;
                // The user did not let us automatically correlate. Use
                // the correlation for the plot source.
                correlation = cL->FindCorrelation(source);
            }
        }
    }
    else
    {
        debug3 << mName << "There was only one database. Use a trivial "
               << "correlation for " << source.c_str() << endl;
        correlation = cL->FindCorrelation(source);
    }

    return correlation;
}

// ****************************************************************************
//  Method: ViewerPlotList::SetHostDatabaseName
//
//  Purpose:
//    Set the default host and database names associatied with the plot list.
//
//  Arguments:
//    hostDB :  The host database string to use for setting the host
//              and database names.
//
//  Programmer: Eric Brugger
//  Creation:   August 30, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Jul 30 15:24:13 PST 2002
//    I moved the splitting code into SplitHostDatabase.
//
//    Brad Whitlock, Mon Mar 1 13:26:58 PST 2004
//    I changed the routine so if an empty name is entered, the name of the
//    database will be empty.
//
//    Brad Whitlock, Thu Mar 25 15:30:09 PST 2004
//    I made it use the file server.
//
// ****************************************************************************

void
ViewerPlotList::SetHostDatabaseName(const std::string &hostDB)
{
    //
    // Split the database name into its host and filename components.
    //
    if(hostDB.size() > 0)
    {
        ViewerFileServer::SplitHostDatabase(hostDB, hostName, databaseName);

        //
        // Save the hostDatabaseName. Note that we don't just set it equal
        // to hostDB in case a name without a host was passed.
        //
        hostDatabaseName = ViewerFileServer::ComposeDatabaseName(hostName,
                                                                 databaseName);
    }
    else
    {
        hostDatabaseName = "";
        hostName = "";
        databaseName = "";
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
//   Brad Whitlock, Thu Mar 25 15:32:10 PST 2004
//   I made it use the file server.
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
    hostDatabaseName = ViewerFileServer::ComposeDatabaseName(hostName,
        databaseName);
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
//   Brad Whitlock, Fri Mar 26 14:40:39 PST 2004
//   I made it use strings.
//
// ****************************************************************************

bool
ViewerPlotList::FileInUse(const std::string &host, const std::string &database) const
{
    for (int i = 0; i < nPlots; ++i)
    {
        if (host == plots[i].plot->GetHostName() &&
            database == plots[i].plot->GetDatabaseName())
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
// Method: ViewerPlotList::GetNumVisiblePlots
//
// Purpose: 
//   Counts the number of plots that are realized and NOT hidden.
//
// Returns:    The number of plots that are visible in the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:21:23 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlotList::GetNumVisiblePlots() const
{
    int nVisiblePlots = 0;

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].realized && !plots[i].hidden &&
            !plots[i].plot->GetErrorFlag())
        {
            ++nVisiblePlots;
        }
    }

    return nVisiblePlots;
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
//    Brad Whitlock, Mon Mar 22 15:12:13 PST 2004
//    I added code to update the time sliders since a new time slider could
//    have been created if we allow automatic database correlation. I also
//    changed how the default SIL restriction gets set.
//
//    Jeremy Meredith, Wed Mar 24 12:56:34 PST 2004
//    Since it is possible for GetDefaultSILRestriction to throw an exception
//    I added a try/catch around it.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map plots to the engine used to create them.
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
        newPlot = NewPlot(type, engineKey, hostName, databaseName,
                          var, applyOperators);
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
    // Find a compatible plot to set the new plot's SIL restriction.
    //
    int compatiblePlotIndex = FindCompatiblePlot(newPlot);

    if (compatiblePlotIndex > -1)
    {
        avtSILRestriction_p new_silr = GetDefaultSILRestriction(
            newPlot->GetHostName(), newPlot->GetDatabaseName(),
            newPlot->GetVariableName(), newPlot->GetState());
        ViewerPlot *matchedPlot = plots[compatiblePlotIndex].plot;
        new_silr->SetFromCompatibleRestriction(matchedPlot->GetSILRestriction());
        newPlot->SetSILRestriction(new_silr);
    }
    UpdateSILRestrictionAtts();

    //
    // If we added a plot, it's possible that a time slider was created if
    // we allow automatic database correlation. We need to send the time
    // sliders back to the client.
    //
    ViewerWindowManager::Instance()->UpdateWindowInformation(
         WINDOWINFO_TIMESLIDERS);

    return plotId;
}

// ****************************************************************************
//  Method: ViewerPlotList::SetPlotRange
//
//  Purpose:
//    Set the range for the specified plot.
//
//  Arguments:
//    plotId    : The id of the plot.
//    index0    : The start index of the plot.
//    index1    : The end index of the plot.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
//  Modifications:
//    Brad Whitlock, Sun Jan 25 22:09:49 PST 2004
//    Changed how animation works.
//
//    Brad Whitlock, Mon Apr 5 14:42:39 PST 2004
//    I renamed the method.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotRange(int plotId, int index0, int index1)
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
    plots[plotId].plot->SetRange(index0, index1);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdatePlotAtts(false);
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    UpdateFrame();
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
    UpdateFrame();
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
    UpdateFrame();
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
//    Brad Whitlock, Mon Apr 5 14:51:56 PST 2004
//    Changed the name of a method call.
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
    plots[plotId].plot->AddDatabaseKeyframe(frame, state);

    //
    // Update the client attributes.
    //
    UpdatePlotList();
    UpdateSILRestrictionAtts();

    //
    // Update the frame.
    //
    UpdateFrame();
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
    UpdateFrame();
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
    UpdateFrame();
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
//    Brad Whitlock, Wed Feb 4 11:49:12 PDT 2004
//    Added code to copy the time sliders to the new plot list. I also made
//    sure that the plot's state is set properly after it gets created.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map plots to the engine used to create them.
//
//    Brad Whitlock, Fri Apr 2 10:57:29 PDT 2004
//    I rewrote the method so it uses the new ViewerPlot copy constructor.
//
// ****************************************************************************

void
ViewerPlotList::CopyFrom(const ViewerPlotList *pl)
{
    //
    // Copy the database and the host database.
    //
    SetHostDatabaseName(pl->GetHostDatabaseName());

    //
    // Copy the engine key
    //
    SetEngineKey(pl->GetEngineKey());

    //
    // Copy the animation playback mode.
    //
    playbackMode = pl->GetPlaybackMode();

    //
    // If the plot list being copied is in keyframe mode then put the
    // current plot list in keyframe mode.
    //
    if(pl->keyframeMode)
        SetNKeyframes(pl->GetNKeyframes());
    SetKeyframeMode(pl->keyframeMode);

    //
    // Copy the time sliders and the active time slider from the input
    // plot list to this plot list.
    //
    for(StringIntMap::const_iterator pos = pl->timeSliders.begin();
        pos != pl->timeSliders.end(); ++pos)
    {
        timeSliders[pos->first] = pos->second;
    }
    activeTimeSlider = pl->activeTimeSlider;

    //
    // Copy the plots from the input plot list (pl) to this plot list.
    //
    int plotsAdded = 0;
    for (int i = 0; i < pl->GetNumPlots(); ++i)
    {
         //
         // Try and create a copy of the i'th plot.
         //
         ViewerPlot *src = pl->GetPlot(i);
         ViewerPlot *dest = 0;
         TRY
         {
             dest = new ViewerPlot(*src);
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

         if(dest != 0)
         {
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
//    Brad Whitlock, Fri Apr 2 11:13:17 PDT 2004
//    I added code that makes the plot aware that it belonges to this
//    plot list.
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
    plots[nPlots].plot->RegisterViewerPlotList(this);
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
//   Brad Whitlock, Sun Jan 25 22:10:58 PST 2004
//   I added support for database correlations.
//
//   Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//   Added an engine key to map plots to the engine used to create them.
//
//   Brad Whitlock, Thu Apr 8 15:46:46 PST 2004
//   I added support for keyframing back in.
//
//   Mark C. Miller, Wed Apr 14 10:44:42 PDT 2004
//   I added code to support the catch-all mesh feature and compute a new
//   variable name from the catch all mesh name
//
// ****************************************************************************

ViewerPlot *
ViewerPlotList::NewPlot(int type, const EngineKey &ek,
                        const std::string &host, const std::string &db,
                        const std::string &var, bool applyOperators)
{
    //
    // Get the correlation for the plotDB and use that for the number of states.
    // We could use the metadata but asking for that often requires a state (as
    // in the case of time-varying metadata). Since we only need the number of
    // states, it is okay to use the trivial correlation.
    //
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
    int nStates = 1;
    std::string plotSource(ViewerFileServer::ComposeDatabaseName(host, db));
    DatabaseCorrelation *correlation = cL->FindCorrelation(plotSource);
    if(correlation != 0)
        nStates = correlation->GetNumStates();

    //
    // If the variable name we've recieved is the name for the catch-all
    // mesh and if database metadata indicates we should be using the
    // catch-all mesh, replace variableName with the name of the mesh for
    // the active variable(s)
    //
    std::string newVarName = var;
    if (var == Init::CatchAllMeshName)
        GetMeshVarNameForActivePlots(host, db, newVarName);

    //
    // Get the correlation for the active time slider so we can see if it
    // uses the plot source. If so then use the correlated time state for
    // the plot.
    // 
    int plotState = 0;
    if(nStates > 1 && HasActiveTimeSlider())
    {
        //
        // The time slider's correlation did not use the new plot's
        // source. Find a correlation that best matches the plots in
        // the plot list and the new plot source.
        //
        DatabaseCorrelation *mostSuitable = GetMostSuitableCorrelation(
            plotSource, true);
        if(mostSuitable)
        {
            int activeTSState = timeSliders[activeTimeSlider];
            plotState = mostSuitable->GetCorrelatedTimeState(plotSource,
                activeTSState);
            if(plotState == -1)
                plotState = 0;
        }
    }

    //
    // Get the default SIL restriction.
    //
    avtSILRestriction_p silr(0);
    silr = GetDefaultSILRestriction(host, db, newVarName, plotState);

    if (*silr == 0)
    {
        char str[400];
        SNPRINTF(str, 400, "VisIt could not create a SIL restriction for %s. "
                     "The plot of \"%s\" cannot be added.", db.c_str(), newVarName.c_str());
        Error(str);
        return 0;
    }

    //
    // Create the initialized plot at the specified time state.
    //
    ViewerPlotFactory *plotFactory = viewerSubject->GetPlotFactory();
    ViewerPlot *plot = 0;
    TRY
    {
        //
        // If we're not in keyframing mode then we only want to size the
        // plot's caches so they can contain enough actors, etc for the
        // number of states in the plot's database.
        //
        int cacheIndex(plotState);
        int cacheSize(nStates);

        //
        // If we're in keyframing mode, we want to size the plot's cache
        // so it can store enough actors for the desired number of frames.
        //
        if(GetKeyframeMode())
            GetTimeSliderStates(KF_TIME_SLIDER, cacheIndex, cacheSize);

        //
        // Create the plot.
        //
        plot = plotFactory->CreatePlot(type, ek, host, db, newVarName,
                                       silr, plotState, nStates,
                                       cacheIndex, cacheSize);
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
    UpdateFrame();
}

// ****************************************************************************
//  Method: ViewerPlotList::ClearActors
//
//  Purpose:
//    Clear the actors associated with the plots in the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   March 30, 2004
//
// ****************************************************************************

void
ViewerPlotList::ClearActors()
{
    //
    // Clear the actors associated with the plots.
    //
    for (int i = 0; i < nPlots; i++)
    {
        plots[i].plot->ClearActors();
    }
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
    // all other plots into their new positions.
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
        UpdateFrame();
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
//    Brad Whitlock, Sun Jan 25 22:33:49 PST 2004
//    I changed how animations are stopped. I also added code to set the 
//    active time slider to be the active source if all plots are deleted
//    and the correlation for the active time slider does not contain
//    the active source.
//
//    Brad Whitlock, Wed Apr 7 22:30:35 PST 2004
//    I prevented the time slider from being reset to the active source if
//    the time slider was the keyframe animation time slider. I removed a
//    call to set the opaque flag for mesh plots since it is now done when
//    plots are added to the window.
//
//    Brad Whitlock, Mon Apr 19 08:30:55 PDT 2004
//    I added code to make sure that we don't set the time slider to the
//    active source unless the active source is an MT database.
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
    }
    else
    {
        // If there are no plots, make sure we stop animation.
        animationMode = StopMode;

        //
        // Make sure that we change the active time slider to be the 
        // active source if the correlation for the active time slider
        // does not contain the active source.
        //
        if(HasActiveTimeSlider())
        {
            DatabaseCorrelationList *cL = ViewerFileServer::Instance()->
                GetDatabaseCorrelationList();
            DatabaseCorrelation *tsCorrelation = cL->FindCorrelation(
                activeTimeSlider);
            if(tsCorrelation != 0 &&
               !tsCorrelation->UsesDatabase(hostDatabaseName) &&
               cL->FindCorrelation(hostDatabaseName) != 0)
            {
                SetActiveTimeSlider(hostDatabaseName);

                ViewerWindowManager::Instance()->UpdateWindowInformation(
                    WINDOWINFO_TIMESLIDERS, window->GetWindowId());
            }
        }
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
    UpdateFrame();
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
//  Modifications:
//    Brad Whitlock, Mon Dec 8 15:28:19 PST 2003
//    I changed the logic so it does not try and match additional features
//    if the databases are not the same in the first place.
//
//    Brad Whitlock, Fri Mar 26 08:36:04 PDT 2004
//    I made it use strings for host, db, var.
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
        // ignore the the plot in the list that is the same as the given plot
        if (plots[i].plot == givenPlot)
            continue;

        // check basic compatibility
        bool sameHost = plots[i].plot->GetHostName() ==
                        givenPlot->GetHostName();
        bool sameDB   = plots[i].plot->GetDatabaseName() ==
                        givenPlot->GetDatabaseName();
        //
        // If the host and database are the same, check for compatibility
        // in other features.
        //
        if(sameHost && sameDB)
        {
            int numFeaturesMatched = 0;

            if (strcmp(plots[i].plot->GetPlotName(),givenPlot->GetPlotName()) == 0)
                numFeaturesMatched++;
            if (strcmp(plots[i].plot->GetPluginID(),givenPlot->GetPluginID()) == 0)
                numFeaturesMatched++;
            if (plots[i].plot->GetVariableName() ==
                givenPlot->GetVariableName())
                numFeaturesMatched++;
            if (plots[i].plot->GetType() == givenPlot->GetType())
                numFeaturesMatched++;
            if (plots[i].plot->GetNetworkID() == givenPlot->GetNetworkID())
                numFeaturesMatched++;
            if (plots[i].plot->GetVarType() == givenPlot->GetVarType())
                numFeaturesMatched++;

            if (numFeaturesMatched > maxFeaturesMatched)
            {
                maxFeaturesMatched = numFeaturesMatched;
                compatibleIndex = i;
            }
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
//    Mark C. Miller Mon Dec  8 09:23:45 PST 2003
//    Added call to UpdatePlotList
//
//    Brad Whitlock, Sat Jan 31 22:16:45 PST 2004
//    I removed the frame argument.
//
// ****************************************************************************

void
ViewerPlotList::TransmutePlots(bool turningOffScalableRendering)
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
    // Update the GUI's plot list information
    //
    UpdatePlotList();

    //
    // transmute the actors associated with the plots
    //
    for (i = 0; i < nPlots; i++)
    {
        if (plots[i].realized &&
           !plots[i].hidden &&
            plots[i].plot->IsInRange() &&
           !plots[i].plot->GetErrorFlag())
        {
            plots[i].plot->TransmuteActor(turningOffScalableRendering);
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
//   Kathleen Bonnell, Thu Aug 28 10:10:35 PDT 2003 
//   Added call to CanMeshPlotBeOpaque.
//
//   Brad Whitlock, Thu Apr 8 11:31:59 PDT 2004
//   I removed the call to CanMeshPlotBeOpaque since it is now called from
//   a central location.
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
 
    //
    // Update the client attributes.
    //
    UpdatePlotList();

    //
    // Update the frame.
    //
    UpdateFrame();
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
    UpdateFrame();
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
//    Brad Whitlock, Fri Mar 26 08:27:56 PDT 2004
//    I made it use string.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotVar(const std::string &variable)
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
    UpdateFrame();
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
//    Brad Whitlock, Tue Feb 3 22:56:48 PST 2004
//    I changed how the plot atts get set.
//
//    Kathleen Bonnell, Mon Aug  9 15:08:14 PDT 2004 
//    If no ACTIVE plot matches the plot type, but there is ONE plot of the
//    given type, go ahead and set that plot's attributes.
//
// ****************************************************************************

void
ViewerPlotList::SetPlotAtts(const int plotType)
{
    //
    // Loop through the list setting the plot attributes from the client
    // for any plots that are active and/or match the type.
    // If any are active, we will set those; if there is only one that
    // that matches, we will set that one, otherwise we will set a
    // warning.
    //
    int i;
    intVector matchingPlots;
    intVector selectedPlots;
    for (i = 0; i < nPlots; i++)
    {
        if (plots[i].plot->GetType() == plotType)
        {
            matchingPlots.push_back(i);
            if (plots[i].active)
            {
                selectedPlots.push_back(i);
            }
        }
    }

    //
    // If plots were selected, update the frame.
    //
    if (selectedPlots.size() > 0)
    {
        for (i = 0; i < selectedPlots.size(); i++)
        {
            plots[selectedPlots[i]].plot->SetPlotAttsFromClient();
        }
        //
        // If we're in keyframing mode then send the plot list since
        // to the client because it contains the keyframe indices.
        //
        if(GetKeyframeMode())
            UpdatePlotList();

        // Update the frame.
        UpdateFrame();
    }
    else if (matchingPlots.size() == 1) 
    {
        plots[matchingPlots[0]].plot->SetPlotAttsFromClient();
        //
        // If we're in keyframing mode then send the plot list since
        // to the client because it contains the keyframe indices.
        //
        if(GetKeyframeMode())
            UpdatePlotList();

        // Update the frame.
        UpdateFrame();
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
    UpdateFrame();
}

// ****************************************************************************
// Method: ViewerPlotList::ActivateSource
//
// Purpose: 
//   Activates the specified source.
//
// Arguments:
//   source : The source that we want to be active.
//
// Note:       This is like opening a database that is already open but also
//             has the effect of setting the active time slider.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:52:08 PST 2004
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 16:18:00 PST 2004
//    Added code to set the engine key upon changing sources.
//   
// ****************************************************************************

void
ViewerPlotList::ActivateSource(const std::string &source, const EngineKey &ek)
{
    SetHostDatabaseName(source);
    SetEngineKey(ek);

    //
    // Find a correlation for the active time slider. Also get the metadata
    // for the new source so we can make sure that it has more than one time
    // state.
    //
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();
    const avtDatabaseMetaData *md = fs->GetMetaData(hostName, databaseName);
    DatabaseCorrelation *activeTSC = cL->FindCorrelation(activeTimeSlider);

    if(md != 0 && activeTSC != 0 && md->GetNumStates() > 1)
    {
        if(!HasActiveTimeSlider() || !activeTSC->UsesDatabase(source))
        {
            if(nPlots > 1)
            {
                //
                // We have some plots so look through all of the correlations
                // for a correlation that has the source and maybe some
                // of the plot databases.
                //
                StringIntMap scores;
                for(int i = 0; i < cL->GetNumDatabaseCorrelations(); ++i)
                {
                    const DatabaseCorrelation &c = cL->GetDatabaseCorrelation(i);
                    if(c.UsesDatabase(source))
                    { 
                        scores[c.GetName()] = 0;
                        for(int j = 0; j < nPlots; ++j)
                        {
                            if(c.UsesDatabase(plots[j].plot->GetSource()))
                                ++scores[c.GetName()];
                        }
                    }
                }

                //
                // Look through the scores to find the most suitable
                // time slider.
                //
                if(scores.size() == 0)
                {
                    // Should not get here but be safe.
                    activeTimeSlider = source;
                }
                else
                {
                    int maxScore = -1;
                    for(StringIntMap::const_iterator pos = scores.begin();
                        pos != scores.end(); ++pos)
                    {
                        if(pos->second > maxScore) 
                        {
                            maxScore = pos->second;
                            activeTimeSlider = pos->first;
                        }
                    }
                }
            }
            else
            {
                // There are no other plots to consider. Just use the 
                // source's time slider.
                activeTimeSlider = source;
            }
        }
    } // end numstates > 1

    //
    // Update the window information since the source and active time
    // slider could have changed.
    //
    ViewerWindowManager::Instance()->UpdateWindowInformation(
        WINDOWINFO_SOURCE | WINDOWINFO_TIMESLIDERS, window->GetWindowId());
}

// ****************************************************************************
// Method: ViewerPlotList::CloseDatabase
//
// Purpose: 
//   Closes the named database if possible.
//
// Arguments:
//   dbName : The name of the database to close.
//
// Returns:    An int that encodes the flags that must be sent back to the
//             client in the window information.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 22 15:38:12 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlotList::CloseDatabase(const std::string &dbName)
{
    int retval = 0;

    //
    // We're closing the open database for the plot list
    // but make the first active plot's database be active if
    // there are plots.
    //
    std::string newDB;
    intVector activePlots;
    GetActivePlotIDs(activePlots);
    if(activePlots.size() > 0)
        newDB = GetPlot(activePlots[0])->GetSource();

    //
    // If we're closing the database for the plot list, set the new open
    // database, if any, so VisIt can still be used.
    //
    if(hostDatabaseName == dbName)
    {
        SetHostDatabaseName(newDB);
        retval |= WINDOWINFO_SOURCE;
    }

    //
    // If the active time slider for the plot list was also
    // the database that we closed, try and use the time
    // time slider for the active plot's database, if one
    // exists. If not, use the first time slider that is not
    // the database that we closed.
    //
    if(HasActiveTimeSlider() && dbName == activeTimeSlider)
    {
        DatabaseCorrelation *correlation = 0;

        if(newDB != "")
            correlation = GetMostSuitableCorrelation(newDB, false);

        if(correlation != 0)
            SetActiveTimeSlider(correlation->GetName());
        else
            activeTimeSlider = "";

        retval |= WINDOWINFO_TIMESLIDERS;
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerPlotList::ReplaceDatabase
//
// Purpose: 
//   Replaces the database used in the current plots with a new database and
//   regenerates the plots.
//
// Arguments:
//   key      : The key of the engine that will process the data.
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
//   Brad Whitlock, Fri Mar 26 14:02:12 PST 2004
//   Use the time state for getting the new SIL restriction.
//
//   Brad Whitlock, Mon Apr 5 11:24:18 PDT 2004
//   I changed the code so it should be able to change the size of the caches
//   for a plot without having to clear the actors.
//
//   Brad Whitlock, Fri Apr 9 17:18:39 PST 2004
//   I made it clear the actors for now so reopen works correctly.
//
//   Eric Brugger, Fri Apr 16 14:03:33 PDT 2004
//   I modified the routine to update the plot list and plot attributes in
//   the client if any plots were updated.
//
//   Brad Whitlock, Mon May 3 12:51:20 PDT 2004
//   I made it use an engine key instead of a host.
//
// ****************************************************************************

void
ViewerPlotList::ReplaceDatabase(const EngineKey &key,
    const std::string &database, int timeState, bool setTimeState,
    bool onlyReplaceSame)
{
    //
    // Loop through the list replacing the plot's database.
    //
    bool defaultChanged = false;
    bool plotsReplaced = false;
    const std::string &host = key.OriginalHostName();
    for (int i = 0; i < nPlots; i++)
    {
        //
        // Decide which files to replace.
        //
        ViewerPlot *plot = plots[i].plot;
        bool sameHost = (key == plot->GetEngineKey());
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
                    database, plot->GetVariableName(), timeState);
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
                    plot->SetHostDatabaseName(key, database.c_str());
                    plot->SetSILRestriction(silr);
                    plotsReplaced = true;

                    //
                    // Tell the plot to update its cache size and whether it should
                    // clear out its actors when it does change the cache size.
                    // Currently, we want to clear the actors but in the
                    // future when we have CheckForNewStates fully implemented,
                    // we may not want to clear actors.
                    //
                    plot->UpdateCacheSize(GetKeyframeMode(), true);
                }
            }
            CATCH(InvalidVariableException)
            {
                char str[1024];
                SNPRINTF(str, 1024, "The %s plot of \"%s\" cannot be regenerated "
                             "using the database: %s since the variable "
                             "is not contained in the new database.",
                             plot->GetPlotName(),
                             plot->GetVariableName().c_str(),
                             database.c_str());
                Error(str);
            }
            ENDTRY
        }
    }

    //
    // Update the client attributes.
    //
    UpdateExpressionList(true);

    //
    // Update the plot and SIL restriction attributes if necessary.
    //
    if (plotsReplaced)
    {
        UpdatePlotList();
        UpdatePlotAtts();
        UpdateSILRestrictionAtts();
    }

    //
    // Update the frame.
    //
    if(setTimeState)
        SetTimeSliderState(timeState);
    else
        UpdateFrame();
}

// ****************************************************************************
// Method: ViewerPlotList::OverlayDatabase
//
// Purpose: 
//   Creates new plots based on the current plots and adds them to the plot
//   list.
//
// Arguments:
//   ek       : The key of the engine that will process the data.
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
//   Jeremy Meredith, Wed Mar 24 12:57:09 PST 2004
//   Since it is possible for NewPlot to throw an exception, I added
//   try/catch around it.
//
//   Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//   Added an engine key to map plots to the engine used to create them.
//
//   Brad Whitlock, Mon Apr 5 12:13:07 PDT 2004
//   I rewrote the method using a copy constructor.
//
//   Brad Whitlock, Mon May 3 12:43:31 PDT 2004
//   I removed the host argument since that info is in the engine key.
//
// ****************************************************************************

void
ViewerPlotList::OverlayDatabase(const EngineKey &ek,
                                const std::string &database)
{
    //
    // Loop over the initial list of plots and add new plots based on them
    // which use the new data files.
    //
    int nInitialPlots = nPlots;
    for (int i = 0; i < nInitialPlots; ++i)
    {
        ViewerPlot *newPlot = 0;
        TRY
        {
            newPlot = new ViewerPlot(*(plots[i].plot));
            newPlot->SetHostDatabaseName(ek, database);
            newPlot->UpdateCacheSize(GetKeyframeMode(), true);
        }
        CATCHALL(...)
        {
            // newPlot will be zero if an error occurred, so we don't
            // need to do further error handling right here
        }
        ENDTRY

        //
        // If the plot was created, add it to the plot list.
        //
        if (newPlot)
        {
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
                         plots[i].plot->GetVariableName().c_str(),
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
//   state    : The state at which we want the SIL restriction.
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
//   Brad Whitlock, Wed Dec 31 14:16:55 PST 2003
//   I added code to catch InvalidVariableException if it gets thrown from
//   avtDatabaseMetaData::MeshForVar.
//
//   Jeremy Meredith, Wed Mar 24 12:51:24 PST 2004
//   I moved the try/catch block around avtDatabaseMetaData::MeshForVar
//   up a level or two.  If we catch it too soon, then we will get later
//   errors that overwrite the error message from it.  This meant ensuring
//   any path up the stack would eventually catch the exception.  Luckily
//   there are only a few paths that needed to be filled in.
//
//   Brad Whitlock, Fri Mar 26 13:46:00 PST 2004
//   I added a mandatory state argument so the right SIL is returned.
//
// ****************************************************************************

avtSILRestriction_p
ViewerPlotList::GetDefaultSILRestriction(const std::string &host,
    const std::string &database, const std::string &var, int state)
{
    avtSILRestriction_p silr(0);
    ViewerFileServer *server = ViewerFileServer::Instance();
    int topSet = 0;
    char str[400];

    //
    // Get the SIL from the file server.
    //
    const avtSIL *sil = server->GetSILForState(host, database, state);
    if (sil == 0)
    {
        SNPRINTF(str, 400, "VisIt could not read the SIL for %s at state %d.",
            database.c_str(), state);
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
        (avtDatabaseMetaData *)server->GetMetaDataForState(host, database, state);
    if (md == 0)
    {
        SNPRINTF(str, 400, "VisIt could not read the MetaData for %s at state %d.",
            database.c_str(), state);
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
    std::string meshName;
    meshName = md->MeshForVar(realvar);// Don't catch InvalidVariableException!

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
//    Brad Whitlock, Fri Mar 26 14:48:51 PST 2004
//    Made plot name comparison use strings.
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
        bool sameDB =  (plot0->GetDatabaseName() == ploti->GetDatabaseName());
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
        UpdateFrame();
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
    UpdateFrame(false);
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
    UpdateFrame();
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
    UpdateFrame();
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
//    Brad Whitlock, Sun Jan 25 22:57:02 PST 2004
//    I added the concept of multiple time sliders.
//
// ****************************************************************************

bool
ViewerPlotList::ArePlotsUpToDate() const
{
    for (int i = 0; i < nPlots; i++)
    {
        ViewerPlot *p = plots[i].plot;

        //
        // If the frame is not in range for this plot then skip to the next
        // plot because this plot doesn't need to ruin it for the other plots
        // which may be defined at this frame.
        //
        if (!p->IsInRange())
            continue;

#ifdef VIEWER_MT
        //
        // An unfortunate degenerate case.  Trying to execute the plot
        // caused an exception.  If the plot was executed previously, then
        // we need to clear it out of the ViewerWindow, so pretend that
        // it is up-to-date.
        //
        if (p->NoActorExists() && p->GetErrorFlag())
            return true;
#endif
        //
        // The real issue is that a plot must want to be drawn (realized is
        // true), it must not be hidden (hidden is false). If a plot meets
        // those conditions and it has no actor or it has its error flag set
        // then it is not up to date we return false.
        //
        bool isCandidate = plots[i].realized && !plots[i].hidden;
        bool needsGenerated = (p->NoActorExists() ||
                               p->GetErrorFlag());
        if (isCandidate && needsGenerated)
            return false;
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
//    Update the plots for the specified time slider time state.
//
//  Arguments:
//    state     The state to use for updating the plots.
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
//    Jeremy Meredith, Mon Mar 22 17:40:13 PST 2004
//    Removed the ifdef to disable caching.  It was past its expiration date.
//    I added a check to see if a plot was interrupted during its creation.
//
//    Brad Whitlock, Sun Jan 25 22:59:07 PST 2004
//    I added multiple time sliders.
//
//    Brad Whitlock, Thu Apr 8 11:32:37 PDT 2004
//    I removed the call to CanMeshPlotsBeOpaque because I moved it to a
//    central location to fix issues with the opacity and keyframing.
//
// ****************************************************************************

bool
ViewerPlotList::UpdatePlots(bool animating)
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
            //
            // Generate the plot actor for the current state if it does
            // not already exist.
            //
            if (plots[i].realized &&
                plots[i].plot->IsInRange() &&
                !plots[i].hidden &&
                !plots[i].plot->GetErrorFlag() &&
                plots[i].plot->NoActorExists())
            {
                debug3 << "\tRegenerating plot " << i
                       << " source=" << plots[i].plot->GetSource().c_str()
                       << endl;

                if(interrupted)
                {
                    plots[i].plot->SetErrorFlag(true);
                    continue;
                }

                PlotInfo  *info=0;
    
                info = new PlotInfo;
                info->plot = plots[i].plot;
                info->plotList = this;
                info->window = window;

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

                if(interrupted)
                {
                    plots[i].plot->SetErrorFlag(true);
                    continue;
                }
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

#ifdef VIEWER_MT
    return false;
#else
    return true;
#endif
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
//    Brad Whitlock, Sun Jan 25 23:10:40 PST 2004
//    I added the concept of multiple time sliders.
//
//    Jeremy Meredith, Tue Mar 23 14:29:07 PST 2004
//    I made it check that the SetWindowAtts RPC succeeded before proceeding,
//    and setting the interruption status if not.  The SetWindowAtts RPC is
//    the first one called for a plot, so it can fail if the engine didn't
//    launch.
//
//    Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//    Added an engine key to map plots to the engine used to create them.
//
//    Mark C. Miller, Tue Apr 20 07:44:34 PDT 2004
//    Added code to issue warning message if a created actor has no data
//
//    Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//    Added additional check for IsChangingScalableRenderingMode when
//    issuing no data warning
//
// ****************************************************************************

void *
CreatePlot(void *info)
{
    PlotInfo  *plotInfo=(PlotInfo *)info;

    TRY
    {
        // Couldn't this method call be made once per engine???
        bool success = plotInfo->window->SendWindowEnvironmentToEngine(
                                               plotInfo->plot->GetEngineKey());

        if(success)
        {
            bool createNewActor = true;
            bool turningOffScalableRendering = false;
            bool actorHasNoData = false;
            plotInfo->plot->CreateActor(createNewActor,
                                        turningOffScalableRendering,
                                        actorHasNoData);

            if (actorHasNoData && !plotInfo->window->GetScalableRendering() &&
                !plotInfo->window->IsChangingScalableRenderingMode(true))
            {
                char message[256];
                SNPRINTF(message, sizeof(message),
                    "The %s plot of variable \"%s\" yielded no data.",
                    plotInfo->plot->GetPlotName(),
                    plotInfo->plot->GetVariableName().c_str());
                Warning(message);
            }

        }
        else
            plotInfo->plotList->InterruptUpdatePlotList();
    }
    CATCH(AbortException)
    {
        plotInfo->plotList->InterruptUpdatePlotList();
    }
    ENDTRY

    delete plotInfo;

    return (void *) 0;
}

// ****************************************************************************
//  Method: ViewerPlotList::UpdateWindow
//
//  Purpose:
//    Update the specified window with the plots from the specified frame.
//
//  Arguments:
//    state           : The state to use for the plot.
//    nStates         : The number of states in the animation.
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
//    Brad Whitlock, Sun Jan 25 23:57:33 PST 2004
//    I added the concept of multiple time sliders.
//
//    Eric Brugger, Tue Mar 30 16:12:41 PST 2004
//    I added a call to UpdateDataExtents.
//
//    Brad Whitlock, Fri Apr 2 15:43:10 PST 2004
//    Added support for keyframing. I also changed how the opaque flag for
//    mesh plots is set.
//
// ****************************************************************************

void
ViewerPlotList::UpdateWindow(bool immediateUpdate)
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
    int         state = 0;
    int         nStates = 1;

    globalWindowMode = WINMODE_NONE;
    globalExtents[0] = DBL_MAX; globalExtents[1] = -DBL_MAX;
    globalExtents[2] = DBL_MAX; globalExtents[3] = -DBL_MAX;
    globalExtents[4] = DBL_MAX; globalExtents[5] = -DBL_MAX;

    //
    // Before we add plots to the window, set the opaque mesh flag on all
    // of the mesh plots so they can adjust to the plots that are going to
    // be in the window.
    //
    CanMeshPlotBeOpaque();

    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].plot->GetErrorFlag())
        {
            // Don't draw bad plots.
            debug5 << "\tplot " << i << " was not added to the window "
                   << "because it had an error" << endl;
            continue;
        }

        //
        // If the reader or the actor is bad then mark the plot as bad. This
        // usually happens when a plot generated before the current plot has
        // had an error and the current plot has not been generated.
        //
        if (*(plots[i].plot->GetReader()) == 0 ||
            plots[i].plot->NoActorExists())
        {
            debug5 << "\tplot " << i << " was not added to the window "
                   << "because it has no actor for cache index "
                   << plots[i].plot->GetCacheIndex()
                   << ", which is the index that should be shown "
                   << "for the plot." << endl;
            continue;
        }

        if (plots[i].plot->IsInRange() &&
            plots[i].realized == true && plots[i].hidden == false)
        {
            avtActor_p &actor = plots[i].plot->GetActor();
            WINDOW_MODE plotWindowMode = actor->GetWindowMode();
            int plotDimension = plots[i].plot->GetSpatialDimension();

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

                double *plotExtents = plots[i].plot->GetSpatialExtents();

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
                plots[i].plot->UpdateDataExtents();

                debug5 << "\t plot " << i << " was added to the window." << endl;
                window->AddPlot(actor);

                //
                // Get the plot's database states for the start, current, and
                // end frames.
                //
                if(startState == -1)
                {
                    //
                    // Get the current state and total number of states for the
                    // active time slider's correlation.
                    //
                    if(HasActiveTimeSlider())
                    {
                        GetTimeSliderStates(activeTimeSlider, state, nStates);
                        startFrame = 0;
                        endFrame = nStates;
                    }
                    else
                    {
                        startFrame = 0;
                        endFrame = 1;
                    }

                    //
                    // Get the number of states and the current state for
                    // the i'th plot's databases.
                    //
                    DatabaseCorrelationList *cL = ViewerFileServer::Instance()->
                        GetDatabaseCorrelationList();
                    DatabaseCorrelation *c = cL->FindCorrelation(
                        plots[i].plot->GetSource());
                    if(c != 0)
                    {
                        startState = 0;
                        curState = plots[i].plot->GetState();
                        endState = c->GetNumStates();
                    }
                    else
                    {
                        startState = 0;
                        curState = 0;
                        endState = 1;
                    }
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
    if(startState != -1)
    {
        window->SetFrameAndState(nStates, startFrame, state, endFrame,
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
//    Brad Whitlock, Sun Feb 1 00:30:30 PDT 2004
//    I changed a bunch of the calls to ViewerPlot so we don't need the 
//    animation frame.
//
//    Brad Whitlock, Mon Apr 5 11:48:07 PDT 2004
//    I made keyframing work again after I broke it.
//
// ****************************************************************************

void
ViewerPlotList::UpdatePlotAtts(bool updateThoseNotRepresented) const
{
    //
    // Return if this isn't the active plot list.
    //
    if (this != ViewerWindowManager::Instance()->GetActiveWindow()->GetPlotList())
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
        if (plot->IsInRange() && plots[i].active)
        {
            plotCount[plotType]++;
            if (plotCount[plotType] == 1)
            {
                plot->SetClientAttsFromPlot();
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
//  Method: ViewerPlotList::GetPlotAtts
//
//  Purpose:
//     Populate an STL vector with the current attributes of every active,
//     realized, non-hidden plot in the plot list AND in the current frame.
//
//  Programmer: Mark C. Miller
//  Creation:   07Apr03 
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 23 14:27:15 PST 2004
//    Scalable rendering required an actor to not only exist but also
//    have some actor before scalable rendering would affect it.  I
//    made the test for plots to be rendered more stringent.
//
//    Brad Whitlock, Sat Jan 31 22:35:47 PST 2004
//    I made frame no longer necessary.
//
//    Jeremy Meredith, Thu Mar 25 15:34:23 PST 2004
//    I added a list of engine keys.
//
//    Brad Whitlock, Mon Apr 5 12:21:02 PDT 2004
//    I made it use GetPlotAtts and I renamed the method.
//
// ****************************************************************************

void
ViewerPlotList::GetPlotAtts(
   std::vector<const char*>&             pluginIDsList,
   std::vector<EngineKey>&               engineKeysList,
   std::vector<int>&                     plotIdsList,
   std::vector<const AttributeSubject*>& attsList) const
{
    for (int i = 0; i < nPlots; i++)
    {
        if (plots[i].plot->IsInRange() && !plots[i].hidden &&
            plots[i].realized && !plots[i].plot->NoActorExists())
        {
            ViewerPlot *plot = plots[i].plot;

            pluginIDsList.push_back(plot->GetPluginID());
            engineKeysList.push_back(plot->GetEngineKey());
            plotIdsList.push_back(plot->GetNetworkID());
            attsList.push_back(plot->GetPlotAtts());
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
//    Brad Whitlock, Sat Jan 31 22:36:30 PST 2004
//    I made most calls to ViewerPlot not require a frame.
//
//    Jeremy Meredith, Tue Mar 30 12:28:52 PST 2004
//    Added suport for simulations.
//
//    Brad Whitlock, Mon Apr 5 09:56:35 PDT 2004
//    I moved much of the code into ViewerPlot.
//
// ****************************************************************************

void
ViewerPlotList::UpdatePlotList() const
{
    //
    // Update the tools.
    //
    window->UpdateTools();

    //
    // Return if this isn't the active animation.
    //
    if (this != ViewerWindowManager::Instance()->GetActiveWindow()->GetPlotList())
    {
        return;
    }

    //
    // Update the client PlotList attribute subject.
    //
    clientAtts->ClearPlots();
    for (int i = 0; i < nPlots; i++)
    {
        Plot plot;
        plots[i].plot->InitializePlot(plot);
        plot.SetActiveFlag(plots[i].active);
        plot.SetHiddenFlag(plots[i].hidden);

        // Figure out the stage of completion that the plot is at.
        if (plots[i].plot->GetErrorFlag())
        {
            plot.SetStateType(Plot::Error);
        }
        else
        {
            if (plots[i].realized)
            {
                if(plots[i].plot->IsInRange() &&
                   plots[i].plot->NoActorExists())
                    plot.SetStateType(Plot::Pending);
                else
                    plot.SetStateType(Plot::Completed);
            }
            else
                plot.SetStateType(Plot::NewlyCreated);
        }

        // Add the plot to the list.
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
// Arguments:
//   considerPlots : Whether to consider the active plots when choosing the
//                   name of the database to use.
//   update        : Whether to notify the client.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 24 16:47:12 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Mar 17 11:47:38 PDT 2004
//   I changed how it determines the state to use for the metadata.
//
// ****************************************************************************

void
ViewerPlotList::UpdateExpressionList(bool considerPlots, bool update)
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
    int t = 0;
    ViewerFileServer *fileServer = ViewerFileServer::Instance();

    if(considerPlots && nPlots > 0)
    {
        for(int i = 0; i < nPlots; ++i)
        {
            if(plots[i].active)
            {
                host = plots[i].plot->GetHostName();
                db = plots[i].plot->GetDatabaseName();
                t = plots[i].plot->GetState();
                break;
            }
        }
    }
    else if(HasActiveTimeSlider())
    {
        // We're not considering plots so let's use the time state
        // for the active source in the time slider's correlation.
        int state = 0, nStates = 1;
        GetTimeSliderStates(GetActiveTimeSlider(), state, nStates);
        DatabaseCorrelationList *cL = fileServer->GetDatabaseCorrelationList();
        // Look for a correlation for the active time slider.
        DatabaseCorrelation *c = cL->FindCorrelation(GetActiveTimeSlider());
        if(c != 0)
        {
            //
            // If the active time slider uses the active source then
            // use that correlated time state to get the metadata.
            //
            int cts = c->GetCorrelatedTimeState(GetHostDatabaseName(), state);
            t = (cts != -1) ? cts : 0;
        }
    }

    //
    // Try and get the metadata for the database.
    //
    if(host.size() > 0 && db.size() > 0)
    {
        const avtDatabaseMetaData *md;
        if((md = fileServer->GetMetaDataForState(host, db, t)) != 0)
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
        if(update)
            exprList->Notify();
    }
}

// ****************************************************************************
// Method: ViewerPlotList::UpdateExpressionListUsingDB
//
// Purpose: 
//   Populates the expression list with the user-defined expressions and
//   the expressions from the specified database but does not notify the
//   client.
//
// Arguments:
//   host : The host where the data is located.
//   db   : The name of the database.
//   t    : The time state to use for getting metadata.
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec 31 14:06:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Mar 26 14:50:24 PST 2004
//   I made it use GetMetaDataForState.
//
// ****************************************************************************

void
ViewerPlotList::UpdateExpressionListUsingDB(const std::string &host,
    const std::string &db, int t) const
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
    // Try and get the metadata for the database.
    //
    if(host.size() > 0 && db.size() > 0)
    {
        ViewerFileServer *fileServer = ViewerFileServer::Instance();
        const avtDatabaseMetaData *md = fileServer->GetMetaDataForState(host, db, t);
        if(md != 0)
        {
            // Add the expressions for the database.
            for (int j = 0 ; j < md->GetNumberOfExpressions(); ++j)
                newList.AddExpression(*(md->GetExpression(j)));
        }
    }

    *exprList = newList;
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
//    Brad Whitlock, Sat Jan 31 22:37:06 PST 2004
//    I made many of the methods calls to ViewerPlot not require a frame.
//
// ****************************************************************************

void
ViewerPlotList::GetPlotLimits(int nDimensions, double *limits) const
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
        if (plots[i].plot->IsInRange() &&
            !plots[i].plot->GetErrorFlag() &&
            plots[i].realized == true)
        {
            double *plotExtents = plots[i].plot->GetSpatialExtents();

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
//  Method: ViewerPlotList::GetMaintainDataMode
//
//  Purpose: 
//    Return the plotlist's maintain data mode.
//
//  Returns:    The maintain data mode.
//
//  Programmer: Eric Brugger
//  Creation:   March 30, 2004
//
// ****************************************************************************

bool
ViewerPlotList::GetMaintainDataMode() const
{
    return window->GetMaintainDataMode();
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
            val |= plots[i].plot->HandleTool(ti);
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
        UpdateFrame();
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
// Method: ViewerPlotList::SetPipelineCaching
//
// Purpose: 
//   Turn pipeline caching on or off.
//
// Arguments:
//   val : Whether we should cache pipelines.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 2 15:09:15 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetPipelineCaching(bool val)
{
    pipelineCaching = val;
    if(!pipelineCaching)
        ClearPipelines();

    if (pipelineCaching && avtCallback::GetNowinMode() == true)
    {
        debug1 << "Overriding request to do pipeline caching, since we are in "
               << "no-win mode." << endl;
        pipelineCaching = false;
    }
}

// ****************************************************************************
// Method: ViewerPlotList::GetPipelineCaching
//
// Purpose: 
//   Returns whether pipeline caching is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 2 15:09:44 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::GetPipelineCaching() const
{
    return pipelineCaching;
}

// ****************************************************************************
//  Method: ViewerPlotList::ClearPipelines
//
//  Purpose: 
//    Clear all pipelines for all states.
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
//    Brad Whitlock, Mon Feb 2 15:12:59 PST 2004
//    I made it clear actors, etc for all time, since that it how it was
//    used for the most part.
//
// ****************************************************************************

void
ViewerPlotList::ClearPipelines()
{
    for (int i = 0; i < nPlots; i++)
    {
        plots[i].plot->ClearActors();
    }
}

// ****************************************************************************
//  Method: ViewerPlotList::StartPick
//
//  Purpose: Start pick mode.
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
//    Brad Whitlock, Sat Jan 31 22:49:22 PST 2004
//    I removed the frame argument.
//
//    Kathleen Bonnell, Tue Jun  1 17:57:52 PDT 2004 
//    Added bool args needZones and needInvTransform.
//
// ****************************************************************************

void 
ViewerPlotList::StartPick(const bool needZones, const bool needInvTransform)
{
    bool needsUpdate = false;
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active && !plots[i].hidden)
        {
            needsUpdate |= plots[i].plot->StartPick(needZones, needInvTransform);
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
        UpdateFrame();
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
// Method: ViewerPlotList::ResetNetworkIds
//
// Purpose: 
//   Resets the network ids for all plots that use the specified engine key.
//
// Arguments:
//   key : The engine key to use.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 3 14:19:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::ResetNetworkIds(const EngineKey &key)
{
    for (int i = 0; i < nPlots; ++i)
        plots[i].plot->ResetNetworkIds(key);
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
//  Modifications:
//    Brad Whitlock, Fri Mar 26 08:31:46 PDT 2004
//    I made it use string.
//
// ****************************************************************************

std::string
ViewerPlotList::GetVarName()
{
    for (int i = 0; i < nPlots; ++i)
    {
        if (plots[i].active && plots[i].realized && !plots[i].hidden)
        {
            return plots[i].plot->GetVariableName();
        }
    }

    return std::string("");
}

// ****************************************************************************
//  Method: ViewerPlotList::GetMeshVarNameForActivePlots
//
//  Purpose:    Determine the mesh variable name for the currently active
//              plots. If there are no currently active plots or the mesh for
//              all the currently active plots is NOT the same, throw an
//              exception
//
//  Programmer: Mark C. Miller 
//  Creation:   April 13, 2004
//
// ****************************************************************************

void
ViewerPlotList::GetMeshVarNameForActivePlots(const std::string &host,
    const std::string &db, std::string &meshName) const
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    const avtDatabaseMetaData *md = fs->GetMetaData(host, db);

    if (!md->GetUseCatchAllMesh())
    {
        EXCEPTION1(InvalidVariableException,
            "Finding the mesh for the active plot is not a supported feature "
            "in the current database");
    }

    intVector activePlotIDs;
    GetActivePlotIDs(activePlotIDs);

    if (activePlotIDs.size() == 0)
    {
        EXCEPTION1(InvalidVariableException,
            "The mesh for the active plot cannot be determined because there "
            "is no currently active plot");
    }

    ViewerPlot *activePlot = GetPlot(activePlotIDs[0]);
    std::string activeVarName = activePlot->GetVariableName();
    std::string tmpMeshName = md->MeshForVar(activeVarName);
    int i;
    for (i = 1; i < activePlotIDs.size(); i++)
    {
        activePlot = GetPlot(activePlotIDs[i]);
        activeVarName = activePlot->GetVariableName();
        if (tmpMeshName != md->MeshForVar(activeVarName))
        {
            EXCEPTION1(InvalidVariableException,
                "The mesh for the active plot cannot be determined because there "
                "are multiple currently active plots which do not have the same "
                "mesh");
        }
    }

    meshName = tmpMeshName;
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
//   Hank Childs, Thu Oct  2 14:22:16 PDT 2003
//   Renamed from GetPlotID.  Made it return a vector of ids.
//
// ****************************************************************************

void
ViewerPlotList::GetActivePlotIDs(intVector &ids) const
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
//  Modifcations:
//    Brad Whitlock, Fri Apr 2 16:12:51 PST 2004
//    Added code to switch all plots into keyframe mode.
//
// ****************************************************************************
 
void
ViewerPlotList::SetKeyframeMode(const bool mode)
{
    if(keyframeMode != mode)
    {
        keyframeMode = mode;

        //
        // If the user did not set the number of keyframes to use, determine
        // the maximum number of states used in the plots and the active
        // source. Use that for the initial number of keyframes. Once the
        // plot list is in keyframing mode, the user can change the number
        // of keyframes.
        //
        if(keyframeMode && !nKeyframesWasUserSet)
        {
            int nFrames = 1;
            if(hostDatabaseName != "")
            {
                const avtDatabaseMetaData *md = ViewerFileServer::Instance()->
                    GetMetaData(hostName, databaseName);
                nFrames = (md != 0) ? md->GetNumStates() : 1;
            }
            for(int i = 0; i < nPlots; ++i)
            {
                const avtDatabaseMetaData *md = plots[i].plot->GetMetaData();
                nFrames = max(((md != 0) ? md->GetNumStates() : 1), nFrames);
            }
            nKeyframes = nFrames;
            nKeyframesWasUserSet = false;
            debug3 << "Setting the number of keyframes to " << nKeyframes
                   << " because the number was never set by the user."
                   << endl;
        }

        // Switch all of the plots into the prevailing keyframe mode.
        for(int i = 0; i < nPlots; ++i)
            plots[i].plot->SetKeyframeMode(keyframeMode);

        if(keyframeMode)
        {
            // When switching into keyframing mode, create a keyframe time
            // slider and make it be the active time slider.
            CreateTimeSlider(KF_TIME_SLIDER, 0);
            SetActiveTimeSlider(KF_TIME_SLIDER);
        }
        else
        {
            // We're going out of keyframing mode. Make it okay to reset
            // the number of frames the next time we enter keyframing mode.
            nKeyframesWasUserSet = false;

            // When leaving keyframing mode, delete the keyframe time
            // slider and use the time slider for the active source.
            StringIntMap::iterator pos = timeSliders.find(KF_TIME_SLIDER);
            if(pos != timeSliders.end())
                timeSliders.erase(pos);

            // Use the active source as the new time slider.
            SetActiveTimeSlider(hostDatabaseName);
        }

        //
        // Send some updated information back to the client.
        //
        UpdatePlotList();
        ViewerWindowManager::Instance()->UpdateKeyframeAttributes();
        ViewerWindowManager::Instance()->UpdateWindowInformation(WINDOWINFO_TIMESLIDERS);

        //
        // Update the frame.
        //
        UpdateFrame();
    }
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
// Method: ViewerPlotList::SetNKeyframes
//
// Purpose: 
//   Tells all of the plots to update their number of keyframes so the plot
//   caches are the right size.
//
// Arguments:
//   nFrames : The new number of keyframes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 5 14:22:07 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerPlotList::SetNKeyframes(int nFrames)
{
    bool different = nKeyframes != nFrames;
    nKeyframes = nFrames;
    nKeyframesWasUserSet = true;

    //
    // If we're in keyframe mode, update the size of the plot caches.
    //
    if(different && keyframeMode)
    {
        // If the keyframe time slider is out of bounds, put it back into
        // the new range.
        int state = 0, nStates = 1;
        GetTimeSliderStates(KF_TIME_SLIDER, state, nStates);
        if(state > nFrames)
        {
            debug1 << "The keyframe time slider's state was outside the "
                "acceptable range. Set its state to: " << nFrames-1 << endl;
            timeSliders[KF_TIME_SLIDER] = nFrames - 1;
        }

        // Update the plot cache sizes.
        for(int i = 0; i < nPlots; ++i)
            plots[i].plot->UpdateCacheSize(keyframeMode, keyframeMode, nKeyframes);
        ViewerWindowManager::Instance()->UpdateKeyframeAttributes();
        ViewerWindowManager::Instance()->UpdateWindowInformation(WINDOWINFO_TIMESLIDERS);
    }
}

// ****************************************************************************
// Method: ViewerPlotList::GetNKeyframes
//
// Purpose: 
//   Returns the number of keyframes.
//
// Returns:    The number of keyframes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:41:04 PST 2004
//
// Modifications:
//   
// ****************************************************************************

int
ViewerPlotList::GetNKeyframes() const
{
    return nKeyframes;
}

// ****************************************************************************
// Method: ViewerPlotList::GetNKeyframesWasUserSet
//
// Purpose: 
//   Returns whether the number of keyframes was set by the user.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 8 15:41:26 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerPlotList::GetNKeyframesWasUserSet() const
{
    return nKeyframesWasUserSet;
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
//   Brad Whitlock, Thu Dec 18 13:42:29 PST 2003
//   Added the completeSave argument to save out the SIL attributes.
//
//   Brad Whitlock, Mon Feb 2 15:19:50 PST 2004
//   I added code to save out the pipeline caching mode, keyframing mode and
//   number of frames, and the active time sliders and their states. I also
//   moved saving certain attributes to ViewerPlot::CreateNode. I removed
//   hostDatabaseName from the session file.
//
//   Jeremy Meredith, Tue Mar 30 10:39:20 PST 2004
//   Added an engine key to map plots to the engine used to create them.
//   Since plots from simulations cannot be created, they should not be
//   saved with session files.  Added extra logic to avoid saving a simulation
//   as the current active source as well.
//
// ****************************************************************************

void
ViewerPlotList::CreateNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *plotlistNode = new DataNode("ViewerPlotList");
    parentNode->AddNode(plotlistNode);

    std::string sourceToSave = "";
    if (!engineKey.IsSimulation())
        sourceToSave = databaseName;

    //
    // Save the time sliders.
    //    
    if(timeSliders.size() > 0)
    {
        DataNode *tsNode = new DataNode("timeSliders");
        for(StringIntMap::const_iterator ts = timeSliders.begin();
            ts != timeSliders.end(); ++ts)
        {
            tsNode->AddNode(new DataNode(ts->first, ts->second));
        }        
        plotlistNode->AddNode(tsNode);

        if(HasActiveTimeSlider())
            plotlistNode->AddNode(new DataNode("activeTimeSlider", activeTimeSlider));
    }

    //
    // Let all of the plots save themselves to the config file.
    //
    int numRealPlots = 0;
    for(int i = 0; i < nPlots; ++i)
    {
        if (plots[i].plot->GetEngineKey().IsSimulation())
            continue;

        if (engineKey.IsSimulation())
            sourceToSave = plots[i].plot->GetDatabaseName();

        char tmp[20];
        SNPRINTF(tmp, 20, "plot%02d", numRealPlots);
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
            plots[i].plot->GetVariableName()));
        plotNode->AddNode(new DataNode("active", plots[i].active));
        plotNode->AddNode(new DataNode("hidden", plots[i].hidden));
        plotNode->AddNode(new DataNode("realized", plots[i].realized));

        // Let the plot add its attributes to the node.
        plots[i].plot->CreateNode(plotNode);

        // Increment the plot counter
        numRealPlots++;
    }

    //
    // Add information specific to the animation.
    //
    plotlistNode->AddNode(new DataNode("hostName", hostName));
    if (sourceToSave != "")
        plotlistNode->AddNode(new DataNode("databaseName", sourceToSave));
    plotlistNode->AddNode(new DataNode("nPlots", numRealPlots));
    plotlistNode->AddNode(new DataNode("keyframeMode", keyframeMode));
    plotlistNode->AddNode(new DataNode("nKeyframes", nKeyframes));
    plotlistNode->AddNode(new DataNode("pipelineCaching", pipelineCaching));
    plotlistNode->AddNode(new DataNode("playbackMode",
        PlaybackMode_ToString(playbackMode)));
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
//   Brad Whitlock, Wed Dec 31 13:56:24 PST 2003
//   I added code to update the expression list so plots of expressions can
//   be successfully created.
//
//   Hank Childs, Wed Mar 24 08:10:16 PST 2004
//   Have SIL restriction use Compact SIL Atts in constructor, especially 
//   because topSet is now a string and not an int.
//
//   Brad Whitlock, Wed Mar 24 12:08:16 PST 2004
//   I added code to set pipelineCaching mode, keyframeMode, nKeyframes,
//   playbackMode, and the time sliders. I also moved setting of some plot
//   settings into ViewerPlot::SetFromNode.
//
//   Jeremy Meredith, Wed Mar 24 12:58:09 PST 2004
//   Since it is possible for NewPlot to throw an exception, I added
//   try/catch around it.
//
//   Jeremy Meredith, Tue Mar 30 17:18:38 PST 2004
//   Added support for simulations.
//
//   Brad Whitlock, Fri Apr 2 15:51:59 PST 2004
//   I added support for keyframing.
//
// ****************************************************************************

bool
ViewerPlotList::SetFromNode(DataNode *parentNode)
{
    DataNode *node;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    DatabaseCorrelationList *cL = fs->GetDatabaseCorrelationList();

    if(parentNode == 0)
        return false;

    DataNode *plotlistNode = parentNode->GetNode("ViewerPlotList");
    if(plotlistNode == 0)
        return false;

    // Make sure that the host, database, and host+database get set.
    if((node = plotlistNode->GetNode("hostName")) != 0)
        hostName = node->AsString();
    if((node = plotlistNode->GetNode("databaseName")) != 0)
        databaseName = node->AsString();
    if(hostName.size() > 0 && databaseName.size() > 0)
    {
        // Expand the database name.
        databaseName = fs->ExpandedFileName(hostName, databaseName);
        hostDatabaseName = fs->ComposeDatabaseName(hostName, databaseName);
        debug1 << "The active source will be: " << databaseName.c_str()
               << ", " << hostDatabaseName.c_str() << endl;
    }
    else
        hostDatabaseName = "";
    engineKey = EngineKey(hostName, "");

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
    if((node = plotlistNode->GetNode("nKeyframes")) != 0)
    {
        int nkf = node->AsInt();
        if(nkf > 0)
        {
            nKeyframes = nkf;
            nKeyframesWasUserSet = true;
        }
    }
    if((node = plotlistNode->GetNode("playbackMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetPlaybackMode(PlaybackMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PlaybackMode value;
            if(PlaybackMode_FromString(node->AsString(), value))
                SetPlaybackMode(value);
        }
    }

    //
    // Set the time slider state for each of the time sliders. They are
    // created as necessary.
    //
    DataNode *tsNode = plotlistNode->GetNode("timeSliders");
    if(tsNode != 0)
    {
        // Clear out the defined time sliders.
        timeSliders.clear();

        //
        // Define a new set of time sliders.
        //
        bool createdCorrelations = false;
        DataNode **tsNodes = tsNode->GetChildren();
        for(int tsIndex = 0; tsIndex < tsNode->GetNumChildren(); ++tsIndex)
        {
            DataNode *ts = tsNodes[tsIndex];
            if(ts->GetNodeType() == INT_NODE)
            {
                int tsState = ts->AsInt();
                tsState = (tsState >= 0) ? tsState : 0;

                //
                // Use the node key as the name of the time slider. Be sure to
                // expand the name though if the name looks like a database name.
                // If the name has a colon in it, assume that the name needs to
                // be expanded.
                //
                std::string tsHost, tsDB, tsName(ts->GetKey());
                if(tsName.find(":") != std::string::npos)
                {
                    debug3 << "Time slider before name expansion: "
                           << tsName.c_str() << endl;
                    fs->ExpandDatabaseName(tsName, tsHost, tsDB);
                    debug3 << "Time slider after name expansion: "
                           << tsName.c_str() << endl;
                }

                //
                // See if there is a database correlation for the time slider
                // that we want to define. If there isn't then we're processing
                // a pre 1.3 session file and we need to make a new correlation
                // it its file has metadata and that metadata is for a
                // time-varying file. Don't create the correlation if the
                // time slider is the keyframing time slider.
                //
                DatabaseCorrelation *correlation = cL->FindCorrelation(tsName);
                if(correlation == 0 && tsName != KF_TIME_SLIDER)
                {
                     //
                     // If the time slider database has multiple time steps then
                     // create a correlation for it.
                     //
                     const avtDatabaseMetaData *md = fs->GetMetaData(tsHost, tsDB);
                     if(md != 0 && md->GetNumStates() > 1)
                     {
                         stringVector dbs; dbs.push_back(ts->GetKey());
                         DatabaseCorrelation *c = fs->CreateDatabaseCorrelation(
                             tsName, dbs, 0, md->GetNumStates());
                         if(c)
                         {
                             debug3 << "Created correlation " << c->GetName().c_str()
                                    << " because it did not exist even though "
                                       "it should have existed." << endl;
                             cL->AddDatabaseCorrelation(*c);
                             createdCorrelations = true;
                             correlation = cL->FindCorrelation(tsName);
                         }
                     }
                }

                if(correlation != 0 || tsName == KF_TIME_SLIDER)
                {
                    debug3 << "Creating time slider \"" << tsName.c_str()
                           << "\" at state " << tsState << endl;
                    timeSliders[tsName] = tsState;
                }
                else
                {
                    debug3 << "Did not create time slider \""
                           << tsName.c_str() << "\"" << endl;
                }
            }
        }

        // If we created correlations then we need to tell the client.
        if(createdCorrelations)
            cL->Notify();
    }

    //
    // Try and set the active time slider.
    //
    DataNode *atsNode = plotlistNode->GetNode("activeTimeSlider");
    if(atsNode != 0 && atsNode->GetNodeType() == STRING_NODE)
    {
        std::string tsName(atsNode->AsString());

        //
        // Pre 1.3 session files might not have fully expanded names. If the
        // time slider name has a ":" in it then try and expand it.
        //
        if(tsName.find(":") != std::string::npos)
        {
            std::string tsHost, tsDB;
            fs->ExpandDatabaseName(tsName, tsHost, tsDB);
        }

        //
        // Set the active time slider if there is a correlation for the
        // time slider. If there is no correlation at this point, the
        // desired time slider must be for a single time state database.
        // Since that must be the case, don't set the time slider.
        //
        if(cL->FindCorrelation(tsName) != 0 || tsName == KF_TIME_SLIDER)
            SetActiveTimeSlider(tsName);
    }

    //
    // Now that the host and database have been set, update the expression
    // list so we can create plots that use expression variables without
    // having to change the open database.
    //
    std::string exprHost(hostName), exprDB(databaseName);
    UpdateExpressionListUsingDB(hostName, databaseName, 0);

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
                // Expand the plot's DB in case it contains ".."
                plotDB = fs->ExpandedFileName(plotHost, plotDB);

                //
                // Update the expression list if the plot does not use the
                // current expression database.
                //
                bool failure = false;
                if(plotHost != exprHost || plotDB != exprDB)
                {
                    TRY
                    {
                        UpdateExpressionListUsingDB(plotHost, plotDB, 0);
                        exprHost = plotHost;
                        exprDB = plotDB;
                    }
                    CATCH(VisItException)
                    {
                        failure = true;
                    }
                    ENDTRY
                }

                //
                // Try and create the plot.  (It won't be a simulation, so it
                // is safe to create an Engine Key with an empty sim name.)
                //
                ViewerPlot *plot = 0;
                if(!failure)
                {
                    TRY
                    {
                        plot = NewPlot(type,engineKey,plotHost,
                                       plotDB,plotVar,false);
                    }
                    CATCHALL(...)
                    {
                        // plot will be zero if an error occurred, so we don't
                        // need to do further error handling right here
                    }
                    ENDTRY
                }

                if(plot)
                {
                    /////////////////////// Reformat pre 1.3 session files ///////////////////
                    DataNode *vPlotNode;
                    if((vPlotNode = plotNode->GetNode("ViewerPlot")) != 0)
                    {
                        // Reparent certain items from the plot## nodes to the ViewerPlot
                        // node inside of the plot## node.
                        const char *transferNodes[] = {"beginFrame", "endFrame",
                            "CompactSILRestrictionAttributes"};
                        for(int id = 0; id < 3; ++id)
                        {
                            DataNode *transferNode;
                            if((transferNode = plotNode->GetNode(transferNodes[id])) != 0)
                            {
                                debug1 << "Moving " << transferNodes[id]
                                       << " to the ViewerPlot node." << endl;
                                plotNode->RemoveNode(transferNodes[id], false);
                                vPlotNode->AddNode(transferNode);
                            }
                            else
                            {
                                debug1 << "Could not move " << transferNodes[id]
                                       << " to the ViewerPlot node." << endl;
                            }
                        }
                    }
                    ///////////////// Done reformatting pre 1.3 session files ////////////////

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

    // If the expression list does not contain the expressions from the
    // open file, update it.
    if(hostName != exprHost || databaseName != exprDB)
    {
        UpdateExpressionList(true);
    }
    else
    {
        // It has the right contents but we need to send it to the client.
        ExpressionList *exprList = ParsingExprList::Instance()->GetList();
        exprList->Notify();
    }

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
        if (plots[i].plot->IsInRange() && !plots[i].hidden &&
            !plots[i].plot->IsMesh())
            canBeOpaque = false;
    }

    for (i = 0; i < nPlots; ++i)
        plots[i].plot->SetOpaqueMeshIsAppropriate(canBeOpaque);
}


// ****************************************************************************
//  Method:  ViewerPlotList::SetEngineKey
//
//  Purpose:
//    Tell the plot list the current engine key.
//
//  Arguments:
//    s          true if it is a simulation
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 25, 2004
//
// ****************************************************************************

void
ViewerPlotList::SetEngineKey(const EngineKey &ek)
{
    engineKey = ek;
}

// ****************************************************************************
//  Method:  ViewerPlotList::GetEngineKey
//
//  Purpose:
//    return the key for the current engine
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 25, 2004
//
// ****************************************************************************
const EngineKey &
ViewerPlotList::GetEngineKey() const
{
    return engineKey;
}

// ****************************************************************************
//  Method:  ViewerPlotList::GetNumberOfCells
//
//  Programmer:  Mark C. Miller 
//  Creation:    May 11, 2004
//
// ****************************************************************************
int
ViewerPlotList::GetNumberOfCells(bool polysOnly) const
{
    int i;
    int sum = 0;
    for (i = 0; i < nPlots; i++)
    {
        if (plots[i].realized)
        {
            avtActor_p actor = plots[i].plot->GetActor();

            if (*actor != NULL)
            {
                avtDataObject_p dob = actor->GetDataObject();

                if (*dob != NULL)
                   sum += dob->GetNumberOfCells(polysOnly);
            }
        }
    }
    return sum;
}

// ****************************************************************************
//  Method:  ViewerPlotList::GetWindowId
//
//  Programmer:  Mark C. Miller 
//  Creation:    June 8, 2004 
//
// ****************************************************************************
int
ViewerPlotList::GetWindowId() const
{
    return window->GetWindowId();
}
