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

#include <VisitExtentsTool.h>
#include <ExtentsAttributes.h>
#include <PlotInfoAttributes.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
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
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Added support for selective axis labeling in associated plot.
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
    stringVector initAxisGroupNames;
    intVector    initAxisLabelStates;
    doubleVector initAxisXIntervals;

    curTimeOrdinal = 1;
    activeAxisIndex = 0;

    for (int extentNum = 0; extentNum < EA_DEFAULT_NUMBER_OF_EXTENTS; extentNum++)
    {
        initScalarNames.push_back("default");
        initScalarMinima.push_back(-1e+37); initScalarMaxima.push_back(1e+37);
        initMinima.push_back(0.0); initMaxima.push_back(1.0);
        initMinTimeOrdinals.push_back(0); initMaxTimeOrdinals.push_back(0);
        initAxisGroupNames.push_back(std::string("(not_in_a_group)"));
        initAxisLabelStates.push_back(EA_DRAW_ALL_LABELS | EA_LABELS_NOW_VISIBLE);
        initAxisXIntervals.push_back(-1.0);
    }

    Interface.SetScalarNames(initScalarNames);

    Interface.SetScalarMinima(initScalarMinima);
    Interface.SetScalarMaxima(initScalarMaxima);

    Interface.SetMinima(initMinima);
    Interface.SetMaxima(initMaxima);
    
    Interface.SetMinTimeOrdinals(initMinTimeOrdinals);
    Interface.SetMaxTimeOrdinals(initMaxTimeOrdinals);
    
    Interface.SetToolDrawsAxisLabels(false);
    
    Interface.SetAxisGroupNames(initAxisGroupNames);
    Interface.SetAxisLabelStates(initAxisLabelStates);
    Interface.SetAxisXIntervals(initAxisXIntervals);

    Interface.SetLeftSliderX(EA_DEFAULT_LEFT_SLIDER_X);
    Interface.SetRightSliderX(EA_DEFAULT_RIGHT_SLIDER_X);
    Interface.SetSlidersBottomY(EA_DEFAULT_SLIDERS_BOTTOM_Y);
    Interface.SetSlidersTopY(EA_DEFAULT_SLIDERS_TOP_Y);

    sliderActor = NULL; sliderMapper = NULL; sliderData = NULL;

    GetCurrentPlotAttributes();

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
    
    DeleteTextActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::GetCurrentPlotAttributes
//
// Purpose: Gets current attributes of the plot associated with the Extents
//          tool and uses them to define constants needed for rendering
//          performed by the tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov  8 16:01:27 PST 2006
//
// Modifications:
//
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Added support for selective axis labeling in associated plot.
//
// ****************************************************************************

void VisitExtentsTool::GetCurrentPlotAttributes()
{
    UpdateToolAttributesWithPlotAttributes();

    double leftSliderX    = Interface.GetLeftSliderX();
    double rightSliderX   = Interface.GetRightSliderX();
    double slidersBottomY = Interface.GetSlidersBottomY();
    double slidersTopY    = Interface.GetSlidersTopY();
    
    toolExtentCount = Interface.GetScalarNames().size();

    if (toolExtentCount < VET_MINIMUM_NUMBER_OF_EXTENTS)
        toolExtentCount = VET_MINIMUM_NUMBER_OF_EXTENTS;
        
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

    double minWindowDim =
        (double)(pixelWidth < pixelHeight ? pixelWidth : pixelHeight);
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

    hotPointRadius = VET_HOTPOINT_RADIUS * winWidth;

    minSlidableX = leftSliderX *winWidth + winXMin;
    maxSlidableX = rightSliderX*winWidth + winXMin;
    minSlidableY = worldBotSliderY - hotPointRadius;
    maxSlidableY = worldTopSliderY + hotPointRadius;

    sliderXStride = (maxSlidableX - minSlidableX) / (double)(toolExtentCount-1);
    
    if (toolExtentCount > VET_MAX_HORIZONTAL_TEXT_AXES)
    {
        axisTitleTextSize = (int)(minWindowDim*VET_V_TITLE_SIZE_MIN_DIM_RATIO+0.5);
        axisTitleDispXOffset = minWindowDim * VET_V_TITLE_X_OFFSET;
        axisTitleY = worldBotSliderY - VET_V_AXIS_BOTTOM_MARGIN;

        boundTextSize = (int)(minWindowDim*VET_V_BOUND_SIZE_MIN_DIM_RATIO + 0.5);

        axisMinY = worldBotSliderY - VET_V_AXIS_BOTTOM_MARGIN;
        axisMaxY = worldTopSliderY + VET_V_AXIS_TOP_MARGIN;
        axisMinDispXOffset = minWindowDim * VET_V_AXIS_MIN_X_OFFSET;
        axisMaxDispXOffset = minWindowDim * VET_V_AXIS_MAX_X_OFFSET;

        sliderMinY = axisMinY;
        sliderMaxY = axisMaxY;
        sliderMinDispXOffset = minWindowDim * VET_V_SLIDER_MIN_X_OFFSET;
        sliderMaxDispXOffset = minWindowDim * VET_V_SLIDER_MAX_X_OFFSET;
    }
    else
    {
        axisTitleTextSize = (int)(minWindowDim*VET_H_TITLE_SIZE_MIN_DIM_RATIO+0.5);
        axisTitleY = worldBotSliderY + hToWRatio*VET_H_TITLE_Y_OFFSET;

        boundTextSize = (int)(minWindowDim*VET_H_BOUND_SIZE_MIN_DIM_RATIO + 0.5);

        axisMinY = worldBotSliderY + hToWRatio*VET_H_AXIS_MIN_Y_OFFSET;
        axisMaxY = worldTopSliderY + hToWRatio*VET_H_AXIS_MAX_Y_OFFSET;

        sliderMinY = worldBotSliderY + hToWRatio*VET_H_SLIDER_MIN_Y_OFFSET;
        sliderMaxY = worldTopSliderY + hToWRatio*VET_H_SLIDER_MAX_Y_OFFSET;
    }
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
//    Mark Blair, Wed Nov  8 16:01:27 PST 2006
//    Transplanted some of this method's functionality to GetCurrentPlotAttributes.
//
// ****************************************************************************

void VisitExtentsTool::InitializeHotPoints()
{
    doubleVector extentMinima = Interface.GetMinima();
    doubleVector extentMaxima = Interface.GetMaxima();
    
    int extentNum;

    if (extentMinima.size() < VET_MINIMUM_NUMBER_OF_EXTENTS)
    {
        extentMinima.clear(); extentMaxima.clear();

        for (extentNum = 0; extentNum < toolExtentCount; extentNum++)
        {
            extentMinima.push_back(0.0); extentMaxima.push_back(1.0);
        }
    }

    HotPoint hotPoint;

    hotPoint.pt.z   = 0.0;
    hotPoint.radius = hotPointRadius;
    hotPoint.tool   = this;

    hotPoints.clear();
    
    for (extentNum = 0; extentNum < toolExtentCount; extentNum++)
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
// Method: VisitExtentsTool::UpdateToolAttributesWithPlotAttributes
//
// Purpose: Gets attributes of the plot to which the Extents tool is applied
//          (currently this can only be a ParallelAxis plot) and updates the
//          tool's attributes accordingly.  This would be analogous to the
//          plot's CreateCompatible method when called with "ExtentsAttributes"
//          as the input parameter.
//
// Programmer: Mark Blair
// Creation:   Thu Oct 26 18:40:28 PDT 2006
//
// Modifications:
//
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Added support for selective axis labeling in associated plot.
//
// ****************************************************************************

void VisitExtentsTool::UpdateToolAttributesWithPlotAttributes()
{
    const PlotInfoAttributes *parAxisAtts = proxy.GetPlotInfoAtts("ParallelAxis");
    
    if (parAxisAtts == NULL) return;

    doubleVector axisAttVals = parAxisAtts->GetOutputArray();    
    if (axisAttVals.size() < 5) return;

    stringVector newAxisNames;
    doubleVector newAxisMinima;
    doubleVector newAxisMaxima;
    doubleVector newSliderMinima;
    doubleVector newSliderMaxima;
    intVector    newMinTimeOrds;
    intVector    newMaxTimeOrds;
    stringVector newGroupNames;
    intVector    newLabelStates;
    doubleVector newXIntervals;
    
    int valCount = axisAttVals.size();
    int valIndex = 4;
    char nameChar;
    std::string newAxisName;
    std::string newGroupName;
    
    while (axisAttVals[valIndex] != 0)
    {
        newAxisName.clear();
        
        while (valIndex < valCount)
        {
            if ((nameChar = (char)axisAttVals[valIndex++]) == '\0') break;
            newAxisName += nameChar;
        }
        
        if (valIndex >= valCount) return;

        newGroupName.clear();
        
        while (valIndex < valCount)
        {
            if ((nameChar = (char)axisAttVals[valIndex++]) == '\0') break;
            newGroupName += nameChar;
        }
        
        if (valIndex+9 > valCount) return;
        
        newAxisNames.push_back(newAxisName);
        newAxisMinima.push_back(axisAttVals[valIndex++]);
        newAxisMaxima.push_back(axisAttVals[valIndex++]);
        newSliderMinima.push_back(axisAttVals[valIndex++]);
        newSliderMaxima.push_back(axisAttVals[valIndex++]);
        newMinTimeOrds.push_back((int)axisAttVals[valIndex++]);
        newMaxTimeOrds.push_back((int)axisAttVals[valIndex++]);
        newGroupNames.push_back(newGroupName);
        newLabelStates.push_back((int)axisAttVals[valIndex++]);
        newXIntervals.push_back(axisAttVals[valIndex++]);
    }
    
    Interface.SetScalarNames(newAxisNames);
    Interface.SetScalarMinima(newAxisMinima);
    Interface.SetScalarMaxima(newAxisMaxima);
    Interface.SetMinima(newSliderMinima);
    Interface.SetMaxima(newSliderMaxima);
    Interface.SetMinTimeOrdinals(newMinTimeOrds);
    Interface.SetMaxTimeOrdinals(newMaxTimeOrds);
    Interface.SetAxisGroupNames(newGroupNames);
    Interface.SetAxisLabelStates(newLabelStates);
    Interface.SetAxisXIntervals(newXIntervals);

    Interface.SetLeftSliderX(axisAttVals[0]);
    Interface.SetRightSliderX(axisAttVals[1]);
    Interface.SetSlidersBottomY(axisAttVals[2]);
    Interface.SetSlidersTopY(axisAttVals[3]);    
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
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Signals the plot that the tool will now draw all axis labels.
//
// ****************************************************************************

void VisitExtentsTool::Enable()
{
    bool toolWasEnabled = IsEnabled();

    Interface.SetToolDrawsAxisLabels(true);
    Interface.ExecuteCallback();

    VisitInteractiveTool::Enable();

    if (!toolWasEnabled) UpdateTool();
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
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Signals the plot that it must now draw all axis labels.
//
// ****************************************************************************

void VisitExtentsTool::Disable()
{
    bool toolWasEnabled = IsEnabled();
    
    Interface.SetToolDrawsAxisLabels(false);
    Interface.ExecuteCallback();

    VisitInteractiveTool::Disable();

    if (toolWasEnabled)
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

    RemoveText();

    GetCurrentPlotAttributes();

    InitializeHotPoints();

    CreateSliderActor();
    CreateTextActors();

    proxy.GetCanvas()->AddActor(sliderActor);

    AddText();
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
// Purpose: Creates all text actors needed by the tool.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateTextActors()
{
    CreateAxisTitleActors();
    CreateAxisBoundActors();
    CreateSliderBoundActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisTitleActors
//
// Purpose: Creates a text actor for each axis title to be displayed on the
//          plot.  Ideally each title should appear on the plot exactly as it
//          would if the Extents tool were not enabled.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateAxisTitleActors()
{
    stringVector axisTitles = Interface.GetScalarNames();

    avtVector titlePos;
    vtkTextActor *axisTitleActor;
    int titleNum;
    double axisTitleX;
    char axisTitle[VET_H_TITLE_MAX_CHARS + 1];
    double titleColor[3] = { VET_TITLE_RED, VET_TITLE_GREEN, VET_TITLE_BLUE };
    
    for (titleNum = 0; titleNum < axisTitleActors.size(); titleNum++)
        axisTitleActors[titleNum]->Delete();

    axisTitleActors.clear();
    
    for (titleNum = 0; titleNum < toolExtentCount; titleNum++)
    {
        axisTitleActor = vtkTextActor::New();
        
        axisTitleActor->ScaledTextOff();
        axisTitleActor->GetTextProperty()->SetColor(titleColor);
        axisTitleActor->GetTextProperty()->SetFontFamilyToArial();
        axisTitleActor->GetTextProperty()->BoldOn();
        axisTitleActor->GetTextProperty()->SetFontSize(axisTitleTextSize);
        
        axisTitleX = (double)titleNum*sliderXStride + minSlidableX;
        titlePos = ComputeWorldToDisplay(avtVector(axisTitleX, axisTitleY, 0.0));

        if (toolExtentCount > VET_MAX_HORIZONTAL_TEXT_AXES)
        {
            MakeAxisTitleText(
                axisTitle, axisTitles[titleNum], VET_V_TITLE_MAX_CHARS);

            titlePos.x += axisTitleDispXOffset;

            axisTitleActor->GetTextProperty()->SetOrientation(90.0);

            axisTitleActor->GetTextProperty()->SetJustificationToRight();
            axisTitleActor->GetTextProperty()->SetVerticalJustificationToBottom();
        }
        else
        {
            MakeAxisTitleText(
                axisTitle, axisTitles[titleNum], VET_H_TITLE_MAX_CHARS);

            axisTitleActor->GetTextProperty()->SetOrientation(0.0);

            if (titleNum == 0)
                axisTitleActor->GetTextProperty()->SetJustificationToLeft();
            else if (titleNum == toolExtentCount-1)
                axisTitleActor->GetTextProperty()->SetJustificationToRight();
            else
                axisTitleActor->GetTextProperty()->SetJustificationToCentered();
            
            axisTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
        }

        axisTitleActor->SetInput(axisTitle);
        axisTitleActor->SetPosition(titlePos.x, titlePos.y);
        
        axisTitleActors.push_back(axisTitleActor);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisBoundActors
//
// Purpose: Creates a text actor for each numerical bound to be displayed at
//          one end of an axis on the plot.  This bound corresponds to the
//          axis variable's value at that end of the axis.  Ideally each such
//          bound should appear on the plot exactly as it would if the Extents
//          tool were not enabled.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateAxisBoundActors()
{
    doubleVector axisMinima = Interface.GetScalarMinima();
    doubleVector axisMaxima = Interface.GetScalarMaxima();

    avtVector axisMinPos;
    avtVector axisMaxPos;
    vtkTextActor *axisMinActor, *axisMaxActor;
    int boundNum;
    double axisX;
    char minBoundText[VET_H_BOUND_MAX_CHARS + 1];
    char maxBoundText[VET_H_BOUND_MAX_CHARS + 1];
    double axisBoundColor[3] =
        { VET_AXIS_BOUND_RED, VET_AXIS_BOUND_GREEN, VET_AXIS_BOUND_BLUE };
    
    for (boundNum = 0; boundNum < axisMinActors.size(); boundNum++)
    {
        axisMinActors[boundNum]->Delete();
        axisMaxActors[boundNum]->Delete();
    }

    axisMinActors.clear(); axisMaxActors.clear();
    
    for (boundNum = 0; boundNum < toolExtentCount; boundNum++)
    {
        axisMinActor = vtkTextActor::New();
        axisMaxActor = vtkTextActor::New();
        
        axisMinActor->ScaledTextOff();
        axisMinActor->GetTextProperty()->SetColor(axisBoundColor);
        axisMinActor->GetTextProperty()->SetFontFamilyToArial();
        axisMinActor->GetTextProperty()->SetFontSize(boundTextSize);
        axisMinActor->GetTextProperty()->BoldOn();

        axisMaxActor->ScaledTextOff();
        axisMaxActor->GetTextProperty()->SetColor(axisBoundColor);
        axisMaxActor->GetTextProperty()->SetFontFamilyToArial();
        axisMaxActor->GetTextProperty()->SetFontSize(boundTextSize);
        axisMaxActor->GetTextProperty()->BoldOn();
        
        axisX = (double)boundNum*sliderXStride + minSlidableX;

        axisMinPos = ComputeWorldToDisplay(avtVector(axisX, axisMinY, 0.0));
        axisMaxPos = ComputeWorldToDisplay(avtVector(axisX, axisMaxY, 0.0));

        if (toolExtentCount > VET_MAX_HORIZONTAL_TEXT_AXES)
        {
            axisMinPos.x += axisMinDispXOffset;
            axisMaxPos.x += axisMaxDispXOffset;
        
            axisMinActor->GetTextProperty()->SetOrientation(90.0);
            axisMaxActor->GetTextProperty()->SetOrientation(90.0);

            axisMinActor->GetTextProperty()->SetJustificationToRight();
            axisMinActor->GetTextProperty()->SetVerticalJustificationToBottom();

            axisMaxActor->GetTextProperty()->SetJustificationToLeft();
            axisMaxActor->GetTextProperty()->SetVerticalJustificationToBottom();
        }
        else
        {
            axisMinActor->GetTextProperty()->SetOrientation(0.0);
            axisMaxActor->GetTextProperty()->SetOrientation(0.0);

            if (boundNum == 0)
            {
                axisMinActor->GetTextProperty()->SetJustificationToLeft();
                axisMaxActor->GetTextProperty()->SetJustificationToLeft();
            }
            else if (boundNum == toolExtentCount-1)
            {
                axisMinActor->GetTextProperty()->SetJustificationToRight();
                axisMaxActor->GetTextProperty()->SetJustificationToRight();
            }
            else
            {
                axisMinActor->GetTextProperty()->SetJustificationToCentered();
                axisMaxActor->GetTextProperty()->SetJustificationToCentered();
            }
            
            axisMinActor->GetTextProperty()->SetVerticalJustificationToCentered();
            axisMaxActor->GetTextProperty()->SetVerticalJustificationToCentered();
        }
            
        axisMinActor->SetPosition(axisMinPos.x, axisMinPos.y);
        axisMaxActor->SetPosition(axisMaxPos.x, axisMaxPos.y);

        MakeDataBoundText(minBoundText, axisMinima[boundNum]);
        MakeDataBoundText(maxBoundText, axisMaxima[boundNum]);

        axisMinActor->SetInput(minBoundText);
        axisMaxActor->SetInput(maxBoundText);
        
        axisMinActors.push_back(axisMinActor);
        axisMaxActors.push_back(axisMaxActor);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateSliderBoundActors
//
// Purpose: Creates a text actor for each numerical bound to be displayed next
//          to the axis bound at one end of an axis on the plot.  This bound
//          denotes the value currently selected by the corresponding min or
//          max slider along that axis.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateSliderBoundActors()
{
    doubleVector axisMinima   = Interface.GetScalarMinima();
    doubleVector axisMaxima   = Interface.GetScalarMaxima();
    doubleVector sliderMinima = Interface.GetMinima();
    doubleVector sliderMaxima = Interface.GetMaxima();

    avtVector sliderMinPos;
    avtVector sliderMaxPos;
    vtkTextActor *sliderMinActor, *sliderMaxActor;
    int boundNum;
    double axisX, axisMinBound, axisRange, sliderMinBound, sliderMaxBound;
    char minBoundText[VET_H_BOUND_MAX_CHARS + 1];
    char maxBoundText[VET_H_BOUND_MAX_CHARS + 1];
    double sliderBoundColor[3] =
        { VET_SLIDER_BOUND_RED, VET_SLIDER_BOUND_GREEN, VET_SLIDER_BOUND_BLUE };
    
    for (boundNum = 0; boundNum < sliderMinActors.size(); boundNum++)
    {
        sliderMinActors[boundNum]->Delete();
        sliderMaxActors[boundNum]->Delete();
    }

    sliderMinActors.clear(); sliderMaxActors.clear();
    
    for (boundNum = 0; boundNum < toolExtentCount; boundNum++)
    {
        sliderMinActor = vtkTextActor::New();
        sliderMaxActor = vtkTextActor::New();
        
        sliderMinActor->ScaledTextOff();
        sliderMinActor->GetTextProperty()->SetColor(sliderBoundColor);
        sliderMinActor->GetTextProperty()->SetFontFamilyToArial();
        sliderMinActor->GetTextProperty()->SetFontSize(boundTextSize);
        sliderMinActor->GetTextProperty()->BoldOn();

        sliderMaxActor->ScaledTextOff();
        sliderMaxActor->GetTextProperty()->SetColor(sliderBoundColor);
        sliderMaxActor->GetTextProperty()->SetFontFamilyToArial();
        sliderMaxActor->GetTextProperty()->SetFontSize(boundTextSize);
        sliderMaxActor->GetTextProperty()->BoldOn();
        
        axisX = (double)boundNum*sliderXStride + minSlidableX;

        sliderMinPos = ComputeWorldToDisplay(avtVector(axisX, sliderMinY, 0.0));
        sliderMaxPos = ComputeWorldToDisplay(avtVector(axisX, sliderMaxY, 0.0));

        if (toolExtentCount > VET_MAX_HORIZONTAL_TEXT_AXES)
        {
            sliderMinPos.x += sliderMinDispXOffset;
            sliderMaxPos.x += sliderMaxDispXOffset;
        
            sliderMinActor->GetTextProperty()->SetOrientation(90.0);
            sliderMaxActor->GetTextProperty()->SetOrientation(90.0);

            sliderMinActor->GetTextProperty()->SetJustificationToRight();
            sliderMinActor->GetTextProperty()->SetVerticalJustificationToBottom();

            sliderMaxActor->GetTextProperty()->SetJustificationToLeft();
            sliderMaxActor->GetTextProperty()->SetVerticalJustificationToBottom();
        }
        else
        {
            sliderMinActor->GetTextProperty()->SetOrientation(0.0);
            sliderMaxActor->GetTextProperty()->SetOrientation(0.0);

            if (boundNum == 0)
            {
                sliderMinActor->GetTextProperty()->SetJustificationToLeft();
                sliderMaxActor->GetTextProperty()->SetJustificationToLeft();
            }
            else if (boundNum == toolExtentCount-1)
            {
                sliderMinActor->GetTextProperty()->SetJustificationToRight();
                sliderMaxActor->GetTextProperty()->SetJustificationToRight();
            }
            else
            {
                sliderMinActor->GetTextProperty()->SetJustificationToCentered();
                sliderMaxActor->GetTextProperty()->SetJustificationToCentered();
            }
            
            sliderMinActor->GetTextProperty()->SetVerticalJustificationToCentered();
            sliderMaxActor->GetTextProperty()->SetVerticalJustificationToCentered();
        }
            
        sliderMinActor->SetPosition(sliderMinPos.x, sliderMinPos.y);
        sliderMaxActor->SetPosition(sliderMaxPos.x, sliderMaxPos.y);
        
        axisMinBound = axisMinima[boundNum];
        axisRange = axisMaxima[boundNum] - axisMinBound;
        sliderMinBound = sliderMinima[boundNum]*axisRange + axisMinBound;
        sliderMaxBound = sliderMaxima[boundNum]*axisRange + axisMinBound;

        MakeDataBoundText(minBoundText, sliderMinBound);
        MakeDataBoundText(maxBoundText, sliderMaxBound);

        sliderMinActor->SetInput(minBoundText);
        sliderMaxActor->SetInput(maxBoundText);
        
        sliderMinActors.push_back(sliderMinActor);
        sliderMaxActors.push_back(sliderMaxActor);
    }
}


// ****************************************************************************
// Method: VisitPlaneTool::DeleteTextActors
//
// Purpose: Deletes any text actors and mappers created by the tool.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteTextActors()
{
    int actorNum;

    for (actorNum = 0; actorNum < axisTitleActors.size(); actorNum++)
        axisTitleActors[actorNum]->Delete();

    for (actorNum = 0; actorNum < axisMinActors.size(); actorNum++)
    {
        axisMinActors[actorNum]->Delete();
        axisMaxActors[actorNum]->Delete();
    }

    for (actorNum = 0; actorNum < sliderMinActors.size(); actorNum++)
    {
        sliderMinActors[actorNum]->Delete();
        sliderMaxActors[actorNum]->Delete();
    }

    axisTitleActors.clear();
    axisMinActors.clear();   axisMaxActors.clear();
    sliderMinActors.clear(); sliderMaxActors.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::AddText
//
// Purpose: Adds to the foreground canvas any text actors created by the tool.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::AddText()
{
    int actorNum;

    for (actorNum = 0; actorNum < axisTitleActors.size(); actorNum++)
        proxy.GetForeground()->AddActor2D(axisTitleActors[actorNum]);

    for (actorNum = 0; actorNum < axisMinActors.size(); actorNum++)
    {
        proxy.GetForeground()->AddActor2D(axisMinActors[actorNum]);
        proxy.GetForeground()->AddActor2D(axisMaxActors[actorNum]);
    }

    for (actorNum = 0; actorNum < sliderMinActors.size(); actorNum++)
    {
        proxy.GetForeground()->AddActor2D(sliderMinActors[actorNum]);
        proxy.GetForeground()->AddActor2D(sliderMaxActors[actorNum]);
    }
}


// ****************************************************************************
// Method: VisitPlaneTool::RemoveText
//
// Purpose: Removed from the foreground canvas any text actors created by
//          the tool.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::RemoveText()
{
    int actorNum;

    for (actorNum = 0; actorNum < axisTitleActors.size(); actorNum++)
        proxy.GetForeground()->RemoveActor2D(axisTitleActors[actorNum]);

    for (actorNum = 0; actorNum < axisMinActors.size(); actorNum++)
    {
        proxy.GetForeground()->RemoveActor2D(axisMinActors[actorNum]);
        proxy.GetForeground()->RemoveActor2D(axisMaxActors[actorNum]);
    }

    for (actorNum = 0; actorNum < sliderMinActors.size(); actorNum++)
    {
        proxy.GetForeground()->RemoveActor2D(sliderMinActors[actorNum]);
        proxy.GetForeground()->RemoveActor2D(sliderMaxActors[actorNum]);
    }
}


// ****************************************************************************
// Method: VisitPlaneTool::UpdateText
//
// Purpose: Updates the info to be displayed by any text actors created by
//          the tool.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::UpdateText()
{
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
//    Allows the tool to remove and re-add actors so that they will be rendered
//    after the plot.
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


// *****************************************************************************
//  Method: VisitExtentsTool::MakeAxisTitleText
//
//  Purpose: Creates a displayable axis title from an axis variable name.  Long
//           titles and compound titles are shortened in a meaningful way.
//
//  Programmer: Mark Blair
//  Creation:   Tue Nov 28 13:25:41 PST 2006
//
//  Modifications:
//
// *****************************************************************************

void VisitExtentsTool::MakeAxisTitleText(
    char titleText[], const std::string &axisTitle, int maxTitleChars)
{
    int rawTitleLen;
    char *secondPart;
    char rawTitle[121];
    
    strncpy(rawTitle, axisTitle.c_str(), 120);
    
    if ((rawTitleLen = strlen(rawTitle)) <= maxTitleChars)
    {
        strcpy(titleText, rawTitle);
    }
    else
    {
        if ((secondPart = strrchr(rawTitle, '/')) == NULL)
            secondPart = &rawTitle[rawTitleLen-3];
        else if (strlen(secondPart) > 4)
            secondPart[4] = '0';
            
        rawTitle[maxTitleChars-strlen(secondPart)-2] = '\0';
        
        sprintf(titleText, "%s..%s", rawTitle, secondPart);
    }
}


// *****************************************************************************
//  Method: VisitExtentsTool::MakeDataBoundText
//
//  Purpose: Converts a double-precision floating-point data bound value to a
//           VisIt-style C string representation of a data bound.
//
//  Programmer: Mark Blair
//  Creation:   Thu Nov 16 16:11:52 PST 2006
//
//  Modifications:
//
//     Mark Blair, Mon Nov 20 16:54:21 PST 2006
//     No longer stripping low-order '0' characters from E-format exponents.
//
// *****************************************************************************

void VisitExtentsTool::MakeDataBoundText(char boundText[], double boundValue)
{
    int textLen, charNum;

    if (boundValue < -9e+36) {
        strcpy(boundText, "min");
        return;
    }

    if (boundValue > +9e+36) {
        strcpy(boundText, "max");
        return;
    }

    sprintf (boundText, "%g", boundValue);

    if (strchr(boundText, 'e') != NULL) return;
    if (strchr(boundText, 'E') != NULL) return;
    
    if ((textLen = strlen(boundText)) < 3) return;

    for (charNum = textLen - 1; charNum > 1; charNum--)
    {
        if (boundText[charNum  ] != '0') break;
        if (boundText[charNum-1] == '.') break;
        if (boundText[charNum-1] == '+') break;
        if (boundText[charNum-1] == '-') break;
    }

    boundText[charNum + 1] = '\0';
}
