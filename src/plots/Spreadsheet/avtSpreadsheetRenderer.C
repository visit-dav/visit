/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <avtSpreadsheetRenderer.h>

#include <vtkFieldData.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkDataSet.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkToolkits.h>

#include <avtCallback.h>

#include <avtOpenGLSpreadsheetTraceRenderer.h>
#ifdef VTK_USE_MANGLED_MESA
#include <avtMesaSpreadsheetTraceRenderer.h>
#endif

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
// ****************************************************************************

void
avtSpreadsheetRenderer::RenderTracePlane(vtkDataSet *ds)
{
    // Get the extents
    bool mustDeleteBounds = false;
    vtkDataArray *bounds = ds->GetFieldData()->GetArray("avtOriginalBounds");
    if (!bounds)
    {
        bounds=vtkFloatArray::New();
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
#ifdef VTK_USE_MANGLED_MESA
            if(avtCallback::GetSoftwareRendering())
                rendererImplementation = new avtMesaSpreadsheetTraceRenderer;
            else
#endif
                rendererImplementation = new avtOpenGLSpreadsheetTraceRenderer;
        }
         
        if(rendererImplementation != 0)
             rendererImplementation->Render(ds, bounds, atts, fgColor);
    }

    if (mustDeleteBounds) bounds->Delete();
}

