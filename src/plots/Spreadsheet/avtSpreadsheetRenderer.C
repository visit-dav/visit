// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtSpreadsheetRenderer.h>

#include <vtkFieldData.h>
#include <vtkDoubleArray.h>
#include <vtkDataSet.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <avtCallback.h>

#include <avtOpenGLSpreadsheetTraceRenderer.h>

// ****************************************************************************
// Method: avtSpreadsheetRenderer::avtSpreadsheetRenderer
//
// Purpose: 
//   Constructor for the avtSpreadsheetRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:29:14 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtSpreadsheetRenderer::avtSpreadsheetRenderer() : avtCustomRenderer(), atts()
{
    plotDisplay = 0;
    rendererImplementation = 0;
    fgColor[0] = fgColor[1] = fgColor[2] = 1.;
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::~avtSpreadsheetRenderer
//
// Purpose: 
//   Destructor for the avtSpreadsheetRenderer class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:29:58 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtSpreadsheetRenderer::~avtSpreadsheetRenderer()
{
    ReleaseGraphicsResources();
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::New
//
// Purpose: 
//   Returns a new instance of the class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:30:14 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtSpreadsheetRenderer *
avtSpreadsheetRenderer::New(void)
{
    return new avtSpreadsheetRenderer;
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::ReleaseGraphicsResources
//
// Purpose: 
//   Releases the graphics resources.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 09:48:14 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetRenderer::ReleaseGraphicsResources()
{
    if (rendererImplementation != 0)
    {
        VTKRen->GetRenderWindow()->MakeCurrent();
        delete rendererImplementation;
        rendererImplementation = 0;
    }
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::SetAtts
//
// Purpose: 
//   Sets the plot attributes that will be used for rendering.
//
// Arguments:
//   ssa : The spreadsheet attributes that will be used.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:30:37 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetRenderer::SetAtts(const AttributeGroup *ssa)
{
    atts = *((SpreadsheetAttributes *)ssa);
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::SetForegroundColor
//
// Purpose: 
//   Sets the foreground so the renderer can draw the plot bounding box in the
//   correct color.
//
// Arguments:
//   fg : The rgb values for the color.
//
// Returns:    True if the foreground does not matched the cached foreground
//             color. False otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:31:07 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtSpreadsheetRenderer::SetForegroundColor(const double *fg)
{
    bool retVal = (fgColor[0] != fg[0] || 
                   fgColor[1] != fg[1] || 
                   fgColor[2] != fg[2]);
    fgColor[0] = fg[0];
    fgColor[1] = fg[1];
    fgColor[2] = fg[2];

    return retVal;
}

// ****************************************************************************
// Method: avtSpreadsheetRenderer::RenderTracePlane
//
// Purpose: 
//   Renders the trace plane for the dataset. This is the piece of the
//   Spreadsheet plot that exists in the vis window.
//
// Arguments:
//   ds : The data to be rendered by the Spreadsheet plot.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 14:32:10 PST 2007
//
// Modifications:
//   
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Thu Nov 29 18:17:50 PST 2007
//   Support showing current cell outline. Calculate bounds from data set if
//   "avtOriginalBounds" does not exist.
//
//   Brad Whitlock, Wed Jun 10 14:10:34 PST 2009
//   I made Mesa suport be conditional.
//
//   Kathleen Bonnell, Tue Nov 16 10:14:35 PST 2010
//   The distinction between Mesa and OpenGL no longer exists at the plot 
//   level.
//
//   Kathleen Biagas, Tue Oct 16 15:14:42 MST 2012
//   Create vtkDoubleArray for bounds, that's how avtOriginalBounds is created.
//
// ****************************************************************************

void
avtSpreadsheetRenderer::RenderTracePlane(vtkDataSet *ds)
{
    // Get the extents
    bool mustDeleteBounds = false;
    vtkDataArray *bounds = ds->GetFieldData()->GetArray("avtOriginalBounds");
    if (!bounds)
    {
        bounds=vtkDoubleArray::New();
        bounds->SetNumberOfTuples(6);
        for (vtkIdType i=0; i<6; ++i)
            bounds->SetTuple1(i, ds->GetBounds()[i]);
        mustDeleteBounds=true;
    }

    if(atts.GetShowTracerPlane() || atts.GetShowPatchOutline() ||
            atts.GetShowCurrentCellOutline())
    {
        if(rendererImplementation == 0)
        {
            rendererImplementation = new avtOpenGLSpreadsheetTraceRenderer;
        }
         
        if(rendererImplementation != 0)
             rendererImplementation->Render(ds, bounds, atts, fgColor);
    }

    if (mustDeleteBounds) bounds->Delete();
}

