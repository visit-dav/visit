// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                avtCallback.C                              //
// ************************************************************************* //

#include <avtCallback.h>

// For NULL
#include <stdio.h>

#include <DebugStream.h>


using std::string;


WarningCallback               avtCallback::warningCallback = NULL;
void                         *avtCallback::warningCallbackArgs = NULL;

ResetTimeoutCallback          avtCallback::resetTimeoutCallback = NULL;
void                         *avtCallback::resetTimeoutCallbackArgs = NULL;

WindowAttributes              avtCallback::windowAtts;
LightList                     avtCallback::lightList;

bool                          avtCallback::nowinMode = false;
bool                          avtCallback::nowinInteractionMode = false;
bool                          avtCallback::swRendering = false;
bool                          avtCallback::safeMode = false;
#ifdef VISIT_OSPRAY
bool                          avtCallback::useOSPRay = false;
#endif

GlobalAttributes::BackendType avtCallback::backendType = GlobalAttributes::VTK;

GetDatabaseCallback           avtCallback::getDatabaseCallback = NULL;
void                         *avtCallback::getDatabaseCallbackArgs = NULL; 

bool                          avtCallback::haveRenderingException = false;
std::string                   avtCallback::renderingExceptionMessage;

std::string                   avtCallback::auxSessionKey;


// ****************************************************************************
//  Method: avtCallback::RegisterWarningCallback
//
//  Purpose:
//      Registers a callback that all data object sources can issues warnings
//      through.
//
//  Arguments:
//      wc      The warning callback.
//      args    The arguments to the warning callback.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
// ****************************************************************************

void
avtCallback::RegisterWarningCallback(WarningCallback wc, void *args)
{
    warningCallback     = wc;
    warningCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtCallback::IssueWarning
//
//  Purpose:
//      Issues a warning through static functions.  Provided as a convenience
//      to derived types and to provide a single point of source.
//
//  Arguments:
//      msg     The message to issue for the warning.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//
//    Mark C. Miller, Wed Jul 21 09:51:18 PDT 2004
//    Made it return true if it was able to issue the warning and false
//    otherwise
//
// ****************************************************************************

bool
avtCallback::IssueWarning(const char *msg)
{
    if (warningCallback != NULL)
    {
        warningCallback(warningCallbackArgs, msg);
        return true;
    }
    else
    {
        debug1 << "Would like to have issued warning \"" << msg 
               << "\", but no callback was registered." << endl;
        return false;
    }
}


// ****************************************************************************
//  Method: avtCallback::RegisterResetTimeoutCallback
//
//  Purpose:
//      Registers a callback that allows AVT modules to reset the timeout.
//
//  Arguments:
//      rtc     The reset timeout callback.
//      args    The arguments to the reset timeout callback.
//
//  Programmer: Hank Childs
//  Creation:   September 5, 2006
//
// ****************************************************************************

void
avtCallback::RegisterResetTimeoutCallback(ResetTimeoutCallback wc, void *args)
{
    resetTimeoutCallback     = wc;
    resetTimeoutCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtCallback::ResetTimeout
//
//  Purpose:
//      Resets the timeout.
//
//  Arguments:
//      secs    The number of seconds to reset the timer to.
//
//  Programmer: Hank Childs
//  Creation:   September 5, 2006
//
// ****************************************************************************

void
avtCallback::ResetTimeout(int secs)
{
    if (resetTimeoutCallback != NULL)
        resetTimeoutCallback(resetTimeoutCallbackArgs, secs);
    else
        debug1 << "Would like to have reset the timeout"
               << ", but no callback was registered." << endl;
}


// ****************************************************************************
//  Method: avtCallback::SetBackendType
//
//  Purpose:
//
//  Programmer: Cameron Christensen
//  Creation:   July 03, 2014
//
// ****************************************************************************

void
avtCallback::SetBackendType(GlobalAttributes::BackendType type)
{
    backendType = type; 
}


// ****************************************************************************
//  Method: avtCallback::SetBackendType
//
//  Purpose:
//
//  Programmer: Cameron Christensen
//  Creation:   July 03, 2014
//
// ****************************************************************************

GlobalAttributes::BackendType
avtCallback::GetBackendType() 
{
    return backendType;
}


// ****************************************************************************
//  Method: avtCallback::GetCurrentWindowAtts
//
//  Purpose:
//      Gets the current window attributes (assumes that those attributes
//      have been previously registered; that is typically done in avtPlot).
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

const WindowAttributes &
avtCallback::GetCurrentWindowAtts(void)
{
    return windowAtts;
}


// ****************************************************************************
//  Method: avtCallback::SetCurrentWindowAtts
//
//  Purpose:
//      Sets the current window attributes.  This is typically done by avtPlot.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
avtCallback::SetCurrentWindowAtts(const WindowAttributes &atts)
{
    windowAtts = atts;
}


// ****************************************************************************
//  Method: avtCallback::GetCurrentLightList
//
//  Purpose:
//      Gets the current light list.
//
//  Programmer: Jeremy Meredith
//  Creation:   October  2, 2003
//
// ****************************************************************************

const LightList &
avtCallback::GetCurrentLightList(void)
{
    return lightList;
}


// ****************************************************************************
//  Method: avtCallback::SetCurrentLightList
//
//  Purpose:
//      Sets the current light list.
//
//  Programmer: Jeremy Meredith
//  Creation:   October  2, 2003
//
// ****************************************************************************

void
avtCallback::SetCurrentLightList(const LightList &l)
{
    lightList = l;
}

// ****************************************************************************
//  Method: avtCallback::RegisterGetDatabaseCallback
//
//  Purpose:
//      Registers a callback that can be called to get a database.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

void
avtCallback::RegisterGetDatabaseCallback(GetDatabaseCallback gdc, 
                                         void *gdcArgs)
{
    getDatabaseCallback     = gdc;
    getDatabaseCallbackArgs = gdcArgs;
}


// ****************************************************************************
//  Method: avtCallback::GetDatabaseCallback
//
//  Purpose:
//      A callback that gets a database.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

ref_ptr<avtDatabase>
avtCallback::GetDatabase(const string &filename, int time, const char *format)
{
    if (getDatabaseCallback == NULL)
    {
        debug1 << "Unable to get a database since no callback has "
               << "been registered." << endl;
        return NULL;
    }

    return getDatabaseCallback(getDatabaseCallbackArgs, filename, time,format);
}


