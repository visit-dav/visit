// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtSpreadsheetRenderer.h>

//
// This file contains engine-specific versions of avtSpreadsheetRenderer methods.
//

// ****************************************************************************
// Method: avtSpreadsheetRenderer::SetAlternateDisplay
//
// Purpose: 
//   The engine has no alternate display so set it to 0.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:01:43 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetRenderer::SetAlternateDisplay(void *)
{
    plotDisplay = 0;
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::Render
//
// Purpose: 
//   Renders the trace plane.
//
// Arguments:
//   ds : The dataset whose trace plane will be drawn.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:01:43 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetRenderer::Render(vtkDataSet *ds)
{
    RenderTracePlane(ds);
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::SetColorTable
//
// Purpose: 
//   The color table is not used because the engine has no alternate display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:02:39 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtSpreadsheetRenderer::SetColorTable(const char *)
{
    return false;
}
