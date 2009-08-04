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
