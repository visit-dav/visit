#include <VisitBoxTool.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkOutlineSource.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <avtVector.h>

#define BOX_SIZE         1.

// ****************************************************************************
// Method: VisitBoxTool::VisitBoxTool
//
// Purpose: 
//   This is the constructor for the plane tool.
//
// Arguments:
//   p : A reference to the tool proxy.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:39 PDT 2002
//
// Modifications:
//
// ****************************************************************************

VisitBoxTool::VisitBoxTool(VisWindowToolProxy &p) : VisitInteractiveTool(p),
    Interface(p)
{
    depthTranslate = false;
    addedOutline = false;
    addedBbox = false;
    activeHotPoint = 0;

    HotPoint h;
    h.radius = 1./60.; // See what a good value is.
    h.tool = this;

    //
    // Add the box origin hotpoint.
    //
    h.pt = avtVector(0.,  0.,  0.);
    h.callback = TranslateCallback;
    origHotPoints.push_back(h);

    //
    // Add the resize X hotpoint.
    //
    h.pt = avtVector(BOX_SIZE, 0., 0.); // replace later.
    h.callback = ResizeCallback1;
    origHotPoints.push_back(h);

    //
    // Add the resize Y hotpoint.
    //
    h.pt = avtVector(0., BOX_SIZE,  0.); // replace later.
    h.callback = ResizeCallback2;
    origHotPoints.push_back(h);

    //
    // Add the resize Z hotpoint.
    //
    h.pt = avtVector(0., 0., BOX_SIZE); // replace later.
    h.callback = ResizeCallback3;
    origHotPoints.push_back(h);

    //
    // Add the resize hotpoint.
    //
    h.pt = avtVector(BOX_SIZE, BOX_SIZE, BOX_SIZE); // replace later.
    h.callback = ResizeCallback4;
    origHotPoints.push_back(h);

    //
    // Set up some defaults for the plane equation.
    //
    float bounds[6];
    proxy.GetBounds(bounds);
    double extents[6];
    extents[0] = bounds[0];
    extents[1] = bounds[1];
    extents[2] = bounds[2];
    extents[3] = bounds[3];
    extents[4] = bounds[4];
    extents[5] = bounds[5];
    Interface.SetExtents(extents);

    addedOutline = false;
    addedBbox    = false;

    hotPoints = origHotPoints;
    CreateBoxActor();
    CreateTextActors();
    CreateOutline();
}

// ****************************************************************************
// Method: VisitBoxTool::~VisitBoxTool
//
// Purpose: 
//   This is the destructor for the plane tool class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:15 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

VisitBoxTool::~VisitBoxTool()
{
    if(boxActor != NULL)
    {
        boxActor->Delete();
        boxActor = NULL;
    }

    if(boxMapper != NULL)
    {
        boxMapper->Delete();
        boxMapper = NULL;
    }

    if(boxData != NULL)
    {
        boxData->Delete();
        boxData = NULL;
    }

    // Delete the text mappers and actors
    DeleteTextActors();
    DeleteOutline();
}

// ****************************************************************************
// Method: VisitBoxTool::Enable
//
// Purpose: 
//   This method enables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:16 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::Enable()
{
    bool val = IsEnabled();
    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if(!val)
    {
        UpdateTool();
        proxy.GetCanvas()->AddActor(boxActor);
        AddText();
    }
}

// ****************************************************************************
// Method: VisitBoxTool::Disable
//
// Purpose: 
//   This method disables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:17 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::Disable()
{
    bool val = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    if(val)
    {
        proxy.GetCanvas()->RemoveActor(boxActor);
        RemoveText();
    }

    // Make sure that the tool does not use the bounds stored in its
    // interface the next time it is enabled.
    Interface.UnInitialize();
}

// ****************************************************************************
// Method: VisitBoxTool::IsAvailable
//
// Purpose: 
//   Returns whether or not the tool is available for use.
//
// Returns:    Whether or not the tool is available for use.
//
// Note:       This may have to change later.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:18 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
VisitBoxTool::IsAvailable() const
{
    return (proxy.GetMode() == WINMODE_3D) && proxy.HasPlots();
}

// ****************************************************************************
// Method: VisitBoxTool::Start2DMode
//
// Purpose: 
//   This method switches the tool to 2D mode. In this case, the tool is
//   turned off.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:18 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool::Start2DMode()
{
    Disable();
}

// ****************************************************************************
// Method: VisitBoxTool::Stop3DMode
//
// Purpose: 
//   This method tells the tool that 3D mode is stopping. The tool is disabled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:19 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool::Stop3DMode()
{
    Disable();
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
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:20 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002 
//   Use vtkTextProperty to set actor color instead of vtkProperty.
//
// ****************************************************************************

void
VisitBoxTool::SetForegroundColor(float r, float g, float b)
{
    float color[3] = {r, g, b};

    // Change the colors in the box.
    boxActor->GetProperty()->SetColor(color);

    // Set the colors of the text actors.
    originTextActor->GetTextProperty()->SetColor(color);
    for(int i = 0; i < 4; ++i)
        cornerTextActor[i]->GetTextProperty()->SetColor(color);
}

// ****************************************************************************
// Method: VisitBoxTool::UpdateView
//
// Purpose: 
//   Updates the color of the normal vector based on the camera.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:23 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::UpdateView()
{
    if(IsEnabled())
    {
        UpdateText();
    }
}

// ****************************************************************************
// Method: VisitBoxTool::UpdateTool
//
// Purpose: 
//   Repostions the tool using the attributes stored in the Interface.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:25 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool::UpdateTool()
{
    // If the interface has not been initialized, use the bounding box
    // as the extents.
    if(!Interface.Initialized())
    {
        float e[6];
        proxy.GetBounds(e);
        double d[6];
        for(int i = 0; i < 6; ++i)
            d[i] = double(e[i]);
        Interface.SetExtents(d);
    }

    // Use the extents from the interface.
    const double *extents = Interface.GetExtents();
    double dX = extents[1] - extents[0];
    double dY = extents[3] - extents[2];
    double dZ = extents[5] - extents[4];
    SMtx.MakeScale(dX, dY, dZ);
    TMtx.MakeTranslate(extents[0],
                       extents[2],
                       extents[4]);

    DoTransformations();
    UpdateText();
}

// ****************************************************************************
// Method: VisitBoxTool::CreateBoxActor
//
// Purpose: 
//   Creates the vector actor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:25 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::CreateBoxActor()
{
    // Store the colors and points in the polydata.
    vtkOutlineSource *source = vtkOutlineSource::New();
    float extents[6];
    extents[0] = 0.;
    extents[1] = BOX_SIZE;
    extents[2] = 0.;
    extents[3] = BOX_SIZE;
    extents[4] = 0.;
    extents[5] = BOX_SIZE;
    source->SetBounds(extents);
    boxData = source->GetOutput();
    boxData->Register(NULL);

    boxMapper = vtkPolyDataMapper::New();
    boxMapper->SetInput(boxData);

    boxActor = vtkActor::New();
    boxActor->GetProperty()->SetRepresentationToWireframe();
    boxActor->GetProperty()->SetLineWidth(2.);
    vtkMatrix4x4 *m = vtkMatrix4x4::New(); m->Identity();
    boxActor->SetUserMatrix(m);
    boxActor->SetMapper(boxMapper);
    m->Delete();
    source->Delete();
}

// ****************************************************************************
// Method: VisitBoxTool::CreateTextActors
//
// Purpose: 
//   Create the text actors and mappers used to draw the origin/normal info.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:26 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
// ****************************************************************************

void
VisitBoxTool::CreateTextActors()
{
    originTextActor = vtkTextActor::New();
    originTextActor->ScaledTextOff(); 

    for(int i = 0; i < 4; ++i)
    {
        cornerTextActor[i] = vtkTextActor::New();
        cornerTextActor[i]->ScaledTextOff();
    }
}

// ****************************************************************************
// Method: VisitBoxTool::DeleteTextActors
//
// Purpose: 
//   Deletes the text actors and mappers.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:27 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   textMappers no longer required.
//
// ****************************************************************************

void
VisitBoxTool::DeleteTextActors()
{
    if(originTextActor != NULL)
    {
        originTextActor->Delete();
        originTextActor = NULL;
    }

    for(int i = 0; i < 4; ++i)
    {
        if(cornerTextActor[i] != NULL)
        {
            cornerTextActor[i]->Delete();
            cornerTextActor[i] = NULL;
        }

    }
}

// ****************************************************************************
// Method: VisitBoxTool::AddText
//
// Purpose: 
//   Adds the text actors to the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:27 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::AddText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->AddActor2D(originTextActor);
    for(int i = 0; i < 4; ++i)
        proxy.GetForeground()->AddActor2D(cornerTextActor[i]);
#endif
}

// ****************************************************************************
// Method: VisitBoxTool::RemoveText
//
// Purpose: 
//   Removes the text actors from the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:28 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::RemoveText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->RemoveActor2D(originTextActor);
    for(int i = 0; i < 4; ++i)
        proxy.GetForeground()->RemoveActor2D(cornerTextActor[i]);
#endif
}

// ****************************************************************************
// Method: VisitBoxTool::UpdateText
//
// Purpose: 
//   Updates the info that the text actors display.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:30 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace mapper with actor.
//
// ****************************************************************************

void
VisitBoxTool::UpdateText()
{
    char str[100];
    sprintf(str, "Origin <%1.3g %1.3g %1.3g>", hotPoints[0].pt.x,
            hotPoints[0].pt.y, hotPoints[0].pt.z);
    originTextActor->SetInput(str);
    avtVector originScreen = ComputeWorldToDisplay(hotPoints[0].pt);
    float pt[3] = {originScreen.x, originScreen.y, 0.};
    originTextActor->GetPositionCoordinate()->SetValue(pt);

    for(int i = 1; i < 5; ++i)
    {
        if(i == 1)
            sprintf(str, " X = %1.3g", hotPoints[i].pt.x);
        else if(i == 2)
            sprintf(str, " Y = %1.3g", hotPoints[i].pt.y);
        else if(i == 3)
            sprintf(str, " Z = %1.3g", hotPoints[i].pt.z);
        else
            sprintf(str, " XYZ<%1.3g %1.3g %1.3g>",
                hotPoints[i].pt.x, hotPoints[i].pt.y, hotPoints[i].pt.z);
        cornerTextActor[i-1]->SetInput(str);
        avtVector originScreen = ComputeWorldToDisplay(hotPoints[i].pt);
        float pt[3] = {originScreen.x, originScreen.y, 0.};
        cornerTextActor[i-1]->GetPositionCoordinate()->SetValue(pt);
    }
}

// ****************************************************************************
// Method: VisitBoxTool::CreateOutline
//
// Purpose: 
//   Creates the outline objects that are shown when moving or resizing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:23:58 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002 
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//   
// ****************************************************************************

void
VisitBoxTool::CreateOutline()
{
    int i;

    for(i = 0; i < 3; ++i)
    {
        outlineData[i] = NULL;
        outlineMapper[i] = vtkPolyDataMapper::New();
        outlineActor[i] = vtkActor::New();
        outlineActor[i]->GetProperty()->SetLineWidth(1.);
        outlineActor[i]->SetMapper(outlineMapper[i]);
    }

    for(i = 0; i < 4; ++i)
    {
        outlineTextActor[i] = vtkTextActor::New();
        outlineTextActor[i]->ScaledTextOff();
    }
}

// ****************************************************************************
// Method: VisitBoxTool::DeleteOutline
//
// Purpose: 
//   Deletes the outline objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:24:22 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   textMappers no longer required.
//   
// ****************************************************************************

void
VisitBoxTool::DeleteOutline()
{
    int i;

    for(i = 0; i < 3; ++i)
    {
        if(outlineActor[i] != NULL)
        {
            outlineActor[i]->Delete();
            outlineActor[i] = NULL;
        }

        if(outlineMapper[i] != NULL)
        {
            outlineMapper[i]->Delete();
            outlineMapper[i] = NULL;
        }

        if(outlineData[i] != NULL)
        {
            outlineData[i]->Delete();
            outlineData[i] = NULL;
        }
    }

    for(i = 0; i < 4; ++i)
    {
        if(outlineTextActor[i] != NULL)
        {
            outlineTextActor[i]->Delete();
            outlineTextActor[i] = NULL;
        }
    }
}

// ****************************************************************************
// Method: VisitBoxTool::AddOutline
//
// Purpose: 
//   Adds the outline to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:24:42 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool::AddOutline()
{
    if(proxy.HasPlots())
    {
        addedOutline = true;
        UpdateOutline();

        int i, n = (activeHotPoint == 0 || activeHotPoint == 4) ? 3 : 2;
        for(i = 0; i < n; ++i)
            proxy.GetCanvas()->AddActor(outlineActor[i]);

#ifndef NO_ANNOTATIONS
        for(i = 0; n == 2 && i < 4; ++i)
            proxy.GetForeground()->AddActor2D(outlineTextActor[i]);
#endif
    }
    else
    {
        addedOutline = false;
    }
}

// ****************************************************************************
// Method: VisitBoxTool::RemoveOutline
//
// Purpose: 
//   Removes the outline from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:25:00 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool::RemoveOutline()
{
    if (addedOutline)
    {

        int i, n = (activeHotPoint == 0 || activeHotPoint == 4) ? 3 : 2;
        for(i = 0; i < n; ++i)
            proxy.GetCanvas()->RemoveActor(outlineActor[i]);

#ifndef NO_ANNOTATIONS
        for(i = 0; n == 2 && i < 4; ++i)
            proxy.GetForeground()->RemoveActor2D(outlineTextActor[i]);
#endif
    }
    addedOutline = false;
}

// ****************************************************************************
// Method: VisitBoxTool:: GetBoundingBoxOutline
//
// Purpose: 
//   Gets the 4 vertices used to draw a outline plane.
//
// Arguments:
//   a       : The index of the plane.
//   verts   : The return vertex array.
//   giveMin : Whether or not to return the min plane.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:25:19 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool:: GetBoundingBoxOutline(int a, avtVector *verts, bool giveMin)
{
    float extents[6];
    proxy.GetBounds(extents);

    if(a == 1)
    {
        float x = giveMin ? hotPoints[0].pt.x : hotPoints[1].pt.x;
        verts[0] = avtVector(x, extents[2], extents[4]);
        verts[1] = avtVector(x, extents[3], extents[4]);
        verts[2] = avtVector(x, extents[3], extents[5]);
        verts[3] = avtVector(x, extents[2], extents[5]);
    }
    else if(a == 2)
    {
        float y = giveMin ? hotPoints[0].pt.y : hotPoints[2].pt.y;
        verts[0] = avtVector(extents[0], y, extents[4]);
        verts[1] = avtVector(extents[1], y, extents[4]);
        verts[2] = avtVector(extents[1], y, extents[5]);
        verts[3] = avtVector(extents[0], y, extents[5]);
    }
    else if(a == 3)
    {
        float z = giveMin ? hotPoints[0].pt.z : hotPoints[3].pt.z;
        verts[0] = avtVector(extents[0], extents[2], z);
        verts[1] = avtVector(extents[1], extents[2], z);
        verts[2] = avtVector(extents[1], extents[3], z);
        verts[3] = avtVector(extents[0], extents[3], z);
    }
}

// ****************************************************************************
// Method: VisitBoxTool::UpdateOutline
//
// Purpose: 
//   Updates the outline.
//
// Note:       Updates the outline as either intersecting boxes or planes. It
//             depends on the active hot point.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:26:49 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace mapper with actor.
//   
// ****************************************************************************

void
VisitBoxTool::UpdateOutline()
{
    if(!addedOutline)
        return;

    if(activeHotPoint == 0 || activeHotPoint == 4)
    {
        float totalExtents[6];
        proxy.GetBounds(totalExtents);

        for(int i = 0; i < 3; ++i)
        {
            if(outlineData[i] != NULL)
                outlineData[i]->Delete();

             vtkOutlineSource *source = vtkOutlineSource::New();
             float extents[6];
             extents[0] = (i == 0) ? totalExtents[0] : hotPoints[0].pt.x;
             extents[1] = (i == 0) ? totalExtents[1] : hotPoints[4].pt.x;
             extents[2] = (i == 1) ? totalExtents[2] : hotPoints[0].pt.y;
             extents[3] = (i == 1) ? totalExtents[3] : hotPoints[4].pt.y;
             extents[4] = (i == 2) ? totalExtents[4] : hotPoints[0].pt.z;
             extents[5] = (i == 2) ? totalExtents[5] : hotPoints[4].pt.z;
             source->SetBounds(extents);
             outlineData[i] = source->GetOutput();
             outlineData[i]->Register(NULL);
             source->Delete();
             // Set the mapper's input to be the new dataset.
             outlineMapper[i]->SetInput(outlineData[i]);
             float color[3];
             color[0] = (i == 0) ? 1. : 0.;
             color[1] = (i == 1) ? 1. : 0.;
             color[2] = (i == 2) ? 1. : 0.;
             outlineActor[i]->GetProperty()->SetColor(color);
        }
    }
    else
    {
        int  nverts = 4;
        int  plane[6];
        bool planeFlag[6];
        avtVector verts[4];

        plane[0] = activeHotPoint; plane[1] = activeHotPoint;
        planeFlag[0] = true; planeFlag[1] = false;

        for(int j = 0; j < 2; ++j)
        {
            if(outlineData[j] != NULL)
                outlineData[j]->Delete();

            int numPts = 4;
            int numCells = 4;

            vtkPoints *pts = vtkPoints::New();
            pts->SetNumberOfPoints(numPts);
            vtkCellArray *lines = vtkCellArray::New();
            lines->Allocate(lines->EstimateSize(numCells, 2)); 
            vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
            colors->SetNumberOfComponents(3);
            colors->SetNumberOfTuples(numCells);

            // Store the colors and points in the polydata.
            outlineData[j] = vtkPolyData::New();
            outlineData[j]->Initialize();
            outlineData[j]->SetPoints(pts);
            outlineData[j]->SetLines(lines);
            outlineData[j]->GetCellData()->SetScalars(colors);
            pts->Delete(); lines->Delete(); colors->Delete();

            GetBoundingBoxOutline(plane[j], verts, planeFlag[j]);

            //
            // Now that we have a clipped polygon, create a polydata from that.
            //
            float fg[3];
            proxy.GetForegroundColor(fg);
            unsigned char r = (unsigned char)(fg[0] * 255.);
            unsigned char g = (unsigned char)(fg[1] * 255.);
            unsigned char b = (unsigned char)(fg[2] * 255.);
            for(int i = 0, index = 0; i < nverts; ++i, index += 3)
            {
                // Add points to the vertex list.
                float coord[3];
                coord[0] = verts[i].x;
                coord[1] = verts[i].y;
                coord[2] = verts[i].z;
                pts->SetPoint(i, coord);

                // Add a cell to the polydata.
                vtkIdType   ptIds[2];
                ptIds[0] = i;
                ptIds[1] = (i < (nverts - 1)) ? (i + 1) : 0;
                lines->InsertNextCell(2, ptIds);

                // Store the color.
                unsigned char *rgb = colors->GetPointer(index);
                rgb[0] = r;
                rgb[1] = g;
                rgb[2] = b;
            }

            // Set the mapper's input to be the new dataset.
            outlineMapper[j]->SetInput(outlineData[j]);
        }

        //
        // Update the text along the edge of the outline.
        //
        char str[100];
        if(activeHotPoint == 1)
            sprintf(str, " X = %1.3g", hotPoints[1].pt.x);
        else if(activeHotPoint == 2)
            sprintf(str, " Y = %1.3g", hotPoints[2].pt.y);
        else
            sprintf(str, " Z = %1.3g", hotPoints[3].pt.z);
        for(int i = 0; i < 4; ++i)
        {
            outlineTextActor[i]->SetInput(str);
            avtVector originScreen = ComputeWorldToDisplay(verts[i]);
            float pt[3] = {originScreen.x, originScreen.y, 0.};
            outlineTextActor[i]->GetPositionCoordinate()->SetValue(pt);
        }
    }
}

// ****************************************************************************
// Method: VisitBoxTool::CallCallback
//
// Purpose: 
//   Lets the outside world know that the tool has a new slice plane.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:31 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisitBoxTool::CallCallback()
{
    // Fill the extents array that we'll give to the client. Make sure the
    // min extents are actually first.
    double extents[6];
    bool minFirstX = (hotPoints[0].pt.x < hotPoints[1].pt.x);
    bool minFirstY = (hotPoints[0].pt.y < hotPoints[2].pt.y);
    bool minFirstZ = (hotPoints[0].pt.z < hotPoints[3].pt.z);
    extents[0] = minFirstX ? hotPoints[0].pt.x : hotPoints[1].pt.x;
    extents[1] = minFirstX ? hotPoints[1].pt.x : hotPoints[0].pt.x;
    extents[2] = minFirstY ? hotPoints[0].pt.y : hotPoints[2].pt.y;
    extents[3] = minFirstY ? hotPoints[2].pt.y : hotPoints[0].pt.y;
    extents[4] = minFirstZ ? hotPoints[0].pt.z : hotPoints[3].pt.z;
    extents[5] = minFirstZ ? hotPoints[3].pt.z : hotPoints[0].pt.z;

    Interface.SetExtents(extents);
    Interface.ExecuteCallback();
}

// ****************************************************************************
// Method: VisitBoxTool::InitialActorSetup
//
// Purpose: 
//   Makes the text and outline actors active and starts bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:31 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitBoxTool::InitialActorSetup()
{
    // Enter bounding box mode if there are plots.
    if(proxy.HasPlots())
    {
        addedBbox = true;
        proxy.StartBoundingBox();
    }

    AddOutline();
}

// ****************************************************************************
// Method: VisitBoxTool::FinalActorSetup
//
// Purpose: 
//   Removes certain actors from the renderer and ends bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 12:22:32 PDT 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  3 17:03:34 PST 2003 
//   If transparencies exist, have the plots recalculate render order, so
//   that this tool is rendered before the transparent actors. 
//
// ****************************************************************************

void
VisitBoxTool::FinalActorSetup()
{
    // End bounding box mode.
    if(addedBbox)
    {
        proxy.EndBoundingBox();
    }
    addedBbox = false;

    RemoveOutline();
    if (proxy.TransparenciesExist())
        proxy.RecalculateRenderOrder();
}

// ****************************************************************************
//  Method:  VisitBoxTool::Translate
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
//  Creation:    Wed Oct 30 12:22:33 PDT 2002
//
//  Modifications:
// 
// ****************************************************************************

void
VisitBoxTool::Translate(CB_ENUM e, int, int shift, int x, int y)
{
    if(shift)
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

        if(depthTranslate)
            depthTranslationDistance = ComputeDepthTranslationDistance();

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
        avtVector motion;

        if(depthTranslate)
            motion = depthTranslationDistance * double(y - lastY);
        else
            motion = newPoint - oldPoint;

        avtMatrix T(avtMatrix::CreateTranslate(motion));
        TMtx =  T * TMtx;
        DoTransformations();

        // Update the text and outline actors.
        UpdateText();
        UpdateOutline();

        // Render the window
        proxy.Render();
    }
    else
    {
        // Call the tool's callback.
        CallCallback();

        // Remove the right actors.
        FinalActorSetup();

        depthTranslate = false;
    }
}

// ****************************************************************************
//  Method:  VisitBoxTool::Resize
//
//  Purpose:
//    This is the handler method that is called when the resize hotpoint
//    is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Oct 30 12:22:36 PDT 2002
//
//  Modifications:
// 
// ****************************************************************************

#define TOLERANCE 0.0001

void
VisitBoxTool::Resize(CB_ENUM e, int, int, int x, int y)
{
    HotPoint &origin = hotPoints[0];
    HotPoint &resize = hotPoints[activeHotPoint];

    float dX, dY, dZ;
    avtVector originScreen, resizeScreen;

    if(e == CB_START)
    {
        originScreen = ComputeWorldToDisplay(origin.pt);
        resizeScreen = ComputeWorldToDisplay(resize.pt);

        if(activeHotPoint == 4)
        {
            dY = originScreen.y - resizeScreen.y;
            originalDistance = dY;
            if(originalDistance < TOLERANCE && originalDistance > -TOLERANCE)
                originalDistance = TOLERANCE;
        }

        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        double scale;

        if(activeHotPoint == 4)
        {
             originScreen = ComputeWorldToDisplay(origin.pt);
             dY = originScreen.y - double(y);
             if(dY < TOLERANCE && dY > -TOLERANCE)
                 dY = TOLERANCE;
             scale = dY / originalDistance;
             if(scale < TOLERANCE && scale > -TOLERANCE)
                 scale = TOLERANCE;
        }
        else
        {
            float extents[6];
            proxy.GetBounds(extents);
            int *size = proxy.GetCanvas()->GetSize();

            int    screenDelta = y - lastY;
            float wLength;
            dX = extents[1] - extents[0];
            dY = extents[3] - extents[2];
            dZ = extents[5] - extents[4];
            if(activeHotPoint == 1)
                wLength = dX;
            else if(activeHotPoint == 2)
                wLength = dY;
            else
                wLength = dZ;

            if(wLength > TOLERANCE && wLength < TOLERANCE)
                wLength = TOLERANCE;

            float worldDelta = (float(screenDelta) / float(size[1])) * wLength;

            avtVector motion(resize.pt - origin.pt);
            float currentDist;
            if(activeHotPoint == 1)
                currentDist = motion.x;
            else if(activeHotPoint == 2)
                currentDist = motion.y;
            else
                currentDist = motion.z;

            if(currentDist > -TOLERANCE && currentDist < TOLERANCE)
                currentDist = TOLERANCE;

            scale = (worldDelta + currentDist) / currentDist;
        }

        // Do the hotpoint and actor transformations
        avtMatrix S;
        if(activeHotPoint == 1)
            S = avtMatrix::CreateScale(scale, 1., 1.);
        else if(activeHotPoint == 2)
            S = avtMatrix::CreateScale(1., scale, 1.);
        else if(activeHotPoint == 3)
            S = avtMatrix::CreateScale(1., 1., scale);
        else if(activeHotPoint == 4)
            S = avtMatrix::CreateScale(scale);

        SMtx = S * SMtx;
        DoTransformations();

        // Save the distance for next time through.
        originalDistance = dY;

        UpdateOutline();

        // Render the window
        proxy.Render();
    }
    else
    {
        // Call the tool's callback.
        CallCallback();

        // Remove the right actors.
        FinalActorSetup();
    }
}

// ****************************************************************************
//  Method:  VisitBoxTool::DoTransformations
//
//  Purpose:
//    Applies the current transformation to the hotpoints.
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Oct 30 12:22:38 PDT 2002
//
//  Modifications:
//
// ****************************************************************************

void
VisitBoxTool::DoTransformations()
{
    avtMatrix M = TMtx * SMtx;

    for (int i=0; i<hotPoints.size(); i++)
        hotPoints[i].pt = M * origHotPoints[i].pt;

    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    tmp->DeepCopy(M.GetElements());
    boxActor->SetUserMatrix(tmp);
    tmp->Delete();
}


// ****************************************************************************
//  Method:  VisitBoxTool::ReAddToWindow
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
VisitBoxTool::ReAddToWindow()
{
    if(IsEnabled())
    {
        proxy.GetCanvas()->RemoveActor(boxActor);
        proxy.GetCanvas()->AddActor(boxActor);
    }
}


//
// Static callback functions.
//

void
VisitBoxTool::TranslateCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitBoxTool *bt = (VisitBoxTool *)it;
    bt->SetActiveHotPoint(0);
    bt->Translate(e, ctrl, shift, x, y);
}

void
VisitBoxTool::ResizeCallback1(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitBoxTool *bt = (VisitBoxTool *)it;
    bt->SetActiveHotPoint(1);
    bt->Resize(e, ctrl, shift, x, y);
}

void
VisitBoxTool::ResizeCallback2(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitBoxTool *bt = (VisitBoxTool *)it;
    bt->SetActiveHotPoint(2);
    bt->Resize(e, ctrl, shift, x, y);
}

void
VisitBoxTool::ResizeCallback3(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitBoxTool *bt = (VisitBoxTool *)it;
    bt->SetActiveHotPoint(3);
    bt->Resize(e, ctrl, shift, x, y);
}

void
VisitBoxTool::ResizeCallback4(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitBoxTool *bt = (VisitBoxTool *)it;
    bt->SetActiveHotPoint(4);
    bt->Resize(e, ctrl, shift, x, y);
}
