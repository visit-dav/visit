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
// ****************************************************************************

VisWinUserInfo::VisWinUserInfo(VisWindowColleagueProxy &p) 
    : VisWinColleague(p)
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
    {
        user = getenv("LOGNAME");
    }
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
    infoString = new char[strlen("user: ") + strlen(user) + strlen("\n") 
                          + strlen(current_time) + 1];
    sprintf(infoString, "user: %s\n%s", user, current_time);

    //
    // Create and position the actors.
    //
    infoActor = vtkTextActor::New();
    infoActor->ScaledTextOn();
    infoActor->SetInput(infoString);
    vtkCoordinate *pos = infoActor->GetPositionCoordinate();
    pos->SetCoordinateSystemToNormalizedViewport();
    pos->SetValue(0.75, 0.015, 0.);
    infoActor->SetWidth(0.2);
    infoActor->SetHeight(0.1);

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
// ****************************************************************************

void
VisWinUserInfo::SetForegroundColor(float fr, float fg, float fb)
{
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
