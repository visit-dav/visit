#include <math.h>
#include <VisitPlaneTool.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <avtVector.h>

#define PLANE_SIZE         1.
#define NUM_ARROWS         3
#define ARROW_NUMSIDES     12
#define ARROW_LENGTH       PLANE_SIZE
#define ARROW_HEAD_LENGTH  (0.15 * ARROW_LENGTH)
#define ARROW_HEAD_RADIUS  (ARROW_HEAD_LENGTH * 0.5)
#define ARROW_SHAFT_RADIUS (ARROW_HEAD_RADIUS * 0.4)
#define CELLS_PER_ARROW    (4 * ARROW_NUMSIDES)

// ****************************************************************************
// Method: VisitPlaneTool::VisitPlaneTool
//
// Purpose: 
//   This is the constructor for the plane tool.
//
// Arguments:
//   p : A reference to the tool proxy.
//
// Programmer: Brad Whitlock, Jeremy Meredith
// Creation:   Mon Oct 8 11:42:07 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:11:26 PDT 2002
//   Set up some defaults for the plane equation.
//
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Initialized booleans for what has been added to the screen.
//
//   Brad Whitlock, Fri Oct 18 15:15:31 PST 2002
//   Created radius text.
//
// ****************************************************************************

VisitPlaneTool::VisitPlaneTool(VisWindowToolProxy &p) : VisitInteractiveTool(p),
    Interface(p)
{
    normalAway = false;

    HotPoint h;
    h.radius = 1./60.; // See what a good value is.
    h.tool = this;

    //
    // Add the plane origin hotpoint.
    //
    h.pt = avtVector(0.,  0.,  0.);
    h.callback = TranslateCallback;
    origHotPoints.push_back(h);

    //
    // Add the rotate X hotpoint.
    //
    h.pt = avtVector(0., PLANE_SIZE,  0.); // replace later.
    h.callback = RotateXCallback;
    origHotPoints.push_back(h);

    //
    // Add the rotate Y hotpoint.
    //
    h.pt = avtVector(PLANE_SIZE,  0.,  0.); // replace later.
    h.callback = RotateYCallback;
    origHotPoints.push_back(h);

    //
    // Add the free rotate hotpoint.
    //
    h.pt = avtVector(0.,  0., PLANE_SIZE); // replace later.
    h.callback = FreeRotateCallback;
    origHotPoints.push_back(h);

    //
    // Add the resize hotpoint.
    //
    h.pt = avtVector(PLANE_SIZE, PLANE_SIZE,  0.); // replace later.
    h.callback = ResizeCallback;
    origHotPoints.push_back(h);

    //
    // Add the translate along normal hotpoint.
    //
    h.pt = avtVector(0., 0., PLANE_SIZE * 0.5); // replace later.
    h.callback = TranslateNormalCallback;
    origHotPoints.push_back(h);

    //
    // Set up some defaults for the plane equation.
    //
    float bounds[6];
    proxy.GetBounds(bounds);
    double dX = bounds[1] - bounds[0];
    double dY = bounds[3] - bounds[2];
    double dZ = bounds[5] - bounds[4];
    double rad = 0.5 * sqrt((dX * dX + dY * dY) * 0.5);
    Interface.SetOrigin(bounds[0] + 0.5 * dX,
                        bounds[2] + 0.5 * dY,
                        bounds[4] + 0.5 * dZ);
    Interface.SetRadius(rad);

    addedOutline = false;
    addedBbox    = false;

    hotPoints = origHotPoints;
    CreateVectorActor();
    CreatePlaneActor();
    CreateTextActors();
    CreateOutline();
}

// ****************************************************************************
// Method: VisitPlaneTool::~VisitPlaneTool
//
// Purpose: 
//   This is the destructor for the plane tool class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:42:45 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

VisitPlaneTool::~VisitPlaneTool()
{
    if(vectorActor != NULL)
    {
        vectorActor->Delete();
        vectorActor = NULL;
    }

    if(vectorMapper != NULL)
    {
        vectorMapper->Delete();
        vectorMapper = NULL;
    }

    if(vectorData != NULL)
    {
        vectorData->Delete();
        vectorData = NULL;
    }

    if(planeActor != NULL)
    {
        planeActor->Delete();
        planeActor = NULL;
    }

    if(planeMapper != NULL)
    {
        planeMapper->Delete();
        planeMapper = NULL;
    }

    if(planeData != NULL)
    {
        planeData->Delete();
        planeData = NULL;
    }

    // Delete the text mappers and actors
    DeleteTextActors();

    // Delete the outline actor and mapper.
    DeleteOutline();
}

// ****************************************************************************
// Method: VisitPlaneTool::Enable
//
// Purpose: 
//   This method enables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:45:45 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Feb 11 17:00:34 PST 2002
//   Rewrote it so it calls the UpdateTool method to position the plane.
//   Added code to add text.
//
// ****************************************************************************

void
VisitPlaneTool::Enable()
{
    bool val = IsEnabled();
    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if(!val)
    {
        UpdateTool();
        proxy.GetCanvas()->AddActor(vectorActor);
        proxy.GetCanvas()->AddActor(planeActor);
        AddText();
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::Disable
//
// Purpose: 
//   This method disables the tool.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:46:00 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:20:59 PDT 2002
//   Added code to remove text.
//
// ****************************************************************************

void
VisitPlaneTool::Disable()
{
    bool val = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    if(val)
    {
        proxy.GetCanvas()->RemoveActor(vectorActor);
        proxy.GetCanvas()->RemoveActor(planeActor);
        RemoveText();
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::IsAvailable
//
// Purpose: 
//   Returns whether or not the tool is available for use.
//
// Returns:    Whether or not the tool is available for use.
//
// Note:       This may have to change later.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 10:23:02 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jul 23 10:18:35 PDT 2002
//   I made the tool unavailable unless there are plots.
//   
// ****************************************************************************

bool
VisitPlaneTool::IsAvailable() const
{
    return (proxy.GetMode() == WINMODE_3D) && proxy.HasPlots();
}

// ****************************************************************************
// Method: VisitPlaneTool::Start2DMode
//
// Purpose: 
//   This method switches the tool to 2D mode. In this case, the tool is
//   turned off.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:46:54 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::Start2DMode()
{
    Disable();
}

// ****************************************************************************
// Method: VisitPlaneTool::Stop3DMode
//
// Purpose: 
//   This method tells the tool that 3D mode is stopping. The tool is disabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:47:35 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::Stop3DMode()
{
    Disable();
}

// ****************************************************************************
// Method: VisitPlaneTool::SetForegroundColor
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
// Creation:   Mon Oct 8 11:43:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:16:59 PDT 2002
//   Added the upAxis actor.
//
//   Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002  
//   vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002 
//   Use vtkTextProperty to set actor color instead of vtkProperty.
//
// ****************************************************************************

void
VisitPlaneTool::SetForegroundColor(float r, float g, float b)
{
    int                   i, index;
    vtkDataArray         *scalars;
    vtkUnsignedCharArray *colorData;
    unsigned char ur = (unsigned char)(r * 255);
    unsigned char ug = (unsigned char)(g * 255);
    unsigned char ub = (unsigned char)(b * 255);
    //
    // Change the colors in the plane
    //
    scalars = planeData->GetCellData()->GetScalars();
    colorData = vtkUnsignedCharArray::SafeDownCast(scalars);
    for(i = 0, index = 0; i < 4; ++i, index += 3)
    {
        unsigned char *rgb = colorData->GetPointer(index);
        rgb[0] = ur;
        rgb[1] = ug;
        rgb[2] = ub;
    }
    planeData->Modified();

    scalars = vectorData->GetCellData()->GetScalars();
    colorData = vtkUnsignedCharArray::SafeDownCast(scalars);
    for(i = CELLS_PER_ARROW, index = CELLS_PER_ARROW * 3;
        i < (NUM_ARROWS * CELLS_PER_ARROW);
        ++i, index += 3)
    {
        unsigned char *rgb = colorData->GetPointer(index);
        rgb[0] = ur;
        rgb[1] = ug;
        rgb[2] = ub;
    }
    vectorData->Modified();

    //
    // Update the colors of the normal vector if it is facing us.
    //
    if(!normalAway)
       SetAwayColor(r, g, b);

    // Delete the text mappers/actors so they will be created again using
    // the right colors.
    float color[3] = {r, g, b};
    originTextActor->GetTextProperty()->SetColor(color);
    normalTextActor->GetTextProperty()->SetColor(color);
    upAxisTextActor->GetTextProperty()->SetColor(color);
}

// ****************************************************************************
// Method: VisitPlaneTool::UpdateView
//
// Purpose: 
//   Updates the color of the normal vector based on the camera.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 16:36:54 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:54:06 PDT 2002
//   Added code to update the text.
//
// ****************************************************************************

void
VisitPlaneTool::UpdateView()
{
    if(IsEnabled())
    {
        UpdateNormalVectorColor();
        UpdateText();
    }
}

// ****************************************************************************
// Method: VisItPlaneTool::UpdateTool
//
// Purpose: 
//   Repostions the tool using the attributes stored in the Interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 09:46:00 PDT 2002
//
// Modifications:
//   Brad Whitlock, Wed Oct 9 11:38:34 PDT 2002
//   I made it so it can get the scale from the interface's attributes if
//   a scale was provided through them.
//
// ****************************************************************************

void
VisitPlaneTool::UpdateTool()
{
    avtVector from(0,0,0);
    avtVector at(Interface.GetNormal()[0],
                 Interface.GetNormal()[1],
                 Interface.GetNormal()[2]);
    avtVector up(Interface.GetUpAxis()[0],
                 Interface.GetUpAxis()[1],
                 Interface.GetUpAxis()[2]);
    RMtx.MakeRBT(from, at, up);
    TMtx.MakeTranslate(Interface.GetOrigin()[0],
                       Interface.GetOrigin()[1],
                       Interface.GetOrigin()[2]);

    // Calculate the scale based on the bounds because there is nothing
    // else in the code yet that can provide the radius that is used
    // to calculate the scale.
    double scale;
    if(Interface.GetHaveRadius())
    {
        // The scale is the radius divided by the square root of 2.
        scale = Interface.GetRadius() / 1.414214;
    }
    else
    {
        float bounds[6];
        proxy.GetBounds(bounds);
        double dX = bounds[1] - bounds[0];
        double dY = bounds[3] - bounds[2];
        scale = 0.5 * sqrt(dX * dX + dY * dY) / 1.414214;
    }
    SMtx.MakeScale(scale);
    DoTransformations();

    UpdateNormalVectorColor();
    UpdateText();
}

// ****************************************************************************
// Method: VisitPlaneTool::Normal
//
// Purpose: 
//   Returns the unit-length plane normal.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 14:30:46 PST 2001
//
// Modifications:
//   
// ****************************************************************************

avtVector
VisitPlaneTool::Normal() const
{
    avtVector normal(hotPoints[3].pt - hotPoints[0].pt);
    normal.normalize();
    return normal;
}

// ****************************************************************************
// Method: VisitPlaneTool::CreatePlaneActor
//
// Purpose: 
//   Creates the plane actor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:44:22 PDT 2001
//
// Modifications:
//
//   Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//   Make ptIds of type vtkIdType to match VTK 4.0 API.
//   
//   Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//   vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray for
//   colors instead.
//
//   Kathleen Bonnell, Fri Jul 26 10:52:09 PDT 2002    
//   Fix memory leak: Delete vtkMatrix4x4 m.
//
// ****************************************************************************

void
VisitPlaneTool::CreatePlaneActor()
{
    planeData = vtkPolyData::New();

    int numPts = 4;
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *lines = vtkCellArray::New();
    lines->Allocate(lines->EstimateSize(numPts, 2)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(4);

    // Store the colors and points in the polydata.
    planeData->Initialize();
    planeData->SetPoints(pts);
    planeData->SetLines(lines);
    planeData->GetCellData()->SetScalars(colors);
    pts->Delete(); lines->Delete(); colors->Delete(); 

    // Add points to the vertex list.
    float coord[3];
    coord[2] = 0.;
    coord[0] = PLANE_SIZE;
    coord[1] = PLANE_SIZE;
    pts->SetPoint(0, coord);
    coord[0] = -PLANE_SIZE;
    coord[1] = PLANE_SIZE;
    pts->SetPoint(1, coord);
    coord[0] = -PLANE_SIZE;
    coord[1] = -PLANE_SIZE;
    pts->SetPoint(2, coord);
    coord[0] = PLANE_SIZE;
    coord[1] = -PLANE_SIZE;
    pts->SetPoint(3, coord);

    // Add a cell to the polydata.
    vtkIdType   ptIds[2];
    ptIds[0] = 0;
    ptIds[1] = 1;
    lines->InsertNextCell(2, ptIds);
    ptIds[0] = 1;
    ptIds[1] = 2;
    lines->InsertNextCell(2, ptIds);
    ptIds[0] = 2;
    ptIds[1] = 3;
    lines->InsertNextCell(2, ptIds);
    ptIds[0] = 3;
    ptIds[1] = 0;
    lines->InsertNextCell(2, ptIds);

    // Write the colors into the array directly
    float fg[3];
    proxy.GetForegroundColor(fg);
    for(int i = 0, index = 0; i < 4; ++i, index += 3)
    {
        unsigned char *rgb = colors->GetPointer(index);
        rgb[0] = (unsigned char)(fg[0] * 255.);
        rgb[1] = (unsigned char)(fg[1] * 255.);
        rgb[2] = (unsigned char)(fg[2] * 255.);
    }

    planeMapper = vtkPolyDataMapper::New();
    planeMapper->SetInput(planeData);

    planeActor = vtkActor::New();
    planeActor->GetProperty()->SetLineWidth(2.);
    vtkMatrix4x4 *m = vtkMatrix4x4::New(); m->Identity();
    planeActor->SetUserMatrix(m);
    planeActor->SetMapper(planeMapper);
    m->Delete();
}

// ****************************************************************************
// Method: VisitPlaneTool::CreateVectorActor
//
// Purpose: 
//   Creates the vector actor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 8 11:44:40 PDT 2001
//
// Modifications:
//   
//   Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//   Make ptIds of type vtkIdType to match VTK 4.0 API.
//   
//   Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//   vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray for
//   colors instead.
//   
//   Kathleen Bonnell, Fri Jul 26 10:52:09 PDT 2002    
//   Fix memory leak: Delete vtkMatrix4x4 m.
//
// ****************************************************************************

void
VisitPlaneTool::CreateVectorActor()
{
    double dTheta = (2. * 3.14159) / double(ARROW_NUMSIDES);
    int numPtsPerVector = 3 * ARROW_NUMSIDES + 2;
    int numPts = (NUM_ARROWS * numPtsPerVector);
    int numCells = (NUM_ARROWS * CELLS_PER_ARROW);

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *polys = vtkCellArray::New();
    polys->Allocate(polys->EstimateSize(numCells, 4)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(numCells);

    // Store the colors and points in the polydata.
    vectorData = vtkPolyData::New();
    vectorData->Initialize();
    vectorData->SetPoints(pts);
    vectorData->SetPolys(polys);
    vectorData->GetCellData()->SetScalars(colors);
    pts->Delete(); polys->Delete(); colors->Delete(); 

    //
    // Create the points.
    //
    float  coords[3];
    vtkIdType    ptIds[4];
    int    i, ptIndex = 0;
    double theta;
    int    index[3][3] = {{0, 1, 2},{2, 1, 0},{0, 2, 1}};
    for(int vec = 0; vec < NUM_ARROWS; ++vec)
    {
        int offset = vec * numPtsPerVector;
        for(i = 0, theta = 0.; i < ARROW_NUMSIDES; ++i, theta += dTheta)
        {
            coords[index[vec][0]] = cos(theta) * ARROW_SHAFT_RADIUS;
            coords[index[vec][1]] = sin(theta) * ARROW_SHAFT_RADIUS;
            coords[index[vec][2]] = 0.;
            pts->SetPoint(ptIndex++, coords);
        }
        for(i = 0, theta = 0.; i < ARROW_NUMSIDES; ++i, theta += dTheta)
        {
            coords[index[vec][0]] = cos(theta) * ARROW_SHAFT_RADIUS;
            coords[index[vec][1]] = sin(theta) * ARROW_SHAFT_RADIUS;
            coords[index[vec][2]] = (ARROW_LENGTH - ARROW_HEAD_LENGTH);
            pts->SetPoint(ptIndex++, coords);
        }
        for(i = 0, theta = 0.; i < ARROW_NUMSIDES; ++i, theta += dTheta)
        {
            coords[index[vec][0]] = cos(theta) * ARROW_HEAD_RADIUS;
            coords[index[vec][1]] = sin(theta) * ARROW_HEAD_RADIUS;
            coords[index[vec][2]] = (ARROW_LENGTH - ARROW_HEAD_LENGTH);
            pts->SetPoint(ptIndex++, coords);
        }
        coords[index[vec][0]] = 0.;
        coords[index[vec][1]] = 0.;
        coords[index[vec][2]] = 0.;
        int startPoint = ptIndex;
        pts->SetPoint(ptIndex++, coords);
        coords[index[vec][0]] = 0.;
        coords[index[vec][1]] = 0.;
        coords[index[vec][2]] = ARROW_LENGTH;
        int endPoint = ptIndex;
        pts->SetPoint(ptIndex++, coords);

        //
        // Create the endpoint cells.
        //
        for(i = 0; i < ARROW_NUMSIDES; ++i)
        {
            ptIds[0] = startPoint;
            ptIds[1] = offset + i;
            ptIds[2] = offset + ((i < (ARROW_NUMSIDES - 1)) ? (i + 1) : 0);
            polys->InsertNextCell(3, ptIds);
        }

        //
        // Create the shaft cells.
        //
        for(i = 0; i < ARROW_NUMSIDES; ++i)
        {
            ptIds[0] = offset + i;
            ptIds[1] = offset + i + ARROW_NUMSIDES;
            ptIds[2] = offset + ((i < (ARROW_NUMSIDES - 1)) ? (i + ARROW_NUMSIDES + 1) : ARROW_NUMSIDES);
            ptIds[3] = offset + ((i < (ARROW_NUMSIDES - 1)) ? (i + 1) : 0);
            polys->InsertNextCell(4, ptIds);
        }

        //
        // Create the head-back cells.
        //
        for(i = 0; i < ARROW_NUMSIDES; ++i)
        {
            ptIds[0] = offset + i + ARROW_NUMSIDES;
            ptIds[1] = offset + i + ARROW_NUMSIDES * 2;
            ptIds[2] = offset + ((i < (ARROW_NUMSIDES - 1)) ? (i + ARROW_NUMSIDES * 2 + 1) : (ARROW_NUMSIDES * 2));
            ptIds[3] = offset + ((i < (ARROW_NUMSIDES - 1)) ? (i + ARROW_NUMSIDES + 1) : ARROW_NUMSIDES);
            polys->InsertNextCell(4, ptIds);
        }

        //
        // Create the arrow front cells.
        //
        for(i = 0; i < ARROW_NUMSIDES; ++i)
        {
            ptIds[0] = offset + i + ARROW_NUMSIDES * 2;
            ptIds[1] = endPoint;
            ptIds[2] = offset + ((i < (ARROW_NUMSIDES - 1)) ? (i + ARROW_NUMSIDES * 2 + 1) : (ARROW_NUMSIDES * 2));
            polys->InsertNextCell(3, ptIds);
        }
    }

    //
    // Write the cell colors into the array directly
    //
    float fg[3];
    proxy.GetForegroundColor(fg);
    int ii;
    for(i = 0, ii = 0; i < numCells; ++i, ii += 3)
    {
        unsigned char *rgb = colors->GetPointer(ii);
        rgb[0] = (unsigned char)(fg[0] * 255.);
        rgb[1] = (unsigned char)(fg[1] * 255.);
        rgb[2] = (unsigned char)(fg[2] * 255.);
    }

    vectorMapper = vtkPolyDataMapper::New();
    vectorMapper->SetInput(vectorData);

    vectorActor = vtkActor::New();
    vtkMatrix4x4 *m = vtkMatrix4x4::New(); m->Identity();
    vectorActor->SetUserMatrix(m);
    vectorActor->SetMapper(vectorMapper);
    m->Delete();
}

// ****************************************************************************
// Method: VisitPlaneTool::CreateTextActors
//
// Purpose: 
//   Create the text actors and mappers used to draw the origin/normal info.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 10:06:34 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:15:01 PDT 2002
//   Added upAxis actor.
//
//   Hank Childs, Tue May 21 13:37:44 PDT 2002
//   Removed configurable hooks to not instantiate actors.  This makes for less
//   of a maintenance problem.
//
//   Brad Whitlock, Fri Oct 18 15:17:07 PST 2002
//   Added radius actor.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace vtkActor2d/vtkTextMapper pairs with vtkTextActor.
//
// ****************************************************************************

void
VisitPlaneTool::CreateTextActors()
{
    normalTextActor = vtkTextActor::New();
    normalTextActor->ScaledTextOff();

    originTextActor = vtkTextActor::New();
    originTextActor->ScaledTextOff();

    upAxisTextActor = vtkTextActor::New();
    upAxisTextActor->ScaledTextOff();

    radiusTextActor = vtkTextActor::New();
    radiusTextActor->ScaledTextOff();
}

// ****************************************************************************
// Method: VisitPlaneTool::DeleteTextActors
//
// Purpose: 
//   Deletes the text actors and mappers.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 10:07:05 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:15:30 PDT 2002
//   Added upAxis actor and mapper.
//
//   Hank Childs, Tue May 21 13:37:44 PDT 2002
//   Removed configurable hooks to not delete actors.  This makes for less
//   of a maintenance problem.
//
//   Brad Whitlock, Fri Oct 18 15:18:31 PST 2002
//   Added radius actor and mapper.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   TextMappers no longer required.
//
// ****************************************************************************

void
VisitPlaneTool::DeleteTextActors()
{
    if(normalTextActor != NULL)
    {
        normalTextActor->Delete();
        normalTextActor = NULL;
    }

    if(originTextActor != NULL)
    {
        originTextActor->Delete();
        originTextActor = NULL;
    }

    if(upAxisTextActor != NULL)
    {
        upAxisTextActor->Delete();
        upAxisTextActor = NULL;
    }

    if(radiusTextActor != NULL)
    {
        radiusTextActor->Delete();
        radiusTextActor = NULL;
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::AddText
//
// Purpose: 
//   Adds the text actors to the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 10:09:44 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:08:46 PDT 2002
//   Added upAxis actors.
//
// ****************************************************************************

void
VisitPlaneTool::AddText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->AddActor2D(normalTextActor);
    proxy.GetForeground()->AddActor2D(originTextActor);
    proxy.GetForeground()->AddActor2D(upAxisTextActor);
#endif
}

// ****************************************************************************
// Method: VisitPlaneTool::AddRadiusText
//
// Purpose: 
//   Adds the radius text actor to the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 18 15:19:37 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitPlaneTool::AddRadiusText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->AddActor2D(radiusTextActor);
#endif
}

// ****************************************************************************
// Method: VisitPlaneTool::RemoveText
//
// Purpose: 
//   Removes the text actors from the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 10:09:44 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:08:46 PDT 2002
//   Added upAxis actors.
//
// ****************************************************************************

void
VisitPlaneTool::RemoveText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->RemoveActor2D(normalTextActor);
    proxy.GetForeground()->RemoveActor2D(originTextActor);
    proxy.GetForeground()->RemoveActor2D(upAxisTextActor);
#endif
}

// ****************************************************************************
// Method: VisitPlaneTool::RemoveRadiusText
//
// Purpose: 
//   Removes the radius text actor from the foreground canvas.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 18 15:20:11 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
VisitPlaneTool::RemoveRadiusText()
{
#ifndef NO_ANNOTATIONS
    proxy.GetForeground()->RemoveActor2D(radiusTextActor);
#endif
}

// ****************************************************************************
// Method: VisitPlaneTool::UpdateText
//
// Purpose: 
//   Updates the info that the text actors display.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 10:12:11 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:08:46 PDT 2002
//   Added upAxis actors.
//
//   Hank Childs, Tue May 21 13:37:44 PDT 2002
//   Removed configurable hooks to not update text.  This makes for less of a
//   maintenance problem.
//
//   Hank Childs, Wed May 22 09:08:28 PDT 2002
//   Explicitly set the third value to be 0. for vtkCoordinate::SetValue calls.
//
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace mapper with actor.
//
// ****************************************************************************

void
VisitPlaneTool::UpdateText()
{
    char str[100];
    sprintf(str, "Origin <%1.3g %1.3g %1.3g>", hotPoints[0].pt.x,
            hotPoints[0].pt.y, hotPoints[0].pt.z);
    originTextActor->SetInput(str);
    avtVector originScreen = ComputeWorldToDisplay(hotPoints[0].pt);
    float pt[3] = {originScreen.x, originScreen.y, 0.};
    originTextActor->GetPositionCoordinate()->SetValue(pt);

    avtVector normal(Normal());
    sprintf(str, "Normal <%1.3g %1.3g %1.3g>", normal.x, normal.y, normal.z);
    normalTextActor->SetInput(str);
    avtVector normalScreen = ComputeWorldToDisplay(hotPoints[3].pt);
    float pt2[3] = {normalScreen.x, normalScreen.y, 0.};
    normalTextActor->GetPositionCoordinate()->SetValue(pt2);

    // Create a normalized up vector.
    avtVector up(hotPoints[1].pt.x - hotPoints[0].pt.x,
                 hotPoints[1].pt.y - hotPoints[0].pt.y,
                 hotPoints[1].pt.z - hotPoints[0].pt.z);
    up.normalize();
    sprintf(str, "Up <%1.3g %1.3g %1.3g>", up.x, up.y, up.z);
    upAxisTextActor->SetInput(str);
    avtVector upAxisScreen = ComputeWorldToDisplay(hotPoints[1].pt);
    float pt3[3] = {upAxisScreen.x, upAxisScreen.y, 0.};
    upAxisTextActor->GetPositionCoordinate()->SetValue(pt3);
}

// ****************************************************************************
// Method: VisItPlaneTool::UpdateRadiusText
//
// Purpose: 
//   Updates the radius text.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 18 15:22:46 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Fri Dec 13 16:41:12 PST 2002
//   Replace mapper with actor.
//   
// ****************************************************************************

void
VisitPlaneTool::UpdateRadiusText()
{
    char str[100];
    double rad = (hotPoints[4].pt - hotPoints[0].pt).norm();
    sprintf(str, "Radius = %g", rad);
    radiusTextActor->SetInput(str);
    avtVector radiusScreen = ComputeWorldToDisplay(hotPoints[4].pt);
    float pt3[3] = {radiusScreen.x, radiusScreen.y, 0.};
    radiusTextActor->GetPositionCoordinate()->SetValue(pt3);
}

// ****************************************************************************
// Method: VisitPlaneTool::CreateOutline
//
// Purpose: 
//   Creates the outline actor and mapper.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:30:45 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::CreateOutline()
{
    outlineData = NULL;
    outlineMapper = vtkPolyDataMapper::New();
    outlineActor = vtkActor::New();
    outlineActor->GetProperty()->SetLineWidth(2.);
    outlineActor->SetMapper(outlineMapper);
}

// ****************************************************************************
// Method: VisitPlaneTool::DeleteOutline
//
// Purpose: 
//   Deletes the outline actor and mapper.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:31:14 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::DeleteOutline()
{
    if(outlineActor != NULL)
    {
        outlineActor->Delete();
        outlineActor = NULL;
    }

    if(outlineMapper != NULL)
    {
        outlineMapper->Delete();
        outlineMapper = NULL;
    }

    if(outlineData != NULL)
    {
        outlineData->Delete();
        outlineData = NULL;
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::AddOutline
//
// Purpose: 
//   Updates the outline and adds its actor to the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:31:32 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Update internal state to account for possible error conditions.
//
// ****************************************************************************

void
VisitPlaneTool::AddOutline()
{
    if(proxy.HasPlots())
    {
        addedOutline = true;
        UpdateOutline();
        proxy.GetCanvas()->AddActor(outlineActor);
        proxy.GetCanvas()->RemoveActor(planeActor);
    }
    else
    {
        addedOutline = false;
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::RemoveOutline
//
// Purpose: 
//   Removes the outline actor from the renderer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:31:52 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Depend on internal state instead of proxy so that we handle error
//   conditions better.
//
// ****************************************************************************

void
VisitPlaneTool::RemoveOutline()
{
    if (addedOutline)
    {
        proxy.GetCanvas()->RemoveActor(outlineActor);
        proxy.GetCanvas()->AddActor(planeActor);
    }
    addedOutline = false;
}

// ****************************************************************************
// Method: VisitPlaneTool::UpdateOutline
//
// Purpose: 
//   Recreates the intersection of the slice plane with the bounding box.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:32:30 PST 2001
//
// Modifications:
//   
//   Kathleen Bonnell, Mon Oct 29 15:08:16 PST 2001
//   Make ptIds of type vtkIdType to match VTK 4.0 API.
//   
//   Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//   vtkScalars has been deprecated in VTK 4.0, use vtkUnsignedCharArray for
//   colors instead.
//
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Depend on internal state instead of proxy so that we handle error
//   conditions better.
//
// ****************************************************************************

void
VisitPlaneTool::UpdateOutline()
{
    if(!addedOutline)
        return;

    if(outlineData != NULL)
        outlineData->Delete();

    // Use the max number of points an intersection will have, plus a little :)
    int numPts = 8;
    int numCells = 8;

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *lines = vtkCellArray::New();
    lines->Allocate(lines->EstimateSize(numCells, 2)); 
    vtkUnsignedCharArray *colors = vtkUnsignedCharArray::New();
    colors->SetNumberOfComponents(3);
    colors->SetNumberOfTuples(numCells);

    // Store the colors and points in the polydata.
    outlineData = vtkPolyData::New();
    outlineData->Initialize();
    outlineData->SetPoints(pts);
    outlineData->SetLines(lines);
    outlineData->GetCellData()->SetScalars(colors);
    pts->Delete(); lines->Delete(); colors->Delete();

    //
    // Figure out the outline of the bounding box with the slice plane.
    //
    int nverts = 0;
    avtVector verts[8];
    GetBoundingBoxOutline(verts, nverts);

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
    outlineMapper->SetInput(outlineData);
}

// ****************************************************************************
// Method: VisitPlaneTool::ClipAgainstPlane
//
// Purpose: 
//   Clips a polygon against a plane.
//
// Arguments:
//   v            : The array of vertices.
//   nverts       : The number of vertices in the polygon.
//   normal       : The normal of the plane. This points into the volume.
//   reference_pt : A point on the clipping plane.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 15:10:10 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::ClipAgainstPlane(avtVector *v, int &nverts,
    const avtVector &normal, const avtVector &reference_pt) const
{
    // Since we'll only be clipping quads, the number of vertices cannot ever
    // grow beyond this arbitrary limit of 10 vertices.
    avtVector new_geometry[8], tmp, *s, *p;
    int       new_npts = 0;
    int       i, s_index, p_index;
    float     t, s_dot_product, p_dot_product;

    if(nverts < 3)
        return;

    p_index = 0;
    s_index = nverts - 1;

    while(p_index < nverts)
    {
        // Create pointers into the vertex array.
        s = v + s_index;
        p = v + p_index;

        // Calculate the vector defined by a relative point on the plane 
        // and s, then calculate the dot product of the inside normal 
        // and this (s - Relative point) vector.
        tmp = reference_pt - s[0];
        s_dot_product = tmp * normal;

        // Do the same for p.
        tmp = reference_pt - p[0];
        p_dot_product = tmp * normal;

        // Determine the category of the line segment s-p.
        // p is inside the plane.
        if(p_dot_product > 0.)
        {
            // s is inside the plane. Add p to the output list.
            if(s_dot_product > 0.)
            {
                new_geometry[new_npts++] = (*p);
            }
            else
            {
                // s is outside the plane. Calculate the intersection.
                t = s_dot_product / (s_dot_product - p_dot_product);

                tmp = (p[0] - s[0]) * t + s[0];

                // Add the new point.
                new_geometry[new_npts++] = tmp;
  
                // Add the point p.
                new_geometry[new_npts++] = p[0];
            }
        }
        else if(s_dot_product > 0.)
        {
            // s is inside the plane.
            t = s_dot_product / (s_dot_product - p_dot_product);

            tmp = (p[0] - s[0]) * t + s[0];

            // Add the new point.
            new_geometry[new_npts++] = tmp;
        }
        
        // Iterate to the next point.
        s_index = p_index;
        ++p_index;
    } // end while

    // Copy the new list of points into the v array.
    for(i = 0; i < new_npts; ++i)
        v[i] = new_geometry[i];
    nverts = new_npts;
}

// ****************************************************************************
// Method: VisitPlaneTool::ClipAgainstBoundingBox
//
// Purpose: 
//   Creates the outline of the slice plane with the bounding box.
//
// Arguments:
//   v      : The array of vertices that make up the polygon.
//   nverts : The number of vertices in the polygon.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 15:05:35 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::GetBoundingBoxOutline(avtVector *v, int &nverts) const
{
    //
    // Create a big plane.
    //
    float bounds[6];
    proxy.GetBounds(bounds);
    double scale = ((bounds[1] - bounds[0]) +
                    (bounds[3] - bounds[2]) +
                    (bounds[5] - bounds[4])) * 10.;
    // UpAxis
    const avtVector &origin = hotPoints[0].pt;
    avtVector upAxis(hotPoints[1].pt - origin);
    upAxis.normalize();
    upAxis *= scale;
    // RightVector
    avtVector rightVector(hotPoints[2].pt - origin);
    rightVector.normalize();
    rightVector *= scale;
    // Create the points.
    nverts = 4;
    v[0] = origin + upAxis + rightVector;
    v[1] = origin + upAxis - rightVector;
    v[2] = origin - upAxis - rightVector;
    v[3] = origin - upAxis + rightVector;

    //
    // Clip the plane to the bounding box.
    //

    // XY-plane towards -Z.
    avtVector normal, reference_point;
    normal = avtVector(0., 0., 1.);
    reference_point = avtVector(origin.x, origin.y, bounds[5]);
    ClipAgainstPlane(v, nverts, normal, reference_point);

    // YZ-plane towards -X.
    normal = avtVector(1., 0., 0.);
    reference_point = avtVector(bounds[1], origin.y, origin.z);
    ClipAgainstPlane(v, nverts, normal, reference_point);

    // XY-plane towards +Z.
    normal = avtVector(0., 0., -1.);
    reference_point = avtVector(origin.x, origin.y, bounds[4]);
    ClipAgainstPlane(v, nverts, normal, reference_point);

    // YZ-plane towards +X.
    normal = avtVector(-1., 0., 0.);
    reference_point = avtVector(bounds[0], origin.y, origin.z);
    ClipAgainstPlane(v, nverts, normal, reference_point);

    // XZ-plane towards -Y.
    normal = avtVector(0., 1., 0.);
    reference_point = avtVector(origin.x, bounds[3], origin.z);
    ClipAgainstPlane(v, nverts, normal, reference_point);

    // XZ-plane towards +Y.
    normal = avtVector(0., -1., 0.);
    reference_point = avtVector(origin.x, bounds[2], origin.z);
    ClipAgainstPlane(v, nverts, normal, reference_point);
}

// ****************************************************************************
// Method: VisitPlaneTool::SetAwayColor
//
// Purpose: 
//   Sets the color of the normal vector.
//
// Arguments:
//   r : The red component of the color.
//   g : The green component of the color.
//   b : The blue component of the color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:28:03 PST 2001
//
// Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

void
VisitPlaneTool::SetAwayColor(float r, float g, float b)
{
    int                   i, index;
    vtkDataArray         *scalars;
    vtkUnsignedCharArray *colorData;
    unsigned char ur = (unsigned char)(r * 255);
    unsigned char ug = (unsigned char)(g * 255);
    unsigned char ub = (unsigned char)(b * 255);

    scalars = vectorData->GetCellData()->GetScalars();
    colorData = vtkUnsignedCharArray::SafeDownCast(scalars);
    for(i = 0, index = 0; i < CELLS_PER_ARROW; ++i, index += 3)
    {
        unsigned char *rgb = colorData->GetPointer(index);
        rgb[0] = ur;
        rgb[1] = ug;
        rgb[2] = ub;
    }
    vectorData->Modified();
}

// ****************************************************************************
// Method: VisitPlaneTool::UpdateNormalVectorColor
//
// Purpose: 
//   Updates the color of the normal vector depending on the direction of
//   the normal and the current camera vector.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:28:56 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::UpdateNormalVectorColor()
{
    bool away = FacingAway();

    if(away != normalAway)
    {
        normalAway = away;
        float color[3] = {1., 0., 0.};
        if(!normalAway)
            proxy.GetForegroundColor(color);

        SetAwayColor(color[0], color[1], color[2]);
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::FacingAway
//
// Purpose: 
//   Determines whether the plane normal is facing away from the camera.
//
// Returns:    True if the plane normal faces away, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 11 17:29:31 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisitPlaneTool::FacingAway() const
{
    vtkCamera *camera = proxy.GetCanvas()->GetActiveCamera();
    const double *pos = camera->GetPosition();
    const double *focus = camera->GetFocalPoint();
    avtVector camvec(pos[0]-focus[0],pos[1]-focus[1],pos[2]-focus[2]);
    camvec.normalize();
    double dot = camvec * Normal();
    return (dot < 0.);
}

// ****************************************************************************
// Method: VisitPlaneTool::CallCallback
//
// Purpose: 
//   Lets the outside world know that the tool has a new slice plane.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 10 09:35:13 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::CallCallback()
{
    // Origin
    avtVector origin(hotPoints[0].pt);

    // Normal
    avtVector normal(Normal());

    // UpAxis
    avtVector upAxis(hotPoints[1].pt - origin);
    upAxis.normalize();
    
    // Radius
    double radius = (hotPoints[4].pt - origin).norm();
    Interface.SetOrigin(origin.x, origin.y, origin.z);
    Interface.SetNormal(normal.x, normal.y, normal.z);
    Interface.SetUpAxis(upAxis.x, upAxis.y, upAxis.z);
    Interface.SetRadius(radius);
    Interface.ExecuteCallback();
}

// ****************************************************************************
// Method: VisitPlaneTool::InitialActorSetup
//
// Purpose: 
//   Makes the text and outline actors active and starts bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 14:37:20 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:19:43 PDT 2002
//   Made it so text is not removed.
//   
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Update internal state to account for possible error conditions.
//
// ****************************************************************************

void
VisitPlaneTool::InitialActorSetup()
{
    // Add the text and outline actors.
    AddOutline();

    // Enter bounding box mode if there are plots.
    if(proxy.HasPlots())
    {
        addedBbox = true;
        proxy.StartBoundingBox();
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::FinalActorSetup
//
// Purpose: 
//   Removes certain actors from the renderer and ends bounding box mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 14:38:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 12 11:19:43 PDT 2002
//   Made it so text is not removed.
//
//   Hank Childs, Mon Apr 15 17:33:48 PDT 2002
//   Depend on internal state instead of proxy so that we handle error
//   conditions better.
//
//   Kathleen Bonnell, Wed Dec  3 17:03:34 PST 2003 
//   If transparencies exist, have the plots recalculate render order, so
//   that this tool is rendered before the transparent actors. 
//
// ****************************************************************************

void
VisitPlaneTool::FinalActorSetup()
{
    // Remove the text and outline actors.
    RemoveOutline();

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
//  Method:  VisitPlaneTool::Translate
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
//  Programmer:  Jeremy Meredith, Brad Whitlock
//  Creation:    October  9, 2001
//
// ****************************************************************************

void
VisitPlaneTool::Translate(CB_ENUM e, int, int, int x, int y)
{
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

        avtVector motion = newPoint - oldPoint;
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
    }
}

// ****************************************************************************
// Method: VisitPlaneTool::TranslateNormal
//
// Purpose: 
//    This is the handler method that is called when the translate along
//    normal hotpoint is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    y : The y location of the mouse in pixels.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 10 16:44:06 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisitPlaneTool::TranslateNormal(CB_ENUM e, int, int, int, int y)
{
    if(e == CB_START)
    {
        // Get the size of the bounding box.
        float bounds[6];
        proxy.GetBounds(bounds);

        // Figure out a good delta translation.
        int *size = proxy.GetCanvas()->GetSize();
        double dX = bounds[1] - bounds[0];
        double dY = bounds[3] - bounds[2];
        double dZ = bounds[5] - bounds[4];
        double diagonal = sqrt(dX*dX + dY*dY + dZ*dZ);

        // This is the amount to translate by
        translationDistance = diagonal * 0.5 / double(size[1]);

        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        // Figure out the amount to translate in x,y,z.
        avtVector motion = hotPoints[3].pt - hotPoints[0].pt;
        motion.normalize();
        int dY = y - lastY;
        motion *= (dY * translationDistance);

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
    }
}

// ****************************************************************************
//  Method:  VisitPlaneTool::RotateX
//
//  Purpose:
//    This is the handler method that is called when the rotate about X
//    hotpoint is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Jeremy Meredith, Brad Whitlock
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
VisitPlaneTool::RotateX(CB_ENUM e, int, int, int x, int y)
{
    if(e == CB_START)
    {
        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        // Get the previous and current mouse coords in units
        avtVector prev = DisplayToUnit(avtVector(lastX,lastY));
        avtVector curr = DisplayToUnit(avtVector(x,y));

        // Get the trackball rotation
        avtMatrix R(GetTrackballMatrix(prev, curr, 0));

        // Rotate the actors and hotpoints.
        RMtx = R * RMtx;
        DoTransformations();

        // Update the color of the normal vector.
        UpdateNormalVectorColor();

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
    }
}


// ****************************************************************************
//  Method:  VisitPlaneTool::RotateY
//
//  Purpose:
//    This is the handler method that is called when the rotate about Y
//    hotpoint is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Jeremy Meredith, Brad Whitlock
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
VisitPlaneTool::RotateY(CB_ENUM e, int, int, int x, int y)
{
    if(e == CB_START)
    {
        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        // Get the previous and current mouse coords in units
        avtVector prev = DisplayToUnit(avtVector(lastX,lastY));
        avtVector curr = DisplayToUnit(avtVector(x,y));

        // Get the trackball rotation
        avtMatrix R(GetTrackballMatrix(prev, curr, 1));

        // Rotate the actors and hotpoints.
        RMtx = R * RMtx;
        DoTransformations();

        // Update the color of the normal vector.
        UpdateNormalVectorColor();

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
    }
}

// ****************************************************************************
//  Method:  VisitPlaneTool::FreeRotate
//
//  Purpose:
//    This is the handler method that is called when the free-rotate hotpoint
//    is active.
//
//  Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
//  Programmer:  Jeremy Meredith, Brad Whitlock
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
VisitPlaneTool::FreeRotate(CB_ENUM e, int, int, int x , int y)
{
    if(e == CB_START)
    {
        // Make the right actors active.
        InitialActorSetup();
    }
    else if(e == CB_MIDDLE)
    {
        // Get the previous and current mouse coords in units
        avtVector prev = DisplayToUnit(avtVector(lastX,lastY));
        avtVector curr = DisplayToUnit(avtVector(x,y));

        // Get the trackball rotation
        avtMatrix R(GetTrackballMatrix(prev, curr));

        // Rotate the actors and hotpoints.
        RMtx = R * RMtx;
        DoTransformations();

        // Update the color of the normal vector.
        UpdateNormalVectorColor();

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
    }
}

// ****************************************************************************
//  Method:  VisitPlaneTool::Resize
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
//  Programmer:  Jeremy Meredith, Brad Whitlock
//  Creation:    October  9, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Oct 21 11:55:08 PDT 2002
//    I added code to added and remove the radius text.
//
// ****************************************************************************

void
VisitPlaneTool::Resize(CB_ENUM e, int, int, int x, int y)
{
    HotPoint &origin = hotPoints[0];
    HotPoint &resize = hotPoints[4];

    double dX, dY;
    avtVector originScreen, resizeScreen;

    if(e == CB_START)
    {
        originScreen = ComputeWorldToDisplay(origin.pt);
        resizeScreen = ComputeWorldToDisplay(resize.pt);

        dX = originScreen.x - resizeScreen.x;
        dY = originScreen.y - resizeScreen.y;
        originalDistance = sqrt(dX * dX + dY * dY);

        // Start bounding box mode.
        if(proxy.HasPlots())
        {
            addedBbox = true;
            proxy.StartBoundingBox();
        }

        RemoveText();
        AddRadiusText();
    }
    else if(e == CB_MIDDLE)
    {
        originScreen = ComputeWorldToDisplay(origin.pt);

        dX = originScreen.x - double(x);
        dY = originScreen.y - double(y);
        double dist = sqrt(dX * dX + dY * dY);
        double scale = dist / originalDistance;

        // Do the hotpoint and actor transformations
        avtMatrix S(avtMatrix::CreateScale(scale));
        SMtx = S * SMtx;
        DoTransformations();

        // Save the distance for next time through.
        originalDistance = dist;

        UpdateRadiusText();

        // Render the window
        proxy.Render();
    }
    else
    {
        // Call the tool's callback.
        CallCallback();

        RemoveRadiusText();
        AddText();

        // End bounding box mode.
        if(addedBbox)
            proxy.EndBoundingBox();
        addedBbox = false;
    }
}


// ****************************************************************************
//  Method:  VisitPlaneTool::GetTrackballMatrix
//
//  Purpose:
//    Given the current and previous mouse coordinates, this method computes
//    a suitable trackball matrix.
//
//  Arguments:
//    prev : The previous mouse location.
//    curr : The current mouse location.
//    axis : The axis to which we want to restrict the rotation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Oct 16 16:34:22 PST 2001
//    Modified the matrix code.
//
// ****************************************************************************

avtMatrix
VisitPlaneTool::GetTrackballMatrix(const avtVector &prev,
                                   const avtVector &curr,
                                   int axis)
{
    // Constrain to an axis if requested
    if (axis != -1)
    {
        avtVector constrain;
        switch (axis)
        {
          case 0: constrain = origHotPoints[2].pt - origHotPoints[0].pt; break;
          case 1: constrain = origHotPoints[1].pt - origHotPoints[0].pt; break;
          case 2: constrain = origHotPoints[3].pt - origHotPoints[0].pt; break;
        }
        // transform from model to world space
        constrain = (RMtx ^ constrain).normalized();
        trackball.SetConstrainAxis(constrain);
    }
    else
        trackball.ClearConstrainAxis();

    // Center rotations around the hot point origin, not the screen center
    avtVector orig = DisplayToUnit(ComputeWorldToDisplay(hotPoints[0].pt));
    trackball.SetCenter(orig);

    // Set the camera
    vtkRenderer *ren = proxy.GetCanvas();
    vtkMatrix4x4 *vtm = ren->GetActiveCamera()->GetViewTransformMatrix();
    double dvtm[16];
    vtkMatrix4x4::DeepCopy(dvtm, vtm);
    avtMatrix C(dvtm);
    trackball.SetCamera(C);

    // Do the rotation
    trackball.PerformRotation(prev, curr);

    return trackball.GetMatrix();
}


// ****************************************************************************
//  Method:  VisitPlaneTool::DoTransformations
//
//  Purpose:
//    Applies the current transformation to the hotpoints.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
//  Modifications:
//    Brad Whitlock, Tue Oct 16 16:17:32 PST 2001
//    Modified how the actors' matrices are set.
//
// ****************************************************************************
void
VisitPlaneTool::DoTransformations()
{
    avtMatrix M = TMtx * RMtx * SMtx;

    for (int i=0; i<hotPoints.size(); i++)
        hotPoints[i].pt = M * origHotPoints[i].pt;


    vtkMatrix4x4 *tmp = vtkMatrix4x4::New();
    tmp->DeepCopy(M.GetElements());
    vectorActor->SetUserMatrix(tmp);
    planeActor->SetUserMatrix(tmp);
    tmp->Delete();
}


//
// Static callback functions.
//

void
VisitPlaneTool::TranslateCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPlaneTool *pt = (VisitPlaneTool *)it;
    pt->Translate(e, ctrl, shift, x, y);
}

void
VisitPlaneTool::TranslateNormalCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPlaneTool *pt = (VisitPlaneTool *)it;
    pt->TranslateNormal(e, ctrl, shift, x, y);
}

void
VisitPlaneTool::RotateXCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPlaneTool *pt = (VisitPlaneTool *)it;
    pt->RotateX(e, ctrl, shift, x, y);
}

void
VisitPlaneTool::RotateYCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPlaneTool *pt = (VisitPlaneTool *)it;
    pt->RotateY(e, ctrl, shift, x, y);
}

void
VisitPlaneTool::FreeRotateCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPlaneTool *pt = (VisitPlaneTool *)it;
    pt->FreeRotate(e, ctrl, shift, x, y);
}

void
VisitPlaneTool::ResizeCallback(VisitInteractiveTool *it, CB_ENUM e,
    int ctrl, int shift, int x, int y)
{
    VisitPlaneTool *pt = (VisitPlaneTool *)it;
    pt->Resize(e, ctrl, shift, x, y);
}

// ****************************************************************************
// Method: VisitPlaneTool::TurnLightingOn
//
// Purpose: 
//   Adjust actor's lighting coefficients for Lighting ON condition. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 5, 2002 
//
// Modifications:
//
// ****************************************************************************

void
VisitPlaneTool::TurnLightingOn()
{
    vectorActor->GetProperty()->SetAmbient(0.);
    vectorActor->GetProperty()->SetDiffuse(1.);
}

// ****************************************************************************
// Method: VisitPlaneTool::TurnLightingOff
//
// Purpose: 
//   Adjust actor's lighting coefficients for Lighting OFF condition. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 5, 2002 
//
// Modifications:
//
// ****************************************************************************

void
VisitPlaneTool::TurnLightingOff()
{
    vectorActor->GetProperty()->SetAmbient(1.);
    vectorActor->GetProperty()->SetDiffuse(0.);
}


// ****************************************************************************
//  Method:  VisitPlaneTool::ReAddToWindow
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
VisitPlaneTool::ReAddToWindow()
{
    if(IsEnabled())
    {
        proxy.GetCanvas()->RemoveActor(planeActor);
        proxy.GetCanvas()->AddActor(planeActor);
    }
}
