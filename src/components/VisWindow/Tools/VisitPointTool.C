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

#include <VisitPointTool.h>

#include <math.h>

#include <vector>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <avtVector.h>

// ****************************************************************************
// Method: VisitPointTool::VisitPointTool
//
// Purpose: 
//   This is the constructor for the point tool.
//
// Arguments:
//    p : A reference to the tool proxy.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// Modifications:
//   Akira Haddox, Wed Jul  2 14:56:33 PDT 2003
//   Added axis translate.
//
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

VisitPointTool::VisitPointTool(VisWindowToolProxy &p) : VisitInteractiveTool(p),
    Interface(p)
{
    window3D = false;
    addedBbox = false;
    addedGuide = false;
    axisTranslate = none;

    HotPoint h;
    h.radius = 1./60.; // See what a good value is.
    h.tool = this;

    //
    // Add the hotpoint
    //
    h.pt = avtVector(0., 0.,  0.);
    h.callback = TranslateCallback;
    hotPoints.push_back(h);

    guideActor = NULL;
    guideMapper = NULL;
    guideData = NULL;

    CreateTextActors();
    CreateGuide();
}

// ****************************************************************************
// Method: VisitPointTool::~VisitPointTool
//
// Purpose: 
//   This is the destructor for the point tool class.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:23:01 PDT 2003
//
// Modifications:
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

VisitPointTool::~VisitPointTool()
{
    // Delete the text mappers and actors
    DeleteTextActors();
 
    // Delete the guide
    DeleteGuide();
}

// ****************************************************************************
// Method: VisitPointTool::Enable
//
// Purpose: 
//   This method enables the tool.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// Modifications:
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

void
VisitPointTool::Enable()
{
    bool val = IsEnabled();
    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if(!val)
    {
        UpdateTool();
        AddText();
    }
}

// ****************************************************************************
// Method: VisitPointTool::Disable
//
// Purpose: 
//   This method disables the tool.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// Modifications:
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

void
VisitPointTool::Disable()
{
    bool val = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    if(val)
    {
        RemoveText();
    }
}

// ****************************************************************************
// Method: VisitPointTool::IsAvailable
//
// Purpose: 
//   Returns whether or not the tool is available for use.
//
// Returns:    Whether or not the tool is available for use.
//
// Note:       This may have to change later.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 14:36:26 EST 2008
//    Disabled for AxisArray window mode.
//
// ****************************************************************************

bool
VisitPointTool::IsAvailable() const
{

    return proxy.GetMode() != WINMODE_AXISARRAY && proxy.HasPlots();
}

// ****************************************************************************
// Method: VisitPointTool::Start2DMode
//
// Purpose: 
//   This method switches the tool to 2D mode.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::Start2DMode()
{
    window3D = false;

    // We're switching out of 3D. Set all the Z coordinates in the
    // hotpoints to 0.
    hotPoints[0].pt.z = 0.;
}

// ****************************************************************************
// Method: VisitPointTool::Stop3DMode
//
// Purpose: 
//   This method tells the tool that 3D mode is stopping.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::Stop3DMode()
{
    window3D = false;

    // We're switching out of 3D. Set all the Z coordinates in the
    // hotpoints to 0.
    hotPoints[0].pt.z = 0.;
}

// ****************************************************************************
// Method: VisitPointTool::Start3DMode
//
// Purpose: 
//   Indicates that the window is switching to 3D.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::Start3DMode()
{
    window3D = true;
}


// ****************************************************************************
// Method: VisitBoxTool::SetForegroundColor
//
// Purpose: 
//   This method sets the tool's foreground color.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 10:26:34 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::SetForegroundColor(double r, double g, double b)
{
    double color[3] = {r, g, b};

    // Set the colors of the text actors.
    pointTextActor->GetTextProperty()->SetColor(color);
}


// ****************************************************************************
// Method: VisitPointTool::UpdateView
//
// Purpose: 
//   Updates the position of the text based on the camera position.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::UpdateView()
{
    if(IsEnabled())
    {
        UpdateText();
    }
}

// ****************************************************************************
// Method: VisitPointTool::UpdateTool
//
// Purpose: 
//   Repostions the tool using the attributes stored in the Interface.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// Modifications:
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

void
VisitPointTool::UpdateTool()
{
    hotPoints[0].pt = avtVector((double*)Interface.GetPoint());

    UpdateText();
}


// ****************************************************************************
// Method: VisitPointTool::CreateTextActors
//
// Purpose: 
//   Create the text actors and mappers used to draw the point info.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::CreateTextActors()
{
    pointTextActor = vtkTextActor::New();
    pointTextActor->ScaledTextOff();
}

// ****************************************************************************
// Method: VisitPointTool::DeleteTextActors
//
// Purpose: 
//   Deletes the text actors and mappers.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::DeleteTextActors()
{
    if (pointTextActor)
    {
        pointTextActor->Delete();
        pointTextActor = NULL;
    }
}

// ****************************************************************************
// Method: VisitPointTool::AddText
//
// Purpose: 
//   Adds the text actors to the foreground canvas.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::AddText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->AddActor2D(pointTextActor);
#endif
}

// ****************************************************************************
// Method: VisitPointTool::RemoveText
//
// Purpose: 
//   Removes the text actors from the foreground canvas.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::RemoveText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->RemoveActor2D(pointTextActor);
#endif
}

// ****************************************************************************
// Method: VisitPointTool::UpdateText
//
// Purpose: 
//   Updates the info that the text actors display.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::UpdateText()
{
    char str[100];
    sprintf(str, "XYZ<%1.3g %1.3g %1.3g>", 
            hotPoints[0].pt.x, hotPoints[0].pt.y, hotPoints[0].pt.z);
    pointTextActor->SetInput(str);
    avtVector originScreen = ComputeWorldToDisplay(hotPoints[0].pt);
    double pt[3] = {originScreen.x, originScreen.y, 0.};
    pointTextActor->GetPositionCoordinate()->SetValue(pt);
}

// ****************************************************************************
// Method: VisitPointTool::CreateGuide
//
// Purpose: 
//   Creates the guide actor and mapper. The guide is the 3D crosshairs that
//   show where point endpoints are relative to the bounding box.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::CreateGuide()
{
    guideData = NULL;
    guideMapper = vtkPolyDataMapper::New();
    guideActor = vtkActor::New();
    guideActor->GetProperty()->SetLineWidth(1.);
    guideActor->GetProperty()->SetRepresentationToWireframe();
    guideActor->SetMapper(guideMapper);
}

// ****************************************************************************
// Method: VisitPointTool::DeleteGuide
//
// Purpose: 
//   Deletes the guide.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::DeleteGuide()
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
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::AddGuide()
{
    if(proxy.HasPlots() && window3D)
    {
        addedGuide = true;
        UpdateGuide();
        proxy.GetCanvas()->AddActor(guideActor);
    }
    else
    {
        addedGuide = false;
    }
}

// ****************************************************************************
// Method: VisitPointTool::RemoveGuide
//
// Purpose: 
//   Removes the guide actor from the renderer.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::RemoveGuide()
{
    if (addedGuide)
    {
        proxy.GetCanvas()->RemoveActor(guideActor);
    }
    addedGuide = false;
}

// ****************************************************************************
// Method: VisitPointTool::UpdateGuide
//
// Purpose: 
//   Recreates the points in the guide.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::UpdateGuide()
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
    vtkCellArray *points = vtkCellArray::New();
    points->Allocate(points->EstimateSize(numCells, 2)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(numCells);

    // Store the colors and points in the polydata.
    guideData = vtkPolyData::New();
    guideData->Initialize();
    guideData->SetPoints(pts);
    guideData->SetLines(points);
    guideData->GetCellData()->SetScalars(colors);
    pts->Delete(); points->Delete(); colors->Delete();

    //
    // Figure out the guide points.
    //
    int nverts = 9;
    avtVector verts[9];
    GetGuidePoints(verts);

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
    points->InsertNextCell(2, ptIds); \
    ptIds[0] = B; \
    ptIds[1] = C; \
    points->InsertNextCell(2, ptIds); \
    ptIds[0] = C; \
    ptIds[1] = D; \
    points->InsertNextCell(2, ptIds); \
    ptIds[0] = D; \
    ptIds[1] = A; \
    points->InsertNextCell(2, ptIds);

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
// Method: VisitPointTool::GetGuidePoints
//
// Purpose: 
//   Returns which axis most faces the camera.
//
// Returns:    0 = X-axis, 1 = Y-axis, 2 = Z-axis
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::GetGuidePoints(avtVector *pts)
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
        pts[0] = avtVector(hotPoints[0].pt.x, ymin, zmax);
        pts[1] = avtVector(hotPoints[0].pt.x, ymin, zmin);
        pts[2] = avtVector(hotPoints[0].pt.x, ymax, zmin);
        pts[3] = avtVector(hotPoints[0].pt.x, ymax, zmax);
        pts[4] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, zmax);
        pts[5] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, zmin);
        pts[6] = avtVector(hotPoints[0].pt.x, ymax, hotPoints[0].pt.z);
        pts[7] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[8] = avtVector(hotPoints[0].pt.x, ymin, hotPoints[0].pt.z);
    }
    else if(axis == 2 || axis == 3)
    {
        pts[0] = avtVector(xmin, hotPoints[0].pt.y, zmax);
        pts[1] = avtVector(xmax, hotPoints[0].pt.y, zmax);
        pts[2] = avtVector(xmax, hotPoints[0].pt.y, zmin);
        pts[3] = avtVector(xmin, hotPoints[0].pt.y, zmin);
        pts[4] = avtVector(xmin, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[5] = avtVector(xmax, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[6] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, zmin);
        pts[7] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[8] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, zmax);
    }
    else if(axis == 4 || axis == 5)
    {
        pts[0] = avtVector(xmin, ymin, hotPoints[0].pt.z);
        pts[1] = avtVector(xmax, ymin, hotPoints[0].pt.z);
        pts[2] = avtVector(xmax, ymax, hotPoints[0].pt.z);
        pts[3] = avtVector(xmin, ymax, hotPoints[0].pt.z);
        pts[4] = avtVector(xmin, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[5] = avtVector(xmax, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[6] = avtVector(hotPoints[0].pt.x, ymax, hotPoints[0].pt.z);
        pts[7] = avtVector(hotPoints[0].pt.x, hotPoints[0].pt.y, hotPoints[0].pt.z);
        pts[8] = avtVector(hotPoints[0].pt.x, ymin, hotPoints[0].pt.z);
    }
}

// ****************************************************************************
// Method: VisitPointTool::CallCallback
//
// Purpose: 
//   Lets the outside world know that the tool has a new point.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::CallCallback()
{
    // Point 1
    avtVector pt1(hotPoints[0].pt);

    Interface.SetPoint(pt1.x, pt1.y, pt1.z);
    Interface.ExecuteCallback();
}

// ****************************************************************************
// Method: VisitPointTool::InitialActorSetup
//
// Purpose: 
//   Makes the text actors active and starts bounding box mode.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::InitialActorSetup()
{
    // Enter bounding box mode if there are plots.
    if(proxy.HasPlots() && window3D)
    {
        // Add the guide
        AddGuide();

        addedBbox = true;
        proxy.StartBoundingBox();
    }
}

// ****************************************************************************
// Method: VisitPointTool::FinalActorSetup
//
// Purpose: 
//   Removes certain actors from the renderer and ends bounding box mode.
//
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// ****************************************************************************

void
VisitPointTool::FinalActorSetup()
{
    RemoveGuide();

    // End bounding box mode.
    if(addedBbox)
    {
        proxy.EndBoundingBox();
    }
    addedBbox = false;
}

// ****************************************************************************
//  Method:  VisitPointTool::ComputeTranslationDistance
//
//  Purpose:
//      Compute a vector for point motion based on a particular
//      direction.
//
//  Arguments:
//      direction       the direction to move in
//
// Programmer: Akira Haddox
// Creation:   July 2, 2003
//
// ****************************************************************************

avtVector
VisitPointTool::ComputeTranslationDistance(int direction)
{
    // This shouldn't happen, but just in case
    if (direction == none)
        return avtVector(0,0,0);

    if (direction == inAndOut)
        return ComputeDepthTranslationDistance();

    vtkCamera *camera = proxy.GetCanvas()->GetActiveCamera();
    
    int i;

    int *size = proxy.GetCanvas()->GetSize();
    double bounds[6];
    proxy.GetBounds(bounds);

    double dx = bounds[1] - bounds[0];
    double dy = bounds[3] - bounds[2];
    double dz = bounds[5] - bounds[4];
    
    std::vector<avtVector> axes;
    axes.push_back(avtVector(1., 0., 0.) * (dx / double(size[1])));
    axes.push_back(avtVector(-1., 0., 0.) * (dx / double(size[1])));
    axes.push_back(avtVector(0., 1., 0.) * (dy / double(size[1])));
    axes.push_back(avtVector(0., -1., 0.) * (dy / double(size[1])));
    axes.push_back(avtVector(0., 0., 1.) * (dz / double(size[1])));
    axes.push_back(avtVector(0., 0., -1.) * (dz / double(size[1])));
    
    avtVector camvec; // The vector to dot with

    const double *up = camera->GetViewUp();

    if (direction == upAndDown)
    {
        // Find what vector of {i,j,k,-i,-j,-k} best represents 'up'
        // The vector we want is the camera up vector.
        camvec.x = up[0];
        camvec.y = up[1];
        camvec.z = up[2];
    }
    else
    {
        // Find what vector best represents 'right'
        // The vector we want is the cross of the focus vector
        //  and the up vector.
        avtVector upVec(up[0], up[1], up[2]);
        const double *pos = camera->GetPosition();
        const double *focus = camera->GetFocalPoint();
        avtVector focusVec(focus[0]-pos[0],focus[1]-pos[1],focus[2]-pos[2]);
      
        camvec = focusVec % upVec;
    }
    
    camvec.normalize();
    
    double dots[6];
    for (i = 0; i < 6; ++i)
       dots[i] = camvec * axes[i]; 

    // Find the index of the largest dot product.
    int largestDotIndex = 0;
    for(i = 1; i < 6; ++i)
    {
        if(dots[i] > dots[largestDotIndex])
            largestDotIndex = i;
    }
    
    return axes[largestDotIndex];   
}

// ****************************************************************************
//  Method:  VisitPointTool::Translate
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
// Programmer: Akira Haddox
// Creation:   Mon Jun  9 09:21:40 PDT 2003
//
// Modifications:
//   Akira Haddox, Wed Jul  2 14:58:44 PDT 2003
//   Added contrained translation along an axis.
//
//   Akira Haddox, Mon Aug  4 12:48:02 PDT 2003
//   Removed unneeded point actor.
//
// ****************************************************************************

void
VisitPointTool::Translate(CB_ENUM e, int ctrl, int shift, int x, int y, int)
{
    if (axisTranslate == none)
    {
        if (shift && !ctrl)
            if (window3D)
                axisTranslate = inAndOut;
            else
                axisTranslate = none;
        else if (shift && ctrl)
            axisTranslate = leftAndRight;
        else if (ctrl)
            axisTranslate = upAndDown;
        else
            axisTranslate = none;
    }

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

        if (axisTranslate != none)
        {
            translationDistance = ComputeTranslationDistance(axisTranslate);
        }

        // Make the right actors active.
        InitialActorSetup();
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

        if (axisTranslate)
        {
            double delta;
            if (axisTranslate == leftAndRight)
                delta = x - lastX;
            else
                delta = y - lastY;
            motion = translationDistance * delta;
        }

        hotPoints[0].pt = (hotPoints[0].pt + motion);

        // Update the text actors.
        UpdateText();

        // Update the guide
        UpdateGuide();

        // Render the window
        proxy.Render();
    }
    else
    {
        // Call the tool's callback.
        CallCallback();

        // Remove the right actors.
        FinalActorSetup();

        axisTranslate = none;
    }
}

//
// Static callback functions.
//
//
void
VisitPointTool::TranslateCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPointTool *lt = (VisitPointTool *)it;
    lt->Translate(e, ctrl, shift, x, y, 0);
}
