#include <math.h>

#include <VisitExtentsTool.h>
#include <ExtentsAttributes.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkObject.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <avtVector.h>


// ****************************************************************************
// Method: VisitExtentsTool::VisitExtentsTool
//
// Purpose: This is the constructor for the extents tool.
//
// Arguments:
//   p : A reference to the tool proxy.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Added running time ordinal for tagging slider changes.
//
// ****************************************************************************

VisitExtentsTool::VisitExtentsTool(VisWindowToolProxy &p) : VisitInteractiveTool(p),
    Interface(p)
{
    stringVector initScalarNames;
    doubleVector initScalarMinima;
    doubleVector initScalarMaxima;
    doubleVector initMinima;
    doubleVector initMaxima;
    intVector    initMinTimeOrdinals;
    intVector    initMaxTimeOrdinals;

    curTimeOrdinal = 1;
    activeAxisIndex = 0;

    for (int extentNum = 0; extentNum < EA_DEFAULT_NUMBER_OF_EXTENTS; extentNum++)
    {
        initScalarNames.push_back("default");
        initScalarMinima.push_back(-1e+37); initScalarMaxima.push_back(1e+37);
        initMinima.push_back(0.0); initMaxima.push_back(1.0);
        initMinTimeOrdinals.push_back(0); initMaxTimeOrdinals.push_back(0);
    }

    Interface.SetScalarNames (initScalarNames);

    Interface.SetScalarMinima(initScalarMinima);
    Interface.SetScalarMaxima(initScalarMaxima);

    Interface.SetMinima(initMinima);
    Interface.SetMaxima(initMaxima);
    
    Interface.SetMinTimeOrdinals(initMinTimeOrdinals);
    Interface.SetMaxTimeOrdinals(initMaxTimeOrdinals);

    Interface.SetLeftSliderX(EA_DEFAULT_LEFT_SLIDER_X);
    Interface.SetRightSliderX(EA_DEFAULT_RIGHT_SLIDER_X);
    Interface.SetSlidersBottomY(EA_DEFAULT_SLIDERS_BOTTOM_Y);
    Interface.SetSlidersTopY(EA_DEFAULT_SLIDERS_TOP_Y);

    sliderActor = NULL; sliderMapper = NULL; sliderData = NULL;

/*
    SetForegroundColor(VET_ARROW_RED, VET_ARROW_GREEN, VET_ARROW_BLUE);
*/

    InitializeHotPoints();

    CreateSliderActor();
    CreateTextActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::~VisitExtentsTool
//
// Purpose: This is the destructor for the extents tool class.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//   
// ****************************************************************************

VisitExtentsTool::~VisitExtentsTool()
{
    if (sliderActor != NULL)
    {
        sliderActor->Delete();
        sliderActor = NULL;
    }

    if (sliderMapper != NULL)
    {
        sliderMapper->Delete();
        sliderMapper = NULL;
    }

    if (sliderData != NULL)
    {
        sliderData->Delete();
        sliderData = NULL;
    }

    // Delete the text mappers and actors
    DeleteTextActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::InitializeHotPoints
//
// Purpose: Initializes hotpoints of the Extents tool according to current
//          values of the associated ExtentsAttributes.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Thu Aug 31 17:56:00 PDT 2006
//    Changed to new interface for GetBounds in VTK.
//
// ****************************************************************************

void VisitExtentsTool::InitializeHotPoints()
{
    doubleVector extentMinima = Interface.GetMinima();
    doubleVector extentMaxima = Interface.GetMaxima();

    double leftSliderX    = Interface.GetLeftSliderX();
    double rightSliderX   = Interface.GetRightSliderX();
    double slidersBottomY = Interface.GetSlidersBottomY();
    double slidersTopY    = Interface.GetSlidersTopY();

    int extentCount = extentMinima.size();
    int extentNum;

    if (extentCount < VET_MINIMUM_NUMBER_OF_EXTENTS)
    {
        extentCount = VET_MINIMUM_NUMBER_OF_EXTENTS;

        extentMinima.clear(); extentMaxima.clear();

        for (extentNum = 0; extentNum < extentCount; extentNum++)
        {
            extentMinima.push_back(0.0); extentMaxima.push_back(1.0);
        }
    }

    double winBounds[6];
    proxy.GetBounds(winBounds);

    double winXMin = winBounds[0];
    double winYMin = winBounds[2];
    double winWidth  = winBounds[1] - winXMin;
//
//  KLUDGE OF THE CENTURY!  UTTERLY SCANDALOUS!  An empirical curve fit.
//  PLEASE FIX ME!
//
    int pixelWidth, pixelHeight;
    proxy.GetSize(pixelWidth, pixelHeight);

    double hToWRatio = (double)pixelHeight / (double)pixelWidth;
    double fudgeTerm = 0.07;

    if (hToWRatio < 1.05477)
        fudgeTerm -= (1.05477 - hToWRatio) * 0.07;
    else
        fudgeTerm += (1.0 - 1.05477/hToWRatio) * 0.12;

    double winHeight = winYMin + (hToWRatio+fudgeTerm)*winWidth;

    double worldBotSliderY = slidersBottomY*winHeight + winYMin;
    double worldTopSliderY = slidersTopY*winHeight + winYMin;
    normToWorldYScale = worldTopSliderY - worldBotSliderY;

    hotPointRadius = VET_HOTPOINT_RADIUS_FRACTION * winWidth;

    minSlidableX = leftSliderX *winWidth + winXMin;
    maxSlidableX = rightSliderX*winWidth + winXMin;
    minSlidableY = worldBotSliderY - hotPointRadius;
    maxSlidableY = worldTopSliderY + hotPointRadius;

    sliderXStride = (maxSlidableX - minSlidableX) / (double)(extentCount-1);

    HotPoint hotPoint;

    hotPoint.pt.z   = 0.0;
    hotPoint.radius = hotPointRadius;
    hotPoint.tool   = this;

    hotPoints.clear();

    for (extentNum = 0; extentNum < extentCount; extentNum++)
    {
        hotPoint.pt.x = (double)extentNum*sliderXStride + minSlidableX;
        hotPoint.pt.y = minSlidableY + extentMinima[extentNum]*normToWorldYScale;
        hotPoint.callback = SliderMinimumCallback;
        hotPoints.push_back(hotPoint);

        hotPoint.pt.y =
            maxSlidableY - (1.0-extentMaxima[extentNum])*normToWorldYScale;
        hotPoint.callback = SliderMaximumCallback;
        hotPoints.push_back(hotPoint);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::Enable
//
// Purpose: This method enables the tool.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::Enable()
{
    bool toolIsEnabled = IsEnabled();

    VisitInteractiveTool::Enable();

    // Add the actors to the canvas.
    if (!toolIsEnabled)
    {
        UpdateTool();
        proxy.GetCanvas()->AddActor(sliderActor);
        AddText();
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::Disable
//
// Purpose: This method disables the tool.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::Disable()
{
    bool toolIsEnabled = IsEnabled();

    VisitInteractiveTool::Disable();

    // Remove the actors from the canvas if the tool was enabled.
    if (toolIsEnabled)
    {
        proxy.GetCanvas()->RemoveActor(sliderActor);
        RemoveText();
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::IsAvailable
//
// Purpose: Returns whether or not the tool is available for use.
//
// Returns: Whether or not the tool is available for use.
//
// Programmer: Mark Blair
// Creation:   Tue Nov  1 17:17:00 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool VisitExtentsTool::IsAvailable() const
{
    return (proxy.GetPlotListIndex("ParallelAxis") >= 0);
}


// ****************************************************************************
// Method: VisitExtentsTool::SetForegroundColor
//
// Purpose: This method sets the tool's foreground color.
//
// Arguments:
//   r : The red color component.
//   g : The green color component.
//   b : The blue color component.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::SetForegroundColor(float r, float g, float b)
{
    unsigned char arrowRed   = (unsigned char)(r * 255.0);
    unsigned char arrowGreen = (unsigned char)(g * 255.0);
    unsigned char arrowBlue  = (unsigned char)(b * 255.0);

    vtkDataArray *scalars = sliderData->GetCellData()->GetScalars();
    vtkUnsignedCharArray *arrowColors = vtkUnsignedCharArray::SafeDownCast(scalars);
    int numArrows = hotPoints.size();
    unsigned char *rgb;

    for (int arrowNum = 0; arrowNum < numArrows; arrowNum++)
    {
        rgb = arrowColors->GetPointer(arrowNum*3);
        rgb[0] = arrowRed; rgb[1] = arrowGreen; rgb[2] = arrowBlue;
    }

    sliderData->Modified();
}


// ****************************************************************************
// Method: VisItExtentsTool::UpdateTool
//
// Purpose: Reinitializes the tool according to its attributes.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::UpdateTool()
{
    if (sliderActor != NULL)
        proxy.GetCanvas()->RemoveActor(sliderActor);

    InitializeHotPoints();

    CreateSliderActor();
    CreateTextActors();

    proxy.GetCanvas()->AddActor(sliderActor);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateSliderActor
//
// Purpose: Creates the slider actor.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateSliderActor()
{
    if (sliderActor != NULL)
    {
        sliderActor->Delete(); sliderMapper->Delete(); sliderData->Delete();
    }

    sliderActor = vtkActor::New();
    sliderMapper = vtkPolyDataMapper::New();
    sliderData = vtkPolyData::New();

    arrowPoints = vtkPoints::New();
    vtkCellArray *arrowTriangles = vtkCellArray::New();
    vtkUnsignedCharArray *arrowColors = vtkUnsignedCharArray::New();

    int numArrows = hotPoints.size();
    int numExtents = numArrows / 2;
    int numPoints = numArrows * 3;

    arrowPoints->SetNumberOfPoints(numPoints);
    arrowTriangles->Allocate(arrowTriangles->EstimateSize(numArrows, 3));
    arrowColors->SetNumberOfComponents(3);
    arrowColors->SetNumberOfTuples(numArrows);

    //
    // Store the colors and points in the polydata.
    //
    sliderData->Initialize();
    sliderData->SetPoints(arrowPoints);
    sliderData->SetPolys(arrowTriangles);
    sliderData->GetCellData()->SetScalars(arrowColors);

    arrowPoints->Delete(); arrowTriangles->Delete(); arrowColors->Delete();

    //
    // Create the arrow triangle points.
    //
    int minHotPointID, minHPVertexID;
    double minHotPointX, minHotPointY, maxHotPointY;
    float arrowLeftX, arrowRightX;

    vtkIdType pointIDs[3];
    float coords[3];
    coords[2] = 0.0;

    for (int extentNum = 0; extentNum < numExtents; extentNum++)
    {
        minHotPointID = extentNum * 2; minHPVertexID = minHotPointID * 3;

        minHotPointX = hotPoints[minHotPointID  ].pt.x;
        minHotPointY = hotPoints[minHotPointID  ].pt.y;
        maxHotPointY = hotPoints[minHotPointID+1].pt.y;

        arrowLeftX  = (float)(minHotPointX - hotPointRadius);
        arrowRightX = (float)(minHotPointX + hotPointRadius);

        coords[0] = arrowLeftX;
        coords[1] = (float)minHotPointY;
        arrowPoints->SetPoint(minHPVertexID, coords);

        coords[0] = arrowRightX;
        arrowPoints->SetPoint(minHPVertexID+1, coords);

        coords[0] = (float)minHotPointX;
        coords[1] = (float)(minHotPointY + hotPointRadius);
        arrowPoints->SetPoint(minHPVertexID+2, coords);

        coords[1] = (float)(maxHotPointY - hotPointRadius);
        arrowPoints->SetPoint(minHPVertexID+3, coords);

        coords[0] = arrowLeftX;
        coords[1] = (float)maxHotPointY;
        arrowPoints->SetPoint(minHPVertexID+4, coords);

        coords[0] = arrowRightX;
        arrowPoints->SetPoint(minHPVertexID+5, coords);
    }

    for (vtkIdType pointID = 0; pointID < numPoints; pointID += 3)
    {
        pointIDs[0] = pointID;
        pointIDs[1] = pointID + 1;
        pointIDs[2] = pointID + 2;

        arrowTriangles->InsertNextCell(3, pointIDs);
    }

    //
    // Store the arrow polygon colors into the colors array directly.
    //
    float fgColor[3] = { VET_ARROW_RED, VET_ARROW_GREEN, VET_ARROW_BLUE };

/* Use of global foreground color is not a good idea for parallel coordinate plot.
    proxy.GetForegroundColor(fgColor);
*/

    unsigned char *rgb;
    unsigned char arrowRed   = (unsigned char)(fgColor[0] * 255.0);
    unsigned char arrowGreen = (unsigned char)(fgColor[1] * 255.0);
    unsigned char arrowBlue  = (unsigned char)(fgColor[2] * 255.0);

    for (int arrowNum = 0; arrowNum < numArrows; arrowNum++)
    {
        rgb = arrowColors->GetPointer(arrowNum*3);
        rgb[0] = arrowRed; rgb[1] = arrowGreen; rgb[2] = arrowBlue;
    }

    sliderMapper->SetInput(sliderData);
    sliderActor->SetMapper(sliderMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateTextActors
//
// Purpose: Creates any text actors and corresponding mappers needed by the tool.
//
// Note: There are currently no text actors (Oct 31 2005).  This method is just
//       a reminder in the code in case any text might be drawn by the tool
//       itself at some time in the future.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateTextActors()
{
    // Maybe text actors in the furure.
    return;
}


// ****************************************************************************
// Method: VisitPlaneTool::DeleteTextActors
//
// Purpose: Deletes any text actors and mappers created by the tool.
//
// Note: There are currently no text actors (Oct 31 2005).  This method is just
//       a reminder in the code in case any text might be drawn by the tool
//       itself at some time in the future.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteTextActors()
{
    // Maybe text actors in the furure.
    return;
}


// ****************************************************************************
// Method: VisitExtentsTool::AddText
//
// Purpose: Adds to the foreground canvas any text actors created by the tool.
//
// Note: There are currently no text actors (Oct 31 2005).  This method is just
//       a reminder in the code in case any text might be drawn by the tool
//       itself at some time in the future.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::AddText()
{
    // Maybe text actors in the furure.
    return;
}


// ****************************************************************************
// Method: VisitPlaneTool::RemoveText
//
// Purpose: Removed from the foreground canvas any text actors created by
//          the tool.
//
// Note: There are currently no text actors (Oct 31 2005).  This method is just
//       a reminder in the code in case any text might be drawn by the tool
//       itself at some time in the future.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::RemoveText()
{
    // Maybe text actors in the furure.
    return;
}


// ****************************************************************************
// Method: VisitPlaneTool::UpdateText
//
// Purpose: Updates the info to be displayed by any text actors created by
//          the tool.
//
// Note: There are currently no text actors (Oct 31 2005).  This method is just
//       a reminder in the code in case any text might be drawn by the tool
//       itself at some time in the future.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::UpdateText()
{
    // Maybe text actors in the future.
    return;
}


// ****************************************************************************
// Method:  VisitExtentsTool::SliderMinimum
//
// Purpose: This is the handler method that is called when a hotpoint for one
//          of the slider minima is in motion.
// Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Changed interface to update minima on an individual basis.
//
// ****************************************************************************

void VisitExtentsTool::SliderMinimum(CB_ENUM e, int ctrl, int shift, int x, int y)
{
    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        if (cursorPos.x < minSlidableX)
            cursorPos.x = minSlidableX;
        else if (cursorPos.x > maxSlidableX)
            cursorPos.x = maxSlidableX;

        activeAxisIndex =
            (int)((cursorPos.x-minSlidableX+sliderXStride*0.5) / sliderXStride);
        activeHotPointID = activeAxisIndex * 2;
        activeHPMaxY = hotPoints[activeHotPointID+1].pt.y - hotPointRadius*2.0;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        if (cursorPos.y < minSlidableY)
            cursorPos.y = minSlidableY;
        else if (cursorPos.y > activeHPMaxY)
            cursorPos.y = activeHPMaxY;

        hotPoints[activeHotPointID].pt.x = hotPoints[activeHotPointID+1].pt.x;
        hotPoints[activeHotPointID].pt.y = cursorPos.y;

        MoveSliderMinimumArrow(activeHotPointID);

        // Update any text created by the tool.
        UpdateText();

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        // Call the tool's callback for a changed minimum.
        CallMinCallback();

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::SliderMaximum
//
// Purpose: This is the handler method that is called when a hotpoint for one
//          of the slider maxima is in motion.
// Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Changed interface to update maxima on an individual basis.
//
// ****************************************************************************

void VisitExtentsTool::SliderMaximum(CB_ENUM e, int ctrl, int shift, int x, int y)
{
    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        if (cursorPos.x < minSlidableX)
            cursorPos.x = minSlidableX;
        else if (cursorPos.x > maxSlidableX)
            cursorPos.x = maxSlidableX;

        activeAxisIndex =
            (int)((cursorPos.x-minSlidableX+sliderXStride*0.5) / sliderXStride);
        activeHotPointID = activeAxisIndex*2 + 1;
        activeHPMinY = hotPoints[activeHotPointID-1].pt.y + hotPointRadius*2.0;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        if (cursorPos.y > maxSlidableY)
            cursorPos.y = maxSlidableY;
        else if (cursorPos.y < activeHPMinY)
            cursorPos.y = activeHPMinY;

        hotPoints[activeHotPointID].pt.x = hotPoints[activeHotPointID-1].pt.x;
        hotPoints[activeHotPointID].pt.y = cursorPos.y;

        MoveSliderMaximumArrow(activeHotPointID);

        // Update any text created by the tool.
        UpdateText();

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        // Call the tool's callback for a changed maximum.
        CallMaxCallback();

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CallMinCallback
//
// Purpose: Lets the outside world know that the tool has a new extent minimum.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//   
//    Mark Blair, Thu Aug 31 17:56:00 PDT 2006
//    Changed to new interface for GetBounds in VTK.
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Changed interface to update minima on an individual basis.  Also updates
//    associated time ordinal.
//
// ****************************************************************************

void VisitExtentsTool::CallMinCallback()
{
    doubleVector newMinima          = Interface.GetMinima();
    intVector    newMinTimeOrdinals = Interface.GetMinTimeOrdinals();
    
    if ((activeAxisIndex < 0) || (activeAxisIndex >= newMinima.size())) return;
    
    newMinima[activeAxisIndex] =
        (hotPoints[activeAxisIndex*2].pt.y - minSlidableY) / normToWorldYScale;
    newMinTimeOrdinals[activeAxisIndex] = curTimeOrdinal++;
    
    Interface.SetMinima(newMinima);
    Interface.SetMinTimeOrdinals(newMinTimeOrdinals);

    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method: VisitExtentsTool::CallMaxCallback
//
// Purpose: Lets the outside world know that the tool has a new extent maximum.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//   
//    Mark Blair, Thu Aug 31 17:56:00 PDT 2006
//    Changed to new interface for GetBounds in VTK.
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Changed interface to update maxima on an individual basis.  Also updates
//    associated time ordinal.
//
// ****************************************************************************

void VisitExtentsTool::CallMaxCallback()
{
    doubleVector newMaxima          = Interface.GetMaxima();
    intVector    newMaxTimeOrdinals = Interface.GetMaxTimeOrdinals();
    
    if ((activeAxisIndex < 0) || (activeAxisIndex >= newMaxima.size())) return;
    
    newMaxima[activeAxisIndex] = 1.0 -
        (maxSlidableY - hotPoints[activeAxisIndex*2+1].pt.y) / normToWorldYScale;
    newMaxTimeOrdinals[activeAxisIndex] = curTimeOrdinal++;

    Interface.SetMaxima(newMaxima);
    Interface.SetMaxTimeOrdinals(newMaxTimeOrdinals);

    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method:  VisitExtentsTool::MoveSliderMinimumArrow
//
// Purpose: Changes Y coordinates of a slider minimum arrow's vertices to
//          reflect the arrow's movement up or down.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Thu Aug 31 17:56:00 PDT 2006
//    Changed to new interface for GetPoint in VTK.
//
// ****************************************************************************

void VisitExtentsTool::MoveSliderMinimumArrow(int hotPointID)
{
    double hotPointY = hotPoints[hotPointID].pt.y;
    double vertexXYZ[3];

    for (int vertexID = hotPointID*3; vertexID < hotPointID*3+3; vertexID++)
    {
        arrowPoints->GetPoint(vertexID, vertexXYZ);

        if (vertexID != hotPointID*3 + 2)
            vertexXYZ[1] = hotPointY;
        else
            vertexXYZ[1] = hotPointY + hotPointRadius;

        arrowPoints->SetPoint(vertexID, vertexXYZ);
    }

    sliderData->Modified();
}


// ****************************************************************************
// Method:  VisitExtentsTool::MoveSliderMaximumArrow
//
// Purpose: Changes Y coordinates of a slider maximum arrow's vertices to
//          reflect the arrow's movement up or down.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Thu Aug 31 17:56:00 PDT 2006
//    Changed to new interface for GetPoint in VTK.
//
// ****************************************************************************

void VisitExtentsTool::MoveSliderMaximumArrow(int hotPointID)
{
    double hotPointY = hotPoints[hotPointID].pt.y;
    double vertexXYZ[3];

    for (int vertexID = hotPointID*3; vertexID < hotPointID*3+3; vertexID++)
    {
        arrowPoints->GetPoint(vertexID, vertexXYZ);

        if (vertexID != hotPointID*3)
            vertexXYZ[1] = hotPointY;
        else
            vertexXYZ[1] = hotPointY - hotPointRadius;

        arrowPoints->SetPoint(vertexID, vertexXYZ);
    }

    sliderData->Modified();
}


// ****************************************************************************
// Method:  VisitExtentsTool::ReAddToWindow
//
// Purpose:
//    Allows the tool to re-add any actors affected by anti-aliasing to remove
//    and re-add themselves back to the renderer, so that they will be rendered
//    after plots.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::ReAddToWindow()
{
    if (IsEnabled())
    {
        proxy.GetCanvas()->RemoveActor(sliderActor);
        proxy.GetCanvas()->AddActor(sliderActor);
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
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
//  Modifications:
//
// ****************************************************************************

void VisitExtentsTool::FullFrameOn(const double, const int)
{
    if (IsEnabled()) UpdateTool();
}


// ****************************************************************************
// Method:  VisitLineTool::FullFrameOff
//
// Purpose: Updates the tool.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::FullFrameOff()
{
    if (IsEnabled()) UpdateTool();
}


// ****************************************************************************
// Method:  (Static callback functions)
//
// Purpose: Static versions of callbacks for hotpoints associated with the
//          tool's minimum and maximum arrows.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::SliderMinimumCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->SliderMinimum(e, ctrl, shift, x, y);
}


void VisitExtentsTool::SliderMaximumCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->SliderMaximum(e, ctrl, shift, x, y);
}
