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

ImageCallback                 avtCallback::imageCallback = NULL;
void                         *avtCallback::imageCallbackArgs = NULL;

WindowAttributes              avtCallback::windowAtts;
LightList                     avtCallback::lightList;

bool                          avtCallback::nowinMode = false;
bool                          avtCallback::swRendering = false;

UpdatePlotAttributesCallback  avtCallback::updatePlotAttributesCallback = NULL;
void                         *avtCallback::updatePlotAttributesCallbackArgs 
                                                                        = NULL;

GetDatabaseCallback           avtCallback::getDatabaseCallback = NULL;
void                         *avtCallback::getDatabaseCallbackArgs = NULL; 

bool                          avtCallback::haveRenderingException = false;
std::string                   avtCallback::renderingExceptionMessage;


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
//  Method: avtCallback::RegisterImageCallback
//
//  Purpose:
//      Registers a callback that allows a plot to get a new image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtCallback::RegisterImageCallback(ImageCallback ic, void *ica)
{
    imageCallback     = ic;
    imageCallbackArgs = ica;
}


// ****************************************************************************
//  Method: avtCallback::GetImage
//
//  Purpose:
//      Goes through the image callback to get the new image.
//
//  Arguments:
//      index   The plot index.
//      dob     A place to put the data object.
//
//  Returns:    A void * representation of the image.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2001
//
// ****************************************************************************

void
avtCallback::GetImage(int index, avtDataObject_p &dob)
{
    if (imageCallback != NULL)
    {
        imageCallback(imageCallbackArgs, index, dob);
    }
    else
    {
        debug1 << "Would like to have gotten a new image, but no callback "
               << "was registered." << endl;
        dob = NULL;
    }
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
//  Method: avtCallback::RegisterUpdatePlotAttributesCallback
//
//  Purpose:
//      Registers a callback that can be called to update plot attributes.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
// ****************************************************************************

void
avtCallback::RegisterUpdatePlotAttributesCallback(
                             UpdatePlotAttributesCallback upac, void *upacArgs)
{
    updatePlotAttributesCallback     = upac;
    updatePlotAttributesCallbackArgs = upacArgs;
}


// ****************************************************************************
//  Method: avtCallback::UpdatePlotAttributes
//
//  Purpose:
//      Registers a callback that can be called to update plot attributes.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2001
//
// ****************************************************************************

void
avtCallback::UpdatePlotAttributes(const string &str, int index,
                                  AttributeSubject *atts)
{
    if (updatePlotAttributesCallback == NULL)
    {
        debug1 << "Unable to update plot attributes since no callback has "
               << "been registered." << endl;
        return;
    }

    updatePlotAttributesCallback(updatePlotAttributesCallbackArgs, str, 
                                 index, atts);
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


