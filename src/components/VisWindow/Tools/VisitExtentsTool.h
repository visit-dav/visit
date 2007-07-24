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

#ifndef VISIT_EXTENTS_TOOL_H
#define VISIT_EXTENTS_TOOL_H

#include <viswindow_exports.h>
#include <VisitInteractiveTool.h>
#include <avtExtentsToolInterface.h>

#include <vtkTextProperty.h>

#define VET_MINIMUM_NUMBER_OF_EXTENTS     2

#define VET_BUTTON_QUAD_INNER_RADIUS      0.0085
#define VET_BUTTON_LOGO_RADIUS            0.005
#define VET_BUTTON_CHECK_MARK_RADIUS      0.007
#define VET_BUTTON_HOTPOINT_RADIUS        0.018
#define VET_MARK_SLIDER_HOTPOINT_RADIUS   0.01
#define VET_AXIS_SLIDER_HOTPOINT_RADIUS   0.015

#define VET_BUTTON_BEVEL_WIDTH_WEIGHT     0.4
#define VET_BUTTON_BEVEL_EXTRA_WEIGHT     0.0005

#define VET_MAX_BUTTONS                   16

#define VET_LEFT_MOVE_LEFT_BUTTON_ID      0
#define VET_RIGHT_MOVE_LEFT_BUTTON_ID     1
#define VET_LEFT_MOVE_RIGHT_BUTTON_ID     2
#define VET_RIGHT_MOVE_RIGHT_BUTTON_ID    3
#define VET_LEFT_MARK_INFO_ON_BUTTON_ID   4
#define VET_RIGHT_MARK_INFO_ON_BUTTON_ID  5
#define VET_EXPAND_BETWEEN_BUTTON_ID      6
#define VET_UNDO_LAST_EXPANSION_BUTTON_ID 7
#define VET_THRESHOLD_BETWEEN_BUTTON_ID   8
#define VET_SHOW_TITLES_ONLY_BUTTON_ID    9
    
#define VET_LEFT_MOVE_LEFT_BUTTON_X       0.025
#define VET_LEFT_MOVE_LEFT_BUTTON_Y       0.06
#define VET_RIGHT_MOVE_LEFT_BUTTON_X      0.945
#define VET_RIGHT_MOVE_LEFT_BUTTON_Y      0.06
#define VET_LEFT_MOVE_RIGHT_BUTTON_X      0.055
#define VET_LEFT_MOVE_RIGHT_BUTTON_Y      0.06
#define VET_RIGHT_MOVE_RIGHT_BUTTON_X     0.975
#define VET_RIGHT_MOVE_RIGHT_BUTTON_Y     0.06
#define VET_LEFT_MARK_INFO_ON_BUTTON_X    0.2
#define VET_LEFT_MARK_INFO_ON_BUTTON_Y    0.06
#define VET_RIGHT_MARK_INFO_ON_BUTTON_X   0.8
#define VET_RIGHT_MARK_INFO_ON_BUTTON_Y   0.06
#define VET_EXPAND_BETWEEN_BUTTON_X       0.48
#define VET_EXPAND_BETWEEN_BUTTON_Y       0.06
#define VET_UNDO_LAST_EXPANSION_BUTTON_X  0.48
#define VET_UNDO_LAST_EXPANSION_BUTTON_Y  0.025
#define VET_THRESHOLD_BETWEEN_BUTTON_X    0.52
#define VET_THRESHOLD_BETWEEN_BUTTON_Y    0.06
#define VET_SHOW_TITLES_ONLY_BUTTON_X     0.52
#define VET_SHOW_TITLES_ONLY_BUTTON_Y     0.025

#define VET_NOT_A_BUTTON_X               -1.0

#define VET_MAX_BUTTON_LABELS             VET_MAX_BUTTONS

#define VET_MOVE_LEFT_MARK_B_LAB_ID       0
#define VET_MOVE_RIGHT_MARK_B_LAB_ID      1
#define VET_LEFT_MARK_INFO_ON_B_LAB_ID    2
#define VET_RIGHT_MARK_INFO_ON_B_LAB_ID   3
#define VET_EXPAND_BETWEEN_B_LAB_ID       4
#define VET_UNDO_LAST_EXPANSION_B_LAB_ID  5
#define VET_THRESHOLD_BETWEEN_B_LAB_ID    6
#define VET_SHOW_TITLES_ONLY_B_LAB_ID     7

#define VET_MOVE_LEFT_MARK_B_LABEL_X      0.04
#define VET_MOVE_LEFT_MARK_B_LABEL_Y      0.035
#define VET_MOVE_RIGHT_MARK_B_LABEL_X     0.96
#define VET_MOVE_RIGHT_MARK_B_LABEL_Y     0.035
#define VET_LEFT_MARK_INFO_ON_B_LABEL_X   0.2
#define VET_LEFT_MARK_INFO_ON_B_LABEL_Y   0.025
#define VET_RIGHT_MARK_INFO_ON_B_LABEL_X  0.8
#define VET_RIGHT_MARK_INFO_ON_B_LABEL_Y  0.025
#define VET_EXPAND_BETWEEN_B_LABEL_X      0.46
#define VET_EXPAND_BETWEEN_B_LABEL_Y      0.06
#define VET_UNDO_LAST_EXPANSION_B_LABEL_X 0.46
#define VET_UNDO_LAST_EXPANSION_B_LABEL_Y 0.025
#define VET_THRESHOLD_BETWEEN_B_LABEL_X   0.54
#define VET_THRESHOLD_BETWEEN_B_LABEL_Y   0.06
#define VET_SHOW_TITLES_ONLY_B_LABEL_X    0.54
#define VET_SHOW_TITLES_ONLY_B_LABEL_Y    0.025

#define VET_MOVE_LEFT_MARK_B_L_JUST       VTK_TEXT_CENTERED
#define VET_MOVE_RIGHT_MARK_B_L_JUST      VTK_TEXT_CENTERED
#define VET_LEFT_MARK_INFO_ON_B_L_JUST    VTK_TEXT_CENTERED
#define VET_RIGHT_MARK_INFO_ON_B_L_JUST   VTK_TEXT_CENTERED
#define VET_EXPAND_BETWEEN_B_L_JUST       VTK_TEXT_RIGHT
#define VET_UNDO_LAST_EXPANSION_B_L_JUST  VTK_TEXT_RIGHT
#define VET_THRESHOLD_BETWEEN_B_L_JUST    VTK_TEXT_LEFT
#define VET_SHOW_TITLES_ONLY_B_L_JUST     VTK_TEXT_LEFT

#define VET_B_LABEL_SIZE_MIN_DIM_RATIO    0.016 // Size (not the ratio) is in pixels

#define VET_BUTTON_IS_A_TOGGLE            0x00000001
#define VET_BUTTON_TOGGLE_IS_ON           0x00000002
#define VET_BUTTON_HAS_LEFT_ARROW_LOGO    0x00000004
#define VET_BUTTON_HAS_RIGHT_ARROW_LOGO   0x00000008
#define VET_BUTTON_HAS_CHECK_MARK         0x00000010
#define VET_BUTTON_HAS_ARROW_LOGO        (VET_BUTTON_HAS_LEFT_ARROW_LOGO |     \
                                          VET_BUTTON_HAS_RIGHT_ARROW_LOGO )
#define VET_BUTTON_HAS_INNER_GLYPH       (VET_BUTTON_HAS_ARROW_LOGO |          \
                                          VET_BUTTON_HAS_CHECK_MARK )
                                          
#define VET_MARK_TITLE_SIZE_WIDTH_RATIO   0.016 // Size (not the ratio) is in pixels
#define VET_MARK_TITLE_X_OFFSET           0.005

#define VET_H_TITLE_SIZE_MIN_DIM_RATIO    0.020 // Size (not the ratio) is in pixels
#define VET_H_TITLE_MAX_CHARS             15    // Must be >= VET_V_TITLE_MAX_CHARS
// #define VET_H_TITLE_Y_OFFSET             -0.085
#define VET_H_TITLE_Y_OFFSET             -0.075
#define VET_H_BOUND_SIZE_MIN_DIM_RATIO    0.016 // Size (not the ratio) is in pixels
#define VET_H_BOUND_MAX_CHARS             25    // Must be >= VET_V_BOUND_MAX_CHARS
#define VET_H_MARK_Y_OFFSET               0.060
// #define VET_H_AXIS_MIN_Y_OFFSET          -0.065
#define VET_H_AXIS_MIN_Y_OFFSET          -0.055
#define VET_H_AXIS_MAX_Y_OFFSET           0.115
// #define VET_H_SLIDER_MIN_Y_OFFSET        -0.045
#define VET_H_SLIDER_MIN_Y_OFFSET        -0.035
#define VET_H_SLIDER_MAX_Y_OFFSET         0.095

#define VET_V_TITLE_SIZE_MIN_DIM_RATIO    0.020 // Size is in pixels
#define VET_V_TITLE_MAX_CHARS             12    // Must be <= VET_H_TITLE_MAX_CHARS
#define VET_V_TITLE_X_OFFSET              0.006
#define VET_V_BOUND_SIZE_MIN_DIM_RATIO    0.016 // Size (not the ratio) is in pixels
#define VET_V_BOUND_MAX_CHARS             25    // Must be <= VET_H_BOUND_MAX_CHARS
#define VET_V_MARK_Y_OFFSET               0.035
#define VET_V_AXIS_MIN_X_OFFSET          -0.01
#define VET_V_AXIS_MAX_X_OFFSET          -0.005
#define VET_V_SLIDER_MIN_X_OFFSET         0.02
#define VET_V_SLIDER_MAX_X_OFFSET         0.01
#define VET_V_AXIS_BOTTOM_MARGIN          0.02
#define VET_V_AXIS_TOP_MARGIN             0.05

#define VET_MIN_AXIS_EXTENSION_DOTS       3
#define VET_MAX_AXIS_EXTENSION_DOTS       12
#define VET_AXIS_EXTENSION_DOT_RADIUS     0.0025
#define VET_PIXELS_PER_A_E_DOT_INTERVAL   12

#define VET_BUTTON_COLOR_0_RED            0.85
#define VET_BUTTON_COLOR_0_GREEN          0.85
#define VET_BUTTON_COLOR_0_BLUE           0.85

#define VET_BUTTON_COLOR_1_RED            0.65
#define VET_BUTTON_COLOR_1_GREEN          0.65
#define VET_BUTTON_COLOR_1_BLUE           0.65

#define VET_BUTTON_COLOR_2_RED            0.45
#define VET_BUTTON_COLOR_2_GREEN          0.45
#define VET_BUTTON_COLOR_2_BLUE           0.45

#define VET_BUTTON_COLOR_3_RED            0.25
#define VET_BUTTON_COLOR_3_GREEN          0.25
#define VET_BUTTON_COLOR_3_BLUE           0.25

#define VET_BUTTON_LOGO_RED               0.0
#define VET_BUTTON_LOGO_GREEN             0.0
#define VET_BUTTON_LOGO_BLUE              0.0

#define VET_BUTTON_LABEL_RED              0.0
#define VET_BUTTON_LABEL_GREEN            0.0
#define VET_BUTTON_LABEL_BLUE             0.0

#define VET_MARK_SLIDER_RED               0.0
#define VET_MARK_SLIDER_GREEN             0.0
#define VET_MARK_SLIDER_BLUE              1.0

#define VET_MARK_TITLE_RED                0.0
#define VET_MARK_TITLE_GREEN              0.0
#define VET_MARK_TITLE_BLUE               1.0

#define VET_AXIS_EXTENSION_RED            0.0
#define VET_AXIS_EXTENSION_GREEN          0.75
#define VET_AXIS_EXTENSION_BLUE           0.75

#define VET_AXIS_SLIDER_RED               1.0
#define VET_AXIS_SLIDER_GREEN             0.0
#define VET_AXIS_SLIDER_BLUE              0.0

#define VET_AXIS_TITLE_RED                0.0
#define VET_AXIS_TITLE_GREEN              0.0
#define VET_AXIS_TITLE_BLUE               1.0

#define VET_AXIS_BOUND_RED                0.0
#define VET_AXIS_BOUND_GREEN              0.75
#define VET_AXIS_BOUND_BLUE               0.75

#define VET_AXIS_EXTENT_RED               1.0
#define VET_AXIS_EXTENT_GREEN             0.0
#define VET_AXIS_EXTENT_BLUE              0.0

// Forward declarations
class VisWindow;
class vtkActor2D;
class vtkPoints;
class vtkPolyDataMapper2D;
class vtkPolyData;
class vtkTextActor;
class vtkUnsignedCharArray;

// ****************************************************************************
// Class: VisitExtentsTool
//
// Purpose: This class contains an interactive tool to manipulate the values
//          of individual minima and maxima within a set of (min,max) pairs.
//          So far its use is restricted to 2-D VisIt plots.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 18:35:00 PST 2005
//
// Modifications:
//
//    Mark Blair, Wed Sep 13 12:18:53 PDT 2006
//    Added ShowsHotPointHighlights method.  Extents tool shows no highlights.
//
//    Mark Blair, Fri Sep 15 16:41:45 PDT 2006
//    Added methods for updating minima and maxima on an individual basis.
//    Also added running time ordinal for tagging slider changes.
//
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    Added UpdateToolAttributesWithPlotAttributes method.
//
//    Mark Blair, Thu Nov  2 12:33:23 PST 2006
//    Added methods and members for selective axis labeling in associated plot.
//
//    Mark Blair, Tue Nov 28 13:25:41 PST 2006
//    Added support for improved plot versus tool interaction.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Mark Blair, Wed Mar 14 18:04:12 PDT 2007
//    Added support for ganged axis sliders.
//
//    Mark Blair, Thu Jul 19 19:01:44 PDT 2007
//    Now uses VTK 2-D foreground renderer for everything drawn by the tool.
//    Also added ConvertPointsToPixelCoords.
//
// ****************************************************************************

class VISWINDOW_API VisitExtentsTool : public VisitInteractiveTool
{
  public:
             VisitExtentsTool(VisWindowToolProxy &);
    virtual ~VisitExtentsTool();

    virtual void Enable();
    virtual void Disable();
    virtual bool IsAvailable() const;
    virtual bool ShowsHotPointHighlights() const { return false; };

    virtual const char       *GetName() const { return "Extents"; };
    virtual avtToolInterface &GetInterface() { return Interface; };

    virtual void UpdateTool();

    virtual void ReAddToWindow();
    virtual void FullFrameOn(const double, const int);
    virtual void FullFrameOff(void);

  protected:
    void DoOneTimeInitializations();

    void GetCurrentPlotAttributes();
    void UpdateToolAttributesWithPlotAttributes();
    
    void PositionAxisSequenceMarks();
    
    void InitializeAllHotpoints();
    void InitializeButtonHotpoints();
    void InitializeMarkSliderHotpoints();
    void InitializeAxisSliderHotpoints();

    void CreateAllActors();
    void CreateButtonQuadsActor();
    void CreateButtonLogosActor();
    void CreateButtonChecksActor();
    void CreateButtonLabelsActors();
    void CreateAxisExtensionsActor();
    void CreateMarkSlidersActor();
    void CreateMarkTitlesActors();
    void CreateAxisSlidersActor();
    void CreateAxisTitlesActors();
    void CreateAxisBoundsActors();
    void CreateAxisExtentsActors();

    void DeleteAllActors();
    void DeleteButtonQuadsActor();
    void DeleteButtonLogosActor();
    void DeleteButtonChecksActor();
    void DeleteButtonLabelsActors();
    void DeleteAxisExtensionsActor();
    void DeleteMarkSlidersActor();
    void DeleteMarkTitlesActors();
    void DeleteAxisSlidersActor();
    void DeleteAxisTitlesActors();
    void DeleteAxisBoundsActors();
    void DeleteAxisExtentsActors();
    
    void AddAllActors();
    void RemoveAllActors();

    static void MoveLeftMarkOneAxisLeftButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void MoveRightMarkOneAxisLeftButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void MoveLeftMarkOneAxisRightButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void MoveRightMarkOneAxisRightButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void ShowAxisInfoAtLeftMarkIfOnButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void ShowAxisInfoAtRightMarkIfOnButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void ExpandBetweenMarkedAxesButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void UndoLastAxisSectionExpansionButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void ThresholdBetweenMarkedAxesOnlyIfOnButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void ShowAxisTitlesOnlyIfOnButtonCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void LeftAxisSelectionMarkCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void RightAxisSelectionMarkCallback(
        VisitInteractiveTool *it, CB_ENUM e, int ctrl, int shift, int x, int y);
    static void AxisSliderMinimumCallback(
        VisitInteractiveTool *it, CB_ENUM e, int crtl, int shift, int x, int y);
    static void AxisSliderMaximumCallback(
        VisitInteractiveTool *it, CB_ENUM e, int crtl, int shift, int x, int y);

    void ButtonAction(CB_ENUM e, int ctrl, int shift, int x, int y, int buttonID);
    void PressButton(int buttonID);
    void ReleaseButton(int buttonID);
    void CallButtonCallback(int buttonID);

    void LeftAxisSelectionMark(CB_ENUM e, int ctrl, int shift, int x, int y);
    void RightAxisSelectionMark(CB_ENUM e, int ctrl, int shift, int x, int y);
    void MoveLeftMarkSliderArrow(int axisIndex);
    void MoveRightMarkSliderArrow(int axisIndex);
    void CallLeftMarkMoveCallback();
    void CallRightMarkMoveCallback();

    void AxisSliderMinimum(CB_ENUM e, int crtl, int shift, int x, int y);
    void AxisSliderMaximum(CB_ENUM e, int crtl, int shift, int x, int y);
    void GangAxisSliders(bool draggedSliderIsMin, CB_ENUM e, int x, int y);
    void UpdateGangedAxisSliderPositions(bool draggedSliderIsMin, double cursorY);
    void MoveAxisSliderMinimumArrow(int axisIndex);
    void MoveAxisSliderMaximumArrow(int axisIndex);
    void CallExtentsCallback(bool sendMinima, bool sendMaxima);
    void CallExtentMaximumCallback();

    void ExpandAxisSequenceBetweenSelectionMarks();
    void UndoPreviousAxisSequenceExpansion();
    
    int AxisClosestToTipOfArrow(double arrowTipX);
    void MakeAxisTitleText(
        char titleText[], const std::string &axisTitle, int maxTitleChars);
    void MakeDataBoundText(char boundText[], double boundValue);
    void ConvertPointsToPixelCoords(float *xyzTriples, int pointCount);

    vtkActor2D                  *buttonQuadsActor;
    vtkPolyDataMapper2D         *buttonQuadsMapper;
    vtkPolyData                 *buttonQuadsData;

    vtkActor2D                  *buttonLogosActor;
    vtkPolyDataMapper2D         *buttonLogosMapper;
    vtkPolyData                 *buttonLogosData;

    vtkActor2D                  *buttonChecksActor;
    vtkPolyDataMapper2D         *buttonChecksMapper;
    vtkPolyData                 *buttonChecksData;

    vtkActor2D                  *markSlidersActor;
    vtkPolyDataMapper2D         *markSlidersMapper;
    vtkPolyData                 *markSlidersData;

    vtkActor2D                  *axisExtensionsActor;
    vtkPolyDataMapper2D         *axisExtensionsMapper;
    vtkPolyData                 *axisExtensionsData;

    vtkActor2D                  *axisSlidersActor;
    vtkPolyDataMapper2D         *axisSlidersMapper;
    vtkPolyData                 *axisSlidersData;

    vtkUnsignedCharArray        *buttonQuadsColors;
    vtkPoints                   *buttonQuadsPoints;
    vtkPoints                   *buttonLogosPoints;
    vtkUnsignedCharArray        *buttonChecksColors;
    vtkPoints                   *buttonChecksPoints;
    vtkPoints                   *markArrowPoints;
    vtkPoints                   *axisDotPoints;
    vtkPoints                   *axisArrowPoints;
    
    std::vector<vtkTextActor *> buttonLabelsActors;
    std::vector<vtkTextActor *> markTitlesActors;
    std::vector<vtkTextActor *> axisTitlesActors;
    std::vector<vtkTextActor *> axisMinimaActors;
    std::vector<vtkTextActor *> axisMaximaActors;
    std::vector<vtkTextActor *> extentMinimaActors;
    std::vector<vtkTextActor *> extentMaximaActors;

    avtExtentsToolInterface Interface;
    
    int                  plotListIndex;
    
    int                  curTimeOrdinal;
    int                  parallelAxisCount;
    bool                 labelsAreVertical;

    double               normToWorldYScale;
    double               windowHToWRatio;
    
    double               bqHotpointRadius;
    double               buttonLogoRadius;
    double               buttonCheckRadius;
    double               msHotpointRadius;
    double               asHotpointRadius;

    int                  bqHotpointStartID;
    int                  bqHotpointStopID;
    int                  msHotpointStartID;
    int                  msHotpointStopID;
    int                  asHotpointStartID;
    int                  asHotpointStopID;
    
    double               buttonHotpointYOffset;
    double               buttonInnerRadius;
    double               buttonOuterRadius;
        
    int                  leftShownAxisID;
    int                  rightShownAxisID;
    int                  leftSelectedAxisID;
    int                  rightSelectedAxisID;
    double               leftAxisX;
    double               rightAxisX;
    double               markSliderXRange;
    double               markTitleXOffset;
    double               markArrowTipY;
    
    intVector            leftExpandAxisIDs;
    intVector            rightExpandAxisIDs;

    int                  axisTitleTextSize;
    double               axisTitleY;
    double               axisTitleDispXOffset;
    stringVector         axisTitles;

    int                  buttonLabelTextSize;
    int                  markTitleTextSize;
    int                  boundTextSize;
    
    double               minSlidableX;
    double               maxSlidableX;
    double               minSlidableY;
    double               maxSlidableY;

    double               axisMinY;
    double               axisMaxY;
    double               axisMinDispXOffset;
    double               axisMaxDispXOffset;

    double               extentMinY;
    double               extentMaxY;
    double               extentMinDispXOffset;
    double               extentMaxDispXOffset;
    
    int                  aeDotsPerAxis;
    double               aeDotInterval;
    double               aeBottomDotY;

    int                  activeAxisIndex;
    int                  activeHotPointID;
    int                  activeMinHPID;
    int                  activeMaxHPID;
    double               leftMarkHPX;
    double               rightMarkHPX;
    double               activeMinHPY;
    double               activeMaxHPY;
    double               gangedHPDeltaY;
    intVector            asHotpointAxisIDs;
    intVector            asAxisHotpointIDs;
    
    intVector            buttonLogoIDs;
    intVector            buttonCheckIDs;
    
    unsigned char        buttonColor0Red;
    unsigned char        buttonColor0Green;
    unsigned char        buttonColor0Blue;
    
    unsigned char        buttonColor1Red;
    unsigned char        buttonColor1Green;
    unsigned char        buttonColor1Blue;
    
    unsigned char        buttonColor2Red;
    unsigned char        buttonColor2Green;
    unsigned char        buttonColor2Blue;
    
    unsigned char        buttonColor3Red;
    unsigned char        buttonColor3Green;
    unsigned char        buttonColor3Blue;
    
    unsigned char        buttonLogoRed;
    unsigned char        buttonLogoGreen;
    unsigned char        buttonLogoBlue;
    
    unsigned char        markSliderRed;
    unsigned char        markSliderGreen;
    unsigned char        markSliderBlue;
    
    unsigned char        axisExtensionRed;
    unsigned char        axisExtensionGreen;
    unsigned char        axisExtensionBlue;
    
    unsigned char        axisSliderRed;
    unsigned char        axisSliderGreen;
    unsigned char        axisSliderBlue;
    
    bool                 buttonAlreadyPressed;
    
    double               buttonXs[VET_MAX_BUTTONS];
    double               buttonYs[VET_MAX_BUTTONS];
    int                  buttonFlags[VET_MAX_BUTTONS];

    char                *buttonLabels[VET_MAX_BUTTON_LABELS];
    double               buttonLabelXs[VET_MAX_BUTTON_LABELS];
    double               buttonLabelYs[VET_MAX_BUTTON_LABELS];
    int                  buttonLabelJusts[VET_MAX_BUTTON_LABELS];
};

#endif
