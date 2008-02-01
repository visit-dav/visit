/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <math.h>
#include <VisitLineTool.h>

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkLineSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <avtVector.h>

#define LINE_SIZE 1.

// ****************************************************************************
// Method: VisitLineTool::VisitLineTool
//
// Purpose: 
//   This is the constructor for the line tool.
//
// Arguments:
//    p : A reference to the tool proxy.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:38:59 PST 2002
//
// Modifications:
//   
// ****************************************************************************

VisitLineTool::VisitLineTool(VisWindowToolProxy &p) : VisitInteractiveTool(p),
    Interface(p)
{
    window3D = false;
    addedBbox = false;
    addedGuide = false;
    enlongating = false;
    depthTranslate = false;

    HotPoint h;
    h.radius = 1./60.; // See what a good value is.
    h.tool = this;

    //
    // Add the line start hotpoint.
    //
    h.pt = avtVector(0.,  0.,  0.);
    h.callback = TranslatePoint1Callback;
    hotPoints.push_back(h);

    //
    // Add the line end hotpoint.
    //
    h.pt = avtVector(0., LINE_SIZE,  0.); // replace later.
    h.callback = TranslatePoint2Callback;
    hotPoints.push_back(h);

    //
    // Add the translate hotpoint
    //
    h.pt = avtVector(0., LINE_SIZE * 0.5,  0.); // replace later.
    h.callback = TranslateCallback;
    hotPoints.push_back(h);


    lineSource = NULL;
    lineData = NULL;
    lineActor = NULL;
    lineMapper = NULL;
    guideActor = NULL;
    guideMapper = NULL;
    guideData = NULL;

    InitializePoints();

    CreateLineActor();
    CreateTextActors();
    CreateGuide();
}

// ****************************************************************************
// Method: VisitLineTool::~VisitLineTool
//
// Purpose: 
//   This is the destructor for the line tool class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:45:08 PST 2002
//
// Modifications:
//   
// ****************************************************************************

VisitLineTool::~VisitLineTool()
{
    if(lineActor != NULL)
    {
        lineActor->Delete();
        lineActor = NULL;
    }

    if(lineMapper != NULL)
    {
        lineMapper->Delete();
        lineMapper = NULL;
    }

    if(lineData != NULL)
    {
        lineData->Delete();
        lineData = NULL;
    }

    if(lineSource != NULL)
    {
        lineSource->Delete();
        lineSource = NULL;
    }

    // Delete the text mappers and actors
    DeleteTextActors();
 
    // Delete the guide
    DeleteGuide();
}

// ****************************************************************************
// Method: VisitLineTool::InitializePoints
//
// Purpose: 
//   Uses the bounding box to determine good initial values for the line
//   endpoints.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 15:15:03 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::InitializePoints()
{
    //
    // Set up some defaults for the line endpoints.
    //
    double bounds[6];
    proxy.GetBounds(bounds);
    double dY = bounds[3] - bounds[2];
    double dZ = bounds[5] - bounds[4];
    double Z = 0.;
    if(window3D)
        Z = bounds[4] + 0.5 * dZ;
    avtVector p1(bounds[0], bounds[2] + 0.5 * dY, Z);
    avtVector p2(bounds[1], bounds[2] + 0.5 * dY, Z);
    Interface.SetPoint1(p1.x, p1.y, p1.z);
    Interface.SetPoint2(p2.x, p2.y, p2.z);
    hotPoints[0].pt = p1;
    hotPoints[1].pt = p2;
    hotPoints[2].pt = (p1 + p2) * 0.5;
}

// ****************************************************************************
// Method: VisitLineTool::Enable
//
// Purpose: 
//   This method enables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:46:43 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Sat Jul 13 18:03:18 PDT 2002 
//   Comment out call to InitializePoints so that this tool can be initialized
//   from a plot's attributes.
//
// ****************************************************************************

void
VisitLineTool::Enable()
{
    bool val = IsEnabled();
    //InitializePoints();
    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if(!val)
    {
        UpdateTool();
        proxy.GetCanvas()->AddActor(lineActor);
        AddText();
    }
}

// ****************************************************************************
// Method: VisitLineTool::Disable
//
// Purpose: 
//   This method disables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:46:43 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitLineTool::Disable()
{
    bool val = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    if(val)
    {
        proxy.GetCanvas()->RemoveActor(lineActor);
        RemoveText();
    }
}

// ****************************************************************************
// Method: VisitLineTool::IsAvailable
//
// Purpose: 
//   Returns whether or not the tool is available for use.
//
// Returns:    Whether or not the tool is available for use.
//
// Note:       This may have to change later.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:46:43 PST 2002
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:36:26 EST 2008
//    Disabled for AxisArray window mode.
//
// ****************************************************************************

bool
VisitLineTool::IsAvailable() const
{
    return proxy.GetMode() != WINMODE_AXISARRAY && proxy.HasPlots();
}

// ****************************************************************************
// Method: VisitLineTool::Start2DMode
//
// Purpose: 
//   This method switches the tool to 2D mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:46:54 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::Start2DMode()
{
    window3D = false;

    // We're switching out of 3D. Set all the Z coordinates in the
    // hotpoints to 0.
    hotPoints[0].pt.z = 0.;
    hotPoints[1].pt.z = 0.;
    hotPoints[2].pt.z = 0.;
}

// ****************************************************************************
// Method: VisitLineTool::Stop3DMode
//
// Purpose: 
//   This method tells the tool that 3D mode is stopping.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:47:35 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::Stop3DMode()
{
    window3D = false;

    // We're switching out of 3D. Set all the Z coordinates in the
    // hotpoints to 0.
    hotPoints[0].pt.z = 0.;
    hotPoints[1].pt.z = 0.;
    hotPoints[2].pt.z = 0.;
}

// ****************************************************************************
// Method: VisitLineTool::Start3DMode
//
// Purpose: 
//   Indicates that the window is switching to 3D.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 15:00:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::Start3DMode()
{
    window3D = true;
}

// ****************************************************************************
// Method: VisitLineTool::SetForegroundColor
//
// Purpose: 
//   This method sets the tool's foreground color.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:46:43 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002 
//   Use vtkTextProperty to set actor color instead of vtkProperty.
//
// ****************************************************************************

void
VisitLineTool::SetForegroundColor(double r, double g, double b)
{
    double color[3] = {r, g, b};
    lineActor->GetProperty()->SetColor(color);
    lineActor->GetProperty()->SetAmbient(1.);
    lineActor->GetProperty()->SetDiffuse(1.);
    lineActor->GetProperty()->SetSpecular(1.);
    pointTextActor[0]->GetTextProperty()->SetColor(color);
    pointTextActor[1]->GetTextProperty()->SetColor(color);
}

// ****************************************************************************
// Method: VisitLineTool::UpdateView
//
// Purpose: 
//   Updates the position of the text based on the camera position.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:46:43 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitLineTool::UpdateView()
{
    if(IsEnabled())
    {
        UpdateText();
    }
}

// ****************************************************************************
// Method: VisitLineTool::UpdateTool
//
// Purpose: 
//   Repostions the tool using the attributes stored in the Interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 15:55:44 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Jun  6 15:36:24 PDT 2003 
//   Added support for full-frame mode.
//   
// ****************************************************************************

void
VisitLineTool::UpdateTool()
{
    hotPoints[0].pt = avtVector((double*)Interface.GetPoint1());
    hotPoints[1].pt = avtVector((double*)Interface.GetPoint2());
    hotPoints[2].pt = (avtVector((double*)Interface.GetPoint1()) +
                       avtVector((double*)Interface.GetPoint2())) * 0.5;

    if (proxy.GetFullFrameMode())
    {
        // 
        // Translate the hotPoints so they appear in the correct position
        // in full-frame mode. 
        // 
        double scale;
        int type;
        proxy.GetScaleFactorAndType(scale, type);
        if (type == 0 ) // x_axis
        {
            hotPoints[0].pt.x *= scale;
            hotPoints[1].pt.x *= scale;
            hotPoints[2].pt.x *= scale;
        }
        else            // x_axis
        {
            hotPoints[0].pt.y *= scale;
            hotPoints[1].pt.y *= scale;
            hotPoints[2].pt.y *= scale;
        }
    }

    UpdateLine();
    UpdateText();
}

// ****************************************************************************
// Method: VisitLineTool::CreateLineActor
//
// Purpose: 
//   Creates the plane actor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:44:22 PDT 2001
//
// Modifications:
//
// ****************************************************************************

void
VisitLineTool::CreateLineActor()
{
    lineSource = vtkLineSource::New();
    lineMapper = vtkPolyDataMapper::New();
    UpdateLine();

    lineActor = vtkActor::New();
    lineActor->GetProperty()->SetRepresentationToWireframe();
    lineActor->GetProperty()->SetLineWidth(2.);
    lineActor->SetMapper(lineMapper);
}

// ****************************************************************************
// Method: VisitLineTool::UpdateLine
//
// Purpose: 
//   Updates the position of the line.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:59:10 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Jul 26 10:52:09 PDT 2002   
//   Register lineData to avoid memory problems.
//   
// ****************************************************************************

void
VisitLineTool::UpdateLine()
{
    avtVector p1(hotPoints[0].pt);
    avtVector p2(hotPoints[1].pt);
    lineSource->SetPoint1(p1.x, p1.y, p1.z);
    lineSource->SetPoint2(p2.x, p2.y, p2.z);
    lineSource->SetResolution(1);
    lineData = lineSource->GetOutput();
    lineData->Register(NULL);

    lineMapper->SetInput(lineData);
}

// ****************************************************************************
// Method: VisitLineTool::CreateTextActors
//
// Purpose: 
//   Create the text actors and mappers used to draw the point info.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:01:37 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
// ****************************************************************************

void
VisitLineTool::CreateTextActors()
{
    pointTextActor[0] = vtkTextActor::New();
    pointTextActor[0]->ScaledTextOff();

    pointTextActor[1] = vtkTextActor::New();
    pointTextActor[1]->ScaledTextOff();
}

// ****************************************************************************
// Method: VisitLineTool::DeleteTextActors
//
// Purpose: 
//   Deletes the text actors and mappers.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:01:57 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   TextMappers no longer required.
//
// ****************************************************************************

void
VisitLineTool::DeleteTextActors()
{
    for(int i = 0; i < 2; ++i)
    {
        if(pointTextActor[i] != NULL)
        {
            pointTextActor[i]->Delete();
            pointTextActor[i] = NULL;
        }
    }
}

// ****************************************************************************
// Method: VisitLineTool::AddText
//
// Purpose: 
//   Adds the text actors to the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:07:34 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitLineTool::AddText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->AddActor2D(pointTextActor[0]);
    proxy.GetForeground()->AddActor2D(pointTextActor[1]);
#endif
}

// ****************************************************************************
// Method: VisitLineTool::RemoveText
//
// Purpose: 
//   Removes the text actors from the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:07:53 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitLineTool::RemoveText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->RemoveActor2D(pointTextActor[0]);
    proxy.GetForeground()->RemoveActor2D(pointTextActor[1]);
#endif
}

// ****************************************************************************
// Method: VisitLineTool::UpdateText
//
// Purpose: 
//   Updates the info that the text actors display.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:08:08 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace mapper with actor.
//
// ****************************************************************************

void
VisitLineTool::UpdateText()
{
    const char *prefix[] = {"Start", "End"};

    for(int i = 0; i < 2; ++i)
    {
        char str[100];
        sprintf(str, "%s <%1.3g %1.3g %1.3g>", prefix[i],
                hotPoints[i].pt.x, hotPoints[i].pt.y, hotPoints[i].pt.z);
        pointTextActor[i]->SetInput(str);
        avtVector originScreen = ComputeWorldToDisplay(hotPoints[i].pt);
        double pt[3] = {originScreen.x, originScreen.y, 0.};
        pointTextActor[i]->GetPositionCoordinate()->SetValue(pt);
    }
}

// ****************************************************************************
// Method: VisitLineTool::CreateGuide
//
// Purpose: 
//   Creates the guide actor and mapper. The guide is the 3D crosshairs that
//   show where line endpoints are relative to the bounding box.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 10:46:43 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::CreateGuide()
{
    guideData = NULL;
    guideMapper = vtkPolyDataMapper::New();
    guideActor = vtkActor::New();
    guideActor->GetProperty()->SetLineWidth(1.);
    guideActor->GetProperty()->SetRepresentationToWireframe();
    guideActor->SetMapper(guideMapper);
}

// ****************************************************************************
// Method: VisitLineTool::DeleteGuide
//
// Purpose: 
//   Deletes the guide.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 10:47:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::DeleteGuide()
{
    if(guideActor != NULL)
    {
        guideActor->Delete();
        guideActor = NULL;
    }

    if(guideMapper != NULL)
    {
        guideMapper->Delete();
        guideMapper = NULL;
    }

    if(guideData != NULL)
    {
        guideData->Delete();
        guideData = NULL;
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::AddGuide
//
// Purpose: 
//   Updates the guide and adds its actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 10:50:28 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::AddGuide(int pointIndex)
{
    if(proxy.HasPlots() && window3D)
    {
        addedGuide = true;
        UpdateGuide(pointIndex);
        proxy.GetCanvas()->AddActor(guideActor);
    }
    else
    {
        addedGuide = false;
    }
}

// ****************************************************************************
// Method: VisitLineTool::RemoveGuide
//
// Purpose: 
//   Removes the guide actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 10:51:58 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::RemoveGuide()
{
    if (addedGuide)
    {
        proxy.GetCanvas()->RemoveActor(guideActor);
    }
    addedGuide = false;
}

// ****************************************************************************
// Method: VisitLineTool::UpdateGuide
//
// Purpose: 
//   Recreates the points in the guide.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 11:41:30 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::UpdateGuide(int pointIndex)
{
    if(!addedGuide)
        return;

    if(guideData != NULL)
        guideData->Delete();

    // Use the max number of points an intersection will have.
    int numPts = 9;
    int numCells = 16;

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *lines = vtkCellArray::New();
    lines->Allocate(lines->EstimateSize(numCells, 2)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(numCells);

    // Store the colors and points in the polydata.
    guideData = vtkPolyData::New();
    guideData->Initialize();
    guideData->SetPoints(pts);
    guideData->SetLines(lines);
    guideData->GetCellData()->SetScalars(colors);
    pts->Delete(); lines->Delete(); colors->Delete();

    //
    // Figure out the guide points.
    //
    int nverts = 9;
    avtVector verts[9];
    GetGuidePoints(pointIndex, verts);

    //
    // Now that we have guide points, create a polydata from that.
    //
    double fg[3];
    proxy.GetForegroundColor(fg);
    unsigned char r = (unsigned char)((float)fg[0] * 255.f);
    unsigned char g = (unsigned char)((float)fg[1] * 255.f);
    unsigned char b = (unsigned char)((float)fg[2] * 255.f);
    int i, index;
    for(i = 0, index = 0; i < nverts; ++i, index += 3)
    {
        // Add points to the vertex list.
        double coord[3];
        coord[0] = verts[i].x;
        coord[1] = verts[i].y;
        coord[2] = verts[i].z;
        pts->SetPoint(i, coord);

    }

    for(i = 0, index = 0; i < numCells; ++i, index += 3)
    {    
        // Store the color.
        unsigned char *rgb = colors->GetPointer(index);
        rgb[0] = r;
        rgb[1] = g;
        rgb[2] = b;
    }

#define CREATEQUAD(A,B,C,D)     ptIds[0] = A; \
    ptIds[1] = B; \
    lines->InsertNextCell(2, ptIds); \
    ptIds[0] = B; \
    ptIds[1] = C; \
    lines->InsertNextCell(2, ptIds); \
    ptIds[0] = C; \
    ptIds[1] = D; \
    lines->InsertNextCell(2, ptIds); \
    ptIds[0] = D; \
    ptIds[1] = A; \
    lines->InsertNextCell(2, ptIds);

    // Add cells to the polydata.
    vtkIdType ptIds[2];
    CREATEQUAD(0,8,7,4);
    CREATEQUAD(8,1,5,7);
    CREATEQUAD(7,5,2,6);
    CREATEQUAD(4,7,6,3);

    // Set the mapper's input to be the new dataset.
    guideMapper->SetInput(guideData);
}

// ****************************************************************************
// Method: VisitLineTool::GetGuidePoints
//
// Purpose: 
//   Returns which axis most faces the camera.
//
// Returns:    0 = X-axis, 1 = Y-axis, 2 = Z-axis
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 11:08:13 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::GetGuidePoints(int pi, avtVector *pts)
{
    int axis = FacingAxis();

    // Fill the return pts array.
    double bounds[6];
    proxy.GetBounds(bounds);
    double xmin = bounds[0];
    double xmax = bounds[1];
    double ymin = bounds[2];
    double ymax = bounds[3];
    double zmin = bounds[4];
    double zmax = bounds[5];

    if(axis == 0 || axis == 1)
    {
        pts[0] = avtVector(hotPoints[pi].pt.x, ymin, zmax);
        pts[1] = avtVector(hotPoints[pi].pt.x, ymin, zmin);
        pts[2] = avtVector(hotPoints[pi].pt.x, ymax, zmin);
        pts[3] = avtVector(hotPoints[pi].pt.x, ymax, zmax);
        pts[4] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, zmax);
        pts[5] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, zmin);
        pts[6] = avtVector(hotPoints[pi].pt.x, ymax, hotPoints[pi].pt.z);
        pts[7] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[8] = avtVector(hotPoints[pi].pt.x, ymin, hotPoints[pi].pt.z);
    }
    else if(axis == 2 || axis == 3)
    {
        pts[0] = avtVector(xmin, hotPoints[pi].pt.y, zmax);
        pts[1] = avtVector(xmax, hotPoints[pi].pt.y, zmax);
        pts[2] = avtVector(xmax, hotPoints[pi].pt.y, zmin);
        pts[3] = avtVector(xmin, hotPoints[pi].pt.y, zmin);
        pts[4] = avtVector(xmin, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[5] = avtVector(xmax, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[6] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, zmin);
        pts[7] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[8] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, zmax);
    }
    else if(axis == 4 || axis == 5)
    {
        pts[0] = avtVector(xmin, ymin, hotPoints[pi].pt.z);
        pts[1] = avtVector(xmax, ymin, hotPoints[pi].pt.z);
        pts[2] = avtVector(xmax, ymax, hotPoints[pi].pt.z);
        pts[3] = avtVector(xmin, ymax, hotPoints[pi].pt.z);
        pts[4] = avtVector(xmin, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[5] = avtVector(xmax, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[6] = avtVector(hotPoints[pi].pt.x, ymax, hotPoints[pi].pt.z);
        pts[7] = avtVector(hotPoints[pi].pt.x, hotPoints[pi].pt.y, hotPoints[pi].pt.z);
        pts[8] = avtVector(hotPoints[pi].pt.x, ymin, hotPoints[pi].pt.z);
    }
}

// ****************************************************************************
// Method: VisitLineTool::CallCallback
//
// Purpose: 
//   Lets the outside world know that the tool has a new slice plane.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 18 16:12:37 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitLineTool::CallCallback()
{
    // Point 1
    avtVector pt1(hotPoints[0].pt);

    // Point 2
    avtVector pt2(hotPoints[1].pt);

    Interface.SetPoint1(pt1.x, pt1.y, pt1.z);
    Interface.SetPoint2(pt2.x, pt2.y, pt2.z);
    Interface.ExecuteCallback();
}

// ****************************************************************************
// Method: VisitLineTool::InitialActorSetup
//
// Purpose: 
//   Makes the text actors active and starts bounding box mode.
//
// Arguments:
//   pointIndex : The index of the hotpoint being moved.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 14:37:20 PST 2001
//
// Modifications:
//
// ****************************************************************************

void
VisitLineTool::InitialActorSetup(int pointIndex)
{
    // Enter bounding box mode if there are plots.
    if(proxy.HasPlots() && window3D)
    {
        // Add the guide
        AddGuide(pointIndex);

        addedBbox = true;
        proxy.StartBoundingBox();
    }
}

// ****************************************************************************
// Method: VisitLineTool::FinalActorSetup
//
// Purpose: 
//   Removes certain actors from the renderer and ends bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 14:38:06 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  3 17:03:34 PST 2003 
//   If transparencies exist, have the plots recalculate render order, so
//   that this tool is rendered before the transparent actors. 
//
// ****************************************************************************

void
VisitLineTool::FinalActorSetup()
{
    RemoveGuide();

    // End bounding box mode.
    if(addedBbox)
    {
        proxy.EndBoundingBox();
    }
    addedBbox = false;
    if (proxy.TransparenciesExist())
        proxy.RecalculateRenderOrder();
}

// ****************************************************************************
//  Method:  VisitLineTool::Translate
//
//  Purpose:
//    This is the handler method that is called when the translate hotpoint
//    is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Jun 18 16:15:18 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Oct 30 09:37:57 PDT 2002
//    Moved some code into the base class.
//
// ****************************************************************************

void
VisitLineTool::Translate(CB_ENUM e, int ctrl, int shift, int x, int y,
    int pointIndex)
{
    if(ctrl)
        enlongating = true;
    else if(shift && window3D)
        depthTranslate = true;

    if(e == CB_START)
    {
        vtkRenderer *ren = proxy.GetCanvas();
        vtkCamera *camera = ren->GetActiveCamera();
        double ViewFocus[3];
        camera->GetFocalPoint(ViewFocus);
        ComputeWorldToDisplay(ViewFocus[0], ViewFocus[1],
                              ViewFocus[2], ViewFocus);
        // Store the focal depth.
        focalDepth = ViewFocus[2];

        if(enlongating)
        {
            // Get the size of the bounding box.
            double bounds[6];
            proxy.GetBounds(bounds);
            double dX = bounds[1] - bounds[0];
            double dY = bounds[3] - bounds[2];
            double dZ = bounds[5] - bounds[4];
            double diagonal = sqrt(dX*dX + dY*dY + dZ*dZ);
            int *size = proxy.GetCanvas()->GetSize();

            // This is the amount by which to translate.
            translationDistance = diagonal * 0.5 / double(size[1]);
        }

        if(depthTranslate)
        {
            depthTranslationDistance = ComputeDepthTranslationDistance();
        }

        // Make the right actors active.
        InitialActorSetup(pointIndex);
    }
    else if(e == CB_MIDDLE)
    {
        avtVector newPoint = ComputeDisplayToWorld(avtVector(x,y,focalDepth));
        //
        // Have to recalculate the old mouse point since the viewport has
        // moved, so we can't move it outside the loop
        //
        avtVector oldPoint = ComputeDisplayToWorld(avtVector(lastX,lastY,focalDepth));
        avtVector motion(newPoint - oldPoint);

        if(pointIndex == 0)
        {
            if(enlongating)
            {
                motion = (hotPoints[0].pt - hotPoints[1].pt);
                motion.normalize();
                motion *= ((y - lastY) * translationDistance);
            }
            else if(depthTranslate)
                motion = depthTranslationDistance * double(y - lastY);

            hotPoints[0].pt = (hotPoints[0].pt + motion);
            hotPoints[2].pt = (hotPoints[0].pt + hotPoints[1].pt) * 0.5;
        }
        else if(pointIndex == 1)
        {
            if(enlongating)
            {
                motion = (hotPoints[1].pt - hotPoints[0].pt);
                motion.normalize();
                motion *= ((y - lastY) * translationDistance);
            }
            else if(depthTranslate)
                motion = depthTranslationDistance * double(y - lastY);
            hotPoints[1].pt = (hotPoints[1].pt + motion);
            hotPoints[2].pt = (hotPoints[0].pt + hotPoints[1].pt) * 0.5;
        }
        else
        {
            if(depthTranslate)
                motion = depthTranslationDistance * double(y - lastY);

            for(int i = 0; i < 3; ++i)
                hotPoints[i].pt = (hotPoints[i].pt + motion);
        }

        // Update the line actor
        UpdateLine();

        // Update the text actors.
        UpdateText();

        // Update the guide
        UpdateGuide(pointIndex);

        // Render the window
        proxy.Render();
    }
    else
    {
        // Call the tool's callback.
        CallCallback();

        // Remove the right actors.
        FinalActorSetup();

        // Indicate that we are not enlongating the line or translating it
        // in the depth dimension.
        enlongating = false;
        depthTranslate = false;
    }
}

// ****************************************************************************
//  Method:  VisitLineTool::ReAddToWindow
//
//  Purpose:
//    Allows the tool to re-add any actors affected by anti-aliasing to remove
//    and re-add themselves back to the renderer, so that they will be rendered
//    after plots.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    Wed May 28 16:09:47 PDT 2003 
//
//  Modifications:
//
// ****************************************************************************

void
VisitLineTool::ReAddToWindow()
{
    if(IsEnabled())
    {
        proxy.GetCanvas()->RemoveActor(lineActor);
        proxy.GetCanvas()->AddActor(lineActor);
    }
}


// ****************************************************************************
//  Method:  VisitLineTool::FullFrameOn
//
//  Purpose: Updates the tool.
//
//  Arguments:
//    <unused>   The axis scale factor.
//    <unused>   The axis scale type.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 6, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
VisitLineTool::FullFrameOn(const double, const int)
{
    if(IsEnabled())
    {
        UpdateTool();
    }
}


// ****************************************************************************
//  Method:  VisitLineTool::FullFrameOn
//
//  Purpose: Updates the tool.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 6, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
VisitLineTool::FullFrameOff()
{
    if(IsEnabled())
    {
        UpdateTool();
    }
}


//
// Static callback functions.
//

void
VisitLineTool::TranslatePoint1Callback(VisitInteractiveTool *it, CB_ENUM e,
                                       int ctrl, int shift, int x, int y, int)
{
    VisitLineTool *lt = (VisitLineTool *)it;
    lt->Translate(e, ctrl, shift, x, y, 0);
}

void
VisitLineTool::TranslatePoint2Callback(VisitInteractiveTool *it, CB_ENUM e,
                                       int ctrl, int shift, int x, int y, int)
{
    VisitLineTool *lt = (VisitLineTool *)it;
    lt->Translate(e, ctrl, shift, x, y, 1);
}

void
VisitLineTool::TranslateCallback(VisitInteractiveTool *it, CB_ENUM e,
                                 int ctrl, int shift, int x, int y, int)
{
    VisitLineTool *lt = (VisitLineTool *)it;
    lt->Translate(e, ctrl, shift, x, y, 2);
}
