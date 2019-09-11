// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtSpreadsheetRenderer.h>
#include <SpreadsheetViewer.h>

//
// This file contains viewer-specific versions of avtSpreadsheetRenderer methods.
//

// ****************************************************************************
// Method: avtSpreadsheetRenderer::SetAlternateDisplay
//
// Purpose: 
//   Associates an alternate display with this renderer object.
//
// Arguments:
//   dpy : The handle to the alternate display.
//
// Note:       The alternate display is tied to the renderer using this
//             method since the alternate display exists for the duration of
//             the life of the ViewerPlot object with which it is associated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:04:17 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetRenderer::SetAlternateDisplay(void *dpy)
{
    plotDisplay = dpy;
    if(plotDisplay != 0)
    {
        ((SpreadsheetViewer *)plotDisplay)->setAllowRender(true);
    }
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::Render
//
// Purpose: 
//   This method is called when the vis window needs to render the input dataset.
//
// Arguments:
//   ds : The dataset to be rendered.
//
// Note:       This method renders the dataset using the trace plane renderer
//             and also allows the alternate display to render the data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:06:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetRenderer::Render(vtkDataSet *ds)
{
    // Make the alternate display render the data too.
    if(plotDisplay != 0)
    {
        ((SpreadsheetViewer *)plotDisplay)->render(ds);
    }

    // Render the trace plane.
    RenderTracePlane(ds);
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::SetColorTable
//
// Purpose: 
//   This method lets the alternate display determine whether it needs to
//   update its colors in response to a colortable-changing event.
//
// Arguments:
//   ctName : The name of the color table.
//
// Returns:    True if the display needs to update.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 10:07:17 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtSpreadsheetRenderer::SetColorTable(const char *ctName)
{
    bool ret = false;

    if(plotDisplay != 0)
    {
        ret = ((SpreadsheetViewer *)plotDisplay)->setColorTable(ctName);
    }

    return ret;
}
