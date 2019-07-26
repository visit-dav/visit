// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                VisWinBackground.C                         //
// ************************************************************************* //

#include <vtkRenderer.h>
#include <vtkBackgroundActor.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkTexture.h>
#include <vtkTexturedBackgroundActor.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>
#include <VisWinBackground.h>

#include <avtCallback.h>
#include <DebugStream.h>
#include <snprintf.h>

// Static
bool VisWinBackground::sphereModeError1 = false;
bool VisWinBackground::sphereModeError2 = false;

// ****************************************************************************
//  Method: VisWinBackground constructor
//
//  Arguments:
//      p      A proxy that allows more access to the VisWindow for this 
//             colleague.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 29 15:39:16 PST 2001
//
//  Modifications:
//   Brad Whitlock, Fri Nov 16 11:02:20 PST 2007
//   Added support for background images.
//
// ****************************************************************************

VisWinBackground::VisWinBackground(VisWindowColleagueProxy &p) :
    VisWinColleague(p)
{
    bgActor = vtkBackgroundActor::New();
    textureActor = vtkTexturedBackgroundActor::New();
    addedBackground = false;
    visible = true;
}

// ****************************************************************************
//  Method: VisWinBackground destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 29 15:39:16 PST 2001
//
//  Modifications:
//    Brad Whitlock, Fri Nov 16 11:02:00 PST 2007
//    Added support for background images.
//
// ****************************************************************************

VisWinBackground::~VisWinBackground()
{
    if (bgActor != NULL)
    {
        bgActor->Delete();
        bgActor = NULL;
    }

    if(textureActor != 0)
    {
        textureActor->Delete();
        textureActor = 0;
    }
}

// ****************************************************************************
// Method: VisWinBackground::SetGradientBackgroundColors
//
// Purpose: 
//   Sets the gradient colors used in the colleague's actor.
//
// Arguments:
//   gradStyle : The style of the gradient.
//   c1*       : The color components for color 1.
//   c2*       : The color components for color 2.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 15:49:48 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinBackground::SetGradientBackgroundColors(int gradStyle,
    double c1R, double c1G, double c1B, double c2R, double c2G, double c2B)
{
    // Set the colors.
    double c1[3], c2[3];
    c1[0] = c1R;
    c1[1] = c1G;
    c1[2] = c1B;
    c2[0] = c2R;
    c2[1] = c2G;
    c2[2] = c2B;
    switch(gradStyle)
    {
    case 0: // Top to bottom
        bgActor->SetColor(0, c2);
        bgActor->SetColor(1, c2);
        bgActor->SetColor(2, c1);
        bgActor->SetColor(3, c1);
        break;
    case 1: // Bottom to top
        bgActor->SetColor(0, c1);
        bgActor->SetColor(1, c1);
        bgActor->SetColor(2, c2);
        bgActor->SetColor(3, c2);
        break;
    case 2: // Left to right
        bgActor->SetColor(0, c1);
        bgActor->SetColor(1, c2);
        bgActor->SetColor(2, c2);
        bgActor->SetColor(3, c1);
        break;
    case 3: // Right to left
        bgActor->SetColor(0, c2);
        bgActor->SetColor(1, c1);
        bgActor->SetColor(2, c1);
        bgActor->SetColor(3, c2);
        break;
    case 4: // Radial
        bgActor->SetColor(0, c1);
        bgActor->SetColor(1, c2);
        break;
    }

    // Set the gradient fill mode.
    bgActor->SetGradientFillMode(gradStyle);
    bgActor->Modified();
}

// ****************************************************************************
// Method: VisWinBackground::UpdatePlotList
//
// Purpose: 
//   Updates the background mode when the plot list changes. This ensures that
//   we can successfully shift into image sphere mode from when we did not have
//   plots.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 19 17:56:25 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
VisWinBackground::UpdatePlotList(std::vector<avtActor_p> &)
{
    // Update the background in image sphere mode in case we were unable to
    // originally draw the image sphere due to having no plots.
    if(mediator.GetBackgroundMode() == 3)
        SetBackgroundMode(mediator.GetBackgroundMode());
}

// ****************************************************************************
// Method: VisWinBackground::SetBackgroundMode
//
// Purpose: 
//   Sets the background type. This is 0 for solid in which case the actor
//   is removed from the renderer. Otherwise, it means that the actor is added
//   to the renderer.
//
// Arguments:
//   mode : The new background mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 15:48:18 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Nov 16 11:01:45 PST 2007
//   Changed so it supports background images.
//
// ****************************************************************************

void
VisWinBackground::SetBackgroundMode(int mode)
{
    RemoveBackgroundFromWindow();
    if(mode > 0)
        AddBackgroundToWindow(mode);
}

// ****************************************************************************
// Method: VisWinBackground::SetBackgroundImage
//
// Purpose: 
//   Sets the background image for the colleague to use.
//
// Arguments:
//   imgFile : The name of the image file.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 16 10:56:59 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
VisWinBackground::SetBackgroundImage(const std::string &imgFile, int nx, int ny)
{
    if(textureActor->SetTextureAndRenderers(imgFile.c_str(),
        mediator.GetBackground(), mediator.GetCanvas()) == -1 &&
        imgFile.size() > 0)
    {
        char msg[1024];
        SNPRINTF(msg, 1024, "VisIt could not read the image file %s"
            " for use as a background image. No image background will be displayed.",
            imgFile.c_str());
        avtCallback::IssueWarning(msg);
    } 

    // Set the number of times the image repeats in x,y.
    textureActor->SetImageRepetitions(nx, ny);
}

// ****************************************************************************
//  Method: VisWinBackground::AddBackgroundToWindow
//
//  Purpose:
//      Adds the Background to the window.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 29 15:39:59 PST 2001
//
//  Modifications:
//    Brad Whitlock, Fri Nov 16 11:01:18 PST 2007
//    Added background image support.
//
// ****************************************************************************

void
VisWinBackground::AddBackgroundToWindow(int mode)
{
    if (addedBackground)
    {
        return;
    }

    //
    // Add the Background to the background (note that we are using a different
    // renderer's camera -- the canvas').
    //
    vtkRenderer *background = mediator.GetBackground();
    if(mode == 1)
    {
        background->AddActor2D(bgActor);
        bgActor->SetVisibility(true);
    }
    else if(mode == 2)
    {
        background->AddActor2D(textureActor);
        textureActor->SetVisibility(true);
        textureActor->SetSphereMode(false);
    }
    else if(mode == 3)
    {
        background->AddActor2D(textureActor);
        textureActor->SetVisibility(true);
        bool doSphereMode = true;

        if(mediator.GetMode() != WINMODE_3D)
        {
            if(!sphereModeError1)
            {
                avtCallback::IssueWarning("Image sphere background mode "
                    "may only be used with 3D plots. In the meantime, the "
                    "2D image background mode will be used instead.");
                sphereModeError1 = true;
            }
            doSphereMode = false;
        }

        if(!mediator.HasPlots())
        {
#if 0
            if(!sphereModeError2)
            {
                avtCallback::IssueWarning("Image sphere background mode "
                    "may only be used when there are 3D plots in the visualization"
                    " window. Until 3D plots are drawn, the 2D image background "
                    "will be used instead.");
                sphereModeError2 = true;
            }
#endif
            doSphereMode = false;
        }
        textureActor->SetSphereMode(doSphereMode);
    }

    addedBackground = true;
}


// ****************************************************************************
//  Method: VisWinBackground::RemoveBackgroundFromWindow
//
//  Purpose:
//      Removes the Background from the window.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 29 15:39:59 PST 2001
//
//  Modifications:
//   Brad Whitlock, Fri Nov 16 11:01:00 PST 2007
//   Added background image support.
//
// ****************************************************************************

void
VisWinBackground::RemoveBackgroundFromWindow()
{
    if (! addedBackground)
    {
        return;
    }

    vtkRenderer *background = mediator.GetBackground();
    background->RemoveActor2D(bgActor);
    background->RemoveActor2D(textureActor);

    addedBackground = false;
}

void
VisWinBackground::SetVisibility(int vis)
{
    bool bvis = vis > 0;
    if(bvis != visible)
    {
        bgActor->SetVisibility(bvis);
        textureActor->SetVisibility(bvis);
        visible = bvis;
    }
}

int
VisWinBackground::GetVisibility() const
{
    return visible ? 1 : 0;
}
