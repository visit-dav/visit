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
        float desiredSizePixels = (viewport->GetSize()[1] * this->TextHeight);
        // This relation seems to give us good results when we measure on the screen.
        int desiredSizePoints = int(desiredSizePixels * 4.f / 3.f);
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

