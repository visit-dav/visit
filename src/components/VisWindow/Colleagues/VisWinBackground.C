// ************************************************************************* //
//                                VisWinBackground.C                         //
// ************************************************************************* //

#include <vtkRenderer.h>
#include <vtkBackgroundActor.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>
#include <VisWinBackground.h>

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
//   
// ****************************************************************************

VisWinBackground::VisWinBackground(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    bgActor = vtkBackgroundActor::New();
    addedBackground = false;
}

// ****************************************************************************
//  Method: VisWinBackground destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Aug 29 15:39:16 PST 2001
//
//  Modifications:
//
// ****************************************************************************

VisWinBackground::~VisWinBackground()
{
    if (bgActor != NULL)
    {
        bgActor->Delete();
        bgActor = NULL;
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
    float c1R, float c1G, float c1B, float c2R, float c2G, float c2B)
{
    // Set the colors.
    float c1[3], c2[3];
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
//   
// ****************************************************************************

void
VisWinBackground::SetBackgroundMode(int mode)
{
    if(mode == 0)
        RemoveBackgroundFromWindow();
    else
        AddBackgroundToWindow();
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
//
// ****************************************************************************

void
VisWinBackground::AddBackgroundToWindow(void)
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
    background->AddActor2D(bgActor);
    bgActor->SetVisibility(true);

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
//
// ****************************************************************************

void
VisWinBackground::RemoveBackgroundFromWindow(void)
{
    if (! addedBackground)
    {
        return;
    }

    vtkRenderer *background = mediator.GetBackground();
    background->RemoveActor2D(bgActor);

    addedBackground = false;
}

