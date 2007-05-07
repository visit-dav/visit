/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include <DebugStream.h>


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
//    Added support for selective axis information in associated plot.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Mark Blair, Wed Mar 14 18:04:12 PDT 2007
//    Added support for ganged axis sliders.
//
// ****************************************************************************

VisitExtentsTool::VisitExtentsTool(VisWindowToolProxy &p) : VisitInteractiveTool(p),
    Interface(p)
{
    int allAxesInfoFlags = EA_THRESHOLD_BY_EXTENT_FLAG |
                           EA_SHOW_ALL_AXIS_INFO_FLAGS | EA_AXIS_INFO_SHOWN_FLAG;
    int axisInfoFlags, extentNum;
    
    stringVector initScalarNames;
    doubleVector initScalarMinima;
    doubleVector initScalarMaxima;
    doubleVector initMinima;
    doubleVector initMaxima;
    intVector    initMinTimeOrdinals;
    intVector    initMaxTimeOrdinals;
    stringVector initAxisGroupNames;
    intVector    initAxisInfoFlagSets;
    doubleVector initAxisXPositions;
    
    curTimeOrdinal = 1;
    activeAxisIndex = 0;
    gangedHPDeltaY = -1.0;

    for (extentNum = 0; extentNum < EA_DEFAULT_NUMBER_OF_EXTENTS; extentNum++)
    {
        axisInfoFlags = allAxesInfoFlags;
        
        if (extentNum == 0)
            axisInfoFlags |= EA_LEFT_SHOWN_AXIS_FLAG | EA_LEFT_SELECTED_AXIS_FLAG;
        else if (extentNum == EA_DEFAULT_NUMBER_OF_EXTENTS-1)
            axisInfoFlags |= EA_RIGHT_SHOWN_AXIS_FLAG | EA_RIGHT_SELECTED_AXIS_FLAG;

        initScalarNames.push_back("default");
        initScalarMinima.push_back(-1e+37);
        initScalarMaxima.push_back(1e+37);
        initMinima.push_back(0.0);
        initMaxima.push_back(1.0);
        initMinTimeOrdinals.push_back(0);
        initMaxTimeOrdinals.push_back(0);
        initAxisGroupNames.push_back(std::string("(not_in_a_group)"));
        initAxisInfoFlagSets.push_back(axisInfoFlags);
        initAxisXPositions.push_back(
            (double)extentNum / (double)(EA_DEFAULT_NUMBER_OF_EXTENTS-1));
    }

    Interface.SetScalarNames(initScalarNames);

    Interface.SetScalarMinima(initScalarMinima);
    Interface.SetScalarMaxima(initScalarMaxima);

    Interface.SetMinima(initMinima);
    Interface.SetMaxima(initMaxima);
    
    Interface.SetMinTimeOrdinals(initMinTimeOrdinals);
    Interface.SetMaxTimeOrdinals(initMaxTimeOrdinals);
    
    Interface.SetPlotToolModeFlags(EA_AXIS_INFO_AUTO_LAYOUT_FLAG);
    
    Interface.SetAxisGroupNames(initAxisGroupNames);
    Interface.SetAxisInfoFlagSets(initAxisInfoFlagSets);
    Interface.SetAxisXPositions(initAxisXPositions);

    Interface.SetLeftSliderX(EA_DEFAULT_LEFT_SLIDER_X);
    Interface.SetRightSliderX(EA_DEFAULT_RIGHT_SLIDER_X);
    Interface.SetSlidersBottomY(EA_DEFAULT_SLIDERS_BOTTOM_Y);
    Interface.SetSlidersTopY(EA_DEFAULT_SLIDERS_TOP_Y);

    GetCurrentPlotAttributes();
    
    DoOneTimeInitializations();
    
    PositionAxisSequenceMarks();
    InitializeAllHotpoints();
    CreateAllActors();
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
    DeleteAllActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::DoOneTimeInitializations
//
// Purpose: Perform all initializations that only need to be done when the
//          Extents tool's constructor is called.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
// ****************************************************************************

void VisitExtentsTool::DoOneTimeInitializations()
{
    buttonQuadsActor    = NULL; buttonQuadsMapper    = NULL; buttonQuadsData    = NULL;
    buttonLogosActor    = NULL; buttonLogosMapper    = NULL; buttonLogosData    = NULL;
    buttonChecksActor   = NULL; buttonChecksMapper   = NULL; buttonChecksData   = NULL;
    markSlidersActor    = NULL; markSlidersMapper    = NULL; markSlidersData    = NULL;
    axisExtensionsActor = NULL; axisExtensionsMapper = NULL; axisExtensionsData = NULL;
    axisSlidersActor    = NULL; axisSlidersMapper    = NULL; axisSlidersData    = NULL;
    
    buttonLabelsActors.clear();
    markTitlesActors.clear();
    axisTitlesActors.clear();
    axisMinimaActors.clear();   axisMaximaActors.clear();
    extentMinimaActors.clear(); extentMaximaActors.clear();
    
    leftExpandAxisIDs.clear(); rightExpandAxisIDs.clear();
    
    for (int buttonID = 0; buttonID < VET_MAX_BUTTONS; buttonID++)
    {
        buttonXs[buttonID] = VET_NOT_A_BUTTON_X;
    }

    buttonXs[VET_LEFT_MOVE_LEFT_BUTTON_ID]      = VET_LEFT_MOVE_LEFT_BUTTON_X;
    buttonXs[VET_RIGHT_MOVE_LEFT_BUTTON_ID]     = VET_RIGHT_MOVE_LEFT_BUTTON_X;
    buttonXs[VET_LEFT_MOVE_RIGHT_BUTTON_ID]     = VET_LEFT_MOVE_RIGHT_BUTTON_X;
    buttonXs[VET_RIGHT_MOVE_RIGHT_BUTTON_ID]    = VET_RIGHT_MOVE_RIGHT_BUTTON_X;
    buttonXs[VET_LEFT_MARK_INFO_ON_BUTTON_ID]   = VET_LEFT_MARK_INFO_ON_BUTTON_X;
    buttonXs[VET_RIGHT_MARK_INFO_ON_BUTTON_ID]  = VET_RIGHT_MARK_INFO_ON_BUTTON_X;
    buttonXs[VET_EXPAND_BETWEEN_BUTTON_ID]      = VET_EXPAND_BETWEEN_BUTTON_X;
    buttonXs[VET_UNDO_LAST_EXPANSION_BUTTON_ID] = VET_UNDO_LAST_EXPANSION_BUTTON_X;
    buttonXs[VET_THRESHOLD_BETWEEN_BUTTON_ID]   = VET_THRESHOLD_BETWEEN_BUTTON_X;
    buttonXs[VET_SHOW_TITLES_ONLY_BUTTON_ID]    = VET_SHOW_TITLES_ONLY_BUTTON_X;

    buttonFlags[VET_LEFT_MOVE_LEFT_BUTTON_ID]      = VET_BUTTON_HAS_LEFT_ARROW_LOGO;
    buttonFlags[VET_RIGHT_MOVE_LEFT_BUTTON_ID]     = VET_BUTTON_HAS_LEFT_ARROW_LOGO;
    buttonFlags[VET_LEFT_MOVE_RIGHT_BUTTON_ID]     = VET_BUTTON_HAS_RIGHT_ARROW_LOGO;
    buttonFlags[VET_RIGHT_MOVE_RIGHT_BUTTON_ID]    = VET_BUTTON_HAS_RIGHT_ARROW_LOGO;
    buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID]   = VET_BUTTON_IS_A_TOGGLE |
                                                     VET_BUTTON_HAS_CHECK_MARK;
    buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID]  = VET_BUTTON_IS_A_TOGGLE |
                                                     VET_BUTTON_HAS_CHECK_MARK;
    buttonFlags[VET_EXPAND_BETWEEN_BUTTON_ID]      = 0;
    buttonFlags[VET_UNDO_LAST_EXPANSION_BUTTON_ID] = 0;
    buttonFlags[VET_THRESHOLD_BETWEEN_BUTTON_ID]   = VET_BUTTON_IS_A_TOGGLE |
                                                     VET_BUTTON_HAS_CHECK_MARK;
    buttonFlags[VET_SHOW_TITLES_ONLY_BUTTON_ID]    = VET_BUTTON_IS_A_TOGGLE |
                                                     VET_BUTTON_HAS_CHECK_MARK;
    
    for (int labelID = 0; labelID < VET_MAX_BUTTON_LABELS; labelID++)
    {
        buttonLabels[labelID] = NULL;
    }
    
    buttonLabels[VET_MOVE_LEFT_MARK_B_LAB_ID]      = "Move left mark";
    buttonLabels[VET_MOVE_RIGHT_MARK_B_LAB_ID]     = "Move right mark";
    buttonLabels[VET_LEFT_MARK_INFO_ON_B_LAB_ID]   = "Left mark axis\ninfo shown";
    buttonLabels[VET_RIGHT_MARK_INFO_ON_B_LAB_ID]  = "Right mark axis\ninfo shown";
    buttonLabels[VET_EXPAND_BETWEEN_B_LAB_ID]      = "Expand between marks";
    buttonLabels[VET_UNDO_LAST_EXPANSION_B_LAB_ID] = "Undo last expansion";
    buttonLabels[VET_THRESHOLD_BETWEEN_B_LAB_ID]   = "Threshold between marks";
    buttonLabels[VET_SHOW_TITLES_ONLY_B_LAB_ID]    = "Show axis titles only";

    buttonLabelXs[VET_MOVE_LEFT_MARK_B_LAB_ID]      = VET_MOVE_LEFT_MARK_B_LABEL_X;
    buttonLabelXs[VET_MOVE_RIGHT_MARK_B_LAB_ID]     = VET_MOVE_RIGHT_MARK_B_LABEL_X;
    buttonLabelXs[VET_LEFT_MARK_INFO_ON_B_LAB_ID]   = VET_LEFT_MARK_INFO_ON_B_LABEL_X;
    buttonLabelXs[VET_RIGHT_MARK_INFO_ON_B_LAB_ID]  = VET_RIGHT_MARK_INFO_ON_B_LABEL_X;
    buttonLabelXs[VET_EXPAND_BETWEEN_B_LAB_ID]      = VET_EXPAND_BETWEEN_B_LABEL_X;
    buttonLabelXs[VET_UNDO_LAST_EXPANSION_B_LAB_ID] = VET_UNDO_LAST_EXPANSION_B_LABEL_X;
    buttonLabelXs[VET_THRESHOLD_BETWEEN_B_LAB_ID]   = VET_THRESHOLD_BETWEEN_B_LABEL_X;
    buttonLabelXs[VET_SHOW_TITLES_ONLY_B_LAB_ID]    = VET_SHOW_TITLES_ONLY_B_LABEL_X;

    buttonLabelJusts[VET_MOVE_LEFT_MARK_B_LAB_ID] =
        VET_MOVE_LEFT_MARK_B_L_JUST;
    buttonLabelJusts[VET_MOVE_RIGHT_MARK_B_LAB_ID] =
        VET_MOVE_RIGHT_MARK_B_L_JUST;
    buttonLabelJusts[VET_LEFT_MARK_INFO_ON_B_LAB_ID] =
        VET_LEFT_MARK_INFO_ON_B_L_JUST;
    buttonLabelJusts[VET_RIGHT_MARK_INFO_ON_B_LAB_ID] =
        VET_RIGHT_MARK_INFO_ON_B_L_JUST;
    buttonLabelJusts[VET_EXPAND_BETWEEN_B_LAB_ID] =
        VET_EXPAND_BETWEEN_B_L_JUST;
    buttonLabelJusts[VET_UNDO_LAST_EXPANSION_B_LAB_ID] =
        VET_UNDO_LAST_EXPANSION_B_L_JUST;
    buttonLabelJusts[VET_THRESHOLD_BETWEEN_B_LAB_ID] =
        VET_THRESHOLD_BETWEEN_B_L_JUST;
    buttonLabelJusts[VET_SHOW_TITLES_ONLY_B_LAB_ID] =
        VET_SHOW_TITLES_ONLY_B_L_JUST;

    buttonColor0Red   = (unsigned char)(VET_BUTTON_COLOR_0_RED   * 255.0);
    buttonColor0Green = (unsigned char)(VET_BUTTON_COLOR_0_GREEN * 255.0);
    buttonColor0Blue  = (unsigned char)(VET_BUTTON_COLOR_0_BLUE  * 255.0);

    buttonColor1Red   = (unsigned char)(VET_BUTTON_COLOR_1_RED   * 255.0);
    buttonColor1Green = (unsigned char)(VET_BUTTON_COLOR_1_GREEN * 255.0);
    buttonColor1Blue  = (unsigned char)(VET_BUTTON_COLOR_1_BLUE  * 255.0);

    buttonColor2Red   = (unsigned char)(VET_BUTTON_COLOR_2_RED   * 255.0);
    buttonColor2Green = (unsigned char)(VET_BUTTON_COLOR_2_GREEN * 255.0);
    buttonColor2Blue  = (unsigned char)(VET_BUTTON_COLOR_2_BLUE  * 255.0);

    buttonColor3Red   = (unsigned char)(VET_BUTTON_COLOR_3_RED   * 255.0);
    buttonColor3Green = (unsigned char)(VET_BUTTON_COLOR_3_GREEN * 255.0);
    buttonColor3Blue  = (unsigned char)(VET_BUTTON_COLOR_3_BLUE  * 255.0);

    buttonLogoRed   = (unsigned char)(VET_BUTTON_LOGO_RED   * 255.0);
    buttonLogoGreen = (unsigned char)(VET_BUTTON_LOGO_GREEN * 255.0);
    buttonLogoBlue  = (unsigned char)(VET_BUTTON_LOGO_BLUE  * 255.0);

    markSliderRed   = (unsigned char)(VET_MARK_SLIDER_RED   * 255.0);
    markSliderGreen = (unsigned char)(VET_MARK_SLIDER_GREEN * 255.0);
    markSliderBlue  = (unsigned char)(VET_MARK_SLIDER_BLUE  * 255.0);

    axisExtensionRed   = (unsigned char)(VET_AXIS_EXTENSION_RED   * 255.0);
    axisExtensionGreen = (unsigned char)(VET_AXIS_EXTENSION_GREEN * 255.0);
    axisExtensionBlue  = (unsigned char)(VET_AXIS_EXTENSION_BLUE  * 255.0);

    axisSliderRed   = (unsigned char)(VET_AXIS_SLIDER_RED   * 255.0);
    axisSliderGreen = (unsigned char)(VET_AXIS_SLIDER_GREEN * 255.0);
    axisSliderBlue  = (unsigned char)(VET_AXIS_SLIDER_BLUE  * 255.0);
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
//    Added support for selective axis information in associated plot.
//
//    Mark Blair, Mon Jan  8 16:31:01 PST 2007
//    Added support for the dotted axis extension lines above labelled axes.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
// ****************************************************************************

void VisitExtentsTool::GetCurrentPlotAttributes()
{
    UpdateToolAttributesWithPlotAttributes();

    int plotToolModeFlags = Interface.GetPlotToolModeFlags();
    double leftSliderX    = Interface.GetLeftSliderX();
    double rightSliderX   = Interface.GetRightSliderX();
    double slidersBottomY =
        Interface.GetSlidersBottomY() + EA_TOOL_BUTTON_MARGIN_FRACTION;
    double slidersTopY    =
        Interface.GetSlidersTopY() - EA_TOOL_MARK_MARGIN_FRACTION;

    parallelAxisCount = Interface.GetScalarNames().size();
    labelsAreVertical = ((plotToolModeFlags & EA_VERTICAL_TEXT_AXIS_INFO_FLAG) != 0);

    if (parallelAxisCount < VET_MINIMUM_NUMBER_OF_EXTENTS)
    {
        debug3 << "VET/GCPA/1: Axis attribute data from plot involves "
               << "fewer than 2 axes." << endl;
        parallelAxisCount = VET_MINIMUM_NUMBER_OF_EXTENTS;
    }
        
    if ((plotToolModeFlags & EA_PLOT_AXES_WERE_MODIFIED_FLAG) != 0)
    {
        if (leftExpandAxisIDs.size() > 0)
        {
            leftExpandAxisIDs.clear(); rightExpandAxisIDs.clear();

            leftExpandAxisIDs.push_back(0);
            rightExpandAxisIDs.push_back(parallelAxisCount-1);
        }
        
        plotToolModeFlags &= (0xffffffff ^ EA_PLOT_AXES_WERE_MODIFIED_FLAG);
        
        Interface.SetPlotToolModeFlags(plotToolModeFlags);
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

    double minWindowDim =
        (double)(pixelWidth < pixelHeight ? pixelWidth : pixelHeight);
    double maxWindowDim =
        (double)(pixelWidth > pixelHeight ? pixelWidth : pixelHeight);
    double fudgeTerm = 0.07;

    windowHToWRatio = (double)pixelHeight / (double)pixelWidth;

    if (windowHToWRatio < 1.05477)
        fudgeTerm -= (1.05477 - windowHToWRatio) * 0.07;
    else
        fudgeTerm += (1.0 - 1.05477/windowHToWRatio) * 0.12;

    double winHeight = (windowHToWRatio+fudgeTerm) * winWidth;

    double worldBotSliderY = slidersBottomY*winHeight + winYMin;
    double worldTopSliderY = slidersTopY*winHeight + winYMin;
    normToWorldYScale = worldTopSliderY - worldBotSliderY;

    double buttonRadiusScale = winWidth * (minWindowDim / maxWindowDim);
    buttonInnerRadius = VET_BUTTON_QUAD_INNER_RADIUS * buttonRadiusScale;
    buttonLogoRadius  = VET_BUTTON_LOGO_RADIUS * buttonRadiusScale;
    buttonCheckRadius = VET_BUTTON_CHECK_MARK_RADIUS * buttonRadiusScale;
    bqHotpointRadius  = VET_BUTTON_HOTPOINT_RADIUS * buttonRadiusScale;
    msHotpointRadius  = VET_MARK_SLIDER_HOTPOINT_RADIUS * winWidth;
    asHotpointRadius  = VET_AXIS_SLIDER_HOTPOINT_RADIUS * winWidth;
    
    double extraWeightTerm = (minWindowDim >= 1024.0) ? 0.0 :
        (1024.0-minWindowDim) * VET_BUTTON_BEVEL_EXTRA_WEIGHT;
    buttonOuterRadius =
        (VET_BUTTON_BEVEL_WIDTH_WEIGHT+extraWeightTerm+1.0) * buttonInnerRadius;
    buttonHotpointYOffset =
        (windowHToWRatio >= 1.0) ? 0 : (1.0-windowHToWRatio)*buttonInnerRadius;
    buttonLabelTextSize = (int)(minWindowDim*VET_B_LABEL_SIZE_MIN_DIM_RATIO + 0.5);

    leftAxisX  =  leftSliderX*winWidth + winXMin;
    rightAxisX = rightSliderX*winWidth + winXMin;

    markSliderXRange = rightAxisX - leftAxisX;
    markTitleXOffset = (double)pixelWidth * VET_MARK_TITLE_X_OFFSET;
    markTitleTextSize =
        (int)((double)pixelWidth*VET_MARK_TITLE_SIZE_WIDTH_RATIO + 0.5);

    minSlidableX = leftAxisX - msHotpointRadius;
    maxSlidableX = rightAxisX + msHotpointRadius;
    minSlidableY = worldBotSliderY - asHotpointRadius;
    maxSlidableY = worldTopSliderY + asHotpointRadius;

    if (labelsAreVertical)
    {
        axisTitleTextSize = (int)(minWindowDim*VET_V_TITLE_SIZE_MIN_DIM_RATIO+0.5);
        axisTitleDispXOffset = minWindowDim * VET_V_TITLE_X_OFFSET;
        axisTitleY = worldBotSliderY - VET_V_AXIS_BOTTOM_MARGIN;

        boundTextSize = (int)(minWindowDim*VET_V_BOUND_SIZE_MIN_DIM_RATIO + 0.5);

        markArrowTipY = worldTopSliderY + VET_V_MARK_Y_OFFSET;

        axisMinY = worldBotSliderY - VET_V_AXIS_BOTTOM_MARGIN;
        axisMaxY = worldTopSliderY + VET_V_AXIS_TOP_MARGIN;
        axisMinDispXOffset = minWindowDim * VET_V_AXIS_MIN_X_OFFSET;
        axisMaxDispXOffset = minWindowDim * VET_V_AXIS_MAX_X_OFFSET;

        extentMinY = axisMinY;
        extentMaxY = axisMaxY;
        extentMinDispXOffset = minWindowDim * VET_V_SLIDER_MIN_X_OFFSET;
        extentMaxDispXOffset = minWindowDim * VET_V_SLIDER_MAX_X_OFFSET;
    }
    else
    {
        axisTitleTextSize = (int)(minWindowDim*VET_H_TITLE_SIZE_MIN_DIM_RATIO+0.5);
        axisTitleY = worldBotSliderY + windowHToWRatio*VET_H_TITLE_Y_OFFSET;

        boundTextSize = (int)(minWindowDim*VET_H_BOUND_SIZE_MIN_DIM_RATIO + 0.5);
        
        markArrowTipY = worldTopSliderY + windowHToWRatio*VET_H_MARK_Y_OFFSET;

        axisMinY = worldBotSliderY + windowHToWRatio*VET_H_AXIS_MIN_Y_OFFSET;
        axisMaxY = worldTopSliderY + windowHToWRatio*VET_H_AXIS_MAX_Y_OFFSET;

        extentMinY = worldBotSliderY + windowHToWRatio*VET_H_SLIDER_MIN_Y_OFFSET;
        extentMaxY = worldTopSliderY + windowHToWRatio*VET_H_SLIDER_MAX_Y_OFFSET;
    }

    double aePixelHeight = (double)pixelHeight * (extentMaxY - worldTopSliderY);
    double idealDotsPerAxis =
        aePixelHeight/(double)VET_PIXELS_PER_A_E_DOT_INTERVAL - 1.0;

    aeDotsPerAxis = (int)(idealDotsPerAxis + 0.5);
    
    if (aeDotsPerAxis < VET_MIN_AXIS_EXTENSION_DOTS)
        aeDotsPerAxis = VET_MIN_AXIS_EXTENSION_DOTS;
    else if (aeDotsPerAxis > VET_MAX_AXIS_EXTENSION_DOTS)
        aeDotsPerAxis = VET_MAX_AXIS_EXTENSION_DOTS;
        
    aeDotInterval =
        aePixelHeight / ((double)pixelHeight * (double)(aeDotsPerAxis+1));
    aeBottomDotY = worldTopSliderY + aeDotInterval;
}


// ****************************************************************************
// Method: VisitExtentsTool::InitializeAllHotpoints
//
// Purpose: Initializes all hotpoints used in the Extents tool according to
//          current values of the associated ExtentsAttributes.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::InitializeAllHotpoints()
{
    hotPoints.clear();

    InitializeButtonHotpoints();
    InitializeMarkSliderHotpoints();
    InitializeAxisSliderHotpoints();
}


// ****************************************************************************
// Method: VisitExtentsTool::InitializeButtonHotpoints
//
// Purpose: Initializes hotpoints for all buttons used in the Extents tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::InitializeButtonHotpoints()
{
    buttonYs[VET_LEFT_MOVE_LEFT_BUTTON_ID] =
        windowHToWRatio * VET_LEFT_MOVE_LEFT_BUTTON_Y;
    buttonYs[VET_RIGHT_MOVE_LEFT_BUTTON_ID] =
        windowHToWRatio * VET_RIGHT_MOVE_LEFT_BUTTON_Y;
    buttonYs[VET_LEFT_MOVE_RIGHT_BUTTON_ID] =
        windowHToWRatio * VET_LEFT_MOVE_RIGHT_BUTTON_Y;
    buttonYs[VET_RIGHT_MOVE_RIGHT_BUTTON_ID] =
        windowHToWRatio * VET_RIGHT_MOVE_RIGHT_BUTTON_Y;
    buttonYs[VET_LEFT_MARK_INFO_ON_BUTTON_ID] =
        windowHToWRatio * VET_LEFT_MARK_INFO_ON_BUTTON_Y;
    buttonYs[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] =
        windowHToWRatio * VET_RIGHT_MARK_INFO_ON_BUTTON_Y;
    buttonYs[VET_EXPAND_BETWEEN_BUTTON_ID] =
        windowHToWRatio * VET_EXPAND_BETWEEN_BUTTON_Y;
    buttonYs[VET_UNDO_LAST_EXPANSION_BUTTON_ID] =
        windowHToWRatio * VET_UNDO_LAST_EXPANSION_BUTTON_Y;
    buttonYs[VET_THRESHOLD_BETWEEN_BUTTON_ID] =
        windowHToWRatio * VET_THRESHOLD_BETWEEN_BUTTON_Y;
    buttonYs[VET_SHOW_TITLES_ONLY_BUTTON_ID] =
        windowHToWRatio * VET_SHOW_TITLES_ONLY_BUTTON_Y;
    
    buttonLabelYs[VET_MOVE_LEFT_MARK_B_LAB_ID] =
        windowHToWRatio * VET_MOVE_LEFT_MARK_B_LABEL_Y;
    buttonLabelYs[VET_MOVE_RIGHT_MARK_B_LAB_ID] =
        windowHToWRatio * VET_MOVE_RIGHT_MARK_B_LABEL_Y;
    buttonLabelYs[VET_LEFT_MARK_INFO_ON_B_LAB_ID] =
        windowHToWRatio * VET_LEFT_MARK_INFO_ON_B_LABEL_Y;
    buttonLabelYs[VET_RIGHT_MARK_INFO_ON_B_LAB_ID] =
        windowHToWRatio * VET_RIGHT_MARK_INFO_ON_B_LABEL_Y;
    buttonLabelYs[VET_EXPAND_BETWEEN_B_LAB_ID] =
        windowHToWRatio * VET_EXPAND_BETWEEN_B_LABEL_Y;
    buttonLabelYs[VET_UNDO_LAST_EXPANSION_B_LAB_ID] =
        windowHToWRatio * VET_UNDO_LAST_EXPANSION_B_LABEL_Y;
    buttonLabelYs[VET_THRESHOLD_BETWEEN_B_LAB_ID] =
        windowHToWRatio * VET_THRESHOLD_BETWEEN_B_LABEL_Y;
    buttonLabelYs[VET_SHOW_TITLES_ONLY_B_LAB_ID] =
        windowHToWRatio * VET_SHOW_TITLES_ONLY_B_LABEL_Y;
    
    HotPoint hotPoint;

    hotPoint.pt.z   = 0.0;
    hotPoint.radius = bqHotpointRadius;
    hotPoint.tool   = this;

    bqHotpointStartID = hotPoints.size();

    for (int buttonID = 0; buttonID < VET_MAX_BUTTONS; buttonID++)
    {
        if (buttonXs[buttonID] == VET_NOT_A_BUTTON_X) break;

        hotPoint.pt.x = buttonXs[buttonID];
        hotPoint.pt.y = buttonYs[buttonID];

        hotPoints.push_back(hotPoint);
    }

    hotPoints[VET_LEFT_MOVE_LEFT_BUTTON_ID].callback =
        MoveLeftMarkOneAxisLeftButtonCallback;
    hotPoints[VET_RIGHT_MOVE_LEFT_BUTTON_ID].callback =
        MoveRightMarkOneAxisLeftButtonCallback;
    hotPoints[VET_LEFT_MOVE_RIGHT_BUTTON_ID].callback =
        MoveLeftMarkOneAxisRightButtonCallback;
    hotPoints[VET_RIGHT_MOVE_RIGHT_BUTTON_ID].callback =
        MoveRightMarkOneAxisRightButtonCallback;
    hotPoints[VET_LEFT_MARK_INFO_ON_BUTTON_ID].callback =
        ShowAxisInfoAtLeftMarkIfOnButtonCallback;
    hotPoints[VET_RIGHT_MARK_INFO_ON_BUTTON_ID].callback =
        ShowAxisInfoAtRightMarkIfOnButtonCallback;
    hotPoints[VET_EXPAND_BETWEEN_BUTTON_ID].callback =
        ExpandBetweenMarkedAxesButtonCallback;
    hotPoints[VET_UNDO_LAST_EXPANSION_BUTTON_ID].callback =
        UndoLastAxisSectionExpansionButtonCallback;
    hotPoints[VET_THRESHOLD_BETWEEN_BUTTON_ID].callback =
        ThresholdBetweenMarkedAxesOnlyIfOnButtonCallback;
    hotPoints[VET_SHOW_TITLES_ONLY_BUTTON_ID].callback =
        ShowAxisTitlesOnlyIfOnButtonCallback;

    bqHotpointStopID = hotPoints.size();
}


// ****************************************************************************
// Method: VisitExtentsTool::InitializeMarkSliderHotpoints
//
// Purpose: Initializes hotpoints for the two axis selector mark slider arrows
//          used in the Extents tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::InitializeMarkSliderHotpoints()
{
    doubleVector axisXPositions = Interface.GetAxisXPositions();

    HotPoint hotPoint;

    hotPoint.pt.z   = 0.0;
    hotPoint.radius = msHotpointRadius;
    hotPoint.tool   = this;

    msHotpointStartID = hotPoints.size();
    
    hotPoint.pt.x = axisXPositions[leftSelectedAxisID]*markSliderXRange +
                    leftAxisX - msHotpointRadius;
    hotPoint.pt.y = markArrowTipY;
    hotPoint.callback = LeftAxisSelectionMarkCallback;
    hotPoints.push_back(hotPoint);

    hotPoint.pt.x = axisXPositions[rightSelectedAxisID]*markSliderXRange +
                    leftAxisX + msHotpointRadius;
    hotPoint.callback = RightAxisSelectionMarkCallback;
    hotPoints.push_back(hotPoint);

    msHotpointStopID = hotPoints.size();
}


// ****************************************************************************
// Method: VisitExtentsTool::InitializeAxisSliderHotpoints
//
// Purpose: Initializes hotpoints for axis slider arrows of the Extents tool
//          according to current values of the associated ExtentsAttributes.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Wed Nov  8 16:01:27 PST 2006
//    Transplanted some of this method's functionality to GetCurrentPlotAttributes.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Create axis slider hotpoints for shown axes only.
//
// ****************************************************************************

void VisitExtentsTool::InitializeAxisSliderHotpoints()
{
    intVector    axisInfoFlagSets = Interface.GetAxisInfoFlagSets();
    doubleVector axisXPositions   = Interface.GetAxisXPositions();
    doubleVector extentMinima     = Interface.GetMinima();
    doubleVector extentMaxima     = Interface.GetMaxima();
    
    asHotpointAxisIDs.clear(); asAxisHotpointIDs.clear();
    
    int hotpointPairID = 0;
    bool hotpointPairExists;

    HotPoint hotPoint;

    hotPoint.pt.z   = 0.0;
    hotPoint.radius = asHotpointRadius;
    hotPoint.tool   = this;

    asHotpointStartID = hotPoints.size();

    for (int extentNum = 0; extentNum < parallelAxisCount; extentNum++)
    {
        hotpointPairExists = true;

        if (axisXPositions[extentNum] < 0.0)
            hotpointPairExists = false;
        else if (axisXPositions[extentNum] > 1.0)
            hotpointPairExists = false;
        else if ((axisInfoFlagSets[extentNum] & EA_AXIS_INFO_SHOWN_FLAG) == 0)
            hotpointPairExists = false;

        if (!hotpointPairExists)
        {
            asAxisHotpointIDs.push_back(-1);
            continue;
        }

        hotPoint.pt.x = axisXPositions[extentNum]*markSliderXRange + leftAxisX;
        hotPoint.pt.y = minSlidableY + extentMinima[extentNum]*normToWorldYScale;
        hotPoint.callback = AxisSliderMinimumCallback;
        hotPoints.push_back(hotPoint);
        
        hotPoint.pt.y =
            maxSlidableY - (1.0-extentMaxima[extentNum])*normToWorldYScale;
        hotPoint.callback = AxisSliderMaximumCallback;
        hotPoints.push_back(hotPoint);
        
        asHotpointAxisIDs.push_back(extentNum);
        asAxisHotpointIDs.push_back(hotpointPairID); hotpointPairID++;
    }

    asHotpointStopID = hotPoints.size();
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
//    Added support for selective axis information in associated plot.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now updates plot/tool mode flags.
//
// ****************************************************************************

void VisitExtentsTool::UpdateToolAttributesWithPlotAttributes()
{
    const PlotInfoAttributes *parAxisAtts = proxy.GetPlotInfoAtts("ParallelAxis");
    
    if (parAxisAtts == NULL) return;

    doubleVector axisAttVals = parAxisAtts->GetOutputArray();

    if (axisAttVals.size() < 6)
    {
        debug3 << "VET/UTAWPA/1: Truncated axis attribute data received from plot."
               << endl;
        return;
    }
    
    stringVector newAxisNames;
    doubleVector newAxisMinima;
    doubleVector newAxisMaxima;
    doubleVector newExtentMinima;
    doubleVector newExtentMaxima;
    intVector    newMinTimeOrds;
    intVector    newMaxTimeOrds;
    stringVector newGroupNames;
    intVector    newInfoFlagSets;
    doubleVector newXPositions;
    
    int valCount = axisAttVals.size();
    int valIndex = 5;
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
        newExtentMinima.push_back(axisAttVals[valIndex++]);
        newExtentMaxima.push_back(axisAttVals[valIndex++]);
        newMinTimeOrds.push_back((int)axisAttVals[valIndex++]);
        newMaxTimeOrds.push_back((int)axisAttVals[valIndex++]);
        newGroupNames.push_back(newGroupName);
        newInfoFlagSets.push_back((int)axisAttVals[valIndex++]);
        newXPositions.push_back(axisAttVals[valIndex++]);
    }

    Interface.SetScalarNames(newAxisNames);
    Interface.SetScalarMinima(newAxisMinima);
    Interface.SetScalarMaxima(newAxisMaxima);
    Interface.SetMinima(newExtentMinima);
    Interface.SetMaxima(newExtentMaxima);
    Interface.SetMinTimeOrdinals(newMinTimeOrds);
    Interface.SetMaxTimeOrdinals(newMaxTimeOrds);
    Interface.SetAxisGroupNames(newGroupNames);
    Interface.SetAxisInfoFlagSets(newInfoFlagSets);
    Interface.SetAxisXPositions(newXPositions);

    Interface.SetLeftSliderX(axisAttVals[0]);
    Interface.SetRightSliderX(axisAttVals[1]);
    Interface.SetSlidersBottomY(axisAttVals[2]);
    Interface.SetSlidersTopY(axisAttVals[3]);
    
    Interface.SetPlotToolModeFlags((int)axisAttVals[4]);
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
//    Signals the plot that the tool will now draw all axis information.
//
// ****************************************************************************

void VisitExtentsTool::Enable()
{
    bool toolWasEnabled = IsEnabled();

    Interface.SetPlotToolModeFlags(
        Interface.GetPlotToolModeFlags() | EA_TOOL_DRAWS_AXIS_INFO_FLAG);
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
//    Signals the plot that it must now draw all axis information.
//
// ****************************************************************************

void VisitExtentsTool::Disable()
{
    bool toolWasEnabled = IsEnabled();

    Interface.SetPlotToolModeFlags(Interface.GetPlotToolModeFlags() &
        (0xffffffff ^ EA_TOOL_DRAWS_AXIS_INFO_FLAG));
    Interface.ExecuteCallback();

    VisitInteractiveTool::Disable();

    if (toolWasEnabled) RemoveAllActors();
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

/* debug 112906
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
*/


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
    RemoveAllActors();

    GetCurrentPlotAttributes();
    PositionAxisSequenceMarks();
    InitializeAllHotpoints();
    
    CreateAllActors();
    AddAllActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::PositionAxisSequenceMarks
//
// Purpose: Sets positions of the Extents tool's left and right axis selection
//          marks if they are not already in valid positions.  Also repositions
//          the tool's left and right "shown axis" marks (which indicate the
//          entire axis sequence to be plotted) if necessary.
//
// Programmer: Mark Blair
// Creation:   Mon Dec 18 14:26:42 PST 2006
//
// Modifications:
//
//    Mark Blair, Wed Jan 17 16:40:37 PST 2007
//    Now repositions "shown axis" marks if necessary.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Records diagnostic information in debug log if marked axes abnormally
//    positioned.
//   
// ****************************************************************************

void VisitExtentsTool::PositionAxisSequenceMarks()
{
    int axisID, axisInfoFlags;
    int plotToolModeFlags = Interface.GetPlotToolModeFlags();
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();
    
    leftShownAxisID    = -1; rightShownAxisID    = -1;
    leftSelectedAxisID = -1; rightSelectedAxisID = -1;

    for (axisID = 0; axisID < parallelAxisCount; axisID++)
    {
        axisInfoFlags = axisInfoFlagSets[axisID];

        if ((axisInfoFlags & EA_LEFT_SHOWN_AXIS_FLAG) != 0)
            leftShownAxisID = axisID;

        if ((axisInfoFlags & EA_RIGHT_SHOWN_AXIS_FLAG) != 0)
            rightShownAxisID = axisID;

        if ((axisInfoFlags & EA_LEFT_SELECTED_AXIS_FLAG) != 0)
            leftSelectedAxisID = axisID;

        if ((axisInfoFlags & EA_RIGHT_SELECTED_AXIS_FLAG) != 0)
            rightSelectedAxisID = axisID;
    }
    
    if ((Interface.GetPlotToolModeFlags() & EA_SHOW_MARKED_AXES_ONLY_FLAG) == 0)
    {
        leftShownAxisID = 0; rightShownAxisID = parallelAxisCount - 1;
    }
    else
    {
        if (leftShownAxisID  == -1) leftShownAxisID  = 0;
        if (rightShownAxisID == -1) rightShownAxisID = parallelAxisCount - 1;
        
        if (leftShownAxisID == rightShownAxisID)
        {
            if (leftShownAxisID > 0)
                leftShownAxisID--;
            else
                rightShownAxisID++;
        }
        else if (leftShownAxisID > rightShownAxisID)
        {
            axisID           = leftShownAxisID;
            leftShownAxisID  = rightShownAxisID;
            rightShownAxisID = axisID;
        }
    }
    
    bool axisMarkError = false;
    
    if (leftSelectedAxisID == -1)
    {
        axisMarkError = true;
        leftSelectedAxisID = leftShownAxisID;
    }

    if (rightSelectedAxisID == -1)
    {
        axisMarkError = true;
        rightSelectedAxisID = rightShownAxisID;
    }
    
    if (leftSelectedAxisID < leftShownAxisID)
    {
        axisMarkError = true;
        leftSelectedAxisID  = leftShownAxisID;
    }

    if (rightSelectedAxisID > rightShownAxisID)
    {
        axisMarkError = true;
        rightSelectedAxisID = rightShownAxisID;
    }
        
    if (leftSelectedAxisID == rightSelectedAxisID)
    {
        axisMarkError = true;

        if (leftSelectedAxisID > leftShownAxisID)
            leftSelectedAxisID--;
        else
            rightSelectedAxisID++;
    }
    else if (leftSelectedAxisID > rightSelectedAxisID)
    {
        axisMarkError = true;

        axisID              = leftSelectedAxisID;
        leftSelectedAxisID  = rightSelectedAxisID;
        rightSelectedAxisID = axisID;
    }
    
    if (axisMarkError)
        debug3 << "VET/PASM/1: Shown/selected axes abnormally positioned." << endl;

    for (axisID = 0; axisID < parallelAxisCount; axisID++)
    {
        axisInfoFlags =
            axisInfoFlagSets[axisID] & (0xffffffff ^ EA_AXIS_SEQUENCE_FLAGS);
            
        if (axisID == leftShownAxisID)
            axisInfoFlags |= EA_LEFT_SHOWN_AXIS_FLAG;
        else if (axisID == rightShownAxisID)
            axisInfoFlags |= EA_RIGHT_SHOWN_AXIS_FLAG;
            
        if (axisID == leftSelectedAxisID)
            axisInfoFlags |= EA_LEFT_SELECTED_AXIS_FLAG;
        else if (axisID == rightSelectedAxisID)
            axisInfoFlags |= EA_RIGHT_SELECTED_AXIS_FLAG;
            
        axisInfoFlagSets[axisID] = axisInfoFlags;
    }
    
    buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] &=
        (0xffffffff ^ VET_BUTTON_TOGGLE_IS_ON);
    buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] &=
        (0xffffffff ^ VET_BUTTON_TOGGLE_IS_ON);

    if ((axisInfoFlagSets[leftSelectedAxisID] & EA_AXIS_INFO_SHOWN_FLAG) != 0)
        buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] |= VET_BUTTON_TOGGLE_IS_ON;
    
    if ((axisInfoFlagSets[rightSelectedAxisID] & EA_AXIS_INFO_SHOWN_FLAG) != 0)
        buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] |= VET_BUTTON_TOGGLE_IS_ON;
    
    Interface.SetAxisInfoFlagSets(axisInfoFlagSets);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAllActors
//
// Purpose: Creates all actors and mappers used in the Extents tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now creates actors that support all varaible axis spacing and axis group
//    conventions.
//   
// ****************************************************************************

void VisitExtentsTool::CreateAllActors()
{
    CreateButtonQuadsActor();
    CreateButtonLogosActor();
    CreateButtonChecksActor();
    CreateButtonLabelsActors();
    
    CreateAxisExtensionsActor();
    
    CreateMarkSlidersActor();
    CreateMarkTitlesActors();
    
    CreateAxisSlidersActor();
    CreateAxisTitlesActors();
    CreateAxisBoundsActors();
    CreateAxisExtentsActors();
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateButtonQuadsActor
//
// Purpose: Creates the actor and mapper for the 5 quadrilaterals of each of
//          the Extents tool's buttons.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateButtonQuadsActor()
{
    DeleteButtonQuadsActor();

    buttonQuadsActor = vtkActor::New();
    buttonQuadsMapper = vtkPolyDataMapper::New();
    buttonQuadsData = vtkPolyData::New();

    buttonQuadsPoints = vtkPoints::New();
    vtkCellArray *buttonQuads = vtkCellArray::New();
    buttonQuadsColors = vtkUnsignedCharArray::New();

    int numButtons = bqHotpointStopID - bqHotpointStartID;
    int numQuads   = numButtons * 5;
    int numPoints  = numButtons * 8;

    buttonQuadsPoints->SetNumberOfPoints(numPoints);
    buttonQuads->Allocate(buttonQuads->EstimateSize(numQuads, 4));
    buttonQuadsColors->SetNumberOfComponents(3);
    buttonQuadsColors->SetNumberOfTuples(numQuads);

    buttonQuadsData->Initialize();
    buttonQuadsData->SetPoints(buttonQuadsPoints);
    buttonQuadsData->SetPolys(buttonQuads);
    buttonQuadsData->GetCellData()->SetScalars(buttonQuadsColors);

    buttonQuadsPoints->Delete();
    buttonQuads->Delete();
    buttonQuadsColors->Delete();
    
    unsigned char *quadRGB = buttonQuadsColors->GetPointer(0);
    
    int buttonID, hotpointID;
    vtkIdType pointID;
    vtkIdType innerSWpointID, innerSEpointID, innerNWpointID, innerNEpointID;
    vtkIdType outerSWpointID, outerSEpointID, outerNWpointID, outerNEpointID;
    double hotpointX, hotpointY;

    vtkIdType pointIDs[4];
    double cornerCoords[3];
    cornerCoords[2] = 0.0;
    
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();

    buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] &= 
        (0xffffffff ^ VET_BUTTON_TOGGLE_IS_ON);
    buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] &= 
        (0xffffffff ^ VET_BUTTON_TOGGLE_IS_ON);

    if ((axisInfoFlagSets[leftSelectedAxisID] & EA_AXIS_INFO_SHOWN_FLAG) != 0)
        buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] |= VET_BUTTON_TOGGLE_IS_ON;
    if ((axisInfoFlagSets[rightSelectedAxisID] & EA_AXIS_INFO_SHOWN_FLAG) != 0)
        buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] |= VET_BUTTON_TOGGLE_IS_ON;

    for (buttonID = 0, pointID = 0; buttonID < numButtons; buttonID++)
    {
        hotpointID = buttonID + bqHotpointStartID;

        hotpointX = hotPoints[hotpointID].pt.x;
        hotpointY = hotPoints[hotpointID].pt.y + buttonHotpointYOffset;

        cornerCoords[0] = hotpointX - buttonInnerRadius;
        cornerCoords[1] = hotpointY - buttonInnerRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        innerSWpointID = pointID++;

        cornerCoords[1] = hotpointY + buttonInnerRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        innerNWpointID = pointID++;

        cornerCoords[0] = hotpointX + buttonInnerRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        innerNEpointID = pointID++;

        cornerCoords[1] = hotpointY - buttonInnerRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        innerSEpointID = pointID++;

        cornerCoords[0] = hotpointX - buttonOuterRadius;
        cornerCoords[1] = hotpointY - buttonOuterRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        outerSWpointID = pointID++;

        cornerCoords[1] = hotpointY + buttonOuterRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        outerNWpointID = pointID++;

        cornerCoords[0] = hotpointX + buttonOuterRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        outerNEpointID = pointID++;

        cornerCoords[1] = hotpointY - buttonOuterRadius;
        buttonQuadsPoints->SetPoint(pointID, cornerCoords);
        outerSEpointID = pointID++;

        pointIDs[0] = innerSWpointID;
        pointIDs[1] = innerNWpointID;
        pointIDs[2] = innerNEpointID;
        pointIDs[3] = innerSEpointID;
        buttonQuads->InsertNextCell(4, pointIDs);
        
        pointIDs[2] = outerNWpointID;
        pointIDs[3] = outerSWpointID;
        buttonQuads->InsertNextCell(4, pointIDs);
        
        pointIDs[0] = innerNEpointID;
        pointIDs[3] = outerNEpointID;
        buttonQuads->InsertNextCell(4, pointIDs);
        
        pointIDs[1] = innerSEpointID;
        pointIDs[2] = outerSEpointID;
        buttonQuads->InsertNextCell(4, pointIDs);
        
        pointIDs[0] = innerSWpointID;
        pointIDs[3] = outerSWpointID;
        buttonQuads->InsertNextCell(4, pointIDs);
        
        if ((buttonFlags[buttonID] & VET_BUTTON_HAS_CHECK_MARK) != 0)
        {
            *quadRGB++ = buttonColor1Red;
            *quadRGB++ = buttonColor1Green;
            *quadRGB++ = buttonColor1Blue;

            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;
            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;

            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;
            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;
        }
        else    // Currently all toggle buttons use check marks and no others do.
        {
            *quadRGB++ = buttonColor1Red;
            *quadRGB++ = buttonColor1Green;
            *quadRGB++ = buttonColor1Blue;

            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;
            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;

            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;
            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;
        }
    }

    buttonQuadsMapper->SetInput(buttonQuadsData);
    buttonQuadsActor->SetMapper(buttonQuadsMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateButtonLogosActor
//
// Purpose: Creates the actor and mapper for logos appearing on some of the
//          Extents tool's buttons.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateButtonLogosActor()
{
    DeleteButtonLogosActor();

    buttonLogosActor = vtkActor::New();
    buttonLogosMapper = vtkPolyDataMapper::New();
    buttonLogosData = vtkPolyData::New();

    buttonLogosPoints = vtkPoints::New();
    vtkCellArray *arrowTriangles = vtkCellArray::New();
    vtkUnsignedCharArray *arrowColors = vtkUnsignedCharArray::New();
    
    int buttonID, numArrows;
    
    buttonLogoIDs.clear();
    
    for (buttonID = 0, numArrows = 0; buttonID < VET_MAX_BUTTONS; buttonID++)
    {
        if (buttonXs[buttonID] == VET_NOT_A_BUTTON_X) break;

        if ((buttonFlags[buttonID] & VET_BUTTON_HAS_ARROW_LOGO) == 0)
            buttonLogoIDs.push_back(-1);
        else
        {
            buttonLogoIDs.push_back(numArrows);
            numArrows++;
        }
    }

    int numPoints = numArrows * 3;

    buttonLogosPoints->SetNumberOfPoints(numPoints);
    arrowTriangles->Allocate(arrowTriangles->EstimateSize(numArrows, 3));
    arrowColors->SetNumberOfComponents(3);
    arrowColors->SetNumberOfTuples(numArrows);

    buttonLogosData->Initialize();
    buttonLogosData->SetPoints(buttonLogosPoints);
    buttonLogosData->SetPolys(arrowTriangles);
    buttonLogosData->GetCellData()->SetScalars(arrowColors);

    buttonLogosPoints->Delete(); arrowTriangles->Delete(); arrowColors->Delete();

    unsigned char *arrowRGB = arrowColors->GetPointer(0);

    vtkIdType pointID;
    bool itIsLeft;
    double buttonX, buttonY;

    vtkIdType pointIDs[3];
    double arrowCoords[3];
    arrowCoords[2] = 0.0;

    for (buttonID = 0, pointID = 0; buttonID < VET_MAX_BUTTONS; buttonID++)
    {
        if (buttonXs[buttonID] == VET_NOT_A_BUTTON_X) break;
        if ((buttonFlags[buttonID] & VET_BUTTON_HAS_ARROW_LOGO) == 0) continue;
        
        itIsLeft = ((buttonFlags[buttonID] & VET_BUTTON_HAS_LEFT_ARROW_LOGO) != 0);

        buttonX = buttonXs[buttonID];
        buttonY = buttonYs[buttonID] + buttonHotpointYOffset;
        
        arrowCoords[0] =
            itIsLeft ? buttonX - buttonLogoRadius : buttonX + buttonLogoRadius;
        arrowCoords[1] = buttonY;
        buttonLogosPoints->SetPoint(pointID, arrowCoords);
        pointIDs[0] = pointID++;
        
        arrowCoords[0] =
            itIsLeft ? buttonX + buttonLogoRadius : buttonX - buttonLogoRadius;
        arrowCoords[1] = buttonY - buttonLogoRadius;
        buttonLogosPoints->SetPoint(pointID, arrowCoords);
        pointIDs[1] = pointID++;
        
        arrowCoords[1] = buttonY + buttonLogoRadius;
        buttonLogosPoints->SetPoint(pointID, arrowCoords);
        pointIDs[2] = pointID++;

        arrowTriangles->InsertNextCell(3, pointIDs);
        
        *arrowRGB++ = buttonLogoRed;
        *arrowRGB++ = buttonLogoGreen;
        *arrowRGB++ = buttonLogoBlue;
    }

    buttonLogosMapper->SetInput(buttonLogosData);
    buttonLogosActor->SetMapper(buttonLogosMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateButtonChecksActor
//
// Purpose: Creates the actor and mapper for check marks that appear and
//          disappear in the Extent tool's toggle buttons.
//
// Programmer: Mark Blair
// Creation:   Tue Jan 23 19:13:03 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateButtonChecksActor()
{
    DeleteButtonChecksActor();

    buttonChecksActor = vtkActor::New();
    buttonChecksMapper = vtkPolyDataMapper::New();
    buttonChecksData = vtkPolyData::New();

    buttonChecksPoints = vtkPoints::New();
    vtkCellArray *checkMarkBars = vtkCellArray::New();
    buttonChecksColors = vtkUnsignedCharArray::New();
    
    int buttonID, numChecks;
    
    buttonCheckIDs.clear();
    
    for (buttonID = 0, numChecks = 0; buttonID < VET_MAX_BUTTONS; buttonID++)
    {
        if (buttonXs[buttonID] == VET_NOT_A_BUTTON_X) break;

        if ((buttonFlags[buttonID] & VET_BUTTON_HAS_CHECK_MARK) == 0)
            buttonCheckIDs.push_back(-1);
        else
        {
            buttonCheckIDs.push_back(numChecks);
            numChecks++;
        }
    }

    int numPoints = numChecks * 8;
    int numBars = numChecks * 2;

    buttonChecksPoints->SetNumberOfPoints(numPoints);
    checkMarkBars->Allocate(checkMarkBars->EstimateSize(numBars, 4));
    buttonChecksColors->SetNumberOfComponents(3);
    buttonChecksColors->SetNumberOfTuples(numBars);

    buttonChecksData->Initialize();
    buttonChecksData->SetPoints(buttonChecksPoints);
    buttonChecksData->SetPolys(checkMarkBars);
    buttonChecksData->GetCellData()->SetScalars(buttonChecksColors);

    buttonChecksPoints->Delete(); buttonChecksColors->Delete();
    checkMarkBars->Delete();

    unsigned char *checkRGB = buttonChecksColors->GetPointer(0);

    vtkIdType pointID;
    double buttonX, buttonY;

    vtkIdType pointIDs[4];
    double barCoords[3];
    barCoords[2] = 0.0;

    for (buttonID = 0, pointID = 0; buttonID < VET_MAX_BUTTONS; buttonID++)
    {
        if (buttonXs[buttonID] == VET_NOT_A_BUTTON_X) break;
        if ((buttonFlags[buttonID] & VET_BUTTON_HAS_CHECK_MARK) == 0) continue;

        buttonX = buttonXs[buttonID];
        buttonY = buttonYs[buttonID] + buttonHotpointYOffset;

        barCoords[0] = buttonX - buttonCheckRadius;
        barCoords[1] = buttonY - buttonLogoRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[0] = pointID++;

        barCoords[0] = buttonX - buttonLogoRadius;
        barCoords[1] = buttonY - buttonCheckRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[1] = pointID++;

        barCoords[0] = buttonX + buttonCheckRadius;
        barCoords[1] = buttonY + buttonLogoRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[2] = pointID++;

        barCoords[0] = buttonX + buttonLogoRadius;
        barCoords[1] = buttonY + buttonCheckRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[3] = pointID++;

        checkMarkBars->InsertNextCell(4, pointIDs);

        barCoords[0] = buttonX - buttonCheckRadius;
        barCoords[1] = buttonY + buttonLogoRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[0] = pointID++;

        barCoords[0] = buttonX - buttonLogoRadius;
        barCoords[1] = buttonY + buttonCheckRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[1] = pointID++;

        barCoords[0] = buttonX + buttonCheckRadius;
        barCoords[1] = buttonY - buttonLogoRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[2] = pointID++;

        barCoords[0] = buttonX + buttonLogoRadius;
        barCoords[1] = buttonY - buttonCheckRadius;
        buttonChecksPoints->SetPoint(pointID, barCoords);
        pointIDs[3] = pointID++;

        checkMarkBars->InsertNextCell(4, pointIDs);

        if ((buttonFlags[buttonID] & VET_BUTTON_TOGGLE_IS_ON) == 0)
        {
            *checkRGB++ = buttonColor1Red;
            *checkRGB++ = buttonColor1Green;
            *checkRGB++ = buttonColor1Blue;

            *checkRGB++ = buttonColor1Red;
            *checkRGB++ = buttonColor1Green;
            *checkRGB++ = buttonColor1Blue;
        }
        else
        {
            memset((void *)checkRGB, 0, 6);
            checkRGB += 6;
        }
    }

    buttonChecksMapper->SetInput(buttonChecksData);
    buttonChecksActor->SetMapper(buttonChecksMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateButtonLabelsActors
//
// Purpose: Creates text actors for the labels that appear next to buttons
//          or button pairs of the Extents tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateButtonLabelsActors()
{
    vtkTextActor *buttonLabelActor;
    avtVector labelPos;
    double labelColor[3] =
        { VET_BUTTON_LABEL_RED, VET_BUTTON_LABEL_GREEN, VET_BUTTON_LABEL_BLUE };

    DeleteButtonLabelsActors();

    buttonLabelsActors.clear();

    for (int labelID = 0; labelID < VET_MAX_BUTTON_LABELS; labelID++)
    {
        if (buttonLabels[labelID] == NULL) break;

        buttonLabelActor = vtkTextActor::New();
        
        buttonLabelActor->ScaledTextOff();

        buttonLabelActor->GetTextProperty()->SetColor(labelColor);
        buttonLabelActor->GetTextProperty()->SetFontFamilyToTimes();
        buttonLabelActor->GetTextProperty()->BoldOn();
        buttonLabelActor->GetTextProperty()->SetFontSize(buttonLabelTextSize);
        buttonLabelActor->GetTextProperty()->SetOrientation(0.0);
        buttonLabelActor->GetTextProperty()->SetJustification(buttonLabelJusts[labelID]);
        buttonLabelActor->GetTextProperty()->SetVerticalJustificationToCentered();
        
        labelPos = ComputeWorldToDisplay(
                   avtVector(buttonLabelXs[labelID], buttonLabelYs[labelID], 0.0));
        buttonLabelActor->SetPosition(labelPos.x, labelPos.y);

        buttonLabelActor->SetInput(buttonLabels[labelID]);

        buttonLabelsActors.push_back(buttonLabelActor);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisExtensionsActor
//
// Purpose: Creates the actor and mapper for dotted lines that extend the top of
//          each currently labelled axis to the axis maximum and extent maximum
//          bound labels for that axis.
//
// Programmer: Mark Blair
// Creation:   Mon Jan  8 16:31:01 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateAxisExtensionsActor()
{
    DeleteAxisExtensionsActor();

    axisExtensionsActor = vtkActor::New();
    axisExtensionsMapper = vtkPolyDataMapper::New();
    axisExtensionsData = vtkPolyData::New();

    axisDotPoints = vtkPoints::New();
    vtkCellArray *dotPolygons = vtkCellArray::New();
    vtkUnsignedCharArray *dotColors = vtkUnsignedCharArray::New();

    int numExtensions = asHotpointAxisIDs.size();
    int numDots = numExtensions * aeDotsPerAxis;
    int numPoints = numDots * 4;

    axisDotPoints->SetNumberOfPoints(numPoints);
    dotPolygons->Allocate(dotPolygons->EstimateSize(numDots, 4));
    dotColors->SetNumberOfComponents(3);
    dotColors->SetNumberOfTuples(numDots);

    axisExtensionsData->Initialize();
    axisExtensionsData->SetPoints(axisDotPoints);
    axisExtensionsData->SetPolys(dotPolygons);
    axisExtensionsData->GetCellData()->SetScalars(dotColors);

    axisDotPoints->Delete(); dotPolygons->Delete(); dotColors->Delete();

    int extensionNum, axisDotNum;
    vtkIdType pointID = 0;
    double dotCenterX, dotCenterY;

    vtkIdType pointIDs[4];
    double vertexCoords[3];
    vertexCoords[2] = 0.0;

    for (extensionNum = 0; extensionNum < numExtensions; extensionNum++)
    {
        dotCenterX = hotPoints[extensionNum*2+asHotpointStartID].pt.x;
        
        for (axisDotNum = 0; axisDotNum < aeDotsPerAxis; axisDotNum++)
        {
            dotCenterY = aeBottomDotY + (double)axisDotNum*aeDotInterval;

            vertexCoords[0] = dotCenterX;
            vertexCoords[1] = dotCenterY - VET_AXIS_EXTENSION_DOT_RADIUS;
            axisDotPoints->SetPoint(pointID, vertexCoords);
            pointIDs[0] = pointID++;
            
            vertexCoords[0] = dotCenterX + VET_AXIS_EXTENSION_DOT_RADIUS;
            vertexCoords[1] = dotCenterY;
            axisDotPoints->SetPoint(pointID, vertexCoords);
            pointIDs[1] = pointID++;
            
            vertexCoords[0] = dotCenterX;
            vertexCoords[1] = dotCenterY + VET_AXIS_EXTENSION_DOT_RADIUS;
            axisDotPoints->SetPoint(pointID, vertexCoords);
            pointIDs[2] = pointID++;
            
            vertexCoords[0] = dotCenterX - VET_AXIS_EXTENSION_DOT_RADIUS;
            vertexCoords[1] = dotCenterY;
            axisDotPoints->SetPoint(pointID, vertexCoords);
            pointIDs[3] = pointID++;
            
            dotPolygons->InsertNextCell(4, pointIDs);
        }
    }

    unsigned char *dotRGB;

    for (int dotNum = 0; dotNum < numDots; dotNum++)
    {
        dotRGB = dotColors->GetPointer(dotNum*3);

        dotRGB[0] = axisExtensionRed;
        dotRGB[1] = axisExtensionGreen;
        dotRGB[2] = axisExtensionBlue;
    }

    axisExtensionsMapper->SetInput(axisExtensionsData);
    axisExtensionsActor->SetMapper(axisExtensionsMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateMarkSlidersActor
//
// Purpose: Creates the actor and mapper for slider arrowheads of the Extents
//          tool's axis selection marks.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateMarkSlidersActor()
{
    DeleteMarkSlidersActor();

    markSlidersActor = vtkActor::New();
    markSlidersMapper = vtkPolyDataMapper::New();
    markSlidersData = vtkPolyData::New();

    markArrowPoints = vtkPoints::New();
    vtkCellArray *arrowTriangles = vtkCellArray::New();
    vtkUnsignedCharArray *arrowColors = vtkUnsignedCharArray::New();

    markArrowPoints->SetNumberOfPoints(6);
    arrowTriangles->Allocate(arrowTriangles->EstimateSize(2, 3));
    arrowColors->SetNumberOfComponents(3);
    arrowColors->SetNumberOfTuples(2);

    markSlidersData->Initialize();
    markSlidersData->SetPoints(markArrowPoints);
    markSlidersData->SetPolys(arrowTriangles);
    markSlidersData->GetCellData()->SetScalars(arrowColors);

    markArrowPoints->Delete(); arrowTriangles->Delete(); arrowColors->Delete();

    double leftHotpointX  = hotPoints[msHotpointStartID  ].pt.x;
    double rightHotpointX = hotPoints[msHotpointStartID+1].pt.x;
        
    double arrowBottomY  = markArrowTipY - msHotpointRadius;
    double arrowTopY     = markArrowTipY + msHotpointRadius;

    vtkIdType pointIDs[3];
    double vertexCoords[3];
    vertexCoords[2] = 0.0;

    vertexCoords[0] = leftHotpointX;
    vertexCoords[1] = arrowBottomY;
    markArrowPoints->SetPoint(0, vertexCoords);
    pointIDs[0] = 0;

    vertexCoords[1] = arrowTopY;
    markArrowPoints->SetPoint(1, vertexCoords);
    pointIDs[1] = 1;

    vertexCoords[0] = leftHotpointX + msHotpointRadius;
    vertexCoords[1] = markArrowTipY;
    markArrowPoints->SetPoint(2, vertexCoords);
    pointIDs[2] = 2;

    arrowTriangles->InsertNextCell(3, pointIDs);

    vertexCoords[0] = rightHotpointX - msHotpointRadius;
    markArrowPoints->SetPoint(3, vertexCoords);
    pointIDs[0] = 3;

    vertexCoords[0] = rightHotpointX;
    vertexCoords[1] = arrowTopY;
    markArrowPoints->SetPoint(4, vertexCoords);
    pointIDs[1] = 4;

    vertexCoords[1] = arrowBottomY;
    markArrowPoints->SetPoint(5, vertexCoords);
    pointIDs[2] = 5;

    arrowTriangles->InsertNextCell(3, pointIDs);

    unsigned char *arrowRGB = arrowColors->GetPointer(0);

    arrowRGB[0] = markSliderRed;
    arrowRGB[1] = markSliderGreen;
    arrowRGB[2] = markSliderBlue;

    arrowRGB[3] = markSliderRed;
    arrowRGB[4] = markSliderGreen;
    arrowRGB[5] = markSliderBlue;

    markSlidersMapper->SetInput(markSlidersData);
    markSlidersActor->SetMapper(markSlidersMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateMarkTitlesActors
//
// Purpose: Creates a text actor for each of the two axis titles that accompany
//          the two axis selection marks.
//
// Programmer: Mark Blair
// Creation:   Fri Jan  5 15:10:35 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::CreateMarkTitlesActors()
{
    vtkTextActor *markTitleActor;
    avtVector titlePos;
    char markAxisTitle[VET_H_TITLE_MAX_CHARS + 1];
    double titleColor[3] =
        { VET_MARK_TITLE_RED, VET_MARK_TITLE_GREEN, VET_MARK_TITLE_BLUE };
        
    DeleteMarkTitlesActors();
    
    axisTitles = Interface.GetScalarNames();

    markTitlesActors.clear();

    markTitleActor = vtkTextActor::New();
        
    markTitleActor->ScaledTextOff();

    markTitleActor->GetTextProperty()->SetColor(titleColor);
    markTitleActor->GetTextProperty()->SetFontFamilyToArial();
    markTitleActor->GetTextProperty()->BoldOn();
    markTitleActor->GetTextProperty()->SetFontSize(markTitleTextSize);
    markTitleActor->GetTextProperty()->SetOrientation(0.0);
    markTitleActor->GetTextProperty()->SetJustificationToRight();
    markTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
        
    titlePos = ComputeWorldToDisplay(
               avtVector(hotPoints[msHotpointStartID].pt.x, markArrowTipY, 0.0));
    titlePos.x -= markTitleXOffset;
    markTitleActor->SetPosition(titlePos.x, titlePos.y);

    MakeAxisTitleText(
        markAxisTitle, axisTitles[leftSelectedAxisID], VET_H_TITLE_MAX_CHARS);
    markTitleActor->SetInput(markAxisTitle);

    markTitlesActors.push_back(markTitleActor);

    markTitleActor = vtkTextActor::New();
        
    markTitleActor->ScaledTextOff();

    markTitleActor->GetTextProperty()->SetColor(titleColor);
    markTitleActor->GetTextProperty()->SetFontFamilyToArial();
    markTitleActor->GetTextProperty()->BoldOn();
    markTitleActor->GetTextProperty()->SetFontSize(markTitleTextSize);
    markTitleActor->GetTextProperty()->SetOrientation(0.0);
    markTitleActor->GetTextProperty()->SetJustificationToLeft();
    markTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
        
    titlePos = ComputeWorldToDisplay(
               avtVector(hotPoints[msHotpointStartID+1].pt.x, markArrowTipY, 0.0));
    titlePos.x += markTitleXOffset;
    markTitleActor->SetPosition(titlePos.x, titlePos.y);

    MakeAxisTitleText(
        markAxisTitle, axisTitles[rightSelectedAxisID], VET_H_TITLE_MAX_CHARS);
    markTitleActor->SetInput(markAxisTitle);

    markTitlesActors.push_back(markTitleActor);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisSlidersActor
//
// Purpose: Creates the actor and mapper for slider arrowheads on the Extents
//          tool's individual plot axes.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now creates sliders only for those axes with information currently visible.
//   
// ****************************************************************************

void VisitExtentsTool::CreateAxisSlidersActor()
{
    DeleteAxisSlidersActor();

    axisSlidersActor = vtkActor::New();
    axisSlidersMapper = vtkPolyDataMapper::New();
    axisSlidersData = vtkPolyData::New();

    axisArrowPoints = vtkPoints::New();
    vtkCellArray *arrowTriangles = vtkCellArray::New();
    vtkUnsignedCharArray *arrowColors = vtkUnsignedCharArray::New();

    int numArrows = asHotpointStopID - asHotpointStartID;
    int numSliderPairs = numArrows / 2;
    int numPoints = numArrows * 3;

    axisArrowPoints->SetNumberOfPoints(numPoints);
    arrowTriangles->Allocate(arrowTriangles->EstimateSize(numArrows, 3));
    arrowColors->SetNumberOfComponents(3);
    arrowColors->SetNumberOfTuples(numArrows);

    axisSlidersData->Initialize();
    axisSlidersData->SetPoints(axisArrowPoints);
    axisSlidersData->SetPolys(arrowTriangles);
    axisSlidersData->GetCellData()->SetScalars(arrowColors);

    axisArrowPoints->Delete(); arrowTriangles->Delete(); arrowColors->Delete();

    int sliderPairNum, minHotPointID;
    vtkIdType pointID = 0;
    double minHotPointX, minHotPointY, maxHotPointY, arrowLeftX, arrowRightX;

    vtkIdType pointIDs[3];
    double vertexCoords[3];
    vertexCoords[2] = 0.0;

    for (sliderPairNum = 0; sliderPairNum < numSliderPairs; sliderPairNum++)
    {
        minHotPointID = sliderPairNum*2 + asHotpointStartID;

        minHotPointX = hotPoints[minHotPointID  ].pt.x;
        minHotPointY = hotPoints[minHotPointID  ].pt.y;
        maxHotPointY = hotPoints[minHotPointID+1].pt.y;
        
        arrowLeftX  = minHotPointX - asHotpointRadius;
        arrowRightX = minHotPointX + asHotpointRadius;

        vertexCoords[0] = arrowLeftX;
        vertexCoords[1] = minHotPointY;
        axisArrowPoints->SetPoint(pointID, vertexCoords);
        pointIDs[0] = pointID++;

        vertexCoords[0] = arrowRightX;
        axisArrowPoints->SetPoint(pointID, vertexCoords);
        pointIDs[1] = pointID++;

        vertexCoords[0] = minHotPointX;
        vertexCoords[1] = minHotPointY + asHotpointRadius;
        axisArrowPoints->SetPoint(pointID, vertexCoords);
        pointIDs[2] = pointID++;

        arrowTriangles->InsertNextCell(3, pointIDs);

        vertexCoords[1] = maxHotPointY - asHotpointRadius;
        axisArrowPoints->SetPoint(pointID, vertexCoords);
        pointIDs[0] = pointID++;

        vertexCoords[0] = arrowLeftX;
        vertexCoords[1] = maxHotPointY;
        axisArrowPoints->SetPoint(pointID, vertexCoords);
        pointIDs[1] = pointID++;

        vertexCoords[0] = arrowRightX;
        axisArrowPoints->SetPoint(pointID, vertexCoords);
        pointIDs[2] = pointID++;

        arrowTriangles->InsertNextCell(3, pointIDs);
    }

    unsigned char *arrowRGB;

    for (int arrowNum = 0; arrowNum < numArrows; arrowNum++)
    {
        arrowRGB = arrowColors->GetPointer(arrowNum*3);

        arrowRGB[0] = axisSliderRed;
        arrowRGB[1] = axisSliderGreen;
        arrowRGB[2] = axisSliderBlue;
    }

    axisSlidersMapper->SetInput(axisSlidersData);
    axisSlidersActor->SetMapper(axisSlidersMapper);
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisTitlesActors
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
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now creates titles only for those axes with information currently visible.
//   
// ****************************************************************************

void VisitExtentsTool::CreateAxisTitlesActors()
{
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();

    avtVector titlePos;
    vtkTextActor *axisTitleActor;
    int titleNum, titleAxisID, minHotpointID;
    double axisTitleX;
    char axisTitle[VET_H_TITLE_MAX_CHARS + 1];
    double titleColor[3] =
        { VET_AXIS_TITLE_RED, VET_AXIS_TITLE_GREEN, VET_AXIS_TITLE_BLUE };

    DeleteAxisTitlesActors();

    axisTitles = Interface.GetScalarNames();    // Accessed in two places.

    axisTitlesActors.clear();

    for (titleNum = 0; titleNum < asHotpointAxisIDs.size(); titleNum++)
    {
        titleAxisID = asHotpointAxisIDs[titleNum];

        if ((axisInfoFlagSets[titleAxisID] & EA_SHOW_AXIS_TITLE_ONLY_FLAG) == 0)
            continue;

        axisTitleActor = vtkTextActor::New();
        
        axisTitleActor->ScaledTextOff();

        axisTitleActor->GetTextProperty()->SetColor(titleColor);
        axisTitleActor->GetTextProperty()->SetFontFamilyToArial();
        axisTitleActor->GetTextProperty()->BoldOn();
        axisTitleActor->GetTextProperty()->SetFontSize(axisTitleTextSize);
        
        minHotpointID = titleNum*2 + asHotpointStartID;

        axisTitleX = hotPoints[minHotpointID].pt.x;
        titlePos = ComputeWorldToDisplay(avtVector(axisTitleX, axisTitleY, 0.0));
        
        if (labelsAreVertical)
        {
            MakeAxisTitleText(
                axisTitle, axisTitles[titleAxisID], VET_V_TITLE_MAX_CHARS);

            titlePos.x += axisTitleDispXOffset;

            axisTitleActor->GetTextProperty()->SetOrientation(90.0);

            axisTitleActor->GetTextProperty()->SetJustificationToRight();
            axisTitleActor->GetTextProperty()->SetVerticalJustificationToBottom();
        }
        else
        {
            MakeAxisTitleText(
                axisTitle, axisTitles[titleAxisID], VET_H_TITLE_MAX_CHARS);

            axisTitleActor->GetTextProperty()->SetOrientation(0.0);

            if (titleAxisID == leftShownAxisID)
                axisTitleActor->GetTextProperty()->SetJustificationToLeft();
            else if (titleAxisID == rightShownAxisID)
                axisTitleActor->GetTextProperty()->SetJustificationToRight();
            else
                axisTitleActor->GetTextProperty()->SetJustificationToCentered();
            
            axisTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
        }

        axisTitleActor->SetInput(axisTitle);
        axisTitleActor->SetPosition(titlePos.x, titlePos.y);
        
        axisTitlesActors.push_back(axisTitleActor);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisBoundsActors
//
// Purpose: Creates a text actor for each axis bound to be displayed at the top
//          and bottom of an axis on the plot.  This bound corresponds to the
//          axis variable's value at that end of the axis.  Ideally each such
//          bound should appear on the plot exactly as it would if the Extents
//          tool were not enabled.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now creates bound labels only for those axes with information currently
//    visible.
//   
// ****************************************************************************

void VisitExtentsTool::CreateAxisBoundsActors()
{
    doubleVector axisMinima    = Interface.GetScalarMinima();
    doubleVector axisMaxima    = Interface.GetScalarMaxima();
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();

    avtVector axisMinPos;
    avtVector axisMaxPos;
    vtkTextActor *axisMinActor, *axisMaxActor;
    int boundNum, boundAxisID, minHotpointID;
    double axisX;
    char boundMinText[VET_H_BOUND_MAX_CHARS + 1];
    char boundMaxText[VET_H_BOUND_MAX_CHARS + 1];
    double axisBoundColor[3] =
        { VET_AXIS_BOUND_RED, VET_AXIS_BOUND_GREEN, VET_AXIS_BOUND_BLUE };

    DeleteAxisBoundsActors();

    axisMinimaActors.clear(); axisMaximaActors.clear();
    
    for (boundNum = 0; boundNum < asHotpointAxisIDs.size(); boundNum++)
    {
        boundAxisID = asHotpointAxisIDs[boundNum];

        if ((axisInfoFlagSets[boundAxisID] & EA_SHOW_AXIS_LIMITS_ONLY_FLAG) == 0)
            continue;

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
        
        minHotpointID = boundNum*2 + asHotpointStartID;

        axisX = hotPoints[minHotpointID].pt.x;

        axisMinPos = ComputeWorldToDisplay(avtVector(axisX, axisMinY, 0.0));
        axisMaxPos = ComputeWorldToDisplay(avtVector(axisX, axisMaxY, 0.0));
        
        if (labelsAreVertical)
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

            if (boundAxisID == leftShownAxisID)
            {
                axisMinActor->GetTextProperty()->SetJustificationToLeft();
                axisMaxActor->GetTextProperty()->SetJustificationToLeft();
            }
            else if (boundAxisID == rightShownAxisID)
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

        MakeDataBoundText(boundMinText, axisMinima[boundAxisID]);
        MakeDataBoundText(boundMaxText, axisMaxima[boundAxisID]);

        axisMinActor->SetInput(boundMinText);
        axisMaxActor->SetInput(boundMaxText);
        
        axisMinimaActors.push_back(axisMinActor);
        axisMaximaActors.push_back(axisMaxActor);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CreateAxisExtentsActors
//
// Purpose: Creates a text actor for each extents bound to be displayed next
//          to the axis bound at one end of an axis on the plot.  This bound
//          denotes the value currently selected by the corresponding extent
//          min or max slider along that axis.
//
// Programmer: Mark Blair
// Creation:   Thu Nov 16 16:11:52 PST 2006
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now creates extents labels only for those axes with information currently
//    visible.
//   
// ****************************************************************************

void VisitExtentsTool::CreateAxisExtentsActors()
{
    doubleVector axisMinima    = Interface.GetScalarMinima();
    doubleVector axisMaxima    = Interface.GetScalarMaxima();
    doubleVector extentMinima  = Interface.GetMinima();
    doubleVector extentMaxima  = Interface.GetMaxima();
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();

    avtVector extentMinPos;
    avtVector extentMaxPos;
    vtkTextActor *extentMinActor, *extentMaxActor;
    int extentNum, extentAxisID, minHotpointID;
    double axisX, axisMinBound, axisRange, extentMinBound, extentMaxBound;
    char extentMinText[VET_H_BOUND_MAX_CHARS + 1];
    char extentMaxText[VET_H_BOUND_MAX_CHARS + 1];
    double axisExtentColor[3] =
        { VET_AXIS_EXTENT_RED, VET_AXIS_EXTENT_GREEN, VET_AXIS_EXTENT_BLUE };

    DeleteAxisExtentsActors();

    extentMinimaActors.clear(); extentMaximaActors.clear();
    
    for (extentNum = 0; extentNum < asHotpointAxisIDs.size(); extentNum++)
    {
        extentAxisID = asHotpointAxisIDs[extentNum];

        if ((axisInfoFlagSets[extentAxisID] & EA_SHOW_SLIDER_LIMITS_ONLY_FLAG) == 0)
            continue;

        extentMinActor = vtkTextActor::New();
        extentMaxActor = vtkTextActor::New();

        extentMinActor->ScaledTextOff();
        extentMinActor->GetTextProperty()->SetColor(axisExtentColor);
        extentMinActor->GetTextProperty()->SetFontFamilyToArial();
        extentMinActor->GetTextProperty()->SetFontSize(boundTextSize);
        extentMinActor->GetTextProperty()->BoldOn();

        extentMaxActor->ScaledTextOff();
        extentMaxActor->GetTextProperty()->SetColor(axisExtentColor);
        extentMaxActor->GetTextProperty()->SetFontFamilyToArial();
        extentMaxActor->GetTextProperty()->SetFontSize(boundTextSize);
        extentMaxActor->GetTextProperty()->BoldOn();
        
        minHotpointID = extentNum*2 + asHotpointStartID;

        axisX = hotPoints[minHotpointID].pt.x;

        extentMinPos = ComputeWorldToDisplay(avtVector(axisX, extentMinY, 0.0));
        extentMaxPos = ComputeWorldToDisplay(avtVector(axisX, extentMaxY, 0.0));
        
        if (labelsAreVertical)
        {
            extentMinPos.x += extentMinDispXOffset;
            extentMaxPos.x += extentMaxDispXOffset;
        
            extentMinActor->GetTextProperty()->SetOrientation(90.0);
            extentMaxActor->GetTextProperty()->SetOrientation(90.0);

            extentMinActor->GetTextProperty()->SetJustificationToRight();
            extentMinActor->GetTextProperty()->SetVerticalJustificationToBottom();

            extentMaxActor->GetTextProperty()->SetJustificationToLeft();
            extentMaxActor->GetTextProperty()->SetVerticalJustificationToBottom();
        }
        else
        {
            extentMinActor->GetTextProperty()->SetOrientation(0.0);
            extentMaxActor->GetTextProperty()->SetOrientation(0.0);

            if (extentAxisID == leftShownAxisID)
            {
                extentMinActor->GetTextProperty()->SetJustificationToLeft();
                extentMaxActor->GetTextProperty()->SetJustificationToLeft();
            }
            else if (extentAxisID == rightShownAxisID)
            {
                extentMinActor->GetTextProperty()->SetJustificationToRight();
                extentMaxActor->GetTextProperty()->SetJustificationToRight();
            }
            else
            {
                extentMinActor->GetTextProperty()->SetJustificationToCentered();
                extentMaxActor->GetTextProperty()->SetJustificationToCentered();
            }
            
            extentMinActor->GetTextProperty()->SetVerticalJustificationToCentered();
            extentMaxActor->GetTextProperty()->SetVerticalJustificationToCentered();
        }
            
        extentMinActor->SetPosition(extentMinPos.x, extentMinPos.y);
        extentMaxActor->SetPosition(extentMaxPos.x, extentMaxPos.y);
        
        axisMinBound = axisMinima[extentAxisID];
        axisRange = axisMaxima[extentAxisID] - axisMinBound;
        extentMinBound = extentMinima[extentAxisID]*axisRange + axisMinBound;
        extentMaxBound = extentMaxima[extentAxisID]*axisRange + axisMinBound;

        MakeDataBoundText(extentMinText, extentMinBound);
        MakeDataBoundText(extentMaxText, extentMaxBound);

        extentMinActor->SetInput(extentMinText);
        extentMaxActor->SetInput(extentMaxText);
        
        extentMinimaActors.push_back(extentMinActor);
        extentMaximaActors.push_back(extentMaxActor);
    }
}


// ****************************************************************************
// Method: VisitPlaneTool::DeleteAllActors
//
// Purpose: Deletes all actors and mappers created by the Extents tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now deletes all actors that support varaible axis spacing and axis group
//    conventions.
//   
// ****************************************************************************

void VisitExtentsTool::DeleteAllActors()
{
    DeleteButtonQuadsActor();
    DeleteButtonLogosActor();
    DeleteButtonChecksActor();
    DeleteButtonLabelsActors();
    
    DeleteAxisExtensionsActor();
    
    DeleteMarkSlidersActor();
    DeleteMarkTitlesActors();
    
    DeleteAxisSlidersActor();
    DeleteAxisTitlesActors();
    DeleteAxisBoundsActors();
    DeleteAxisExtentsActors();

    asHotpointAxisIDs.clear(); asAxisHotpointIDs.clear();
    buttonLogoIDs.clear(); buttonCheckIDs.clear();
    axisTitles.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteButtonQuadsActor
//
// Purpose: Deletes the actor and mapper created for the quadrilaterals of the
//          Extents tool's buttons.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteButtonQuadsActor()
{
    if (buttonQuadsActor != NULL)
    {
        buttonQuadsActor->Delete();
        buttonQuadsActor = NULL;
    }

    if (buttonQuadsMapper != NULL)
    {
        buttonQuadsMapper->Delete();
        buttonQuadsMapper = NULL;
    }

    if (buttonQuadsData != NULL)
    {
        buttonQuadsData->Delete();
        buttonQuadsData = NULL;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteButtonLogosActor
//
// Purpose: Deletes the actor and mapper created for logos that appear on some
//          the Extents tool's buttons.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteButtonLogosActor()
{
    if (buttonLogosActor != NULL)
    {
        buttonLogosActor->Delete();
        buttonLogosActor = NULL;
    }

    if (buttonLogosMapper != NULL)
    {
        buttonLogosMapper->Delete();
        buttonLogosMapper = NULL;
    }

    if (buttonLogosData != NULL)
    {
        buttonLogosData->Delete();
        buttonLogosData = NULL;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteButtonChecksActor
//
// Purpose: Deletes the actor and mapper created for check marks that appear
//          and disappear in the Extent tool's toggle buttons.
//
// Programmer: Mark Blair
// Creation:   Tue Jan 23 19:13:03 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteButtonChecksActor()
{
    if (buttonChecksActor != NULL)
    {
        buttonChecksActor->Delete();
        buttonChecksActor = NULL;
    }

    if (buttonChecksMapper != NULL)
    {
        buttonChecksMapper->Delete();
        buttonChecksMapper = NULL;
    }

    if (buttonChecksData != NULL)
    {
        buttonChecksData->Delete();
        buttonChecksData = NULL;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteButtonLabelsActors
//
// Purpose: Deletes text actors created for the labels that appear next to
//          buttons or button pairs of the Extents tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteButtonLabelsActors()
{
    for (int actorNum = 0; actorNum < buttonLabelsActors.size(); actorNum++)
        buttonLabelsActors[actorNum]->Delete();

    buttonLabelsActors.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteMarkSlidersActor
//
// Purpose: Deletes the actor and mapper created for slider arrowheads of the
//          Extents tool's axis selection marks.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteMarkSlidersActor()
{
    if (markSlidersActor != NULL)
    {
        markSlidersActor->Delete();
        markSlidersActor = NULL;
    }

    if (markSlidersMapper != NULL)
    {
        markSlidersMapper->Delete();
        markSlidersMapper = NULL;
    }

    if (markSlidersData != NULL)
    {
        markSlidersData->Delete();
        markSlidersData = NULL;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteMarkTitlesActors
//
// Purpose: Deletes text actors created for the axis titles that accompany the
//          Extents tool's two axis selection marks.
//
// Programmer: Mark Blair
// Creation:   Fri Jan  5 15:10:35 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteMarkTitlesActors()
{
    if (markTitlesActors.size() == 2)
    {
        markTitlesActors[0]->Delete();
        markTitlesActors[1]->Delete();
    }
    
    markTitlesActors.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteAxisExtensionsActor
//
// Purpose: Deletes the actor and mapper for dotted lines that extend the top of
//          each currently labelled axis to the axis maximum and extent maximum
//          bound labels for that axis.
//
// Programmer: Mark Blair
// Creation:   Mon Jan  8 16:31:01 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteAxisExtensionsActor()
{
    if (axisExtensionsActor != NULL)
    {
        axisExtensionsActor->Delete();
        axisExtensionsActor = NULL;
    }

    if (axisExtensionsMapper != NULL)
    {
        axisExtensionsMapper->Delete();
        axisExtensionsMapper = NULL;
    }

    if (axisExtensionsData != NULL)
    {
        axisExtensionsData->Delete();
        axisExtensionsData = NULL;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteAxisSlidersActor
//
// Purpose: Deletes the actor and mapper created for slider arrowheads on the
//          Extents tool's individual plot axes.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteAxisSlidersActor()
{
    if (axisSlidersActor != NULL)
    {
        axisSlidersActor->Delete();
        axisSlidersActor = NULL;
    }

    if (axisSlidersMapper != NULL)
    {
        axisSlidersMapper->Delete();
        axisSlidersMapper = NULL;
    }

    if (axisSlidersData != NULL)
    {
        axisSlidersData->Delete();
        axisSlidersData = NULL;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteAxisTitlesActors
//
// Purpose: Deletes text actors created for titles accompanying the plot axes.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteAxisTitlesActors()
{
    for (int actorNum = 0; actorNum < axisTitlesActors.size(); actorNum++)
        axisTitlesActors[actorNum]->Delete();

    axisTitlesActors.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteAxisBoundsActors
//
// Purpose: Deletes text actors created for numerical labels corresponding to
//          bounds of the the plot axes.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteAxisBoundsActors()
{
    for (int actorNum = 0; actorNum < axisMinimaActors.size(); actorNum++)
    {
        axisMinimaActors[actorNum]->Delete();
        axisMaximaActors[actorNum]->Delete();
    }

    axisMinimaActors.clear(); axisMaximaActors.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::DeleteAxisExtentsActors
//
// Purpose: Deletes text actors created for numerical labels corresponding to
//          values selected by the Extents tool's axis sldier arrowheads.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::DeleteAxisExtentsActors()
{
    for (int actorNum = 0; actorNum < extentMinimaActors.size(); actorNum++)
    {
        extentMinimaActors[actorNum]->Delete();
        extentMaximaActors[actorNum]->Delete();
    }

    extentMinimaActors.clear(); extentMaximaActors.clear();
}


// ****************************************************************************
// Method: VisitExtentsTool::AddAllActors
//
// Purpose: Adds to the foreground canvas all actors created by the tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now adds all actors that support varaible axis spacing and axis group
//    conventions.
//   
// ****************************************************************************

void VisitExtentsTool::AddAllActors()
{
    int actorNum;

    proxy.GetCanvas()->AddActor(buttonQuadsActor);
    proxy.GetCanvas()->AddActor(buttonLogosActor);
    proxy.GetCanvas()->AddActor(buttonChecksActor);
    proxy.GetCanvas()->AddActor(axisExtensionsActor);
    proxy.GetCanvas()->AddActor(markSlidersActor);
    proxy.GetCanvas()->AddActor(axisSlidersActor);

    for (actorNum = 0; actorNum < buttonLabelsActors.size(); actorNum++)
        proxy.GetForeground()->AddActor2D(buttonLabelsActors[actorNum]);

    if (markTitlesActors.size() == 2)
    {
        proxy.GetForeground()->AddActor2D(markTitlesActors[0]);
        proxy.GetForeground()->AddActor2D(markTitlesActors[1]);
    }

    for (actorNum = 0; actorNum < axisTitlesActors.size(); actorNum++)
        proxy.GetForeground()->AddActor2D(axisTitlesActors[actorNum]);

    for (actorNum = 0; actorNum < axisMinimaActors.size(); actorNum++)
    {
        proxy.GetForeground()->AddActor2D(axisMinimaActors[actorNum]);
        proxy.GetForeground()->AddActor2D(axisMaximaActors[actorNum]);
    }

    for (actorNum = 0; actorNum < extentMinimaActors.size(); actorNum++)
    {
        proxy.GetForeground()->AddActor2D(extentMinimaActors[actorNum]);
        proxy.GetForeground()->AddActor2D(extentMaximaActors[actorNum]);
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::RemoveAllActors
//
// Purpose: Removes from the foreground canvas all actors created by the tool.
//
// Programmer: Mark Blair
// Creation:   Wed Nov 29 20:08:58 PST 2006
//
// Modifications:
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now removes all actors that support varaible axis spacing and axis group
//    conventions.
//   
// ****************************************************************************

void VisitExtentsTool::RemoveAllActors()
{
    int actorNum;

    if (buttonQuadsActor != NULL)
        proxy.GetCanvas()->RemoveActor(buttonQuadsActor);

    if (buttonLogosActor != NULL)
        proxy.GetCanvas()->RemoveActor(buttonLogosActor);

    if (buttonChecksActor != NULL)
        proxy.GetCanvas()->RemoveActor(buttonChecksActor);

    if (axisExtensionsActor != NULL)
        proxy.GetCanvas()->RemoveActor(axisExtensionsActor);

    if (markSlidersActor != NULL)
        proxy.GetCanvas()->RemoveActor(markSlidersActor);

    if (axisSlidersActor != NULL)
        proxy.GetCanvas()->RemoveActor(axisSlidersActor);

    for (actorNum = 0; actorNum < buttonLabelsActors.size(); actorNum++)
        proxy.GetForeground()->RemoveActor2D(buttonLabelsActors[actorNum]);

    if (markTitlesActors.size() == 2)
    {
        proxy.GetForeground()->RemoveActor2D(markTitlesActors[0]);
        proxy.GetForeground()->RemoveActor2D(markTitlesActors[1]);
    }

    for (actorNum = 0; actorNum < axisTitlesActors.size(); actorNum++)
        proxy.GetForeground()->RemoveActor2D(axisTitlesActors[actorNum]);

    for (actorNum = 0; actorNum < axisMinimaActors.size(); actorNum++)
    {
        proxy.GetForeground()->RemoveActor2D(axisMinimaActors[actorNum]);
        proxy.GetForeground()->RemoveActor2D(axisMaximaActors[actorNum]);
    }

    for (actorNum = 0; actorNum < extentMinimaActors.size(); actorNum++)
    {
        proxy.GetForeground()->RemoveActor2D(extentMinimaActors[actorNum]);
        proxy.GetForeground()->RemoveActor2D(extentMaximaActors[actorNum]);
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::ButtonAction
//
// Purpose: This is the handler method that is called when a hotpoint for one
//          of the Extents tool's buttons is being pressed.
// Arguments:
//           e : The state of the hotpoint activity. (START, MIDDLE, END)
//           x : The x location of the mouse in pixels
//           y : The y location of the mouse in pixels
//    buttonID : Index of the button being pressed
//
// Programmer: Mark Blair
// Creation:   Thu Dec  7 12:47:17 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::ButtonAction(
    CB_ENUM e, int ctrl, int shift, int x, int y, int buttonID)
{
    if (e == CB_START)
    {
        activeHotPointID = buttonID + bqHotpointStartID;
        
        buttonAlreadyPressed = false;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        hotPoints[activeHotPointID].pt.x = buttonXs[buttonID];
        hotPoints[activeHotPointID].pt.y = buttonYs[buttonID];

        PressButton(buttonID);
        proxy.Render();
    }
    else
    {
        hotPoints[activeHotPointID].pt.x = buttonXs[buttonID];
        hotPoints[activeHotPointID].pt.y = buttonYs[buttonID];

        ReleaseButton(buttonID);
        proxy.Render();

        CallButtonCallback(buttonID);

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::LeftAxisSelectionMark
//
// Purpose: This is the handler method that is called when the left axis
//          selection mark's hotpoint is in motion.
//
// Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::LeftAxisSelectionMark(
    CB_ENUM e, int ctrl, int shift, int x, int y)
{
    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x + msHotpointRadius);
        activeHotPointID = msHotpointStartID;
        rightMarkHPX = hotPoints[msHotpointStartID+1].pt.x - msHotpointRadius*2.0;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        if (cursorPos.x < minSlidableX)
            cursorPos.x = minSlidableX;
        else if (cursorPos.x > rightMarkHPX)
            cursorPos.x = rightMarkHPX;

        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x + msHotpointRadius);

        hotPoints[activeHotPointID].pt.x = cursorPos.x;
        hotPoints[activeHotPointID].pt.y = markArrowTipY;

        MoveLeftMarkSliderArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        doubleVector axisXPositions = Interface.GetAxisXPositions();

        if (cursorPos.x < minSlidableX)
            cursorPos.x = minSlidableX;
        else if (cursorPos.x > rightMarkHPX)
            cursorPos.x = rightMarkHPX;

        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x + msHotpointRadius);
        
        if (activeAxisIndex >= rightSelectedAxisID)
            activeAxisIndex = rightSelectedAxisID - 1;

        hotPoints[activeHotPointID].pt.x =
            axisXPositions[activeAxisIndex]*markSliderXRange + minSlidableX;
        hotPoints[activeHotPointID].pt.y = markArrowTipY;

        MoveLeftMarkSliderArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();

        // Call the tool's callback for a changed minimum.
        CallLeftMarkMoveCallback();

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::RightAxisSelectionMark
//
// Purpose: This is the handler method that is called when the right axis
//          selection mark's hotpoint is in motion.
//
// Arguments:
//    e : The state of the hotpoint activity. (START, MIDDLE, END)
//    x : The x location of the mouse in pixels.
//    y : The y location of the mouse in pixels.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::RightAxisSelectionMark(
    CB_ENUM e, int ctrl, int shift, int x, int y)
{
    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x - msHotpointRadius);
        activeHotPointID = msHotpointStartID + 1;
        leftMarkHPX = hotPoints[msHotpointStartID].pt.x + msHotpointRadius*2.0;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        if (cursorPos.x > maxSlidableX)
            cursorPos.x = maxSlidableX;
        else if (cursorPos.x < leftMarkHPX)
            cursorPos.x = leftMarkHPX;

        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x - msHotpointRadius);

        hotPoints[activeHotPointID].pt.x = cursorPos.x;
        hotPoints[activeHotPointID].pt.y = markArrowTipY;

        MoveRightMarkSliderArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        doubleVector axisXPositions = Interface.GetAxisXPositions();

        if (cursorPos.x > maxSlidableX)
            cursorPos.x = maxSlidableX;
        else if (cursorPos.x < leftMarkHPX)
            cursorPos.x = leftMarkHPX;

        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x - msHotpointRadius);
        
        if (activeAxisIndex <= leftSelectedAxisID)
            activeAxisIndex = leftSelectedAxisID + 1;

        hotPoints[activeHotPointID].pt.x = minSlidableX + msHotpointRadius*2.0 +
            axisXPositions[activeAxisIndex]*markSliderXRange;
        hotPoints[activeHotPointID].pt.y = markArrowTipY;

        MoveRightMarkSliderArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();

        // Call the tool's callback for a changed minimum.
        CallRightMarkMoveCallback();

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::AxisSliderMinimum
//
// Purpose: This is the handler method that is called when a hotpoint for one
//          of the axis minima sliders is in motion.
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
//    Mark Blair, Thu Dec 21 18:54:43 PST 2006
//    Upgraded to support non-uniform axis spacing.
//
//    Mark Blair, Wed Mar 14 18:04:12 PDT 2007
//    Upgraded to support ganged axis sliders.
//
// ****************************************************************************

void VisitExtentsTool::AxisSliderMinimum(CB_ENUM e, int ctrl, int shift, int x, int y)
{
    bool gangedSlidersMotion = false;
    
    if (gangedHPDeltaY > -1.0)
        gangedSlidersMotion = true;
    else if ((shift != 0) && (e == CB_START))
        gangedSlidersMotion = true;

    if (gangedSlidersMotion)
    {
        GangAxisSliders(true, e, x, y);
        return;
    }

    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        if (cursorPos.x < leftAxisX)
            cursorPos.x = leftAxisX;
        else if (cursorPos.x > rightAxisX)
            cursorPos.x = rightAxisX;
            
        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x);

        if (asAxisHotpointIDs[activeAxisIndex] < 0)
        {
            debug3 << "VET/ASMin/1: Invalid hotpoint index." << endl;
            activeAxisIndex = leftSelectedAxisID;
        }

        activeMinHPID = asAxisHotpointIDs[activeAxisIndex]*2 + asHotpointStartID;
        activeMaxHPID = activeMinHPID + 1;

        activeMaxHPY = hotPoints[activeMaxHPID].pt.y - asHotpointRadius*2.0;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        if (cursorPos.y < minSlidableY)
            cursorPos.y = minSlidableY;
        else if (cursorPos.y > activeMaxHPY)
            cursorPos.y = activeMaxHPY;

        hotPoints[activeMinHPID].pt.x = hotPoints[activeMaxHPID].pt.x;
        hotPoints[activeMinHPID].pt.y = cursorPos.y;

        MoveAxisSliderMinimumArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        if (cursorPos.y < minSlidableY)
            cursorPos.y = minSlidableY;
        else if (cursorPos.y > activeMaxHPY)
            cursorPos.y = activeMaxHPY;

        hotPoints[activeMinHPID].pt.x = hotPoints[activeMaxHPID].pt.x;
        hotPoints[activeMinHPID].pt.y = cursorPos.y;

        MoveAxisSliderMinimumArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();

        // Broadcast changed extent to the rest of the world.
        CallExtentsCallback(true, false);

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::AxisSliderMaximum
//
// Purpose: This is the handler method that is called when a hotpoint for one
//          of the axis maxima sliders is in motion.
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
//    Mark Blair, Thu Dec 21 18:54:43 PST 2006
//    Upgraded to support non-uniform axis spacing.
//
//    Mark Blair, Wed Mar 14 18:04:12 PDT 2007
//    Upgraded to support ganged axis sliders.
//
// ****************************************************************************

void VisitExtentsTool::AxisSliderMaximum(CB_ENUM e, int ctrl, int shift, int x, int y)
{
    bool gangedSlidersMotion = false;
    
    if (gangedHPDeltaY > -1.0)
        gangedSlidersMotion = true;
    else if ((shift != 0) && (e == CB_START))
        gangedSlidersMotion = true;

    if (gangedSlidersMotion)
    {
        GangAxisSliders(false, e, x, y);
        return;
    }

    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        if (cursorPos.x < leftAxisX)
            cursorPos.x = leftAxisX;
        else if (cursorPos.x > rightAxisX)
            cursorPos.x = rightAxisX;

        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x);

        if (asAxisHotpointIDs[activeAxisIndex] < 0)
        {
            debug3 << "VET/ASMax/1: Invalid hotpoint index." << endl;
            activeAxisIndex = leftSelectedAxisID;
        }

        activeMinHPID = asAxisHotpointIDs[activeAxisIndex]*2 + asHotpointStartID;
        activeMaxHPID = activeMinHPID + 1;

        activeMinHPY = hotPoints[activeMinHPID].pt.y + asHotpointRadius*2.0;
        
/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        if (cursorPos.y > maxSlidableY)
            cursorPos.y = maxSlidableY;
        else if (cursorPos.y < activeMinHPY)
            cursorPos.y = activeMinHPY;

        hotPoints[activeMaxHPID].pt.x = hotPoints[activeMinHPID].pt.x;
        hotPoints[activeMaxHPID].pt.y = cursorPos.y;

        MoveAxisSliderMaximumArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        if (cursorPos.y > maxSlidableY)
            cursorPos.y = maxSlidableY;
        else if (cursorPos.y < activeMinHPY)
            cursorPos.y = activeMinHPY;

        hotPoints[activeMaxHPID].pt.x = hotPoints[activeMinHPID].pt.x;
        hotPoints[activeMaxHPID].pt.y = cursorPos.y;

        MoveAxisSliderMaximumArrow(activeAxisIndex);
        
        // Render the vis window.
        proxy.Render();

        // Broadcast changed extent to the rest of the world.
        CallExtentsCallback(false, true);

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method:  VisitExtentsTool::GangAxisSliders
//
// Purpose: This is the handler method that is called when a hotpoint for one
//          of the axis sliders is in motion and that slider is ganged with its
//          opposite slider on the same axis.
//
// Arguments:
//    dragedSliderIsMin: True if slider being dragged is an axis minimum slider
//                   e : The state of the hotpoint activity. (START, MIDDLE, END)
//                   x : The x location of the mouse in pixels.
//                   y : The y location of the mouse in pixels.
//
// Programmer: Mark Blair
// Creation:   Wed Mar 14 18:04:12 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::GangAxisSliders(
    bool draggedSliderIsMin, CB_ENUM e, int x, int y)
{
    avtVector cursorPos = ComputeDisplayToWorld(avtVector(x,y,0.0));

    if (e == CB_START)
    {
        if (cursorPos.x < leftAxisX)
            cursorPos.x = leftAxisX;
        else if (cursorPos.x > rightAxisX)
            cursorPos.x = rightAxisX;

        activeAxisIndex = AxisClosestToTipOfArrow(cursorPos.x);

        if (asAxisHotpointIDs[activeAxisIndex] < 0)
        {
            debug3 << "VET/GAS/1: Invalid hotpoint index." << endl;
            activeAxisIndex = leftSelectedAxisID;
        }

        activeMinHPID = asAxisHotpointIDs[activeAxisIndex]*2 + asHotpointStartID;
        activeMaxHPID = activeMinHPID + 1;

        gangedHPDeltaY = hotPoints[activeMaxHPID].pt.y - hotPoints[activeMinHPID].pt.y;

/* Makes plot disappear while hotpoint is in motion; ridiculous for parallel axis plot.
        proxy.StartBoundingBox();
*/
    }
    else if (e == CB_MIDDLE)
    {
        UpdateGangedAxisSliderPositions(draggedSliderIsMin, cursorPos.y);

        MoveAxisSliderMinimumArrow(activeAxisIndex);
        MoveAxisSliderMaximumArrow(activeAxisIndex);

        // Render the vis window.
        proxy.Render();
    }
    else
    {
        UpdateGangedAxisSliderPositions(draggedSliderIsMin, cursorPos.y);

        MoveAxisSliderMinimumArrow(activeAxisIndex);
        MoveAxisSliderMaximumArrow(activeAxisIndex);
        
        // Render the vis window.
        proxy.Render();

        // Broadcast changed extent to the rest of the world.
        CallExtentsCallback(true, true);
        
        gangedHPDeltaY = -1.0;

/* Makes plot reappear after hotpoint has moved; see comment above.
        proxy.EndBoundingBox();
*/
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::UpdateGangedAxisSliderPositions
//
// Purpose: Updates hotpoint positions of a pair of ganged axis sliders that
//          are currently in motion.
//
// Programmer: Mark Blair
// Creation:   Wed Mar 14 18:04:12 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void VisitExtentsTool::UpdateGangedAxisSliderPositions(
    bool draggedSliderIsMin, double cursorY)
{
    double cursorYPos = cursorY;
    
    if (draggedSliderIsMin)
    {
        if (cursorYPos < minSlidableY)
            cursorYPos = minSlidableY;
        else if (cursorYPos + gangedHPDeltaY > maxSlidableY)
            cursorYPos = maxSlidableY - gangedHPDeltaY;
            
        hotPoints[activeMinHPID].pt.y = cursorYPos;
        hotPoints[activeMaxHPID].pt.y = cursorYPos + gangedHPDeltaY;
            
        hotPoints[activeMinHPID].pt.x = hotPoints[activeMaxHPID].pt.x;
    }
    else
    {
        if (cursorYPos > maxSlidableY)
            cursorYPos = maxSlidableY;
        else if (cursorYPos - gangedHPDeltaY < minSlidableY)
            cursorYPos = minSlidableY + gangedHPDeltaY;
            
        hotPoints[activeMaxHPID].pt.y = cursorYPos;
        hotPoints[activeMinHPID].pt.y = cursorYPos - gangedHPDeltaY;
            
        hotPoints[activeMaxHPID].pt.x = hotPoints[activeMinHPID].pt.x;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CallButtonCallback
//
// Purpose: Performs the action corresponding to a press of one of the Extents
//          tool's buttons.
//
// Programmer: Mark Blair
// Creation:   Thu Dec  7 12:47:17 PST 2006
//
// Modifications:
//   
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports buttons for all functions of the Extents tool.
//   
// ****************************************************************************

void VisitExtentsTool::CallButtonCallback(int buttonID)
{
    doubleVector axisXPositions;
    intVector axisInfoFlagSets;
    int plotToolModeFlags, axisID;

    switch (buttonID)
    {
        case VET_LEFT_MOVE_LEFT_BUTTON_ID:
            if ((activeAxisIndex = leftSelectedAxisID-1) < leftShownAxisID) break;
            
            axisXPositions = Interface.GetAxisXPositions();

            hotPoints[msHotpointStartID].pt.x =
                axisXPositions[activeAxisIndex]*markSliderXRange + minSlidableX;

            MoveLeftMarkSliderArrow(activeAxisIndex);
            proxy.Render();

            CallLeftMarkMoveCallback();

            break;

        case VET_LEFT_MOVE_RIGHT_BUTTON_ID:

            if ((activeAxisIndex = leftSelectedAxisID+1) >= rightSelectedAxisID) break;
            
            axisXPositions = Interface.GetAxisXPositions();

            hotPoints[msHotpointStartID].pt.x =
                axisXPositions[activeAxisIndex]*markSliderXRange + minSlidableX;

            MoveLeftMarkSliderArrow(activeAxisIndex);
            proxy.Render();

            CallLeftMarkMoveCallback();

            break;

        case VET_RIGHT_MOVE_LEFT_BUTTON_ID:

            if ((activeAxisIndex = rightSelectedAxisID-1) <= leftSelectedAxisID) break;
            
            axisXPositions = Interface.GetAxisXPositions();

            hotPoints[msHotpointStartID+1].pt.x = minSlidableX + msHotpointRadius*2.0 +
                axisXPositions[activeAxisIndex]*markSliderXRange;

            MoveRightMarkSliderArrow(activeAxisIndex);
            proxy.Render();

            CallRightMarkMoveCallback();

            break;

        case VET_RIGHT_MOVE_RIGHT_BUTTON_ID:

            if ((activeAxisIndex = rightSelectedAxisID+1) > rightShownAxisID) break;
            
            axisXPositions = Interface.GetAxisXPositions();

            hotPoints[msHotpointStartID+1].pt.x = minSlidableX + msHotpointRadius*2.0 +
                axisXPositions[activeAxisIndex]*markSliderXRange;

            MoveRightMarkSliderArrow(activeAxisIndex);
            proxy.Render();

            CallRightMarkMoveCallback();

            break;

        case VET_LEFT_MARK_INFO_ON_BUTTON_ID:
        
            axisInfoFlagSets = Interface.GetAxisInfoFlagSets();

            axisInfoFlagSets[leftSelectedAxisID] ^= EA_AXIS_INFO_SHOWN_FLAG;

            Interface.SetAxisInfoFlagSets(axisInfoFlagSets);
            Interface.ExecuteCallback();

            break;

        case VET_RIGHT_MARK_INFO_ON_BUTTON_ID:

            axisInfoFlagSets = Interface.GetAxisInfoFlagSets();

            axisInfoFlagSets[rightSelectedAxisID] ^= EA_AXIS_INFO_SHOWN_FLAG;

            Interface.SetAxisInfoFlagSets(axisInfoFlagSets);
            Interface.ExecuteCallback();

            break;

        case VET_EXPAND_BETWEEN_BUTTON_ID:
        
            ExpandAxisSequenceBetweenSelectionMarks();

            break;

        case VET_UNDO_LAST_EXPANSION_BUTTON_ID:
        
            UndoPreviousAxisSequenceExpansion();

            break;

        case VET_THRESHOLD_BETWEEN_BUTTON_ID:

            plotToolModeFlags =
                Interface.GetPlotToolModeFlags() ^ EA_THRESHOLD_SELECTED_ONLY_FLAG;

            Interface.SetPlotToolModeFlags(plotToolModeFlags);
            Interface.ExecuteCallback();

            break;

        case VET_SHOW_TITLES_ONLY_BUTTON_ID:

            plotToolModeFlags = Interface.GetPlotToolModeFlags();
            axisInfoFlagSets  = Interface.GetAxisInfoFlagSets();
            
            plotToolModeFlags ^= EA_SHOW_LIMITED_AXIS_INFO_FLAG;
            
            for (axisID = 0; axisID < axisInfoFlagSets.size(); axisID++)
            {
                axisInfoFlagSets[axisID] ^= EA_SHOW_AXIS_LIMITS_ONLY_FLAG;
                axisInfoFlagSets[axisID] ^= EA_SHOW_SLIDER_LIMITS_ONLY_FLAG;
            }

            Interface.SetPlotToolModeFlags(plotToolModeFlags);
            Interface.SetAxisInfoFlagSets(axisInfoFlagSets);
            Interface.ExecuteCallback();

            break;
    }
}


// ****************************************************************************
// Method: VisitExtentsTool::CallLeftMarkMoveCallback
//
// Purpose: Performs the action corresponding to a move of the Extents tool's
//          left axis selection mark.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void VisitExtentsTool::CallLeftMarkMoveCallback()
{
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();
    
    // Reverse temporary button toggle setting flag.  Yes, it's a kludge.
    buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] ^= VET_BUTTON_TOGGLE_IS_ON;
    
    axisInfoFlagSets[leftSelectedAxisID] &=
        (0xffffffff ^ EA_LEFT_SELECTED_AXIS_FLAG);
    leftSelectedAxisID = activeAxisIndex;
    axisInfoFlagSets[leftSelectedAxisID] |= EA_LEFT_SELECTED_AXIS_FLAG;
    
    Interface.SetAxisInfoFlagSets(axisInfoFlagSets);
    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method: VisitExtentsTool::CallRightMarkMoveCallback
//
// Purpose: Performs the action corresponding to a move of the Extents tool's
//          right axis selection mark.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void VisitExtentsTool::CallRightMarkMoveCallback()
{
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();
    
    // Reverse temporary button toggle setting flag.  Yes, it's a kludge.
    buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] ^= VET_BUTTON_TOGGLE_IS_ON;
    
    axisInfoFlagSets[rightSelectedAxisID] &=
        (0xffffffff ^ EA_RIGHT_SELECTED_AXIS_FLAG);
    rightSelectedAxisID = activeAxisIndex;
    axisInfoFlagSets[rightSelectedAxisID] |= EA_RIGHT_SELECTED_AXIS_FLAG;
    
    Interface.SetAxisInfoFlagSets(axisInfoFlagSets);
    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method: VisitExtentsTool::CallExtentsCallback
//
// Purpose: Lets the outside world know that the tool has new extents.
//
// Programmer: Mark Blair
// Creation:   Wed Mar 14 18:04:12 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void VisitExtentsTool::CallExtentsCallback(bool sendMinima, bool sendMaxima)
{
    if (sendMinima)
    {
        doubleVector newMinima          = Interface.GetMinima();
        intVector    newMinTimeOrdinals = Interface.GetMinTimeOrdinals();
    
        if ((activeAxisIndex < 0) || (activeAxisIndex >= newMinima.size()))
        {
            debug3 << "VET/CEC/1: Invalid axis index." << endl;
            return;
        }
    
        newMinima[activeAxisIndex] =
            (hotPoints[activeMinHPID].pt.y - minSlidableY) / normToWorldYScale;
        newMinTimeOrdinals[activeAxisIndex] = curTimeOrdinal;
        
        if (!sendMaxima) curTimeOrdinal++;
    
        Interface.SetMinima(newMinima);
        Interface.SetMinTimeOrdinals(newMinTimeOrdinals);
    }
    
    if (sendMaxima)
    {
        doubleVector newMaxima          = Interface.GetMaxima();
        intVector    newMaxTimeOrdinals = Interface.GetMaxTimeOrdinals();
    
        if ((activeAxisIndex < 0) || (activeAxisIndex >= newMaxima.size()))
        {
            debug3 << "VET/CEC/2: Invalid axis index." << endl;
            return;
        }
    
        newMaxima[activeAxisIndex] = 1.0 -
            (maxSlidableY - hotPoints[activeMaxHPID].pt.y) / normToWorldYScale;
        newMaxTimeOrdinals[activeAxisIndex] = curTimeOrdinal++;

        Interface.SetMaxima(newMaxima);
        Interface.SetMaxTimeOrdinals(newMaxTimeOrdinals);
    }
    
    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method:  VisitExtentsTool::PressButton
//
// Purpose: Changes colors of a button's polygons to signify that the button
//          has been pressed.  Also redraws button at its original position.
//
// Programmer: Mark Blair
// Creation:   Thu Dec  7 12:47:17 PST 2006
//
// Modifications:
//   
//     Mark Blair, Tue Jan 23 19:13:03 PST 2007
//     Now supports toggle buttons via check marks (actually X marks).
//
// ****************************************************************************

void VisitExtentsTool::PressButton(int buttonID)
{
    if (buttonCheckIDs[buttonID] < 0)
    {
        debug3 << "VET/PB/1: Invalid button index." << endl;
        buttonID = 0;
    }

    bool buttonIsToggle = ((buttonFlags[buttonID] & VET_BUTTON_IS_A_TOGGLE) != 0);

    if (!buttonAlreadyPressed)
    {
        if (!buttonIsToggle)
        {
            unsigned char *quadRGB = buttonQuadsColors->GetPointer(buttonID*15);

            *quadRGB++ = buttonColor2Red;
            *quadRGB++ = buttonColor2Green;
            *quadRGB++ = buttonColor2Blue;

            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;
            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;

            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;
            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB   = buttonColor0Blue;
        }
        else    // Currently all toggle buttons use check marks.
        {
            unsigned char *checkRGB =
                buttonChecksColors->GetPointer(buttonCheckIDs[buttonID]*6);

            if ((buttonFlags[buttonID] & VET_BUTTON_TOGGLE_IS_ON) != 0)
            {
                *checkRGB++ = buttonColor1Red;
                *checkRGB++ = buttonColor1Green;
                *checkRGB++ = buttonColor1Blue;

                *checkRGB++ = buttonColor1Red;
                *checkRGB++ = buttonColor1Green;
                *checkRGB   = buttonColor1Blue;
            }
            else
            {
                memset((void *)checkRGB, 0, 6);
                checkRGB += 6;
            }
        }

        buttonAlreadyPressed = true;
    }
    
    if (buttonIsToggle)
        buttonChecksData->Modified();
    else
        buttonQuadsData->Modified();
}


// ****************************************************************************
// Method:  VisitExtentsTool::ReleaseButton
//
// Purpose: Changes colors of a button's polygons to signify that a pressed
//          button has been released.  Also redraws button at its original
//          position.
//
// Programmer: Mark Blair
// Creation:   Thu Dec  7 12:47:17 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void VisitExtentsTool::ReleaseButton(int buttonID)
{
    if (buttonAlreadyPressed)
    {
        if ((buttonFlags[buttonID] & VET_BUTTON_IS_A_TOGGLE) != 0)
        {
            buttonFlags[buttonID] ^= VET_BUTTON_TOGGLE_IS_ON;
        }
        else
        {
            unsigned char *quadRGB = buttonQuadsColors->GetPointer(buttonID*15);

            *quadRGB++ = buttonColor1Red;
            *quadRGB++ = buttonColor1Green;
            *quadRGB++ = buttonColor1Blue;

            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;
            *quadRGB++ = buttonColor0Red;
            *quadRGB++ = buttonColor0Green;
            *quadRGB++ = buttonColor0Blue;

            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB++ = buttonColor3Blue;
            *quadRGB++ = buttonColor3Red;
            *quadRGB++ = buttonColor3Green;
            *quadRGB   = buttonColor3Blue;
        }
        
        buttonAlreadyPressed = false;
    }

    buttonQuadsData->Modified();
}


// ****************************************************************************
// Method:  VisitExtentsTool::MoveLeftMarkSliderArrow
//
// Purpose: Changes X coordinates of the left axis selection mark arrow's
//          vertices to reflect the arrow's movement left or right.  Also
//          updates the accompanying axis title to reflect the arrow's current
//          position.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//
//     Mark Blair, Tue Jan 23 19:13:03 PST 2007
//     Updates corresponding axis info toggle button to reflect arrow position.
//   
// ****************************************************************************

void VisitExtentsTool::MoveLeftMarkSliderArrow(int axisIndex)
{
    double hotPointX = hotPoints[msHotpointStartID].pt.x;
    avtVector titlePos;
    char markAxisTitle[VET_H_TITLE_MAX_CHARS + 1];
    double vertexXYZ[3];
    vertexXYZ[2] = 0.0;

    for (vtkIdType vertexID = 0; vertexID < 3; vertexID++)
    {
        markArrowPoints->GetPoint(vertexID, vertexXYZ);

        if (vertexID != 2)
            vertexXYZ[0] = hotPointX;
        else
            vertexXYZ[0] = hotPointX + msHotpointRadius;

        markArrowPoints->SetPoint(vertexID, vertexXYZ);
    }

    markSlidersData->Modified();
    
    titlePos = ComputeWorldToDisplay(avtVector(hotPointX, markArrowTipY, 0.0));
    titlePos.x -= markTitleXOffset;
    markTitlesActors[0]->SetPosition(titlePos.x, titlePos.y);

    MakeAxisTitleText(
        markAxisTitle, axisTitles[axisIndex], VET_H_TITLE_MAX_CHARS);
    markTitlesActors[0]->SetInput(markAxisTitle);
        
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();
            
    if ((axisInfoFlagSets[axisIndex] & EA_AXIS_INFO_SHOWN_FLAG) == 0)
        buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] |= VET_BUTTON_TOGGLE_IS_ON;
    else  // Temporarily toggle button flag for PressButton.  Yes, it's a kludge.
    {
        buttonFlags[VET_LEFT_MARK_INFO_ON_BUTTON_ID] &= 
            (0xffffffff ^ VET_BUTTON_TOGGLE_IS_ON);
    }
    
    buttonAlreadyPressed = false;

    PressButton(VET_LEFT_MARK_INFO_ON_BUTTON_ID);
}


// ****************************************************************************
// Method:  VisitExtentsTool::MoveRightMarkSliderArrow
//
// Purpose: Changes X coordinates of the right axis selection mark arrow's
//          vertices to reflect the arrow's movement left or right.  Also
//          updates the accompanying axis title to reflect the arrow's current
//          position.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//
//     Mark Blair, Tue Jan 23 19:13:03 PST 2007
//     Updates corresponding axis info toggle button to reflect arrow position.
//   
// ****************************************************************************

void VisitExtentsTool::MoveRightMarkSliderArrow(int axisIndex)
{
    double hotPointX = hotPoints[msHotpointStartID+1].pt.x;
    avtVector titlePos;
    char markAxisTitle[VET_H_TITLE_MAX_CHARS + 1];
    double vertexXYZ[3];
    vertexXYZ[2] = 0.0;

    for (vtkIdType vertexID = 3; vertexID < 6; vertexID++)
    {
        markArrowPoints->GetPoint(vertexID, vertexXYZ);

        if (vertexID != 3)
            vertexXYZ[0] = hotPointX;
        else
            vertexXYZ[0] = hotPointX - msHotpointRadius;

        markArrowPoints->SetPoint(vertexID, vertexXYZ);
    }
    
    markSlidersData->Modified();
    
    titlePos = ComputeWorldToDisplay(avtVector(hotPointX, markArrowTipY, 0.0));
    titlePos.x += markTitleXOffset;
    markTitlesActors[1]->SetPosition(titlePos.x, titlePos.y);

    MakeAxisTitleText(
        markAxisTitle, axisTitles[axisIndex], VET_H_TITLE_MAX_CHARS);
    markTitlesActors[1]->SetInput(markAxisTitle);
        
    intVector axisInfoFlagSets = Interface.GetAxisInfoFlagSets();
            
    if ((axisInfoFlagSets[axisIndex] & EA_AXIS_INFO_SHOWN_FLAG) == 0)
        buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] |= VET_BUTTON_TOGGLE_IS_ON;
    else  // Temporarily toggle button flag for PressButton.  Yes, it's a kludge.
    {
        buttonFlags[VET_RIGHT_MARK_INFO_ON_BUTTON_ID] &= 
            (0xffffffff ^ VET_BUTTON_TOGGLE_IS_ON);
    }
    
    buttonAlreadyPressed = false;

    PressButton(VET_RIGHT_MARK_INFO_ON_BUTTON_ID);
}


// ****************************************************************************
// Method:  VisitExtentsTool::ExpandAxisSequenceBetweenSelectionMarks
//
// Purpose: Expands the sequence of axes between the left and right selection
//          marks to fill the entire width of the ParallelAxis plot.
//
// Programmer: Mark Blair
// Creation:   Thu Jan 25 16:16:51 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::ExpandAxisSequenceBetweenSelectionMarks()
{
    if (leftShownAxisID == leftSelectedAxisID)
    {
        if (rightShownAxisID == rightSelectedAxisID) return;
    }

    int plotToolModeFlags       = Interface.GetPlotToolModeFlags();
    intVector axisInfoFlagSets  = Interface.GetAxisInfoFlagSets();
    
    leftExpandAxisIDs.push_back(leftShownAxisID);
    rightExpandAxisIDs.push_back(rightShownAxisID);
    
    axisInfoFlagSets[leftShownAxisID]  &= (0xffffffff ^ EA_LEFT_SHOWN_AXIS_FLAG);
    axisInfoFlagSets[rightShownAxisID] &= (0xffffffff ^ EA_RIGHT_SHOWN_AXIS_FLAG);
    
    leftShownAxisID = leftSelectedAxisID; rightShownAxisID = rightSelectedAxisID;
    
    axisInfoFlagSets[leftShownAxisID]  |= EA_LEFT_SHOWN_AXIS_FLAG;
    axisInfoFlagSets[rightShownAxisID] |= EA_RIGHT_SHOWN_AXIS_FLAG;
    
    plotToolModeFlags |= EA_SHOW_MARKED_AXES_ONLY_FLAG;

    Interface.SetPlotToolModeFlags(plotToolModeFlags);
    Interface.SetAxisInfoFlagSets(axisInfoFlagSets);

    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method:  VisitExtentsTool::UndoPreviousAxisSequenceExpansion
//
// Purpose: Restores the displayed axis sequence to what it was just prior to
//          the previous axis sequence expansion.  If any axes were added to
//          or deleted from the ParallelAxis plot in the meantime, or if the
//          order of axes in the plot was changed in the meantime, then all the
//          plot's axes will be displayed.
//
// Programmer: Mark Blair
// Creation:   Thu Jan 25 16:16:51 PST 2007
//
// Modifications:
//
// ****************************************************************************

void VisitExtentsTool::UndoPreviousAxisSequenceExpansion()
{
    int stackDepth = leftExpandAxisIDs.size();

    if (stackDepth == 0) return;
    
    int plotToolModeFlags       = Interface.GetPlotToolModeFlags();
    intVector axisInfoFlagSets  = Interface.GetAxisInfoFlagSets();
    
    axisInfoFlagSets[leftShownAxisID]  &= (0xffffffff ^ EA_LEFT_SHOWN_AXIS_FLAG);
    axisInfoFlagSets[rightShownAxisID] &= (0xffffffff ^ EA_RIGHT_SHOWN_AXIS_FLAG);
    
    leftShownAxisID  = leftExpandAxisIDs[stackDepth-1];
    rightShownAxisID = rightExpandAxisIDs[stackDepth-1];
    
    axisInfoFlagSets[leftShownAxisID]  |= EA_LEFT_SHOWN_AXIS_FLAG;
    axisInfoFlagSets[rightShownAxisID] |= EA_RIGHT_SHOWN_AXIS_FLAG;
    
    leftExpandAxisIDs.pop_back(); rightExpandAxisIDs.pop_back();
    
    if (stackDepth == 1)
        plotToolModeFlags &= (0xffffffff ^ EA_SHOW_MARKED_AXES_ONLY_FLAG);

    Interface.SetPlotToolModeFlags(plotToolModeFlags);
    Interface.SetAxisInfoFlagSets(axisInfoFlagSets);

    Interface.ExecuteCallback();
}


// ****************************************************************************
// Method:  VisitExtentsTool::MoveAxisSliderMinimumArrow
//
// Purpose: Changes Y coordinates of an axis slider minimum arrow's vertices to
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

void VisitExtentsTool::MoveAxisSliderMinimumArrow(int axisIndex)
{
    int hotpointPairID = asAxisHotpointIDs[axisIndex];
    vtkIdType firstVertexID = (vtkIdType)(hotpointPairID * 6);
    double hotPointY = hotPoints[hotpointPairID*2+asHotpointStartID].pt.y;
    double vertexXYZ[3];
    
    vertexXYZ[2] = 0.0;

    for (vtkIdType vertexID = firstVertexID; vertexID < firstVertexID+3; vertexID++)
    {
        axisArrowPoints->GetPoint(vertexID, vertexXYZ);

        if (vertexID != firstVertexID+2)
            vertexXYZ[1] = hotPointY;
        else
            vertexXYZ[1] = hotPointY + asHotpointRadius;

        axisArrowPoints->SetPoint(vertexID, vertexXYZ);
    }

    axisSlidersData->Modified();
}


// ****************************************************************************
// Method:  VisitExtentsTool::MoveAxisSliderMaximumArrow
//
// Purpose: Changes Y coordinates of an axis slider maximum arrow's vertices to
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

void VisitExtentsTool::MoveAxisSliderMaximumArrow(int axisIndex)
{
    int hotpointPairID = asAxisHotpointIDs[axisIndex];
    vtkIdType firstVertexID = (vtkIdType)(hotpointPairID*6 + 3);
    double hotPointY = hotPoints[hotpointPairID*2+asHotpointStartID+1].pt.y;
    double vertexXYZ[3];

    vertexXYZ[2] = 0.0;

    for (vtkIdType vertexID = firstVertexID; vertexID < firstVertexID+3; vertexID++)
    {
        axisArrowPoints->GetPoint(vertexID, vertexXYZ);

        if (vertexID != firstVertexID)
            vertexXYZ[1] = hotPointY;
        else
            vertexXYZ[1] = hotPointY - asHotpointRadius;

        axisArrowPoints->SetPoint(vertexID, vertexXYZ);
    }

    axisSlidersData->Modified();
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
    // Nothing yet.
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
//          tool's buttons and slider arrowheads.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Thu Dec  7 12:47:17 PST 2006
//    Added callbacks for buttons.
//
// ****************************************************************************

void VisitExtentsTool::MoveLeftMarkOneAxisLeftButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_LEFT_MOVE_LEFT_BUTTON_ID);
}


void VisitExtentsTool::MoveRightMarkOneAxisLeftButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_RIGHT_MOVE_LEFT_BUTTON_ID);
}


void VisitExtentsTool::MoveLeftMarkOneAxisRightButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_LEFT_MOVE_RIGHT_BUTTON_ID);
}


void VisitExtentsTool::MoveRightMarkOneAxisRightButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_RIGHT_MOVE_RIGHT_BUTTON_ID);
}


void VisitExtentsTool::ShowAxisInfoAtLeftMarkIfOnButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_LEFT_MARK_INFO_ON_BUTTON_ID);
}


void VisitExtentsTool::ShowAxisInfoAtRightMarkIfOnButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_RIGHT_MARK_INFO_ON_BUTTON_ID);
}


void VisitExtentsTool::ExpandBetweenMarkedAxesButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_EXPAND_BETWEEN_BUTTON_ID);
}


void VisitExtentsTool::UndoLastAxisSectionExpansionButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_UNDO_LAST_EXPANSION_BUTTON_ID);
}


void VisitExtentsTool::ThresholdBetweenMarkedAxesOnlyIfOnButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_THRESHOLD_BETWEEN_BUTTON_ID);
}


void VisitExtentsTool::ShowAxisTitlesOnlyIfOnButtonCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->ButtonAction(e, ctrl, shift, x, y, VET_SHOW_TITLES_ONLY_BUTTON_ID);
}


void VisitExtentsTool::LeftAxisSelectionMarkCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->LeftAxisSelectionMark(e, ctrl, shift, x, y);
}


void VisitExtentsTool::RightAxisSelectionMarkCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->RightAxisSelectionMark(e, ctrl, shift, x, y);
}


void VisitExtentsTool::AxisSliderMinimumCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->AxisSliderMinimum(e, ctrl, shift, x, y);
}


void VisitExtentsTool::AxisSliderMaximumCallback(
    VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y)
{
    VisitExtentsTool *et = (VisitExtentsTool *)it;
    et->AxisSliderMaximum(e, ctrl, shift, x, y);
}


// *****************************************************************************
// Method: VisitExtentsTool::AxisClosestToTipOfArrow
//
// Purpose: Given the X position of of a slider arrow's tip, this method
//          determines which axis in the plot is closest to that position.
//
// Programmer: Mark Blair
// Creation:   Thu Dec 21 18:54:43 PST 2006
//
// Modifications:
//
//     Mark Blair, Wed Jan 31 12:05:19 PST 2007
//     Modified to support axis sequence expansion.
//
// *****************************************************************************

int VisitExtentsTool::AxisClosestToTipOfArrow(double arrowTipX)
{
    doubleVector axisXPositions = Interface.GetAxisXPositions();

    int axisID;
    double x0To1 = (arrowTipX - leftAxisX) / markSliderXRange;
    double distanceFraction;
    
    if (x0To1 <= 0.0) return(leftShownAxisID);
    if (x0To1 >= 1.0) return(rightShownAxisID);
    
    for (axisID = leftShownAxisID + 1; axisID <= rightShownAxisID; axisID++)
    {
        if (x0To1 <= axisXPositions[axisID]) break;
    }
    
    if (axisID > rightShownAxisID) return rightShownAxisID;
    
    distanceFraction = (x0To1 - axisXPositions[axisID-1]) /
                       (axisXPositions[axisID] - axisXPositions[axisID-1]);
                       
    if (distanceFraction < 0.5) axisID--;
    
    return axisID;
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
//     Mark Blair, Mon Apr 23 18:53:27 PDT 2007
//     Simplified format conventions.  Now also consistent with Threshold op.
//
// *****************************************************************************

void VisitExtentsTool::MakeAxisTitleText(
    char titleText[], const std::string &axisTitle, int maxTitleChars)
{
    int rawTitleLen;
    char rawTitle[121];
    
    strncpy(rawTitle, axisTitle.c_str(), 120);
    
    if ((rawTitleLen = strlen(rawTitle)) <= maxTitleChars)
        strcpy(titleText, rawTitle);
    else
    {
        rawTitle[maxTitleChars-3] = '\0';
        sprintf(titleText,"%s..%s", rawTitle, &rawTitle[rawTitleLen-2]);
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
