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
//                              VisWinUserInfo.C                             //
// ************************************************************************* //

#include <time.h>

#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <VisWinUserInfo.h>
#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#if defined(_WIN32)
#include <windows.h>
#endif

const float VisWinUserInfo::defaultUserInfoHeight = 0.1;
const float VisWinUserInfo::defaultUserInfoWidth = 0.2;

// ****************************************************************************
//  Method: VisWinUserInfo constructor
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//    Hank Childs, Thu Jul  6 10:48:11 PDT 2000
//    Removed code to add the user info, since that is now done elsewhere.
//
//    Brad Whitlock, Tue Apr 16 17:15:51 PST 2002
//    Added a Windows implementation.
//
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002  
//    infoActor now a vtkTextActor instead of vtkScaledTextActor. infoMapper
//    no longer necessary (new vtk api). 
//
//    Brad Whitlock, Wed Oct 29 08:48:17 PDT 2003
//    I moved some code into the UpdateUserText method.
//
//    Brad Whitlock, Wed Jan 30 15:21:05 PST 2008
//    Added textAttributes. Moved size and placement code to UpdateUserText.
//
// ****************************************************************************

VisWinUserInfo::VisWinUserInfo(VisWindowColleagueProxy &p) 
    : VisWinColleague(p), textAttributes()
{
    infoString = NULL;

    //
    // Create and position the actors.
    //
    infoActor = vtkTextActor::New();
    infoActor->ScaledTextOn();
    UpdateUserText();

    //
    // This user info will actually be added when we determine that the
    // VisWindow has plots.
    //
    addedUserInfo = false;
}


// ****************************************************************************
//  Method: VisWinUserInfo destructor
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    infoMapper no longer required.
//
// ****************************************************************************

VisWinUserInfo::~VisWinUserInfo()
{
    if (infoActor != NULL)
    {
        infoActor->Delete();
        infoActor = NULL;
    }
    if (infoString != NULL)
    {
        delete [] infoString;
        infoString = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinUserInfo::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color of the user info actor.
//
//  Arguments:
//      fr      The red (rgb) foreground color.
//      fg      The green (rgb) foreground color.
//      fb      The blue (rgb) foreground color.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 14:07:15 PST 2002 
//    Use new vtkTextProperty.
//
//    Brad Whitlock, Wed Jan 30 15:54:53 PST 2008
//    Only use foreground color if we're supposed to.
//
// ****************************************************************************

void
VisWinUserInfo::SetForegroundColor(double fr, double fg, double fb)
{
    if(textAttributes.useForegroundColor)
        infoActor->GetTextProperty()->SetColor(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinUserInfo::AddToWindow
//
//  Purpose:
//      Adds the user info to the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Brad Whitlock, Wed Nov 1 10:36:25 PDT 2000
//    Made it appear in the foreground instead of the background.
//
//    Hank Childs, Thu Aug  9 14:01:02 PDT 2001
//    Don't add the actor if we don't want annotation.
//
// ****************************************************************************

void
VisWinUserInfo::AddToWindow(void)
{
    //
    // Add the actor to the background.
    //
    vtkRenderer *foreground = mediator.GetForeground();
#ifndef NO_ANNOTATIONS
    foreground->AddActor2D(infoActor);
#endif

    addedUserInfo = true;
}


// ****************************************************************************
//  Method: VisWinUserInfo::RemoveFromWindow
//
//  Purpose:
//      Removes the user info from the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Brad Whitlock, Wed Nov 1 10:37:08 PDT 2000
//    Made it appear in the foreground instead of the background.
//
//    Hank Childs, Thu Aug  9 14:01:02 PDT 2001
//    Don't add the actor if we don't want annotation.
//
// ****************************************************************************

void
VisWinUserInfo::RemoveFromWindow(void)
{
    //
    // Remove the actor from the background.
    //
    vtkRenderer *foreground = mediator.GetForeground();
#ifndef NO_ANNOTATIONS
    foreground->RemoveActor2D(infoActor);
#endif

    addedUserInfo = false;
}


// ****************************************************************************
//  Method: VisWinUserInfo::HasPlots
//
//  Purpose:
//      Does the appropriate thing when we know the vis window has plots.  For
//      the user info, this means adding it to the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinUserInfo::HasPlots(void)
{
    AddToWindow();
}


// ****************************************************************************
//  Method: VisWinUserInfo::NoPlots
//
//  Purpose:
//      Does the appropriate thing when we know the vis window does not have
//      plots.  For the user info, this means removing it from the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinUserInfo::NoPlots(void)
{
    RemoveFromWindow();
}


// ****************************************************************************
// Method: VisWinUserInfo::UpdatePlotList
//
// Purpose: 
//   Updates the user information and time when the plot list changes.
//
// Arguments:
//   p : A vector of reference pointers to plot objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 08:50:08 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinUserInfo::UpdatePlotList(std::vector<avtActor_p> &p)
{
    if(p.size() > 0)
        UpdateUserText();
}

// ****************************************************************************
// Method: VisWinUserInfo::SetVisibility
//
// Purpose: 
//   Sets the visibility of the user information.
//
// Arguments:
//   val : Whether or not the string is visible.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 08:17:19 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinUserInfo::SetVisibility(bool val)
{
    infoActor->SetVisibility(val?1:0);
}

// ****************************************************************************
// Method: VisWinUserInfo::UpdateUserText
//
// Purpose: 
//   Updates the user and time string.
//
// Programmer: Hank Childs
// Creation:   Wed Oct 29 08:45:44 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Oct 29 08:45:55 PDT 2003
//   I moved this code from the constructor to this function, which can be
//   called again and again.
//
//   Brad Whitlock, Wed Jan 30 16:00:48 PST 2008
//   Added code to set the text size.
//
//   Brad Whitlock, Wed Mar 26 14:41:55 PDT 2008
//   Changed height to scale in the text attributes.
//
//   Brad Whitlock, Thu Jun 11 11:18:13 PDT 2009
//   If we can't get the username, use the string "user" as the username so
//   the code doesn't crash.
//
// ****************************************************************************

void
VisWinUserInfo::UpdateUserText()
{
    if(infoActor)
    {
        //
        // Get the user name.
        //
        char *user = NULL;
#if defined(_WIN32)
        char username[100];
        DWORD maxLen = 100;
        GetUserName((LPTSTR)username, (LPDWORD)&maxLen);
        user = username;
#else
        user = getenv("USER");
        if (user == NULL)
            user = getenv("LOGNAME");
        if(user == NULL)
            user = "user";
#endif

        //
        // Get the date.
        //
        time_t  binary_time  = time(0);
        char   *current_time = ctime(&binary_time);
        current_time[strlen(current_time)-1] = '\0';

        //
        // Set the mapper to have a combined string separated by a new line.  This
        // makes relative positioning _much_ easier.
        //
        delete [] infoString;
        infoString = new char[strlen("user: ") + strlen(user) + strlen("\n") 
                              + strlen(current_time) + 1];
        sprintf(infoString, "user: %s\n%s", user, current_time);
        infoActor->SetInput(infoString);

        // Place the user info based on its size.
        float scale = textAttributes.scale;
        vtkCoordinate *pos = infoActor->GetPositionCoordinate();
        pos->SetCoordinateSystemToNormalizedViewport();
        pos->SetValue(1. - ((defaultUserInfoWidth * scale) + 0.05), 0.015, 0.);
        infoActor->SetWidth(defaultUserInfoWidth * scale);
        infoActor->SetHeight(textAttributes.scale * 2.);
    }
}

// ****************************************************************************
// Method: VisWinUserInfo::SetTextAttributes
//
// Purpose: 
//   Sets the user info's text properties.
//
// Arguments:
//   textAtts : The new text attributes.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:57:27 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisWinUserInfo::SetTextAttributes(const VisWinTextAttributes &textAtts)
{
    textAttributes = textAtts;

    // Update the actor's text color.
    if(textAttributes.useForegroundColor)
    {
        double color[3];
        mediator.GetForegroundColor(color);
        infoActor->GetTextProperty()->SetColor(color[0],color[1],color[2]);
        infoActor->GetTextProperty()->SetOpacity(1.);
    }
    else
    {
        infoActor->GetTextProperty()->SetColor(
            textAttributes.color[0],
            textAttributes.color[1],
            textAttributes.color[2]);
        infoActor->GetTextProperty()->SetOpacity(textAttributes.color[3]);
    }

    infoActor->GetTextProperty()->SetFontFamily((int)textAttributes.font);
    infoActor->GetTextProperty()->SetBold(textAttributes.bold?1:0);
    infoActor->GetTextProperty()->SetItalic(textAttributes.italic?1:0);
}
