/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <math.h>
#include <VisWinTools.h>
#include <VisWindowColleagueProxy.h>

#include <VisitInteractiveTool.h>
#include <VisitBoxTool.h>
#include <VisitLineTool.h>
#include <VisitPlaneTool.h>
#include <VisitPointTool.h>
#include <VisitSphereTool.h>

#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkUnsignedCharArray.h>

#include <BadIndexException.h>

// ****************************************************************************
// Class: vtkHighlightActor2D
//
// Purpose:
//   This is a private class that is used in the VisWinTools colleague as an
//   actor that is added to the foreground renderer to show the locations of
//   all of the hotpoints in the enabled tools.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 11:38:36 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Thu Dec 12 15:12:36 PST 2002 
//   Use RenderOverlay instead of RenderOpaqueGeometry, to match new VTK API. 
//   
// ****************************************************************************

class vtkHighlightActor2D : public vtkActor2D
{
public:
    vtkTypeMacro(vtkHighlightActor2D, vtkActor2D);

    // Description:
    // Instantiate a hotpoints actor. 
    static vtkHighlightActor2D *New();

    // Description:
    // Draw the actor as per the vtkProp superclass' API.
    virtual int RenderOpaqueGeometry(vtkViewport *viewport) { return 0; };
    virtual int RenderTranslucentGeometry(vtkViewport *) {return 0; };
    virtual int RenderOverlay(vtkViewport *); 

    void SetToolPointer(VisitInteractiveTool **t);
    void SetNumTools(int n);
    void SetHelperRenderer(vtkRenderer *ren);
protected:
    vtkHighlightActor2D();
    virtual ~vtkHighlightActor2D();
    vtkHighlightActor2D(const vtkHighlightActor2D &) {};
    void operator=(const vtkHighlightActor2D &) {};

    void RegenerateHighlight();

    vtkActor2D           *highlightActor;
    vtkPolyData          *highlightData;
    vtkPolyDataMapper2D  *highlightMapper;
    vtkRenderer          *helperRenderer;

    // These must be updated by the Colleague when it changes its fields.
    VisitInteractiveTool **tools;
    int                    numTools;
};

// ****************************************************************************
// Method: VisWinTools::VisWinTools
//
// Purpose: 
//   Constructor for the VisWinTools class.
//
// Arguments:
//   v : A reference to a VisWindowColleagueProxy object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:42:25 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri May 3 11:07:48 PDT 2002
//   Added the sphere tool.
//
//   Brad Whitlock, Tue Jun 18 15:34:04 PST 2002
//   Added the line tool.
//
//   Brad Whitlock, Mon Oct 28 15:32:33 PST 2002
//   Added the box tool.
//
// ****************************************************************************

VisWinTools::VisWinTools(VisWindowColleagueProxy &v) : VisWinColleague(v),
    toolProxy(v)
{
    highlightActor = NULL;
    highlightAdded = false;

    // Initialize the tools array.
    for(int i = 0; i < 10; ++i)
        tools[i] = NULL;
    numTools = 0;

    // Create the tools.
    boxTool = new VisitBoxTool(toolProxy);
    lineTool = new VisitLineTool(toolProxy);
    planeTool = new VisitPlaneTool(toolProxy);
    pointTool = new VisitPointTool(toolProxy);
    sphereTool = new VisitSphereTool(toolProxy);

    // Add the tools to the tools array.
    tools[numTools++] = boxTool;
    tools[numTools++] = lineTool;
    tools[numTools++] = planeTool;
    tools[numTools++] = sphereTool;
    tools[numTools++] = pointTool;
}

// ****************************************************************************
// Method: VisWinTools::~VisWinTools
//
// Purpose: 
//   This is the destructor for the VisWinTools class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:43:18 PST 2001
//
// Modifications:
//   
// ****************************************************************************

VisWinTools::~VisWinTools()
{
    if(highlightActor != NULL)
    {
        highlightActor->Delete();
        highlightActor = NULL;
    }

    for(int i = 0; i < numTools; ++i)
        delete tools[i];
}

// ****************************************************************************
// Method: VisWinTools::SetBackgroundColor
//
// Purpose: 
//   Sets the background color used for the tool.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:43:39 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::SetBackgroundColor(double r, double g, double b)
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->SetBackgroundColor(r, g, b);
}

// ****************************************************************************
// Method: VisWinTools::SetForegroundColor
//
// Purpose: 
//   Sets the foreground color used for the tool.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:43:39 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::SetForegroundColor(double r, double g, double b)
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->SetForegroundColor(r, g, b);
}

// ****************************************************************************
// Method: VisWinTools::Start2DMode
//
// Purpose: 
//   Tells tools that we're starting 2D mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:44:58 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::Start2DMode()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->Start2DMode();
    UpdateHighlight();
    toolProxy.Render();
}

// ****************************************************************************
// Method: VisWinTools::Stop2DMode
//
// Purpose: 
//   Tells tools that we're quitting 2D mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:45:28 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::Stop2DMode()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->Stop2DMode();
    UpdateHighlight();
    toolProxy.Render();
}

// ****************************************************************************
// Method: VisWinTools::Start3DMode
//
// Purpose: 
//   Tells tools that we're starting 3D mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:45:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::Start3DMode()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->Start3DMode();
    UpdateHighlight();
    toolProxy.Render();
}

// ****************************************************************************
// Method: VisWinTools::Stop3DMode
//
// Purpose: 
//   Tells tools that we're quitting 3D mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:46:09 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::Stop3DMode()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->Start3DMode();
    UpdateHighlight();
    toolProxy.Render();
}

// ****************************************************************************
// Method: VisWinTools::UpdateView
//
// Purpose: 
//   Tells all of the tools to make any view-dependent changes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 16:34:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::UpdateView()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->UpdateView();
}

// ****************************************************************************
// Method: VisWinTools::UpdateView
//
// Purpose: 
//   Tells all of the tools that there are no plots.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 16:34:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::NoPlots()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->NoPlots();
}

// ****************************************************************************
// Method: VisWinTools::GetHotPoint
//
// Purpose: 
//   Returns a const pointer to the HotPoint that was clicked at point (x,y).
//   If no hotpoint was clicked, a NULL pointer is returned.
//
// Arguments:
//   x : The x location of the click in device coordinates.
//   y : The y location of the click in device coordinates.
//
// Returns:   A pointer to a hotpoint or NULL. 
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 13:47:28 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Oct 15 21:59:36 PST 2001
//   Added Hank's solution to the crash that was happening.
//
// ****************************************************************************

bool
VisWinTools::GetHotPoint(int x, int y, HotPoint &h) const
{
    double          v[4];
    bool            retval = false;
    vtkRenderer    *ren = mediator.GetCanvas();
    int            *size = ren->GetSize();

    // If we cannot get the size then return.
    if(size[0] == 0 || size[1] == 0)
        return retval;

    // Compute the normalized device coordinates of the mouse click.
    double ndX = double(x) / double(size[0]);
    double ndY = double(y) / double(size[1]);
    double minZ = 10.e6;

    for(int i = 0; i < numTools; ++i)
    {
        if(tools[i]->IsEnabled())
        {
            const HotPointVector &pts = tools[i]->HotPoints();
            for(int j = 0; j < pts.size(); ++j)
            {
                // Use the background renderer to compute the normalized
                // device coordinate of the hotpoint from the world space
                // coordinate.
                ren->SetWorldPoint(pts[j].pt.x, pts[j].pt.y, pts[j].pt.z, 1.0);
                ren->WorldToDisplay();
                ren->GetDisplayPoint(v);
                v[0] /= double(size[0]);
                v[1] /= double(size[1]);

                double dX = v[0] - ndX;
                double dY = v[1] - ndY;
                double distSquared = dX * dX + dY * dY;
                if(distSquared <= (pts[j].radius * pts[j].radius))
                {
                    // If it is the first time through, set the Z value.
                    if(!retval)
                        minZ = v[2];

                    // The hotpoint that is closer to the camera will have a
                    // smaller Z value.
                    if(v[2] <= minZ)
                    {
                        h = pts[j];
                        minZ = v[2];
                        retval = true;
                    }
                }
            }
        }
    }

    return retval;
}

// ****************************************************************************
// Method: VisWinTools::NumToolsEnabled
//
// Purpose: 
//   Returns the number of tools that are enabled.
//
// Returns:    The number of tools that are enabled.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 21:32:57 PST 2001
//
// Modifications:
//   
// ****************************************************************************

int
VisWinTools::NumToolsEnabled() const
{
    int numEnabled = 0;
    for(int i = 0; i < numTools; ++i)
    {
        if(tools[i]->IsEnabled())
            ++numEnabled;
    }

    return numEnabled;
}

// ****************************************************************************
// Method: VisWinTools::GetToolName
//
// Purpose: 
//   Returns the name of the specified tool.
//
// Arguments:
//   i : The index of the tool.
//
// Returns:    The name of the specified tool.
// 
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 11:11:56 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const char *
VisWinTools::GetToolName(int i) const
{
    if(i < 0 || i > numTools)
        EXCEPTION2(BadIndexException, 0, numTools);

    return tools[i]->GetName();
}

// ****************************************************************************
// Method: VisWinTools::GetNumTools
//
// Purpose: 
//   Returns the number of tools.
//
// Returns:    The number of tools.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 11:12:56 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

int
VisWinTools::GetNumTools() const
{
    return numTools;
}

// ****************************************************************************
// Method: VisWinTools::SetToolEnabled
//
// Purpose: 
//   Sets the enabled state of a tool.
//
// Arguments:
//   i   : The index of the tool.
//   val : The new enabled state of the tool.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 11:13:30 PDT 2001
//
// Modifications:
//   
//   Hank Childs, Wed Sep  4 08:24:08 PDT 2002
//   Add a call to recalculate the rendering order.  This is because if this
//   tool is added after a transparent plot, it will not show up.
//
// ****************************************************************************

void
VisWinTools::SetToolEnabled(int i, bool val)
{
    if(i < 0 || i > numTools)
        EXCEPTION2(BadIndexException, 0, numTools);

    if(val)
        tools[i]->Enable();
    else
        tools[i]->Disable();

    UpdateHighlight();
    toolProxy.RecalculateRenderOrder();
    toolProxy.Render();
}

// ****************************************************************************
// Method: VisWinTools::GetToolEnabled
//
// Purpose: 
//   Returns whether or not the tool is enabled.
//
// Arguments:
//   i : The index of the specified tool.
//
// Returns:    Whether or not the tool is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 11:14:17 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisWinTools::GetToolEnabled(int i) const
{
    if(i < 0 || i > numTools)
        EXCEPTION2(BadIndexException, 0, numTools);

    return tools[i]->IsEnabled();
}

// ****************************************************************************
// Method: VisWinTools::GetToolAvailable
//
// Purpose: 
//   Returns whether or not the tool is available.
//
// Arguments:
//   i : The index of the specified tool.
//
// Returns:    Whether or not the tool is available.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 11:14:17 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisWinTools::GetToolAvailable(int i) const
{
    if(i < 0 || i > numTools)
        EXCEPTION2(BadIndexException, 0, numTools);

    return tools[i]->IsAvailable();
}

// ****************************************************************************
// Method: VisWinTools::UpdateHighlight
//
// Purpose: 
//   Updates the positions, etc of the hotpoint highlights that are drawn in
//   the foreground renderer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 2 23:58:02 PST 2001
//
// Modifications:
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//    Make the highlightActor un-pickable.
//   
// ****************************************************************************

void
VisWinTools::UpdateHighlight()
{
    if(NumToolsEnabled() < 1)
    {
        if(highlightActor != NULL)
        {
            toolProxy.GetForeground()->RemoveActor2D(highlightActor);
            highlightAdded = false;
        }
    }
    else
    {
        // Regenerate the actor's highlight squares using the hotpoints of the
        // tools that are enabled.
        if(highlightActor == NULL)
        {
            highlightActor = vtkHighlightActor2D::New();
            highlightActor->SetToolPointer(tools);
            highlightActor->SetHelperRenderer(toolProxy.GetCanvas());
            highlightActor->PickableOff();
        }
        highlightActor->SetNumTools(numTools);

        // Add the actor to the foreground renderer.
        if(!highlightAdded)
            toolProxy.GetForeground()->AddActor2D(highlightActor);

        highlightAdded = true;
    }
}

// ****************************************************************************
// Method: VisWinTools::SetHighlightEnabled
//
// Purpose: 
//   Either adds or removes the highlights from the foreground renderer.
//
// Arguments:
//   val : Whether or not to show the highlights.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 00:12:10 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::SetHighlightEnabled(bool val)
{
    if(val)
    {
        UpdateHighlight();
    }
    else if(highlightAdded)
    {
        toolProxy.GetForeground()->RemoveActor2D(highlightActor);
        highlightAdded = false;
    }
}

// ****************************************************************************
// Method: VisWinTools::GetToolInterface
//
// Purpose: 
//   Returns the interface for the i'th tool.
//
// Arguments:
//   i : The index of the tool we're after.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 11 14:37:22 PST 2002
//
// Modifications:
//   
// ****************************************************************************

avtToolInterface &
VisWinTools::GetToolInterface(int i) const
{
    if(i < 0 || i > numTools)
        EXCEPTION2(BadIndexException, 0, numTools);

    return tools[i]->GetInterface();
}

// ****************************************************************************
// Method: VisWinTools::UpdateTool
//
// Purpose: 
//   Update the specified tool.
//
// Arguments:
//   i : The tool that we want to update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 09:42:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::UpdateTool(int i)
{
    if(i < 0 || i > numTools)
        EXCEPTION2(BadIndexException, 0, numTools);

    tools[i]->UpdateTool();
}

// ****************************************************************************
// Method: VisWinTools::TurnLightingOn
//
// Purpose: 
//   Tells all of the tools that lighting is turned on. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 5, 2002 
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::TurnLightingOn()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->TurnLightingOn();
}

// ****************************************************************************
// Method: VisWinTools::TurnLightingOff
//
// Purpose: 
//   Tells all of the tools that lighting is turned off. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 5, 2002 
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::TurnLightingOff()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->TurnLightingOff();
}


// ****************************************************************************
// Method: VisWinTools::ReAddToWindow
//
// Purpose: 
//   Allows tools to re-add themselves to the window after the plots. Used
//   in anti-aliasing mode. 
//
// Programmer: Kathleen Bonnell 
// Creation:   May 28, 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  3 16:49:14 PST 2003
//   Renamed.
//   
// ****************************************************************************

void
VisWinTools::ReAddToolsToWindow()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->ReAddToWindow();
}


// **************************************************************************//
//                           vtkHighlightActor2D                             //
// **************************************************************************//

// ****************************************************************************
// Method: vtkHighlightActor2D::vtkHighlightActor2D
//
// Purpose: 
//   This is the constructor for the vtkHighlightActor2D class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 11:44:20 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

vtkHighlightActor2D::vtkHighlightActor2D()
{
    highlightActor = NULL;
    highlightMapper = NULL;
    highlightData = NULL;
}

// ****************************************************************************
// Method: vtkHighlightActor2D::~vtkHighlightActor2D
//
// Purpose: 
//   This is the destructor for the vtkHighlightActor2D class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 11:44:20 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

vtkHighlightActor2D::~vtkHighlightActor2D()
{
    if(highlightActor != NULL)
    {
        highlightActor->Delete();
        highlightActor = NULL;
    }
    if(highlightData != NULL)
    {
        highlightData->Delete();
        highlightData = NULL;
    }
    if(highlightMapper != NULL)
    {
        highlightMapper->Delete();
        highlightMapper = NULL;
    }
}

// ****************************************************************************
// Method: vtkHighlightActor2D::New
//
// Purpose: 
//   Returns a new instance of the vtkHighlightActor2D class.
//
// Returns:    A new instance of the vtkHighlightActor2D class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 11:44:53 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

vtkHighlightActor2D *vtkHighlightActor2D::New()
{
    return new vtkHighlightActor2D;
}

// ****************************************************************************
// Method: vtkHighlightActor2D::RenderOpaqueGeometry
//
// Purpose: 
//   Renders the actor's opaque geometry.
//
// Arguments:
//   v : The viewport into which we're rendering.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 11:45:20 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Thu Dec 12 15:12:36 PST 2002
//   Renamed to RenderOverlay, call actor RenderOverlay to match new VTK API.
//   
// ****************************************************************************

int vtkHighlightActor2D::RenderOverlay(vtkViewport *v)
{
    RegenerateHighlight();
    return highlightActor->RenderOverlay(v); 
}

//
// Set methods.
//

void vtkHighlightActor2D::SetToolPointer(VisitInteractiveTool **t)
{
    tools = t;
}

void vtkHighlightActor2D::SetNumTools(int n)
{
    numTools = n;
}

void vtkHighlightActor2D::SetHelperRenderer(vtkRenderer *ren)
{
    helperRenderer = ren;
}

// ****************************************************************************
// Method: vtkHighlightActor2D::RegenerateHighlight
//
// Purpose: 
//   This method regenerates the boxes that are displayed to represent the
//   hotpoints.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 11:41:54 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//   Make ptIds of type vtkIdType to match VTK 4.0 API.
//   
//   Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//   Make the highlightActor un-pickable.
//
//   Brad Whitlock, Tue Feb 12 11:57:22 PDT 2002
//   Added code to delete objects that get recreated.
// 
//   Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//   vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray
//   for colors instead.
//
// ****************************************************************************

void
vtkHighlightActor2D::RegenerateHighlight()
{
    // Count the number of hot points that we need to display.
    int i, numHotPoints = 0;    
    for(i = 0; i < numTools; ++i)
    {
        if(tools[i]->IsEnabled())
            numHotPoints += tools[i]->HotPoints().size();
    }

    // Allocate some temporary arrays that we'll use to construct the
    // highlightData object.
    int numPts = 4 * numHotPoints;
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *lines = vtkCellArray::New();
    lines->Allocate(lines->EstimateSize(numPts, 2)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(numPts);

    // Store the colors and points in the polydata.
    if(highlightData != NULL)
        highlightData->Delete();
    highlightData = vtkPolyData::New();
    highlightData->Initialize();
    highlightData->SetPoints(pts);
    highlightData->SetLines(lines);
    highlightData->GetCellData()->SetScalars(colors);
    pts->Delete(); lines->Delete(); colors->Delete(); 

    // Add a cell to the polyData for each active hotpoint.
    int    ptIndex = 0;
    int    cellIndex = 0;
    int    winWidth = helperRenderer->GetSize()[0];
    int    winHeight = helperRenderer->GetSize()[1];
    double v[4];
    double  coord[3];
    vtkIdType    ptIds[4];

    coord[2] = 0.;
#define SQRT_OF_2 1.4142136

    for(i = 0; i < numTools; ++i)
    {
        if(tools[i]->IsEnabled())
        {
            const HotPointVector &hpts = tools[i]->HotPoints();
            for(int j = 0; j < hpts.size(); ++j)
            {
                // Use the background renderer to compute the normalized
                // device coordinate of the hotpoint from the world space
                // coordinate.
                helperRenderer->SetWorldPoint(hpts[j].pt.x, hpts[j].pt.y, hpts[j].pt.z, 1.0);
                helperRenderer->WorldToDisplay();
                helperRenderer->GetDisplayPoint(v);

                double dX = double((hpts[j].radius * winWidth) / SQRT_OF_2);
                double dY = double((hpts[j].radius * winHeight) / SQRT_OF_2);

                coord[0] = v[0] + dX;
                coord[1] = v[1] + dY;
                pts->SetPoint(ptIndex, coord);
                coord[0] = v[0] + dX;
                coord[1] = v[1] - dY;
                pts->SetPoint(ptIndex + 1, coord);
                coord[0] = v[0] - dX;
                coord[1] = v[1] - dY;
                pts->SetPoint(ptIndex + 2, coord);
                coord[0] = v[0] - dX;
                coord[1] = v[1] + dY;
                pts->SetPoint(ptIndex + 3, coord);

                for(int k = 0; k < 4; ++k)
                {
                    ptIds[0] = ptIndex + k;
                    ptIds[1] = (k < 3) ? (ptIndex + k + 1) : ptIndex;
                    lines->InsertNextCell(2, ptIds);

                    // Add the color.
                    unsigned char *rgb = colors->GetPointer(cellIndex * 3);
                    rgb[0] = 255;
                    rgb[1] = 0;
                    rgb[2] = 0;
                    ++cellIndex;
                }
                ptIndex += 4;
            }
        }
    }

    if(highlightMapper != NULL)
        highlightMapper->Delete();
    highlightMapper = vtkPolyDataMapper2D::New();
    highlightMapper->SetInput(highlightData);

    if(highlightActor != NULL)
        highlightActor->Delete();
    highlightActor = vtkActor2D::New();
    highlightActor->GetProperty()->SetLineWidth(2.);
    highlightActor->SetMapper(highlightMapper);
    highlightActor->PickableOff();
}


// ****************************************************************************
// Method: VisWinTools::FullFrameOn
//
// Purpose: 
//   Tells tools that FullFrame mode has been turned on. 
//
// Arguments:
//   scale : The axis scale factor. 
//   type  : The axis scale type.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::FullFrameOn(const double scale, const int type)
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->FullFrameOn(scale, type);
}


// ****************************************************************************
// Method: VisWinTools::FullFrameOff
//
// Purpose: 
//   Tells tools that FullFrame mode has been turned off. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 6, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
VisWinTools::FullFrameOff()
{
    for(int i = 0; i < numTools; ++i)
        tools[i]->FullFrameOff();
}
