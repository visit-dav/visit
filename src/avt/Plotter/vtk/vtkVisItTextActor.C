// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <vtkVisItTextActor.h>

#include <vtkObjectFactory.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>
#include <vtkWindow.h>

vtkStandardNewMacro(vtkVisItTextActor);

vtkVisItTextActor::vtkVisItTextActor() : vtkTextActor()
{
    this->TextHeight = 0.03f;
    this->SetTextScaleModeToViewport();
}

vtkVisItTextActor::~vtkVisItTextActor()
{
}

// ****************************************************************************
// Method: vtkVisItTextActor::SetTextHeight
//
// Purpose: 
//   Set the text height as a percentage of the viewport height.
//
// Arguments:
//   val : The viewport height.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 19 15:23:27 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
vtkVisItTextActor::SetTextHeight(float val)
{
    this->TextHeight = val;
    // Just put a value into the text properties too since that will fool
    // the actor into changing when we render it later.
    this->TextProperty->SetFontSize(int(val * 100));
    // some changes in vtkTextActor require the call to 'Modified' to preserve
    // our expected behavior.
    this->TextProperty->Modified();
}

// ****************************************************************************
// Method: vtkVisItTextActor::ComputeScaledFont
//
// Purpose: 
//   Compute the scaled font to use based on the viewport size.
//
// Arguments:
//   viewport : the viewport that is rendering the text.
//
// Returns:    
//
// Note:       We only have custom code for the TEXT_SCALE_MODE_VIEWPORT case
//             and we use the parent class' code for other cases.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 19 15:22:18 PDT 2011
//
// Modifications:
//
//   Cyrus Harrison, Mon Nov 10 10:13:28 PST 2025
//   Changed scaling heurstic to be DPI aware.
//
// ****************************************************************************

void
vtkVisItTextActor::ComputeScaledFont(vtkViewport *viewport)
{
  // Override the behavior for viewport scaling mode.
  if (this->TextScaleMode == TEXT_SCALE_MODE_VIEWPORT)
    {
    if (this->TextProperty != NULL &&
        this->ScaledTextProperty->GetMTime() < this->TextProperty->GetMTime())
      {
        this->ScaledTextProperty->ShallowCopy(this->TextProperty);
      }

    if (   (viewport->GetMTime() > this->BuildTime)
        || (   viewport->GetVTKWindow()
            && (viewport->GetVTKWindow()->GetMTime() > this->BuildTime) )
        || (   this->TextProperty
            && (this->TextProperty->GetMTime() > this->BuildTime) ) )
      {
        int desiredSizePoints = GetDPIScaledFontSize(viewport,this->TextHeight);
        this->ScaledTextProperty->SetFontSize(desiredSizePoints);

        // An heuristic for the shadow offset. We can get rid of this if we
        // ever expose shadow offset in the text attributes.
        int offsetX = desiredSizePoints / 40;
        if(offsetX < 2) offsetX = 2;
        int offsetY = -offsetX / 2;
        this->ScaledTextProperty->SetShadowOffset(offsetX, offsetY);
      }
    return;
    }
  else
    {
       // For other scaling modes, just call the parent class.
       vtkTextActor::ComputeScaledFont(viewport);
    }
}

// ****************************************************************************
// Method: vtkVisItTextActor::RenderOpaqueGeometry
//
// Purpose:
//   Render text, make sure viewport changes are refle
//
// Arguments:
//   viewport    Pointer to vtk viewport
//
// Modifications:
//
// ****************************************************************

int vtkVisItTextActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  // the text size is relative to the viewport
  // so if the viewport has changed, our text has as well
  if(viewport->GetMTime() > this->GetMTime())
  {
    this->Modified();
  }
  return vtkTextActor::RenderOpaqueGeometry(viewport);
}

// ****************************************************************************
// Method: vtkVisItTextActor::GetDPIScaledFontSize
//
// Purpose:
//   Helper for font size scaling that takes into account DPI settings
//
// Arguments:
//   viewport    Pointer to vtk viewport
//   fontHeight  Desired font height
//
// Returns:  Scaled font size
//
// Programmer: Cyrus Harrison
// Creation:   Mon Nov 10 11:03:28 PST 2025
//
// Modifications:
//   Cyrus Harrison, Wed Nov 19 11:13:41 PST 2025
//   Use direct scaling, avoid extra 1.05 scaling.
//
// ****************************************************************************
double
vtkVisItTextActor::GetDPIScaledFontSize(vtkViewport *viewport,
                                        double fontHeight)
{
  // Desired size seems relative to common dpi (72)
  // divide DPI to avoid extra large fonts
  vtkWindow *win = viewport->GetVTKWindow();
  return (viewport->GetSize()[1] * fontHeight * 72.0 / double(win->GetDPI()));;
}
